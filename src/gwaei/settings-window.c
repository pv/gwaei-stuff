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
//! @brief To be written
//!


#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


void _settingswindow_initialize_dictionary_tree_view (GtkTreeView*);
void _settingswindow_attach_signals (GwSettingsWindow*);
void _settingswindow_remove_signals (GwSettingsWindow*);

//!
//! @brief Sets the initial status of the dictionaries in the settings dialog
//!
GwSettingsWindow* gw_settingswindow_new (GwSearchWindow *transient_for, GList *link) 
{
    GwSettingsWindow *temp;

    temp = (GwSettingsWindow*) malloc(sizeof(GwSettingsWindow));

    if (temp != NULL)
    {
      gw_app_block_searches (app);

      gw_window_init (GW_WINDOW (temp), GW_WINDOW_SETTINGS, "settings.ui", "settings_window", link);
      gw_settingswindow_init (temp, GW_WINDOW (transient_for));
    }

    return temp;
}


//!
//! @brief Frees the memory used by the settings
//!
void gw_settingswindow_destroy (GwSettingsWindow *window)
{

    gw_settingswindow_deinit (window);
    gw_window_deinit (GW_WINDOW (window));

    free(window);

    if (lw_dictinfolist_get_total (LW_DICTINFOLIST (app->dictinfolist)) == 0)
      gw_app_quit (app);

    gw_app_unblock_searches (app);
}


void gw_settingswindow_init (GwSettingsWindow *window, GwWindow *transient_for)
{
    //Declarations
    GtkTreeView *view;

    //Initializations
    view = GTK_TREE_VIEW (gtk_builder_get_object (window->builder, "manage_dictionaries_treeview"));
    window->notebook = GTK_NOTEBOOK (gtk_builder_get_object (window->builder, "settings_notebook"));
    window->dictinstlist = NULL;

    _settingswindow_initialize_dictionary_tree_view (view);

    _settingswindow_attach_signals (window);

    if (lw_dictinfolist_get_total (LW_DICTINFOLIST (app->dictinfolist)) == 0)
      gtk_notebook_set_current_page (window->notebook, 1);

    gw_window_set_transient_for (GW_WINDOW (window), transient_for);

    gw_settingswindow_check_for_dictionaries (window);

    //We are going to lazily update the sensitivity of the spellcheck buttons only when the window is created
    GtkToggleButton *checkbox;
    gboolean enchant_exists;

    checkbox = GTK_TOGGLE_BUTTON (gtk_builder_get_object (window->builder, "query_spellcheck")); 
    enchant_exists = g_file_test (ENCHANT, G_FILE_TEST_IS_REGULAR);

    gtk_widget_set_sensitive (GTK_WIDGET (checkbox), enchant_exists);
}


void gw_settingswindow_deinit (GwSettingsWindow *window)
{
    gtk_widget_hide (GTK_WIDGET (window->toplevel));

    if (window->dictinstlist != NULL)
    {
      lw_dictinstlist_free (window->dictinstlist);
      window->dictinstlist = NULL;
    }

    _settingswindow_remove_signals (window);
}


