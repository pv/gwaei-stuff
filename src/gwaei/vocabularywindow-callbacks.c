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
#include <gwaei/vocabularymodel.h>
#include <gwaei/vocabularywindow-private.h>



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
    GtkTreeIter iter;
    GtkTreeModel *model;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    model = gtk_tree_view_get_model (priv->item_treeview);

    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
        GW_VOCABULARYMODEL_COLUMN_KANJI, gettext("(Click to set Kanji)"), 
        GW_VOCABULARYMODEL_COLUMN_FURIGANA, gettext("(Click to set Furigana)"),
        GW_VOCABULARYMODEL_COLUMN_DEFINITIONS, gettext("(Click to set Definitions)"),
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
    GList *list;
    GtkTreePath *path;
    GtkTreeIter treeiter;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    model = gtk_tree_view_get_model (priv->item_treeview);
    selection = gtk_tree_view_get_selection (priv->item_treeview);

    while (gtk_tree_selection_count_selected_rows (selection))
    {
      list = gtk_tree_selection_get_selected_rows (selection, &model);
      path = list->data;

      gtk_tree_model_get_iter (model, &treeiter, path);
      gtk_list_store_remove (GTK_LIST_STORE (model), &treeiter);

      g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
      g_list_free (list); list = NULL;
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

    gw_vocabularywindow_set_selected_vocabulary (window);
}


G_MODULE_EXPORT void
gw_vocabularywindow_cell_edited_cb (GtkCellRendererText *renderer,
                                    gchar               *path_string,
                                    gchar               *new_text,
                                    gpointer             data       )
{
    //Declarations
    GtkTreeView *view;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint column;

    //Initializations
    view = GTK_TREE_VIEW (data);
    model = GTK_TREE_MODEL (gtk_tree_view_get_model (view));
    gtk_tree_model_get_iter_from_string (model, &iter, path_string);
    column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (renderer), "column"));
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, column, new_text, -1);
}


G_MODULE_EXPORT void
gw_vocabularywindow_save_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;

    gw_vocabularywindow_save (window);
}

