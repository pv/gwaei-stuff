#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


static gpointer _installprogresswindow_install_thread (gpointer);
static int _installprogresswindow_update_dictinst_cb (double, gpointer);
static gboolean _installprogresswindow_update_ui_timeout (gpointer);


//!
//! @brief Setup the installprogress.c source code
//!
GwInstallProgressWindow* gw_installprogresswindow_new (GwSettingsWindow *transient_for)
{
  GwInstallProgressWindow *temp;

  temp = (GwInstallProgressWindow*) malloc(sizeof(GwInstallProgressWindow));

  if (temp != NULL)
  {
    gw_window_init (GW_WINDOW (temp), GW_WINDOW_SEARCH, "installprogress.ui", "installprogress_dialog");
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

    window->label = GTK_LABEL (gtk_builder_get_object (window->builder, "install_progress_label"));
    window->sublabel = GTK_LABEL (gtk_builder_get_object (window->builder, "sub_install_progress_label"));
    window->progressbar = GTK_PROGRESS_BAR (gtk_builder_get_object (window->builder, "install_progress_progressbar"));

    gw_window_set_transient_for (GW_WINDOW (window), GW_WINDOW (transient_for));
}


void gw_installprogresswindow_deinit (GwInstallProgressWindow *window)
{
    gtk_widget_hide (GTK_WIDGET (window->toplevel));
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

    gw_common_handle_error (&error, window->transient_for->toplevel, TRUE);
}


static gpointer _installprogresswindow_install_thread (gpointer data)
{
    //Declarations
    GwInstallProgressWindow *window;
    GwSettingsWindow *settingswindow;
    GList *iter;
    LwDictInst *di;
    GtkBuilder *builder;
    gint timeoutid;
    GError *error;

    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (data);
    if (window == NULL) return NULL;
    settingswindow = GW_SETTINGSWINDOW (window->transient_for);
    error = NULL;

    //Do the installation
    for (iter = settingswindow->dictinstlist->list; iter != NULL && error == NULL; iter = iter->next)
    {
      di = LW_DICTINST (iter->data);
      if (di->selected)
      {
        timeoutid = g_timeout_add (100, _installprogresswindow_update_ui_timeout, di);
        lw_dictinst_install (di, _installprogresswindow_update_dictinst_cb, &error);
        g_source_remove (timeoutid);
      }
    }
    g_timeout_add (100, _installprogresswindow_update_ui_timeout, NULL);

    //Cleanup
gdk_threads_enter ();
    gw_common_handle_error (&error, settingswindow->toplevel, FALSE);
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


static int _installprogresswindow_update_dictinst_cb (double fraction, gpointer data)
{
    //Declarations
    GwInstallProgressWindow *window;

    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (data);

    g_mutex_lock (window->di->mutex); 
    window->install_fraction = lw_dictinst_get_total_progress (window->di, fraction);
    g_mutex_unlock (window->di->mutex);

    return 0;
}


//!
//! @brief Callback to update the install dialog progress.  The data passed to it should be
//!        in the form of a LwDictInst.  If it is NULL, the progress window will be closed.
//!
static gboolean _installprogresswindow_update_ui_timeout (gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwInstallProgressWindow *window;
    GwSettingsWindow *settingswindow;
    LwDictInst *di;
    GList *iter;
    int current_to_install;
    int total_to_install;
    char *text_installing;
    char *text_installing_markup;
    char *text_left;
    char *text_left_markup;
    char *text_progressbar;

    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (data);
    settingswindow = GW_SETTINGSWINDOW (window->transient_for);
    current_to_install = 0;
    total_to_install = 0;

    g_mutex_lock (window->di->mutex);

    //Calculate the number of dictionaries left to install
    for (iter = settingswindow->dictinstlist->list; iter != NULL; iter = iter->next)
    {
      di = LW_DICTINST (iter->data);
      if (di != NULL && di->selected)
      {
        current_to_install++;
      }
      if (iter->data == window->di) break;
    }

    //Calculate the number of dictionaries left to install
    for (iter = settingswindow->dictinstlist->list; iter != NULL; iter = iter->next)
    {
      di = LW_DICTINST (iter->data);
      if (di->selected)
      {
        total_to_install++;
      }
    }
    
    di = window->di;

    text_progressbar =  g_markup_printf_escaped (gettext("Installing %s..."), di->filename);
    text_left = g_strdup_printf (gettext("Installing dictionary %d of %d..."), current_to_install, total_to_install);
    text_left_markup = g_markup_printf_escaped ("<big><b>%s</b></big>", text_left);
    text_installing = lw_dictinst_get_status_string (di, TRUE);
    text_installing_markup = g_markup_printf_escaped ("<small>%s</small>", text_installing);

    gtk_label_set_markup (window->label, text_left_markup);
    gtk_label_set_markup (window->sublabel, text_installing_markup);
    gtk_progress_bar_set_fraction (window->progressbar, window->install_fraction);
    gtk_progress_bar_set_text (window->progressbar, text_progressbar);

    g_mutex_unlock (window->di->mutex);

    //Cleanup
    g_free (text_progressbar);
    g_free (text_left);
    g_free (text_left_markup);
    g_free (text_installing);
    g_free (text_installing_markup);

    return TRUE;
}


