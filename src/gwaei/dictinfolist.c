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
//! @file dictinfo.c
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
GwDictInfoList* gw_dictinfolist_new (const int MAX, LwPrefManager *pm)
{
    GwDictInfoList *temp;
    int i;

    temp = (GwDictInfoList*) malloc(sizeof(GwDictInfoList));

    if (temp != NULL)
    {

      //Setup the model and view
      temp->model = gtk_list_store_new (
          TOTAL_GW_DICTINFOLIST_COLUMNS, 
          G_TYPE_STRING, 
          G_TYPE_STRING, 
          G_TYPE_STRING, 
          G_TYPE_STRING, 
          G_TYPE_STRING, 
          G_TYPE_STRING, 
          G_TYPE_POINTER);

      temp->list = NULL;
      temp->mutex = g_mutex_new();
      temp->max = MAX;
      for (i = 0; i < TOTAL_GW_DICTINFOLIST_SIGNALIDS; i++)
      {
        temp->signalids[i] = 0;
      }

      gw_dictinfolist_reload (temp, pm);

      temp->signalids[GW_DICTINFOLIST_SIGNALID_ROW_CHANGED] = g_signal_connect (
            G_OBJECT (temp->model),
            "row-deleted", 
            G_CALLBACK (gw_dictinfolist_list_store_row_changed_action_cb),
            NULL
      );
    }
    return temp;
}


void gw_dictinfolist_free (GwDictInfoList *dm)
{
    g_object_unref (dm->model);
    free (dm);
}


void gw_dictinfolist_rebuild_liststore (GwDictInfoList *dil)
{
    //Declarations
    LwDictInfo *di;
    GtkTreeIter tree_iter;
    char *iconname;
    char shortcutname[10];
    char ordernumber[10];
    char *favoriteicon;
    GList *list_iter;

    favoriteicon = "emblem-favorite";

    gtk_list_store_clear (GTK_LIST_STORE (dil->model));

    for (list_iter = dil->list; list_iter != NULL; list_iter = list_iter->next)
    {
      di = LW_DICTINFO (list_iter->data);
      if (di == NULL) continue;

      if (di->load_position == 0)
         iconname = favoriteicon;
      else
        iconname = NULL;
      if (di->load_position + 1 < 10)
        sprintf (shortcutname, "Alt-%d", (di->load_position + 1));
      else
        strcpy(shortcutname, "");
      if ((di->load_position + 1) < 1000)
        sprintf (ordernumber, "%d", (di->load_position + 1));
      else
        strcpy(ordernumber, "");

      gtk_list_store_append (GTK_LIST_STORE (dil->model), &tree_iter);
      gtk_list_store_set (
          dil->model, &tree_iter,
          GW_DICTINFOLIST_COLUMN_IMAGE,        iconname,
          GW_DICTINFOLIST_COLUMN_POSITION,     ordernumber,
          GW_DICTINFOLIST_COLUMN_NAME,         di->shortname,
          GW_DICTINFOLIST_COLUMN_LONG_NAME,    di->longname,
          GW_DICTINFOLIST_COLUMN_ENGINE,       lw_util_dicttype_to_string (di->type),
          GW_DICTINFOLIST_COLUMN_SHORTCUT,     shortcutname,
          GW_DICTINFOLIST_COLUMN_DICT_POINTER, di,
          -1
      );
    }
}


//!
//! Sets updates the list of dictionaries against the list in the global dictlist
//!
void gw_dictinfolist_reload (GwDictInfoList *dil, LwPrefManager *pm)
{
    lw_dictinfolist_reload (LW_DICTINFOLIST (dil));
    lw_dictinfolist_load_dictionary_order_from_pref (LW_DICTINFOLIST (dil), pm);

    if (dil->signalids[GW_DICTINFOLIST_SIGNALID_ROW_CHANGED] > 0)
      g_signal_handler_block (dil->model, dil->signalids[GW_DICTINFOLIST_SIGNALID_ROW_CHANGED]);

    gw_dictinfolist_rebuild_liststore (dil);

    if (dil->signalids[GW_DICTINFOLIST_SIGNALID_ROW_CHANGED] > 0)
      g_signal_handler_unblock (dil->model, dil->signalids[GW_DICTINFOLIST_SIGNALID_ROW_CHANGED]);
}


/*
G_MODULE_EXPORT void gw_dictinfolist_cursor_changed_cb (GtkTreeView *treeview, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *button = GTK_WIDGET (gtk_builder_get_object (builder, "remove_dictionary_button"));
    GtkTreeSelection *selection = gtk_tree_view_get_selection (_view);
    GtkTreeIter iter;

    GtkTreeModel *tmodel = GTK_TREE_MODEL (_model);
    gboolean has_selection = gtk_tree_selection_get_selected (selection, &tmodel, &iter);
    gtk_widget_set_sensitive (GTK_WIDGET (button), has_selection);
}
*/

