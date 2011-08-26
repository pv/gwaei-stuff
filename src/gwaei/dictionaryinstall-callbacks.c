#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>

static void _dictinstwindow_clear_details_box (GwDictInstWindow*);
static void _dictinstwindow_fill_details_box (GwDictInstWindow*, LwDictInst*);
static void _dictinstwindow_update_add_button_sensitivity (GwDictInstWindow*);


/*

G_MODULE_EXPORT void do_toggle_other_dictionary_show (GtkWidget *widget, gpointer data)
{
    GtkWidget *checkbox = GTK_WIDGET (data);
    GtkWidget *table = GTK_WIDGET (widget);
    GtkWidget *dialog = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_dialog"));

    gboolean active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbox));
    if (active) gtk_widget_show (table);
    else gtk_widget_hide (table);
}
*/


//CALLBACKS//////////////////////////////////////

G_MODULE_EXPORT void gw_dictionaryinstallwindow_filename_entry_changed_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictInstWindow *window;
    const char *value;

    //Initializations
    window = GW_DICTINSTWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    value = gtk_entry_get_text (GTK_ENTRY (widget));

    lw_dictinst_set_filename (window->di, value);

    _dictinstwindow_update_add_button_sensitivity (window);
}


G_MODULE_EXPORT void gw_dictionaryinstallwindow_engine_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictInstWindow *window;
    int value;

    //Initializations
    window = GW_DICTINSTWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    value = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    window->di->type = value;
}

G_MODULE_EXPORT void gw_dictionaryinstallwindow_source_entry_changed_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictInstWindow *window;
    const char *value = gtk_entry_get_text (GTK_ENTRY (widget));

    //Initializations
    window = GW_DICTINSTWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    value = gtk_entry_get_text (GTK_ENTRY (widget));

    //Set the LwDictInst value
    lw_dictinst_set_download_source (window->di, value);

    //Update the preference if approprate
    if (window->di->schema != NULL && window->di->key != NULL)
    {
      lw_prefmanager_set_string_by_schema (app->prefmanager, window->di->schema, window->di->key, value);
    }

    _dictinstwindow_update_add_button_sensitivity (window);
}


G_MODULE_EXPORT void gw_dictionaryinstallwindow_reset_default_uri_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictInstWindow *window;

    //Initializations
    window = GW_DICTINSTWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));

    if (window->di->schema == NULL || window->di->key == NULL) return;

    lw_prefmanager_reset_value_by_schema (app->prefmanager, window->di->schema, window->di->key);
}


G_MODULE_EXPORT void gw_dictionaryinstallwindow_select_file_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictInstWindow *window;
    GtkWidget *dialog;
    char *filename;

    //Initializations
    window = GW_DICTINSTWINDOW (data);
    dialog = gtk_file_chooser_dialog_new (
      "Select File",
      GTK_WINDOW (window->toplevel),
      GTK_FILE_CHOOSER_ACTION_OPEN,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
      NULL
    );
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      if (filename != NULL)
      {
        gtk_entry_set_text (window->source_entry, filename);
        g_free (filename);
      }
    }

    gtk_widget_destroy (dialog);
}


G_MODULE_EXPORT void gw_dictionaryinstallwindow_encoding_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declaraitons
    GwDictInstWindow *window;
    int value;

    //Initializations
    window = GW_DICTINSTWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    value = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    lw_dictinst_set_encoding (window->di, value);
}


G_MODULE_EXPORT void gw_dictionaryinstallwindow_compression_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictInstWindow *window;
    int value;

    //Initializations
    window = GW_DICTINSTWINDOW (data);
    value = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    lw_dictinst_set_compression (window->di, value);
}


G_MODULE_EXPORT void gw_dictionaryinstallwindow_split_checkbox_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictInstWindow *window;
    gboolean value;

    //Initializations
    window = GW_DICTINSTWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    lw_dictinst_set_split (window->di, value);
}


G_MODULE_EXPORT void gw_dictionaryinstallwindow_merge_checkbox_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictInstWindow *window;
    gboolean value;

    //Initializations
    window = GW_DICTINSTWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    value = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (widget));

    lw_dictinst_set_merge (window->di, value);
}


