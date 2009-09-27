/* KanjiPad - Japanese handwriting recognition front end
 * Copyright (C) 1997 Owen Taylor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <gtk/gtk.h>
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "kanjipad.h"
#include "callbacks.h"
#include "padarea.h"

#include <gwaei/definitions.h>
#include <gwaei/preferences.h>

typedef struct {
  gchar d[2];
} kp_wchar;

#define WCHAR_EQ(a,b) (a.d[0] == b.d[0] && a.d[1] == b.d[1])

/* user interface elements */
static GdkPixmap *kpixmap;
GtkWidget *karea;
GtkWidget *target_text_widget = NULL;

#define MAX_GUESSES 10
kp_wchar kanjiguess[MAX_GUESSES];
int num_guesses = 0;
kp_wchar kselected;


/* globals for engine communication */
static int engine_pid;
GIOChannel *from_engine;
GIOChannel *to_engine;

static char *data_file = NULL;


static void
karea_get_char_size (GtkWidget *widget,
		     int       *width,
		     int       *height)
{
  PangoLayout *layout = gtk_widget_create_pango_layout (widget, "\xe6\xb6\x88");
  pango_layout_get_pixel_size (layout, width, height);

  g_object_unref (layout);
}


static gchar *
utf8_for_char (kp_wchar ch)
{
  gchar *string_utf;
  GError *err = NULL;
  gchar str[3];

  str[0] = ch.d[0] + 0x80;
  str[1] = ch.d[1] + 0x80;
  str[2] = 0;

  string_utf = g_convert (str, -1, "UTF-8", "EUC-JP", NULL, NULL, &err);
  if (!string_utf)
    {
      g_printerr ("Cannot convert string from EUC-JP to UTF-8: %s\n",
		  err->message);
      exit (1);
    }

  return string_utf;
}

static void
karea_draw_character (GtkWidget *w,
		      int        index,
		      int        selected)
{
  PangoLayout *layout;
  gchar *string_utf;
  gint char_width, char_height;
  gint x;

  karea_get_char_size (w, &char_width, &char_height);

  if (selected >= 0)
    {
      gdk_draw_rectangle (kpixmap,
			  selected ? w->style->bg_gc[GTK_STATE_SELECTED] :
			  w->style->white_gc,
			  TRUE,
			  0, (char_height + 6) *index, w->allocation.width - 1, char_height + 5);
    }

  string_utf = utf8_for_char (kanjiguess[index]);
  layout = gtk_widget_create_pango_layout (w, string_utf);
  g_free (string_utf);

  x = (w->allocation.width - char_width) / 2;
  
  gdk_draw_layout (kpixmap, 
		   (selected > 0) ? w->style->white_gc :
		                    w->style->black_gc,
		   x, (char_height + 6) * index + 3, layout);
  g_object_unref (layout);
}


static void
karea_draw (GtkWidget *w)
{
  gint width = w->allocation.width;
  gint height = w->allocation.height;
  int i;

  gdk_draw_rectangle (kpixmap, 
		      w->style->white_gc, TRUE,
		      0, 0, width, height);
  

  for (i=0; i<num_guesses; i++)
    {
      if (WCHAR_EQ (kselected, kanjiguess[i]))
	karea_draw_character (w, i, 1);
      else
	karea_draw_character (w, i, -1);
    }

  gtk_widget_queue_draw (w);
}

gboolean
karea_configure_event (GtkWidget *w, GdkEventConfigure *event)
{
  if (kpixmap)
    g_object_unref (kpixmap);

  kpixmap = gdk_pixmap_new (w->window, event->width, event->height, -1);

  karea_draw (w);
  
  return TRUE;
}

gboolean
karea_expose_event (GtkWidget *w, GdkEventExpose *event)
{
  if (!kpixmap)
    return 0;

  gdk_draw_drawable (w->window,
		     w->style->fg_gc[GTK_STATE_NORMAL], kpixmap,
		     event->area.x, event->area.y,
		     event->area.x, event->area.y,
		     event->area.width, event->area.height);

    return 0;
}

static int
karea_erase_selection (GtkWidget *w)
{
  int i;
  if (kselected.d[0] || kselected.d[1])
    {
      for (i=0; i<num_guesses; i++)
	{
	  if (WCHAR_EQ (kselected, kanjiguess[i]))
	    {
	      karea_draw_character (w, i, 0);
	    }
	}
    }
  return TRUE;
}

static void
karea_primary_clear (GtkClipboard *clipboard,
		                 gpointer      owner     )
{
  GtkWidget *w = owner;
  
  karea_erase_selection (w);
  kselected.d[0] = kselected.d[1] = 0;

  gtk_widget_queue_draw (w);
}

