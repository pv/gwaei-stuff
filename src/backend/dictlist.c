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
#include <regex.h>
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
//! @return An allocated GwDictList that will be needed to be freed by gw_dictlist_free ()
//!
GwDictList* gw_dictlist_new ()
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
GList* gw_dictlist_get_selected()
{
    return _dictionaries->selected;
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
GList* gw_dictlist_get_dict_by_load_position (int request)
{
    GList* current = gw_dictlist_get_list();
    GwDictInfo *di = NULL;
    do
    {
       di = (GwDictInfo*) current->data;
       if (current && di && di->load_position == request)
         break;
       current = g_list_next (current);
    } while (current);
    return current;
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
GList* gw_dictlist_set_selected_by_load_position(int request)
{
    GList* current_dictionary = gw_dictlist_get_list();
    do
    {
       if (((GwDictInfo*)current_dictionary->data)->load_position == request)
         break;
       current_dictionary = g_list_next (current_dictionary);
    } while (current_dictionary != NULL);

    _dictionaries->selected = current_dictionary;
    return current_dictionary;
}


//!
//! @brief Adds a dictionary to the GwDictList
//!
//! This function does not install files or anything.  It just looks for the
//! dictionary by the name specified in the .waei folder and then sets up it's
//! initial state using gw_dictinfo_new().  It will also make sure the
//! dictionary is not added twice.
//!
//! @param name Name of the dictionary to add
//!
void gw_dictlist_add_dictionary(GwDictEngine ENGINE, char *name)
{
    GwDictInfo *di;
    if  (gw_dictlist_check_if_loaded_by_name (name) == FALSE)
      di = gw_dictinfo_new (ENGINE, name);
    if (di != NULL)
      _dictionaries->list = g_list_append (_dictionaries->list, di);
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
GList* gw_dictlist_remove_first()
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
void gw_dictlist_free ()
{
    while (_dictionaries->list != NULL)
      _dictionaries->list = gw_dictlist_remove_first();

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
//! @param request a const string to search for in the dictionary names
//! @return returns the GwDictInfo dictionary object of the result or null.
//!
GwDictInfo* gw_dictlist_get_dictinfo_by_name (GwDictEngine ENGINE, const char* request)
{
    GList *current;
    current = _dictionaries->list;
    GwDictInfo *di;
    di = NULL;

    while (current != NULL)
    {
      di = (GwDictInfo*) current->data;
      if (di->engine == ENGINE && strcmp (di->name, request) == 0)
        break;
      current = current->next;
      di = NULL;
    }

    return di;
}


//!
//! @brief Searchs for a specific dictionary by name to see if it is installed
//!
//! This as a convenience function to see if a dictionary is installed,
//! negating the need to see if it was added to the dictionary list and if
//! it has the GW_DICT_STATUS_INSTALLED status set.
//!
//! @param request a const string to search for in the dictionary names
//! @return returns true if the dictionary is installed
//!
gboolean gw_dictlist_check_if_loaded_by_name (char* name)
{
    GList *current = _dictionaries->list;
    GwDictInfo *di;

    while (current != NULL)
    {
      di = (GwDictInfo*) current->data;
      if (strcmp (di->name, name) == 0 && di->status == GW_DICT_STATUS_INSTALLED)
        return TRUE;
      current = current->next;
    }

    return FALSE;
}


//!
//! @brief Counts how many dictionaries in the dictionary list have a specific status
//!
//! The function will loop through each item in the dictionary list,
//! incrementing its count by 1 for each found dictionary, then returning the
//! number. The statuses include: GW_DICT_STATUS_INSTALLING, GW_DICT_STATUS_INSTALLED, GW_DICT_STATUS_NOT_INSTALLED,
//! GW_DICT_STATUS_UPDATING, GW_DICT_STATUS_UPDATED, and GW_DICT_STATUS_CANCELING. See dictionaries.h for the current list.
//!
//! @param status the integer status to check for
//! @return returns the number of dictionaries with the status
//!
int gw_dictlist_get_total_with_status (GwDictStatus status)
{
    GwDictInfo *di;
    GList *current = _dictionaries->list;
    int i = 0;

    while (current != NULL)
    {
      di = (GwDictInfo*) current->data;
      if (di->status == status)
      {
        i++;
      }
      current = current->next;
    }

    return i;
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
int gw_dictlist_get_total()
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
GList* gw_dictlist_get_list()
{
    return _dictionaries->list;
}


//!
//! @brief Set the initial status of the dictionary list
//!
//! The built in dictionaries are set up, then any additional manually installed
//! user dictionaries are searched for and set up.
//!
void gw_dictlist_initialize ()
{
    if (_dictionaries != NULL)
      gw_dictlist_free ();

    _dictionaries = gw_dictlist_new ();
       
    char** dictionaries = gw_io_get_dictionary_file_list ();
    char** atoms = NULL;
    GwDictEngine engine = -1;
    char *dictname = NULL;
    int i = 0;

    for (i = 0;  dictionaries[i] != NULL; i++)
    {
      atoms = g_strsplit_set (dictionaries[i], "/", 2);
      if (atoms != NULL && atoms[0] != NULL && atoms[1] != NULL) 
      {
        engine = gw_util_get_engine_from_enginename (atoms[0]);
        dictname = atoms[1];
        gw_dictlist_add_dictionary (engine, dictname);
      }
      g_strfreev(atoms);
    }
    g_strfreev(dictionaries);

    gw_dictlist_load_dictionary_order_from_pref ();
}


//!
//! @brief Does the required post processing to create the Mix dictionary
//!
//! THIS IS A PRIVATE FUNCTION. The function removes the current Mix dictionary
//! if it is there, then attempts to create a new mix dictionary.
//!
//! @see gw_io_create_mix_dictionary ()
//! @returns Returns true on success
//!
static gboolean _create_mix_dictionary()
{
/*
    GwDictInfo* mix;
    mix = gw_dictlist_get_dictinfo_by_id(GW_DICT_ID_MIX);
    g_remove (mix->path);
    mix->status = GW_DICT_STATUS_NOT_INSTALLED;

    GwDictInfo* kanji;
    kanji = gw_dictlist_get_dictinfo_by_id(GW_DICT_ID_KANJI);
    GwDictInfo* radicals;
    radicals = gw_dictlist_get_dictinfo_by_id(GW_DICT_ID_RADICALS);

    char *mpath = mix->path;
    char *kpath = kanji->path;
    char *rpath = radicals->path;

    mix->status = GW_DICT_STATUS_INSTALLING;

    gboolean ret;
    ret = gw_io_create_mix_dictionary(mpath, kpath, rpath);
   
    if (ret)
    {
      mix->status = GW_DICT_STATUS_INSTALLED;
      mix->total_lines =  gw_io_get_total_lines_for_path (mix->path);
    }
    else
      mix->status = GW_DICT_STATUS_ERRORED;

    return ret;
*/
    return FALSE;
}


//!
//! @brief Splits out the place names from the person names
//!
//! THIS IS A PRIVATE FUNCTION.  Function checks to see the Names dictionary
//! is installed, then tries to split out the Names and Places dictionaries.
//! If there is a failure, false is returned and the GError error gets set.
//!
//! @see gw_io_split_places_from_names_dictionary ()
//! @param error set a GError to the pointer when an error occurs
//! @returns Returns true on success
//!
static gboolean _split_places_from_names_dictionary(GError **error)
{
/*
    GwDictInfo* di_places;
    di_places = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_PLACES);
    GwDictInfo* di_names;
    di_names = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_NAMES);

    if (di_names->status == GW_DICT_STATUS_NOT_INSTALLED) return FALSE;

    char *raw = di_names->sync_path;
    char source[FILENAME_MAX];
    strncpy (source, di_names->path, FILENAME_MAX);
    strncat (source, ".new", FILENAME_MAX - strlen (di_names->path));

    char *names = di_names->path;
    char *places = di_places->path;
    
    gboolean ret = TRUE;

    if (ret)
      ret = gw_io_copy_with_encoding(raw, source, "EUC-JP","UTF-8", error);
       
    if (ret)
    {
      remove(names);
      remove(places);
      ret = gw_io_split_places_from_names_dictionary (source, names, places);
    }


    if (ret)
    {
      g_remove(source);
      di_places->status = GW_DICT_STATUS_INSTALLED;
      di_places->total_lines =  gw_io_get_total_lines_for_path (di_places->path);
      di_names->status  = GW_DICT_STATUS_INSTALLED;
      di_names->total_lines =  gw_io_get_total_lines_for_path (di_names->path);
    }
    else
    {
      g_remove(source);
      g_remove(names);
      remove(places);
      di_places->status = GW_DICT_STATUS_ERRORED;
      di_names->status  = GW_DICT_STATUS_ERRORED;
    }
    
    return ret;
*/
  return FALSE;
}


//!
//! @brief Figures out the postprocessing required by the dictionary name
//!
//! The name is parsed then it is decided if create_mix_dictionary () or
//! split_places_from_names_dictionary () should be called. The function
//! returns a GError on failure through the error parameter.
//!
//! @see create_mix_dictionary ()
//! @see split_places_from_names_dictionary ()
//! @param name Name of the dictionary that should be postprocessesed
//! @param error set a GError to the pointer when an error occurs
//! @returns Returns true on success
//!
void gw_dictlist_preform_postprocessing_by_name (char* name, GError **error)
{
/*
    //Sanity check
    GwDictInfo* di;
    di = gw_dictlist_get_dictinfo_by_name (name);
    if (di->status != GW_DICT_STATUS_INSTALLING &&
        di->status != GW_DICT_STATUS_UPDATING &&
        di->status != GW_DICT_STATUS_REBUILDING)
      return;
    if (di->status == GW_DICT_STATUS_CANCELING)
      return;

    //Setup some pointers
    GwDictInfo* k_di = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_KANJI);
    GwDictInfo* r_di = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_RADICALS);
    GwDictInfo* n_di = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_NAMES);

    //Rebuild the mix dictionary
    if ((di->id == GW_DICT_ID_RADICALS && k_di->status == GW_DICT_STATUS_INSTALLED) || 
        (di->id == GW_DICT_ID_KANJI    && r_di->status == GW_DICT_STATUS_INSTALLED) ||
        (di->id == GW_DICT_ID_MIX && k_di->status == GW_DICT_STATUS_INSTALLED && r_di->status == GW_DICT_STATUS_INSTALLED)
       )
    {
      di->status = GW_DICT_STATUS_REBUILDING;
      create_mix_dictionary ();
    }
    //Rebuild the names dictionary
    else if(di->id == GW_DICT_ID_NAMES)
    {
      di->status = GW_DICT_STATUS_REBUILDING;
      split_places_from_names_dictionary(error);
    }

    //Restore the previous state if the install wasn't canceled
    if (di->status != GW_DICT_STATUS_CANCELING)
    {
      di->status = GW_DICT_STATUS_INSTALLED;
    }
*/
}


//!
//! @brief All dictionaries with a specific status get switched to the requested one.
//!
//! Each dictionary in the dictionary list is examined and if it matches a
//! a specific status, it is switched to the requested one.  This is a good way
//! to quickly cleanup after possible installation errors to reset the
//! dictionaries to an uninstalled status state. The possible statuses include:
//! GW_DICT_STATUS_INSTALLING, GW_DICT_STATUS_INSTALLED, GW_DICT_STATUS_NOT_INSTALLED, GW_DICT_STATUS_UPDATING, GW_DICT_STATUS_UPDATED, and GW_DICT_STATUS_CANCELING.
//! See dictionaries.h for the current list.
//!
//! @param OLD const int of the current install status
//! @param NEW const int of the new install status
//!
void gw_dictlist_normalize_all_status_from_to (const GwDictStatus OLD, const GwDictStatus NEW)
{
    GList *current = _dictionaries->list;
    GwDictInfo *di;

    while (current != NULL)
    {
      di = (GwDictInfo*) current->data;
      if (di->status == OLD)
        di->status = NEW;
      current = current->next;
    }
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
    GwDictInfo *di = NULL;
    GList *list =  gw_dictlist_get_list();
    GList *iterator = list;
    int load_position = 0;

    _dictionaries->list = g_list_sort (list, _load_order_compare_function);

    for (iterator = list; iterator != NULL; iterator = iterator->next)
    {
      di = (GwDictInfo*) iterator->data;
      if (di->status != GW_DICT_STATUS_NOT_INSTALLED)
      {
        di->load_position = load_position;
        load_position++;
      }
    }
}


//
//! @brief Saves the current load order to the preferences
//
void gw_dictlist_save_dictionary_order_pref ()
{
    //Make sure things are sorted and normal
    _sort_and_normalize_dictlist_load_order ();

    //Declarations and initializations;
    char *load_order = NULL;
    GwDictInfo *di = NULL;
    GList *list =  gw_dictlist_get_list();
    GList *iterator = list;
    int max = GW_DICTLIST_MAX_DICTIONARIES;
    char **atom = (char**) malloc((max + 1) * sizeof(int));
    int i = 0;

    //Create the string to write to the prefs with the last one NULL terminated
    for (iterator = list; iterator != NULL && i < max; iterator = iterator->next)
    {
      atom[i] = g_strdup_printf ("%s/%s", gw_util_get_engine_name (di->engine), di->name);
      if (atom == NULL) { printf("Out of memory\n"); exit(1); }
      i++;
    }
    atom[i] = NULL;

    load_order = g_strjoinv (";", atom);

    //Free the used memory
    for (i = 0; i < max; i++)
    {
      free (atom[i]);
      atom[i] = NULL;
    }
    g_free (load_order);
    load_order = NULL;
}


//
//! @brief Loads the load order from the preferences
//
void gw_dictlist_load_dictionary_order_from_pref ()
{
    char load_order[1000];
    char **load_order_array;
    char **engine_name_array;
    char **ptr = NULL;
    GwDictEngine engine;
    char *name;
    GwDictInfo *di = NULL;
    int load_position = 0;
    
    gw_pref_get_string (load_order, GW_SCHEMA_DICTIONARY, GW_KEY_LOAD_ORDER, 1000);
    load_order_array = g_strsplit_set (load_order, ";", GW_DICTLIST_MAX_DICTIONARIES);
    
    for (ptr = load_order_array; *ptr != NULL; ptr++)
    {
      //Sanity checking
      if (*ptr == NULL || **ptr == '\0') { 
        printf("failed sanity check 1\n");
        gw_pref_reset_value (GW_SCHEMA_DICTIONARY, GW_KEY_LOAD_ORDER);
        gw_dictlist_load_dictionary_order_from_pref ();
        return;
      }

      engine_name_array = g_strsplit_set (*ptr, "/", -1); 

      //Sanity checking
      if (engine_name_array[0] == NULL || engine_name_array[1] == NULL)
      {
        printf("failed sanity check 2\n");
        gw_pref_reset_value (GW_SCHEMA_DICTIONARY, GW_KEY_LOAD_ORDER);
        gw_dictlist_load_dictionary_order_from_pref ();
        return;
      }

      engine = gw_util_get_engine_from_enginename (engine_name_array[0]);
      name = engine_name_array[1];

      //Sanity Checking
      if ((di = gw_dictlist_get_dictinfo_by_name (engine, name)) != NULL)
      {
        di->load_position = load_position;
        load_position++;
      }

      g_strfreev (engine_name_array);
      engine_name_array = NULL;
    }

    g_strfreev (load_order_array);
    load_order_array = NULL;
}




