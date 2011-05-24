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
//! @file src/dictlist-object.c
//!
//! @brief Management of dictionary objects
//!
//! The functions her generally manage the creation, destruction, and searching
//! of dictionaries.  The GwDictInfo objects also are used as a convenient
//! container for variables pointing towards download locations, install locations
//! etc.
//!


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libintl.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <gwaei/backend.h>

//Private member and method declarations
static GwDictList *_dictionaries;

static gboolean _create_mix_dictionary ();
static gboolean _split_places_from_names_dictionary (GError**);
static gint     _load_order_compare_function (gconstpointer, gconstpointer);
static void     _sort_and_normalize_dictlist_load_order ();


//!
//! @brief Constructor for a dictionary list object.
//!
//! This object is used to help manage groups of dictionaries in a sane way.
//!
//! @return An allocated GwDictList that will be needed to be freed by gw_dictinfolist_free ()
//!
GwDictList* gw_dictinfolist_new ()
{
    GwDictList *temp;

    //Allocate some memory
    if ((temp = malloc(sizeof(GwDictList))) == NULL) return NULL;

    temp->list = NULL;
    temp->selected = NULL;
    temp->mutex = g_mutex_new();

    return temp;
}


//!
//! @brief Get the currently selected GwDictInfo object
//!
//! A function used for abstracting what is the current dictionary for the GUI
//! instead of relying on the status of a particular widget.
//!
//! @return The position in the GwDictList of the GwDictInfo
//!
GList* gw_dictinfolist_get_selected ()
{
    return _dictionaries->selected;
}


//!
//! @brief Get the currently selected GwDictInfo object
//!
//! A function used for abstracting what is the current dictionary for the GUI
//! instead of relying on the status of a particular widget.
//!
//! @return The position in the GwDictList of the GwDictInfo
//!
GwDictInfo* gw_dictinfolist_get_selected_dictinfo ()
{
    GList *iter;
    GwDictInfo *di;

    iter = _dictionaries->selected;
    di = (GwDictInfo*) iter->data;

    return di;
}


//!
//! @brief Gets the dictionary by load position in the GUI
//!
//! Each dictionary has a load position recorded in the GUI when it is added to
//! it.  This function makes it easy for a GUI callback to find that exact
//! dictionary in the GwDictList.
//!
//! @param request The GUI load position of the desired dictionary
//! @return The position in the GwDictList of the GwDictInfo
//!
GList* gw_dictinfolist_get_dict_by_load_position (int request)
{
    GList *list;
    GList *iter;
    GwDictInfo *di;

    list = gw_dictinfolist_get_list();
    iter = list;
    do
    {
       di = (GwDictInfo*) iter->data;
       if (iter != NULL && di != NULL && di->load_position == request)
         break;
       iter = iter->next;
    } while (iter != NULL);

    return iter;
}


//!
//! @brief Gets the dictionary by load position in the GUI
//!
//! Each dictionary has a load position recorded in the GUI when it is added to
//! it.  This function makes it easy for a GUI callback to find that exact
//! dictionary in the GwDictList.
//!
//! @param request The GUI load position of the desired dictionary
//! @return The position in the GwDictList of the GwDictInfo
//!
GList* gw_dictinfolist_set_selected_by_load_position (int request)
{
    //Declarations
    GList* iter;
    GwDictInfo *di;

    for (iter = gw_dictinfolist_get_list(); iter != NULL; iter = g_list_next (iter))
    {
        di = iter->data;
       if (di->load_position == request)
         break;
    }

    _dictionaries->selected = iter;
    return iter;
}


//!
//! @brief Adds a dictionary to the GwDictList with sanity checks
//!
//! @param ENGINE Engine of the dictionary to add
//! @param FILENAME Name of the dictionary to add
//!
void gw_dictinfolist_add_dictionary (const GwEngine ENGINE, const char *FILENAME)
{
    //Sanity check
    if (gw_dictinfolist_check_if_loaded (ENGINE, FILENAME) == TRUE) return;

    //Declarations
    GwDictInfo *di;

    //Initializations
    di = gw_dictinfo_new (ENGINE, FILENAME);

    //Append to the dictionary list if was loadable
    if (di != NULL) _dictionaries->list = g_list_append (_dictionaries->list, di);
}