static void
karea_primary_get (GtkClipboard     *clipboard,
		               GtkSelectionData *selection_data,
		               guint             info,
		               gpointer          owner           )
{
  if (kselected.d[0] || kselected.d[1])
    {
      gchar *string_utf = utf8_for_char (kselected);
      gtk_selection_data_set_text (selection_data, string_utf, -1);
      g_free (string_utf);
    }
}


gboolean
karea_button_press_event (GtkWidget *w, GdkEventButton *event)
{
  //Clear the highlighted area of the entry and store the start position
  gint start, end;
  gtk_editable_get_selection_bounds(GTK_EDITABLE (target_text_widget), &start, &end);
  gtk_editable_delete_text (GTK_EDITABLE(target_text_widget), start, end);

  int j;
  gint char_height;
  GtkClipboard *clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);

  static const GtkTargetEntry targets[] = {
    { "STRING", 0, 0 },
    { "TEXT",   0, 0 }, 
    { "COMPOUND_TEXT", 0, 0 },
    { "UTF8_STRING", 0, 0 }
  };

  karea_erase_selection (w);

  karea_get_char_size (w, NULL, &char_height);

  j = event->y / (char_height + 6);
  if (j < num_guesses)
    {
      kselected = kanjiguess[j];
      karea_draw_character (w, j, 1);
      
      if (!gtk_clipboard_set_with_owner (clipboard, targets, G_N_ELEMENTS (targets),
					karea_primary_get, karea_primary_clear, G_OBJECT (w)))
	    karea_primary_clear (clipboard, w);
    }
  else
    {
      kselected.d[0] = 0;
      kselected.d[1] = 0;
      if (gtk_clipboard_get_owner (clipboard) == G_OBJECT (w))
	        gtk_clipboard_clear (clipboard);
    }

  gtk_widget_queue_draw (w);


  //Copy to clipboard if text_target_widget is NULL
  if ((kselected.d[0] || kselected.d[1]) && target_text_widget == NULL) {
    char *string_utf = utf8_for_char (kselected);
      gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), string_utf, -1);
    g_free (string_utf);
  }
  //Insert the text into the editable widget
  else if (kselected.d[0] || kselected.d[1]) {
    //Append the text at the cursor position
    char *string_utf;
    string_utf = utf8_for_char (kselected);
      gtk_editable_insert_text( GTK_EDITABLE(target_text_widget),
                                string_utf, -1, &start            );
      gtk_editable_set_position (GTK_EDITABLE(target_text_widget), start);
    g_free (string_utf);
  }

  //Cleanup so the user can draw the next character
  pad_area_clear (pad_area);

  return TRUE;
}


/*For use by external programs to direct Kanjipad's output*/
void kanjipad_set_target_text_widget(GtkWidget* widget) {
  target_text_widget = GTK_WIDGET (widget);
}
GtkWidget* kanjipad_get_target_text_widget() {
  return target_text_widget;
}


#define BUFLEN 256

static gboolean
engine_input_handler (GIOChannel *source, GIOCondition condition, gpointer data)
{
  static gchar *p;
  static gchar *line;
  GError *err = NULL;
  GIOStatus status;
  int i;

  status = g_io_channel_read_line (from_engine, &line, NULL, NULL, &err);
  switch (status)
    {
    case G_IO_STATUS_ERROR:
      g_printerr ("Error reading from engine: %s\n", err->message);
      exit(1);
      break;
    case G_IO_STATUS_NORMAL:
      break;
    case G_IO_STATUS_EOF:
      g_printerr ("Engine no longer exists");
      exit (1);
      break;
    case G_IO_STATUS_AGAIN:
      g_assert_not_reached ();
      break;
    }

  if (line[0] == 'K')
    {
      unsigned int t1, t2;
      p = line+1;
      for (i=0; i<MAX_GUESSES; i++)
	{
	  while (*p && isspace(*p)) p++;
	  if (!*p || sscanf(p, "%2x%2x", &t1, &t2) != 2)
	    {
	      i--;
	      break;
	    }
	  kanjiguess[i].d[0] = t1;
	  kanjiguess[i].d[1] = t2;
	  while (*p && !isspace(*p)) p++;
	}
      num_guesses = i+1;
      karea_draw(karea);
    }

  g_free (line);

  return TRUE;
}


