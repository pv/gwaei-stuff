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
//! @file src/gtk-settings-callbacks.c
//!
//! @brief Abstraction layer for gtk callbacks
//! Callbacks for activities initiated by the user. Most of the gtk code here
//! should still be abstracted to the interface C file when possible.
//!


#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


//!
//! @brief Callback to toggle the hiragana-katakana conversion setting for the search entry
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settingswindow_hira_kata_conv_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    gboolean request;

    //Initializations
    request = lw_prefmanager_get_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_HIRA_KATA);

    lw_prefmanager_set_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_HIRA_KATA, !request);
}


//!
//! @brief Callback to toggle the katakana-hiragana conversion setting for the search entry
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settingswindow_kata_hira_conv_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    gboolean request;

    //Initializations
    request = lw_prefmanager_get_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_KATA_HIRA);

    lw_prefmanager_set_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_KATA_HIRA, !request);
}


//!
//! @brief Callback to toggle spellcheck in the search entry
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settingswindow_spellcheck_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    gboolean request;

    //Initializations
    request = lw_prefmanager_get_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK);

    lw_prefmanager_set_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK, !request);
}


//!
//! @brief Callback to toggle search as you type in the search entry
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settingswindow_search_as_you_type_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    gboolean request;

    //Initializations
    request = lw_prefmanager_get_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_SEARCH_AS_YOU_TYPE);

    lw_prefmanager_set_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_SEARCH_AS_YOU_TYPE, !request);
}



//!
//! @brief Callback to toggle romaji-kana conversion
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settingswindow_romaji_kana_conv_changed_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    int active;

    //Initializations
    active = gtk_combo_box_get_active(GTK_COMBO_BOX (widget));

    lw_prefmanager_set_int_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_ROMAN_KANA, active);
}


//!
//! @brief Callback to set the user selected color to the color swatch for text highlighting
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settingswindow_swatch_color_changed_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GdkColor color;
    char *hex_color_string;
    char *pref_key;
    char *letter;

    //Initializations
    gtk_color_button_get_color(GTK_COLOR_BUTTON (widget), &color);
    hex_color_string = gdk_color_to_string (&color);
    pref_key = g_strdup_printf ("%s", gtk_buildable_get_name (GTK_BUILDABLE (widget)));
    letter = strchr(pref_key, '_');
    if (letter == NULL) return;
    *letter = '-';

    //Set the color in the prefs
    if (pref_key != NULL && hex_color_string != NULL)
    {
      lw_prefmanager_set_string_by_schema (app->prefmanager, LW_SCHEMA_HIGHLIGHT, pref_key, hex_color_string);
    }

    //Cleanup
    if (pref_key != NULL) g_free (pref_key);
    if (hex_color_string != NULL) g_free (hex_color_string);
}


//!
//! @brief Callback to reset all the colors for all the swatches to the default in the preferences
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settingswindow_reset_all_swatches_activated_cb (GtkWidget *widget, gpointer data)
{
    //Initializations
    int i;
    char *pref_key[] = {
      LW_KEY_MATCH_FG,
      LW_KEY_MATCH_BG,
      LW_KEY_HEADER_FG,
      LW_KEY_HEADER_BG,
      LW_KEY_COMMENT_FG,
      NULL
    };

    //Start setting the default values
    for (i = 0; pref_key[i] != NULL; i++)
    {
      lw_prefmanager_reset_value_by_schema (app->prefmanager, LW_SCHEMA_HIGHLIGHT, pref_key[i]);
    }
}


//!
//! @brief Sets the preference key for the global font usage
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settingswindow_use_global_document_font_toggled_cb (GtkWidget *widget, gpointer data)
{
    gboolean request;

    request = lw_prefmanager_get_boolean_by_schema (app->prefmanager, LW_SCHEMA_FONT, LW_KEY_FONT_USE_GLOBAL_FONT);

    lw_prefmanager_set_boolean_by_schema (app->prefmanager, LW_SCHEMA_FONT, LW_KEY_FONT_USE_GLOBAL_FONT, !request);
}


//!
//! @brief Sets the preference key for the new custom document font
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settingswindow_custom_document_font_changed_cb (GtkWidget *widget, gpointer data)
{
    GwSettingsWindow *window;
    GtkWidget *button;
    const char *font;

    window = GW_SETTINGSWINDOW (gw_app_get_window (app, GW_WINDOW_SETTINGS, widget));
    button = GTK_WIDGET (gtk_builder_get_object (window->builder, "custom_font_fontbutton"));
    font = gtk_font_button_get_font_name (GTK_FONT_BUTTON (button));

    lw_prefmanager_set_string_by_schema (app->prefmanager, LW_SCHEMA_FONT, LW_KEY_FONT_CUSTOM_FONT, font);
}


//!
//! @brief Sets the state of the use global document font checkbox without triggering the signal handler
//! @param setting The new checked state for the use global document font checkbox 
//!
G_MODULE_EXPORT void gw_settingswindow_sync_use_global_document_font_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GtkWidget *checkbox;
    GtkWidget *hbox;
    gboolean request;

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    if (window == NULL) return;
    checkbox = GTK_WIDGET (gtk_builder_get_object (window->builder, "system_font_checkbox"));
    hbox = GTK_WIDGET (gtk_builder_get_object (window->builder, "system_document_font_hbox"));
    request = lw_prefmanager_get_boolean (settings, KEY);

    //Updates
    g_signal_handlers_block_by_func (checkbox, gw_settingswindow_use_global_document_font_toggled_cb, NULL);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), request);
    gtk_widget_set_sensitive (hbox, !request);
    g_signal_handlers_unblock_by_func (checkbox, gw_settingswindow_use_global_document_font_toggled_cb, NULL);
}


