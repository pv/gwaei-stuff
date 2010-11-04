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
//! @file src/none-preferences.c
//!
//! @brief File that just returns the default pref option
//!
//! Unlike the other preference backends, this is not a backend.
//! It relies on the default values set in include/definitions.h
//! to do anything.
//!



#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/preferences.h>


int gw_pref_get_int (char *key, int backup)
{
    return backup;
}

int gw_pref_get_default_int (char *key, int backup)
{
    return backup;
}

void gw_pref_set_int (char *key, int request)
{
    return;
}


gboolean gw_pref_get_boolean (char *key, gboolean backup)
{
    return backup;
}


gboolean gw_pref_get_default_boolean (char *key, gboolean backup)
{
    return backup;
}


void gw_pref_set_boolean (char *key, gboolean request)
{
  return;
}


char* gw_pref_get_string (char *output, char *key, char* backup, int n)
{
  strncpy(output, backup, n);
  return output;
}


char* gw_pref_get_default_string (char *key, char* backup, char* fallback, int length)
{
    return fallback;
}


void gw_pref_set_string (char *key, const char* request)
{
  return;
}


//
//Preference initializations
//

void gw_prefs_initialize_preferences()
{
  return;
}




//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_dictionary_source_gconf_key_changed_action (gpointer client,
                                                    guint cnxn_id,
                                                    gpointer entry,
                                                    gpointer data       )
{
      gw_ui_set_dictionary_source (data, (char*) entry);
}


//!
//! @brief Adds a preference change listener for the selected key
//!
//! @param key The preference key
//! @param callback_function The function to call when the key changes
//! @param data The userdata to pass to the callback function
//!
void gw_prefs_add_change_listener (const char *key, void (*callback_function) (gpointer, guint, gpointer, gpointer), gpointer data)
{
  char uri[100];
  gw_util_strncpy_fallback_from_key(uri, key, 100);
  (*callback_function) (NULL, 0, uri, data);
}



