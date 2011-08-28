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
//! @file src/gtk-radicals-callbacks.c
//!
//! @brief Abstraction layer for gtk callbacks 
//!
//! This file in specifically written for the radical search tool popup.
//!


#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


//!
//! @brief Brings up the search tool dialog
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_radicalswindow_show_cb (GtkWidget *widget, gpointer data)
{
}


//!
//! @brief Resets the states of all the buttons as if the dialog was just freshly opened
//!
//! @param widget Currently unused GtkWidget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_radicalswindow_clear_cb (GtkWidget *widget, gpointer data)
{
    //Declaratins
    GwRadicalsWindow *window;

    //Initializations
    window = GW_RADICALSWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    if (window == NULL) return;

    gw_app_block_searches (app);

    gw_radicalswindow_deselect_all_radicals (window);
    gtk_toggle_button_set_active (window->strokes_checkbutton, FALSE);

    gw_app_unblock_searches (app);
}


//!
//! @brief The function that does the grunt work of setting up a search using the window
//!
//! The function will get the data from the buttons to set up the query and the dictionary
//! with that to set up the searchitem. 
//!
//! @param widget Currently unused GtkWidget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_radicalswindow_search_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwRadicalsWindow *window;
    GwSearchWindow *searchwindow;
    LwDictInfo *di;
    char *text_query;
    char *text_radicals;
    char *text_strokes;
    GError *error;

    //Initializations
    window = GW_RADICALSWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    if (window == NULL) return;
    searchwindow = GW_SEARCHWINDOW (window->transient_for);
    di = lw_dictinfolist_get_dictinfo (LW_DICTINFOLIST (app->dictinfolist), LW_DICTTYPE_KANJI, "Kanji");
    if (di == NULL) return;

    text_radicals = gw_radicalswindow_strdup_all_selected (window);
    text_strokes = gw_radicalswindow_strdup_prefered_stroke_count (window);
    text_query = g_strdup_printf ("%s%s", text_radicals, text_strokes);
    error = NULL;

    //Sanity checks
    if (text_query != NULL && strlen(text_query) > 0)
    {
      gtk_entry_set_text (searchwindow->entry, "");
      gw_searchwindow_entry_insert (searchwindow, text_query);
      gw_searchwindow_select_all_by_target (searchwindow, LW_OUTPUTTARGET_ENTRY);
      gw_searchwindow_set_dictionary (searchwindow, di->load_position);

      gw_searchwindow_search_cb (GTK_WIDGET (searchwindow->toplevel), searchwindow->toplevel);
    }

    //Cleanup
    if (text_query != NULL) g_free (text_query);
    if (text_strokes != NULL) g_free (text_strokes);
    if (text_radicals != NULL) g_free (text_radicals);
}


//!
//! @brief Forces a search when the checkbox sensitivity is changed
//!
G_MODULE_EXPORT void gw_radicalswindow_strokes_checkbox_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    gboolean request;
    GwRadicalsWindow *window;

    //Initializations
    window = GW_RADICALSWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    if (window == NULL) return;
    request = gtk_toggle_button_get_active (window->strokes_checkbutton);

    gtk_widget_set_sensitive (GTK_WIDGET (window->strokes_spinbutton), request);

    gw_radicalswindow_search_cb (widget, data);
}


//!
//! @brief Closes the window passed throught the widget pointer
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_radicalswindow_close_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwRadicalsWindow *window;
    
    //Initializations
    window = GW_RADICALSWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));

    gw_app_destroy_window (app, GW_WINDOW (window));
}


