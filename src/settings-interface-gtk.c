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
//! @brief Sets teh initial status of the dictionaries in the settings dialog
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

    di =  gw_dictlist_get_dictionary_by_name ("Radicals");
    il = gw_ui_new_dict_install_line (di);
    gw_ui_add_dict_install_line_to_table (GTK_TABLE (table), il);

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
void gw_ui_update_dictionary_orders ()
{
    GtkWidget *container;
    container = GTK_WIDGET (gtk_builder_get_object (builder, "organize_dictionary_list_hbox"));
    if (container == NULL)
      return;


    //Parse the the names of the dictionary list
    char order[5000];
    gw_pref_get_string (order, GCKEY_GW_LOAD_ORDER, GW_LOAD_ORDER_FALLBACK, 5000);
    char *long_name_list[50];
    char **condensed_name_list[50];
    long_name_list[0] = order;
    int i = 0;
    int j = 0;
    while ((long_name_list[i + 1] = g_utf8_strchr (long_name_list[i], -1, L',')) && i < 50)
    {
      i++;
      *long_name_list[i] = '\0';
      long_name_list[i]++;
    }
    long_name_list[i + 1] = NULL;

    //Get the condensed version
    i = 0;
    j = 0;
    GwDictInfo *di1, *di2;
    while (long_name_list[i] != NULL && long_name_list[j] != NULL)
    {
      di1 = gw_dictlist_get_dictionary_by_name (long_name_list[j]);
      di2 = gw_dictlist_get_dictionary_by_alias (long_name_list[j]);
      if (di1 != NULL && di2 != NULL && strcmp(di1->name, di2->name) == 0 && di2->status == GW_DICT_STATUS_INSTALLED)
      {
        condensed_name_list[i] = &long_name_list[j];
        i++; j++;
      }
      else
      {
        j++;
      }
        
    }
    condensed_name_list[i] = NULL;


    //Declarations
    GtkWidget *label, *dictionary, *move_up_button;
    GtkWidget *move_down_button, *button_box, *button_image;
    GtkWidget *number_label, *eventbox, *quickkey;
    GtkWidget *icon_image;
    GList *list;
    move_down_button = NULL;

    //Clear out old buttons
    list = gtk_container_get_children (GTK_CONTAINER (container));
    while (list != NULL)
    {
      gtk_widget_destroy (GTK_WIDGET(list->data));
      list = gtk_container_get_children (GTK_CONTAINER (container));
    }

    //Add new buttons
    i = 0;
    char *markup;
    GwDictInfo *di;
    while (condensed_name_list[i] != NULL)
    {
      eventbox = gtk_event_box_new();
      if (i == 0)
        icon_image = gtk_image_new_from_icon_name ("emblem-favorite", GTK_ICON_SIZE_MENU);
      else
        icon_image = gtk_image_new_from_stock (GTK_STOCK_YES, GTK_ICON_SIZE_MENU);

      number_label = GTK_WIDGET (gtk_label_new (NULL));
      markup = g_markup_printf_escaped ("<span weight=\"bold\">%d</span>", i + 1);
      gtk_label_set_markup (GTK_LABEL (number_label), markup);
      g_free (markup);

      label = GTK_WIDGET (gtk_label_new (NULL));
      if (di = gw_dictlist_get_dictionary_by_name (*condensed_name_list[i]))
        markup = g_markup_printf_escaped ("<span size=\"larger\">%s</span>", di->long_name);
      else
        markup = g_markup_printf_escaped ("<span size=\"larger\">%s</span>", *condensed_name_list[i]);
      gtk_label_set_markup (GTK_LABEL (label), markup);
      g_free (markup);

      quickkey = GTK_WIDGET (gtk_label_new (NULL));
      if (i < 9)
        markup = g_markup_printf_escaped ("<span size=\"smaller\">Alt-%d</span>", i + 1);
      else
        markup = g_markup_printf_escaped ("<span size=\"smaller\">   </span>", i + 1);
      gtk_label_set_markup (GTK_LABEL (quickkey), markup);
      gtk_widget_set_sensitive (GTK_WIDGET (quickkey), FALSE);
      g_free (markup);

      dictionary = GTK_WIDGET (gtk_hbox_new (FALSE, 5));
      gtk_container_set_border_width (GTK_CONTAINER (dictionary), 10);
      button_box = GTK_WIDGET (gtk_hbox_new (TRUE, 5));

      move_up_button = GTK_WIDGET (gtk_button_new ());
      gtk_button_set_relief (GTK_BUTTON (move_up_button), GTK_RELIEF_NONE);
      button_image = GTK_WIDGET (gtk_image_new_from_stock (GTK_STOCK_GO_UP, GTK_ICON_SIZE_MENU));
      gtk_container_add (GTK_CONTAINER (move_up_button), button_image);
      if (i == 0) gtk_widget_set_sensitive (move_up_button, FALSE);
      g_signal_connect( G_OBJECT (move_up_button),       "clicked",
                        G_CALLBACK (do_move_dictionary_up), GINT_TO_POINTER(i));

      move_down_button = GTK_WIDGET (gtk_button_new ());
      gtk_button_set_relief (GTK_BUTTON (move_down_button), GTK_RELIEF_NONE);
      button_image = GTK_WIDGET (gtk_image_new_from_stock (GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_MENU));
      gtk_container_add (GTK_CONTAINER (move_down_button), button_image);
      g_signal_connect( G_OBJECT (move_down_button),       "clicked",
                        G_CALLBACK (do_move_dictionary_down), GINT_TO_POINTER(i));
      
      gtk_box_pack_start (GTK_BOX (button_box), move_up_button, FALSE, FALSE, 0);
      gtk_box_pack_start (GTK_BOX (button_box), move_down_button, FALSE, FALSE, 0);
      gtk_box_pack_end (GTK_BOX (dictionary), button_box, FALSE, FALSE, 5);

      gtk_box_pack_start (GTK_BOX (dictionary), icon_image, FALSE, FALSE, 5);
      gtk_box_pack_start (GTK_BOX (dictionary), number_label, FALSE, FALSE, 5);
      GtkWidget *temp = GTK_WIDGET (gtk_hbox_new (FALSE, 5));
      gtk_box_pack_start (GTK_BOX (temp), label, FALSE, FALSE, 5);
      gtk_box_pack_start (GTK_BOX (temp), quickkey, FALSE, FALSE, 5);
      gtk_box_pack_start (GTK_BOX (dictionary), temp, TRUE, TRUE, 5);

      if (i % 2)
      {
        gtk_container_add (GTK_CONTAINER (eventbox), dictionary);
        gtk_box_pack_start (GTK_BOX (container), eventbox, FALSE,FALSE, 0);
        gtk_widget_show_all (eventbox);
      }
      else
      {
        gtk_box_pack_start (GTK_BOX (container), dictionary, FALSE, FALSE, 0);
        gtk_widget_show_all (dictionary);
      }
      i++;
    }

    //Hide the last move down arrow
    if (move_down_button != NULL) gtk_widget_set_sensitive (move_down_button, FALSE);

    //Force the container to resize
    gtk_widget_hide (container);
    gtk_widget_show (container);
}
