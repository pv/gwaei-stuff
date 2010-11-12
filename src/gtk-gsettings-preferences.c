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
//! @file src/gtk-gconf-preferences.c
//!
//! @brief Abstraction layer for gconf preferences
//!
//! Allows access to gconf with the ability to specify backup preferences upon
//! failure to get the preference value.
//!


#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <gdk/gdk.h>
#include <gio/gio.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>

#include <gwaei/gtk-main-interface.h>
#ifdef WITH_LIBSEXY
#include <gwaei/gtk-main-interface-sexy.h>
#endif
#include <gwaei/gtk-settings-interface.h>


//!
//! @brief Resets a value in a key
//!
//! @param schemaid A string identifying the schema there the key is
//! @param key A string identifying the key to reset
//!

void gw_pref_reset_value (const char* schemaid, const char *key)
{
    GSettings *settings = NULL;
    if ((settings = g_settings_new (schemaid)) != NULL)
    {
      g_settings_reset (settings, key);
    }
}


//!
//! @brief Returns an integer from the preference backend 
//!
//! @param schemaid The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param backup the value to return on failure
//!
int gw_pref_get_int (const char* schemaid, const char *key)
{
    GSettings *settings = NULL;
    int value = 0;

    if ((settings = g_settings_new (schemaid)) != NULL)
    {
      value = g_settings_get_int (settings, key);
    }

    return value;
}


//!
//! @brief Sets the int to the key in the preferences backend
//!
//! @param schemaid The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void gw_pref_set_int (const char* schemaid, const char *key, const int request)
{
    GSettings *settings = NULL;

    if ((settings = g_settings_new (schemaid)) != NULL)
    {
      g_settings_set_int (settings, key, request);
    }
}


//!
//! @brief Returns an boolean from the preference backend 
//!
//! @param schemaid The key to use to look up the pref
//! @param key The key to use to look up the pref
//!
gboolean gw_pref_get_boolean (const char* schemaid, const char *key)
{
    GSettings *settings = NULL;
    gboolean value = FALSE; 

    if ((settings = g_settings_new (schemaid)) != NULL)
    {
      value = g_settings_get_boolean (settings, key);
    }

    return value;
}


//!
//! @brief Sets the boolean to the key in the preferences backend
//!
//! @param schemaid The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void gw_pref_set_boolean (const char* schemaid, const char *key, const gboolean request)
{
    GSettings *settings = NULL;

    if ((settings = g_settings_new (schemaid)) != NULL)
    {
      g_settings_set_boolean (settings, key, request);
    }
}


//!
//! @brief Returns an string from the preference backend 
//!
//! @output string to copy the pref to
//! @param schemaid The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param backup the value to return on failure
//! @param n The max characters to copy to output
//!
void gw_pref_get_string (char *output, const char* schemaid, const char *key, const int n)
{
    GSettings *settings = NULL;
    gchar *value = NULL; 

    if ((settings = g_settings_new (schemaid)) != NULL)
    {
      value = g_settings_get_string (settings, key);
      if (value != NULL) 
        strncpy(output, value, n);
      else
        output[0] = '\0';
    }
}


