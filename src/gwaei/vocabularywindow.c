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
//! @file vocabularywindow.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <string.h>

#include <glib/gstdio.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/vocabularymodel.h>
#include <gwaei/vocabularywindow-private.h>


//Static declarations
static void gw_vocabularywindow_attach_signals (GwVocabularyWindow*);
static void gw_vocabularywindow_remove_signals (GwVocabularyWindow*);
static void gw_vocabularywindow_init_styles (GwVocabularyWindow*);
static void gw_vocabularywindow_init_list_treeview (GwVocabularyWindow*);
static void gw_vocabularywindow_init_item_treeview (GwVocabularyWindow*);

G_DEFINE_TYPE (GwVocabularyWindow, gw_vocabularywindow, GW_TYPE_WINDOW)

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* 
gw_vocabularywindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwVocabularyWindow *window;

    //Initializations
    window = GW_VOCABULARYWINDOW (g_object_new (GW_TYPE_VOCABULARYWINDOW,
                                                "type",        GTK_WINDOW_TOPLEVEL,
                                                "application", GW_APPLICATION (application),
                                                "ui-xml",      "vocabularywindow.ui",
                                                NULL));

    return GTK_WINDOW (window);
}


static void 
gw_vocabularywindow_init (GwVocabularyWindow *window)
{
    window->priv = GW_VOCABULARYWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwVocabularyWindowPrivate));
}


static void 
gw_vocabularywindow_finalize (GObject *object)
{
//    GwVocabularyWindow *window;
//    GwVocabularyWindowPrivate *priv;

//    window = GW_VOCABULARYWINDOW (object);
//    priv = window->priv;

    G_OBJECT_CLASS (gw_vocabularywindow_parent_class)->finalize (object);
}


static void 
gw_vocabularywindow_constructed (GObject *object)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    /*
    GtkToolButton *toolbutton;
    GtkWidget *widget;
    GtkAccelGroup *accelgroup;
    */

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_vocabularywindow_parent_class)->constructed (object);
    }
    //Initializations
    window = GW_VOCABULARYWINDOW (object);
    priv = window->priv;

    //Set up the gtkbuilder links
    priv->list_treeview = GTK_TREE_VIEW (gw_window_get_object (GW_WINDOW (window), "vocabulary_list_treeview"));
    priv->list_toolbar =  GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "vocabulary_list_toolbar"));
    priv->item_treeview = GTK_TREE_VIEW (gw_window_get_object (GW_WINDOW (window), "vocabulary_item_treeview"));
    priv->item_toolbar =  GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "vocabulary_item_toolbar"));
    priv->study_toolbar =  GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "study_toolbar"));

    //Set up the gtk window
    gtk_window_set_title (GTK_WINDOW (window), gettext("gWaei Vocabulary Manager"));
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
    gtk_window_set_default_size (GTK_WINDOW (window), 620, 300);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");
    gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_NORMAL);
    gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), FALSE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (window), FALSE);

    gw_vocabularywindow_init_styles (window);
    gw_vocabularywindow_init_list_treeview (window);
    gw_vocabularywindow_init_item_treeview (window);
/*
    //Set menu accelerators
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "new_window_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_N), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
*/
    gw_vocabularywindow_attach_signals (window);
}


static void
gw_vocabularywindow_class_init (GwVocabularyWindowClass *klass)
{
    GObjectClass *object_class;

    object_class = G_OBJECT_CLASS (klass);

    object_class->constructed = gw_vocabularywindow_constructed;
    object_class->finalize = gw_vocabularywindow_finalize;

    klass->list_model = GTK_TREE_MODEL (gtk_list_store_new (1, G_TYPE_STRING));
    klass->item_models = NULL;
    klass->list_new_index = 0;

    GtkTreeIter iter;
    gchar **lists;
    int i;
    if ((lists = lw_vocabularylist_get_lists ()) != NULL)
    {
      for (i = 0; lists[i] != NULL; i++)
      {
        gtk_list_store_append (GTK_LIST_STORE (klass->list_model), &iter);
        gtk_list_store_set (GTK_LIST_STORE (klass->list_model), &iter, GW_VOCABULARYLIST_COLUMN_NAME, lists[i], -1);
        klass->item_models = g_list_append (klass->item_models, gw_vocabularymodel_new (lists[i]));
      }
      g_strfreev (lists); lists = NULL;
    }

    g_type_class_add_private (object_class, sizeof (GwVocabularyWindowPrivate));
}


static void 
gw_vocabularywindow_attach_signals (GwVocabularyWindow *window)
{
    //Declarations
//    GwApplication *application;

//    application = gw_window_get_application (GW_WINDOW (window));

    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (gw_vocabularywindow_remove_signals), NULL);
}


static void 
gw_vocabularywindow_remove_signals (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GSource *source;
    gint i;

    priv = window->priv;

    for (i = 0; i < TOTAL_GW_VOCABULARYWINDOW_TIMEOUTIDS; i++)
    {
      if (g_main_current_source () != NULL &&
          !g_source_is_destroyed (g_main_current_source ()) &&
          priv->timeoutid[i] > 0
         )
      {
        source = g_main_context_find_source_by_id (NULL, priv->timeoutid[i]);
        if (source != NULL)
        {
          g_source_destroy (source);
        }
      }
      priv->timeoutid[i] = 0;
    }
}


