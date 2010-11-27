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
//! @file src/gsettings-preferences.c
//!
//! @brief Abstraction layer for gsettings preferences
//!
//! Allows access to gsettings with the ability to specify backup preferences upon
//! failure to get the preference value.
//!


#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <gio/gio.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>


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








