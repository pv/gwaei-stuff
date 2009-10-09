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
#include "padarea.h"

#include <gwaei/definitions.h>
#include <gwaei/preferences.h>



G_MODULE_EXPORT gboolean look_up_callback (GtkWidget *widget, GdkEventButton *event, gpointer data)
{
  GIOChannel *to_engine;
  if ((to_engine = (GIOChannel*) data) != NULL)
  {
    /*	     kill 'HUP',$engine_pid; */
    GList *tmp_list;
    GString *message = g_string_new (NULL);
    GError *err = NULL;
      
    tmp_list = pad_area->strokes;
    while (tmp_list)
    {
      GList *stroke_list = tmp_list->data;
      while (stroke_list)
      {
        gint16 x = ((GdkPoint *)stroke_list->data)->x;
        gint16 y = ((GdkPoint *)stroke_list->data)->y;
        g_string_append_printf (message, "%d %d ", x, y);
        stroke_list = stroke_list->next;
      }
      g_string_append (message, "\n");
      tmp_list = tmp_list->next;
    }
    g_string_append (message, "\n");
    if (g_io_channel_write_chars (to_engine,
          message->str, message->len,
        NULL, &err) != G_IO_STATUS_NORMAL)
    {
      g_printerr ("Cannot write message to engine: %s\n",
      err->message);
      exit (1);
    }
    if (g_io_channel_flush (to_engine, &err) != G_IO_STATUS_NORMAL)
    {
      g_printerr ("Error flushing message to engine: %s\n",
      err->message);
      exit (1);
    }
    g_string_free (message, FALSE);
  }
  return FALSE;
}


G_MODULE_EXPORT void clear_callback (GtkWidget *widget, gpointer data)
{
  pad_area_clear (pad_area);
}


G_MODULE_EXPORT void annotate_callback (GtkWidget *widget, gpointer data)
{
  pad_area_set_annotate (pad_area, !pad_area->annotate);
}


G_MODULE_EXPORT void pad_area_changed_callback (PadArea *area)
{
}


G_MODULE_EXPORT void exit_callback (GtkWidget *widget, gpointer data)
{
    GtkWidget *window = GTK_WIDGET(widget);

    //Get the window attributes
    int x, y, width, height;
    gtk_window_get_position (GTK_WINDOW (window), &x, &y);
    gtk_window_get_size (GTK_WINDOW (window), &width, &height);

    //Hide the widget now because pref can be slow
    gtk_widget_hide(window);

    //Setup our unique key for the window
    int leftover = MAX_GCONF_KEY;
    char key[leftover];

    strncpy(key, GCPATH_GW, leftover);
    leftover -= strlen(GCPATH_GW);
    strncat(key, "/", leftover);
    leftover -= 1;
    strncat(key, gtk_widget_get_name(window), leftover);
    leftover -= strlen(gtk_widget_get_name(window));

    //Set a pointer at the end of the key for easy access
    char *value;
    value = &key[strlen(key)];

    //Start sending the attributes to pref for storage

    strncpy(value, "/x", leftover - strlen("/x"));
    gw_pref_set_int (key, x);
    strncpy(value, "/y", leftover - strlen("/y"));
    gw_pref_set_int (key, y);
    strncpy(value, "/width", leftover - strlen("/width"));
    gw_pref_set_int (key, width);
    strncpy(value, "/height", leftover - strlen("/height"));
    gw_pref_set_int (key, height);

  //Finally exit the program
  //exit (EXIT_SUCCESS);
}


