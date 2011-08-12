#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>

enum { SHORT_NAME, LONG_NAME, DICTINST_PTR, CHECKBOX_STATE, DICTIONARY_STORE_TOTAL_FIELDS };
enum { ENGINE_STORE_ID, ENGINE_STORE_NAME, ENGINE_STORE_TOTAL_FIELDS };
enum { COMPRESSION_STORE_ID, COMPRESSION_STORE_NAME, COMPRESSION_STORE_TOTAL_FIELDS };
enum { ENCODING_STORE_ID, ENCODING_STORE_NAME, ENCODING_STORE_TOTAL_FIELDS };


static void _update_add_button_sensitivity (void);

static void _clear_details_box (GwDictInstWindow *window)
{
    GtkWidget *hbox;
    GList *children;
    GList *iter;

    hbox = GTK_WIDGET (gtk_builder_get_object (window->builder, "dictionary_install_details_hbox"));
    children = gtk_container_get_children (GTK_CONTAINER (hbox));

    //Clear the GUI elements
    for (iter = children; iter != NULL; iter = iter->next)
    {
      gtk_widget_destroy (GTK_WIDGET (iter->data));
    }
    g_list_free (children);
    children = NULL;
}


static void _fill_details_box (GwDictInstWindow *window, LwDictInst *di)
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
    gboolean editable;

    table = gtk_table_new (7, 2, FALSE);
    gtk_table_set_row_spacings (GTK_TABLE (table), 1);
    gtk_table_set_col_spacings (GTK_TABLE (table), 0);
    editable = !di->builtin;

    //First row
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    markup = g_strdup_printf(gettext("<b>%s Install Details</b>"), di->longname);
    label = gtk_label_new (NULL);
    gtk_label_set_markup (GTK_LABEL (label), markup);
    g_free (markup);
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
    g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (gw_dictionaryinstall_filename_entry_changed_cb), di);
    gtk_table_attach_defaults (GTK_TABLE (table), entry, 1, 2, 0, 1);
    gtk_widget_set_sensitive (GTK_WIDGET (entry), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    //Forth row
    label = gtk_label_new (gettext("Engine: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 1, 2);
    combobox = gtk_combo_box_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", ENGINE_STORE_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (temp->engine_store));

    gtk_table_attach_defaults (GTK_TABLE (table), combobox, 1, 2, 1, 2);
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->engine);
    g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (gw_dictionaryinstall_engine_combobox_changed_cb), di);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);


    //Fifth row
    label = gtk_label_new (gettext("Source: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 2, 3);

    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    entry = gtk_entry_new ();
    gtk_entry_set_text (GTK_ENTRY (entry), di->uri[LW_DICTINST_NEEDS_DOWNLOADING]);
    g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (gw_dictionaryinstall_source_entry_changed_cb), di);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (entry), TRUE, TRUE, 0);
    button = gtk_button_new();
    image = gtk_image_new_from_stock (GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU);
    gtk_container_add (GTK_CONTAINER (button), image);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (button), FALSE, FALSE, 0);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gw_dictionaryinstall_select_file_cb), entry);
    button = gtk_button_new();
    image = gtk_image_new_from_stock (GTK_STOCK_UNDO, GTK_ICON_SIZE_MENU);
    gtk_container_add (GTK_CONTAINER (button), image);
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (button), FALSE, FALSE, 0);
    gtk_widget_set_sensitive (GTK_WIDGET (button), !editable);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 1, 2, 2, 3);
    g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gw_dictionaryinstall_reset_default_uri_cb), di);

    //Sixth row
    label = gtk_label_new (gettext("Encoding: "));
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 1, 3, 4);
    combobox = gtk_combo_box_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, TRUE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", ENCODING_STORE_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (temp->encoding_store));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->encoding);
    g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (gw_dictionaryinstall_encoding_combobox_changed_cb), di);

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
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", COMPRESSION_STORE_NAME);
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (temp->compression_store));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), di->compression);
    g_signal_connect (G_OBJECT (combobox), "changed", G_CALLBACK (gw_dictionaryinstall_compression_combobox_changed_cb), di);

    gtk_table_attach_defaults (GTK_TABLE (table), combobox, 1, 2, 4, 5);
    gtk_widget_set_sensitive (GTK_WIDGET (combobox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    //Eighth row
    checkbox = gtk_check_button_new_with_label (gettext("Split Places from Names Dictionary"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), di->split);
    g_signal_connect (G_OBJECT (checkbox), "toggled", G_CALLBACK (gw_dictionaryinstall_split_checkbox_changed_cb), di);
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (checkbox), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 2, 5, 6);
    gtk_widget_set_sensitive (GTK_WIDGET (checkbox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    //Ninth row
    checkbox = gtk_check_button_new_with_label (gettext("Merge Radicals into Kanji Dictionary"));
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), di->merge);
    g_signal_connect (G_OBJECT (checkbox), "toggled", G_CALLBACK (gw_dictionaryinstall_merge_checkbox_changed_cb), di);
    hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 0));
    gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (checkbox), FALSE, FALSE, 0);
    gtk_table_attach_defaults (GTK_TABLE (table), hbox, 0, 2, 6, 7);
    gtk_widget_set_sensitive (GTK_WIDGET (checkbox), editable);
    gtk_widget_set_sensitive (GTK_WIDGET (label), editable);

    gtk_box_pack_start (GTK_BOX (parent), GTK_WIDGET (table), FALSE, FALSE, 5);
    gtk_widget_show_all (GTK_WIDGET (table));

    temp->di = di;
}


