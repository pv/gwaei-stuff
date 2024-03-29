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
//! @file settingswindow.c
//!
//! @brief To be written
//!


#include <string.h>
#include <stdlib.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/settingswindow-private.h>


static void gw_settingswindow_initialize_dictionary_tree_view (GwSettingsWindow*, GtkTreeView*);
static void gw_settingswindow_attach_signals (GwSettingsWindow*);
static void gw_settingswindow_remove_signals (GwSettingsWindow*);

G_DEFINE_TYPE (GwSettingsWindow, gw_settingswindow, GW_TYPE_WINDOW)

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* 
gw_settingswindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwSettingsWindow *window;

    //Initializations
    window = GW_SETTINGSWINDOW (g_object_new (GW_TYPE_SETTINGSWINDOW,
                                            "type",        GTK_WINDOW_TOPLEVEL,
                                            "application", GW_APPLICATION (application),
                                            "ui-xml",      "settingswindow.ui",
                                            NULL));

    return GTK_WINDOW (window);
}


static void 
gw_settingswindow_init (GwSettingsWindow *window)
{
    window->priv = GW_SETTINGSWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwSettingsWindowPrivate));
}


static void 
gw_settingswindow_finalize (GObject *object)
{
    GwSettingsWindow *window;
    GwApplication *application;

    window = GW_SETTINGSWINDOW (object);
    application = gw_window_get_application (GW_WINDOW (window));

    if (g_main_current_source () != NULL) gw_application_unblock_searches (application);

    G_OBJECT_CLASS (gw_settingswindow_parent_class)->finalize (object);
}


static void 
gw_settingswindow_constructed (GObject *object)
{
    //Declarations
    GwSettingsWindow *window;
    GwSettingsWindowPrivate *priv;
    GwApplication *application;
    GtkTreeView *view;
    GwDictInfoList *dictinfolist;
    GtkAccelGroup *accelgroup;
    GtkWidget *widget;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_settingswindow_parent_class)->constructed (object);
    }

    //Initializations
    window = GW_SETTINGSWINDOW (object);
    priv = window->priv;
    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));
    application = gw_window_get_application (GW_WINDOW (window));
    dictinfolist = gw_application_get_dictinfolist (application);
    view = GTK_TREE_VIEW (gw_window_get_object (GW_WINDOW (window), "manage_dictionaries_treeview"));

    gtk_window_set_title (GTK_WINDOW (window), gettext("gWaei Settings"));
    gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ON_PARENT);

    g_signal_connect (G_OBJECT (window), "destroy",
                      G_CALLBACK (gw_settingswindow_remove_signals), NULL);

    if (g_main_current_source () != NULL) gw_application_block_searches (application);

    priv->notebook = GTK_NOTEBOOK (gw_window_get_object (GW_WINDOW (window), "settings_notebook"));

    gw_settingswindow_initialize_dictionary_tree_view (window, view);

    if (lw_dictinfolist_get_total (LW_DICTINFOLIST (dictinfolist)) == 0)
      gtk_notebook_set_current_page (priv->notebook, 1);
    gw_settingswindow_check_for_dictionaries (window);

    //We are going to lazily update the sensitivity of the spellcheck buttons only when the window is created
    GtkToggleButton *checkbox;
    gboolean enchant_exists;

    checkbox = GTK_TOGGLE_BUTTON (gw_window_get_object (GW_WINDOW (window), "query_spellcheck")); 
    enchant_exists = g_file_test (ENCHANT, G_FILE_TEST_IS_REGULAR);

    gtk_widget_set_sensitive (GTK_WIDGET (checkbox), enchant_exists);

    gw_settingswindow_attach_signals (window);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "settings_close_button"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_W), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_Escape), 0, GTK_ACCEL_VISIBLE);
}


static void
gw_settingswindow_class_init (GwSettingsWindowClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gw_settingswindow_constructed;
  object_class->finalize = gw_settingswindow_finalize;

  g_type_class_add_private (object_class, sizeof (GwSettingsWindowPrivate));
}


