/******************************************************************************
    AUTHOR:
    KanjiPad - Japanese handwriting recognition front end
    Copyright (C) 1997 Owen Taylor
    File heavily modified and updated by Zachary Dovel.

    LICENSE:
    This file is part of gWaei.

    gWaei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gWaei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

//!
//! @file src/gtk-main-callbacks.c
//!
//! @brief Abstraction layer for the drawing area
//!
//! Callbacks for activities initiated by the user. Most of the gtk code here
//! should still be abstracted to the interface C file when possible.
//!


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <gtk/gtk.h>
#include <pango/pangocairo.h>

#include <gwaei/gwaei.h>


//!
//! @brief To be written
//!
void gw_kanjipadwindow_initialize_candidates (GwKanjipadWindow *window)
{
    //Declarations
    gint mask;
    PangoFontDescription *desc;

    //Initializations
    mask = (GDK_EXPOSURE_MASK  | GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK);
    desc = pango_font_description_from_string ("Sans 18");

    g_signal_connect (window->candidates, "configure_event", G_CALLBACK (gw_kanjipadwindow_candidatearea_configure_event_cb), window->toplevel);
    g_signal_connect (window->candidates, "draw", G_CALLBACK (gw_kanjipadwindow_candidatearea_draw_cb), window->toplevel);
    g_signal_connect (window->candidates, "button_press_event", G_CALLBACK (gw_kanjipadwindow_candidatearea_button_press_event_cb), window->toplevel);
    gtk_widget_add_events (GTK_WIDGET (window->candidates), mask);

    if (desc != NULL)
    {
      gtk_widget_override_font (GTK_WIDGET (window->candidates), desc);
      pango_font_description_free (desc);
    }
}


//!
//! @brief To be written
//!
static void _kanjipadwindow_get_candidate_character_size (GwKanjipadWindow *window, int *width, int *height)
{
    PangoLayout *layout;

    layout = gtk_widget_create_pango_layout (GTK_WIDGET (window->candidates), "\xe6\xb6\x88");

    if (layout != NULL)
    {
      pango_layout_get_pixel_size (layout, width, height);
      g_object_unref (layout);
    }
    else
    {
      *width = 0;
      *height = 0;
    }
}


//!
//! @brief To be written
//!
static gchar *_kanjipadwindow_utf8_for_char (char wide_character[])
{
    //Declarations
    gchar *string_utf;
    GError *error;
    gchar str[3];

    //Initializaitons
    error = NULL;
    str[0] = wide_character[0] + 0x80;
    str[1] = wide_character[1] + 0x80;
    str[2] = '\0';
    string_utf = g_convert (str, -1, "UTF-8", "EUC-JP", NULL, NULL, &error);

    if (error != NULL)
    {
      g_printerr ("Cannot convert string from EUC-JP to UTF-8: %s\n",
      error->message);
      exit (EXIT_FAILURE);
    }

    return string_utf;
}


//!
//! @brief To be written
//!
static void _kanjipadwindow_draw_candidate_character (GwKanjipadWindow *window, int index, int selected)
{
    //Declarations
    PangoLayout *layout;
    gchar *string_utf;
    gint char_width, char_height;
    gint x, y;
    int allocated_width, allocated_height;
    int width, height;
    cairo_t *cr;
    GtkStyleContext *context;
    GdkRGBA fgcolorn;
    GdkRGBA bgcolorn;
    GdkRGBA fgcolors;
    GdkRGBA bgcolors;

    //Initializations
    cr = cairo_create (window->ksurface);
    allocated_width = gtk_widget_get_allocated_width (GTK_WIDGET (window->candidates));
    allocated_height = gtk_widget_get_allocated_height (GTK_WIDGET (window->candidates));
    context = gtk_widget_get_style_context (GTK_WIDGET (window->candidates));

    gtk_style_context_get_color (context, GTK_STATE_FLAG_NORMAL, &fgcolorn);
    gtk_style_context_get_background_color (context, GTK_STATE_FLAG_NORMAL, &bgcolorn);
    gtk_style_context_get_color (context, GTK_STATE_FLAG_SELECTED, &fgcolors);
    gtk_style_context_get_background_color (context, GTK_STATE_FLAG_SELECTED, &bgcolors);

    _kanjipadwindow_get_candidate_character_size (window, &char_width, &char_height);

    if (selected >= 0)
    {
      if (selected)
        cairo_set_source_rgba (cr, bgcolors.red, bgcolors.green, bgcolors.blue, 1.0);
      else
        cairo_set_source_rgba (cr, bgcolorn.red, bgcolorn.green, bgcolorn.blue, 1.0);

      x = 0;
      y = (char_height + 6) * index;
      width = allocated_width - 1;
      height = char_height + 5;

      cairo_rectangle (cr, x, y, width, height);
      cairo_fill (cr);
    }

    x = (allocated_width - char_width) / 2;
    y = (char_height + 6) * index + 3;
    cairo_translate(cr, x, y);
    string_utf = _kanjipadwindow_utf8_for_char (window->kanji_candidates[index]);
    

    layout = gtk_widget_create_pango_layout (GTK_WIDGET (window->candidates), string_utf);
    g_free (string_utf);
    
    if (selected >= 0 && selected)
      cairo_set_source_rgba (cr, fgcolors.red, fgcolors.green, fgcolors.blue, 1.0);
    else
      cairo_set_source_rgba (cr, fgcolorn.red, fgcolorn.green, fgcolorn.blue, 1.0);

    pango_cairo_update_layout (cr, layout);
    pango_cairo_show_layout (cr, layout);

    g_object_unref (layout);
    cairo_destroy (cr);
}


//!
//! @brief To be written
//!
void gw_kanjipadwindow_draw_candidates (GwKanjipadWindow *window)
{
    //Declarations
    gint width;
    gint height;
    int i;
    cairo_t *cr;

    //Initializations
    height = gtk_widget_get_allocated_height (GTK_WIDGET (window->candidates));
    width = gtk_widget_get_allocated_width (GTK_WIDGET (window->candidates));
    cr = cairo_create (window->ksurface);

    cairo_save (cr);
    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_fill (cr);
    cairo_restore (cr);

    for (i = 0; i < window->total_candidates; i++)
    {
      if (strcmp (window->kselected, window->kanji_candidates[i]) == 0)
        _kanjipadwindow_draw_candidate_character (window, i, 1);
      else
        _kanjipadwindow_draw_candidate_character (window, i, -1);
    }

    gtk_widget_queue_draw (GTK_WIDGET (window->candidates));
}


//!
//! @brief To be written
//!
G_MODULE_EXPORT gboolean gw_kanjipadwindow_candidatearea_configure_event_cb (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
    GwKanjipadWindow *window;

    window = GW_KANJIPADWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    if (window == NULL) return FALSE;

    if (window->ksurface)
      cairo_surface_destroy (window->ksurface);

    window->ksurface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, event->width, event->height);

    gw_kanjipadwindow_draw_candidates (window);

    return TRUE;
}


//!
//! @brief To be written
//!
G_MODULE_EXPORT gboolean gw_kanjipadwindow_candidatearea_draw_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    GwKanjipadWindow *window;

    window = GW_KANJIPADWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    if (window == NULL) return FALSE;

    //Sanity check
    if (window->ksurface == NULL) return FALSE;

    cairo_set_source_surface (cr, window->ksurface, 0, 0);
    cairo_paint (cr);

    return TRUE;
}


//!
//! @brief To be written
//!
static int _kanjipadwindow_erase_candidate_selection (GwKanjipadWindow *window)
{
    int i;
    if (window->kselected[0] || window->kselected[1])
    {
      for (i = 0; i < window->total_candidates; i++)
      {
printf("%d %d\n", i, window->total_candidates);
        if (strncmp (window->kselected, window->kanji_candidates[i], 2))
        {
          _kanjipadwindow_draw_candidate_character (window, i, 0);
        }
      }
    }
    return TRUE;
}


//!
//! @brief To be written
//!
static void _kanjipadwindow_primary_candidates_clear (GtkClipboard *clipboard, gpointer data)
{
    GwKanjipadWindow *window;

    window = GW_KANJIPADWINDOW (data);

    _kanjipadwindow_erase_candidate_selection (window);

    window->kselected[0] = 0;
    window->kselected[1] = 0;

    gtk_widget_queue_draw (GTK_WIDGET (window->candidates));
}


//!
//! @brief To be written
//!
static void _kanjipadwindow_primary_candidates_get (GtkClipboard *clipboard, GtkSelectionData *selection_data, guint info, gpointer data)
{
    GwKanjipadWindow *window;
    gchar *string_utf;

    window = GW_KANJIPADWINDOW (data);

    if (window->kselected[0] || window->kselected[1])
    {
      string_utf = _kanjipadwindow_utf8_for_char (window->kselected);
      gtk_selection_data_set_text (selection_data, string_utf, -1);
      g_free (string_utf);
    }
}


//!
//! @brief To be written
//!
G_MODULE_EXPORT gboolean gw_kanjipadwindow_candidatearea_button_press_event_cb (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
    //Declarations
    GwKanjipadWindow *window;
    GwSearchWindow *searchwindow;
    gint start, end;
    int j;
    gint char_height;
    GtkClipboard *clipboard;
    char *string_utf;

    //Initializations
    window = GW_KANJIPADWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    if (window == NULL) return FALSE;
    searchwindow = GW_SEARCHWINDOW (window->transient_for);
    g_assert (searchwindow != NULL);
    clipboard = gtk_clipboard_get (GDK_SELECTION_PRIMARY);

    gtk_editable_get_selection_bounds (GTK_EDITABLE (searchwindow->entry), &start, &end);
    gtk_editable_delete_text (GTK_EDITABLE (searchwindow->entry), start, end);

    static const GtkTargetEntry targets[] = {
      { "STRING", 0, 0 },
      { "TEXT",   0, 0 }, 
      { "COMPOUND_TEXT", 0, 0 },
      { "UTF8_STRING", 0, 0 }
    };

    _kanjipadwindow_erase_candidate_selection (window);
    _kanjipadwindow_get_candidate_character_size (window, NULL, &char_height);

    j = event->y / (char_height + 6);
    if (j < window->total_candidates)
    {
      gw_kanjipadwindow_draw_candidates (window); 
      strncpy(window->kselected, window->kanji_candidates[j], 2);
      _kanjipadwindow_draw_candidate_character (window, j, 1);
      
      if (!gtk_clipboard_set_with_owner (clipboard, targets, G_N_ELEMENTS (targets),
        _kanjipadwindow_primary_candidates_get, _kanjipadwindow_primary_candidates_clear, G_OBJECT (widget)))
      _kanjipadwindow_primary_candidates_clear (clipboard, widget);
    }
    else
    {
      window->kselected[0] = 0;
      window->kselected[1] = 0;
      if (gtk_clipboard_get_owner (clipboard) == G_OBJECT (widget))
        gtk_clipboard_clear (clipboard);
    }

    gtk_widget_queue_draw (widget);

    //Copy to clipboard if output_widget is NULL
    if ((window->kselected[0] || window->kselected[1]) && searchwindow->entry == NULL)
    {
      string_utf = _kanjipadwindow_utf8_for_char (window->kselected);
      gtk_clipboard_set_text (gtk_clipboard_get (GDK_SELECTION_CLIPBOARD), string_utf, -1);
      g_free (string_utf);
    }
    //Insert the text into the editable widget
    else if (window->kselected[0] || window->kselected[1])
    {
      //Append the text at the cursor position
      string_utf = _kanjipadwindow_utf8_for_char (window->kselected);
      gtk_editable_insert_text (GTK_EDITABLE(searchwindow->entry), string_utf, -1, &start);
      gtk_editable_set_position (GTK_EDITABLE(searchwindow->entry), start);
      g_free (string_utf);
    }

    //Cleanup so the user can draw the next character
    gw_kanjipadwindow_clear_drawingarea (window);

    return TRUE;
}


