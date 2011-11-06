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
#include <gwaei/installprogresswindow-private.h>


static gpointer _installprogresswindow_install_thread (gpointer);

G_DEFINE_TYPE (GwInstallProgressWindow, gw_installprogresswindow, GW_TYPE_WINDOW)

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* gw_installprogresswindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwInstallProgressWindow *window;

    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (g_object_new (GW_TYPE_INSTALLPROGRESSWINDOW,
                                            "type",        GTK_WINDOW_TOPLEVEL,
                                            "application", GW_APPLICATION (application),
                                            "ui-xml",      "installprogresswindow.ui",
                                            NULL));

    return GTK_WINDOW (window);
}


static void gw_installprogresswindow_init (GwInstallProgressWindow *window)
{
    window->priv = GW_INSTALLPROGRESSWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwInstallProgressWindowPrivate));
}


static void gw_installprogresswindow_finalize (GObject *object)
{
    GwInstallProgressWindow *window;
    GwInstallProgressWindowPrivate *priv;

    window = GW_INSTALLPROGRESSWINDOW (object);
    priv = window->priv;
 
    g_mutex_free (priv->mutex); priv->mutex = NULL;
    priv->label = NULL;
    priv->sublabel = NULL;
    priv->progressbar = NULL;

    G_OBJECT_CLASS (gw_installprogresswindow_parent_class)->finalize (object);
}


static void gw_installprogresswindow_constructed (GObject *object)
{
    //Declarations
    GwInstallProgressWindow *window;
    GwInstallProgressWindowPrivate *priv;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_installprogresswindow_parent_class)->constructed (object);
    }

    window = GW_INSTALLPROGRESSWINDOW (object);
    priv = window->priv;

    priv->mutex = g_mutex_new ();
    priv->label = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "install_progress_label"));
    priv->sublabel = GTK_LABEL (gw_window_get_object (GW_WINDOW (window), "sub_install_progress_label"));
    priv->progressbar = GTK_PROGRESS_BAR (gw_window_get_object (GW_WINDOW (window), "install_progress_progressbar"));
}


static void
gw_installprogresswindow_class_init (GwInstallProgressWindowClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gw_installprogresswindow_constructed;
  object_class->finalize = gw_installprogresswindow_finalize;

  g_type_class_add_private (object_class, sizeof (GwInstallProgressWindowPrivate));
}


//!
//! @brief Starts the install when the add button on the dictionary chooser is selected
//!
void gw_installprogresswindow_start (GwInstallProgressWindow *window)
{
    //Sanity check
    g_assert (window != NULL);

    //Declarations
    GwApplication *application;
    GError *error;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    error = NULL;

    //Set the new window
    g_thread_create (_installprogresswindow_install_thread, window, FALSE, &error);

    gw_application_handle_error (application, gtk_window_get_transient_for (GTK_WINDOW (window)), TRUE, &error);
}


static gpointer _installprogresswindow_install_thread (gpointer data)
{
    //Declarations
    GwInstallProgressWindow *window;
    GwInstallProgressWindowPrivate *priv;
    GwSettingsWindow *settingswindow;
    GwApplication *application;
    LwDictInstList *dictinstlist;
    GwDictInfoList *dictinfolist;
    GList *iter;
    LwDictInst *di;
    gint timeoutid;
    GError *error;

    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (data);
    if (window == NULL) return NULL;
    priv = window->priv;
    settingswindow = GW_SETTINGSWINDOW (gtk_window_get_transient_for (GTK_WINDOW (window)));
    application = gw_window_get_application (GW_WINDOW (window));
    dictinstlist = gw_application_get_dictinstlist (application);
    dictinfolist = gw_application_get_dictinfolist (application);
    error = NULL;

    //Do the installation
    timeoutid = g_timeout_add (100, gw_installprogresswindow_update_ui_timeout, window);
    for (iter = dictinstlist->list; iter != NULL && error == NULL; iter = iter->next)
    {
      di = LW_DICTINST (iter->data);
      if (di->selected)
      {
        g_mutex_lock (priv->mutex);
        priv->di = di;
        g_mutex_unlock (priv->mutex);
        lw_dictinst_install (di, gw_installprogresswindow_update_dictinst_cb, window, &error);
      }
    }

    g_mutex_lock (priv->mutex);
    //This will clue the progress window to close itself
    priv->di = NULL;
    g_mutex_unlock (priv->mutex);

    //Cleanup
gdk_threads_enter ();
    gw_application_handle_error (application, GTK_WINDOW (settingswindow), TRUE, &error);
    gw_dictinfolist_reload (dictinfolist);
    lw_dictinstlist_set_cancel_operations (dictinstlist, FALSE);
    if (dictinstlist != NULL)
    {
      lw_dictinstlist_free (dictinstlist);
      dictinstlist = NULL;
    }
    gw_settingswindow_check_for_dictionaries (settingswindow);
gdk_threads_leave ();

    return NULL;
}


