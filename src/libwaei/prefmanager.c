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
//! @file src/preferences.c
//!
//! @brief Abstraction layer for gsettings preferences
//!
//! Allows access to gsettings with the ability to specify backup preferences upon
//! failure to get the preference value.
//!


#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <stdlib.h>

#include <gio/gio.h>

#include <libwaei/libwaei.h>


//!
//! @brief Create a new LwPrefCallbackData object
//!
LwPrefCallbackData* lw_prefcallbackdata_new (const char *SCHEMA, const char *KEY, LwPrefCallback func, gpointer data)
{
    LwPrefCallbackData* temp;
    
    temp = (LwPrefCallbackData*) malloc(sizeof(LwPrefCallbackData));

    if (temp != NULL)
    {
      temp->schema = SCHEMA;
      temp->key = KEY;
      temp->func = func;
      temp->data = data;
    }

    return temp;
}


//!
//! @brief Free a new LwPrefCallbackData object
//!
void lw_prefcallbackdata_free (LwPrefCallbackData* pcd)
{
    free (pcd);
}


//!
//! @brief Checks if a LwPrefCallbackData already exists in the prefmanager list
//!
gboolean lw_prefmanager_callback_exists (LwPrefManager *pm, const char *SCHEMA, const char *KEY, LwPrefCallback func)
{
    LwPrefCallbackData *pcd;
    GList *iter;

    for (iter = pm->callbacklist; iter != NULL; iter = iter->next)
    {
      pcd = (LwPrefCallbackData*) iter->data;
      if (pcd != NULL && pcd->schema == SCHEMA && pcd->key == KEY && pcd->func == func)
        return TRUE;
      pcd = NULL;
    }

    return FALSE;
}


//!
//!  @brief Used for removing all callbacks with a specific data.  The data
//!         generally represents information for a window.
//!
void lw_prefmanager_destroy_callbacks_for_matching_data (LwPrefManager *pm, gpointer data)
{
    GList *iter;
    GList *link;
    LwPrefCallbackData *pcd;

    iter = pm->callbacklist;

    while (iter != NULL)
    {
      pcd = (LwPrefCallbackData*) iter->data;
      if (pcd != NULL && pcd->data == data)
      {
        lw_prefcallbackdata_free (pcd);
        link = iter;
        iter = iter->next;
        pm->callbacklist = g_list_delete_link (pm->callbacklist, link);
      }
      else
      {
        iter = iter->next;
      }
    }
}


//!
//! @brief Adds a LwPrefCallbackData objcet to the list of callbacks in the prefmanager.  This is meant to be used for outside of the
//!        preferences for everything that relies on a preference separately.
//!
void lw_prefmanager_add_callback (LwPrefManager *pm, const char *SCHEMA, const char *KEY, LwPrefCallback func, gpointer data)
{
    LwPrefCallbackData *pcd;

    if (lw_prefmanager_callback_exists (pm, SCHEMA, KEY, func) == FALSE)
    {
      pcd = lw_prefcallbackdata_new (SCHEMA, KEY, func, data);
      if (pcd != NULL)
      {
        pm->callbacklist = g_list_append (pm->callbacklist, pcd);
      }
    }
}


void lw_prefmanager_free_callbacks (LwPrefManager *pm)
{
    GList *iter;
    LwPrefCallbackData *pcd;

    for (iter = pm->callbacklist; iter != NULL; iter = iter->next)
    {
      pcd = (LwPrefCallbackData*) iter->data;
      if (pcd != NULL)
        lw_prefcallbackdata_free (pcd);
      iter->data = NULL;
    }

    g_list_free (pm->callbacklist);
    pm->callbacklist = NULL;
}


void lw_prefmanager_free_settings (LwPrefManager *pm)
{
    GList *iter;
    GSettings *settings;

    for (iter = pm->settingslist; iter != NULL; iter = iter->next)
    {
      settings = (GSettings*) iter->data;
      g_object_unref (settings);
      iter->data = NULL;
    }

    g_list_free (pm->settingslist);
    pm->settingslist = NULL;
}


