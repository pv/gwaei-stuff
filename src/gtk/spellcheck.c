#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

static GList *_corrections = NULL;
static GMutex *_mutex = NULL;
static gboolean _needs_spellcheck = FALSE;
static char* _query_text = NULL;

static void _menuitem_activated_cb (GtkWidget*, gpointer);
static void _populate_popup_cb (GtkEntry*, GtkMenu*, gpointer);
static gboolean _draw_underline_cb (GtkWidget*, cairo_t*, gpointer);
static void _queue_spellcheck_cb (GtkEditable*, gpointer);
static gboolean _update_spellcheck_timeout (gpointer);

void gw_spellcheck_attach_to_entry (GtkEntry*);

struct _SpellingReplacementData {
  GtkEntry *entry;
  int start_offset;
  int end_offset;
  char* replacement_text;
};
typedef struct _SpellingReplacementData _SpellingReplacementData;


struct _StreamWithData {
  int stream;
  gpointer data;
};
typedef struct _StreamWithData _StreamWithData;



static void _free_menuitem_data (GtkWidget *widget, gpointer data)
{
  //Declarations
  _SpellingReplacementData *srd;

  //Initializations
  srd = data;

  //Cleanup
  g_free (srd->replacement_text);
  free (srd);
}

static gpointer _infunc (gpointer data)
{
  //Declarations
  _StreamWithData *swd;
  FILE *file;
  size_t chunk;
  int stream;
  char *text;

  //Initializations
  swd = data;
  stream = swd->stream;
  text = swd->data;
  file = fdopen(stream, "w");

  if (file != NULL)
  {
    chunk = fwrite(text, sizeof(char), strlen(text), file);
    fclose(file);
  }

  return NULL;
}

static gpointer _outfunc (gpointer data)
{
  //Declarations
  const int MAX = 500;
  _StreamWithData *swd;
  FILE *file;
  int stream;
  char buffer[MAX];

  //Initializations
  swd = data;
  stream = swd->stream;
  file = fdopen (swd->stream, "r");

  if (file != NULL)
  {
    g_mutex_lock (_mutex);

    //Clear out the old links
    while (_corrections != NULL)
    {
      g_free (_corrections->data);
      _corrections = g_list_delete_link (_corrections, _corrections);
    }

    //Add the new links
    while (fgets(buffer, MAX, file) != NULL)
    {
      if (buffer[0] != '@' && buffer[0] != '*' && strlen(buffer) > 1)
        _corrections = g_list_append (_corrections, g_strdup (buffer));
    }
    g_mutex_unlock (_mutex);

    //Cleanup
    fclose (file);
  }

  return NULL;
}

//!
//! @brief Attached the spellchecking listeners to a GtkEntry.  It can only be attached once.
//! @param entry A GtkEntry to attach to
//!
void gw_spellcheck_attach_to_entry (GtkEntry *entry)
{
  //Sanity check
  g_assert (_mutex == NULL);

  //Initializations
  _mutex = g_mutex_new ();

  g_signal_connect_after (G_OBJECT (entry), "draw", G_CALLBACK (_draw_underline_cb), NULL);
  g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (_queue_spellcheck_cb), NULL);
  g_signal_connect (G_OBJECT (entry), "populate-popup", G_CALLBACK (_populate_popup_cb), NULL);
  g_timeout_add (500, (GSourceFunc) _update_spellcheck_timeout, (gpointer) entry);
}



//
//Callbacks
//

static void _menuitem_activated_cb (GtkWidget *widget, gpointer data)
{
  //Declarations
  _SpellingReplacementData *srd;
  char *text;
  char *buffer;
  char *replacement;
  int start_offset;
  int end_offset;
  int index;

  //Initializations
  srd = data;
  replacement = srd->replacement_text;
  start_offset = srd->start_offset;
  end_offset = srd->end_offset;
  text = g_strdup (gtk_entry_get_text (GTK_ENTRY (srd->entry)));
  buffer = (char*) malloc (sizeof(char) * (strlen(replacement) + strlen(text)));

  strcpy(buffer, text);
  strcpy (buffer + start_offset, replacement);
  strcat (buffer, text + end_offset);

  index = gtk_editable_get_position (GTK_EDITABLE (srd->entry));
  if (index > end_offset || index > start_offset + strlen(replacement))
    index = index - (end_offset - start_offset) + strlen(replacement);
  gtk_entry_set_text (GTK_ENTRY (srd->entry), buffer);
  gtk_editable_set_position (GTK_EDITABLE (srd->entry), index);

  //Cleanup
  free (buffer);
  g_free (text);
}


static int _get_y_offset (GtkEntry *entry)
{
    //Declarations
    PangoRectangle rect;
    PangoLayout *layout;

    int allocation_offset;
    int layout_offset;
    int rect_offset;

    //Initializations
    layout = gtk_entry_get_layout (GTK_ENTRY (entry));
    pango_layout_get_pixel_extents (layout, &rect, NULL);
    rect_offset = rect.height;
    allocation_offset = gtk_widget_get_allocated_height (GTK_WIDGET (entry));
    gtk_entry_get_layout_offsets (GTK_ENTRY (entry), NULL, &layout_offset);

    return (((allocation_offset - rect_offset) / 2) - layout_offset);
}

