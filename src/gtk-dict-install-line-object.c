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


struct GwUiDictInstallLine {
    GtkWidget *status_icon_check
    GtkWidget *status_icon_error
    GtkWidget *status_message
    GtkWidget *status_progressbar;
    GtkWidget *status_container;

    GtkWidget *button_cancel;
    GtkWidget *button_install;
    GtkWidget *button_remove;
    GtkWidget *button_container;

    GtkWidget *source_uri_entry;
    GtkWidget *source_browse;
    GtkWidget *source_reset;
    GtkWidget *source_container;

    GtkWidget *advanced_expander;
    GtkWidget *advanced_container;
};


//!
//! @brief Allocates and creates a new GwUiDictInstallLine object.
//!
//! @param di A GwDictInfo object to get information from
//!
GwUiDictInstallLine *gw_ui_new_dict_install_line (GwDictInfo *di)
{
    GwUiDictInstallLine *temp = NULL;
    GtkWidget *temp_hbox = NULL;

    if (temp = malloc(sizeof(GwUiDictInstallLine)) != NULL)
    {
      //Row 1 Column 1 Expander with name
      advanced_expander = gtk_expander_new (di->name);
      advanced_hbox = gtk_hbox_new (FALSE, 0);

      //Row 1 Column 2 Status messages area
      status_icon_check = gtk_image_new_from_stock (GTK_STOCK_APPLY, GTK_ICON_SIZE_SMALL_TOOLBAR);
      status_icon_error = gtk_image_new_from_stock (GTK_STOCK_DIALOG_ERROR, GTK_ICON_SIZE_SMALL_TOOLBAR);
      status_icon_message = gtk_label_new ("Installing...");
      status_progressbar = gtk_progressbar_new ();

      status_hbox = gtk_hbox_new (FALSE, 0);

      gtk_container_add (GTK_CONTAINER (status_hbox), status_progressbar);

      temp_hbox = gtk_hbox_new (FALSE, 0); //for vertical alignment
      gtk_container_add (GTK_CONTAINER (status_hbox), temp_hbox);
      gtk_container_add (GTK_CONTAINER (temp_hbox), status_icon_check);
      gtk_container_add (GTK_CONTAINER (temp_hbox), status_icon_error);

      temp_hbox = gtk_hbox_new (FALSE, 0); //for vertical alignment
      gtk_container_add (GTK_CONTAINER (status_hbox), temp_hbox); 
      gtk_container_add (GTK_CONTAINER (temp_hbox), status_icon_message);
      gtk_container_add (GTK_CONTAINER (temp_hbox), status_progressbar);

      //Row 1 Column 3 Button action area to do things to dictionaries
      action_cancel_button = gtk_button_new_from_stock (GTK_STOCK_CANCEL);
      action_install_button = gtk_button_new_from_stock (GTK_STOCK_ADD);
      action_remove_button = gtk_button_new_from_stock (GTK_STOCK_DELETE);

      button_hbox = gtk_hbox_new (FALSE, 0);
      gtk_container_add (GTK_CONTAINER (button_hbox), action_cancel_button);
      gtk_container_add (GTK_CONTAINER (button_hbox), action_install_button);
      gtk_container_add (GTK_CONTAINER (button_hbox), action_remove_button);

      //Row 2 Area for other less general options such as install URI selection
      source_uri_entry = gtk_entry_new ();
      source_browse_button = gtk_button_new_width_label (gettext("Browse..."));
      source_reset_button = gtk_button_new_width_label (gettext("Reset"));

      source_hbox = gtk_hbox_new (FALSE, 0);
      gtk_container_add (GTK_CONTAINER (source_hbox), source_uri_entry);
      gtk_container_add (GTK_CONTAINER (source_hbox), source_browse_button);
      gtk_container_add (GTK_CONTAINER (source_hbox), source_reset_button);

      return temp;
    }
    printf("Error creating dictionary install line gui object.");
    return NULL;
}