//!
//! @brief sets up preferences and makes sure they are in a sane state
//!
LwPrefManager* lw_prefmanager_new ()
{
  LwPrefManager *temp;
  char version[50];

  temp = (LwPrefManager*) malloc(sizeof(LwPrefManager));

  if (temp != NULL)
  {
    temp->settingslist = NULL;
    temp->callbacklist = NULL;
    temp->mutex = g_mutex_new ();

    guint id;
    GSettings *settings;

    settings = lw_prefmanager_get_settings_object (temp, LW_SCHEMA_BASE);
    id = lw_prefmanager_add_change_listener (settings, LW_KEY_TOOLBAR_SHOW, lw_prefmanager_boolean_changed_cb, temp);
/*

number = g_signal_connect (widget, "clicked", callback, data);
g_signal_handler_block (widget, number);

    gchar *dictionary_load_order;
    gchar *dictionary_english_source;
    gchar *dictionary_kanji_source;
    lw_prefmanager_add_boolean_pref_changed_listener (temp, SCHEMA, KEY, &toolbar_show);
    gchar *dictionary_names_places_source;
    lw_prefmanager_add_boolean_pref_changed_listener (temp, SCHEMA, KEY, &toolbar_show);
    gchar *dictionary_examples_source;
    lw_prefmanager_add_boolean_pref_changed_listener (temp, SCHEMA, KEY, &toolbar_show);

    gboolean use_global_document_font;
    lw_prefmanager_add_boolean_pref_changed_listener (temp, SCHEMA, KEY, &toolbar_show);
    gchar *custom_document_font;
    lw_prefmanager_add_boolean_pref_changed_listener (temp, SCHEMA, KEY, &toolbar_show);
    gint magnification;
    lw_prefmanager_add_boolean_pref_changed_listener (temp, SCHEMA, KEY, &toolbar_show);
    
    gchar *comment_foreground;
    lw_prefmanager_add_boolean_pref_changed_listener (temp, SCHEMA, KEY, &toolbar_show);
    gchar *comment_background;
    lw_prefmanager_add_boolean_pref_changed_listener (temp, SCHEMA, KEY, &toolbar_show);
    gchar *match_foreground;
    lw_prefmanager_add_boolean_pref_changed_listener (temp, SCHEMA, KEY, &toolbar_show);
    gchar *match_background;
    lw_prefmanager_add_boolean_pref_changed_listener (temp, SCHEMA, KEY, &toolbar_show);
    gchar *header_foreground;
    lw_prefmanager_add_boolean_pref_changed_listener (temp, SCHEMA, KEY, &toolbar_show);
    gchar *header_background;
    lw_prefmanager_add_boolean_pref_changed_listener (temp, SCHEMA, KEY, &toolbar_show);
*/


/*
    lw_prefmanager_get_string_by_schema (temp, version, LW_SCHEMA_BASE, LW_KEY_PROGRAM_VERSION, 50);
    if (strcmp(version, VERSION) != 0)
    {
      lw_prefmanager_set_string_by_schema (temp, LW_SCHEMA_BASE, LW_KEY_PROGRAM_VERSION, VERSION);
      lw_prefmanager_reset_value_by_schema (temp, LW_SCHEMA_DICTIONARY, LW_KEY_ENGLISH_SOURCE);
      lw_prefmanager_reset_value_by_schema (temp, LW_SCHEMA_DICTIONARY, LW_KEY_KANJI_SOURCE);
      lw_prefmanager_reset_value_by_schema (temp, LW_SCHEMA_DICTIONARY, LW_KEY_NAMES_PLACES_SOURCE);
      lw_prefmanager_reset_value_by_schema (temp, LW_SCHEMA_DICTIONARY, LW_KEY_EXAMPLES_SOURCE);
    }
    */
  }

  return temp;
}


//!
//! @brief Clears memory used by the preferences
//!
void lw_prefmanager_free (LwPrefManager *pm)
{

    lw_prefmanager_free_settings (pm);
    lw_prefmanager_free_callbacks (pm);

    free (pm);
}