//!
//! @brief Removes the first dictionary in the GwDictList
//!
//! Does the work of asking for the GwDictInfo object to free it's memory
//! and then removing the empty object from the GwDictList dictionary list.
//!
//! @param name Name of the dictionary to add
//! @return Returns the dictionary object freed or null.
//!
GList* gw_dictinfolist_remove_first()
{
    GList *list;
    list = _dictionaries->list;

    gw_dictinfo_free(list->data);
    return g_list_delete_link(list, list);
}


//!
//! @brief Frees up the GwDictList dictionary list
//!
//! The work of freeing each individual dictionary is automatically handled,
//! removing the chance for mistakes.
//!
void gw_dictinfolist_free ()
{
    while (_dictionaries->list != NULL)
      _dictionaries->list = gw_dictinfolist_remove_first();

    g_mutex_free (_dictionaries->mutex);
    _dictionaries->mutex = NULL;

    g_free(_dictionaries);
    _dictionaries = NULL;
}

//!
//! @brief Searchs for a specific dictionary by name
//!
//! The function will go through each dictionary until it matches the requested
//! name.
//!
//! @param ENGINE The parsing engine of the dictionary wanted.  There can be
//!               dictionaries with the same name, but different engines.
//! @param NAME A constant string to search for in the dictionary names.
//! @returns The requested GwDictInfo object if found or null.
//!
GwDictInfo* gw_dictinfolist_get_dictinfo (const GwEngine ENGINE, const char* FILENAME)
{
    //Declarations
    GList *iter;
    GwDictInfo *di;

    for (iter = _dictionaries->list; iter != NULL; iter = iter->next)
    {
      di = (GwDictInfo*) iter->data;
      if (di->engine == ENGINE && strcmp (di->filename, FILENAME) == 0)
        break;
      di = NULL;
    }

    return di;
}

//!
//!  @brief  Gets a GwDictInfo object by a fuzzy string description.
//!          It can be either of the form "parser/dictionary" or 
//!          just be the dictionary name.  Case is ignored.
//!  @param FUZZY_DESCRIPTION A fuzzy description of the wanted dictionary.
//!  @returns A matching GwDictInfo object or NULL
//!
GwDictInfo* gw_dictinfolist_get_dictinfo_fuzzy (const char* FUZZY_DESCRIPTION)
{
    //Declarations
    GwDictInfo *di;

    //Initializations
    di = NULL;

    //Try getting the first dictionary if none is specified
    if (FUZZY_DESCRIPTION == NULL )
    {
      if (g_list_length (_dictionaries->list))
        di = _dictionaries->list->data;
      else
        di = NULL;
    }

    //Otherwise try getting a dictionary using a few different string parsers
    else
    {
      if (di == NULL)
        di = gw_dictinfolist_get_dictinfo_by_idstring (FUZZY_DESCRIPTION);
      if (di == NULL)
        di = gw_dictinfolist_get_dictinfo_by_filename (FUZZY_DESCRIPTION);
    }

    return di;
}

//!
//! @brief Grabs the first dictionary with a matching dictionary 
//!        filename.  If you have dictionaries with different 
//!        parsers but the same name, the others will not 
//!        be accessible with this function.
//! @param NAME A constant string to search for in the dictionary names.  
//!             This is a fuzzy search, ignoring ENGINE and case
//! @returns The requested GwDictInfo object if found or null.
//!
GwDictInfo* gw_dictinfolist_get_dictinfo_by_filename (const char* FILENAME)
{
    //Declarations
    GList *iter;
    GwDictInfo *di;

    for (iter = _dictionaries->list; iter != NULL; iter = iter->next)
    {
      di = (GwDictInfo*) iter->data;
      if (g_ascii_strcasecmp (di->filename, FILENAME) == 0)
        break;
      di = NULL;
    }

    return di;
}


