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

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/vocabularywindow-private.h>


typedef enum { 
  GW_VOCABULARYLIST_COLUMN_NAME,
  TOTAL_GW_VOCABULARYLIST_COLUMNS
} GwVocabularyListColumn;


typedef enum { 
  GW_VOCABULARYITEM_COLUMN_KANJI,
  GW_VOCABULARYITEM_COLUMN_FURIGANA,
  GW_VOCABULARYITEM_COLUMN_DEFINITIONS,
  TOTAL_GW_VOCABULARYITEM_COLUMNS
} GwVocabularyItemColumn;


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
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;

    window = GW_VOCABULARYWINDOW (object);
    priv = window->priv;

    G_OBJECT_CLASS (gw_vocabularywindow_parent_class)->finalize (object);
}


static void 
gw_vocabularywindow_constructed (GObject *object)
{
    //Declarations
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;
    GtkToolButton *toolbutton;
    GtkWidget *widget;
    GtkAccelGroup *accelgroup;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_vocabularywindow_parent_class)->constructed (object);
    }
    //Initializations
    window = GW_VOCABULARYWINDOW (object);
    priv = window->priv;

    //Set up the gtkbuilder links
    priv->toolbar =  GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "toolbar"));
    priv->list_treeview = GTK_TREE_VIEW (gw_window_get_object (GW_WINDOW (window), "vocabulary_list_treeview"));
    priv->list_toolbar =  GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "vocabulary_list_toolbar"));
    priv->item_treeview = GTK_TREE_VIEW (gw_window_get_object (GW_WINDOW (window), "vocabulary_item_treeview"));
    priv->item_toolbar =  GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "vocabulary_item_toolbar"));

    //Set up the gtk window
    gtk_window_set_title (GTK_WINDOW (window), gettext("gWaei Vocabulary Manager"));
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
    gtk_window_set_default_size (GTK_WINDOW (window), 620, 500);
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

  g_type_class_add_private (object_class, sizeof (GwVocabularyWindowPrivate));
}


static void 
gw_vocabularywindow_attach_signals (GwVocabularyWindow *window)
{
    //Declarations
//    GwApplication *application;
    GwVocabularyWindowPrivate *priv;

//    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;

    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (gw_vocabularywindow_remove_signals), NULL);
}


static void 
gw_vocabularywindow_remove_signals (GwVocabularyWindow *window)
{
    //Declarations
    GwApplication *application;
    GwVocabularyWindowPrivate *priv;
    GSource *source;
    gint i;

    application = gw_window_get_application (GW_WINDOW (window));
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

    //Initializations
    priv = window->priv;

    //Primary Toolbar
    context = gtk_widget_get_style_context (GTK_WIDGET (priv->toolbar));
    gtk_style_context_add_class (context, GTK_STYLE_CLASS_PRIMARY_TOOLBAR);
    gtk_widget_reset_style (GTK_WIDGET (priv->toolbar));

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
    gtk_style_context_set_junction_sides (context, GTK_JUNCTION_TOP);
    gtk_widget_reset_style (GTK_WIDGET (priv->item_toolbar));
}


static void
gw_vocabularywindow_init_list_treeview (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;

    priv = window->priv;

    //Set up the columns
    column = gtk_tree_view_column_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_set_title (column, gettext("List"));
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_set_attributes (column, renderer, "text", GW_VOCABULARYLIST_COLUMN_NAME, NULL);
    gtk_tree_view_append_column (priv->list_treeview, column);

    //Set up the model
    if (priv->list_model != NULL) g_object_unref (priv->list_model);
    priv->list_model = gtk_list_store_new (1, G_TYPE_STRING);
    gtk_tree_view_set_model (priv->list_treeview, GTK_TREE_MODEL (priv->list_model));

    //Temp code remove later
    GtkTreeIter iter;
    gtk_list_store_append (priv->list_model, &iter);
    gtk_list_store_set (priv->list_model, &iter, GW_VOCABULARYLIST_COLUMN_NAME, "My Vocab List1", -1);
    gtk_list_store_append (priv->list_model, &iter);
    gtk_list_store_set (priv->list_model, &iter, GW_VOCABULARYLIST_COLUMN_NAME, "My Vocab List2", -1);
}


static void
gw_vocabularywindow_init_item_treeview (GwVocabularyWindow *window)
{
    //Declarations
    GwVocabularyWindowPrivate *priv;
    GtkTreeViewColumn *column;
    GtkCellRenderer *renderer;

    priv = window->priv;

    //Set up the columns
    column = gtk_tree_view_column_new ();
    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_set_title (column, gettext("Vocabulary"));
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_set_attributes (column, renderer, "text", GW_VOCABULARYITEM_COLUMN_KANJI, NULL);
    gtk_tree_view_append_column (priv->item_treeview, column);

    if (priv->item_model != NULL) g_object_unref (priv->item_model);
    priv->item_model = gtk_list_store_new (1, G_TYPE_STRING);
    gtk_tree_view_set_model (priv->item_treeview, GTK_TREE_MODEL (priv->item_model));

    //Temp code remove later
    GtkTreeIter iter;
    gtk_list_store_append (priv->item_model, &iter);
    gtk_list_store_set (priv->item_model, &iter, GW_VOCABULARYITEM_COLUMN_KANJI, "サカナ", -1);
    gtk_list_store_append (priv->item_model, &iter);
    gtk_list_store_set (priv->item_model, &iter, GW_VOCABULARYITEM_COLUMN_KANJI, "日本語", -1);
}
