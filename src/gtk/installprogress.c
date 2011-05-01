#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>


gpointer _installprogress_install_thread (gpointer);
int _update_dictinst_install_progress_cb (double, gpointer);
GThread *_thread = NULL;
gint _timeoutid = 0;
gboolean _installprogress_update_ui_timeout (gpointer);


void gw_installprogress_initialize ()
{
    gw_common_load_ui_xml ("installprogress.ui");
}


void gw_installprogress_free ()
{
}

//!
//! @brief Starts the install when the add button on the dictionary chooser is selected
//!
G_MODULE_EXPORT void gw_installprogress_start_cb (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder;
    GtkWidget *install_progress_dialog;
    GtkWidget *dictionary_install_dialog;
    GtkWidget *settings_window;
    GError *error;

    //Initializations
    builder = gw_common_get_builder ();
    install_progress_dialog = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_dialog"));
    dictionary_install_dialog = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_dialog"));
    settings_window = GTK_WIDGET (gtk_builder_get_object (builder, "settings_window"));
    error = NULL;

    //Remove the prevous window
    gtk_widget_destroy (GTK_WIDGET (dictionary_install_dialog));

    //Set the new window
    gtk_window_set_transient_for (GTK_WINDOW (install_progress_dialog), GTK_WINDOW (settings_window));
    gtk_window_set_position (GTK_WINDOW (settings_window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_widget_show (GTK_WIDGET (install_progress_dialog));

    if (_thread == NULL)
      _thread = g_thread_create (_installprogress_install_thread, NULL, TRUE, &error);

    gw_ui_handle_error (&error, TRUE);
}


gpointer _installprogress_install_thread (gpointer data)
{
    //Declarations
    GList *list;
    GList *iter;
    GwDictInst *di;
    GError *error;

    //Initializations
    list = gw_dictinstlist_get_list ();
    error = NULL;

    //Do the installation
    for (iter = list; iter != NULL && error == NULL; iter = iter->next)
    {
      di = (GwDictInst*) iter->data;
      if (di->selected)
      {
        _timeoutid = g_timeout_add (100, _installprogress_update_ui_timeout, di);
        gw_dictinst_install (di, _update_dictinst_install_progress_cb, &error);
        g_mutex_lock (di->mutex);
          di->selected = FALSE;
        g_mutex_unlock (di->mutex);
      }
    }

    //Cleanup
    gw_ui_handle_error (&error, TRUE);

    _thread = NULL;
    g_source_remove (_timeoutid);

    GtkBuilder *builder;
    GtkWidget *install_progress_dialog;
    gdk_threads_enter ();
      builder = gw_common_get_builder ();
      install_progress_dialog = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_dialog"));
      gtk_widget_destroy (GTK_WIDGET (install_progress_dialog));
    gdk_threads_leave ();
}


int _update_dictinst_install_progress_cb (double fraction, gpointer data)
{
    //Declarations
    GwDictInst *di;
    int percent;

    //Initializations
    di = data;
    percent = (int) (fraction * 100.0);

    g_mutex_lock (di->mutex); 
    di->progress = fraction;
    g_mutex_unlock (di->mutex);
}


//!
//! @brief Callback to update the install dialog progress.  Should be run in a separate thread
//!
gboolean _installprogress_update_ui_timeout (gpointer data)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *progressbar;
    GtkWidget *label;
    GList *list;
    GList *iter;
    int left_to_install;
    GwDictInst *di;
    char *text_installing;
    char *text_left;
    char *text_message;
    char *text_progressbar;

    //Initializations
    di = data;
    builder = gw_common_get_builder ();
    label = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_label"));
    progressbar = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_progressbar"));
    list = gw_dictinstlist_get_list ();
    left_to_install = 0;

    //Calculate the number of dictionaries left to install
    for (iter = list; iter != NULL; iter = iter->next)
    {
      di = iter->data;

      g_mutex_lock (di->mutex);
      if (di->selected)
      {
        left_to_install++;
      }
      g_mutex_unlock (di->mutex);
    }

    di = data;
    text_progressbar = gw_dictinst_get_status_string (di, TRUE);
    text_left = g_strdup_printf (ngettext("%d Dictionary left to process...", "%d Dictionaries left to process...", left_to_install), left_to_install);
    text_installing =  g_strdup_printf (gettext("Installing %s..."), di->longname);
    text_message = g_markup_printf_escaped ("%s\n\n<b>%s</b>", text_left, text_installing);

    g_mutex_lock (di->mutex);
      gtk_label_set_markup (GTK_LABEL (label), text_message);
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progressbar), di->progress);
      gtk_progress_bar_set_text (GTK_PROGRESS_BAR (progressbar), text_progressbar);
    g_mutex_unlock (di->mutex);

    //Cleanup
    g_free (text_progressbar);
    g_free (text_left);
    g_free (text_installing);
    g_free (text_message);

    return TRUE;
}