static void
gw_vocabularywindow_init_styles (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GtkStyleContext *context;
    GtkWidget *widget;
    GtkJunctionSides sides;

    //Initializations
    priv = window->priv;

    //Vocabulary list pane
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "vocabulary_list_scrolledwindow"));
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_BOTTOM);
    gtk_widget_reset_style (widget);

    context = gtk_widget_get_style_context (GTK_WIDGET (priv->list_toolbar));
    gtk_style_context_add_class (context, "inline-toolbar");
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_TOP);
    gtk_widget_reset_style (GTK_WIDGET (priv->list_toolbar));

    //Vocabulary listitem pane
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "vocabulary_item_scrolledwindow"));
    context = gtk_widget_get_style_context (widget);
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_BOTTOM);
    gtk_widget_reset_style (widget);

    context = gtk_widget_get_style_context (GTK_WIDGET (priv->item_toolbar));
    gtk_style_context_add_class (context, "inline-toolbar");
    sides = GTK_JUNCTION_CORNER_TOPLEFT | GTK_JUNCTION_CORNER_TOPRIGHT | GTK_JUNCTION_CORNER_BOTTOMRIGHT;
    gtk_style_context_set_junction_sides (context, sides);
    gtk_widget_reset_style (GTK_WIDGET (priv->item_toolbar));

    context = gtk_widget_get_style_context (GTK_WIDGET (priv->study_toolbar));
    gtk_style_context_add_class (context, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
    sides = GTK_JUNCTION_CORNER_BOTTOMLEFT | GTK_JUNCTION_CORNER_TOPLEFT | GTK_JUNCTION_CORNER_TOPRIGHT;
    gtk_style_context_set_junction_sides (context, sides);
    gtk_widget_reset_style (GTK_WIDGET (priv->study_toolbar));
}


static void
gw_vocabularywindow_init_list_treeview (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GwVocabularyWindowClass *klass;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkTreeSelection *selection;

    priv = window->priv;
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    selection = gtk_tree_view_get_selection (priv->list_treeview);

    //Set up the columns
    column = gtk_tree_view_column_new ();
    gtk_tree_view_append_column (priv->list_treeview, column);

    renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer), "editable", TRUE, NULL);
    g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (GW_VOCABULARYLIST_COLUMN_NAME));
    g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (gw_vocabularywindow_list_cell_edited_cb), priv->list_treeview);
    gtk_tree_view_column_set_title (column, gettext("Lists"));
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer, "text", GW_VOCABULARYLIST_COLUMN_NAME, NULL);

    //Set up the model
    gtk_tree_view_set_model (priv->list_treeview, klass->list_model);

    gtk_tree_selection_set_mode (selection, GTK_SELECTION_BROWSE);
}


static void
gw_vocabularywindow_init_item_treeview (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;
    GtkTreeSelection *selection;

    priv = window->priv;
    selection = gtk_tree_view_get_selection (priv->item_treeview);

    //Set up the columns
    column = gtk_tree_view_column_new ();
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer), "editable", TRUE, NULL);
    g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (GW_VOCABULARYMODEL_COLUMN_KANJI));
    g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (gw_vocabularywindow_cell_edited_cb), priv->item_treeview);
    gtk_tree_view_column_set_title (column, gettext("Word"));
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, "text", GW_VOCABULARYMODEL_COLUMN_KANJI, NULL);
    gtk_tree_view_append_column (priv->item_treeview, column);

    column = gtk_tree_view_column_new ();
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer), "editable", TRUE, NULL);
    g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (GW_VOCABULARYMODEL_COLUMN_FURIGANA));
    g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (gw_vocabularywindow_cell_edited_cb), priv->item_treeview);
    gtk_tree_view_column_set_title (column, gettext("Reading"));
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, "text", GW_VOCABULARYMODEL_COLUMN_FURIGANA, NULL);
    gtk_tree_view_append_column (priv->item_treeview, column);

    column = gtk_tree_view_column_new ();
    renderer = gtk_cell_renderer_text_new ();
    g_object_set (G_OBJECT (renderer), "editable", TRUE, NULL);
    g_object_set_data (G_OBJECT (renderer), "column", GINT_TO_POINTER (GW_VOCABULARYMODEL_COLUMN_DEFINITIONS));
    g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (gw_vocabularywindow_cell_edited_cb), priv->item_treeview);
    gtk_tree_view_column_set_title (column, gettext("Definitions"));
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, "text", GW_VOCABULARYMODEL_COLUMN_DEFINITIONS, NULL);
    gtk_tree_view_append_column (priv->item_treeview, column);

    gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
}