//!
//! @brief Fetches a gsettings object by id, and stores it, using the cached one if available
//!
GSettings* lw_prefmanager_get_settings_object (LwPrefManager *pm, const char *SCHEMA)
{
    //Declarations
    GList *iter;
    char *schema;
    GSettings *settings;

    //Initializations
    settings = NULL;

    //Look for an already created gsetting object
    for (iter = pm->settingslist; iter != NULL; iter = iter->next)
    {
      settings = (GSettings*) iter->data;
      g_object_get (G_OBJECT (settings), "schema", &schema, NULL);
      if (schema != NULL && strcmp(schema, SCHEMA) == 0)
        break;
      if (schema != NULL)
        g_free (schema);
      settings = NULL;
    }

    //If not found, create our own and add it to the list
    if (settings == NULL)
    {
      settings = g_settings_new (SCHEMA);
      if (settings != NULL)
      {
        pm->settingslist = g_list_append (pm->settingslist, settings);
      }
    }

    return settings;
}


//!
//! @brief Resets a value in a key
//!
//! @param schema A string identifying the schema there the key is
//! @param key A string identifying the key to reset
//!
void lw_prefmanager_reset_value (GSettings* settings, const char *key)
{
    g_settings_reset (settings, key);
}


//!
//! @brief Resets a value in a key
//!
//! @param schema A string identifying the schema there the key is
//! @param key A string identifying the key to reset
//!
void lw_prefmanager_reset_value_by_schema (LwPrefManager* pm, const char* schema, const char *key)
{
    GSettings* settings;
    
    settings = lw_prefmanager_get_settings_object (pm, schema);
    if (settings != NULL)
    {
      lw_prefmanager_reset_value (settings, key);
    }
}


//!
//! @brief Returns an integer from the preference backend 
//!
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param backup the value to return on failure
//!
int lw_prefmanager_get_int (GSettings *settings, const char *key)
{
    return g_settings_get_int (settings, key);
}


//!
//! @brief Returns an integer from the preference backend 
//!
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param backup the value to return on failure
//!
int lw_prefmanager_get_int_by_schema (LwPrefManager* pm, const char* schema, const char *key)
{
    GSettings* settings;
    int value;
    
    value = 0;
    settings = lw_prefmanager_get_settings_object (pm, schema);

    if (settings != NULL)
    {
      value = lw_prefmanager_get_int (settings, key);
    }

    return value;
}


//!
//! @brief Sets the int to the key in the preferences backend
//!
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void lw_prefmanager_set_int (GSettings *settings, const char *key, const int request)
{
    g_settings_set_int (settings, key, request);
}


//!
//! @brief Sets the int to the key in the preferences backend
//!
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void lw_prefmanager_set_int_by_schema (LwPrefManager* pm, const char* schema, const char *key, const int request)
{
    GSettings* settings;
    
    settings = lw_prefmanager_get_settings_object (pm, schema);
    if (settings != NULL)
    {
      lw_prefmanager_set_int (settings, key, request);
    }
}


//!
//! @brief Returns an boolean from the preference backend 
//!
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//!
gboolean lw_prefmanager_get_boolean (GSettings *settings, const char *key)
{
    return g_settings_get_boolean (settings, key);
}


//!
//! @brief Returns an boolean from the preference backend 
//!
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//!
gboolean lw_prefmanager_get_boolean_by_schema (LwPrefManager* pm, const char* schema, const char *key)
{
    GSettings* settings;
    gboolean value;
    
    settings = lw_prefmanager_get_settings_object (pm, schema);
    value = FALSE; 

    if (settings != NULL)
    {
      value = lw_prefmanager_get_boolean (settings, key);
    }

    return value;
}


//!
//! @brief Sets the boolean to the key in the preferences backend
//!
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void lw_prefmanager_set_boolean (GSettings *settings, const char *key, const gboolean request)
{
    g_settings_set_boolean (settings, key, request);
}


//!
//! @brief Sets the boolean to the key in the preferences backend
//!
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void lw_prefmanager_set_boolean_by_schema (LwPrefManager* pm, const char* schema, const char *key, const gboolean request)
{
    GSettings* settings;
    
    settings = lw_prefmanager_get_settings_object (pm, schema);

    if (settings != NULL)
    {
      lw_prefmanager_set_boolean (settings, key, request);
    }
}


