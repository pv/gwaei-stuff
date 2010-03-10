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
//! @file src/preferences-gconf.c
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

#include <gconf/gconf-client.h>
#include <gwaei/definitions.h>
#include <gwaei/regex.h>


int gw_pref_get_int (char *key, int backup)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    
    int return_value;
    GError *err = NULL;

    return_value = gconf_client_get_int (client, key, &err);

    if (err != NULL)
    {
      g_error_free (err);
      err = NULL;
      return_value = backup;
    }

    g_object_unref (client);

    return return_value;
}


int gw_pref_get_default_int (char *key, int backup)
{
    GConfClient *client;
    client = gconf_client_get_default ();

    GConfValue *value;
    int return_value;
    GError *err = NULL;

    value = gconf_client_get_default_from_schema (client, key, &err);
    if (err != NULL)
    {
      g_error_free (err);
      err = NULL;
      return_value = backup;
    }
    if (value == NULL || value->type != GCONF_VALUE_INT)
    {
      return_value = backup;
    }
    else
    {
      return_value = gconf_value_get_int (value);
    }
    g_object_unref (client);

    return return_value;
}


void gw_pref_set_int (char *key, int request)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    gconf_client_set_int (client, key, request, NULL);
    g_object_unref (client);
}


gboolean gw_pref_get_boolean (char *key, gboolean backup)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    
    gboolean return_value;
    GError *err = NULL;

    return_value = gconf_client_get_bool (client, key, &err);

    if (err != NULL)
    {
      g_error_free (err);
      err = NULL;
      return_value = backup;
    }

    g_object_unref (client);

    return return_value;
}


gboolean gw_pref_get_default_boolean (char *key, gboolean backup)
{
    GConfClient *client;
    client = gconf_client_get_default ();

    GConfValue *value;
    gboolean return_value;
    GError *err = NULL;

    value = gconf_client_get_default_from_schema (client, key, &err);
    if (err != NULL)
    {
      g_error_free (err);
      err = NULL;
      return_value = backup;
    }
    else if (value == NULL || value->type != GCONF_VALUE_BOOL)
    {
      return_value = backup;
    }
    else
    {
      return_value = gconf_value_get_bool (value);
    }
    g_object_unref (client);

    return return_value;
}


void gw_pref_set_boolean (char *key, gboolean request)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    gconf_client_set_bool (client, key, request, NULL);
    g_object_unref (client);
}

//!
//! Returns a preferenc string, using a specified backup if
//! there is an error with up to n characters copied.
//!
char* gw_pref_get_string (char *output, char *key, char* backup, int n)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    
    char* return_value;
    GError *err = NULL;

    return_value = gconf_client_get_string (client, key, &err);

    if (err != NULL)
    {
      g_error_free (err);
      err = NULL;
      return_value = NULL;
      strncpy(output, backup, n);
    }
    else if (return_value == NULL || strlen(return_value) == 0)
    {
      strncpy(output, backup, n);
    }
    else
    {
      strncpy(output, return_value, n);
    }

    g_object_unref (client);

    return return_value;
}

const char* gw_pref_get_default_string (char *key, char* backup)
{
    GConfClient *client;
    client = gconf_client_get_default ();

    GConfValue *value;
    const char* return_value;
    GError *err = NULL;

    value = gconf_client_get_default_from_schema (client, key, &err);
    if (err != NULL || (value != NULL && value->type != GCONF_VALUE_STRING))
    {
      g_error_free (err);
      err = NULL;
      return_value = backup;
    }
    else
    {
      return_value = gconf_value_get_string (value);
    }
    g_object_unref (client);

    return return_value;
}


const char* gw_pref_free_string (char *sting)
{
    
}

void gw_pref_set_string (char *key, const char* request)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    gconf_client_set_string (client, key, request, NULL);
    g_object_unref (client);
}


//
//Actions taken when a gconf value changes
//

void do_dictionary_source_gconf_key_changed_action (gpointer client,
                                                    guint cnxn_id,
                                                    gpointer entry,
                                                    gpointer data   )
/*
void do_dictionary_source_gconf_key_changed_action (GConfClient* client,
                                                    guint cnxn_id,
                                                    GConfEntry *entry,
                                                    gpointer data        )
*/
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_STRING)
      gw_ui_set_dictionary_source (data, gconf_value_get_string(value));
}


void do_toolbar_style_pref_changed_action( GConfClient* client, 
                                           guint        cnxn_id,
                                           GConfEntry*  entry,
                                           gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_STRING)
      gw_ui_set_toolbar_style(gconf_value_get_string(value));
    else
      gw_ui_set_toolbar_style("both");
}


void do_toolbar_show_pref_changed_action ( GConfClient* client,
                                           guint        cnxn_id,
                                           GConfEntry*  entry,
                                           gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gw_ui_set_toolbar_show(gconf_value_get_bool(value));
    else
      gw_ui_set_toolbar_show(FALSE);
}


void do_font_size_pref_changed_action ( GConfClient* client,
                                        guint cnxn_id,
                                        GConfEntry *entry,
                                        gpointer data        )
{
    //Get the size from the GCONF key
    int size;
    size = gconf_client_get_int ( client, GCKEY_GW_FONT_SIZE, NULL);

    char font[100];
    gw_pref_get_string (font, GCKEY_DOCUMENT_FONT_NAME, "Sans 10", 100);
    char *pos = strrchr(font, ' ');
    if (pos != NULL)
      *pos = '\0';

    //If the value is strange, get the default value
    if (size < MIN_FONT_SIZE | size > MAX_FONT_SIZE)
    {
      GConfValue *value;
      value = gconf_client_get_default_from_schema ( client,
                                                     GCKEY_GW_FONT_SIZE,
                                                     NULL                   );
      size = gconf_value_get_int (value);
      if (value != NULL && size >= MIN_FONT_SIZE && size <= MAX_FONT_SIZE) {
        gconf_client_set_int ( client, GCKEY_GW_FONT_SIZE, size, NULL);
      }
      else
        gconf_client_set_int ( client, GCKEY_GW_FONT_SIZE, 12, NULL);
      return;
    }

    gw_ui_set_font(font, size);

    gw_ui_update_toolbar_buttons ();
}


