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
    GwWindow *window;

    switch (TYPE)
    {
      case GW_WINDOW_SEARCH:
        window = (GwWindow*) gw_searchwindow_new ();
        break;
/*
      case GW_WINDOW_SETTINGS:
        window = (GwWindow*) gw_settingswindow_new ();
        break;
      case GW_WINDOW_RADICALS:
        window = (GwWindow*) gw_radicalswindow_new ();
        break;
      case GW_WINDOW_KANJIPAD:
        window = (GwWindow*) gw_kanjipadwindow_new ();
        break;
      case GW_WINDOW_DICTIONARYINSTALL:
        window = (GwWindow*) gw_dictinstwindow_new ();
        break;
      case GW_WINDOW_INSTALLPROGRESS:
        window = (GwWindow*) gw_installprogresswindow_new ();
        break;
*/
      default:
        g_assert_not_reached ();
        window = NULL;
        break;
    }
    return window;
}


void gw_window_destroy (GwWindow *window)
{
    switch (window->type)
    {
      case GW_WINDOW_SEARCH:
        gw_searchwindow_destroy (GW_SEARCHWINDOW (window));
        break;
/*
      case GW_WINDOW_SETTINGS:
        gw_settingswindow_destroy ((GwSettingsWindow*) window);
        break;
      case GW_WINDOW_RADICALS:
        gw_radicalswindow_destroy ((GwRadicalsWindow*) window);
        break;
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


