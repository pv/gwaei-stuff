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
//! @file dictionarymanager.c
//!
//! @brief Unwritten
//!

#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


//!
//! @brief Sets up the dictionary manager.  This is the backbone of every portion of the GUI that allows editing dictionaries
//!
GwDictionaryManager* gw_dictionarymanager_new ()
{
    GwDictionaryManager *temp;

    temp = (GwDictionaryManager*) malloc(sizeof(GwDictionaryManager));

    if (temp != NULL)
    {

      //Setup the model and view
      temp->model = gtk_list_store_new (
          TOTAL_GW_DICTIONARYMANAGER_COLUMNS, 
          G_TYPE_STRING, 
          G_TYPE_STRING, 
          G_TYPE_STRING, 
          G_TYPE_STRING, 
          G_TYPE_STRING, 
          G_TYPE_STRING, 
          G_TYPE_POINTER);

/*
      GtkCellRenderer *renderer;
      GtkTreeViewColumn *column;
      _view = GTK_TREE_VIEW (gtk_builder_get_object (builder, "manage_dictionaries_treeview"));
      gtk_tree_view_set_model (GTK_TREE_VIEW (_view), GTK_TREE_MODEL (_model));

      //Create the columns and renderer for each column
      renderer = gtk_cell_renderer_pixbuf_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new ();
      gtk_tree_view_column_set_title (column, " ");
      gtk_tree_view_column_pack_start (column, renderer, TRUE);
      gtk_tree_view_column_set_attributes (column, renderer, "icon-name", IMAGE, NULL);
      gtk_tree_view_append_column (_view, column);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new_with_attributes ("#", renderer, "text", POSITION, NULL);
      gtk_tree_view_append_column (_view, column);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new_with_attributes (gettext("Name"), renderer, "text", LONG_NAME, NULL);
      gtk_tree_view_column_set_min_width (column, 100);
      gtk_tree_view_append_column (_view, column);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new_with_attributes (gettext("Engine"), renderer, "text", ENGINE, NULL);
      gtk_tree_view_append_column (_view, column);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new_with_attributes (gettext("Shortcut"), renderer, "text", SHORTCUT, NULL);
      gtk_tree_view_append_column (_view, column);

      GtkWidget *combobox = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_combobox"));
      gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (_model));
      renderer = gtk_cell_renderer_text_new();
      gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, FALSE);
      gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", LONG_NAME);

*/
      gw_dictionarymanager_reload (temp);
      temp->signalids[GW_DICTIONARYMANAGER_SIGNALID_ROW_CHANGED] = g_signal_connect (
            G_OBJECT (temp->model),
            "row-deleted", 
            G_CALLBACK (gw_dictionarymanager_list_store_row_changed_action_cb),
            NULL
      );
    }
    return temp;
}


void gw_dictionarymanager_free (GwDictionaryManager *dm)
{
    g_object_unref (dm->model);
    free (dm);
}