/* Open the connection to the engine */
static void 
init_engine()
{
  gchar *argv[] = { DATADIR2 G_DIR_SEPARATOR_S PACKAGE G_DIR_SEPARATOR_S "kpengine", "--data-file", NULL, NULL };
  GError *err = NULL;
  gchar *uninstalled;
  int stdin_fd, stdout_fd;

  uninstalled = g_build_filename (".", "kpengine", NULL);
  if (g_file_test (uninstalled, G_FILE_TEST_EXISTS))
    argv[0] = uninstalled;

  if (data_file)
    argv[2] = data_file;
  else
    argv[1] = NULL;

  if (!g_spawn_async_with_pipes (NULL, /* working directory */
				 argv, NULL,	/* argv, envp */
				 0,
				 NULL, NULL,	/* child_setup */
				 &engine_pid,   /* child pid */
				 &stdin_fd, &stdout_fd, NULL,
				 &err))
    {
      GtkWidget *dialog;

      dialog = gtk_message_dialog_new (NULL, 0,
				       GTK_MESSAGE_ERROR,
				       GTK_BUTTONS_OK,
				       "Could not start engine '%s': %s",
				       argv[0], err->message);
      gtk_dialog_run (GTK_DIALOG (dialog));
      g_error_free (err);
      exit (1);
    }

  g_free (uninstalled);
  
  if (!(to_engine = g_io_channel_unix_new (stdin_fd)))
    g_error ("Couldn't create pipe to child process: %s", g_strerror(errno));
  if (!(from_engine = g_io_channel_unix_new (stdout_fd)))
    g_error ("Couldn't create pipe from child process: %s", g_strerror(errno));

  g_io_add_watch (from_engine, G_IO_IN, engine_input_handler, NULL);
}


static void initialize_window_attributes(GtkWidget *window)
{
    int leftover;

    //Setup the unique key for the window
    leftover = MAX_GCONF_KEY;
    char key[leftover];

    strncpy(key, GCPATH_GWAEI, leftover);
    leftover -= strlen(GCPATH_GWAEI);
    strncat(key, "/", leftover);
    leftover -= 1;
    strncat(key, gtk_widget_get_name(window), leftover);
    leftover -= strlen(gtk_widget_get_name(window));

    //Set a pointer at the end of the key for easy access
    char *value;
    value = &key[strlen(key)];

    //Get the stored attributes from pref
    int x, y, width, height;

    strncpy(value, "/x", leftover - strlen("/x"));
    x = gwaei_pref_get_int (key, 0);
    strncpy(value, "/y", leftover - strlen("/y"));
    y = gwaei_pref_get_int(key, 0);
    strncpy(value, "/width", leftover - strlen("/width"));
    width = gwaei_pref_get_int(key, 100);
    strncpy(value, "/height", leftover - strlen("/height"));
    height = gwaei_pref_get_int(key, 100);

    //Apply the x and y if they are within the screen size
    if (x < gdk_screen_width() && y < gdk_screen_height()) {
      gtk_window_move(GTK_WINDOW (window), x, y);
    }

    //Apply the height and width if they are sane
    if ( width  >= 100                 &&
         width  <= gdk_screen_width()  && 
         height >= 100                 && 
         height <= gdk_screen_height()    )
    {
      gtk_window_resize(GTK_WINDOW(window), width, height);
    }
}


void initialize_kanjipad ()
{
  init_engine();
}


void show_kanjipad (GtkBuilder* builder)
{
  char global_path[FILENAME_MAX];
  strcpy(global_path, DATADIR2);
  strcat(global_path, G_DIR_SEPARATOR_S);
  strcat(global_path, PACKAGE);
  strcat(global_path, G_DIR_SEPARATOR_S);
  strcat(global_path, "kanjipad.xml");
  gtk_builder_add_from_file (builder, global_path,  NULL );
  gtk_builder_connect_signals (builder, NULL);

  GtkWidget *window;
  window = GTK_WIDGET (gtk_builder_get_object(builder, "kanjipad_window"));

  initialize_window_attributes(window);
  gtk_widget_show(window);
  initialize_window_attributes(window);

  GtkWidget *drawing_area;
  drawing_area = GTK_WIDGET (gtk_builder_get_object(builder, "kdrawing_area"));
  pad_area = pad_area_create (drawing_area);
  g_signal_connect (drawing_area, "button_release_event",
		    G_CALLBACK (look_up_callback), (gpointer) to_engine);

  karea = GTK_WIDGET (gtk_builder_get_object(builder, "kguesses"));
  g_signal_connect (karea, "configure_event",
		    G_CALLBACK (karea_configure_event), NULL);
  g_signal_connect (karea, "expose_event",
		    G_CALLBACK (karea_expose_event), NULL);
  g_signal_connect (karea, "button_press_event",
		    G_CALLBACK (karea_button_press_event), NULL);
  gtk_widget_add_events (karea, GDK_EXPOSURE_MASK 
                              | GDK_BUTTON_PRESS_MASK
                              | GDK_BUTTON_RELEASE_MASK);

  PangoFontDescription *font_desc;
#ifdef G_OS_WIN32
  font_desc = pango_font_description_from_string ("MS Gothic 18");
#else
  font_desc = pango_font_description_from_string ("Sans 18");
#endif  
  gtk_widget_modify_font (karea, font_desc);
  pango_font_description_free (font_desc);
}