void _settingswindow_attach_signals (GwSettingsWindow *window)
{
    //Declarations
    int i;

    for (i = 0; i < TOTAL_GW_SETTINGSWINDOW_SIGNALIDS; i++)
      window->signalids[i] = 0;

    window->signalids[GW_SETTINGSWINDOW_SIGNALID_ROMAJI_KANA] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_BASE,
        LW_KEY_ROMAN_KANA,
        gw_settingswindow_sync_romaji_kana_conv_cb,
        window
    );

    window->signalids[GW_SETTINGSWINDOW_SIGNALID_HIRA_KATA] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_BASE,
        LW_KEY_HIRA_KATA,
        gw_settingswindow_sync_hira_kata_conv_cb,
        window
    );

    window->signalids[GW_SETTINGSWINDOW_SIGNALID_KATA_HIRA] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_BASE,
        LW_KEY_KATA_HIRA,
        gw_settingswindow_sync_kata_hira_conv_cb,
        window
    );

    window->signalids[GW_SETTINGSWINDOW_SIGNALID_SPELLCHECK] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SPELLCHECK,
        gw_settingswindow_sync_spellcheck_cb,
        window
    );

    window->signalids[GW_SETTINGSWINDOW_SIGNALID_USE_GLOBAL_DOCUMENT_FONT] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_USE_GLOBAL_FONT,
        gw_settingswindow_sync_use_global_document_font_cb,
        window
    );

    window->signalids[GW_SETTINGSWINDOW_SIGNALID_GLOBAL_DOCUMENT_FONT] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_GNOME_INTERFACE,
        LW_KEY_DOCUMENT_FONT_NAME,
        gw_settingswindow_sync_global_document_font_cb,
        window
    );

    window->signalids[GW_SETTINGSWINDOW_SIGNALID_CUSTOM_FONT] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_CUSTOM_FONT,
        gw_settingswindow_sync_custom_font_cb,
        window
    );

    window->signalids[GW_SETTINGSWINDOW_SIGNALID_SEARCH_AS_YOU_TYPE] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SEARCH_AS_YOU_TYPE,
        gw_settingswindow_sync_search_as_you_type_cb,
        window
    );

    window->signalids[GW_SETTINGSWINDOW_SIGNALID_MATCH_FG] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_MATCH_FG,
        gw_settingswindow_sync_swatch_color_cb,
        gtk_builder_get_object (window->builder, "match_foreground")
    );

    window->signalids[GW_SETTINGSWINDOW_SIGNALID_MATCH_BG] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_MATCH_BG,
        gw_settingswindow_sync_swatch_color_cb,
        gtk_builder_get_object (window->builder, "match_background")
    );

    window->signalids[GW_SETTINGSWINDOW_SIGNALID_COMMENT_FG] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_COMMENT_FG,
        gw_settingswindow_sync_swatch_color_cb,
        gtk_builder_get_object (window->builder, "comment_foreground")
    );

    window->signalids[GW_SETTINGSWINDOW_SIGNALID_HEADER_FG] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_HEADER_FG,
        gw_settingswindow_sync_swatch_color_cb,
        gtk_builder_get_object (window->builder, "header_foreground")
    );

    window->signalids[GW_SETTINGSWINDOW_SIGNALID_HEADER_BG] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_HEADER_BG,
        gw_settingswindow_sync_swatch_color_cb,
        gtk_builder_get_object (window->builder, "header_background")
    );
}


