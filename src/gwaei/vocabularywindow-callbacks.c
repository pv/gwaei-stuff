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
gw_vocabularywindow_new_list_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;

    gw_vocabularywindow_new_list (window);
}


G_MODULE_EXPORT void
gw_vocabularywindow_remove_list_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;

    gw_vocabularywindow_remove_selected_lists (window);
}


G_MODULE_EXPORT void
gw_vocabularywindow_new_word_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    model = gtk_tree_view_get_model (priv->item_treeview);
    selection = gtk_tree_view_get_selection (priv->item_treeview);

    gtk_list_store_append (GTK_LIST_STORE (model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (model), &iter, 
        GW_VOCABULARYMODEL_COLUMN_KANJI, gettext("(Click to set Kanji)"), 
        GW_VOCABULARYMODEL_COLUMN_FURIGANA, gettext("(Click to set Furigana)"),
        GW_VOCABULARYMODEL_COLUMN_DEFINITIONS, gettext("(Click to set Definitions)"),
    -1);
    gtk_tree_selection_unselect_all (selection);
    gtk_tree_selection_select_iter (selection, &iter);
    gw_vocabularymodel_set_has_changes (GW_VOCABULARYMODEL (model), TRUE);
    gw_vocabularywindow_set_has_changes (window, TRUE);
}


G_MODULE_EXPORT void
gw_vocabularywindow_remove_word_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;

    gw_vocabularywindow_remove_selected_words (window);
}


G_MODULE_EXPORT void
gw_vocabularywindow_list_selection_changed_cb (GtkTreeView *view, gpointer data)
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
    GwVocabularyWindow *window;
    GtkTreeView *view;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint column;
    gchar *text;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    view = GTK_TREE_VIEW (data);
    model = GTK_TREE_MODEL (gtk_tree_view_get_model (view));
    gtk_tree_model_get_iter_from_string (model, &iter, path_string);
    column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (renderer), "column"));

    gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, column, &text, -1);
    if (strcmp(text, new_text) != 0)
    {
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, column, new_text, -1);
      gw_vocabularymodel_set_has_changes (GW_VOCABULARYMODEL (model), TRUE);
      gw_vocabularywindow_set_has_changes (window, TRUE);
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_list_cell_edited_cb (GtkCellRendererText *renderer,
                                         gchar               *path_string,
                                         gchar               *new_text,
                                         gpointer             data       )
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowClass *klass;
    GtkTreeView *view;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint column;
    gchar *text;
    gboolean exists;
    gint *indices;
    GtkTreePath *path;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    view = GTK_TREE_VIEW (data);
    model = GTK_TREE_MODEL (gtk_tree_view_get_model (view));
    column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (renderer), "column"));
    exists = FALSE;

    if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter))
    {
      do
      {
        gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, column, &text, -1);
        if (text != NULL)
        {
          if (strcmp(text, new_text) == 0) exists = TRUE;
          g_free (text);
        }
      } while (gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter) && !exists);
    }

    if (!exists)
    {
      gtk_tree_model_get_iter_from_string (model, &iter, path_string);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, column, new_text, -1);

      path = gtk_tree_path_new_from_string (path_string);
      indices = gtk_tree_path_get_indices (path);
      model = GTK_TREE_MODEL (g_list_nth_data (klass->item_models, *indices));
      gw_vocabularymodel_set_name (GW_VOCABULARYMODEL (model), new_text);
      gw_vocabularywindow_set_has_changes (window, TRUE);
    }
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


G_MODULE_EXPORT void
gw_vocabularywindow_reset_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;

    gw_vocabularywindow_reset (window);
}


G_MODULE_EXPORT void
gw_vocabularywindow_list_row_deleted_cb (GtkTreeModel *model, GtkTreePath *path, gpointer data)
{
    GtkTreeView *view;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    
    view = GTK_TREE_VIEW (data);
    selection = gtk_tree_view_get_selection (view);

    gtk_tree_path_prev (path);
    if (gtk_tree_model_get_iter (model, &iter, path))
    {
      gtk_tree_selection_select_iter (selection, &iter);
    }
}


G_MODULE_EXPORT gboolean
gw_vocabularywindow_delete_event_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    gw_vocabularywindow_close_cb (widget, data);

    return TRUE;
}


