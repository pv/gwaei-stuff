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
static void gw_vocabularywindow_init_accelerators (GwVocabularyWindow*);

enum {
    TARGET_LIST_ROW_STRING,
    TARGET_WORD_ROW_STRING
};

//Word model
static GtkTargetEntry word_row_dest_targets[] = { //Will accept drops
    { "words",    GTK_TARGET_SAME_WIDGET, TARGET_WORD_ROW_STRING }
};
static guint n_word_row_dest_targets = G_N_ELEMENTS (word_row_dest_targets);

static GtkTargetEntry word_row_source_targets[] = { //Data given to drags
    { "words",    GTK_TARGET_SAME_WIDGET, TARGET_WORD_ROW_STRING }
};
static guint n_word_row_source_targets = G_N_ELEMENTS (word_row_source_targets);

//List model
static GtkTargetEntry list_row_dest_targets[] = { //Will accept drops
    { "lists",    GTK_TARGET_SAME_WIDGET, TARGET_LIST_ROW_STRING },
    { "words",    GTK_TARGET_SAME_APP,    TARGET_WORD_ROW_STRING }
};
static guint n_list_row_dest_targets = G_N_ELEMENTS (list_row_dest_targets);

static GtkTargetEntry list_row_source_targets[] = { //Data given to drags
    { "lists",    GTK_TARGET_SAME_WIDGET, TARGET_LIST_ROW_STRING }
};
static guint n_list_row_source_targets = G_N_ELEMENTS (list_row_source_targets);




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
    gtk_window_set_default_size (GTK_WINDOW (window), 620, 450);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");
    gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_NORMAL);
    gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), FALSE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (window), FALSE);

    gw_vocabularywindow_init_styles (window);
    gw_vocabularywindow_init_list_treeview (window);
    gw_vocabularywindow_init_item_treeview (window);

    gw_vocabularywindow_init_accelerators (window);

    gw_vocabularywindow_attach_signals (window);

    {
      //Set the initial selection
      GtkTreeSelection *selection;
      GtkTreeModel *model;
      GtkTreeIter iter;
      selection = gtk_tree_view_get_selection (priv->list_treeview);
      model = gtk_tree_view_get_model (priv->list_treeview);
      if (gtk_tree_model_get_iter_first (model, &iter))
      {
        gtk_tree_selection_select_iter (selection, &iter);
        gw_vocabularywindow_set_selected_vocabulary (window);
      }
    }
}


static void
gw_vocabularywindow_init_accelerators (GwVocabularyWindow *window)
{
    GtkWidget *widget;
    GtkAccelGroup *accelgroup;

    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));

    //File popup
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "new_list_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_N), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "new_word_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_N), GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "save_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_S), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "import_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_O), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "export_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_S), GDK_CONTROL_MASK | GDK_SHIFT_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "close_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_W), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    //Edit popup
    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "revert_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_Z), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "cut_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_X), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "copy_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_C), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "paste_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_V), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "delete_menuitem"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_Delete), 0, GTK_ACCEL_VISIBLE);
}


static void
gw_vocabularywindow_class_reset_models (GwVocabularyWindowClass* klass)
{
    GtkTreeIter iter;
    gchar **lists;
    int i;

    gtk_list_store_clear (GTK_LIST_STORE (klass->list_model));
    if (klass->item_models != NULL)
    {
      g_list_foreach (klass->item_models, (GFunc) g_object_unref, NULL);
      g_list_free (klass->item_models); klass->item_models = NULL;
    }

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

    gw_vocabularywindow_class_reset_models (klass);

    g_type_class_add_private (object_class, sizeof (GwVocabularyWindowPrivate));
}


