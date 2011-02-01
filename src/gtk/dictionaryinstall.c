#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>


static GtkListStore *_encoding_store = NULL;
static GtkListStore *_compression_store = NULL;
static GtkListStore *_engine_store = NULL;
static GtkListStore *_dictionary_store = NULL;

enum { SHORT_NAME, LONG_NAME, DICTINST_PTR, CHECKBOX_STATE, DICTIONARY_STORE_TOTAL_FIELDS };
enum { ENGINE_STORE_ID, ENGINE_STORE_NAME, ENGINE_STORE_TOTAL_FIELDS };
enum { COMPRESSION_STORE_ID, COMPRESSION_STORE_NAME, COMPRESSION_STORE_TOTAL_FIELDS };
enum { ENCODING_STORE_ID, ENCODING_STORE_NAME, ENCODING_STORE_TOTAL_FIELDS };

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
    gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (_dictionary_store), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL (_dictionary_store), &iter, CHECKBOX_STATE, &state, -1);
    gtk_list_store_set (GTK_LIST_STORE (_dictionary_store), &iter, CHECKBOX_STATE, !state, -1);
}


static void _clear_details_box ()
{
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *hbox = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_details_hbox"));
    GList *children = gtk_container_get_children (GTK_CONTAINER (hbox));
    GList *iter = children;
    while (iter != NULL)
    {
      gtk_widget_destroy (GTK_WIDGET (iter->data));
      iter = iter->next;
    }
    g_list_free (children);
    children = NULL;
}


static void _fill_details_box (GwDictInst *di)
{
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *parent = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_details_hbox"));
    GtkWidget *button = NULL;
    GtkWidget *image = NULL;
    GtkWidget *entry = NULL;
    GtkWidget *label = NULL;
    GtkWidget *table = NULL;
    GtkWidget *hbox = NULL;
    GtkWidget *combobox = NULL;
    GtkWidget *checkbox = NULL;
    GtkCellRenderer *renderer = NULL;
    gchar *markup = NULL;

    table = gtk_table_new (10, 2, FALSE);

    //First row
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    markup = g_strdup_printf(gettext("<b>%s Details</b>"), di->longname);
    label = gtk_label_new (NULL);
    gtk_label_set_markup(GTK_LABEL (label), markup);
    g_free (markup);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (parent), GTK_WIDGET (hbox), FALSE, FALSE, 0);
    gtk_widget_show_all (GTK_WIDGET (hbox));

    //Second row
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    markup = g_strdup_printf(gettext("%s"), di->description);
    label = gtk_label_new (NULL);
    gtk_label_set_markup(GTK_LABEL (label), markup);
    g_free (markup);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (parent), GTK_WIDGET (hbox), FALSE, FALSE, 0);
    gtk_widget_show_all (GTK_WIDGET (hbox));

    //Third row
    label = gtk_label_new (gettext("Filename: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 0, 1);
    entry = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (entry), di->filename);
    gtk_table_attach_defaults (GTK_TABLE (table), entry, 1, 2, 0, 1);
    gtk_widget_set_sensitive (GTK_WIDGET (entry), di->builtin);
    gtk_widget_set_sensitive (GTK_WIDGET (label), di->builtin);

    //Forth row
    label = gtk_label_new (gettext("Engine: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 1, 2);
    combobox = gtk_combo_box_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", ENGINE_STORE_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (_engine_store));

    gtk_table_attach_defaults (GTK_TABLE (table), combobox, 1, 2, 1, 2);
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->engine);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), di->builtin);
    gtk_widget_set_sensitive (GTK_WIDGET (label), di->builtin);


    //Fifth row
    label = gtk_label_new (gettext("Source: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 2, 3);

    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    entry = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (entry), di->uri[GW_DICTINST_DOWNLOAD_SOURCE]);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (entry), FALSE, FALSE, 0);
    button = gtk_button_new();
    image = gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    gtk_container_add (GTK_CONTAINER (button), image);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (button), FALSE, FALSE, 0);
    button = gtk_button_new();
    image = gtk_image_new_from_stock (GTK_STOCK_UNDO, GTK_ICON_SIZE_MENU);
    gtk_container_add (GTK_CONTAINER (button), image);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (button), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 1, 2, 2, 3);

    //Sixth row
    label = gtk_label_new (gettext("Encoding: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 3, 4);
    combobox = gtk_combo_box_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", ENCODING_STORE_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (_encoding_store));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->encoding);

    gtk_table_attach_defaults (GTK_TABLE (table), combobox, 1, 2, 3, 4);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), di->builtin);
    gtk_widget_set_sensitive (GTK_WIDGET (label), di->builtin);

    //Seventh row
    label = gtk_label_new (gettext("Compression: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 4, 5);
    combobox = gtk_combo_box_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", COMPRESSION_STORE_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (_compression_store));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->compression);

    gtk_table_attach_defaults (GTK_TABLE (table), combobox, 1, 2, 4, 5);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), di->builtin);
    gtk_widget_set_sensitive (GTK_WIDGET (label), di->builtin);

    //Eighth row
    checkbox = gtk_check_button_new_with_label (gettext("Split Places from Names Dictionary"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), di->split_dictionary);
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (checkbox), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 2, 5, 6);
    gtk_widget_set_sensitive (GTK_WIDGET (checkbox), di->builtin);
    gtk_widget_set_sensitive (GTK_WIDGET (label), di->builtin);

    //Ninth row
    checkbox = gtk_check_button_new_with_label (gettext("Merge Radicals into Kanji Dictionary"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), di->merge_dictionary);
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (checkbox), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 2, 6, 7);
    gtk_widget_set_sensitive (GTK_WIDGET (checkbox), di->builtin);
    gtk_widget_set_sensitive (GTK_WIDGET (label), di->builtin);

    gtk_box_pack_start (GTK_BOX (parent), GTK_WIDGET (table), FALSE, FALSE, 0);
    gtk_widget_show_all (GTK_WIDGET (table));
}


