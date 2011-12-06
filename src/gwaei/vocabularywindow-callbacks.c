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
#include <gwaei/vocabularyliststore.h>
#include <gwaei/vocabularywindow-private.h>



G_MODULE_EXPORT void
gw_vocabularywindow_new_list_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    GtkListStore *liststore, *wordstore;
    GtkTreeSelection *selection;
    GtkTreeIter iter;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    liststore = gw_application_get_vocabularyliststore (application);
    selection = gtk_tree_view_get_selection (priv->list_treeview);

    gw_vocabularyliststore_new_list (GW_VOCABULARYLISTSTORE (liststore), &iter);
    gtk_tree_selection_select_iter (selection, &iter);
    wordstore = gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (liststore), &iter);
    gtk_tree_view_set_model (priv->item_treeview, GTK_TREE_MODEL (wordstore));
    gtk_tree_view_set_search_column (priv->item_treeview, GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS);
}


G_MODULE_EXPORT void
gw_vocabularywindow_remove_list_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GList *rowlist;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkTreePath *path;
    gboolean valid;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    model = gtk_tree_view_get_model (priv->list_treeview);
    rowlist = gtk_tree_selection_get_selected_rows (selection, &model);

    gw_vocabularyliststore_remove_path_list (GW_VOCABULARYLISTSTORE (model), rowlist);

    path = (GtkTreePath*) rowlist->data;
    gtk_tree_path_prev (path);
    valid = gtk_tree_model_get_iter (model, &iter, path);

    if (valid)
    {
      gtk_tree_selection_select_iter (selection, &iter);
      store = gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (model), &iter);
      gtk_tree_view_set_model (priv->item_treeview, GTK_TREE_MODEL (store));
      gtk_tree_view_set_search_column (priv->item_treeview, GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS);
    }

    g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (rowlist); rowlist = NULL;
}


static void
gw_vocabularywindow_select_new_word_from_dialog_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwAddVocabularyWindow *avw;
    GtkTreeIter iter;
    GtkTreeSelection *selection;
    gboolean valid;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    avw = GW_ADDVOCABULARYWINDOW (widget);
    selection = gtk_tree_view_get_selection (priv->item_treeview);
    valid = gw_addvocabularywindow_get_iter (GW_ADDVOCABULARYWINDOW (avw), &iter);

    if (valid)
    {
      gtk_tree_selection_unselect_all (selection);
      gtk_tree_selection_select_iter (selection, &iter);
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_new_word_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    GtkWindow *avw;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean valid;
    gchar *list;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    model = gtk_tree_view_get_model (priv->list_treeview);
    if (model == NULL) return;
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    valid = gtk_tree_selection_get_selected (selection, &model, &iter);

    if (valid)
    {
      avw = gw_addvocabularywindow_new (GTK_APPLICATION (application));
      list = gw_vocabularyliststore_get_name_by_iter (GW_VOCABULARYLISTSTORE (model), &iter);
      if (list != NULL)
      {
        gw_addvocabularywindow_set_list (GW_ADDVOCABULARYWINDOW (avw), list);
        gw_addvocabularywindow_set_focus (GW_ADDVOCABULARYWINDOW (avw), GW_ADDVOCABULARYWINDOW_FOCUS_KANJI);
        g_free (list);
      }
      gtk_window_set_transient_for (avw, GTK_WINDOW (window));
      g_signal_connect (G_OBJECT (avw), "destroy", G_CALLBACK (gw_vocabularywindow_select_new_word_from_dialog_cb), window);
      gtk_widget_show (GTK_WIDGET (avw));
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_remove_word_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GList *rowlist;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    selection = gtk_tree_view_get_selection (priv->item_treeview);
    model = gtk_tree_view_get_model (priv->item_treeview);
    rowlist = gtk_tree_selection_get_selected_rows (selection, &model);

    gw_vocabularywordstore_remove_path_list (GW_VOCABULARYWORDSTORE (model), rowlist);

    g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (rowlist); rowlist = NULL;
}


G_MODULE_EXPORT void
gw_vocabularywindow_list_selection_changed_cb (GtkTreeView *view, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    GtkTreeModel *model;
    GtkListStore *store;
    GtkTreeSelection *selection;
    GtkTreeIter iter;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    store = gw_application_get_vocabularyliststore (application);
    model = GTK_TREE_MODEL (store);
    selection = gtk_tree_view_get_selection (priv->list_treeview);

    gtk_tree_selection_get_selected (selection, &model, &iter);

    model = GTK_TREE_MODEL (gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (store), &iter));
    gtk_tree_view_set_model (window->priv->item_treeview, model);
    gtk_tree_view_set_search_column (window->priv->item_treeview, GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS);

    gboolean has_changes;
    has_changes = gw_vocabularywindow_current_wordstore_has_changes (window);

    GtkWidget *widget;
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "revert_menuitem"));
    gtk_widget_set_sensitive (widget, has_changes);
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "revert_toolbutton"));
    gtk_widget_set_sensitive (widget, has_changes);
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

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    view = GTK_TREE_VIEW (data);
    model = gtk_tree_view_get_model (view);
    gtk_tree_model_get_iter_from_string (model, &iter, path_string);
    column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (renderer), "column"));

    gw_vocabularywordstore_set_string (GW_VOCABULARYWORDSTORE (model), &iter, column, new_text);
}