void
gw_vocabularywindow_create_new_list (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GwVocabularyWindowClass *klass;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gchar *name;

    //Initializations
    priv = window->priv;
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    
    name = g_strdup_printf (gettext("New List %d"), ++klass->list_new_index);
    while (gw_vocabularywindow_list_exists (window, name))
    {
      g_free (name);
      name = g_strdup_printf (gettext("New List %d"), ++klass->list_new_index);
    }

    gtk_list_store_append (GTK_LIST_STORE (klass->list_model), &iter);
    gtk_list_store_set (GTK_LIST_STORE (klass->list_model), &iter, 
        GW_VOCABULARYLIST_COLUMN_NAME, name, 
    -1);
    gtk_tree_selection_select_iter (selection, &iter);
    klass->item_models = g_list_append (klass->item_models, gw_vocabularymodel_new (name));

    g_free (name);

    gw_vocabularywindow_set_selected_vocabulary (window);
}


void
gw_vocabularywindow_remove_selected_lists (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowClass *klass;
    GwVocabularyWindowPrivate *priv;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GList *rowpathlist;
    GList *rowreflist;
    GList *listiter;
    GList *link;
    GtkTreeRowReference *rowref;
    GtkTreePath *path;
    GtkTreeIter treeiter;
    gint *indices;

    //Initializations
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    priv = window->priv;
    model = gtk_tree_view_get_model (priv->list_treeview);
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    rowpathlist = gtk_tree_selection_get_selected_rows (selection, &model);
    rowreflist = NULL;

    //Convert the tree paths to row references
    if (rowpathlist != NULL) {
      for (listiter = rowpathlist; listiter != NULL; listiter = listiter->next)
      {
        path = rowpathlist->data;

        indices = gtk_tree_path_get_indices (path);
        if (indices != NULL)
        {
          link = g_list_nth (klass->item_models, *indices);
          g_object_unref (G_OBJECT (link->data));
          klass->item_models = g_list_delete_link (klass->item_models, link);
        }

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

    gtk_tree_view_set_model (priv->item_treeview, NULL);
}


void
gw_vocabularywindow_set_vocabulary_by_index (GwVocabularyWindow *window, gint index)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GwVocabularyWindowClass *klass;
    GtkTreeModel *model;

    //Initializations
    priv = window->priv;
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    model = GTK_TREE_MODEL (g_list_nth_data (klass->item_models, index));

    gtk_tree_view_set_model (priv->item_treeview, model);
    gw_vocabularymodel_load (GW_VOCABULARYMODEL (model));
}


void
gw_vocabularywindow_set_selected_vocabulary (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GwVocabularyWindowClass *klass;
    GtkTreeSelection *selection;
    GtkTreePath *path;
    GtkTreeModel *model;
    GtkTreeIter iter;
    gint* indices;

    //Initializations
    priv = GW_VOCABULARYWINDOW_GET_PRIVATE (window);
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    model = GTK_TREE_MODEL (klass->list_model);
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    gtk_tree_selection_get_selected (selection, &model, &iter);
    path = gtk_tree_model_get_path (model, &iter);
    indices = gtk_tree_path_get_indices (path);

    gw_vocabularywindow_set_vocabulary_by_index (window, indices[0]);

    //Cleanup
    gtk_tree_path_free (path);
}


void
gw_vocabularywindow_save (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowClass *klass;
    GwVocabularyModel *model;
    GList *iter;

    //Initializations
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));

    gw_vocabularywindow_clean_files (window);
    for (iter = klass->item_models; iter != NULL; iter = iter->next)
    {
      model = GW_VOCABULARYMODEL (iter->data);
      gw_vocabularymodel_save (model);
    }
}


void
gw_vocabularywindow_reset (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowClass *klass;
    GwVocabularyModel *model;
    GList *iter;

    //Initializations
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));

    for (iter = klass->item_models; iter != NULL; iter = iter->next)
    {
      model = GW_VOCABULARYMODEL (iter->data);
      gw_vocabularymodel_reset (model);
    }
}


gboolean
gw_vocabularywindow_list_exists (GwVocabularyWindow *window, const gchar *NAME)
{
    //Declarations
    GwVocabularyWindowClass *klass;
    GwVocabularyModel *model;
    GList *iter;
      
    //Initializations
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));

    for (iter = klass->item_models; iter != NULL; iter = iter->next)
    {
      model = GW_VOCABULARYMODEL (iter->data);
      if (strcmp(NAME, gw_vocabularymodel_get_name (model)) == 0) return TRUE;
    }

    return FALSE;
}


void
gw_vocabularywindow_clean_files (GwVocabularyWindow *window)
{
    //Definitions
    GDir *dir;
    gchar *uri;
    gchar *filename;
    const gchar *name;

    if ((uri = lw_util_build_filename (LW_PATH_VOCABULARY, NULL)) != NULL)
    {
      if ((dir = g_dir_open (uri, 0, NULL)) != NULL)
      {
        while ((name = g_dir_read_name (dir)) != NULL)
        {
          if (!gw_vocabularywindow_list_exists (window, name))
          {
            if ((filename = g_build_filename (uri, name, NULL)) != NULL)
            {
              g_remove (filename);
              g_free (filename); filename = NULL;
            }
          }
        }
        g_dir_close (dir); dir = NULL;
      }
      g_free (uri); uri = NULL;
    }
}