void _settingswindow_remove_signals (GwSettingsWindow *window)
{
    //Declarations
    int i;

    lw_preferences_remove_change_listener_by_schema (
        app->preferences, 
        LW_SCHEMA_BASE, 
        window->signalids[GW_SETTINGSWINDOW_SIGNALID_ROMAJI_KANA]
    );

    lw_preferences_remove_change_listener_by_schema (
        app->preferences, 
        LW_SCHEMA_BASE, 
        window->signalids[GW_SETTINGSWINDOW_SIGNALID_HIRA_KATA]
    );

    lw_preferences_remove_change_listener_by_schema (
        app->preferences, 
        LW_SCHEMA_BASE, 
        window->signalids[GW_SETTINGSWINDOW_SIGNALID_KATA_HIRA]
    );

    lw_preferences_remove_change_listener_by_schema (
        app->preferences, 
        LW_SCHEMA_BASE, 
        window->signalids[GW_SETTINGSWINDOW_SIGNALID_SPELLCHECK]
    );

    lw_preferences_remove_change_listener_by_schema (
        app->preferences, 
        LW_SCHEMA_FONT, 
        window->signalids[GW_SETTINGSWINDOW_SIGNALID_USE_GLOBAL_DOCUMENT_FONT]
    );

    lw_preferences_remove_change_listener_by_schema (
        app->preferences, 
        LW_SCHEMA_GNOME_INTERFACE, 
        window->signalids[GW_SETTINGSWINDOW_SIGNALID_GLOBAL_DOCUMENT_FONT]
    );

    lw_preferences_remove_change_listener_by_schema (
        app->preferences, 
        LW_SCHEMA_FONT, 
        window->signalids[GW_SETTINGSWINDOW_SIGNALID_CUSTOM_FONT]
    );

    lw_preferences_remove_change_listener_by_schema (
        app->preferences, 
        LW_SCHEMA_BASE, 
        window->signalids[GW_SETTINGSWINDOW_SIGNALID_SEARCH_AS_YOU_TYPE]
    );

    lw_preferences_remove_change_listener_by_schema (
        app->preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        window->signalids[GW_SETTINGSWINDOW_SIGNALID_MATCH_FG]
    );

    lw_preferences_remove_change_listener_by_schema (
        app->preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        window->signalids[GW_SETTINGSWINDOW_SIGNALID_MATCH_BG]
    );

    lw_preferences_remove_change_listener_by_schema (
        app->preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        window->signalids[GW_SETTINGSWINDOW_SIGNALID_COMMENT_FG]
    );

    lw_preferences_remove_change_listener_by_schema (
        app->preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        window->signalids[GW_SETTINGSWINDOW_SIGNALID_HEADER_FG]
    );

    lw_preferences_remove_change_listener_by_schema (
        app->preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        window->signalids[GW_SETTINGSWINDOW_SIGNALID_HEADER_BG]
    );

    for (i = 0; i < TOTAL_GW_SETTINGSWINDOW_SIGNALIDS; i++)
      window->signalids[i] = 0;
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


void _settingswindow_initialize_dictionary_tree_view (GtkTreeView *view)
{
      //Declarations
      GtkCellRenderer *renderer;
      GtkTreeViewColumn *column;

      gtk_tree_view_set_model (GTK_TREE_VIEW (view), GTK_TREE_MODEL (app->dictinfolist->model));

      //Create the columns and renderer for each column
      renderer = gtk_cell_renderer_pixbuf_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new ();
      gtk_tree_view_column_set_title (column, " ");
      gtk_tree_view_column_pack_start (column, renderer, TRUE);
      gtk_tree_view_column_set_attributes (column, renderer, "icon-name", GW_DICTINFOLIST_COLUMN_IMAGE, NULL);
      gtk_tree_view_append_column (view, column);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new_with_attributes ("#", renderer, "text", GW_DICTINFOLIST_COLUMN_POSITION, NULL);
      gtk_tree_view_append_column (view, column);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new_with_attributes (gettext("Name"), renderer, "text", GW_DICTINFOLIST_COLUMN_LONG_NAME, NULL);
      gtk_tree_view_column_set_min_width (column, 100);
      gtk_tree_view_append_column (view, column);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new_with_attributes (gettext("Engine"), renderer, "text", GW_DICTINFOLIST_COLUMN_ENGINE, NULL);
      gtk_tree_view_append_column (view, column);

      renderer = gtk_cell_renderer_text_new();
      gtk_cell_renderer_set_padding (GTK_CELL_RENDERER (renderer), 6, 4);
      column = gtk_tree_view_column_new_with_attributes (gettext("Shortcut"), renderer, "text", GW_DICTINFOLIST_COLUMN_SHORTCUT, NULL);
      gtk_tree_view_append_column (view, column);
}


//!
//! @brief Disables portions of the interface depending on the currently queued jobs.
//!
void gw_settingswindow_check_for_dictionaries (GwSettingsWindow *window)
{
    //Declarations
    GtkWidget *message;

    //Initializations
    message = GTK_WIDGET (gtk_builder_get_object (window->builder, "please_install_dictionary_hbox"));

    //Set the show state of the dictionaries required message
    if (lw_dictinfolist_get_total (LW_DICTINFOLIST (app->dictinfolist)) > 0)
      gtk_widget_hide (message);
    else
      gtk_widget_show (message);
}

