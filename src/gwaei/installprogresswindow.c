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
//! @file installprogress-window.c
//!
//! @brief To be written
//!

#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


static gpointer _installprogresswindow_install_thread (gpointer);

G_DEFINE_TYPE (GwSearchWindow, gw_searchwindow, GW_TYPE_WINDOW);

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* gw_searchwindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwSearchWindow *window;

    //Initializations
    window = GW_SEARCHWINDOW (g_object_new (GW_TYPE_SEARCHWINDOW,
                                            "type",        GTK_WINDOW_TOPLEVEL,
                                            "application", GW_APPLICATION (application),
                                            "ui-xml",      "searchwindow.ui",
                                            NULL));

    return GTK_WINDOW (window);
}


void gw_installprogresswindow_init (GwInstallProgressWindow *window, GwSettingsWindow *transient_for)
{
    memset(window->priv, 0, sizeof(GwInstallProgressWindowPrivate));

    window->install_fraction = 0.0;
    window->mutex = g_mutex_new ();


    window->label = GTK_LABEL (gtk_builder_get_object (window->builder, "install_progress_label"));
    window->sublabel = GTK_LABEL (gtk_builder_get_object (window->builder, "sub_install_progress_label"));
    window->progressbar = GTK_PROGRESS_BAR (gtk_builder_get_object (window->builder, "install_progress_progressbar"));

    gw_window_set_transient_for (GW_WINDOW (window), GW_WINDOW (transient_for));
}


void gw_installprogresswindow_deinit (GwInstallProgressWindow *window)
{
    g_mutex_free (window->mutex);
}



//!
//! @brief Starts the install when the add button on the dictionary chooser is selected
//!
void gw_installprogresswindow_start (GwInstallProgressWindow *window)
{
    //Sanity check
    g_assert (window != NULL);

    //Declarations
    GError *error;

    //Initializations
    error = NULL;

    //Set the new window
    g_thread_create (_installprogresswindow_install_thread, window, FALSE, &error);

    gw_app_handle_error (app, GW_WINDOW (window->transient_for), TRUE, &error);
}


static gpointer _installprogresswindow_install_thread (gpointer data)
{
    //Declarations
    GwInstallProgressWindow *window;
    GwSettingsWindow *settingswindow;
    GList *iter;
    LwDictInst *di;
    gint timeoutid;
    GError *error;

    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (data);
    if (window == NULL) return NULL;
    settingswindow = GW_SETTINGSWINDOW (window->transient_for);
    error = NULL;

    //Do the installation
    timeoutid = g_timeout_add (100, gw_installprogresswindow_update_ui_timeout, window);
    for (iter = settingswindow->dictinstlist->list; iter != NULL && error == NULL; iter = iter->next)
    {
      di = LW_DICTINST (iter->data);
      if (di->selected)
      {
        g_mutex_lock (window->mutex);
        window->di = di;
        g_mutex_unlock (window->mutex);
        lw_dictinst_install (di, gw_installprogresswindow_update_dictinst_cb, window, &error);
      }
    }

    g_mutex_lock (window->mutex);
    //This will clue the progress window to close itself
    window->di = NULL;
    g_mutex_unlock (window->mutex);

    //Cleanup
gdk_threads_enter ();
    gw_app_handle_error (app, GW_WINDOW (settingswindow), TRUE, &error);
    gw_dictinfolist_reload (app->dictinfolist, app->preferences);
    lw_dictinstlist_set_cancel_operations (settingswindow->dictinstlist, FALSE);
    if (settingswindow->dictinstlist != NULL)
    {
      lw_dictinstlist_free (settingswindow->dictinstlist);
      settingswindow->dictinstlist = NULL;
    }
    gw_settingswindow_check_for_dictionaries (settingswindow);
gdk_threads_leave ();

    return NULL;
}