G_MODULE_EXPORT void _cursor_changed (GtkTreeView *view, gpointer data)
{
    GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
    GwDictInst *di = NULL;
    GtkTreeIter iter;
    GtkTreeModel *model = GTK_TREE_MODEL (_dictionary_store);

    if (!gtk_tree_selection_get_selected (selection, &model, &iter)) return;
    gtk_tree_model_get (model, &iter, DICTINST_PTR, &di, -1);

    _clear_details_box();

    _fill_details_box (di);

    printf("changed\n");
}


void gw_dictionaryinstall_initialize ()
{
    gw_common_load_ui_xml ("dictionaryinstall.ui");

    GtkBuilder *builder = gw_common_get_builder ();
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkTreeView *view;
    GList *list;
    GwDictInst *di;
    GtkTreeIter treeiter;
    int i;

    //Set up the dictionary liststore
    _dictionary_store = gtk_list_store_new (DICTIONARY_STORE_TOTAL_FIELDS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT);
    for (list = gw_dictinstlist_get_list (); list != NULL; list = list->next)
    {
      di = (GwDictInst*) list->data;
      gtk_list_store_append (GTK_LIST_STORE (_dictionary_store), &treeiter);
      gtk_list_store_set (
        _dictionary_store, &treeiter,
        SHORT_NAME, di->shortname,
        LONG_NAME, di->longname,
        DICTINST_PTR, di,
        CHECKBOX_STATE, FALSE, 
        -1
      );
    }

    //Set up the Engine liststore
    _engine_store = gtk_list_store_new (ENGINE_STORE_TOTAL_FIELDS, G_TYPE_INT, G_TYPE_STRING);
    for (i = 0; i < GW_ENGINE_TOTAL; i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (_engine_store), &treeiter);
      gtk_list_store_set (
        _engine_store, &treeiter,
        ENGINE_STORE_ID, i,
        ENGINE_STORE_NAME, gw_util_get_engine_name(i),
        -1
      );
    }

    //Set up the Compression liststore
    _compression_store = gtk_list_store_new (COMPRESSION_STORE_TOTAL_FIELDS, G_TYPE_INT, G_TYPE_STRING);
    for (i = 0; i < GW_COMPRESSION_TOTAL; i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (_compression_store), &treeiter);
      gtk_list_store_set (
        _compression_store, &treeiter,
        COMPRESSION_STORE_ID, i,
        COMPRESSION_STORE_NAME, gw_util_get_compression_name(i),
        -1
      );
    }

    //Set up the Encoding liststore
    _encoding_store = gtk_list_store_new (ENCODING_STORE_TOTAL_FIELDS, G_TYPE_INT, G_TYPE_STRING);
    for (i = 0; i < GW_ENCODING_TOTAL; i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (_encoding_store), &treeiter);
      gtk_list_store_set (
        _encoding_store, &treeiter,
        ENCODING_STORE_ID, i,
        ENCODING_STORE_NAME, gw_util_get_encoding_name(i),
        -1
      );
    }

    //Setup the dictionary list view
    view = GTK_TREE_VIEW (gtk_builder_get_object (builder, "dictionary_install_treeview"));
    gtk_tree_view_set_model (GTK_TREE_VIEW (view), GTK_TREE_MODEL (_dictionary_store));
    g_signal_connect (G_OBJECT (view), "cursor-changed", G_CALLBACK (_cursor_changed), NULL);

    renderer = gtk_cell_renderer_toggle_new ();
    column = gtk_tree_view_column_new_with_attributes (" ", renderer, "active", CHECKBOX_STATE, NULL);
    gtk_tree_view_append_column (view, column);
    g_signal_connect (G_OBJECT (renderer), "toggled", G_CALLBACK (_toggled), NULL);

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes ("Name", renderer, "text", LONG_NAME, NULL);
    gtk_tree_view_append_column (view, column);


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


