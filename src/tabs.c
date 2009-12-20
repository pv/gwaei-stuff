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
#include <stdlib.h>
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
#include <gwaei/callbacks.h>
#include <gwaei/tabs.h>

G_MODULE_EXPORT void do_tab_remove (GtkWidget *widget, gpointer data);


void gw_tab_update_appearance ()
{
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    int pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
    int current_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    GtkWidget *scrolledwindow = GTK_WIDGET (gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), current_page));

    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (notebook), (pages > 1));
}


void gw_guarantee_first_tab ()
{
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    int pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
    if (pages == 0)
    {
      gw_tab_new ();
      gw_tab_update_appearance ();
    }
}


void gw_tab_set_current_tab_text (const char* string)
{
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    int page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    GtkWidget *container = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num);
    GtkWidget *hbox = GTK_WIDGET (gtk_notebook_get_tab_label(GTK_NOTEBOOK (notebook), GTK_WIDGET (container)));
    GList *children = gtk_container_get_children (GTK_CONTAINER (hbox));
    GtkWidget *label = GTK_WIDGET (children->data);
    gtk_label_set_text (GTK_LABEL (label), string);
}


int gw_tab_new ()
{
    //Create contents
    GtkWidget *scrolledwindow = GTK_WIDGET (gtk_scrolled_window_new (NULL, NULL));
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    GtkWidget *textview = GTK_WIDGET (gtk_text_view_new ());
    GObject *textbuffer = G_OBJECT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview)));
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (textview), 10);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (textview), 10);
    gtk_text_view_set_cursor_visible (GTK_TEXT_VIEW (textview), FALSE);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (textview), FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_SHADOW_IN);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (textview), GTK_WRAP_WORD);

    g_signal_connect( G_OBJECT (textview), "drag_motion", G_CALLBACK (do_drag_motion_1), NULL);
    g_signal_connect( G_OBJECT (textview), "focus_in_event", G_CALLBACK (do_update_clipboard_on_focus_change), textview);
    g_signal_connect( G_OBJECT (textview), "button_press_event", G_CALLBACK (do_get_position_for_button_press), NULL);
    g_signal_connect( G_OBJECT (textview), "motion_notify_event", G_CALLBACK (do_get_iter_for_motion), NULL);
    g_signal_connect( G_OBJECT (textview), "drag_drop", G_CALLBACK (do_drag_drop_1), NULL);
    g_signal_connect( G_OBJECT (textview), "button_release_event", G_CALLBACK (do_get_iter_for_button_release), NULL);
    g_signal_connect( G_OBJECT (textview), "drag_leave", G_CALLBACK (do_drag_leave_1), NULL);
    g_signal_connect( G_OBJECT (textview), "drag_data_received", G_CALLBACK (do_search_drag_data_recieved), NULL);
    g_signal_connect( G_OBJECT (textview), "key_press_event", G_CALLBACK (do_focus_change_on_key_press), NULL);
    g_signal_connect( G_OBJECT (textview), "event_after", G_CALLBACK (do_update_icons_for_selection), NULL);

    gtk_container_add (GTK_CONTAINER (scrolledwindow), textview);
    gtk_widget_show_all (GTK_WIDGET (scrolledwindow));

    //Create create tab label
    GtkWidget *hbox = GTK_WIDGET (gtk_hbox_new(FALSE, 0));
    GtkWidget *label = GTK_WIDGET (gtk_label_new(gettext("(Empty)")));
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
    gw_tab_update_appearance ();
    gw_ui_initialize_buffer_marks(textbuffer);
    gw_tab_searchitems = g_list_append(gw_tab_searchitems, NULL);
    gw_ui_update_toolbar_buttons();

    return position;
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
    gw_ui_grab_focus_by_target (GW_TARGET_ENTRY);
    gw_ui_set_dictionary(0);
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
    int pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
    if (pages < 2) return;
    int page_num = gtk_notebook_page_num (GTK_NOTEBOOK (notebook), GTK_WIDGET (data));
    gw_ui_cancel_search_by_tab_number (page_num);
    gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), page_num);

    GList *listitem = g_list_nth (gw_tab_searchitems, page_num);
    gw_tab_searchitems = g_list_delete_link (gw_tab_searchitems, listitem);

    gw_tab_update_appearance ();
}


//!
//! @brief Remove the tab where the close button is clicked
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void do_tab_remove_current (GtkWidget *widget, gpointer data)
{
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    int pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
    if (pages < 2) exit(EXIT_SUCCESS);
    int page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    gw_ui_cancel_search_by_tab_number (page_num);
    gtk_notebook_remove_page (GTK_NOTEBOOK (notebook), page_num);

    GList *listitem = g_list_nth (gw_tab_searchitems, page_num);
    gw_tab_searchitems = g_list_delete_link (gw_tab_searchitems, listitem);

    gw_tab_update_appearance ();
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
G_MODULE_EXPORT void do_tab_switch (GtkNotebook *notebook, GtkNotebookPage *page, int page_num, gpointer data)
{
    GwSearchItem *item = g_list_nth_data(gw_tab_searchitems, page_num);

    gw_ui_set_total_results_label_by_searchitem (item);
    gw_ui_set_search_progressbar_by_searchitem (item);
    gw_ui_set_query_entry_text_by_searchitem (item);
    gw_ui_set_main_window_title_by_searchitem (item);
    gw_ui_set_dictionary_by_searchitem (item);
}


G_MODULE_EXPORT void do_next_tab (GtkWidget *widget, gpointer data)
{
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    gtk_notebook_next_page (GTK_NOTEBOOK (notebook));
}


G_MODULE_EXPORT void do_previous_tab (GtkWidget *widget, gpointer data)
{
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    gtk_notebook_prev_page (GTK_NOTEBOOK (notebook));
}