//!
//! @brief Finds a dictionary by using an id string of the form of 
//!        "engine/dictionary". Case is ignored.
//! @param ENGINE_AND_FILENAME A string in the form "engine/dictionary"
//!                            used to search for a dictionary
//! @returns The requested GwDictInfo object if found or NULL.
//!
GwDictInfo* gw_dictinfolist_get_dictinfo_by_idstring (const char* ENGINE_AND_FILENAME)
{
    //Declarations
    GList *iter;
    GwDictInfo *di;
    char **tokens;
    char *filename;
    GwEngine engine;

    //Initializations
    iter = NULL;
    di = NULL;
    tokens = g_strsplit (ENGINE_AND_FILENAME, "/", 2);

    if (g_strv_length (tokens) == 2)
    {
      engine = gw_util_get_engine_from_enginename (tokens[0]);
      filename = tokens[1];

      for (iter = _dictionaries->list; iter != NULL; iter = iter->next)
      {
        di = (GwDictInfo*) iter->data;
        if (di->engine == engine && g_ascii_strcasecmp (di->filename, filename) == 0)
          break;
        di = NULL;
      }
    }

    g_strfreev (tokens);

    return di;
}



//!
//! @brief Searchs for a specific dictionary by name to see if it is installed
//!
//! This as a convenience function to see if a dictionary is installed,
//! negating the need to see if it was added to the dictionary list and if
//! it has the GW_DICT_STATUS_INSTALLED status set.
//!
//! @param NAME request a const string to search for in the dictionary names
//! @return returns true if the dictionary is installed
//!
gboolean gw_dictinfolist_check_if_loaded (const GwEngine ENGINE, const char* FILENAME)
{
    //Declarations
    GList *iter;
    GwDictInfo *di;

    //Return true if the dictionary exists
    for (iter = _dictionaries->list; iter != NULL; iter = iter->next)
    {
      di = (GwDictInfo*) iter->data;
      if (di->engine == ENGINE && strcmp (di->filename, FILENAME) == 0) 
        return TRUE;
    }

    //Otherwise it doesn't
    return FALSE;
}


//!
//! @brief Returns the number of dictionaries in the dictionary list
//!
//! This is not the number of dictionaries that are active.  It shows
//! how many dictionary names are recorded in the dictionary list.
//! By default, the default dictionaries appended to the list with
//! an UNGW_DICT_STATUS_INSTALLED status if they are unavailable. If the GW_DICT_MIX dictionary
//! is installed, Kanji and Radicals disappear from the GUI, but are still
//! in this list.
//!
//! @return Integer representing the number of installed dictionaries
//!
int gw_dictinfolist_get_total()
{
    return g_list_length(_dictionaries->list);
}


//!
//! @brief Returns a pointer to the list of dictionaries in GwDictList
//!
//! Gets the list variable inside of the GwDictList object and returns it.
//!
//! @return GList of the added dictionaries to the list.
//!
GList* gw_dictinfolist_get_list()
{
    return _dictionaries->list;
}


//!
//! @brief Set the initial status of the dictionary list
//!
//! The built in dictionaries are set up, then any additional manually installed
//! user dictionaries are searched for and set up.
//!
void gw_dictinfolist_initialize ()
{
    if (_dictionaries != NULL)
      gw_dictinfolist_free ();

    _dictionaries = gw_dictinfolist_new ();
       
    char** dictionaries = gw_io_get_dictionary_file_list ();
    char** atoms = NULL;
    GwEngine engine = -1;
    char *dictname = NULL;
    int i = 0;

    for (i = 0;  dictionaries[i] != NULL; i++)
    {
      atoms = g_strsplit_set (dictionaries[i], "/", 2);
      if (atoms != NULL && atoms[0] != NULL && atoms[1] != NULL) 
      {
        engine = gw_util_get_engine_from_enginename (atoms[0]);
        dictname = atoms[1];
        gw_dictinfolist_add_dictionary (engine, dictname);
      }
      g_strfreev(atoms);
    }
    g_strfreev(dictionaries);

    gw_dictinfolist_load_dictionary_order_from_pref ();
}


//!
//! @brief All dictionaries with a specific status get switched to the requested one
//!
//! This function is designed to be passed to g_list_sort and should not be used outside of
//! the dictlist-object.c file.  
//!
//! @param a Pointer to GwDictInfo object a
//! @param b Pointer to GwDictInfo object b
//! @returns Whether the position of a is less than (-1), equal (0) or greater than b (1)
//!
static gint _load_order_compare_function (gconstpointer a, gconstpointer b)
{
    //Declarations and initializations
    GwDictInfo *di_a = (GwDictInfo*) a;
    GwDictInfo *di_b = (GwDictInfo*) b;
    int lpa = di_a->load_position;
    int lpb = di_b->load_position;

    //Exception cases for positions less than 0.
    //We want negative numbers after everything else
    if (lpa < 0 && lpb >= 0)  return 1;
    if (lpa >= 0 && lpb < 0)  return -1;

    //Normal ordering
    if (lpa < lpb) return -1;
    else if (lpa == lpb) return 0;
    else return 1;
}


