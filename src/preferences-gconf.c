/******************************************************************************

  FILE:
  src/preferences-gconf.c

  DESCRIPTION:
  Calls to gconf are abstracted out here. This is the gconf version.

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

#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <gconf/gconf-client.h>
#include <gwaei/definitions.h>
#include <gwaei/regex.h>


int gwaei_pref_get_int (char *key, int backup)
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


int gwaei_pref_get_default_int (char *key, int backup)
{
    GConfClient *client;
    client = gconf_client_get_default ();

    GConfValue *value;
    int return_value;
    GError *err = NULL;

    value = gconf_client_get_default_from_schema (client, key, &err);
    if (err != NULL || (value != NULL && value->type != GCONF_VALUE_INT))
    {
      g_error_free (err);
      err = NULL;
      return_value = backup;
    }
    else
    {
      return_value = gconf_value_get_int (value);
    }
    g_object_unref (client);

    return return_value;
}


void gwaei_pref_set_int (char *key, int request)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    gconf_client_set_int (client, key, request, NULL);
    g_object_unref (client);
}


gboolean gwaei_pref_get_boolean (char *key, gboolean backup)
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


gboolean gwaei_pref_get_default_boolean (char *key, gboolean backup)
{
    GConfClient *client;
    client = gconf_client_get_default ();

    GConfValue *value;
    gboolean return_value;
    GError *err = NULL;

    value = gconf_client_get_default_from_schema (client, key, &err);
    if (err != NULL || (value != NULL && value->type != GCONF_VALUE_BOOL))
    {
      g_error_free (err);
      err = NULL;
      return_value = backup;
    }
    else
    {
      return_value = gconf_value_get_bool (value);
    }
    g_object_unref (client);

    return return_value;
}


void gwaei_pref_set_boolean (char *key, gboolean request)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    gconf_client_set_bool (client, key, request, NULL);
    g_object_unref (client);
}


char* gwaei_pref_get_string (char *output, char *key, char* backup, int n)
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
    else
    {
      strncpy(output, return_value, n);
    }

    g_object_unref (client);

    return return_value;
}


const char* gwaei_pref_get_default_string (char *key, char* backup)
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

const char* gwaei_pref_free_string (char *sting)
{
    
}

void gwaei_pref_set_string (char *key, char* request)
{
    GConfClient *client;
    client = gconf_client_get_default ();
    gconf_client_set_string (client, key, request, NULL);
    g_object_unref (client);
}


//
//Actions taken when a gconf value changes
//

void do_dictionary_source_gconf_key_changed_action (GConfClient* client,
                                                    guint cnxn_id,
                                                    GConfEntry *entry,
                                                    gpointer data        )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_STRING)
    {
      //The last portion of the key happens to be the widget id
      const char *key = gconf_entry_get_key(entry);
      const char *key_ptr = &key[strlen(key)];
      while (*key_ptr != '/')
        key_ptr--;
      key_ptr++;

      gwaei_ui_set_dictionary_source(key_ptr, gconf_value_get_string(value));
    }
}


void do_toolbar_style_pref_changed_action( GConfClient* client, 
                                           guint        cnxn_id,
                                           GConfEntry*  entry,
                                           gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_STRING)
      gwaei_ui_set_toolbar_style(gconf_value_get_string(value));
    else
      gwaei_ui_set_toolbar_style("both");
}


void do_toolbar_show_pref_changed_action ( GConfClient* client,
                                           guint        cnxn_id,
                                           GConfEntry*  entry,
                                           gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gwaei_ui_set_toolbar_show(gconf_value_get_bool(value));
    else
      gwaei_ui_set_toolbar_show(FALSE);
}


void do_font_size_pref_changed_action ( GConfClient* client,
                                        guint cnxn_id,
                                        GConfEntry *entry,
                                        gpointer data        )
{
    //Get the size from the GCONF key
    int size;
    size = gconf_client_get_int ( client, GCKEY_GWAEI_FONT_SIZE, NULL);

    //If the value is strange, get the default value
    if (size < MIN_FONT_SIZE | size > MAX_FONT_SIZE)
    {
      GConfValue *value;
      value = gconf_client_get_default_from_schema ( client,
                                                     GCKEY_GWAEI_FONT_SIZE,
                                                     NULL                   );
      size = gconf_value_get_int (value);
      if (value != NULL && size >= MIN_FONT_SIZE && size <= MAX_FONT_SIZE) {
        gconf_client_set_int ( client, GCKEY_GWAEI_FONT_SIZE, size, NULL);
      }
      else
        gconf_client_set_int ( client, GCKEY_GWAEI_FONT_SIZE, 12, NULL);
      return;
    }

    gwaei_ui_set_font("Sans", size);

    update_toolbar_buttons ();
}


void do_less_relevant_show_pref_changed_action ( GConfClient* client,
                                                 guint        cnxn_id,
                                                 GConfEntry*  entry,
                                                 gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gwaei_ui_set_less_relevant_show(gconf_value_get_bool(value));
    else
      gwaei_ui_set_less_relevant_show(TRUE);
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
         gwaei_ui_set_romanji_kana_conv(selection);
       else
         gwaei_ui_set_romanji_kana_conv(2);
    }
}


void do_hira_kata_conv_pref_changed_action( GConfClient* client, 
                                            guint        cnxn_id,
                                            GConfEntry*  entry,
                                            gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gwaei_ui_set_hiragana_katakana_conv(gconf_value_get_bool(value));
    else
      gwaei_ui_set_hiragana_katakana_conv(TRUE);
}


void do_kata_hira_conv_pref_changed_action( GConfClient *client, 
                                            guint        cnxn_id,
                                            GConfEntry  *entry,
                                            gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gwaei_ui_set_katakana_hiragana_conv(gconf_value_get_bool(value));
    else
      gwaei_ui_set_katakana_hiragana_conv(TRUE);
}


void do_spellcheck_pref_changed_action( GConfClient* client, 
                                        guint        cnxn_id,
                                        GConfEntry*  entry,
                                        gpointer     data     )
{
    GConfValue *value;
    value = gconf_entry_get_value(entry);

    if (value != NULL && value->type == GCONF_VALUE_BOOL)
      gwaei_sexy_ui_set_spellcheck(gconf_value_get_bool(value));
    else
      gwaei_sexy_ui_set_spellcheck(TRUE);
}


#define IS_HEXCOLOR(color) (regexec(&re_hexcolor, (color), 1, NULL, 0) == 0)

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
                                                      GCKEY_GWAEI_FONT_SIZE,
                                                      NULL                  );
        if (value != NULL && value->type == GCONF_VALUE_STRING)
          hex_string = gconf_value_get_string (value);
        else
          return;
      }

      guint red   = gwaei_2digithexstrtoint(hex_string[1], hex_string[2]);
      guint green = gwaei_2digithexstrtoint(hex_string[3], hex_string[4]);
      guint blue  = gwaei_2digithexstrtoint(hex_string[5], hex_string[6]);

      //The last portion of the key happens to be the widget id
      const char *key = gconf_entry_get_key(entry);
      const char *key_ptr = &key[strlen(key)];
      while (*key_ptr != '/')
        key_ptr--;
      key_ptr++;

      gwaei_ui_set_color_to_swatch(key_ptr, red, green, blue);

      gwaei_reload_tagtable_tags();
    }
}



//
//Preference initializations
//



void gwaei_prefs_initialize_preferences()
{
  g_type_init();

  char *string;

  GConfClient *client;
  client = gconf_client_get_default ();
    
    //Add directory listeners gwaei will be using
    gconf_client_add_dir   ( client, GCPATH_INTERFACE, 
                             GCONF_CLIENT_PRELOAD_NONE,
                             NULL                             );
    gconf_client_add_dir   ( client, GCPATH_GWAEI, 
                             GCONF_CLIENT_PRELOAD_NONE,
                             NULL                            );

    //Add preference change notifiers
    gconf_client_notify_add (client, GCKEY_GWAEI_LESS_RELEVANT_SHOW, 
                             do_less_relevant_show_pref_changed_action,
                             NULL, NULL, NULL                );

    gconf_client_notify_add (client, GCKEY_TOOLBAR_STYLE, 
                             do_toolbar_style_pref_changed_action,
                             NULL, NULL, NULL                 );

    gconf_client_notify_add (client, GCKEY_GWAEI_TOOLBAR_SHOW, 
                             do_toolbar_show_pref_changed_action,
                             NULL, NULL, NULL                );

    gconf_client_notify_add (client, GCKEY_GWAEI_FONT_SIZE, 
                             do_font_size_pref_changed_action,
                             NULL, NULL, NULL               );

    gconf_client_notify_add (client, GCKEY_GWAEI_ROMAN_KANA, 
                             do_roman_kana_conv_pref_changed_action,
                             NULL, NULL, NULL                );

    gconf_client_notify_add (client, GCKEY_GWAEI_HIRA_KATA, 
                             do_hira_kata_conv_pref_changed_action,
                             NULL, NULL, NULL                );

    gconf_client_notify_add (client, GCKEY_GWAEI_KATA_HIRA, 
                             do_kata_hira_conv_pref_changed_action,
                             NULL, NULL, NULL                );

    gconf_client_notify_add (client, GCKEY_GWAEI_SPELLCHECK, 
                             do_spellcheck_pref_changed_action,
                             NULL, NULL, NULL                );

    gconf_client_notify_add (client, GCKEY_GWAEI_MATCH_FG, 
                             do_color_value_changed_action,
                             NULL, NULL, NULL                );

    gconf_client_notify_add (client, GCKEY_GWAEI_MATCH_BG, 
                             do_color_value_changed_action,
                             NULL, NULL, NULL                );

    gconf_client_notify_add (client, GCKEY_GWAEI_HEADER_FG, 
                             do_color_value_changed_action,
                             NULL, NULL, NULL                );

    gconf_client_notify_add (client, GCKEY_GWAEI_HEADER_BG, 
                             do_color_value_changed_action,
                             NULL, NULL, NULL                );

    gconf_client_notify_add (client, GCKEY_GWAEI_COMMENT_FG, 
                             do_color_value_changed_action,
                             NULL, NULL, NULL                );

    gconf_client_notify_add (client,
                             GCKEY_GWAEI_ENGLISH_SOURCE, 
                             do_dictionary_source_gconf_key_changed_action,
                             NULL, NULL, NULL                 );

    gconf_client_notify_add (client,
                             GCKEY_GWAEI_KANJI_SOURCE, 
                             do_dictionary_source_gconf_key_changed_action,
                             NULL, NULL, NULL                 );

    gconf_client_notify_add (client,
                             GCKEY_GWAEI_NAMES_SOURCE, 
                             do_dictionary_source_gconf_key_changed_action,
                             NULL, NULL, NULL                 );

    gconf_client_notify_add (client,
                             GCKEY_GWAEI_RADICALS_SOURCE, 
                             do_dictionary_source_gconf_key_changed_action,
                             NULL, NULL, NULL                 );


    //Do an initial trigger of the notifications to set an initial state
    gconf_client_notify (client, GCKEY_GWAEI_TOOLBAR_SHOW);
    gconf_client_notify (client, GCKEY_TOOLBAR_STYLE);
    gconf_client_notify (client, GCKEY_GWAEI_FONT_SIZE);
    gconf_client_notify (client, GCKEY_GWAEI_KATA_HIRA);
    gconf_client_notify (client, GCKEY_GWAEI_HIRA_KATA);
    gconf_client_notify (client, GCKEY_GWAEI_ROMAN_KANA);
    gconf_client_notify (client, GCKEY_GWAEI_SPELLCHECK);
    gconf_client_notify (client, GCKEY_GWAEI_LESS_RELEVANT_SHOW);
    gconf_client_notify (client, GCKEY_GWAEI_MATCH_FG);
    gconf_client_notify (client, GCKEY_GWAEI_MATCH_BG);
    gconf_client_notify (client, GCKEY_GWAEI_HEADER_FG);
    gconf_client_notify (client, GCKEY_GWAEI_HEADER_BG);
    gconf_client_notify (client, GCKEY_GWAEI_COMMENT_FG);
    gconf_client_notify (client, GCKEY_GWAEI_ENGLISH_SOURCE);
    gconf_client_notify (client, GCKEY_GWAEI_KANJI_SOURCE);
    gconf_client_notify (client, GCKEY_GWAEI_NAMES_SOURCE);
    gconf_client_notify (client, GCKEY_GWAEI_PLACES_SOURCE);
    gconf_client_notify (client, GCKEY_GWAEI_RADICALS_SOURCE);

  g_object_unref(client);
}