static void 
gw_vocabularywindow_attach_signals (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;

    priv = window->priv;


    priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_LIST_ROW_DELETED] = g_signal_connect (
        G_OBJECT (gtk_tree_view_get_model(priv->list_treeview)), 
        "row-deleted", 
        G_CALLBACK (gw_vocabularywindow_list_row_deleted_cb), 
        priv->list_treeview
    );

    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (gw_vocabularywindow_remove_signals), NULL);
    g_signal_connect (G_OBJECT (window), "delete-event",
                      G_CALLBACK (gw_vocabularywindow_delete_event_cb), window);
    g_signal_connect (G_OBJECT (window), "event-after",
                      G_CALLBACK (gw_vocabularywindow_event_after_cb), window);
}


static void 
gw_vocabularywindow_remove_signals (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GSource *source;
    gint i;
    GtkTreeModel *model;

    priv = window->priv;
    model = gtk_tree_view_get_model (priv->list_treeview);

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

    g_signal_handler_disconnect (model, priv->signalid[GW_VOCABULARYWINDOW_SIGNALID_LIST_ROW_DELETED]);
}


static void
gw_vocabularywindow_init_styles (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GtkStyleContext *context;
    GtkWidget *widget;
    GtkJunctionSides sides;

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


void
gw_vocabularywindow_list_drag_motion_cb (
  GtkWidget *widget,
  GdkDragContext *context,
  gint x,
  gint y,
  guint time,
  gpointer data)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkTreeView *view;
    GtkTreeModel *model;
    GtkTreeViewDropPosition drop_position;
    GtkTreeSelection *selection;
    GtkTreePath *path;
    GtkTreeIter iter, previous_iter;
    GtkTreeView *source;

    //Initializations
    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (widget), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return;
    priv = window->priv;
    view = GTK_TREE_VIEW (widget);
    selection = gtk_tree_view_get_selection (view);
    model = gtk_tree_view_get_model (view);
    source = GTK_TREE_VIEW (gtk_drag_get_source_widget (context));

    if (source == priv->list_treeview)
    {
      gtk_tree_view_get_dest_row_at_pos (view, x, y, &path, &drop_position);

      if (path != NULL)
      {
        if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_BEFORE) 
          drop_position = GTK_TREE_VIEW_DROP_BEFORE;
        else if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_AFTER) 
          drop_position = GTK_TREE_VIEW_DROP_AFTER;

        gtk_tree_view_set_drag_dest_row (view, path, drop_position);
      }
    }
    else if (source == priv->item_treeview)
    {
      gtk_tree_view_get_path_at_pos (view, x, y, &path, NULL, NULL, NULL);
      if (path == NULL)
      {
        if (gtk_tree_model_get_iter_first (model, &iter)) {
          previous_iter = iter;
          while (gtk_tree_model_iter_next (model, &iter)) previous_iter = iter;
          iter = previous_iter;
        }
      }
      else
      {
        gtk_tree_path_prev (path);
        gtk_tree_model_get_iter (model, &iter, path);
      }
        gtk_tree_selection_select_iter (selection, &iter);
    }

    if (path != NULL) gtk_tree_path_free (path); path = NULL;
}


