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
//! @file src/settings-interface-gtk.c
//!
//! @brief Abstraction layer for gtk objects
//!
//! Used as a go between for functions interacting with GUI interface objects.
//! widgets.
//!


#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>
#include <gwaei/gtk.h>

#include <gwaei/engine.h>
#include <gwaei/callbacks.h>
#include <gwaei/interface.h>
#include <gwaei/gtk-dict-install-line-object.h>


//!
//! @brief Disables portions of the interface depending on the currently queued jobs.
//!
void gw_ui_update_settings_interface()
{
  //Set the install interface
  GtkWidget *close_button;
  close_button = GTK_WIDGET (gtk_builder_get_object (builder, "settings_close_button"));

  GtkWidget *advanced_tab;
  advanced_tab = GTK_WIDGET (gtk_builder_get_object (builder, "advanced_tab"));

  GtkWidget *organize_dictionaries_tab;
  organize_dictionaries_tab = GTK_WIDGET (gtk_builder_get_object (builder, "organize_dictionaries_tab"));

  if (gw_dictlist_get_total_with_status (GW_DICT_STATUS_UPDATING  ) > 0 ||
      gw_dictlist_get_total_with_status (GW_DICT_STATUS_INSTALLING) > 0 ||
      gw_dictlist_get_total_with_status (GW_DICT_STATUS_REBUILDING) > 0   )
  {
    gtk_widget_set_sensitive (close_button,   FALSE);
    gtk_widget_set_sensitive (advanced_tab,  FALSE);
    gtk_widget_set_sensitive (organize_dictionaries_tab,  FALSE);
  }
  else
  {
    gtk_widget_set_sensitive (close_button,   TRUE );
    gtk_widget_set_sensitive (advanced_tab,  TRUE);
    gtk_widget_set_sensitive (organize_dictionaries_tab,  TRUE);
  }


  GtkWidget *message;
  message = GTK_WIDGET (gtk_builder_get_object (builder, "please_install_dictionary_hbox"));
  if (gw_dictlist_get_total_with_status (GW_DICT_STATUS_INSTALLED  ) > 0)
  {
    gtk_widget_hide (message);
  }
  else
  {
    gtk_widget_show (message);
  }

}


//!
//! @brief Sets the text in the source gtkentry for the appropriate dictionary
//!
//! @param widget Pointer to a GtkEntry to set the text of
//! @param value The constant string to use as the source for the text
//!
void gw_ui_set_dictionary_source (GtkWidget *widget, const char* value)
{
    if (widget != NULL && value != NULL)
    {
      g_signal_handlers_block_by_func (GTK_WIDGET (widget), do_source_entry_changed_action, NULL);
      gtk_entry_set_text (GTK_ENTRY (widget), value);
      g_signal_handlers_unblock_by_func (GTK_WIDGET (widget), do_source_entry_changed_action, NULL);
    }
}


//!
//! @brief A progressbar update function made specifially to be used with curl when downloading
//!
//! @param data A GwUiDictInstallLine to use as a base for updating the interface
//! @param dltotal the amount to be downloaded
//! @param dlnow the current amount downloaded
//! @param ultotal The upload total (unused)
//! @param ulnow The current amount upload (unused)
//!
int gw_ui_update_progressbar (void   *data,
                              double  dltotal,
                              double  dlnow,
                              double  ultotal,
                              double  ulnow   )
{
    gdk_threads_enter();

    GwUiDictInstallLine *il = (GwUiDictInstallLine*) data;
    GtkWidget *progressbar = il->status_progressbar;

    if (il->di->status != GW_DICT_STATUS_CANCELING) {
      if (dlnow == 0.0) {
        gw_ui_dict_install_line_progress_bar_set_fraction (il, 0.0);
      }
      else {
        double ratio = dlnow / dltotal;
        gw_ui_dict_install_line_progress_bar_set_fraction (il, ratio);
      }

      gdk_threads_leave ();
      return FALSE;
    }

    gdk_threads_leave ();
    return TRUE;
}


//!
//! @brief Sets the initial status of the dictionaries in the settings dialog
//!
void gw_settings_initialize_installed_dictionary_list () 
{
    GtkWidget *table;
    GwDictInfo *di;
    GwUiDictInstallLine *il;

    table = GTK_WIDGET (gtk_builder_get_object (builder, "dictionaries_table"));

    di =  gw_dictlist_get_dictionary_by_name ("English");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);

    di =  gw_dictlist_get_dictionary_by_name ("Kanji");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);

    di =  gw_dictlist_get_dictionary_by_name ("Names");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);
/*
    di =  gw_dictlist_get_dictionary_by_name ("Radicals");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);
*/
    di =  gw_dictlist_get_dictionary_by_name ("Examples");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);

    table = GTK_WIDGET (gtk_builder_get_object (builder, "other_dictionaries_table"));

    di =  gw_dictlist_get_dictionary_by_name ("French");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);

    di =  gw_dictlist_get_dictionary_by_name ("German");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);

    di =  gw_dictlist_get_dictionary_by_name ("Spanish");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);

    gw_ui_update_settings_interface ();
}