//!
//! @brief Returns an string from the preference backend 
//!
//! @output string to copy the pref to
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param backup the value to return on failure
//! @param n The max characters to copy to output
//!
void lw_prefmanager_get_string (char *output, GSettings *settings, const char *key, const int n)
{
    gchar *value = NULL; 

    value = g_settings_get_string (settings, key);
    g_assert (value != NULL);
    strncpy(output, value, n);

    g_free (value);
    value = NULL;
}


//!
//! @brief Returns an string from the preference backend 
//!
//! @output string to copy the pref to
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param backup the value to return on failure
//! @param n The max characters to copy to output
//!
void lw_prefmanager_get_string_by_schema (LwPrefManager* pm, char *output, const char *schema, const char *key, const int n)
{
    GSettings* settings;
    
    settings = lw_prefmanager_get_settings_object (pm, schema);

    lw_prefmanager_get_string (output, settings, key, n);
}


//!
//! @brief Sets the string to the key in the preferences backend
//!
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void lw_prefmanager_set_string (GSettings *settings, const char *key, const char* request)
{
    g_settings_set_string (settings, key, request);
}


//!
//! @brief Sets the string to the key in the preferences backend
//!
//! @param schema The key to use to look up the pref
//! @param key The key to use to look up the pref
//! @param request The value to set
//!
void lw_prefmanager_set_string_by_schema (LwPrefManager *pm, const char* schema, const char *key, const char* request)
{
    GSettings *settings;

    settings = lw_prefmanager_get_settings_object (pm, schema);

    if (settings != NULL)
    {
      lw_prefmanager_set_string (settings, key, request);
    }
}


//!
//! @brief Adds a preference change listener for the selected key
//!
//! @param schema The key to use to look up the pref
//! @param key The preference key
//! @param callback_function The function to call when the key changes
//! @param data The userdata to pass to the callback function
//!
//! @returns A gulong used to remove a signal later if desired
//!
gulong lw_prefmanager_add_change_listener (GSettings *settings, const char *key, void (*callback_function) (GSettings*, gchar*, gpointer), gpointer data)
{
    g_assert (key != NULL);

    //Declarations
    char *signal_name;
    gulong id;
    GVariant *value;

    //Set up the signal
    signal_name = g_strdup_printf ("changed::%s", key);
    id = g_signal_connect (G_OBJECT (settings), signal_name, G_CALLBACK (callback_function), data);

    //Trigger an initial fire of the change listener
    value = g_settings_get_value (settings, key);
    if (value != NULL) g_settings_set_value (settings, key, value);

    //Cleanup
    g_variant_unref (value);
    value = NULL;
    g_free (signal_name);
    signal_name = NULL;

    return id;
}


//!
//! @brief Adds a preference change listener for the selected key
//!
//! @param schema The key to use to look up the pref
//! @param key The preference key
//! @param callback_function The function to call when the key changes
//! @param data The userdata to pass to the callback function
//!
//! @returns A gulong used to remove a signal later if desired
//!
gulong lw_prefmanager_add_change_listener_by_schema (LwPrefManager *pm, const char* schema, const char *key, void (*callback_function) (GSettings*, gchar*, gpointer), gpointer data)
{
    g_assert (schema != NULL && key != NULL);

    GSettings *settings;
    gulong id;

    settings = lw_prefmanager_get_settings_object (pm, schema);
    id = lw_prefmanager_add_change_listener (settings, key, callback_function, data);

    return id;
}


//!
//! @brief Used to remove a listener
//!
//! @param schema A schema of the GSettings object the signal was connected to
//! @param id The signalid returned by lw_prefmanager_add_change_listener
//!
void lw_prefmanager_remove_change_listener (GSettings *settings, gulong id)
{
    if (g_signal_handler_is_connected (G_OBJECT (settings), id))
      g_signal_handler_disconnect (G_OBJECT (settings), id);
}


//!
//! @brief Used to remove a listener
//!
//! @param schema A schema of the GSettings object the signal was connected to
//! @param id The signalid returned by lw_prefmanager_add_change_listener
//!
void lw_prefmanager_remove_change_listener_by_schema (LwPrefManager *pm, const char* schema, gulong id)
{
    GSettings *settings;

    settings = lw_prefmanager_get_settings_object (pm, schema);

    if (settings != NULL)
    {
      lw_prefmanager_remove_change_listener (settings, id);
    }
}