G_MODULE_EXPORT void gw_dictionaryinstallwindow_cursor_changed_cb (GtkTreeView *view, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictInstWindow *window;
    GtkTreeSelection *selection;
    GtkWidget *hbox;
    GtkWidget *dialog;
    LwDictInst *di;
    GtkTreeIter iter;
    gboolean show_details;
    gboolean has_selection;
    GtkTreeModel *model;
    gboolean editable;
    int height;
    int width;

    //Initializations
    window = GW_DICTINSTWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    hbox = GTK_WIDGET (gtk_builder_get_object (window->builder, "dictionary_install_details_hbox"));
    dialog = GTK_WIDGET (gtk_builder_get_object (window->builder, "dictionary_install_dialog"));

    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
    model = GTK_TREE_MODEL (window->dictionary_store);
    has_selection = gtk_tree_selection_get_selected (selection, &model, &iter);
    show_details = gtk_toggle_button_get_active (window->details_togglebutton);
    editable = FALSE;

    //Set the approprate contents of the detail pane
    if (has_selection)
    {
      _dictinstwindow_clear_details_box (window);
      gtk_tree_model_get (model, &iter, GW_DICTINSTWINDOW_DICTSTOREFIELD_DICTINST_PTR, &di, -1);
      _dictinstwindow_fill_details_box (window, di);
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


G_MODULE_EXPORT void gw_dictionaryinstallwindow_listitem_toggled_cb (GtkCellRendererToggle *renderer, 
                                                                     gchar *path,
                                                                     gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictInstWindow *window;
    GtkTreeIter iter;
    gboolean state;
    LwDictInst *di;

    //Initializations
    window = GW_DICTINSTWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (window->dictionary_store), &iter, path);
    gtk_tree_model_get (GTK_TREE_MODEL (window->dictionary_store), &iter, GW_DICTINSTWINDOW_DICTSTOREFIELD_CHECKBOX_STATE, &state, GW_DICTINSTWINDOW_DICTSTOREFIELD_DICTINST_PTR, &di, -1);
    gtk_list_store_set (GTK_LIST_STORE (window->dictionary_store), &iter, GW_DICTINSTWINDOW_DICTSTOREFIELD_CHECKBOX_STATE, !state, -1);
    di->selected = !state;

    _dictinstwindow_update_add_button_sensitivity (window);
}


G_MODULE_EXPORT void gw_dictionaryinstallwindow_detail_checkbox_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwDictInstWindow *window;
    GtkTreePath *path;
    GtkTreeViewColumn *column;

    //Declarations
    window = GW_DICTINSTWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));

    //Trigger the list item selection callback
    gtk_tree_view_get_cursor (window->view, &path, &column);
    gtk_tree_view_set_cursor (window->view, path, column, FALSE);

    //Cleanup
    gtk_tree_path_free (path);
}


//!
//! @brief Checks the validity of the LwDictInst data and sets the add button sensitivity accordingly
//!
static void _dictinstwindow_update_add_button_sensitivity (GwDictInstWindow *window)
{
    //Declarations
    GwSettingsWindow *settingswindow;
    gboolean sensitivity;

    //Initializations
    settingswindow = GW_SETTINGSWINDOW (window->transient_for);
    sensitivity = lw_dictinstlist_data_is_valid (settingswindow->dictinstlist);

    //Finalize
    gtk_widget_set_sensitive (GTK_WIDGET (window->add_button), sensitivity);
}


static void _dictinstwindow_clear_details_box (GwDictInstWindow *window)
{
    //Declarations
    GtkWidget *hbox;
    GList *children;
    GList *iter;

    //Initializations
    hbox = GTK_WIDGET (gtk_builder_get_object (window->builder, "dictionary_install_details_hbox"));
    children = gtk_container_get_children (GTK_CONTAINER (hbox));

    //Set the volatile widget pointers to null
    window->filename_entry = NULL;
    window->engine_combobox = NULL;
    window->source_entry = NULL;
    window->source_choose_button = NULL;
    window->source_reset_button = NULL;
    window->encoding_combobox = NULL;
    window->compression_combobox = NULL;
    window->split_checkbutton = NULL;
    window->merge_checkbutton = NULL;

    //Clear the GUI elements
    for (iter = children; iter != NULL; iter = iter->next)
    {
      gtk_widget_destroy (GTK_WIDGET (iter->data));
    }
    g_list_free (children);
    children = NULL;

    window->di = NULL;
}