//!
//! @brief Updates the dictionary orders for the dictionary order tab
//!
void gw_ui_initialize_dictionary_order_list ()
{
      #define XPADDING 4
      #define YPADDING 0

      GtkWidget *viewport = GTK_WIDGET (gtk_builder_get_object (builder, "organize_dictionaries_viewport"));
      if (gtk_bin_get_child (GTK_BIN (viewport)) != NULL) return;
      GtkListStore *list_store = GTK_LIST_STORE (gtk_builder_get_object (builder, "list_store_dictionaries"));
      GtkWidget *treeview = GTK_WIDGET (gtk_tree_view_new ());

      GtkWidget *down_button = GTK_WIDGET (gtk_builder_get_object (builder, "move_dictionary_down_button"));
      GtkWidget *up_button = GTK_WIDGET (gtk_builder_get_object (builder, "move_dictionary_up_button"));
      g_signal_connect(G_OBJECT (down_button), "clicked", G_CALLBACK (do_move_dictionary_down), (gpointer) treeview);
      g_signal_connect(G_OBJECT (up_button), "clicked", G_CALLBACK (do_move_dictionary_up), (gpointer) treeview);


      //gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeview), FALSE);
      gtk_tree_view_set_reorderable (GTK_TREE_VIEW (treeview), TRUE);
      gtk_tree_view_set_model (GTK_TREE_VIEW (treeview), GTK_TREE_MODEL (list_store));

      gtk_container_add (GTK_CONTAINER (viewport), GTK_WIDGET (treeview));
      gtk_widget_show_all (GTK_WIDGET (treeview));

      GtkCellRenderer *renderer;
      GtkTreeViewColumn *column;

      renderer = gtk_cell_renderer_pixbuf_new ();
      gtk_cell_renderer_set_padding (renderer, YPADDING, XPADDING);
      column = gtk_tree_view_column_new_with_attributes (NULL, renderer, "icon-name", 1, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

      renderer = gtk_cell_renderer_text_new ();
      gtk_cell_renderer_set_padding (renderer, YPADDING, XPADDING);
      column = gtk_tree_view_column_new_with_attributes (gettext("Order"), renderer, "text", 2, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

      renderer = gtk_cell_renderer_text_new ();
      gtk_cell_renderer_set_padding (renderer, YPADDING, XPADDING);
      gtk_cell_renderer_set_sensitive (renderer, FALSE);
      column = gtk_tree_view_column_new_with_attributes (gettext("Shortcut"), renderer, "text", 3, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

      renderer = gtk_cell_renderer_text_new ();
      gtk_cell_renderer_set_padding (renderer, YPADDING, XPADDING);
      column = gtk_tree_view_column_new_with_attributes (gettext("Dictionary Name"), renderer, "text", 0, NULL);
      gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column);

      #undef XPADDING
      #undef YPADDING
}


void gw_ui_update_dictionary_order_list ()
{
  printf("Updating distiary order list\n");
      GtkListStore *list_store = GTK_LIST_STORE (gtk_builder_get_object (builder, "list_store_dictionaries"));

      GtkTreeIter iter;
      if (gtk_tree_model_get_iter_first (GTK_TREE_MODEL (list_store), &iter) == FALSE) return;
      char *icon_name = NULL;
      char *favorite = "emblem-favorite";
      char *order = NULL;
      char *shortcut = NULL;
      gboolean can_continue = TRUE;

      int i = 1;

      while (i < 10 && can_continue)
      {
        if (i == 1) icon_name = favorite;
        else icon_name = NULL;
        order = g_strdup_printf ("%d", i);
        shortcut = g_strdup_printf ("Alt-%d", i);
        gtk_list_store_set (GTK_LIST_STORE (list_store), &iter, 1, icon_name, 2, order, 3, shortcut, -1); 
        g_free (order);
        g_free (shortcut);
        i++;
        can_continue = gtk_tree_model_iter_next (GTK_TREE_MODEL (list_store), &iter);
      }
      while (can_continue)
      {
        icon_name = NULL;
        order = g_strdup_printf ("%d", i);
        shortcut = NULL;
        gtk_list_store_set (GTK_LIST_STORE (list_store), &iter, 1, icon_name, 2, order, 3, shortcut, -1); 
        g_free (order);
        i++;
        can_continue = gtk_tree_model_iter_next (GTK_TREE_MODEL (list_store), &iter);
      }
}

void gw_ui_set_use_global_document_font_checkbox (gboolean setting)
{
    GtkWidget *checkbox = GTK_WIDGET (gtk_builder_get_object (builder, "system_font_checkbox"));
    GtkWidget *child_settings = GTK_WIDGET (gtk_builder_get_object (builder, "system_document_font_hbox"));

    g_signal_handlers_block_by_func (GTK_WIDGET (checkbox), do_toggle_use_global_document_font, NULL);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), setting);
    gtk_widget_set_sensitive (child_settings, !setting);
    g_signal_handlers_unblock_by_func (GTK_WIDGET (checkbox), do_toggle_use_global_document_font, NULL);
}


void gw_ui_update_global_font_label (char *font_description_string)
{
    GtkWidget *checkbox = GTK_WIDGET (gtk_builder_get_object (builder, "system_font_checkbox"));
    char *text = g_strdup_printf (gettext("_Use the System Document Font (%s)"), font_description_string);

    if (text != NULL)
    {
      gtk_button_set_label (GTK_BUTTON (checkbox), text);
      g_free (text);
      text = NULL;
    }
}


void gw_ui_update_custom_font_button (char *font_description_string)
{
    GtkWidget *button = GTK_WIDGET (gtk_builder_get_object (builder, "custom_font_fontbutton"));
    g_signal_handlers_block_by_func (GTK_WIDGET (button), do_set_custom_document_font, NULL);
    gtk_font_button_set_font_name (GTK_FONT_BUTTON (button), font_description_string);
    g_signal_handlers_unblock_by_func (GTK_WIDGET (button), do_set_custom_document_font, NULL);
}