void do_less_relevant_show_pref_changed_action ( GConfClient* client,
                                                 guint        cnxn_id,
                                                 GConfEntry*  entry,
                                                 gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gw_ui_set_less_relevant_show(gconf_value_get_bool(value));
    else
      gw_ui_set_less_relevant_show(TRUE);
}


void do_roman_kana_conv_pref_changed_action( GConfClient* client, 
                                             guint        cnxn_id,
                                             GConfEntry*  entry,
                                             gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_INT)
    {
       int selection;
       selection = gconf_value_get_int(value);

       if (selection <= 2 && selection >= 0)
         gw_ui_set_romaji_kana_conv(selection);
       else
         gw_ui_set_romaji_kana_conv(2);
    }
}


void do_hira_kata_conv_pref_changed_action (GConfClient* client, 
                                            guint        cnxn_id,
                                            GConfEntry*  entry,
                                            gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gw_ui_set_hiragana_katakana_conv(gconf_value_get_bool(value));
    else
      gw_ui_set_hiragana_katakana_conv(TRUE);
}


void do_kata_hira_conv_pref_changed_action( GConfClient *client, 
                                            guint        cnxn_id,
                                            GConfEntry  *entry,
                                            gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gw_ui_set_katakana_hiragana_conv(gconf_value_get_bool(value));
    else
      gw_ui_set_katakana_hiragana_conv(TRUE);
}


void do_spellcheck_pref_changed_action( GConfClient* client, 
                                        guint        cnxn_id,
                                        GConfEntry*  entry,
                                        gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gw_sexy_ui_set_spellcheck(gconf_value_get_bool(value));
    else
      gw_sexy_ui_set_spellcheck(TRUE);
}


void do_color_value_changed_action( GConfClient* client, 
                                    guint cnxn_id,
                                    GConfEntry *entry,
                                    gpointer data       )
{
    //Get the gconf value
    GConfValue *value;
    value = gconf_entry_get_value(entry);
    if (value != NULL &&  value->type == GCONF_VALUE_STRING)
    {
      const char *hex_string = gconf_value_get_string(value);
       
      //If the format of the string is wrong, get the default one
      if (regexec(&re_hexcolor, gconf_value_get_string(value), 1, NULL, 0) != 0)
      {
        value = gconf_client_get_default_from_schema (client,
                                                      GCKEY_GW_FONT_SIZE,
                                                      NULL                  );
        if (value != NULL && value->type == GCONF_VALUE_STRING)
          hex_string = gconf_value_get_string (value);
        else
          return;
      }

      guint red   = gw_util_2digithexstrtoint(hex_string[1], hex_string[2]);
      guint green = gw_util_2digithexstrtoint(hex_string[3], hex_string[4]);
      guint blue  = gw_util_2digithexstrtoint(hex_string[5], hex_string[6]);

      //The last portion of the key happens to be the widget id
      const char *key = gconf_entry_get_key(entry);
      const char *key_ptr = &key[strlen(key)];
      while (*key_ptr != '/')
        key_ptr--;
      key_ptr++;

      gw_ui_set_color_to_swatch(key_ptr, red, green, blue);

      gw_ui_reload_tagtable_tags();
    }
}


void do_update_dictionary_order_list_changed_action ()
{
      gw_ui_update_dictionary_orders ();
}




//
//Preference initializations
//

void gw_prefs_add_change_listener (const char *KEY, GConfClientNotifyFunc callback_function, gpointer data)
{
  GConfClient *client;
  client = gconf_client_get_default ();

  gconf_client_notify_add (client, KEY, callback_function, data, NULL, NULL);
  gconf_client_notify (client, KEY);
}


void gw_prefs_initialize_preferences()
{
  g_type_init();

  char *string;

  GConfClient *client;
  client = gconf_client_get_default ();
  
  //Add directory listeners gwaei will be using
  gconf_client_add_dir   ( client, GCPATH_INTERFACE, 
                           GCONF_CLIENT_PRELOAD_NONE,
                           NULL                             );
  gconf_client_add_dir   ( client, GCPATH_GW, 
                           GCONF_CLIENT_PRELOAD_NONE,
                           NULL                            );

  //Add preference change notifiers
  gw_prefs_add_change_listener (GCKEY_GW_LESS_RELEVANT_SHOW, do_less_relevant_show_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_TOOLBAR_STYLE, do_toolbar_style_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_TOOLBAR_SHOW, do_toolbar_show_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_FONT_SIZE, do_font_size_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_DOCUMENT_FONT_NAME, do_font_size_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_ROMAN_KANA, do_roman_kana_conv_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_HIRA_KATA, do_hira_kata_conv_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_KATA_HIRA, do_kata_hira_conv_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_SPELLCHECK, do_spellcheck_pref_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_MATCH_FG, do_color_value_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_MATCH_BG, do_color_value_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_HEADER_FG, do_color_value_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_HEADER_BG, do_color_value_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_COMMENT_FG, do_color_value_changed_action, NULL);
  gw_prefs_add_change_listener (GCKEY_GW_LOAD_ORDER, do_dictionary_source_gconf_key_changed_action, NULL);

  g_object_unref(client);
}