//!
//! @brief Sets the string to the key in the preferences backend
//!
//! @param schemaid The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void gw_pref_set_string (const char* schemaid, const char *key, const char* request)
{
    GSettings *settings = NULL;

    if ((settings = g_settings_new (schemaid)) != NULL)
    {
      g_settings_set_string (settings, key, request);
    }
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_dictionary_source_pref_key_changed_action (GSettings *settings,
                                                   gchar *key,
                                                   gpointer data       )
{
    g_signal_handlers_block_by_func (settings, do_dictionary_source_pref_key_changed_action, NULL);
    gchar *value = g_settings_get_string (settings, key);
    if (value != NULL)
    {
      gw_ui_set_dictionary_source (data, value);
    }
    g_free (value);
    g_signal_handlers_unblock_by_func (settings, do_dictionary_source_pref_key_changed_action, NULL);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_toolbar_style_pref_changed_action (GSettings *settings,
                                           gchar *key,
                                           gpointer data       )
{
    gchar *value = g_settings_get_string (settings, key);
    if (value != NULL)
    {
      gw_ui_set_toolbar_style (value);
    }
    else
    {
      gw_ui_set_toolbar_style ("both");
    }
    g_free (value);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_toolbar_show_pref_changed_action (GSettings *settings,
                                          gchar *key,
                                          gpointer data       )
{
    g_signal_handlers_block_by_func (settings, do_toolbar_show_pref_changed_action, NULL);
    gboolean value = g_settings_get_boolean (settings, key);
    gw_ui_set_toolbar_show (value);
    g_signal_handlers_unblock_by_func (settings, do_toolbar_show_pref_changed_action, NULL);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_use_global_document_font_pref_changed_action (GSettings *settings,
                                                      gchar *key,
                                                      gpointer data       )
{
    g_signal_handlers_block_by_func (settings, do_use_global_document_font_pref_changed_action, NULL);
    gboolean value = g_settings_get_boolean (settings, key);
    gw_ui_set_use_global_document_font_checkbox (value);
    gw_ui_set_font (NULL, NULL);
    g_signal_handlers_unblock_by_func (settings, do_use_global_document_font_pref_changed_action, NULL);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_global_document_font_pref_changed_action (GSettings *settings,
                                                  gchar *key,
                                                  gpointer data       )
{
    gchar *value = g_settings_get_string (settings, key);
    if (value != NULL)
    {
      gw_ui_update_global_font_label (value);
      gw_ui_set_font (NULL, NULL);
    }
    g_free (value);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_custom_document_font_pref_changed_action (GSettings *settings,
                                                  gchar *key,
                                                  gpointer data       )
{
    g_signal_handlers_block_by_func (settings, do_custom_document_font_pref_changed_action, NULL);
    gchar *value = g_settings_get_string (settings, key);
    if (value != NULL)
    {
      gw_ui_update_custom_font_button (value);
      gw_ui_set_font (NULL, NULL);
    }
    g_free (value);
    g_signal_handlers_unblock_by_func (settings, do_custom_document_font_pref_changed_action, NULL);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_font_magnification_pref_changed_action (GSettings *settings,
                                                gchar *key,
                                                gpointer data       )
{
    g_signal_handlers_block_by_func (settings, do_font_magnification_pref_changed_action, NULL);
    int magnification = g_settings_get_int (settings, key);

    //Sanity checks on the pref
    if (magnification < GW_MIN_FONT_MAGNIFICATION)
    {
      magnification = GW_MIN_FONT_MAGNIFICATION;
      gw_pref_set_int (GW_SCHEMA_FONT, key, magnification);
    }
    else if (magnification > GW_MAX_FONT_MAGNIFICATION)
    {
      magnification = GW_MAX_FONT_MAGNIFICATION;
      gw_pref_set_int (GW_SCHEMA_FONT, key, magnification);
    }
    //Set the new font
    else
    {
      gw_ui_set_font (NULL, &magnification);
      gw_ui_update_toolbar_buttons ();
    }
    g_signal_handlers_unblock_by_func (settings, do_font_magnification_pref_changed_action, NULL);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_less_relevant_show_pref_changed_action (GSettings *settings,
                                                gchar *key,
                                                gpointer data       )
{
    g_signal_handlers_block_by_func (settings, do_less_relevant_show_pref_changed_action, NULL);
    gboolean value = g_settings_get_boolean (settings, key);

    gw_ui_set_less_relevant_show (value);
    g_signal_handlers_unblock_by_func (settings, do_less_relevant_show_pref_changed_action, NULL);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_roman_kana_conv_pref_changed_action (GSettings *settings,
                                             gchar *key,
                                             gpointer data       )
{
    g_signal_handlers_block_by_func (settings, do_roman_kana_conv_pref_changed_action, NULL);
    int selection = g_settings_get_int (settings, key);
    if (selection <= 2 && selection >= 0)
      gw_ui_set_romaji_kana_conv(selection);
    else
      gw_ui_set_romaji_kana_conv(2);
    g_signal_handlers_unblock_by_func (settings, do_roman_kana_conv_pref_changed_action, NULL);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_hira_kata_conv_pref_changed_action (GSettings *settings,
                                            gchar *key,
                                            gpointer data       )
{
    g_signal_handlers_block_by_func (settings, do_hira_kata_conv_pref_changed_action, NULL);
    gboolean value = g_settings_get_boolean (settings, key);
    gw_ui_set_hiragana_katakana_conv(value);
    g_signal_handlers_unblock_by_func (settings, do_hira_kata_conv_pref_changed_action, NULL);
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_kata_hira_conv_pref_changed_action (GSettings *settings,
                                            gchar *key,
                                            gpointer data       )
{
    g_signal_handlers_block_by_func (settings, do_kata_hira_conv_pref_changed_action, NULL);
    gboolean value = g_settings_get_boolean (settings, key);
    gw_ui_set_katakana_hiragana_conv(value);
    g_signal_handlers_unblock_by_func (settings, do_kata_hira_conv_pref_changed_action, NULL);
}


#ifdef WITH_LIBSEXY
//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_spellcheck_pref_changed_action (GSettings *settings,
                                        gchar *key,
                                        gpointer data       )
{
    g_signal_handlers_block_by_func (settings, do_spellcheck_pref_changed_action, NULL);
    gboolean value = g_settings_get_boolean (settings, key);
    gw_sexy_ui_set_spellcheck (value);
    g_signal_handlers_unblock_by_func (settings, do_spellcheck_pref_changed_action, NULL);
}
#endif


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_color_value_changed_action (GSettings *settings,
                                    gchar *key,
                                    gpointer data       )
{
    g_signal_handlers_block_by_func (settings, do_color_value_changed_action, NULL);
    char hex_color[20];
    gw_pref_get_string (hex_color, GW_SCHEMA_HIGHLIGHT, key, 20);

    GdkColor color;
    if (gdk_color_parse (hex_color, &color) == FALSE)
    {
      gw_pref_reset_value (GW_SCHEMA_HIGHLIGHT, key);
      return;
    }

    //Finish up
    char *name = key;
    char *letter  = strchr(name, '-');
    if (letter != NULL) *letter = '_';
    gw_ui_set_color_to_swatch (name, hex_color);
    gw_ui_buffer_reload_tagtable_tags ();
    g_signal_handlers_unblock_by_func (settings, do_color_value_changed_action, NULL);
}


//!
//! @brief Adds a preference change listener for the selected key
//!
//! @param schemaid The key to use to look up the pref
//! @param key The preference key
//! @param callback_function The function to call when the key changes
//! @param data The userdata to pass to the callback function
//!
void gw_prefs_add_change_listener (const char* schemaid, const char *key, void (*callback_function) (GSettings*, gchar*, gpointer), gpointer data)
{
  GSettings *setting = g_settings_new (schemaid);
  char signal_name[100];
  strcpy(signal_name, "changed::");
  strcat(signal_name, key);

  g_signal_connect (G_OBJECT (setting), signal_name, G_CALLBACK (callback_function), data);
  GVariant *value = g_settings_get_value(setting, key);
  if (value != NULL) g_settings_set_value(setting, key, value);
}


//!
//! @brief Initializes the preferences backend
//!
void gw_prefs_initialize_preferences()
{
  g_type_init();

  gw_prefs_add_change_listener (GW_SCHEMA_FONT, GW_KEY_FONT_USE_GLOBAL_FONT, do_use_global_document_font_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GW_SCHEMA_FONT, GW_KEY_FONT_CUSTOM_FONT, do_custom_document_font_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GW_SCHEMA_FONT, GW_KEY_FONT_MAGNIFICATION, do_font_magnification_pref_changed_action, NULL);

  gw_prefs_add_change_listener (GW_SCHEMA_BASE, GW_KEY_TOOLBAR_SHOW, do_toolbar_show_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GW_SCHEMA_BASE, GW_KEY_LESS_RELEVANT_SHOW, do_less_relevant_show_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GW_SCHEMA_BASE, GW_KEY_ROMAN_KANA, do_roman_kana_conv_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GW_SCHEMA_BASE, GW_KEY_HIRA_KATA, do_hira_kata_conv_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GW_SCHEMA_BASE, GW_KEY_KATA_HIRA, do_kata_hira_conv_pref_changed_action, NULL);
#ifdef WITH_LIBSEXY
  gw_prefs_add_change_listener (GW_SCHEMA_BASE, GW_KEY_SPELLCHECK, do_spellcheck_pref_changed_action, NULL);
#endif
  gw_prefs_add_change_listener (GW_SCHEMA_HIGHLIGHT, GW_KEY_MATCH_FG, do_color_value_changed_action, NULL);
  gw_prefs_add_change_listener (GW_SCHEMA_HIGHLIGHT, GW_KEY_MATCH_BG, do_color_value_changed_action, NULL);
  gw_prefs_add_change_listener (GW_SCHEMA_HIGHLIGHT, GW_KEY_HEADER_FG, do_color_value_changed_action, NULL);
  gw_prefs_add_change_listener (GW_SCHEMA_HIGHLIGHT, GW_KEY_HEADER_BG, do_color_value_changed_action, NULL);
  gw_prefs_add_change_listener (GW_SCHEMA_HIGHLIGHT, GW_KEY_COMMENT_FG, do_color_value_changed_action, NULL);
}







