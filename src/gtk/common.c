/******************************************************************************
    AUTHOR:
    File written and Copyrighted by Zachary Dovel. All Rights Reserved.

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
//! @file src/common.c
//!
//! @brief Common functions used between many files
//!


#include <stdlib.h>
#include <string.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>

static GtkBuilder *_builder = NULL;


void gw_common_initialize ()
{
  _builder = gtk_builder_new ();
}

void gw_common_free ()
{
  g_object_unref (_builder);
  _builder = NULL;
}

//!
//! @brief Sets the position preferences of the window
//!
//! @param window_id The gtkbuilder window id which also acts as a preference key
//!
static void _initialize_window_attributes (char* window_id)
{
/*
    GtkBuilder *builder = gw_common_get_builder ();

    int leftover;

    GtkWidget *window;
    window = GTK_WIDGET (gtk_builder_get_object(builder, window_id));

    //Setup the unique key for the window
    char *window_schema = g_strdup_printf (GW_SCHEMA_BASE ".%s", gtk_buildable_get_name (GTK_BUILDABLE (window)));
    if (window_schema == NULL) return;

    //Some other variable declarations
    int x = 0, y = 0, width = 100, height = 100;

    //Get the stored attributes from pref
    x = gw_pref_get_int (window_schema, "x");
    y = gw_pref_get_int(window_schema, "y");
    width = gw_pref_get_int(window_schema, "width");
    height = gw_pref_get_int(window_schema, "height");

    g_free (window_schema);

    //Apply the x and y if they are within the screen size
    if (x < gdk_screen_width() && y < gdk_screen_height()) {
      gtk_window_move (GTK_WINDOW (window), x, y);
    }

    //Apply the height and width if they are sane
    if ( width  >= 100                 &&
         width  <= gdk_screen_width()  && 
         height >= 100                 && 
         height <= gdk_screen_height() &&
         strcmp (window_id, "main_window") == 0)
    {
      gtk_window_resize (GTK_WINDOW (window), width, height);
    }


    if (strcmp(window_id, "main_window") == 0 &&
        x == 0 && y == 0                        )
    {
      int half_width, half_height;
      gtk_window_get_size (GTK_WINDOW (window), &width, &height);
      half_width = (gdk_screen_width() / 2) - (width / 2);
      half_height =  (gdk_screen_height() / 2) - (height / 2);
      gtk_window_move (GTK_WINDOW (window), half_width, half_height);
    }
    else if (strcmp (window_id, "radicals_window") == 0)
    {
      //Height checking
      GtkWidget *window = GTK_WIDGET (gtk_builder_get_object (builder, "radicals_window"));
      GtkWidget *scrolledwindow = GTK_WIDGET (gtk_builder_get_object (builder, "radical_selection_scrolledwindow"));

      //Get the natural size
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_NEVER);
      gtk_widget_set_size_request (GTK_WIDGET (window), -1, -1);
      gtk_widget_queue_draw (GTK_WIDGET (window));
      int width, height;
      gtk_window_get_size (GTK_WINDOW (window), &width, &height);

      //Set the final size
      if (height > gdk_screen_height ())
        gtk_widget_set_size_request (GTK_WIDGET (window), -1, gdk_screen_height() - 100);
      else
        gtk_widget_set_size_request (GTK_WIDGET (window), -1, height);

      //Allow a vertical scrollbar
      gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);
    }
*/
}


//!
//! @brief Saves the coordinates of a windomw, then closes it
//!
//! @param window_id The gtkbuilder id of the window
//!
void gw_common_close_window (const char* window_id)
{
/*
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *window;
    window = GTK_WIDGET (gtk_builder_get_object (builder, window_id));

    //Get the window attributes
    int x, y, width, height;
    gtk_window_get_position (GTK_WINDOW (window), &x, &y);
    gtk_window_get_size (GTK_WINDOW (window), &width, &height);

    //Hide the widget now because pref can be slow
    gtk_widget_hide (window);

    //Setup our unique key for the window
    char window_schema[100];
    strcpy(window_schema, GW_SCHEMA_BASE);
    strcat(window_schema, ".");
    strcat(window_schema, gtk_buildable_get_name (GTK_BUILDABLE (window)));

    //Start sending the attributes to pref for storage
    gw_pref_set_int (window_schema, "x", x);
    gw_pref_set_int (window_schema, "y", y);
    gw_pref_set_int (window_schema, "width", width);
    gw_pref_set_int (window_schema, "height", height);
*/
}