G_MODULE_EXPORT void
gw_vocabularywindow_close_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GtkWidget *dialog;
    gint response;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;

    if (gw_vocabularywindow_has_changes (window))
    {
      GtkWidget *box;
      GtkWidget *image;
      GtkWidget *label;
      GtkWidget *content_area;
      gchar *markup, *header, *description;


      dialog = gtk_dialog_new_with_buttons ("Save changes before closing?",
                                            GTK_WINDOW (window),
                                            GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                            GTK_STOCK_CLOSE,
                                            GTK_RESPONSE_CLOSE,
                                            GTK_STOCK_CANCEL,
                                            GTK_RESPONSE_CANCEL,
                                            GTK_STOCK_SAVE,
                                            GTK_RESPONSE_APPLY,
                                            NULL);
      content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

      box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 16);
      image = gtk_image_new_from_stock (GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_DIALOG);
      label = gtk_label_new (NULL);
      gtk_misc_set_padding (GTK_MISC (label), 0, 8);
      gtk_box_pack_start (GTK_BOX (box), image, FALSE, FALSE, 0);
      gtk_box_pack_start (GTK_BOX (box), label, TRUE, TRUE, 0);
      gtk_widget_show_all (box);

      header = gettext("Save Changes before Closing?");
      description = gettext("Some of your vocabulary lists have changed since your last save.");
      markup = g_markup_printf_escaped ("<big><b>%s</b></big>\n%s", header, description);
      gtk_label_set_markup (GTK_LABEL (label), markup);
      g_free (markup);
      gtk_container_add (GTK_CONTAINER (content_area), box);
      gtk_container_set_border_width (GTK_CONTAINER (box), 8);
      gtk_container_set_border_width (GTK_CONTAINER (dialog), 6);
      response = gtk_dialog_run (GTK_DIALOG (dialog));

      switch (response)
      {
        case GTK_RESPONSE_APPLY:
          gw_vocabularywindow_save (window); 
          gtk_widget_destroy (GTK_WIDGET (window));
          break;
        case GTK_RESPONSE_CANCEL:
          gtk_widget_destroy (GTK_WIDGET (dialog));
          break;
        case GTK_RESPONSE_CLOSE:
          gtk_widget_destroy (GTK_WIDGET (window));
          break;
        default:
          gtk_widget_destroy (GTK_WIDGET (dialog));
          break;
      }
    }
    else
    {
      gtk_widget_destroy (GTK_WIDGET (window));
    }
}


void
gw_vocabularywindow_export_cb (GtkWidget *widget, gpointer data)
{
  g_warning ("this function is unimplimented\n");
}


void
gw_vocabularywindow_import_cb (GtkWidget *widget, gpointer data)
{
  g_warning ("this function is unimplimented\n");
}


void
gw_vocabularywindow_delete_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;

    gw_vocabularywindow_remove_selected_words (window);
}


void
gw_vocabularywindow_cut_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GtkClipboard *clipboard;
    gchar *text;
    
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
    text = gw_vocabularywindow_selected_words_to_string (window);

    gtk_clipboard_set_text (clipboard, text, -1);

    g_free (text);

    gw_vocabularywindow_remove_selected_words (window);
}


void
gw_vocabularywindow_copy_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GtkClipboard *clipboard;
    gchar *text;
    
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
    text = gw_vocabularywindow_selected_words_to_string (window);

    gtk_clipboard_set_text (clipboard, text, -1);

    g_free (text);

}


void
gw_vocabularywindow_paste_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GtkClipboard *clipboard;
    gchar *text;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
    text = gtk_clipboard_wait_for_text (clipboard);

    gw_vocabularywindow_append_text (window, -1, text);

    g_free (text);
}


G_MODULE_EXPORT gboolean
gw_vocabularywindow_event_after_cb (GtkWidget *widget, GdkEvent *event, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkWidget *focus;
    GtkWidget *menuitem;
    gboolean sensitive;
    gchar *ids[] = { "copy_menuitem", "paste_menuitem", "cut_menuitem", "delete_menuitem", NULL };
    int i = 0;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return FALSE;
    priv = window->priv;
    focus = gtk_window_get_focus (GTK_WINDOW (window));
    sensitive = (GTK_WIDGET (priv->item_treeview) == focus);

    for (i = 0; ids[i] != NULL; i++)
    {
      menuitem = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), ids[i]));
      gtk_widget_set_sensitive (menuitem, sensitive);
    }

    return FALSE;
}