static int _get_x_offset (GtkEntry *entry)
{
    //Declarations
    PangoRectangle rect;
    PangoLayout *layout;

    int allocation_offset;
    int layout_offset;
    int rect_offset;

    //Initializations
    layout = gtk_entry_get_layout (GTK_ENTRY (entry));
    pango_layout_get_pixel_extents (layout, &rect, NULL);
    rect_offset = rect.width;
    allocation_offset = gtk_widget_get_allocated_height (GTK_WIDGET (entry));
    gtk_entry_get_layout_offsets (GTK_ENTRY (entry), &layout_offset, NULL);

    return (layout_offset);
}

static int _get_string_index (GtkEntry *entry, int x, int y)
{
    //Declarations
    int layout_index;
    int entry_index;
    int trailing;
    PangoLayout *layout;

    //Initalizations
    layout = gtk_entry_get_layout (GTK_ENTRY (entry));
    if (pango_layout_xy_to_index (layout, x * PANGO_SCALE, y * PANGO_SCALE, &layout_index, &trailing))
      entry_index = gtk_entry_layout_index_to_text_index (GTK_ENTRY (entry), layout_index);
    else
      entry_index = -1;

    return entry_index;
}


void _populate_popup_cb (GtkEntry *entry, GtkMenu *menu, gpointer data)
{
    //Declarations
    GtkWidget *menuitem;
    char **split;
    char **info;
    char **replacements;
    const char *text;
    GList *iter;

    int index;
    int xpointer, ypointer, xoffset, yoffset, x, y;
    _SpellingReplacementData *srd;
    int start_offset, end_offset;
    int i;

    //Initializations
    text = gtk_entry_get_text (GTK_ENTRY (entry));
    gtk_widget_get_pointer (GTK_WIDGET (entry), &xpointer, &ypointer);
    xoffset = _get_x_offset (entry);
    yoffset = _get_y_offset (entry);
    x = xpointer - xoffset;
    y = yoffset; //Since a GtkEntry is single line, we want the y to always be in the area
    index =  _get_string_index (entry, x, y);

    g_mutex_lock (_mutex);
    for (iter = _corrections; index > -1 && iter != NULL; iter = iter->next)
    {
      //Create the start and end offsets 
      split = g_strsplit (iter->data, ":", 2);
      info = g_strsplit (split[0], " ", -1); 
      start_offset = (int) g_ascii_strtoull (info[3], NULL, 10);
      end_offset = strlen(info[1]) + start_offset;

      //If the mouse position is between the offsets, create the popup menuitems
      if (index >= start_offset && index <= end_offset)
      {
        replacements = g_strsplit (split[1], ",", -1);

        //Separator
        if (replacements[0] != NULL)
        {
          menuitem = gtk_separator_menu_item_new ();
          gtk_widget_show (GTK_WIDGET (menuitem));
          gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
        }

        //Menuitems
        for (i = 0; replacements[i] != NULL; i++)
        {
          g_strstrip(replacements[i]);
          menuitem = gtk_menu_item_new_with_label (replacements[i]);
          srd = (_SpellingReplacementData*) malloc (sizeof(_SpellingReplacementData));
          srd->entry = entry;
          srd->start_offset = start_offset;
          srd->end_offset = end_offset;
          srd->replacement_text = g_strdup (replacements[i]);
          g_signal_connect (G_OBJECT (menuitem), "destroy", G_CALLBACK (_free_menuitem_data), (gpointer) srd);
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (_menuitem_activated_cb), (gpointer) srd);
          gtk_widget_show (GTK_WIDGET (menuitem));
          gtk_menu_shell_append (GTK_MENU_SHELL (menu), menuitem);
        }
        g_strfreev (replacements);
      }
      g_strfreev (split);
      g_strfreev (info);
    }
    g_mutex_unlock (_mutex);
}

gboolean _get_line_coordinates (GtkEntry *entry, int startindex, int endindex, int *x, int *y, int *x2, int *y2)
{
    //Declarations
    int index;
    PangoLayout *layout;
    PangoRectangle rect;
    PangoLayoutIter *iter;
    int xoffset, yoffset;

    //Initializations
    layout = gtk_entry_get_layout (GTK_ENTRY (entry));
    iter = pango_layout_get_iter (layout);
    xoffset = _get_x_offset (GTK_ENTRY (entry));
    yoffset = _get_y_offset (GTK_ENTRY (entry));
    *x = *y = *x2 = *y2 = 0;

    do {
      index = pango_layout_iter_get_index (iter);
      pango_layout_iter_get_char_extents  (iter, &rect);
      if (index == startindex)
      {
        *x = PANGO_PIXELS (rect.x) + xoffset;
        *y = PANGO_PIXELS (rect.y) + yoffset;
      }
      if (index == endindex - 1)
      {
        *x2 = PANGO_PIXELS (rect.width + rect.x) + xoffset;
        *y2 = PANGO_PIXELS (rect.height + rect.y) + yoffset;
      }
    } while (pango_layout_iter_next_char (iter));

    //Cleanup
    pango_layout_iter_free (iter);

    return (x > 0 && y > 0 && x2 > 0 && y2 > 0);
}