static void _dictinstwindow_fill_details_box (GwDictInstWindow *window, LwDictInst *di)
{
    //Declarations
    GtkWidget *parent;
    GtkWidget *button;
    GtkWidget *image;
    GtkWidget *entry;
    GtkWidget *label;
    GtkWidget *table;
    GtkWidget *hbox;
    GtkWidget *combobox;
    GtkWidget *checkbox;
    GtkCellRenderer *renderer;
    gchar *markup;
    gboolean editable;

    //Initializations
    parent = GTK_WIDGET (gtk_builder_get_object (window->builder, "dictionary_install_details_hbox"));
    table = gtk_table_new (7, 2, FALSE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 1);
    gtk_table_set_col_spacings (GTK_TABLE (table), 0);
    editable = !di->builtin;
    window->di = di;

    //First row
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    markup = g_strdup_printf(gettext("<b>%s Install Details</b>"), di->longname);
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), markup);
    if (markup != NULL) g_free (markup);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_box_pack_start (GTK_BOX (parent), GTK_WIDGET (hbox), FALSE, FALSE, 0);
    gtk_widget_show_all (GTK_WIDGET (hbox));

    //Second row
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    markup = g_strdup_printf("%s", di->description);
    label = gtk_label_new (NULL);
    gtk_widget_set_size_request (GTK_WIDGET (label), 300, -1);
    gtk_misc_set_alignment (GTK_MISC (label), 0, 0);
    gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
    gtk_label_set_markup(GTK_LABEL (label), markup);
    if (markup != NULL) g_free (markup);
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
    g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (gw_dictionaryinstallwindow_filename_entry_changed_cb), window->toplevel);
    gtk_table_attach_defaults (GTK_TABLE (table), entry, 1, 2, 0, 1);
    gtk_widget_set_sensitive (GTK_WIDGET (entry), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);
    window->filename_entry = GTK_ENTRY (entry);

    //Forth row
    label = gtk_label_new (gettext("Engine: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 1, 2);

    combobox = gtk_combo_box_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", GW_DICTINSTWINDOW_ENGINESTOREFIELD_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (window->engine_store));

    gtk_table_attach_defaults (GTK_TABLE (table), combobox, 1, 2, 1, 2);
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->type);
    g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (gw_dictionaryinstallwindow_engine_combobox_changed_cb), window->toplevel);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);
    window->engine_combobox = GTK_COMBO_BOX (combobox);


    //Fifth row
    label = gtk_label_new (gettext("Source: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 2, 3);

    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));

    entry = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (entry), di->uri[LW_DICTINST_NEEDS_DOWNLOADING]);
    g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (gw_dictionaryinstallwindow_source_entry_changed_cb), window->toplevel);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (entry), TRUE, TRUE, 0);
    window->source_entry = GTK_ENTRY (entry);

    button = gtk_button_new();
    image = gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    gtk_container_add (GTK_CONTAINER (button), image);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (button), FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gw_dictionaryinstallwindow_select_file_cb), window->toplevel);
    window->source_choose_button = GTK_BUTTON (button);

    button = gtk_button_new();
    image = gtk_image_new_from_stock (GTK_STOCK_UNDO, GTK_ICON_SIZE_MENU);
    gtk_container_add (GTK_CONTAINER (button), image);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (button), FALSE, FALSE, 0);
    gtk_widget_set_sensitive (GTK_WIDGET (button), !editable);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 1, 2, 2, 3);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gw_dictionaryinstallwindow_reset_default_uri_cb), window->toplevel);
    window->source_reset_button = GTK_BUTTON (button);

    //Sixth row
    label = gtk_label_new (gettext("Encoding: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 3, 4);

    combobox = gtk_combo_box_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", GW_DICTINSTWINDOW_ENCODINGSTOREFIELD_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (window->encoding_store));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->encoding);
    g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (gw_dictionaryinstallwindow_encoding_combobox_changed_cb), window->toplevel);
    window->encoding_combobox = GTK_COMBO_BOX (combobox);

    gtk_table_attach_defaults (GTK_TABLE (table), combobox, 1, 2, 3, 4);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    //Seventh row
    label = gtk_label_new (gettext("Compression: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 4, 5);

    combobox = gtk_combo_box_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", GW_DICTINSTWINDOW_COMPRESSIONSTOREFIELD_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (window->compression_store));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->compression);
    g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (gw_dictionaryinstallwindow_compression_combobox_changed_cb), window->toplevel);
    window->compression_combobox = GTK_COMBO_BOX (combobox);

    gtk_table_attach_defaults (GTK_TABLE (table), combobox, 1, 2, 4, 5);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    //Eighth row
    checkbox = gtk_check_button_new_with_label (gettext("Split Places from Names Dictionary"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), di->split);
    g_signal_connect (G_OBJECT (checkbox), "toggled", G_CALLBACK (gw_dictionaryinstallwindow_split_checkbox_toggled_cb), window->toplevel);
    window->split_checkbutton;

    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (checkbox), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 2, 5, 6);
    gtk_widget_set_sensitive (GTK_WIDGET (checkbox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    //Ninth row
    checkbox = gtk_check_button_new_with_label (gettext("Merge Radicals into Kanji Dictionary"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), di->merge);
    g_signal_connect (G_OBJECT (checkbox), "toggled", G_CALLBACK (gw_dictionaryinstallwindow_merge_checkbox_toggled_cb), window->toplevel);
    window->merge_checkbutton;

    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (checkbox), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 2, 6, 7);
    gtk_widget_set_sensitive (GTK_WIDGET (checkbox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    gtk_box_pack_start (GTK_BOX (parent), GTK_WIDGET (table), FALSE, FALSE, 5);
    gtk_widget_show_all (GTK_WIDGET (table));
}


void gw_dictionaryinstallwindow_cancel_cb (GtkWidget *widget, gpointer data)
{
    GwDictInstWindow *window;
    GwSettingsWindow *settingswindow;

    window = GW_DICTINSTWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    settingswindow = GW_SETTINGSWINDOW (window->transient_for);

    if (settingswindow->dictinstlist != NULL)
    {
      lw_dictinstlist_free (settingswindow->dictinstlist);
      settingswindow->dictinstlist = NULL;
    }
}


//!
//! @brief Closes the window passed throught the widget pointer
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_dictionaryinstallwindow_close_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwDictInstWindow *window;
    
    //Initializations
    window = GW_DICTINSTWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));

    gw_app_destroy_window (app, GW_WINDOW (window));
}


