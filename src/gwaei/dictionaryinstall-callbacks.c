#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


//!
//! @brief opens the dictionary install dialog
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_dictionaryinstall_show_cb (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *dialog = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_dialog"));
    GtkWidget *settings_window = GTK_WIDGET (gtk_builder_get_object (builder, "settings_window" ));
    gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (settings_window));
    gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_widget_show (GTK_WIDGET (dialog));
}

/*

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
*/


//CALLBACKS//////////////////////////////////////

G_MODULE_EXPORT void gw_dictionaryinstall_filename_entry_changed_cb (GtkWidget *widget, gpointer data)
{
    g_assert (data != NULL);

    LwDictInst *di = (LwDictInst*) data;
    const char *value = gtk_entry_get_text (GTK_ENTRY (widget));

    lw_dictinst_set_filename (di, value);

    _update_add_button_sensitivity ();
}

G_MODULE_EXPORT void gw_dictionaryinstall_engine_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    g_assert (data != NULL);

    LwDictInst *di = (LwDictInst*) data;
    int value = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    di->engine = value;
}

G_MODULE_EXPORT void gw_dictionaryinstall_source_entry_changed_cb (GtkWidget *widget, gpointer data)
{
    g_assert (data != NULL);

    LwDictInst *di = (LwDictInst*) data;
    const char *value = gtk_entry_get_text (GTK_ENTRY (widget));

    //Set the LwDictInst value
    lw_dictinst_set_download_source (di, value);

    //Update the preference if approprate
    if (di->schema != NULL && di->key != NULL)
    {
      lw_pref_set_string_by_schema (di->schema, di->key, value);
    }

    _update_add_button_sensitivity ();
}


G_MODULE_EXPORT void gw_dictionaryinstall_reset_default_uri_cb (GtkWidget *widget, gpointer data)
{
    LwDictInst* di = (LwDictInst*) data;

    if (di->schema == NULL || di->key == NULL) return;

    lw_pref_reset_value_by_schema (di->schema, di->key);
}


G_MODULE_EXPORT void gw_dictionaryinstall_select_file_cb (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder;
    GtkWidget *window;
    GtkWidget *dialog;
    GtkWidget *entry;

    builder = gw_common_get_builder ();
    entry = GTK_WIDGET (data);
    window = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_dialog"));
    dialog = gtk_file_chooser_dialog_new (
      "Select File",
      GTK_WINDOW (window),
      GTK_FILE_CHOOSER_ACTION_OPEN,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
      NULL
    );
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      char *filename;
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      gtk_entry_set_text (GTK_ENTRY (entry), filename);
      g_free (filename);
    }
    gtk_widget_hide (dialog);
}


G_MODULE_EXPORT void gw_dictionaryinstall_encoding_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    g_assert (data != NULL);

    LwDictInst *di = (LwDictInst*) data;
    int value = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    lw_dictinst_set_encoding (di, value);
}


G_MODULE_EXPORT void gw_dictionaryinstall_compression_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    g_assert (data != NULL);

    LwDictInst *di = (LwDictInst*) data;
    int value = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    lw_dictinst_set_compression (di, value);
}


G_MODULE_EXPORT void gw_dictionaryinstall_split_checkbox_changed_cb (GtkWidget *widget, gpointer data)
{
    g_assert (data != NULL);

    LwDictInst *di = (LwDictInst*) data;
    gboolean value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    lw_dictinst_set_split (di, value);
}


G_MODULE_EXPORT void gw_dictionaryinstall_merge_checkbox_changed_cb (GtkWidget *widget, gpointer data)
{
    g_assert (data != NULL);

    LwDictInst *di = (LwDictInst*) data;
    gboolean value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    lw_dictinst_set_merge (di, value);
}


G_MODULE_EXPORT void gw_dictionaryinstall_cursor_changed_cb (GtkTreeView *view, gpointer data)
{
    //Declarations
    GtkTreeSelection *selection;
    GtkWidget *checkbox;
    GtkWidget *hbox;
    GtkWidget *dialog;
    LwDictInst *di;
    GtkTreeIter iter;
    gboolean show_details;
    gboolean has_selection;
    GtkTreeModel *model;
    GtkBuilder *builder;
    gboolean editable;
    int height;
    int width;

    //Initializations
    builder = gw_common_get_builder ();
    checkbox = GTK_WIDGET (gtk_builder_get_object (builder, "show_dictionary_detail_checkbutton"));
    hbox = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_details_hbox"));
    dialog = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_dialog"));

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
    model = GTK_TREE_MODEL (_dictionary_store);
    has_selection = gtk_tree_selection_get_selected (selection, &model, &iter);
    show_details = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbox));
    editable = FALSE;

    //Set the approprate contents of the detail pane
    if (has_selection)
    {
      _clear_details_box();
      gtk_tree_model_get (model, &iter, DICTINST_PTR, &di, -1);
      _fill_details_box (di);
      editable = !di->builtin;
    }

    //Set the approprate show/hide state of the detail pane
    if (has_selection && (show_details || editable))
      gtk_widget_show (hbox);
    else
      gtk_widget_hide (hbox);

    //Make the window shrink if the detail pane disappeared
    gtk_window_get_size (GTK_WINDOW (dialog), &width, &height);
    gtk_window_resize (GTK_WINDOW (dialog), 1, height);
}


G_MODULE_EXPORT void gw_dictionaryinstall_listitem_toggled_cb (GtkCellRendererToggle *renderer, 
                                                               gchar *path,
                                                               gpointer data)
{
    GtkTreeIter iter;
    gboolean state;
    LwDictInst *di;
    gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (_dictionary_store), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL (_dictionary_store), &iter, CHECKBOX_STATE, &state, DICTINST_PTR, &di, -1);
    gtk_list_store_set (GTK_LIST_STORE (_dictionary_store), &iter, CHECKBOX_STATE, !state, -1);
    di->selected = !state;

    _update_add_button_sensitivity ();
}


G_MODULE_EXPORT void gw_dictionaryinstall_detail_checkbox_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkTreePath *path;
    GtkTreeViewColumn *column;
    GtkBuilder *builder;
    GtkTreeView *view;

    //Declarations
    builder = gw_common_get_builder ();
    view = GTK_TREE_VIEW (gtk_builder_get_object (builder, "dictionary_install_treeview"));

    //Trigger the list item selection callback
    gtk_tree_view_get_cursor (view, &path, &column);
    gtk_tree_view_set_cursor (view, path, column, FALSE);

    //Cleanup
    gtk_tree_path_free (path);
}


//!
//! @brief Checks the validity of the LwDictInst data and sets the add button sensitivity accordingly
//!
static void _update_add_button_sensitivity ()
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *button;
    gboolean sensitivity;

    //Initializations
    builder = gw_common_get_builder ();
    button = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_add_button"));
    sensitivity = lw_dictinstlist_data_is_valid ();

    //Finalize
    gtk_widget_set_sensitive (button, sensitivity);
}