void _draw_line (cairo_t *cr, int x, int y, int x2, int y2)
{
    //Declarations
    int ydelta;
    int xdelta;
    int i;
    gboolean up;

    //Initializations
    xdelta = 2;
    ydelta = 2;
    up = FALSE;
    y += ydelta;
    x++;

    cairo_set_line_width (cr, 0.8);
    cairo_set_source_rgba (cr, 1.0, 0.0, 0.0, 0.8);

    cairo_move_to (cr, x, y2);
    for (i = x + xdelta; i < x2; i += xdelta)
    {
      if (up)
        y2 -= ydelta;
      if (!up)
        y2 += ydelta;
      up = !up;

      cairo_line_to (cr, i, y2);
    }
    cairo_stroke (cr);
}



gboolean _draw_underline_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
  //Declarations
  gint x, y, x2, y2;
  PangoLayout *layout;
  GList *iter;
  char **info;
  char **atoms;
  int start_offset, end_offset;

  //Initializations
  layout = gtk_entry_get_layout (GTK_ENTRY (widget));

  g_mutex_lock (_mutex);
  for (iter = _corrections; iter != NULL; iter = iter->next)
  {
    if (iter->data == NULL) continue;

    info = g_strsplit (iter->data, ":", -1);
    atoms = g_strsplit (info[0], " ", -1);

    start_offset = (int) g_ascii_strtoull (atoms[3], NULL, 10);
    end_offset = strlen(atoms[1]) + start_offset;
    start_offset = gtk_entry_text_index_to_layout_index (GTK_ENTRY (widget), start_offset);
    end_offset = gtk_entry_text_index_to_layout_index (GTK_ENTRY (widget), end_offset);

    //Calculate the line
    if (_get_line_coordinates (GTK_ENTRY (widget), start_offset, end_offset, &x, &y, &x2, &y2))
    {
      _draw_line (cr, x, y, x2, y2);
    }

    g_strfreev (info);
    g_strfreev (atoms);
  }
  g_mutex_unlock (_mutex);

  return FALSE;
}


void _queue_spellcheck_cb (GtkEditable *editable, gpointer data)
{
    g_mutex_lock (_mutex);

    if (_query_text == NULL)
      _query_text = g_strdup (gtk_entry_get_text (GTK_ENTRY (editable)));

    if (strcmp(_query_text, gtk_entry_get_text (GTK_ENTRY (editable))) != 0)
    {
      //Clear out the old links
      while (_corrections != NULL)
      {
        g_free (_corrections->data);
        _corrections = g_list_delete_link (_corrections, _corrections);
      }

      g_free (_query_text);
      _query_text = g_strdup (gtk_entry_get_text (GTK_ENTRY (editable)));
      _needs_spellcheck = TRUE;
    }
    g_mutex_unlock (_mutex);
}

static gboolean _update_spellcheck_timeout (gpointer data)
{
    if (_needs_spellcheck == FALSE) return TRUE;

    _needs_spellcheck = FALSE;

    GtkEditable *editable;
    char *argv[] = { ENCHANT, "-a", "-d", "en", NULL};
    char *text;
    GPid pid;
    int stdin_stream;
    int stdout_stream;
    GError *error;
    gboolean success;
    GThread *inthread;
    GThread *outthread;
    _StreamWithData indata;
    _StreamWithData outdata;

    editable = GTK_EDITABLE (data);
    text = g_strdup (gtk_entry_get_text (GTK_ENTRY (editable)));
    error = NULL;

    success = g_spawn_async_with_pipes (
      NULL, 
      argv,
      NULL,
      0,
      NULL,
      NULL,
      &pid,
      &stdin_stream,
      &stdout_stream,
      NULL,
      &error
    );

    if (success)
    {
      indata.stream = stdin_stream;
      indata.data = text;
      inthread = g_thread_create (_infunc, (gpointer) &indata, TRUE, &error);

      outdata.stream = stdout_stream;
      outdata.data = text;
      outthread = g_thread_create (_outfunc, (gpointer) &outdata, TRUE, &error);

      g_thread_join (inthread);
      g_thread_join (outthread);

      g_spawn_close_pid (pid);
    }

    //Cleanup
    if (text != NULL) g_free (text);
    if (error !=NULL) 
    {
      printf("ERROR: %s\n", error->message);
      g_error_free (error);
    }

    gtk_widget_queue_draw (GTK_WIDGET (data));
  
    return TRUE;
}


