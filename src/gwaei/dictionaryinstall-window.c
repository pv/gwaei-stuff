/******************************************************************************
    AUTHOR:
    File written and Copyrighted by Zachary Dovel. All Rights Reserved.

    LICENSE:
    This file is part of gWaei.

    gWaei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gWaei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

//!
//! @file dictionaryinstally-window.c
//!
//! @brief To be written
//!

#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


GwDictInstWindow* gw_dictinstwindow_new (GwSettingsWindow *transient_for, GList *link)
{
    GwDictInstWindow *temp;

    temp = (GwDictInstWindow*) malloc(sizeof(GwDictInstWindow));
    if (temp != NULL)
    {
      gw_window_init (GW_WINDOW (temp), GW_WINDOW_DICTIONARYINSTALL, "dictionaryinstall.ui", "dictionary_install_dialog", link);
      gw_dictinstwindow_init (temp, transient_for);
    }

    return temp;
}


void gw_dictinstwindow_destroy (GwDictInstWindow *window)
{
  gw_dictinstwindow_deinit (window);
  gw_window_deinit (GW_WINDOW (window));
  free (window);
}


void gw_dictinstwindow_init (GwDictInstWindow *window, GwSettingsWindow *transient_for)
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
    window->dictionary_store = gtk_list_store_new (TOTAL_GW_DICTINSTWINDOW_DICTSTOREFIELDS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER, G_TYPE_INT);

    window->view = GTK_TREE_VIEW (gtk_builder_get_object (window->builder, "dictionary_install_treeview"));
    window->add_button = GTK_BUTTON (gtk_builder_get_object (window->builder, "dictionary_install_add_button"));
    window->details_togglebutton = GTK_TOGGLE_BUTTON (gtk_builder_get_object (window->builder, "show_dictionary_detail_checkbutton"));

    gw_window_set_transient_for (GW_WINDOW (window), GW_WINDOW (transient_for));

    if (transient_for->dictinstlist != NULL)
    {
      lw_dictinstlist_free (transient_for->dictinstlist);
    }
    transient_for->dictinstlist = lw_dictinstlist_new (app->prefmanager);

    //Set up the dictionary liststore
    for (iter = transient_for->dictinstlist->list; iter != NULL; iter = iter->next)
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

    renderer = gtk_cell_renderer_toggle_new ();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 5);
    column = gtk_tree_view_column_new_with_attributes (" ", renderer, "active", GW_DICTINSTWINDOW_DICTSTOREFIELD_CHECKBOX_STATE, NULL);
    gtk_tree_view_append_column (window->view, column);
    g_signal_connect (G_OBJECT (renderer), "toggled", G_CALLBACK (gw_dictionaryinstallwindow_listitem_toggled_cb), window->toplevel);

    renderer = gtk_cell_renderer_text_new ();
    gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 0);
    column = gtk_tree_view_column_new_with_attributes ("Name", renderer, "text", GW_DICTINSTWINDOW_DICTSTOREFIELD_LONG_NAME, NULL);
    gtk_tree_view_append_column (window->view, column);
}


void gw_dictinstwindow_deinit (GwDictInstWindow *window)
{
    gtk_widget_hide (GTK_WIDGET (window->toplevel));

    g_object_unref (window->encoding_store);
    g_object_unref (window->compression_store);
    g_object_unref (window->engine_store);
    g_object_unref (window->dictionary_store);
}

