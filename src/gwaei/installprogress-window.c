#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


static gpointer _installprogresswindow_install_thread (gpointer);


//!
//! @brief Setup the installprogress.c source code
//!
GwInstallProgressWindow* gw_installprogresswindow_new (GwSettingsWindow *transient_for, GList *link)
{
    GwInstallProgressWindow *temp;

    temp = (GwInstallProgressWindow*) malloc(sizeof(GwInstallProgressWindow));

    if (temp != NULL)
    {
      gw_window_init (GW_WINDOW (temp), GW_WINDOW_INSTALLPROGRESS, "installprogress.ui", "install_progress_dialog", link);
      gw_installprogresswindow_init (temp, transient_for);
    }

    return temp;
}


//!
//! @brief Free the memory used by the installprogress.c source code
//!
void gw_installprogresswindow_destroy (GwInstallProgressWindow *window)
{
    gw_window_deinit (GW_WINDOW (window));
    gw_installprogresswindow_deinit (window);

    free (window);
}


void gw_installprogresswindow_init (GwInstallProgressWindow *window, GwSettingsWindow *transient_for)
{
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
    gw_app_handle_error (app, GW_WINDOW (settingswindow), FALSE, &error);
    gw_dictinfolist_reload (app->dictinfolist, app->prefmanager);
    lw_dictinstlist_set_cancel_operations (settingswindow->dictinstlist, FALSE);
    if (settingswindow->dictinstlist != NULL)
    {
      lw_dictinstlist_free (settingswindow->dictinstlist);
      settingswindow->dictinstlist = NULL;
    }
gdk_threads_leave ();

    return NULL;
}