//!
//! Sets updates the list of dictionaries against the list in the global dictlist
//!
void gw_dictionarymanager_reload (GwDictionaryManager *dm)
{
    //Declarations
    LwDictInfo *di;
    GtkTreeIter treeiter;
    char *iconname;
    char *shortcutname;
    char *ordernumber;
    char *favoriteicon;
    GList *iter;

    if (dm->signalids[GW_DICTIONARYMANAGER_SIGNALID_ROW_CHANGED] > 0)
      g_signal_handler_block (dm->model, dm->signalids[GW_DICTIONARYMANAGER_SIGNALID_ROW_CHANGED]);

    gtk_list_store_clear (GTK_LIST_STORE (dm->model));

/*
    GtkMenuShell *shell = GTK_MENU_SHELL (gtk_builder_get_object (builder, "dictionary_popup"));
    if (shell != NULL)
    {
      GList     *children = NULL;
      children = gtk_container_get_children (GTK_CONTAINER (shell));
      while (children != NULL )
      {
        gtk_widget_destroy(children->data);
        children = g_list_delete_link (children, children);
      }
    }
*/

    for (iter = dm->list; iter != NULL; iter = iter->next)
    {
      di = (LwDictInfo*) iter->data;
      favoriteicon = "emblem-favorite";
      if (di->load_position == 0)
         iconname = favoriteicon;
      else
        iconname = NULL;
      if (di->load_position < 9)
        shortcutname = g_strdup_printf ("Alt-%d", (di->load_position + 1));
      else
        shortcutname = NULL;
      ordernumber = g_strdup_printf ("%d", (di->load_position + 1));

      gtk_list_store_append (GTK_LIST_STORE (dm->model), &treeiter);
      gtk_list_store_set (
            dm->model, &treeiter,
            GW_DICTIONARYMANAGER_COLUMN_IMAGE, iconname,
            GW_DICTIONARYMANAGER_COLUMN_POSITION, ordernumber,
            GW_DICTIONARYMANAGER_COLUMN_NAME, di->shortname,
            GW_DICTIONARYMANAGER_COLUMN_LONG_NAME, di->longname,
            GW_DICTIONARYMANAGER_COLUMN_ENGINE, lw_util_get_engine_name (di->engine),
            GW_DICTIONARYMANAGER_COLUMN_SHORTCUT, shortcutname,
            GW_DICTIONARYMANAGER_COLUMN_DICT_POINTER, di,
      -1);
      //Cleanup
      if (ordernumber != NULL) g_free (ordernumber);
      if (shortcutname != NULL) g_free (shortcutname);
    }

/*
    GtkAccelGroup* accel_group = GTK_ACCEL_GROUP (gtk_builder_get_object (builder, "main_accelgroup"));
    GSList* group = NULL;

    for (iter = dm->list; iter != NULL; iter = iter->next)
    {
      //Refill the menu
      item = GTK_WIDGET (gtk_radio_menu_item_new_with_label (group, di->longname));
      group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (item));
      gtk_menu_shell_append (GTK_MENU_SHELL (shell),  GTK_WIDGET (item));
      if (di->load_position == 0) gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
      g_signal_connect(G_OBJECT (item), "toggled", G_CALLBACK (gw_main_dictionary_changed_action_cb), NULL);
      if (di->load_position < 9) gtk_widget_add_accelerator (GTK_WIDGET (item), "activate", accel_group, (GDK_KEY_0 + di->load_position + 1), GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
      gtk_widget_show (item);
    }

    //Fill in the other menu items
    item = GTK_WIDGET (gtk_separator_menu_item_new());
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (item));
    gtk_widget_show (GTK_WIDGET (item));

    item = GTK_WIDGET (gtk_menu_item_new_with_mnemonic(gettext("_Cycle Up")));
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (item));
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (gw_main_cycle_dictionaries_backward_cb), NULL);
    gtk_widget_add_accelerator (GTK_WIDGET (item), "activate", accel_group, GDK_KEY_Up, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_show (GTK_WIDGET (item));

    item = GTK_WIDGET (gtk_menu_item_new_with_mnemonic(gettext("Cycle _Down")));
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (item));
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (gw_main_cycle_dictionaries_forward_cb), NULL);
    gtk_widget_add_accelerator (GTK_WIDGET (item), "activate", accel_group, GDK_KEY_Down, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_show (GTK_WIDGET (item));

    GtkWidget *combobox = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_combobox"));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);
*/

    if (dm->signalids[GW_DICTIONARYMANAGER_SIGNALID_ROW_CHANGED] > 0)
      g_signal_handler_unblock (dm->model, dm->signalids[GW_DICTIONARYMANAGER_SIGNALID_ROW_CHANGED]);
}


/*
G_MODULE_EXPORT void gw_dictionarymanager_cursor_changed_cb (GtkTreeView *treeview, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *button = GTK_WIDGET (gtk_builder_get_object (builder, "remove_dictionary_button"));
    GtkTreeSelection *selection = gtk_tree_view_get_selection (_view);
    GtkTreeIter iter;

    GtkTreeModel *tmodel = GTK_TREE_MODEL (_model);
    gboolean has_selection = gtk_tree_selection_get_selected (selection, &tmodel, &iter);
    gtk_widget_set_sensitive (GTK_WIDGET (button), has_selection);
}


G_MODULE_EXPORT void gw_dictionarymanager_remove_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkBuilder *builder;
    GtkWidget *button;
    GtkTreePath *path;
    GtkTreeIter iter;
    GList *list;
    GError *error;
    GtkTreeSelection *selection;
    GtkTreeModel *tmodel;
    gboolean has_selection;
    gint* indices;
    LwDictInfo *di;

    //Initializations
    builder = gw_common_get_builder ();
    button = GTK_WIDGET (gtk_builder_get_object (builder, "remove_dictionary_button"));
    selection = gtk_tree_view_get_selection (_view);
    tmodel = GTK_TREE_MODEL (_model);
    has_selection = gtk_tree_selection_get_selected (selection, &tmodel, &iter);
    error = NULL;

    //Sanity check
    if (!has_selection) return;

    path = gtk_tree_model_get_path (GTK_TREE_MODEL (_model), &iter);
    indices = gtk_tree_path_get_indices (path);
    list = lw_dictinfolist_get_dict_by_load_position (*indices);

    if (list != NULL)
    {
      di = list->data;
      lw_dictinfo_uninstall (di, NULL, &error);
      gw_dictionarymanager_update_items ();
    }

    //Cleanup
    gtk_tree_path_free (path);

    gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);
}
*/

LwDictInfo* gw_dictionarymanager_get_selected_dictinfo (GwDictionaryManager *dm)
{
    return dm->selected;
}


//!
//! @brief Sets a dictionary in the dictionary manager
//!
LwDictInfo* gw_dictionarymanager_set_selected_by_load_position (GwDictionaryManager *dm, int index)
{
    LwDictInfo *di;

    di = g_list_nth_data (dm->list, index);

    if (di != NULL)
    {
      dm->selected = di;
    }

    return di;
}

