#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>


static GtkListStore *_model = NULL;
static GtkTreeView *_view = NULL;
enum { SHORT_NAME, LONG_NAME, DESC, DICTINST_PTR, CHECKBOX_STATE, TOTAL_FIELDS };

/*
void do_dictionary_source_updated_action (GSettings *settings, gchar *key, gpointer data)
{
    char *id = (char*) data;
    char *value = g_settings_get_string (settings, key);
    char *text = NULL;
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *label = GTK_WIDGET (gtk_builder_get_object (builder, id));

    if (value != NULL && label != NULL)
    {
      text = g_strdup_printf(gettext("Source: <%s>"), value);
      gtk_label_set_text (GTK_LABEL (label), text);
      g_free (text);
    }
    g_free (value);

}
*/


G_MODULE_EXPORT void _toggled (GtkCellRendererToggle *renderer, gchar *path, gpointer data)
{
    GtkTreeIter iter;
    gboolean state;
    gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (_model), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL (_model), &iter, CHECKBOX_STATE, &state, -1);
    gtk_list_store_set (GTK_LIST_STORE (_model), &iter, CHECKBOX_STATE, !state, -1);
}

G_MODULE_EXPORT void _cursor_changed (GtkTreeView *view, gpointer data)
{
    printf("changed\n");
    //TODO: FILL THIS DATA USING THE DICTINST OBJECTS
}


void gw_dictionaryinstall_initialize ()
{
    gw_common_load_ui_xml ("dictionaryinstall.ui");

    GtkBuilder *builder = gw_common_get_builder ();
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    //Setup the model and view
    _model = gtk_list_store_new (TOTAL_FIELDS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT);
    _view = GTK_TREE_VIEW (gtk_builder_get_object (builder, "dictionary_install_treeview"));
    gtk_tree_view_set_model (GTK_TREE_VIEW (_view), GTK_TREE_MODEL (_model));
    g_signal_connect (G_OBJECT (_view), "cursor-changed", _cursor_changed, NULL);

    //Create the columns and renderer for each column
    renderer = gtk_cell_renderer_toggle_new ();
    column = gtk_tree_view_column_new_with_attributes (" ", renderer, "active", CHECKBOX_STATE, NULL);
    gtk_tree_view_append_column (_view, column);
    g_signal_connect (G_OBJECT (renderer), "toggled", _toggled, NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Name", renderer, "text", LONG_NAME, NULL);
    gtk_tree_view_append_column (_view, column);


    GtkTreeIter iter;

    gtk_list_store_append (GTK_LIST_STORE (_model), &iter);
    gtk_list_store_set (_model, &iter,
                        SHORT_NAME, "English",
                        LONG_NAME, "English Dictionary",
                        DESC, "This is edict.",
                        DICTINST_PTR, NULL,
                        CHECKBOX_STATE, TRUE,
                                               -1);

    gtk_list_store_append (GTK_LIST_STORE (_model), &iter);
    gtk_list_store_set (_model, &iter,
                        SHORT_NAME, "Kanji",
                        LONG_NAME, "Kanji Dictionary",
                        DESC, "This is kanjidic.",
                        DICTINST_PTR, NULL,
                        CHECKBOX_STATE, FALSE,
                                               -1);




/*
    char *id = NULL;

    id = g_strdup ("install_english_source_label");
    gw_prefs_add_change_listener (GW_SCHEMA_DICTIONARY, GW_KEY_ENGLISH_SOURCE, do_dictionary_source_updated_action, id);

    id = g_strdup ("install_kanji_source_label");
    gw_prefs_add_change_listener (GW_SCHEMA_DICTIONARY, GW_KEY_KANJI_SOURCE, do_dictionary_source_updated_action, id);

    id = g_strdup ("install_names_source_label");
    gw_prefs_add_change_listener (GW_SCHEMA_DICTIONARY, GW_KEY_NAMES_PLACES_SOURCE, do_dictionary_source_updated_action, id);

    id = g_strdup ("install_examples_source_label");
    gw_prefs_add_change_listener (GW_SCHEMA_DICTIONARY, GW_KEY_EXAMPLES_SOURCE, do_dictionary_source_updated_action, id);
*/

}

void gw_dictionaryinstall_free ()
{
}


//!
//! @brief opens the dictionary install dialog
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_open_dictionary_install_dialog (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *dialog = GTK_WIDGET( gtk_builder_get_object (builder, "dictionary_install_dialog" ));
    GtkWidget *settings_window = GTK_WIDGET(gtk_builder_get_object (builder, "main_window" ));
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (settings_window));
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_widget_show (GTK_WIDGET (dialog));
}


G_MODULE_EXPORT void do_start_dictionary_install (GtkWidget *widget, gpointer data)
{
    printf("start dictionary install\n");
}

G_MODULE_EXPORT void do_toggle_other_dictionary_show (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *checkbox = GTK_WIDGET (data);
    GtkWidget *table = GTK_WIDGET (widget);
    GtkWidget *dialog = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_dialog"));

    gboolean active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbox));
    if (active) gtk_widget_show (table);
    else gtk_widget_hide (table);
}


