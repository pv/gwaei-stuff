#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>


//Static declarations
static gpointer _installprogress_install_thread (gpointer);
static int _installprogress_update_dictinst_cb (double, gpointer);
static gboolean _installprogress_update_ui_timeout (gpointer);
static GThread *_thread = NULL;
static gint _timeoutid = 0;


//!
//! @brief Setup the installprogress.c source code
//!
void gw_installprogress_initialize ()
{
}


//!
//! @brief Free the memory used by the installprogress.c source code
//!
void gw_installprogress_free ()
{
}

//!
//! @brief Starts the install when the add button on the dictionary chooser is selected
//!
G_MODULE_EXPORT void gw_installprogress_start_cb (GtkWidget *widget, gpointer data)
{
    gw_common_load_ui_xml ("installprogress.ui");

    //Declarations
    GtkBuilder *builder;
    GtkWidget *dialog_installprogress;
    GtkWidget *dialog_dictionaryinstall;
    GtkWidget *window_settings;
    GError *error;

    //Initializations
    builder = gw_common_get_builder ();
    dialog_installprogress = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_dialog"));
    dialog_dictionaryinstall = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_dialog"));
    window_settings = GTK_WIDGET (gtk_builder_get_object (builder, "settings_window"));
    error = NULL;
printf("BREAK3\n");

    //Remove the prevous window
    gtk_widget_hide (GTK_WIDGET (dialog_dictionaryinstall));

    //Set the new window
    gtk_window_set_transient_for (GTK_WINDOW (dialog_installprogress), GTK_WINDOW (window_settings));
    gtk_window_set_position (GTK_WINDOW (window_settings), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_widget_show (GTK_WIDGET (dialog_installprogress));

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
    GtkBuilder *builder;
    GtkWidget *dialog_installprogress;

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
        gw_dictinst_install (di, _installprogress_update_dictinst_cb, &error);
        g_source_remove (_timeoutid);
      }
    }

    //Cleanup
    gw_ui_handle_error (&error, TRUE);
    _thread = NULL;

    gdk_threads_enter ();
      builder = gw_common_get_builder ();
      dialog_installprogress = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_dialog"));
      gtk_widget_hide (GTK_WIDGET (dialog_installprogress));
    gdk_threads_leave ();
}


int _installprogress_update_dictinst_cb (double fraction, gpointer data)
{
    //Declarations
    GwDictInst *di;

    //Initializations
    di = data;

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
    GtkWidget *sublabel;
    GList *list;
    GList *iter;
    int current_to_install;
    int total_to_install;
    GwDictInst *di;
    char *text_installing;
    char *text_installing_markup;
    char *text_left;
    char *text_left_markup;
    char *text_progressbar;

    //Initializations
    di = data;
    g_mutex_lock (di->mutex);

    builder = gw_common_get_builder ();
    label = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_label"));
    sublabel = GTK_WIDGET (gtk_builder_get_object (builder, "sub_install_progress_label"));
    progressbar = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_progressbar"));
    list = gw_dictinstlist_get_list ();
    current_to_install = 0;
    total_to_install = 0;


    //Calculate the number of dictionaries left to install
    for (iter = list; iter != NULL; iter = iter->next)
    {
      di = iter->data;
      if (di->selected)
      {
        current_to_install++;
      }
      if (iter->data == data) break;
    }

    //Calculate the number of dictionaries left to install
    for (iter = list; iter != NULL; iter = iter->next)
    {
      di = iter->data;
      if (di->selected)
      {
        total_to_install++;
      }
    }
    
    di = data;

    text_progressbar =  g_markup_printf_escaped (gettext("Installing %s..."), di->filename);
    text_left = g_strdup_printf (gettext("Installing dictionary %d of %d..."), current_to_install, total_to_install);
    text_left_markup = g_markup_printf_escaped ("<big><b>%s</b></big>", text_left);
    text_installing = gw_dictinst_get_status_string (di, TRUE);
    text_installing_markup = g_markup_printf_escaped ("<small>%s</small>", text_installing);

    gtk_label_set_markup (GTK_LABEL (label), text_left_markup);
    gtk_label_set_markup (GTK_LABEL (sublabel), text_installing_markup);
    gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progressbar), gw_dictinst_get_progress (di));
    gtk_progress_bar_set_text (GTK_PROGRESS_BAR (progressbar), text_progressbar);

    g_mutex_unlock (di->mutex);

    //Cleanup
    g_free (text_progressbar);
    g_free (text_left);
    g_free (text_left_markup);
    g_free (text_installing);
    g_free (text_installing_markup);

    return TRUE;
}


G_MODULE_EXPORT void gw_installprogress_cancel_cb (GtkWidget *widget, gpointer data)
{
    printf("cancel button was clicked!\n");
}