//
//! @brief Sorts the dictionaries by their load order and makes the numbers clean with no holes
//
static void _sort_and_normalize_dictlist_load_order ()
{
    //Declarations
    GwDictInfo *di;
    int load_position;
    GList *iter;

    //Initializations
    load_position = 0;

    //Sort the list
    _dictionaries->list = g_list_sort (_dictionaries->list, _load_order_compare_function);

    //Make sure there is no number skipping
    for (iter = _dictionaries->list; iter != NULL; iter = iter->next)
    {
      di = (GwDictInfo*) iter->data;
      di->load_position = load_position;
      load_position++;
    }
}


//
//! @brief Saves the current load order to the preferences
//
void gw_dictinfolist_save_dictionary_order_pref ()
{
    //Make sure things are sorted and normal
    _sort_and_normalize_dictlist_load_order ();

    //Declarations
    char *load_order;
    GwDictInfo *di;
    GList *iter;
    char **atom;
    int i;

    //Initializations;
    atom = (char**) malloc((GW_DICTLIST_MAX_DICTIONARIES + 1) * sizeof(char*));
    i = 0;

    //Create the string to write to the prefs with the last one NULL terminated
    for (iter = gw_dictinfolist_get_list (); iter != NULL && i < GW_DICTLIST_MAX_DICTIONARIES; iter = iter->next)
    {
      di = (GwDictInfo*) iter->data;
      atom[i] = g_strdup_printf ("%s/%s", gw_util_get_engine_name (di->engine), di->filename);
      if (atom == NULL) { printf("Out of memory\n"); exit(1); }
      i++;
    }
    atom[i] = NULL;

    load_order = g_strjoinv (";", atom);
    gw_pref_set_string_by_schema (GW_SCHEMA_DICTIONARY, GW_KEY_LOAD_ORDER, load_order);

    //Free the used memory
    g_strfreev (atom);
    atom = NULL;
    g_free (load_order);
    load_order = NULL;
}


//
//! @brief Loads the load order from the preferences
//
void gw_dictinfolist_load_dictionary_order_from_pref ()
{
    char load_order[1000];
    char **load_order_array;
    char **engine_name_array;
    char **ptr = NULL;
    GwEngine engine;
    char *name;
    GwDictInfo *di = NULL;
    int load_position = 0;
    
    gw_pref_get_string_by_schema (load_order, GW_SCHEMA_DICTIONARY, GW_KEY_LOAD_ORDER, 1000);
    load_order_array = g_strsplit_set (load_order, ";", GW_DICTLIST_MAX_DICTIONARIES);
    
    for (ptr = load_order_array; *ptr != NULL; ptr++)
    {
      //Sanity checking
      if (*ptr == NULL || **ptr == '\0') { 
        printf("failed sanity check 1\n");
        gw_pref_reset_value_by_schema (GW_SCHEMA_DICTIONARY, GW_KEY_LOAD_ORDER);
        gw_dictinfolist_load_dictionary_order_from_pref ();
        return;
      }

      engine_name_array = g_strsplit_set (*ptr, "/", -1); 

      //Sanity checking
      if (engine_name_array[0] == NULL || engine_name_array[1] == NULL)
      {
        printf("failed sanity check 2\n");
        gw_pref_reset_value_by_schema (GW_SCHEMA_DICTIONARY, GW_KEY_LOAD_ORDER);
        gw_dictinfolist_load_dictionary_order_from_pref ();
        return;
      }

      engine = gw_util_get_engine_from_enginename (engine_name_array[0]);
      name = engine_name_array[1];

      //Sanity Checking
      if ((di = gw_dictinfolist_get_dictinfo (engine, name)) != NULL)
      {
        di->load_position = load_position;
        load_position++;
      }

      g_strfreev (engine_name_array);
      engine_name_array = NULL;
    }

    g_strfreev (load_order_array);
    load_order_array = NULL;

    _sort_and_normalize_dictlist_load_order ();
}