GwDictInstWindow* gw_dictinstwindow_new ()
{
    GwDictInstWindow *temp;

    temp = (GwDictInstWindow*) malloc(sizeof(GwDictInstWindow));
    if (temp != NULL)
    {
      temp->encoding_store = NULL;
      temp->compression_store = NULL;
      temp->engine_store = NULL;
      temp->dictionary_store = NULL;
      temp->di = NULL;

      gw_common_load_ui_xml ("dictionaryinstall.ui");

      GtkBuilder *builder = gw_common_get_builder ();
      GtkCellRenderer *renderer;
      GtkTreeViewColumn *column;
      GtkTreeView *view;
      GList *list;
      LwDictInst *di;
      GtkTreeIter treeiter;
      int i;

      //Set up the dictionary liststore
      window->dictionary_store = gtk_list_store_new (DICTIONARY_STORE_TOTAL_FIELDS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT);
      for (list = lw_dictinstlist_get_list (); list != NULL; list = list->next)
      {
        di = (LwDictInst*) list->data;
        gtk_list_store_append (GTK_LIST_STORE (window->dictionary_store), &treeiter);
        gtk_list_store_set (
          window->dictionary_store, &treeiter,
          SHORT_NAME, di->shortname,
          LONG_NAME, di->longname,
          DICTINST_PTR, di,
          CHECKBOX_STATE, FALSE, 
          -1
        );
      }

      //Set up the Engine liststore
      window->engine_store = gtk_list_store_new (ENGINE_STORE_TOTAL_FIELDS, G_TYPE_INT, G_TYPE_STRING);
      for (i = 0; i < LW_ENGINE_TOTAL; i++)
      {
        gtk_list_store_append (GTK_LIST_STORE (window->engine_store), &treeiter);
        gtk_list_store_set (
          window->engine_store, &treeiter,
          ENGINE_STORE_ID, i,
          ENGINE_STORE_NAME, lw_util_get_engine_name(i),
          -1
        );
      }

      //Set up the Compression liststore
      window->compression_store = gtk_list_store_new (COMPRESSION_STORE_TOTAL_FIELDS, G_TYPE_INT, G_TYPE_STRING);
      for (i = 0; i < LW_COMPRESSION_TOTAL; i++)
      {
        gtk_list_store_append (GTK_LIST_STORE (window->compression_store), &treeiter);
        gtk_list_store_set (
          window->compression_store, &treeiter,
          COMPRESSION_STORE_ID, i,
          COMPRESSION_STORE_NAME, lw_util_get_compression_name(i),
          -1
        );
      }

      //Set up the Encoding liststore
      window->encoding_store = gtk_list_store_new (ENCODING_STORE_TOTAL_FIELDS, G_TYPE_INT, G_TYPE_STRING);
      for (i = 0; i < LW_ENCODING_TOTAL; i++)
      {
        gtk_list_store_append (GTK_LIST_STORE (window->encoding_store), &treeiter);
        gtk_list_store_set (
          window->encoding_store, &treeiter,
          ENCODING_STORE_ID, i,
          ENCODING_STORE_NAME, lw_util_get_encoding_name(i),
          -1
        );
      }

      //Setup the dictionary list view
      view = GTK_TREE_VIEW (gtk_builder_get_object (builder, "dictionary_install_treeview"));
      gtk_tree_view_set_model (GTK_TREE_VIEW (view), GTK_TREE_MODEL (window->dictionary_store));
      g_signal_connect (G_OBJECT (view), "cursor-changed", G_CALLBACK (gw_dictionaryinstall_cursor_changed_cb), NULL);

      renderer = gtk_cell_renderer_toggle_new ();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 5);
      column = gtk_tree_view_column_new_with_attributes (" ", renderer, "active", CHECKBOX_STATE, NULL);
      gtk_tree_view_append_column (view, column);
      g_signal_connect (G_OBJECT (renderer), "toggled", G_CALLBACK (gw_dictionaryinstall_listitem_toggled_cb), NULL);

      renderer = gtk_cell_renderer_text_new ();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 0);
      column = gtk_tree_view_column_new_with_attributes ("Name", renderer, "text", LONG_NAME, NULL);
      gtk_tree_view_append_column (view, column);
    }

    return temp;
}

void gw_dictinstwindow_destroy (GwDictInstWindow *window)
{
    g_object_unref (window->encoding_store);
    g_object_unref (window->compression_store);
    g_object_unref (window->engine_store);
    g_object_unref (window->dictionary_store);
}


