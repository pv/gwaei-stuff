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
//!  @file src/dictinfo-object.c
//!
//!  @brief Management of dictionary objects
//!
//!  The functions her generally manage the creation, destruction, and searching
//!  of dictionaries.  The GwDictInfo objects also are used as a convenient
//!  container for variables pointing towards download locations, install locations
//!  etc.
//!


#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <libintl.h>

#include <glib.h>

#include <gwaei/backend.h>

//Static declarations
static gboolean _overlay_default_builtin_dictionary_settings (GwDictInfo*);


//!
//! @brief Creates a new GwDictInfo object
//!
//! Memory for a new GwDictInfo object will be allocated, and the name passed
//! to the function as a param will be searched for in the .waei folder.  If 
//! it is a known name, the long name of the object will betranslated and if
//! it is installed, the status variable set to GW_DICT_STATUS_INSTALLED.
//!
//! @param name Name of the object to create
//! @return An allocated GwDictInfo that will be needed to be freed by gw_dictinfo_free ()
//!
GwDictInfo* gw_dictinfo_new (GwDictEngine ENGINE, char *name)
{
    g_assert (ENGINE >= 0 && ENGINE <= GW_DICT_ENGINE_TOTAL && name != NULL);

    GwDictInfo *temp;

    //Allocate some memory
    if ((temp = malloc(sizeof(GwDictInfo))) == NULL) return NULL;

    temp->load_position = -1;

    //Initialize the members
    temp->name = NULL;
    temp->short_name = NULL;
    temp->long_name = NULL;
    temp->total_lines = 0;
    temp->engine = ENGINE;
    temp->status = -1;

    //Copy the name of the dictionary over
    temp->name = g_strdup_printf ("%s", name);

    if (!_overlay_default_builtin_dictionary_settings (temp))
    {
      temp->long_name = g_strdup_printf (gettext("%s Dictionary"), name);
      temp->short_name = g_strdup_printf ("%s", name);
      temp->load_position = -1;
    }

    char *path = g_build_filename (gw_util_get_directory_for_engine (ENGINE), name, NULL);

    //Set the initial installation status
    if (g_file_test(path, G_FILE_TEST_IS_REGULAR) == TRUE)
      temp->status = GW_DICT_STATUS_INSTALLED;
    else
      temp->status = GW_DICT_STATUS_NOT_INSTALLED;

    g_free (path);
    path = NULL;

    temp->cached_resultlines = NULL;
    temp->current_resultline = NULL;

    //Done
    return temp;
}


//!
//! @brief Releases a GwDictInfo object from memory.
//!
//! Takes care of any of the work needed to release a GwDictInfo object from
//! memory.
//!
//! @param di GwDictInfo object to free
//!
void gw_dictinfo_free(GwDictInfo* di)
{
    g_free (di->name);
    di->name = NULL;

    g_free (di->short_name);
    di->short_name = NULL;

    g_free (di->long_name);
    di->long_name = NULL;

    free (di);
    di = NULL;
}


static gboolean _overlay_default_builtin_dictionary_settings (GwDictInfo *di)
{
    g_assert (di != NULL);

    if (di->engine == GW_DICT_ENGINE_EDICT)
    {
      if (strcmp(di->name, "English") == 0)
      {
        di->long_name = g_strdup_printf ("%s", gettext("English Dictionary"));
        di->short_name = g_strdup_printf ("%s", gettext("English"));
        di->load_position = 1;
      }
      else if (strcmp(di->name, "Names") == 0)
      {
        di->long_name = g_strdup_printf ("%s", gettext("Names Dictionary"));
        di->short_name = g_strdup_printf ("%s", gettext("Names"));
        di->load_position = 3;
      }
      else if (strcmp(di->name, "Places") == 0)
      {
        di->long_name = g_strdup_printf ("%s", gettext("Places Dictionary"));
        di->short_name = g_strdup_printf ("%s", gettext("Places"));
        di->load_position = 4;
      }
    }
    else if (di->engine == GW_DICT_ENGINE_KANJI)
    {
      if (strcmp(di->name, "Kanji") == 0)
      {
        di->long_name = g_strdup_printf ("%s", gettext("Kanji Dictionary"));
        di->short_name = g_strdup_printf ("%s", gettext("Kanji"));
        di->load_position = 2;
      }
    }
    else if (di->engine == GW_DICT_ENGINE_EXAMPLES)
    {
      if (strcmp(di->name, "Examples") == 0)
      {
        di->long_name = g_strdup_printf ("%s", gettext("Examples Dictionary"));
        di->short_name = g_strdup_printf ("%s", gettext("Examples"));
        di->load_position = 5;
      }
    }

    return (di->load_position > -1);
}

