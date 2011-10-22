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
//! @file window.c
//!
//! @brief To be written
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
#include <gwaei/windowprivate.h>

G_DEFINE_TYPE (GwWindow, gw_window, GTK_TYPE_WINDOW);


/*
    switch (TYPE)
    {
      case GW_WINDOW_SETTINGS:
        g_assert (transient_for != NULL);
        window = GW_WINDOW (gw_settingswindow_new (GW_SEARCHWINDOW (transient_for), link));
        gtk_window_set_destroy_with_parent (window->toplevel, TRUE);
        gtk_window_set_position (window->toplevel, GTK_WIN_POS_CENTER_ON_PARENT);
        gtk_window_set_modal (window->toplevel, FALSE);
        break;
      case GW_WINDOW_RADICALS:
        g_assert (transient_for != NULL);
        window = GW_WINDOW (gw_radicalswindow_new (transient_for, link));
        gtk_window_set_destroy_with_parent (window->toplevel, TRUE);
        gtk_window_set_position (window->toplevel, GTK_WIN_POS_MOUSE);
        gtk_window_set_modal (window->toplevel, FALSE);
        break;
      case GW_WINDOW_KANJIPAD:
        window = GW_WINDOW (gw_kanjipadwindow_new (GW_SEARCHWINDOW (transient_for), link));
        gtk_window_set_destroy_with_parent (window->toplevel, TRUE);
        gtk_window_set_position (window->toplevel, GTK_WIN_POS_MOUSE);
        gtk_window_set_modal (window->toplevel, FALSE);
      break;
      case GW_WINDOW_DICTIONARYINSTALL:
        window = GW_WINDOW (gw_dictinstwindow_new (GW_SETTINGSWINDOW (transient_for), link));
        gtk_window_set_destroy_with_parent (window->toplevel, TRUE);
        gtk_window_set_position (window->toplevel, GTK_WIN_POS_CENTER_ON_PARENT);
        gtk_window_set_modal (window->toplevel, TRUE);
        break;
      case GW_WINDOW_INSTALLPROGRESS:
        window = GW_WINDOW (gw_installprogresswindow_new (GW_SETTINGSWINDOW (transient_for), link));
        gtk_window_set_destroy_with_parent (window->toplevel, TRUE);
        gtk_window_set_position (window->toplevel, GTK_WIN_POS_CENTER_ON_PARENT);
        gtk_window_set_modal (window->toplevel, TRUE);
        break;
      default:
        g_assert_not_reached ();
        window = NULL;
        break;
    }
    */


void gw_window_init (GwWindow *window)
{
    //Declarations
    GwWindowPrivate *priv;

    //Initializations
    priv = GW_WINDOW_GET_PRIVATE (window);

    gw_window_private_init (priv);
}


void gw_window_finalize (GObject *object)
{
    GwWindow *window;
    GwWindowPrivate *priv;

    priv = GW_WINDOW_GET_PRIVATE (window);

    gw_window_private_finalize (priv);
    G_OBJECT_CLASS (gw_window_parent_class)->finalize (object);
}


static void
gw_window_class_init (GwWindowClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gw_window_finalize;

  g_type_class_add_private (object_class, sizeof (GwWindowPrivate));
}


//!
//! @brief Loads the gtk builder xml file from the usual paths
//!
//! @param filename The filename of the xml file to look for
//!
gboolean gw_window_load_ui_xml (GwWindow *window, const char *filename)
{
    if (window == NULL || filename == NULL) return FALSE;

    //Declarations
    GwWindowPrivate *priv;
    char *paths[4];
    char **iter;
    char *path;
    gboolean loaded;

    //Initializations
    priv = GW_WINDOW_GET_PRIVATE (window);
    paths[0] = g_build_filename ("ui", filename, NULL);
    paths[1] = g_build_filename ("..", "share", PACKAGE, filename, NULL);
    paths[2] = g_build_filename (DATADIR2, PACKAGE, filename, NULL);
    paths[3] = NULL;
    loaded = FALSE;

    //Search for the files
    for (iter = paths; *iter != NULL && loaded == FALSE; iter++)
    {
      path = *iter;
      if (g_file_test (path, G_FILE_TEST_IS_REGULAR) && gtk_builder_add_from_file (priv->builder, path,  NULL))
      {
        gtk_builder_connect_signals (priv->builder, NULL);
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