static gboolean
gw_vocabularywindow_list_drag_drop_cb (
  GtkWidget *widget,
  GdkDragContext *context,
  gint x,
  gint y,
  guint time,
  gpointer user_data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GwVocabularyWindowClass *klass;
    GdkAtom target_type;
    GList *targets;
    GtkTreeView *view;
    GtkTreeModel *model;
    GtkTreeView *source;
    GtkTreePath *path;
    GtkTreeIter iter, previous_iter;
    GtkTreeIter position;
    GtkTreeSelection *selection;
    GtkTreeViewDropPosition drop_position;
    gint *indices;

    window = GW_VOCABULARYWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (widget), GW_TYPE_VOCABULARYWINDOW));
    if (window == NULL) return FALSE;
    priv = window->priv;
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    view = priv->list_treeview;
    model = gtk_tree_view_get_model (view);
    source = GTK_TREE_VIEW (gtk_drag_get_source_widget (context));
    targets = gdk_drag_context_list_targets (context);
    selection = gtk_tree_view_get_selection (priv->list_treeview);

    printf("drag drop %s\n", gtk_buildable_get_name (GTK_BUILDABLE (widget)));

    if (source == priv->list_treeview)
    {
      gtk_tree_view_get_dest_row_at_pos (view, x, y, &path, &drop_position);

      if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_BEFORE) 
        drop_position = GTK_TREE_VIEW_DROP_BEFORE;
      else if (drop_position == GTK_TREE_VIEW_DROP_INTO_OR_AFTER) 
        drop_position = GTK_TREE_VIEW_DROP_AFTER;

      if (path != NULL)
      {
        GList *source_link, *target_link;
        GtkTreePath *temp_path;

        gtk_tree_selection_get_selected (selection, &model, &iter);

        indices = gtk_tree_path_get_indices (path);
        target_link = g_list_nth (klass->item_models, *indices);

        temp_path = gtk_tree_model_get_path (model, &iter);
        indices = gtk_tree_path_get_indices (temp_path);
        source_link = g_list_nth (klass->item_models, *indices);
        gtk_tree_path_free (temp_path); temp_path = NULL;

        if (path != NULL && gtk_tree_model_get_iter (model, &position, path))
        {
          klass->item_models = g_list_remove_link (klass->item_models, source_link);
          klass->item_models = g_list_insert_before (klass->item_models, target_link, source_link->data);
          if (drop_position == GTK_TREE_VIEW_DROP_BEFORE) 
            gtk_list_store_move_before (GTK_LIST_STORE (model), &iter, &position);
          else if (drop_position == GTK_TREE_VIEW_DROP_AFTER) 
            gtk_list_store_move_after (GTK_LIST_STORE (model), &iter, &position);
        }

        g_list_free (source_link); source_link = NULL;
        gtk_tree_path_free (path); path = NULL;

        gtk_drag_finish (context, TRUE, FALSE, time);
      }
    }
    else if (source == priv->item_treeview)
    {
      gtk_tree_view_get_path_at_pos (priv->list_treeview, x, y, &path, NULL, NULL, NULL);

      if (path == NULL)
      {
        if (gtk_tree_model_get_iter_first (model, &iter))
        {
          previous_iter = iter;
          while (gtk_tree_model_iter_next (model, &iter)) previous_iter = iter;
          path = gtk_tree_model_get_path (model, &previous_iter);
        }
      }
      else
      {
        gtk_tree_path_prev (path);
      }

      if (gtk_tree_model_get_iter (model, &position, path))
      {
        gchar *text = gw_vocabularywindow_selected_words_to_string (window);
        indices = gtk_tree_path_get_indices (path);
        printf("%d %s\n", *indices, text);
        gw_vocabularywindow_append_text (window, *indices, text);
        g_free (text);
      }

      //Reset the selection
      gint index = g_list_index (klass->item_models, gtk_tree_view_get_model (priv->item_treeview));
      gtk_tree_model_get_iter_first (model, &iter);
      while (index-- && gtk_tree_model_iter_next (model, &iter)) ;
      gtk_tree_selection_select_iter (selection, &iter);

      gtk_drag_finish (context, TRUE, FALSE, time);
    }
    else
    {
      gtk_drag_finish (context, FALSE, FALSE, time);
    }

    gtk_tree_path_free (path);

    return TRUE;
}

/*
static void
gw_vocabularywindow_list_drag_data_recieved_cb (
  GtkWidget *widget,
  GdkDragContext *context,
  gint x,
  gint y,
  GtkSelectionData *data,
  guint info,
  guint time,
  gpointer user_data)
{
    gboolean success = TRUE;

    printf("drag data recieved %s\n", gtk_buildable_get_name (GTK_BUILDABLE (widget)));

    switch (info)
    {
      case TARGET_LIST_ROW_STRING:
        printf("LIST_ROW_STRING\n");
        break;
      case TARGET_WORD_ROW_STRING:
        printf("WORD_ROW_STRING\n");
        break;
    }

}
*/


