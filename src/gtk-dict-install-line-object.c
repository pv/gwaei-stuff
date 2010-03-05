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
//! @file src/gtk-dict-install-line-object.c
//!
//! @brief Abstraction layer for gtk object
//!
//! A congromeration of gtk widgets to form a gwaei install line object for easy
//! maintenance.
//!

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <locale.h>
#include <libintl.h>
#include <regex.h>

#include <gtk/gtk.h>

#include <gwaei/gtk.h>
#include <gwaei/definitions.h>
#include <gwaei/utilities.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>

#include <gwaei/engine.h>
#include <gwaei/printing.h>
#include <gwaei/interface.h>


//!
//! @brief Allocates and creates a new GwUiDictInstallLine object.
//!
//! @param di A GwDictInfo object to get information from
//!
GwUiDictInstallLine *gw_ui_new_dict_install_line (GwDictInfo *di)
{
    GwUiDictInstallLine *temp = NULL;
    GtkWidget *temp_hbox = NULL;

    if ((temp = (GwUiDictInstallLine*)malloc(sizeof(GwUiDictInstallLine))) != NULL)
    {
      //Row 1 Column 1 Expander with name
      temp->advanced_expander = gtk_expander_new (di->name);
      temp->advanced_hbox = gtk_hbox_new (FALSE, 0);

      //Row 1 Column 2 Status messages area
      temp->status_icon_check = gtk_image_new_from_stock (GTK_STOCK_APPLY, GTK_ICON_SIZE_SMALL_TOOLBAR);
      temp->status_icon_error = gtk_image_new_from_stock (GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_SMALL_TOOLBAR);
      temp->status_message = gtk_label_new ("Installing...");
      temp->status_progressbar = gtk_progress_bar_new ();

      temp->status_hbox = gtk_hbox_new (FALSE, 0);

      gtk_container_add (GTK_CONTAINER (temp->status_hbox), temp->status_progressbar);

      temp_hbox = gtk_hbox_new (FALSE, 0); //for vertical alignment
      gtk_container_add (GTK_CONTAINER (temp->status_hbox), temp_hbox);
      gtk_container_add (GTK_CONTAINER (temp_hbox), temp->status_icon_check);
      gtk_container_add (GTK_CONTAINER (temp_hbox), temp->status_icon_error);

      temp_hbox = gtk_hbox_new (FALSE, 0); //for vertical alignment
      gtk_container_add (GTK_CONTAINER (temp->status_hbox), temp_hbox); 
      gtk_container_add (GTK_CONTAINER (temp_hbox), temp->status_message);
      gtk_container_add (GTK_CONTAINER (temp_hbox), temp->status_progressbar);

      //Row 1 Column 3 Button action area to do things to dictionaries
      temp->action_cancel_button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
      temp->action_install_button = gtk_button_new_from_stock (GTK_STOCK_ADD);
      temp->action_remove_button = gtk_button_new_from_stock (GTK_STOCK_DELETE);

      temp->action_button_hbox = gtk_hbox_new (FALSE, 0);
      gtk_container_add (GTK_CONTAINER (temp->action_button_hbox), temp->action_cancel_button);
      gtk_container_add (GTK_CONTAINER (temp->action_button_hbox), temp->action_install_button);
      gtk_container_add (GTK_CONTAINER (temp->action_button_hbox), temp->action_remove_button);

      //Row 2 Area for other less general options such as install URI selection
      temp->source_uri_entry = gtk_entry_new ();
      temp->source_browse_button = gtk_button_new_with_label (gettext("Browse..."));
      temp->source_reset_button = gtk_button_new_with_label (gettext("Reset"));

      temp->source_hbox = gtk_hbox_new (FALSE, 0);
      gtk_container_add (GTK_CONTAINER (temp->source_hbox), temp->source_uri_entry);
      gtk_container_add (GTK_CONTAINER (temp->source_hbox), temp->source_browse_button);
      gtk_container_add (GTK_CONTAINER (temp->source_hbox), temp->source_reset_button);

      return temp;
    }
    printf("Error creating dictionary install line gui object.");
    return NULL;
}


void gw_ui_add_dict_install_line_to_table (GtkTable *table, GwUiDictInstallLine *il)
{
  if (table->ncols < 3) table->ncols = 3;

  int row = table->nrows;
  table->nrows++;

  gtk_table_attach_defaults (table, il->advanced_hbox, 0, 0, row, row);
  gtk_table_attach_defaults (table, il->status_hbox, 1, 1, row, row);
  gtk_table_attach_defaults (table, il->action_button_hbox, 2, 2, row, row);

  table->rows++;
  row++;
  gtk_table_attach_defaults (table, il->action_button_hbox, 1, 2, row, row);
}


void gw_ui_destroy_dict_install_line (GwUiDictInstallLine *il)
{
    gtk_widget_destroy (il->status_hbox);
    il->status_hbox = NULL;
    il->status_icon_check = NULL;
    il->status_icon_error = NULL;
    il->status_message = NULL;
    il->status_progressbar = NULL;

    gtk_widget_destroy (il->action_button_hbox);
    il->action_button_hbox = NULL;
    il->action_cancel_button = NULL;
    il->action_install_button = NULL;
    il->action_remove_button = NULL;

    gtk_widget_destroy (il->source_hbox);
    il->source_uri_entry = NULL;
    il->source_browse_button = NULL;
    il->source_reset_button = NULL;

    gtk_widget_destroy (il->advanced_hbox);
    il->advanced_hbox = NULL;
    il->advanced_expander = NULL;
}