G_MODULE_EXPORT void gw_settingswindow_sync_global_document_font_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GtkCheckButton *button;
    char font[50];
    char *text;

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    if (window == NULL) return;
    button = GTK_CHECK_BUTTON (gtk_builder_get_object (window->builder, "system_font_checkbox"));
    lw_prefmanager_get_string_by_schema (app->prefmanager, font, LW_SCHEMA_GNOME_INTERFACE, LW_KEY_DOCUMENT_FONT_NAME, 50);
    text = g_strdup_printf (gettext("_Use the System Document Font (%s)"), font);

    if (text != NULL) 
    {
      gtk_button_set_label (GTK_BUTTON (button), text);
      g_free (text);
    }
}


//!
//! @brief Sets the text in the custom document font button
//! @param font_description_string The font description in the form "Sans 10"
//!
G_MODULE_EXPORT void gw_settingswindow_sync_custom_font_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GtkFontButton *button;
    char font[50];

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    if (window == NULL) return;
    button = GTK_FONT_BUTTON (gtk_builder_get_object (window->builder, "custom_font_fontbutton"));
    lw_prefmanager_get_string_by_schema (app->prefmanager, font, LW_SCHEMA_FONT, LW_KEY_FONT_CUSTOM_FONT, 50);

    //Body
    g_signal_handlers_block_by_func (button, gw_settingswindow_custom_document_font_changed_cb, NULL);
    gtk_font_button_set_font_name (button, font);
    g_signal_handlers_unblock_by_func (button, gw_settingswindow_custom_document_font_changed_cb, NULL);
}


//!
//! @brief Sets the checkbox to show or hide the toolbar
//! @param request How to set the toolbar
//!
G_MODULE_EXPORT void gw_settingswindow_sync_search_as_you_type_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GtkWidget *checkbox;
    gboolean request;

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    if (window == NULL) return;
    checkbox = GTK_WIDGET (gtk_builder_get_object(window->builder, "search_as_you_type_checkbox"));
    request = lw_prefmanager_get_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_SEARCH_AS_YOU_TYPE);

    g_signal_handlers_block_by_func (checkbox, gw_settingswindow_search_as_you_type_toggled_cb, NULL);
    gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (checkbox), request);
    g_signal_handlers_unblock_by_func (checkbox, gw_settingswindow_search_as_you_type_toggled_cb, NULL);
}


//!
//! @brief Disables portions of the interface depending on the currently queued jobs.
//!
G_MODULE_EXPORT void gw_settingswindow_dictionaries_changed_cb (GtkWidget* widget, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GtkWidget *message;
    GList *list;

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    message = GTK_WIDGET (gtk_builder_get_object (window->builder, "please_install_dictionary_hbox"));

    //Set the show state of the dictionaries required message
    if (g_list_length (app->dictinfolist->list) > 0)
      gtk_widget_hide (message);
    else
      gtk_widget_show (message);
}


//!
//! @brief Closes the window passed throught the widget pointer
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_settingswindow_close_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    
    //Initializations
    window = GW_SETTINGSWINDOW (gw_app_get_window (app, GW_WINDOW_SETTINGS, NULL));

    gw_app_destroy_window (app, GW_WINDOW_SETTINGS, NULL);
}


G_MODULE_EXPORT void gw_settingswindow_sync_romaji_kana_conv_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GtkComboBox *combobox;
    int request;

    window = GW_SETTINGSWINDOW (data);
    combobox = GTK_COMBO_BOX (gtk_builder_get_object (window->builder, "query_romaji_to_kana"));
    request = lw_prefmanager_get_int (settings, KEY);

    gtk_combo_box_set_active(combobox, request);
}


G_MODULE_EXPORT void gw_settingswindow_sync_hira_kata_conv_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GtkToggleButton *checkbox;
    gboolean request;

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    checkbox = GTK_TOGGLE_BUTTON (gtk_builder_get_object (window->builder, "query_hiragana_to_katakana"));
    request = lw_prefmanager_get_boolean (settings, KEY);

    g_signal_handlers_block_by_func (checkbox, gw_settingswindow_hira_kata_conv_toggled_cb, NULL);
    gtk_toggle_button_set_active(checkbox, request);
    g_signal_handlers_unblock_by_func (checkbox, gw_settingswindow_hira_kata_conv_toggled_cb, NULL);
}


G_MODULE_EXPORT void gw_settingswindow_sync_kata_hira_conv_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GtkToggleButton *checkbox;
    gboolean request;

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    checkbox = GTK_TOGGLE_BUTTON (gtk_builder_get_object (window->builder, "query_katakana_to_hiragana"));
    request = lw_prefmanager_get_boolean (settings, KEY);

    g_signal_handlers_block_by_func (checkbox, gw_settingswindow_kata_hira_conv_toggled_cb, NULL);
    gtk_toggle_button_set_active(checkbox, request);
    g_signal_handlers_unblock_by_func (checkbox, gw_settingswindow_kata_hira_conv_toggled_cb, NULL);
}


G_MODULE_EXPORT void gw_settingswindow_sync_swatch_color_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSettingsWindow *window;
    GtkColorButton *swatch;
    GdkColor color;
    char hex_color_string[20];

    //Initializations
    window = GW_SETTINGSWINDOW (data);
    swatch = GTK_COLOR_BUTTON (data);
    lw_prefmanager_get_string (hex_color_string, settings, KEY, 20);
    g_assert (swatch != NULL);

    if (gdk_color_parse (hex_color_string, &color) == TRUE)
    {
      gtk_color_button_set_color (swatch, &color);
    }
}