static void
gw_vocabularywindow_list_drag_data_get_cb (
  GtkWidget *widget,
  GdkDragContext *context,
  GtkSelectionData *data,
  guint info,
  guint time,
  gpointer user_data)
{
    printf("drag data get %s\n", gtk_buildable_get_name (GTK_BUILDABLE (widget)));
    gchar *text = "FISH";
    gtk_selection_data_set_text (data, text, -1);
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

    gtk_drag_source_set (
        GTK_WIDGET (priv->list_treeview), 
        GDK_BUTTON1_MASK,
        list_row_source_targets,
        n_list_row_source_targets,
        GDK_ACTION_MOVE
    );

    gtk_drag_dest_set (
        GTK_WIDGET (priv->list_treeview),
        GTK_DEST_DEFAULT_ALL,
        list_row_dest_targets,
        n_list_row_dest_targets,
        GDK_ACTION_MOVE
    );
    /*
    g_signal_connect (
        G_OBJECT (priv->list_treeview), 
        "drag-data-received", 
        G_CALLBACK (gw_vocabularywindow_list_drag_data_recieved_cb), 
        NULL);
    */
    g_signal_connect (
        G_OBJECT (priv->list_treeview), 
        "drag-drop", 
        G_CALLBACK (gw_vocabularywindow_list_drag_drop_cb), 
        NULL);
    g_signal_connect (
        G_OBJECT (priv->list_treeview),
        "drag-data-get",
        G_CALLBACK (gw_vocabularywindow_list_drag_data_get_cb),
        NULL);
    g_signal_connect (
        G_OBJECT (priv->list_treeview),
        "drag-motion",
        G_CALLBACK (gw_vocabularywindow_list_drag_motion_cb),
        NULL);
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

    GtkEntry *entry = GTK_ENTRY (gw_window_get_object (GW_WINDOW (window), "vocabulary_search_entry"));
    gtk_tree_view_set_search_entry (priv->item_treeview, entry);

    gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);

    gtk_drag_source_set (
        GTK_WIDGET (priv->item_treeview), 
        GDK_BUTTON1_MASK,
        word_row_source_targets,
        n_word_row_source_targets,
        GDK_ACTION_MOVE
    );

    gtk_drag_dest_set (
        GTK_WIDGET (priv->item_treeview),
        GTK_DEST_DEFAULT_ALL,
        word_row_dest_targets,
        n_word_row_dest_targets,
        GDK_ACTION_MOVE
    );
}


void
gw_vocabularywindow_new_list (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GwVocabularyWindowClass *klass;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gchar *name;
    GwVocabularyModel *model;

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

    model = GW_VOCABULARYMODEL (gw_vocabularymodel_new (name));
    gw_vocabularymodel_set_has_changes (model, TRUE);
    klass->item_models = g_list_append (klass->item_models, model);

    g_free (name);

    gw_vocabularywindow_set_selected_vocabulary (window);
    gw_vocabularywindow_set_has_changes (window, TRUE);
}


void
gw_vocabularywindow_remove_list_by_index (GwVocabularyWindow *window, gint index)
{
    //Declarations
    GwVocabularyWindowClass *klass;
    GtkTreeModel *model;
    GList *link;
    GtkTreeIter iter;
    gint i;

    //Initializations
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));


    //Remove the list item
    model = GTK_TREE_MODEL (klass->list_model);
    if (gtk_tree_model_get_iter_first (model, &iter))
    {
      i = index;
      while (i > 0 && gtk_tree_model_iter_next (model, &iter)) i--;
      gtk_list_store_remove (GTK_LIST_STORE (model), &iter);
    }

    link = g_list_nth (klass->item_models, index);
    if (link != NULL)
    {
      model = GTK_TREE_MODEL (link->data);
      if (model != NULL)
        g_object_unref (G_OBJECT (model));
      klass->item_models = g_list_delete_link (klass->item_models, link);
    }
    gw_vocabularywindow_set_has_changes (window, TRUE);
}


