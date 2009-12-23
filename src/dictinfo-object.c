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
#include <curl/curl.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/resultline-object.h>
#include <gwaei/dictinfo-object.h>


static int id_increment = 100;


/*
gboolean gw_dictinfo_cache_resultlines (GwDictInfo *di)
{
    if (di->cached_resultlines != NULL) return TRUE;

    FILE *fd = NULL;

    if (di->total_lines < 1)
    {
      //Calculate the number of lines in the dictionary
      char line[MAX_LINE];
      fd = fopen (di->path, "r");
      if (fd != NULL)
      {
        while (fgets(line, MAX_LINE, fd) != NULL)
          di->total_lines++;
        fclose(fd);
      }
      fd = NULL;
    }
    printf("%ld\n", di->total_lines);

    if (di->total_lines > 0)
    {
      fd = fopen (di->path, "r");
      printf("size allocated: %d %d %ld\n", sizeof(GwResultLine), di->total_lines, (sizeof(GwResultLine) * di->total_lines) / 1048576);
      di->cached_resultlines = (GwResultLine*) malloc (sizeof(GwResultLine) * di->total_lines);
      if (di->cached_resultlines == NULL) printf("failed to allocate block memory for resultlines\n");
      GwResultLine *rl = di->cached_resultlines;
      int current_line = 0;

      while (current_line < di->total_lines && fgets(rl[current_line].string, MAX_LINE, fd) != NULL)
      {
        gw_resultline_clear_variables (&rl[current_line]);
        gboolean parse_status;
        switch (di->type)
        {
            case GW_DICT_OTHER:
              gw_resultline_parse_edict_result_string (&rl[current_line]);
              break;
            case GW_DICT_RADICALS:
              gw_resultline_parse_radicaldict_result_string (&rl[current_line]);
              break;
            case GW_DICT_KANJI:
              gw_resultline_parse_kanjidict_result_string (&rl[current_line]);
              break;
            case GW_DICT_EXAMPLES:
              gw_resultline_parse_examplesdict_result_string (&rl[current_line]);
              break;
            default:
              gw_resultline_parse_unknowndict_result_string (&rl[current_line]);
              break;
        }
        current_line++;
      }

      fclose(fd);

      if (current_line < di->total_lines)
      {
         printf("the lines don't match up!\n");
         exit (EXIT_FAILURE);
      }
      return TRUE;
    }
    else
    {
      printf("new function used too much memory\n");
      exit (EXIT_FAILURE);
    }
}
*/


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

    //Copy the name of the dictionary over
    remaining = MAX_DICTIONARY;
    strncpy(temp->name, name, remaining);

    char *english = gettext("English Dictionary");
    char *spanish = gettext("Spanish Dictionary");
    char *kanji = gettext("Kanji Dictionary");
    char *radicals = gettext("Radicals Dictionary");
    char *places = gettext("Places Dictionary");
    char *names = gettext("Names Dictionary");
    char *examples = gettext("Examples Dictionary");
    char *french = gettext("French Dictionary");
    char *german = gettext("German Dictionary");

    if (strcmp(name, "English") == 0)
      strncpy(temp->long_name, english, 100);
    else if (strcmp(name, "Spanish") == 0)
      strncpy(temp->long_name, spanish, 100);
    else if (strcmp(name, "Mix") == 0)
      strncpy(temp->long_name, kanji, 100);
    else if (strcmp(name, "Kanji") == 0)
      strncpy(temp->long_name, kanji, 100);
    else if (strcmp(name, "Radicals") == 0)
      strncpy(temp->long_name, kanji, 100);
    else if (strcmp(name, "Places") == 0)
      strncpy(temp->long_name, places, 100);
    else if (strcmp(name, "Names") == 0)
      strncpy(temp->long_name, names, 100);
    else if (strcmp(name, "Examples") == 0)
      strncpy(temp->long_name, examples, 100);
    else if (strcmp(name, "French") == 0)
      strncpy(temp->long_name, french, 100);
    else if (strcmp(name, "German") == 0)
      strncpy(temp->long_name, german, 100);
    else
      strncpy(temp->long_name, name, 100);

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
    temp->total_lines =  gw_io_get_total_lines_for_path (temp->path);

    //Create id (to show special built in dictionaries)
    if      (strcmp(name, "English") == 0)
    {
      temp->id = GW_DICT_ENGLISH;
      temp->type = GW_DICT_OTHER;
      strncpy (temp->gckey, GCKEY_GW_ENGLISH_SOURCE, 100);
      strcpy (temp->rsync, RSYNC);
      strcat (temp->rsync, " -v ftp.monash.edu.au::nihongo/edict ");
      strcat (temp->rsync, temp->sync_path);
    }
    else if (strcmp (name, "Kanji") == 0)
    {
      temp->id = GW_DICT_KANJI;
      temp->type = GW_DICT_KANJI;
      strncpy (temp->gckey, GCKEY_GW_KANJI_SOURCE, 100);
      strcpy (temp->rsync, RSYNC);
      strcat (temp->rsync, " -v ftp.monash.edu.au::nihongo/kanjidic ");
      strcat (temp->rsync, temp->sync_path);
    }
    else if (strcmp (name, "Radicals") == 0)
    {
      temp->id = GW_DICT_RADICALS;
      temp->type = GW_DICT_RADICALS;
      strncpy (temp->gckey, GCKEY_GW_RADICALS_SOURCE, 100);
      strcpy (temp->rsync, "");
    }
    else if (strcmp (name, "Names") == 0)
    {
      temp->id = GW_DICT_NAMES;
      temp->type = GW_DICT_OTHER;
      strncpy (temp->gckey, GCKEY_GW_NAMES_SOURCE, 100);
      strcpy (temp->rsync, RSYNC);
      strcat (temp->rsync, " -v ftp.monash.edu.au::nihongo/enamdict ");
      strcat (temp->rsync, temp->sync_path);
    }
    else if (strcmp (name, "Places") == 0)
    {
      temp->id = GW_DICT_PLACES;
      temp->type = GW_DICT_OTHER;
      strncpy(temp->gckey, "", 100);
      strcpy (temp->rsync, "");
    }
    else if (strcmp (name, "Examples") == 0)
    {
      temp->id = GW_DICT_EXAMPLES;
      temp->type = GW_DICT_EXAMPLES;
      strncpy(temp->gckey, GCKEY_GW_EXAMPLES_SOURCE, 100);
      strcpy (temp->rsync, "");
      temp->total_lines =  temp->total_lines / 2;
    }
    else if (strcmp (name, "French") == 0)
    {
      temp->id = GW_DICT_FRENCH;
      temp->type = GW_DICT_OTHER;
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
      temp->id = GW_DICT_GERMAN;
      temp->type = GW_DICT_OTHER;
      strncpy(temp->gckey, GCKEY_GW_GERMAN_SOURCE, 100);
      strcpy (temp->rsync, "");
    }
    else if (strcmp (name, "Spanish") == 0)
    {
      temp->id = GW_DICT_SPANISH;
      temp->type = GW_DICT_OTHER;
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
      temp->id = GW_DICT_MIX;
      temp->type = GW_DICT_KANJI;
      strncpy(temp->gckey, "", 100);
      strcpy (temp->rsync, "");
    }
    else
    {
      temp->id = gw_dictinfo_make_dictionary_id ();
      temp->type = GW_DICT_OTHER;
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

