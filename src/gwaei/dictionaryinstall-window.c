#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


GwDictInstWindow* gw_dictinstwindow_new ()
{
    GwDictInstWindow *temp;

    temp = (GwDictInstWindow*) malloc(sizeof(GwDictInstWindow));
    if (temp != NULL)
    {
      gw_window_init (GW_WINDOW (temp), GW_WINDOW_DICTIONARYINSTALL, "dictionaryinstall.ui", "dictionary_install_dialog");
      gw_dictinstwindow_init (temp);
    }

    return temp;
}


void gw_dictinstwindow_destroy (GwDictInstWindow *window)
{
  gw_dictinstwindow_deinit (window);
  gw_window_deinit (GW_WINDOW (window));
}


void gw_dictinstwindow_init (GwDictInstWindow *window)
{
    //Declarations
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GList *iter;
    LwDictInst *di;
    GtkTreeIter treeiter;
    int i;

    //Initializations
    window->di = NULL;
    window->dictinstlist = lw_dictinstlist_new (app->prefmanager);
    window->dictionary_store = gtk_list_store_new (TOTAL_GW_DICTINSTWINDOW_DICTSTOREFIELDS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT);

    window->view = GTK_TREE_VIEW (gtk_builder_get_object (window->builder, "dictionary_install_treeview"));
    window->add_button = GTK_BUTTON (gtk_builder_get_object (window->builder, "dictionary_install_add_button"));
    window->details_togglebutton = GTK_TOGGLE_BUTTON (gtk_builder_get_object (window->builder, "show_dictionary_detail_checkbutton"));

    //Set up the dictionary liststore
    for (iter = window->dictinstlist->list; iter != NULL; iter = iter->next)
    {
      di = LW_DICTINST (iter->data);
      gtk_list_store_append (GTK_LIST_STORE (window->dictionary_store), &treeiter);
      gtk_list_store_set (
        window->dictionary_store, &treeiter,
        GW_DICTINSTWINDOW_DICTSTOREFIELD_SHORT_NAME, di->shortname,
        GW_DICTINSTWINDOW_DICTSTOREFIELD_LONG_NAME, di->longname,
        GW_DICTINSTWINDOW_DICTSTOREFIELD_DICTINST_PTR, di,
        GW_DICTINSTWINDOW_DICTSTOREFIELD_CHECKBOX_STATE, FALSE, 
        -1
      );
    }

    //Set up the Engine liststore
    window->engine_store = gtk_list_store_new (TOTAL_GW_DICTINSTWINDOW_ENGINESTOREFIELDS, G_TYPE_INT, G_TYPE_STRING);
    for (i = 0; i < TOTAL_LW_DICTTYPES; i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (window->engine_store), &treeiter);
      gtk_list_store_set (
        window->engine_store, &treeiter,
        GW_DICTINSTWINDOW_ENGINESTOREFIELD_ID, i,
        GW_DICTINSTWINDOW_ENGINESTOREFIELD_NAME, lw_util_dicttype_to_string (i),
        -1
      );
    }

    //Set up the Compression liststore
    window->compression_store = gtk_list_store_new (TOTAL_GW_DICTINSTWINDOW_COMPRESSIONSTOREFIELDS, G_TYPE_INT, G_TYPE_STRING);
    for (i = 0; i < LW_COMPRESSION_TOTAL; i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (window->compression_store), &treeiter);
      gtk_list_store_set (
        window->compression_store, &treeiter,
        GW_DICTINSTWINDOW_COMPRESSIONSTOREFIELD_ID, i,
        GW_DICTINSTWINDOW_COMPRESSIONSTOREFIELD_NAME, lw_util_get_compression_name (i),
        -1
      );
    }

    //Set up the Encoding liststore
    window->encoding_store = gtk_list_store_new (TOTAL_GW_DICTINSTWINDOW_ENCODINGSTOREFIELDS, G_TYPE_INT, G_TYPE_STRING);
    for (i = 0; i < LW_ENCODING_TOTAL; i++)
    {
      gtk_list_store_append (GTK_LIST_STORE (window->encoding_store), &treeiter);
      gtk_list_store_set (
        window->encoding_store, &treeiter,
        GW_DICTINSTWINDOW_ENCODINGSTOREFIELD_ID, i,
        GW_DICTINSTWINDOW_ENCODINGSTOREFIELD_NAME, lw_util_get_encoding_name(i),
        -1
      );
    }

    //Setup the dictionary list view
    gtk_tree_view_set_model (window->view, GTK_TREE_MODEL (window->dictionary_store));
    g_signal_connect (G_OBJECT (window->view), "cursor-changed", G_CALLBACK (gw_dictionaryinstall_cursor_changed_cb), NULL);

    renderer = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 5);
    column = gtk_tree_view_column_new_with_attributes (" ", renderer, "active", GW_DICTINSTWINDOW_DICTSTOREFIELD_CHECKBOX_STATE, NULL);
    gtk_tree_view_append_column (window->view, column);
    g_signal_connect (G_OBJECT (renderer), "toggled", G_CALLBACK (gw_dictionaryinstall_listitem_toggled_cb), NULL);

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 0);
    column = gtk_tree_view_column_new_with_attributes ("Name", renderer, "text", GW_DICTINSTWINDOW_DICTSTOREFIELD_LONG_NAME, NULL);
    gtk_tree_view_append_column (window->view, column);
}


void gw_dictinstwindow_deinit (GwDictInstWindow *window)
{
    g_object_unref (window->encoding_store);
    g_object_unref (window->compression_store);
    g_object_unref (window->engine_store);
    g_object_unref (window->dictionary_store);
}