static void 
gw_settingswindow_attach_signals (GwSettingsWindow *window)
{
    //Declarations
    GwSettingsWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    int i;

    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);

    for (i = 0; i < TOTAL_GW_SETTINGSWINDOW_SIGNALIDS; i++)
      priv->signalids[i] = 0;

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_ROMAJI_KANA] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_ROMAN_KANA,
        gw_settingswindow_sync_romaji_kana_conv_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_HIRA_KATA] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_HIRA_KATA,
        gw_settingswindow_sync_hira_kata_conv_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_KATA_HIRA] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_KATA_HIRA,
        gw_settingswindow_sync_kata_hira_conv_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_SPELLCHECK] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SPELLCHECK,
        gw_settingswindow_sync_spellcheck_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_USE_GLOBAL_DOCUMENT_FONT] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_USE_GLOBAL_FONT,
        gw_settingswindow_sync_use_global_document_font_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_GLOBAL_DOCUMENT_FONT] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_GNOME_INTERFACE,
        LW_KEY_DOCUMENT_FONT_NAME,
        gw_settingswindow_sync_global_document_font_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_CUSTOM_FONT] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_CUSTOM_FONT,
        gw_settingswindow_sync_custom_font_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_SEARCH_AS_YOU_TYPE] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SEARCH_AS_YOU_TYPE,
        gw_settingswindow_sync_search_as_you_type_cb,
        window
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_MATCH_FG] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_MATCH_FG,
        gw_settingswindow_sync_swatch_color_cb,
        gw_window_get_object (GW_WINDOW (window), "match_foreground")
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_MATCH_BG] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_MATCH_BG,
        gw_settingswindow_sync_swatch_color_cb,
        gw_window_get_object (GW_WINDOW (window), "match_background")
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_COMMENT_FG] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_COMMENT_FG,
        gw_settingswindow_sync_swatch_color_cb,
        gw_window_get_object (GW_WINDOW (window), "comment_foreground")
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_HEADER_FG] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_HEADER_FG,
        gw_settingswindow_sync_swatch_color_cb,
        gw_window_get_object (GW_WINDOW (window), "header_foreground")
    );

    priv->signalids[GW_SETTINGSWINDOW_SIGNALID_HEADER_BG] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_HIGHLIGHT,
        LW_KEY_HEADER_BG,
        gw_settingswindow_sync_swatch_color_cb,
        gw_window_get_object (GW_WINDOW (window), "header_background")
    );
}


static void 
gw_settingswindow_remove_signals (GwSettingsWindow *window)
{
    //Declarations
    GwSettingsWindowPrivate *priv;
    GwApplication *application;
    LwPreferences *preferences;
    int i;

    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    preferences = gw_application_get_preferences (application);

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_BASE, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_ROMAJI_KANA]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_BASE, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_HIRA_KATA]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_BASE, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_KATA_HIRA]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_BASE, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_SPELLCHECK]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_FONT, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_USE_GLOBAL_DOCUMENT_FONT]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_GNOME_INTERFACE, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_GLOBAL_DOCUMENT_FONT]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_FONT, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_CUSTOM_FONT]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_BASE, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_SEARCH_AS_YOU_TYPE]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_MATCH_FG]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_MATCH_BG]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_COMMENT_FG]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_HEADER_FG]
    );

    lw_preferences_remove_change_listener_by_schema (
        preferences, 
        LW_SCHEMA_HIGHLIGHT, 
        priv->signalids[GW_SETTINGSWINDOW_SIGNALID_HEADER_BG]
    );

    for (i = 0; i < TOTAL_GW_SETTINGSWINDOW_SIGNALIDS; i++)
      priv->signalids[i] = 0;
}


//!
//! @brief Sets the text in the source gtkentry for the appropriate dictionary
//!
//! @param widget Pointer to a GtkEntry to set the text of
//! @param value The constant string to use as the source for the text
//!
void 
gw_settings_set_dictionary_source (GtkWidget *widget, const char* value)
{
    if (widget != NULL && value != NULL)
    {
      gtk_entry_set_text (GTK_ENTRY (widget), value);
    }
}


static void 
gw_settingswindow_initialize_dictionary_tree_view (GwSettingsWindow *window, GtkTreeView *view)
{
      //Declarations
      GwApplication *application;
      GwDictInfoList *dictinfolist;
      GtkCellRenderer *renderer;
      GtkTreeViewColumn *column;

      application = gw_window_get_application (GW_WINDOW (window));
      dictinfolist = gw_application_get_dictinfolist (application);

      gtk_tree_view_set_model (GTK_TREE_VIEW (view), GTK_TREE_MODEL (dictinfolist->model));

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
void 
gw_settingswindow_check_for_dictionaries (GwSettingsWindow *window)
{
    //Declarations
    GwApplication *application;
    GwDictInfoList *dictinfolist;
    GtkWidget *message;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    dictinfolist = gw_application_get_dictinfolist (application);
    message = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "please_install_dictionary_hbox"));

    //Set the show state of the dictionaries required message
    if (lw_dictinfolist_get_total (LW_DICTINFOLIST (dictinfolist)) > 0)
      gtk_widget_hide (message);
    else
      gtk_widget_show (message);
}

