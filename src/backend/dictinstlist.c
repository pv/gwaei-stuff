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
      if (!gw_dictinst_data_is_valid (di)) return FALSE;
      if (di->selected) number_selected++;
    }
    return (number_selected > 0);
}

