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
//! @file src/tabs.c
//!
//! @brief Abstraction layer for gtk callbacks
//!
//! Callbacks for activities initiated by the user. Most of the gtk code here
//! should still be abstracted to the interface C file when possible.
//!


#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gtk.h>
#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>

#include <gwaei/engine.h>
#include <gwaei/callbacks.h>
#include <gwaei/interface.h>
#include <gwaei/preferences.h>

G_MODULE_EXPORT void do_tab_remove (GtkWidget *widget, gpointer data);

int gw_tab_new ()
{
  //Create contents
  GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
  GtkWidget *scrolledwindow = GTK_WIDGET (gtk_scrolled_window_new (NULL, NULL));
  GtkWidget *textview = GTK_WIDGET (gtk_text_view_new ());
  gtk_container_add (GTK_CONTAINER (scrolledwindow), textview);
  gtk_widget_show_all (GTK_WIDGET (scrolledwindow));

  //Create create tab label
  GtkWidget *hbox = GTK_WIDGET (gtk_hbox_new(FALSE, 0));
  GtkWidget *label = GTK_WIDGET (gtk_label_new("CLOSE"));
  GtkWidget *close_button = GTK_WIDGET (gtk_button_new ());
  gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
  GtkWidget *button_image = GTK_WIDGET (gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU));
  gtk_container_add (GTK_CONTAINER (close_button), button_image);
  g_signal_connect( G_OBJECT (close_button), "clicked", G_CALLBACK (do_tab_remove), scrolledwindow);
  gtk_container_add (GTK_CONTAINER (hbox), label);
  gtk_container_add (GTK_CONTAINER (hbox), close_button);
  gtk_widget_show_all (GTK_WIDGET (hbox));

  //Finish
  int current = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
  int position = gtk_notebook_append_page (GTK_NOTEBOOK (notebook), scrolledwindow, hbox);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), position);
  gw_ui_initialize_tags();
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), current);

  return position;
}

G_MODULE_EXPORT void gw_tab_delete (int position)
{
}


//!
//! @brief Append a tag to the end of the tags
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void do_new_tab (GtkWidget *widget, gpointer data)
{
  int position = gw_tab_new ();
  GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
  gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), position);
}

//!
//! @brief Remove the tab where the close button is clicked
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void do_tab_remove (GtkWidget *widget, gpointer data)
{
  GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
  int page_num = gtk_notebook_page_num (GTK_WIDGET (notebook), GTK_WIDGET (data));
  gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), page_num);
}


//!
//! @brief Do the side actions required when a tab switch takes place
//!
//! Various side elements should be updated when at tab switch occurs
//! such as the progress bar, querybar, dictionry selection etc.
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void do_tab_switch (GtkWidget *wwidget, gpointer data)
{
}