void
gw_vocabularywindow_remove_selected_lists (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GList *pathlist;
    GList *iter;
    GtkTreePath *path;
    gint *indices;

    //Initializations
    priv = window->priv;
    model = gtk_tree_view_get_model (priv->list_treeview);
    selection = gtk_tree_view_get_selection (priv->list_treeview);
    pathlist = gtk_tree_selection_get_selected_rows (selection, &model);

    //Convert the tree paths to row references
    if (pathlist != NULL) {
      for (iter = g_list_last (pathlist); iter != NULL; iter = iter->prev)
      {
        path = (GtkTreePath*) iter->data;

        indices = gtk_tree_path_get_indices (path);
        if (indices != NULL) gw_vocabularywindow_remove_list_by_index (window, *indices);

        gtk_tree_path_free (path);
      }
      g_list_free (pathlist); pathlist = NULL;
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
    gw_vocabularywindow_set_has_changes (window, FALSE);
}


void
gw_vocabularywindow_reset (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowClass *klass;
    GwVocabularyWindowPrivate *priv;

    //Initializations
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    priv = window->priv;

    gw_vocabularywindow_class_reset_models (klass);
    gw_vocabularywindow_set_has_changes (window, FALSE);

    {
      //Set the initial selection
      GtkTreeSelection *selection;
      GtkTreeModel *model;
      GtkTreeIter iter;
      selection = gtk_tree_view_get_selection (priv->list_treeview);
      model = gtk_tree_view_get_model (priv->list_treeview);
      if (gtk_tree_model_get_iter_first (model, &iter))
      {
        gtk_tree_selection_select_iter (selection, &iter);
        gw_vocabularywindow_set_vocabulary_by_index (window, 0);
      }
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


//!
//! @brief Deletes lists that have no corresponding file
//!
void
gw_vocabularywindow_clean_lists (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowClass *klass;
    GList *iter;
    GwVocabularyModel *model;
    gint index;

    //Initializations
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    iter = klass->item_models;

    while (iter != NULL)
    {
      model = GW_VOCABULARYMODEL (iter->data);
      index = g_list_position (klass->item_models, iter);
      if (!gw_vocabularymodel_file_exists (model))
      {
        iter = iter->next;
        gw_vocabularywindow_remove_list_by_index (window, index);
      }
      else
      {
        iter = iter->next;
      }
    }
}


//!
//! @brief Deletes files that have to coresponding vocabulary list
//!
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


void
gw_vocabularywindow_set_has_changes (GwVocabularyWindow *window, gboolean has_changes)
{
   GtkWidget *widget;

   window->priv->has_changes = has_changes;

   widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "save_toolbutton"));
   gtk_widget_set_sensitive (widget, has_changes);

   widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "revert_toolbutton"));
   gtk_widget_set_sensitive (widget, has_changes);

   widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "save_menuitem"));
   gtk_widget_set_sensitive (widget, has_changes);

   widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "revert_menuitem"));
   gtk_widget_set_sensitive (widget, has_changes);
}


gboolean
gw_vocabularywindow_has_changes (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowClass *klass;
    gboolean has_changes;
    GList *iter;
    GwVocabularyModel *model;

    //Initializations
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    has_changes = FALSE;

    for (iter = klass->item_models; iter != NULL; iter = iter->next)
    {
      model = GW_VOCABULARYMODEL (iter->data);
      if (gw_vocabularymodel_has_changes (model))
      {
        has_changes = TRUE;
        break;
      }
    }

    return (window->priv->has_changes || has_changes); 
}


