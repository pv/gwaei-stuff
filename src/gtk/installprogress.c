#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>


static void _update_install_progress (GwDictInst *di);


void gw_installprogress_initialize ()
{
    gw_common_load_ui_xml ("installprogress.ui");
}


void gw_installprogress_free ()
{
}


G_MODULE_EXPORT void gw_installprogress_start_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *install_progress_dialog;
    GtkWidget *dictionary_install_dialog;
    GtkWidget *settings_window;
    GList *list;
    GList *iter;
    GwDictInst *di;

    //Initializations
    builder = gw_common_get_builder ();
    install_progress_dialog = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_dialog"));
    dictionary_install_dialog = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_dialog"));
    settings_window = GTK_WIDGET (gtk_builder_get_object (builder, "settings_window"));
    list = gw_dictinstlist_get_list ();

    //Remove the prevous window
    gtk_widget_destroy (GTK_WIDGET (dictionary_install_dialog));

    //Set the new window
    gtk_window_set_transient_for (GTK_WINDOW (install_progress_dialog), GTK_WINDOW (settings_window));
    gtk_window_set_position (GTK_WINDOW (settings_window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_widget_show (GTK_WIDGET (install_progress_dialog));

    for (iter = list; iter != NULL; iter = iter->next)
    {
      di = (GwDictInst*) iter->data;
      //gw_dictinst_install_dictionary (di, _update_install_progress);
    }

}


static void _update_install_progress (GwDictInst *di)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *progressbar;
    GtkWidget *label;
    GList *list;
    GList *iter;
    //GwDictInst *di;
    int left_to_install;
    GwDictInst *current;
    char *text;

    //Initializations
    builder = gw_common_get_builder ();
    label = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_label"));
    progressbar = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_bar"));
    list = gw_dictinstlist_get_list ();
    label = GTK_WIDGET (gtk_builder_get_object (builder, "install_progress_label"));
    left_to_install = 0;

    //Update the progress
    for (iter = list; iter != NULL; iter = iter->next)
    {
      di = iter->data;

      g_mutex_lock (di->mutex);
      if (di->selected)
      {
        if (di->progress < 1.0)
        {
          left_to_install++;
        }
      }
      g_mutex_unlock (di->mutex);
    }

    text = g_strdup_printf (gettext("%s dictionaries left to install..."), left_to_install);
    gtk_label_set_text (GTK_LABEL (label), text);

    //Cleanup
    g_free (text);
    
}