//!
//! @brief Shows a window, using the prefs to initialize it's position and does some other checks too
//!
//! @param The gtkbuilder id of the window
//!
void gw_common_show_window (char *id)
{
/*
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *window;
    window = GTK_WIDGET (gtk_builder_get_object (builder, id));
    if (gtk_widget_get_visible (window) == TRUE) return;

    if (strcmp(id, "main_window") == 0 || strcmp (id, "radicals_window") == 0)
    {
      if (strcmp (id, "radicals_window") == 0)
        gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_UTILITY);
      _initialize_window_attributes (id);
      gtk_widget_show (window);
      _initialize_window_attributes (id);
    }
    else if (strcmp (id, "settings_window") == 0)
    {
      GtkWidget *main_window;
      main_window = GTK_WIDGET (gtk_builder_get_object(builder, "main_window"));
      gtk_window_set_transient_for (GTK_WINDOW (window), GTK_WINDOW (main_window));
      gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ON_PARENT);
      gtk_widget_show (window);
    }
    else if (strcmp (id, "kanjipad_window") == 0)
    {
      _initialize_window_attributes (id);
      gw_kanjipad_set_target_text_widget (gw_common_get_widget_by_target (GW_TARGET_ENTRY));
      gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_UTILITY);
      gtk_widget_show (window);
      _initialize_window_attributes (id);
    }
    else
    {
      gtk_widget_show (window);
    }
*/
}


//!
//! @brief Gets the GtkBuilder pointer
//!
GtkBuilder* gw_common_get_builder ()
{
    return _builder;
}



//!
//! @brief Loads the gtk builder xml file from the usual paths
//!
//! @param filename The filename of the xml file to look for
//!
gboolean gw_common_load_ui_xml (const char *filename) {
    g_assert (_builder != NULL);

    char *path = NULL;

    //Try a local path first
    if (
        (path = g_build_filename ("ui", filename, NULL)) != NULL &&
         g_file_test (path, G_FILE_TEST_IS_REGULAR) &&
         gtk_builder_add_from_file (_builder, path,  NULL)
       )
    {
      gtk_builder_connect_signals (_builder, NULL);
      g_free (path);
      path = NULL;
      return TRUE;
    }
    //Try a local path first
    if (
        (path = g_build_filename ("..", "share", PACKAGE, filename, NULL)) != NULL &&
         g_file_test (path, G_FILE_TEST_IS_REGULAR) &&
         gtk_builder_add_from_file (_builder, path,  NULL)
       )
    {
      gtk_builder_connect_signals (_builder, NULL);
      g_free (path);
      path = NULL;
      return TRUE;
    }
    //Try the global path next
    else if (
             (path = g_build_filename (DATADIR2, PACKAGE, filename, NULL)) != NULL &&
              g_file_test (path, G_FILE_TEST_IS_REGULAR) &&
             gtk_builder_add_from_file (_builder, path,  NULL)
            )
    {
      gtk_builder_connect_signals (_builder, NULL);
      g_free (path);
      path = NULL;
      return TRUE;
    }
    else {
      printf("Failed to build path. Exiting\n");
      exit (EXIT_FAILURE);
    }

    return FALSE;
}


//!
//! @brief Retrieves a special GtkWidget designated by the GwTargetOuput signature
//!
//! This function would get the target textview instead of the textbuffer.
//! The focus is on getting the frontend widget.
//!
//! @param TARGET A GwTargetOutput designating the target
//!
GtkWidget* gw_common_get_widget_by_target (const GwTargetOutput TARGET)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *container;
    GtkWidget *notebook;
    GtkWidget *page;
    int page_number;

    switch (TARGET)
    {
      case GW_TARGET_RESULTS:
        notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
        page_number =  gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
        if (page_number == -1) return NULL;
        page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_number);
        if (page == NULL) return NULL;
        return gtk_bin_get_child (GTK_BIN (page));
      case GW_TARGET_ENTRY:
        container = GTK_WIDGET (gtk_builder_get_object (builder, "search_entry_container"));
        return gtk_bin_get_child (GTK_BIN (container));
      default:
        return NULL;
    }
}


//!
//! @brief To be written
//!
gboolean gw_common_widget_equals_target (GtkWidget* widget, const GwTargetOutput TARGET)
{
    GtkWidget* target;
    target = gw_common_get_widget_by_target (TARGET);
    return (GTK_WIDGET (widget) == GTK_WIDGET (target));
}


//!
//! @brief Retrieves a special gobject designated by the GwTargetOuput signature
//!
//! This function would get the target textbuffer instead of the targettext view for example.
//! The focus is on getting the backend widget.
//!
//! @param TARGET A GwTargetOutput designating the target
//!
gpointer gw_common_get_gobject_by_target (const GwTargetOutput TARGET)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *notebook;
    GtkWidget *page;
    int page_number;
    GtkWidget *view;

    switch (TARGET)
    {
      case GW_TARGET_RESULTS:
          notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
          page_number = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
          if (page_number == -1) return NULL;
          page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_number);
          if (page == NULL) return NULL;
          if ((view = gtk_bin_get_child (GTK_BIN (page))) == NULL) return NULL;
          return G_OBJECT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (view)));
      default:
          return NULL;
    }
}