void
gw_vocabularywindow_remove_selected_words (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GtkTreeModel *model;
    GtkTreeSelection *selection;
    GList *list;
    GtkTreePath *path;
    GtkTreeIter treeiter;

    //Initializations
    priv = window->priv;
    model = gtk_tree_view_get_model (priv->item_treeview);
    selection = gtk_tree_view_get_selection (priv->item_treeview);

    if (gtk_tree_selection_count_selected_rows (selection))
    {
      while (gtk_tree_selection_count_selected_rows (selection))
      {
        list = gtk_tree_selection_get_selected_rows (selection, &model);
        path = list->data;

        gtk_tree_model_get_iter (model, &treeiter, path);
        gtk_list_store_remove (GTK_LIST_STORE (model), &treeiter);

        g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
        g_list_free (list); list = NULL;
      }
      gw_vocabularymodel_set_has_changes (GW_VOCABULARYMODEL (model), TRUE);
      gw_vocabularywindow_set_has_changes (window, TRUE);
    }
}


gchar*
gw_vocabularywindow_selected_words_to_string (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GtkTreeSelection *selection;
    GList *selectedlist;
    GList *link;
    GtkTreeIter iter;
    GtkTreePath *path;
    GtkTreeModel *model;
    gchar *kanji, *furigana, *definitions;
    gint i;
    gchar** atoms;
    gchar *text;

    //Initializations
    priv = window->priv;
    selection = gtk_tree_view_get_selection (priv->item_treeview);
    model = gtk_tree_view_get_model (priv->item_treeview);
    selectedlist = gtk_tree_selection_get_selected_rows (selection, &model);
    i = 0;
    atoms = g_new0 (gchar*, g_list_length (selectedlist) + 1);

    for (link = selectedlist; link != NULL; link = link->next)
    {
      path = (GtkTreePath*) link->data;
      if (gtk_tree_model_get_iter (model, &iter, path))
      {
        gtk_tree_model_get (model, &iter,
          GW_VOCABULARYMODEL_COLUMN_KANJI, &kanji,
          GW_VOCABULARYMODEL_COLUMN_FURIGANA, &furigana,
          GW_VOCABULARYMODEL_COLUMN_DEFINITIONS, &definitions,
        -1);
        atoms[i] = g_strjoin (";", kanji, furigana, definitions, NULL);
        g_free (kanji); g_free (furigana); g_free (definitions);
        i++;
      }
      gtk_tree_path_free (path);
    }
    g_list_free (selectedlist);

    text = g_strjoinv ("\n", atoms);
    g_strfreev (atoms); atoms = NULL;

    return text;
}


void
gw_vocabularywindow_append_text (GwVocabularyWindow *window, gint index, const gchar *text)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GwVocabularyWindowClass *klass;
    gchar **rows;
    gchar **atoms;
    gint i, j;
    GtkTreeIter iter;
    GtkTreeModel *model;
    gboolean modified;

    //Initializations
    priv = window->priv;
    klass = GW_VOCABULARYWINDOW_CLASS (G_OBJECT_GET_CLASS (window));
    if (index < 0)
      model = gtk_tree_view_get_model (priv->item_treeview);
    else
      model = GTK_TREE_MODEL (g_list_nth_data (klass->item_models, index));
    modified = FALSE;

    //Sanity check
    if (model == NULL) return;

    rows = g_strsplit (text, "\n", -1);
    if (rows != NULL)
    {
      for (i = 0; rows[i] != NULL; i++)
      {
        atoms = g_strsplit (rows[i], ";", 3);
        if (atoms != NULL)
        {
          gtk_list_store_append (GTK_LIST_STORE (model), &iter);
          for (j = 0; atoms[j] != NULL; j++)
            gtk_list_store_set (GTK_LIST_STORE (model), &iter, j, atoms[j], -1);
          g_strfreev (atoms); atoms = NULL;
          modified = TRUE;
        }
      }
      g_strfreev (rows);
    }

    if (modified)
    {
      gw_vocabularymodel_set_has_changes (GW_VOCABULARYMODEL (model), TRUE);
      gw_vocabularywindow_set_has_changes (window, TRUE);
    }
}

