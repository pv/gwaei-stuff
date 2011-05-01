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
//!  @file src/dictinstlist.c
//!
//!  @brief Management of dictinst objects
//!


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <libintl.h>

#include <glib.h>

#include <gwaei/backend.h>


static GList *_list = NULL;


//!
//! @brief Get the internal dictinstlist as a GList
//!
GList* gw_dictinstlist_get_list ()
{
  return _list;
}

//!
//! @brief Sets up the built-in installabale dictionaries
//!
void gw_dictinstlist_initialize ()
{
  GwDictInst *di = NULL;

  di = gw_dictinst_new_using_pref_uri (
    "English",
    gettext("English"),
    gettext("English Dictionary"),
    gettext("This is the edict dictionary."),
    GW_SCHEMA_DICTIONARY,
    GW_KEY_ENGLISH_SOURCE,
    GW_ENGINE_EDICT,
    GW_COMPRESSION_GZIP,
    GW_ENCODING_EUC_JP,
    FALSE,
    FALSE,
    TRUE 
  );
  _list = g_list_append (_list, di);

  di = gw_dictinst_new_using_pref_uri (
    "Kanji",
    gettext("Kanji"),
    gettext("Kanji Dictionary"),
    gettext("This is the edict dictionary."),
    GW_SCHEMA_DICTIONARY,
    GW_KEY_KANJI_SOURCE,
    GW_ENGINE_KANJI,
    GW_COMPRESSION_GZIP,
    GW_ENCODING_EUC_JP,
    FALSE,
    TRUE,
    TRUE 
  );
  _list = g_list_append (_list, di);

  di = gw_dictinst_new_using_pref_uri (
    "Names and Places",
    gettext("Names and Places"),
    gettext("Names and Places Dictionary"),
    gettext("This is the edict dictionary."),
    GW_SCHEMA_DICTIONARY,
    GW_KEY_NAMES_PLACES_SOURCE,
    GW_ENGINE_EDICT,
    GW_COMPRESSION_GZIP,
    GW_ENCODING_EUC_JP,
    TRUE,
    FALSE,
    TRUE 
  );
  _list = g_list_append (_list, di);

  di = gw_dictinst_new_using_pref_uri (
    "Examples",
    gettext("Examples"),
    gettext("Exmaples Dictionary"),
    gettext("This is the example dictionary."),
    GW_SCHEMA_DICTIONARY,
    GW_KEY_EXAMPLES_SOURCE,
    GW_ENGINE_EXAMPLES,
    GW_COMPRESSION_GZIP,
    GW_ENCODING_EUC_JP,
    FALSE,
    FALSE,
    TRUE 
  );
  _list = g_list_append (_list, di);

  di = gw_dictinst_new (
    "",
    gettext("Other"),
    gettext("Other Dictionary"),
    gettext("Install a custom dictionary."),
    "",
    GW_ENGINE_UNKNOWN,
    GW_COMPRESSION_NONE,
    GW_ENCODING_UTF8,
    FALSE,
    FALSE,
    FALSE
  );
  _list = g_list_append (_list, di);
}


void gw_dictinstlist_free ()
{
    GList *iter = _list;
    GwDictInst *di = NULL;
    while (iter != NULL)
    {
      di = (GwDictInst*) iter->data;
      gw_dictinst_free (di);
      iter->data = NULL;
      iter = iter->next;
    }
    g_list_free (_list);
    _list = NULL;
}


//!
//! @brief Checks to see if the current DictInstList is installation ready
//!
gboolean gw_dictinstlist_data_is_valid ()
{
    //Declarations
    GList *iter;
    GwDictInst* di;
    int number_selected;

    //Initializations
    number_selected = 0;

    for (iter = _list; iter != NULL; iter = iter->next)
    {
      di = (GwDictInst*) iter->data;
      if (!gw_dictinst_data_is_valid (di) && di->selected) return FALSE;
      if (di->selected) number_selected++;
      printf("number selected: %d\n", number_selected);
    }
    return (number_selected > 0);
}


//!
//!  @brief  Gets a GwDictInst object by a fuzzy string description.
//!          It can be either of the form "parser/dictionary" or 
//!          just be the dictionary name.  Case is ignored.
//!  @param FUZZY_DESCRIPTION A fuzzy description of the wanted dictionary.
//!  @returns A matching GwDictInst object or NULL
//!
GwDictInst* gw_dictinstlist_get_dictinst_fuzzy (const char* FUZZY_DESCRIPTION)
{
    //Declarations
    GwDictInst *di;

    //Initializations
    di = NULL;

    //Try getting the first dictionary if none is specified
    if (FUZZY_DESCRIPTION == NULL )
    {
      if (g_list_length (_list))
        di = _list->data;
      else
        di = NULL;
    }

    //Otherwise try getting a dictionary using a few different string parsers
    else
    {
      if (di == NULL)
        di = gw_dictinstlist_get_dictinst_by_idstring (FUZZY_DESCRIPTION);
      if (di == NULL)
        di = gw_dictinstlist_get_dictinst_by_filename (FUZZY_DESCRIPTION);
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
//! @returns The requested GwDictInst object if found or null.
//!
GwDictInst* gw_dictinstlist_get_dictinst_by_filename (const char* FILENAME)
{
    //Declarations
    GList *iter;
    GwDictInst *di;

    for (iter = _list; iter != NULL; iter = iter->next)
    {
      di = (GwDictInst*) iter->data;
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
//! @returns The requested GwDictInst object if found or NULL.
//!
GwDictInst* gw_dictinstlist_get_dictinst_by_idstring (const char* ENGINE_AND_FILENAME)
{
    //Declarations
    GList *iter;
    GwDictInst *di;
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

      for (iter = _list; iter != NULL; iter = iter->next)
      {
        di = (GwDictInst*) iter->data;
        if (di->engine == engine && g_ascii_strcasecmp (di->filename, filename) == 0)
          break;
        di = NULL;
      }
    }

    g_strfreev (tokens);

    return di;
}