G_MODULE_EXPORT void
gw_vocabularywindow_list_cell_edited_cb (GtkCellRendererText *renderer,
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
    gboolean exists;
    gboolean valid;
    GtkListStore *wordstore;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    view = GTK_TREE_VIEW (data);
    model = GTK_TREE_MODEL (gtk_tree_view_get_model (view));
    column = GPOINTER_TO_INT (g_object_get_data (G_OBJECT (renderer), "column"));
    exists = FALSE;
    valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (model), &iter);

    while (valid && !exists)
    {
      gtk_tree_model_get (GTK_TREE_MODEL (model), &iter, column, &text, -1);
      if (text != NULL)
      {
        if (strcmp(text, new_text) == 0) exists = TRUE;
        g_free (text);
      }
      valid = gtk_tree_model_iter_next (GTK_TREE_MODEL (model), &iter);
    } 

    if (!exists)
    {
      gtk_tree_model_get_iter_from_string (model, &iter, path_string);
      gtk_list_store_set (GTK_LIST_STORE (model), &iter, column, new_text, -1);
      wordstore = gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (model), &iter);
      gw_vocabularywordstore_set_name (GW_VOCABULARYWORDSTORE (wordstore), new_text);
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_save_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwApplication *application;
    GtkListStore *liststore;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    application = gw_window_get_application (GW_WINDOW (window));
    liststore = gw_application_get_vocabularyliststore (application);

    gw_vocabularyliststore_save_all (GW_VOCABULARYLISTSTORE (liststore));
}


G_MODULE_EXPORT void
gw_vocabularywindow_reset_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwApplication *application;
    GtkListStore *liststore, *wordstore;
    GtkTreeIter iter;
    GtkTreeModel *model;
    GtkTreeSelection *selection;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    liststore = gw_application_get_vocabularyliststore (application);
    model = GTK_TREE_MODEL (liststore);
    selection = gtk_tree_view_get_selection (priv->list_treeview);

    gw_vocabularyliststore_revert_all (GW_VOCABULARYLISTSTORE (liststore));

    if (!gtk_tree_selection_get_selected (selection, &model, NULL))
    {
      gtk_tree_model_get_iter_first (model, &iter);
      gtk_tree_selection_select_iter (selection, &iter);
      wordstore = gw_vocabularyliststore_get_wordstore_by_iter (GW_VOCABULARYLISTSTORE (liststore), &iter);
      gtk_tree_view_set_model (priv->item_treeview, GTK_TREE_MODEL (wordstore));
      gtk_tree_view_set_search_column (priv->item_treeview, GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS);
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
    GwApplication *application;
    GtkListStore *store;
    GtkWidget *dialog;
    gint response;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    application = gw_window_get_application (GW_WINDOW (window));
    store = gw_application_get_vocabularyliststore (application);

    if (gw_vocabularywindow_has_changes (window))
    {
      GtkWidget *box;
      GtkWidget *image;
      GtkWidget *label;
      GtkWidget *content_area;
      gchar *markup, *header, *description;


      dialog = gtk_dialog_new ();
      gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (window));
      gtk_window_set_title (GTK_WINDOW (dialog), gettext("Save changes before closing?"));
      gtk_dialog_add_button (GTK_DIALOG (dialog), gettext("Close _without Saving"), GTK_RESPONSE_NO);
      gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
      gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_SAVE, GTK_RESPONSE_YES);
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
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
        case GTK_RESPONSE_YES:
          gw_vocabularyliststore_save_all (GW_VOCABULARYLISTSTORE (store)); 
          gtk_widget_destroy (GTK_WIDGET (window));
          break;
        case GTK_RESPONSE_CANCEL:
          break;
        case GTK_RESPONSE_NO:
          gw_vocabularyliststore_revert_all (GW_VOCABULARYLISTSTORE (store)); 
          gtk_widget_destroy (GTK_WIDGET (window));
          break;
        default:
          break;
      }
      gtk_widget_destroy (GTK_WIDGET (dialog));
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
    GwVocabularyWindowPrivate *priv;
    GtkTreeSelection *selection;
    GtkTreeModel *model;
    GList *rowlist;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    model = gtk_tree_view_get_model (priv->item_treeview);

    if (model != NULL)
    {
      selection = gtk_tree_view_get_selection (priv->item_treeview);
      rowlist = gtk_tree_selection_get_selected_rows (selection, &model);

      gw_vocabularywordstore_remove_path_list (GW_VOCABULARYWORDSTORE (model), rowlist);

      g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
      g_list_free (rowlist); rowlist = NULL;
    }
}


