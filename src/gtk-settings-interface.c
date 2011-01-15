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
//! @file src/gtk-settings-interface.c
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

#include <gwaei/engine.h>

#include <gwaei/gtk.h>
#include <gwaei/gtk-settings-callbacks.h>
#include <gwaei/gtk-settings-interface.h>
#include <gwaei/preferences.h>


//!
//! @brief Disables portions of the interface depending on the currently queued jobs.
//!
G_MODULE_EXPORT void gw_ui_update_settings_interface ()
{
    //Set the install interface
    GtkWidget *close_button;
    close_button = GTK_WIDGET (gtk_builder_get_object (builder, "settings_close_button"));

    GtkWidget *advanced_tab;
    advanced_tab = GTK_WIDGET (gtk_builder_get_object (builder, "advanced_tab"));

    if (gw_dictlist_get_total_with_status (GW_DICT_STATUS_UPDATING  ) > 0 ||
        gw_dictlist_get_total_with_status (GW_DICT_STATUS_INSTALLING) > 0 ||
        gw_dictlist_get_total_with_status (GW_DICT_STATUS_REBUILDING) > 0   )
    {
      gtk_widget_set_sensitive (close_button,   FALSE);
      gtk_widget_set_sensitive (advanced_tab,  FALSE);
    }
    else
    {
      gtk_widget_set_sensitive (close_button,   TRUE );
      gtk_widget_set_sensitive (advanced_tab,  TRUE);
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

/*
THIS CODE BREAKS ON WINDOWS
printf("MABREAK8\n");
    char order[5000];
    gw_pref_get_string (order, GW_SCHEMA_DICTIONARY, GW_KEY_LOAD_ORDER, 5000);
printf("MABREAK9\n");
    gtk_widget_set_sensitive (reset_order_button, (strcmp (order, GW_LOAD_ORDER_DEFAULT) != 0));
printf("MABREAK10\n");
*/
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
      gtk_entry_set_text (GTK_ENTRY (widget), value);
    }
}


//!
//! @brief A progressbar update function made specifially to be used with curl when downloading
//!
//! @param message
//! @param percent
//! @param data
//!
int gw_ui_update_progressbar (char *message, int percent, gpointer data)
{
/*
    gdk_threads_enter();

    GwUiDictInstallLine *il = (GwUiDictInstallLine*) data;
    GtkWidget *progressbar = il->status_progressbar;

    if (il->di->status != GW_DICT_STATUS_CANCELING)
    {
      if (percent < 0)
      {
        gw_ui_dict_install_set_message (il, NULL, message);
      }
      else if (percent == 0)
      {
      }
      else
      {
        gdouble ratio = ((gdouble) percent) / 100.0;
        gw_ui_progressbar_set_fraction_by_install_line (il, ratio);
      }
      gdk_threads_leave ();
      return FALSE;
    }

    gdk_threads_leave ();
*/
    return TRUE;
}


//!
//! @brief Sets the initial status of the dictionaries in the settings dialog
//!
void gw_settings_initialize () 
{
    gw_dictionary_manager_init ();
    gw_ui_update_settings_interface ();
}


void gw_ui_set_use_global_document_font_checkbox (gboolean setting)
{
    GtkWidget *checkbox = GTK_WIDGET (gtk_builder_get_object (builder, "system_font_checkbox"));
    GtkWidget *child_settings = GTK_WIDGET (gtk_builder_get_object (builder, "system_document_font_hbox"));

    g_signal_handlers_block_by_func (checkbox, do_toggle_use_global_document_font, NULL);

    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), setting);
    gtk_widget_set_sensitive (child_settings, !setting);

    g_signal_handlers_unblock_by_func (checkbox, do_toggle_use_global_document_font, NULL);
}


void gw_ui_update_global_font_label (const char *font_description_string)
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


void gw_ui_update_custom_font_button (const char *font_description_string)
{
    GtkWidget *button = GTK_WIDGET (gtk_builder_get_object (builder, "custom_font_fontbutton"));
    gtk_font_button_set_font_name (GTK_FONT_BUTTON (button), font_description_string);
}

