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
//! @file src/window.c
//!
//! @brief Determines some basics for windows for gWaei
//!

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <gio/gio.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


GwWindow* gw_window_new (const GwWindowType TYPE)
{
    //Declarations
    GwWindow *window;
    GwWindow *parent;

    //Initializations
    window = NULL;
    parent = NULL;

    switch (TYPE)
    {
      case GW_WINDOW_SEARCH:
        window = GW_WINDOW (gw_searchwindow_new ());
        gtk_window_set_position (window->toplevel, GTK_WIN_POS_MOUSE);
        break;
      case GW_WINDOW_SETTINGS:
        parent = gw_app_get_window (app, GW_WINDOW_SEARCH, NULL);
        window = GW_WINDOW (gw_settingswindow_new (parent));
        if (parent != NULL)
        {
          gtk_window_set_destroy_with_parent (window->toplevel, TRUE);
          gtk_window_set_position (window->toplevel, GTK_WIN_POS_CENTER_ON_PARENT);
          gtk_window_set_modal (window->toplevel, TRUE);
        }
        break;
      case GW_WINDOW_RADICALS:
        parent = gw_app_get_window (app, GW_WINDOW_SEARCH, NULL);
        window = GW_WINDOW (gw_radicalswindow_new (parent));
        break;
/*
      case GW_WINDOW_KANJIPAD:
        window = GW_WINDOW (gw_kanjipadwindow_new ());
        break;
      case GW_WINDOW_DICTIONARYINSTALL:
        window = GW_WINDOW (gw_dictinstwindow_new ());
        break;
      case GW_WINDOW_INSTALLPROGRESS:
        window = GW_WINDOW (gw_installprogresswindow_new ());
        break;
*/
      default:
        g_assert_not_reached ();
        window = NULL;
        break;
    }
    return window;
}


void gw_window_init (GwWindow *window, const GwWindowType TYPE, const char* UI_XML_FILENAME, const char* WINDOW_ID)
{
    window->builder = gtk_builder_new ();
    gw_window_load_ui_xml (window, UI_XML_FILENAME);
    window->toplevel = GTK_WINDOW (gtk_builder_get_object (window->builder, WINDOW_ID));
    window->type = TYPE;
    gw_window_set_transient_for (window, NULL);
}

void gw_window_deinit (GwWindow *window)
{
    gtk_widget_destroy (GTK_WIDGET (window->toplevel));
    g_object_unref (window->builder);
}


void gw_window_destroy (GwWindow *window)
{
    switch (window->type)
    {
      case GW_WINDOW_SEARCH:
        gw_searchwindow_destroy (GW_SEARCHWINDOW (window));
        break;
      case GW_WINDOW_SETTINGS:
        gw_settingswindow_destroy (GW_SETTINGSWINDOW (window));
        break;
      case GW_WINDOW_RADICALS:
        gw_radicalswindow_destroy ((GwRadicalsWindow*) window);
        break;
/*
      case GW_WINDOW_KANJIPAD:
        gw_kanjipadwindow_destroy ((GwKanjipadWindow*) window);
        break;
      case GW_WINDOW_DICTIONARYINSTALL:
        gw_dictinstwindow_destroy ((GwDictInstWindow*) window);
        break;
      case GW_WINDOW_INSTALLPROGRESS:
        gw_installprogreswindow_destroy ((GwInstallProgressWindow*) window);
        break;
*/
      default:
        g_assert_not_reached ();
        break;
    }
}


void gw_window_set_transient_for (GwWindow *window, GwWindow *transient_for)
{
    window->transient_for = transient_for;
    if (transient_for != NULL)
    {
      gtk_window_set_transient_for (window->toplevel, transient_for->toplevel);
    }
}


//!
//! @brief Loads the gtk builder xml file from the usual paths
//!
//! @param filename The filename of the xml file to look for
//!
gboolean gw_window_load_ui_xml (GwWindow *window, const char *filename) {
    g_assert (window->builder != NULL && filename != NULL);

    //Declarations
    char *paths[4];
    char **iter;
    char *path;
    gboolean loaded;

    //Initializations
    paths[0] = g_build_filename ("ui", filename, NULL);
    paths[1] = g_build_filename ("..", "share", PACKAGE, filename, NULL);
    paths[2] = g_build_filename (DATADIR2, PACKAGE, filename, NULL);
    paths[3] = NULL;
    loaded = FALSE;

    //Search for the files
    for (iter = paths; *iter != NULL && loaded == FALSE; iter++)
    {
      path = *iter;
      if (g_file_test (path, G_FILE_TEST_IS_REGULAR) && gtk_builder_add_from_file (window->builder, path,  NULL))
      {
        gtk_builder_connect_signals (window->builder, NULL);
        loaded = TRUE;
      }
    }

    //Cleanup
    for (iter = paths; *iter != NULL; iter++)
    {
      g_free (*iter);
    }

    //Bug test
    g_assert (loaded);

    //Return
    return loaded;
}


