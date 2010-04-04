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

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/resultline-object.h>
#include <gwaei/dictinfo-object.h>


static int id_increment = 100;


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
GwDictInfo* gw_dictinfo_new (char *name)
{
    GwDictInfo *temp;

    //Allocate some memory
    if ((temp = malloc(sizeof(GwDictInfo))) == NULL) return NULL;

    int remaining;
    temp->load_position = -1;

    temp->name = NULL;
    temp->short_name = NULL;
    temp->long_name = NULL;

    //Copy the name of the dictionary over
    remaining = MAX_DICTIONARY;
    temp->name = g_strdup_printf ("%s", name);

    if (strcmp(name, "English") == 0)
    {
      temp->long_name = gettext("English Dictionary");
      temp->short_name = gettext("English");
    }
    else if (strcmp(name, "Spanish") == 0)
    {
      temp->long_name = gettext("Spanish Dictionary");
      temp->short_name = gettext("Spanish");
    }
    else if (strcmp(name, "Mix") == 0)
    {
      temp->long_name = gettext("Kanji Dictionary");
      temp->short_name = gettext("Kanji");
    }
    else if (strcmp(name, "Kanji") == 0)
    {
      temp->long_name = gettext("Kanji Dictionary");
      temp->short_name = gettext("Kanji");
    }
    else if (strcmp(name, "Radicals") == 0)
    {
      temp->long_name = gettext("Radicals Dictionary");
      temp->short_name = gettext("Radicals");
    }
    else if (strcmp(name, "Places") == 0)
    {
      temp->long_name = gettext("Places Dictionary");
      temp->short_name = gettext("Places");
    }
    else if (strcmp(name, "Names") == 0)
    {
      temp->long_name = gettext("Names Dictionary");
      temp->short_name = gettext("Names");
    }
    else if (strcmp(name, "Examples") == 0)
    {
      temp->long_name = gettext("Examples Dictionary");
      temp->short_name = gettext("Examples");
    }
    else if (strcmp(name, "French") == 0)
    {
      temp->long_name = gettext("French Dictionary");
      temp->short_name = gettext("French");
    }
    else if (strcmp(name, "German") == 0)
    {
      temp->long_name = gettext("German Dictionary");
      temp->short_name = gettext("German");
    }
    else
    {
      temp->long_name = g_strdup_printf (gettext("%s Dictionary"), name);
      temp->short_name = g_strdup_printf ("%s", name);
    }

    //Calculate the path to the used dictionary file
    remaining = PATH_MAX;
    gw_util_get_waei_directory(temp->path);
    remaining -= strlen(temp->path);
    strncat(temp->path, name, remaining);

    //Calculate the path to the sync file
    remaining = PATH_MAX;
    gw_util_get_waei_directory(temp->sync_path);
    remaining -= strlen(temp->sync_path);
    strncat(temp->sync_path, "download", remaining);
    remaining -= strlen("download");
    strncat(temp->sync_path, G_DIR_SEPARATOR_S, remaining);
    remaining -= 1;
    strncat(temp->sync_path, name, remaining);

    //Calculate the path to the gz
    remaining = PATH_MAX;
    strcpy(temp->gz_path, temp->sync_path);
    remaining -= strlen(temp->gz_path);
    strncat(temp->gz_path, ".gz", remaining);

    //Update the line count
    if (gw_util_get_runmode () == GW_CONSOLE_RUNMODE)
      temp->total_lines = 1;
    else
      temp->total_lines =  gw_io_get_total_lines_for_path (temp->path);

    //Create id (to show special built in dictionaries)
    if      (strcmp(name, "English") == 0)
    {
      temp->id = GW_DICT_ID_ENGLISH;
      temp->type = GW_DICT_TYPE_EDICT;
      strncpy (temp->gckey, GCKEY_GW_ENGLISH_SOURCE, 100);
      strcpy (temp->rsync, RSYNC);
      strcat (temp->rsync, " -v ftp.monash.edu.au::nihongo/edict ");
      strcat (temp->rsync, temp->sync_path);
    }
    else if (strcmp (name, "Kanji") == 0)
    {
      temp->id = GW_DICT_ID_KANJI;
      temp->type = GW_DICT_TYPE_KANJI;
      strncpy (temp->gckey, GCKEY_GW_KANJI_SOURCE, 100);
      strcpy (temp->rsync, RSYNC);
      strcat (temp->rsync, " -v ftp.monash.edu.au::nihongo/kanjidic ");
      strcat (temp->rsync, temp->sync_path);
    }
    else if (strcmp (name, "Radicals") == 0)
    {
      temp->id = GW_DICT_ID_RADICALS;
      temp->type = GW_DICT_TYPE_RADICALS;
      strncpy (temp->gckey, GCKEY_GW_RADICALS_SOURCE, 100);
      strcpy (temp->rsync, "");
    }
    else if (strcmp (name, "Names") == 0)
    {
      temp->id = GW_DICT_ID_NAMES;
      temp->type = GW_DICT_TYPE_EDICT;
      strncpy (temp->gckey, GCKEY_GW_NAMES_SOURCE, 100);
      strcpy (temp->rsync, RSYNC);
      strcat (temp->rsync, " -v ftp.monash.edu.au::nihongo/enamdict ");
      strcat (temp->rsync, temp->sync_path);
    }
    else if (strcmp (name, "Places") == 0)
    {
      temp->id = GW_DICT_ID_PLACES;
      temp->type = GW_DICT_TYPE_EDICT;
      strncpy(temp->gckey, "", 100);
      strcpy (temp->rsync, "");
    }
    else if (strcmp (name, "Examples") == 0)
    {
      temp->id = GW_DICT_ID_EXAMPLES;
      temp->type = GW_DICT_TYPE_EXAMPLES;
      strncpy(temp->gckey, GCKEY_GW_EXAMPLES_SOURCE, 100);
      strcpy (temp->rsync, "");
      temp->total_lines =  temp->total_lines / 2;
    }
    else if (strcmp (name, "French") == 0)
    {
      temp->id = GW_DICT_ID_FRENCH;
      temp->type = GW_DICT_TYPE_EDICT;
      strncpy(temp->gckey, GCKEY_GW_FRENCH_SOURCE, 100);
      strcpy (temp->rsync, "");

      gw_util_get_waei_directory(temp->gz_path);
      strcat(temp->gz_path, "download");
      strncat(temp->gz_path, G_DIR_SEPARATOR_S, remaining);
      strcat(temp->gz_path, "French.UTF8");

      gw_util_get_waei_directory(temp->sync_path);
      strcat(temp->sync_path, "download");
      strncat(temp->sync_path, G_DIR_SEPARATOR_S, remaining);
      strcat(temp->sync_path, "French.UTF8");

}
    else if (strcmp (name, "German") == 0)
    {
      temp->id = GW_DICT_ID_GERMAN;
      temp->type = GW_DICT_TYPE_EDICT;
      strncpy(temp->gckey, GCKEY_GW_GERMAN_SOURCE, 100);
      strcpy (temp->rsync, "");
    }
    else if (strcmp (name, "Spanish") == 0)
    {
      temp->id = GW_DICT_ID_SPANISH;
      temp->type = GW_DICT_TYPE_EDICT;
      strncpy(temp->gckey, GCKEY_GW_SPANISH_SOURCE, 100);
      strcpy (temp->rsync, "");
      strcpy(temp->gz_path, temp->sync_path);

      gw_util_get_waei_directory(temp->gz_path);
      strcat(temp->gz_path, "download");
      strncat(temp->gz_path, G_DIR_SEPARATOR_S, remaining);
      strcat(temp->gz_path, "Spanish.zip");

      gw_util_get_waei_directory(temp->sync_path);
      strcat(temp->sync_path, "download");
      strncat(temp->sync_path, G_DIR_SEPARATOR_S, remaining);
      strcat(temp->sync_path, "hispamix.euc");

      gw_util_get_waei_directory(temp->path);
      strcat(temp->path, "Spanish");
    }
    else if (strcmp (name, "Mix") == 0)
    {
      temp->id = GW_DICT_ID_MIX;
      temp->type = GW_DICT_TYPE_KANJI;
      strncpy(temp->gckey, "", 100);
      strcpy (temp->rsync, "");
    }
    else
    {
      temp->id = gw_dictinfo_make_dictionary_id ();
      temp->type = GW_DICT_TYPE_OTHER;
      strncpy(temp->gckey, "", 100);
      strcpy (temp->rsync, "");
    }

    //Set the initial installation status
    if (g_file_test(temp->path, G_FILE_TEST_IS_REGULAR) == TRUE)
      temp->status = GW_DICT_STATUS_INSTALLED;
    else
      temp->status = GW_DICT_STATUS_NOT_INSTALLED;

    temp->cached_resultlines = NULL;
    temp->current_resultline = NULL;
    //gw_dictinfo_cache_resultlines (temp);

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
    if (di->name != NULL)
    {
      g_free (di->name);
      di->name = NULL;
    }

    free(di);
    di = NULL;
}


//!
//! @brief Creates a unique id number for a dictionary
//!
//! The function returns an id, then incrementing the internal number for the 
//! next dictionary.
//!
//! @return A unique id integer
//!
int gw_dictinfo_make_dictionary_id ()
{
    id_increment++;
    return id_increment;
}

