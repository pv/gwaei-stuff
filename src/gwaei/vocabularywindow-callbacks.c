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
//! @file vocabularywindow-callbacks.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/vocabularywindow-private.h>


G_MODULE_EXPORT void
gw_vocabularywindow_cell_edited_cb (GtkCellRendererText *renderer,
                                    gchar               *path_string,
                                    gchar               *new_text,
                                    gpointer             data       )
{
}


G_MODULE_EXPORT void
gw_vocabularywindow_list_add_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;

    gw_vocabularywindow_create_new_list (window);
}


G_MODULE_EXPORT void
gw_vocabularywindow_list_remove_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;

    gw_vocabularywindow_remove_selected_lists (window);
}


G_MODULE_EXPORT void
gw_vocabularywindow_item_add_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwVocabularyWindowClass *klass;
    GtkTreeIter iter;
    GtkTreeModel *model;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    klass = G_OBJECT_GET_CLASS (window);
    model = gtk_tree_view_get_model (priv->item_treeview);

    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
        GW_VOCABULARYITEM_COLUMN_KANJI, gettext("(Click to set Kanji)"), 
        GW_VOCABULARYITEM_COLUMN_FURIGANA, gettext("(Click to set Furigana)"),
        GW_VOCABULARYITEM_COLUMN_DEFINITIONS, gettext("(Click to set Definitions)"),
    -1);
}


G_MODULE_EXPORT void
gw_vocabularywindow_item_remove_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GList *rowpathlist;
    GList *rowreflist;
    GList *listiter;
    GtkTreeRowReference *rowref;
    GtkTreePath *path;
    GtkTreeIter treeiter;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    model = gtk_tree_view_get_model (priv->item_treeview);
    selection = gtk_tree_view_get_selection (priv->item_treeview);
    rowpathlist = gtk_tree_selection_get_selected_rows (selection, &model);
    rowreflist = NULL;

    //Convert the tree paths to row references
    if (rowpathlist != NULL) {
      for (listiter = rowpathlist; listiter != NULL; listiter = listiter->next)
      {
        path = rowpathlist->data;
        rowref = gtk_tree_row_reference_new (model, path);
        rowreflist = g_list_append (rowreflist, rowref);
        gtk_tree_path_free (path);
      }
      g_list_free (rowpathlist); rowpathlist = NULL;
    }

    //Use the row references to clear the selected rows in the model
    if (rowreflist != NULL)
    {
      for (listiter = rowreflist; listiter != NULL; listiter = listiter->next)
      {
        rowref = listiter->data;
        path = gtk_tree_row_reference_get_path (rowref);
        gtk_tree_model_get_iter (model, &treeiter, path);
        gtk_list_store_remove (GTK_LIST_STORE (model), &treeiter);
        gtk_tree_path_free (path);
        gtk_tree_row_reference_free (rowref);
      }
      g_list_free (rowreflist); rowreflist = NULL;
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_list_cursor_changed_cb (GtkTreeView *view, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;

    gw_vocabularywindow_load_selected_vocabulary (window);
}

