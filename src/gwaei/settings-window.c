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
//! @file settings-window.c
//!
//! @brief Abstraction layer for gtk objects
//!
//! Used as a go between for functions interacting with GUI interface objects.
//! widgets.
//!


#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


//!
//! @brief Sets the initial status of the dictionaries in the settings dialog
//!
GwSettingsWindow* gw_settingswindow_new () 
{
    GwSettingsWindow *temp;
    GtkWidget *notebook;

    temp = (GwSettingsWindow*) malloc(sizeof(GwSettingsWindow));

    if (temp != NULL)
    {
      gw_app_block_searches (app);

      temp->builder = gtk_builder_new ();
      gw_window_load_ui_xml (GW_WINDOW (temp), "settings.ui");
      temp->toplevel = GTK_WINDOW (gtk_builder_get_object (temp->builder, "settings_window"));
      temp->type = GW_WINDOW_SETTINGS;

      if (g_list_length (app->dictinfolist->list) == 0)
        gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), 1);
    }

    return temp;
}


//!
//! @brief Frees the memory used by the settings
//!
void gw_settingswindow_destroy (GwSettingsWindow *window)
{
    gtk_widget_destroy (GTK_WIDGET (window->toplevel));
    g_object_unref (window->builder);
    free(window);
    gw_app_block_searches (app);
}


//!
//! @brief Sets the text in the source gtkentry for the appropriate dictionary
//!
//! @param widget Pointer to a GtkEntry to set the text of
//! @param value The constant string to use as the source for the text
//!
void gw_settings_set_dictionary_source (GtkWidget *widget, const char* value)
{
    if (widget != NULL && value != NULL)
    {
      gtk_entry_set_text (GTK_ENTRY (widget), value);
    }
}