void
gw_vocabularywindow_cut_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkClipboard *clipboard;
    gchar *text;
    GtkTreeSelection *selection;
    GList *rowlist;
    GtkTreeModel *model;
    
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    model = gtk_tree_view_get_model (priv->item_treeview);

    if (model != NULL)
    {
      clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
      selection = gtk_tree_view_get_selection (priv->item_treeview);
      rowlist = gtk_tree_selection_get_selected_rows (selection, &model);
      text = gw_vocabularywordstore_path_list_to_string (GW_VOCABULARYWORDSTORE (model), rowlist);

      gw_vocabularywordstore_remove_path_list (GW_VOCABULARYWORDSTORE (model), rowlist);

      gtk_clipboard_set_text (clipboard, text, -1);

      g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
      g_list_free (rowlist); rowlist = NULL;
      g_free (text); text = NULL;
    }
}


void
gw_vocabularywindow_copy_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkClipboard *clipboard;
    gchar *text;
    GtkTreeSelection *selection;
    GList *rowlist;
    GtkTreeModel *model;
    
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    model = gtk_tree_view_get_model (priv->item_treeview);

    if (model != NULL)
    {
      clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
      selection = gtk_tree_view_get_selection (priv->item_treeview);
      rowlist = gtk_tree_selection_get_selected_rows (selection, &model);
      text = gw_vocabularywordstore_path_list_to_string (GW_VOCABULARYWORDSTORE (model), rowlist);

      gtk_clipboard_set_text (clipboard, text, -1);

      g_list_foreach (rowlist, (GFunc) gtk_tree_path_free, NULL);
      g_list_free (rowlist); rowlist = NULL;
      g_free (text); text = NULL;
    }
}


void
gw_vocabularywindow_paste_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkClipboard *clipboard;
    GtkListStore *store;
    GtkTreeSelection *selection;
    gchar *text;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    store = GTK_LIST_STORE (gtk_tree_view_get_model (priv->item_treeview));

    if (store != NULL)
    {
      clipboard = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
      selection = gtk_tree_view_get_selection (priv->item_treeview);
      text = gtk_clipboard_wait_for_text (clipboard);

      gw_vocabularywordstore_append_text (GW_VOCABULARYWORDSTORE (store), NULL, TRUE, text);
      gtk_tree_selection_unselect_all (selection);

      g_free (text);
    }
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


G_MODULE_EXPORT void
gw_vocabularywindow_liststore_changed_cb (GwVocabularyListStore *store, gpointer data)
{
  GwVocabularyWindow *window;
  gboolean has_changes;

  window = GW_VOCABULARYWINDOW (data);
  has_changes = gw_vocabularyliststore_has_changes (store);

  gw_vocabularywindow_set_has_changes (window, has_changes);
}


G_MODULE_EXPORT void
gw_vocabularywindow_revert_wordstore_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwVocabularyListStore *liststore;
    GtkListStore *wordstore;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    GtkTreeModel *model;
    gboolean valid;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    liststore = GW_VOCABULARYLISTSTORE (gtk_tree_view_get_model (priv->list_treeview));
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    model = GTK_TREE_MODEL (liststore);
    valid = gtk_tree_selection_get_selected (selection, &model, &iter);

    if (valid)
    {
      wordstore = gw_vocabularyliststore_get_wordstore_by_iter (liststore, &iter);
      gw_vocabularywordstore_reset (GW_VOCABULARYWORDSTORE (wordstore));
      gw_vocabularywordstore_load (GW_VOCABULARYWORDSTORE (wordstore));
    }
}


G_MODULE_EXPORT void
gw_vocabularywindow_toggle_editing_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkToggleToolButton *button;
    gboolean state;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    button = GTK_TOGGLE_TOOL_BUTTON (widget);
    state = gtk_toggle_tool_button_get_active (button);

    g_object_set (G_OBJECT (priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_KANJI]), "editable", state, NULL);
    g_object_set (G_OBJECT (priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_FURIGANA]), "editable", state, NULL);
    g_object_set (G_OBJECT (priv->renderer[GW_VOCABULARYWORDSTORE_COLUMN_DEFINITIONS]), "editable", state, NULL);
}
