/******************************************************************************

  FILE:
  src/dictionaries.c

  DESCRIPTION:
  The functions her generally manage the creation, destruction, and searching
  of dictionaries.  The DictionaryInfo objects also are used as a convenient
  container for variables pointing towards download locations, install locations
  etc.

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
#include <gwaei/dictionaries.h>

DictionaryList *dictionaries;


DictionaryInfo* dictionaryinfo_new (char *name)
{
    DictionaryInfo *temp;

    //Allocate some memory
    if ((temp = malloc(sizeof(DictionaryInfo))) == NULL) return NULL;

    int remaining;
    temp->load_position = -1;

    //Copy the name of the dictionary over
    remaining = MAX_DICTIONARY;
    strncpy(temp->name, name, remaining);

    if (strcmp(name, "English") == 0)
      strncpy(temp->long_name, gettext ("English"), 100);
    else if (strcmp(name, "Spanish") == 0)
      strncpy(temp->long_name, gettext ("Spanish"), 100);
    else if (strcmp(name, "Mix") == 0)
      strncpy(temp->long_name, gettext ("Kanji"), 100);
    else if (strcmp(name, "Kanji") == 0)
      strncpy(temp->long_name, gettext ("Kanji"), 100);
    else if (strcmp(name, "Radicals") == 0)
      strncpy(temp->long_name, gettext ("Radicals"), 100);
    else if (strcmp(name, "Places") == 0)
      strncpy(temp->long_name, gettext ("Places"), 100);
    else if (strcmp(name, "Names") == 0)
      strncpy(temp->long_name, gettext ("Names"), 100);
    else
      strncpy(temp->long_name, name, 100);

    strncat(temp->long_name, gettext(" Dictionary"),  100 - strlen(temp->long_name));

    //Calculate the path to the used dictionary file
    remaining = PATH_MAX;
    get_waei_directory(temp->path);
    remaining -= strlen(temp->path);
    strncat(temp->path, name, remaining);

    //Calculate the path to the sync file
    remaining = PATH_MAX;
    get_waei_directory(temp->sync_path);
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

    //Calculate the number of lines in the dictionary
    char line[MAX_LINE];
    temp->total_lines = 0;
    FILE *fd = fopen (temp->path, "r");
    if (fd != NULL)
    {
      while (fgets(line, MAX_LINE, fd) != NULL)
        temp->total_lines++;
      fclose(fd);
    }

    //Create id (to show special built in dictionaries)
    if      (strcmp(name, "English") == 0)
    {
      temp->id = ENGLISH;
      temp->type = OTHER;
      strncpy (temp->gckey, GCKEY_GWAEI_ENGLISH_SOURCE, 100);
      strcpy (temp->rsync, RSYNC);
      strcat (temp->rsync, " -v ftp.monash.edu.au::nihongo/edict ");
      strcat (temp->rsync, temp->sync_path);
    }
    else if (strcmp (name, "Kanji") == 0)
    {
      temp->id = KANJI;
      temp->type = KANJI;
      strncpy (temp->gckey, GCKEY_GWAEI_KANJI_SOURCE, 100);
      strcpy (temp->rsync, RSYNC);
      strcat (temp->rsync, " -v ftp.monash.edu.au::nihongo/kanjidic ");
      strcat (temp->rsync, temp->sync_path);
    }
    else if (strcmp (name, "Radicals") == 0)
    {
      temp->id = RADICALS;
      temp->type = RADICALS;
      strncpy (temp->gckey, GCKEY_GWAEI_RADICALS_SOURCE, 100);
      strcpy (temp->rsync, "");
    }
    else if (strcmp (name, "Names") == 0)
    {
      temp->id = NAMES;
      temp->type = OTHER;
      strncpy (temp->gckey, GCKEY_GWAEI_NAMES_SOURCE, 100);
      strcpy (temp->rsync, RSYNC);
      strcat (temp->rsync, " -v ftp.monash.edu.au::nihongo/enamdict ");
      strcat (temp->rsync, temp->sync_path);
    }
    else if (strcmp (name, "Places") == 0)
    {
      temp->id = PLACES;
      temp->type = OTHER;
      strncpy(temp->gckey, "", 100);
      strcpy (temp->rsync, "");
    }
    else if (strcmp (name, "Mix") == 0)
    {
      temp->id = MIX;
      temp->type = OTHER;
      strncpy(temp->gckey, "", 100);
      strcpy (temp->rsync, "");
    }
    else
    {
      temp->id = OTHER;
      temp->type = OTHER;
      strncpy(temp->gckey, "", 100);
      strcpy (temp->rsync, "");
    }


    //Set the initial installation status
    if (g_file_test(temp->path, G_FILE_TEST_IS_REGULAR) == TRUE)
      temp->status = INSTALLED;
    else
      temp->status = NOT_INSTALLED;

    //Done
    return temp;
}


void dictionaryitem_free(DictionaryInfo* di)
{
    free(di);
    di = NULL;
}


DictionaryList* dictionarylist_new ()
{
    DictionaryList *temp;

    //Allocate some memory
    if ((temp = malloc(sizeof(DictionaryList))) == NULL) return NULL;

    temp->list = NULL;
    temp->selected = NULL;

    return temp;
}


GList* dictionarylist_get_selected()
{
    return dictionaries->selected;
}

GList* dictionarylist_set_selected_by_load_position(int request)
{
    GList* current_dictionary = dictionarylist_get_list();
    do
    {
       if (((DictionaryInfo*)current_dictionary->data)->load_position == request)
         break;
       current_dictionary = g_list_next (current_dictionary);
    } while (current_dictionary != NULL);

    dictionaries->selected = current_dictionary;
    return current_dictionary;
}

void dictionarylist_add_dictionary(char *name)
{
    DictionaryInfo *di;
    if (dictionarylist_check_if_loaded_by_name (name) == FALSE)
      di = dictionaryinfo_new (name);
    if (di != NULL)
      dictionaries->list = g_list_append (dictionaries->list, di);
}


void dictionarylist_remove_first()
{
    GList *list;
    list = dictionaries->list;

    dictionaryitem_free(list->data);
    list = g_list_delete_link(list, list);
}


dictionarylist_free()
{
    while (dictionaries->list != NULL)
      dictionarylist_remove_first();
}


//This will replace get_dictionary_by_name when approprate
DictionaryInfo* dictionarylist_get_dictionary_by_alias(const char* request)
{
    char name[MAX_DICTIONARY];

    if ((strcmp (request, "Radicals") == 0 || strcmp (request, "Kanji") == 0) &&
         dictionarylist_dictionary_get_status_by_id (MIX) == INSTALLED )
      strncpy (name, "Mix", MAX_DICTIONARY);
    else 
      strncpy (name, request, MAX_DICTIONARY);

    GList *current;
    current = dictionaries->list;
    DictionaryInfo *di;
    di = NULL;

    while (current != NULL)
    {
      di = (DictionaryInfo*) current->data;

      if (strcmp (di->name, name) == 0)
        break;

      current = current->next;
      di = NULL;
    }

    return di;
}


DictionaryInfo* dictionarylist_get_dictionary_by_name(const char* request)
{
    char name[MAX_DICTIONARY];

    if ((strcmp (request, "Radicals") == 0 || strcmp (request, "Kanji") == 0) &&
         dictionarylist_dictionary_get_status_by_id (MIX) == INSTALLED )
      strncpy (name, "Mix", MAX_DICTIONARY);
    else 
      strncpy (name, request, MAX_DICTIONARY);

    GList *current;
    current = dictionaries->list;
    DictionaryInfo *di;
    di = NULL;

    while (current != NULL)
    {
      di = (DictionaryInfo*) current->data;

      if (strcmp (di->name, name) == 0)
        break;

      current = current->next;
      di = NULL;
    }

    return di;
}


gboolean dictionarylist_check_if_loaded_by_name(char* name)
{
    GList *current = dictionaries->list;
    DictionaryInfo *di;

    while (current != NULL)
    {
      di = (DictionaryInfo*) current->data;
      if (strcmp (di->name, name) == 0 && di->status == INSTALLED)
        return TRUE;
      current = current->next;
    }

    return FALSE;
}



int dictionarylist_get_total_with_status(int status)
{
    DictionaryInfo *di;
    GList *current = dictionaries->list;
    int i = 0;

    while (current != NULL)
    {
      di = (DictionaryInfo*) current->data;
      if (di->status == status)
      {
        i++;
      }
      current = current->next;
    }

    return i;
}


int dictionarylist_dictionary_get_status_by_id(int id)
{
    GList *current = dictionaries->list;
    DictionaryInfo *di = (DictionaryInfo*) current->data;

    while (current != NULL && di->id != id)
    {
      di = (DictionaryInfo*) current->data;
      current = current->next;
    }
    
    if (di->id == id)
      return di->status;
    else
      return NOT_INSTALLED;
}


DictionaryInfo* dictionarylist_get_dictionary_by_id(int id)
{
    GList *current = dictionaries->list;
    DictionaryInfo *di = (DictionaryInfo*) current->data;

    while (current != NULL && di->id != id)
    {
      di = (DictionaryInfo*) current->data;
      current = current->next;
    }
    
    return di;
}


int dictionarylist_get_total()
{
    return g_list_length(dictionaries->list);
}


GList* dictionarylist_get_list()
{
    return dictionaries->list;
}


//!!This function should never run DURING a dictionary install
int gwaei_dictionaries_initialize_dictionary_list()
{
    if (dictionaries != NULL)
      dictionarylist_free ();

    dictionaries = dictionarylist_new();
       
    dictionarylist_add_dictionary ("English");
    dictionarylist_add_dictionary ("Mix");
    dictionarylist_add_dictionary ("Kanji");
    dictionarylist_add_dictionary ("Radicals");
    dictionarylist_add_dictionary ("Names");
    dictionarylist_add_dictionary ("Places");

    //Path variables
    char path[FILENAME_MAX];
    if (get_waei_directory (path) == NULL) return;
    char *filename = &path[strlen (path)];

    //Directory variables
    GDir *directory = NULL;
    const gchar *input = NULL;

    //Open the ~/.waei directory
    directory = g_dir_open (path, 0, NULL);
    if (directory != NULL)
    {
      while ((input = g_dir_read_name (directory)) != NULL)
      {
        strcpy (filename, input);
        if (g_file_test(path, G_FILE_TEST_IS_REGULAR) == TRUE &&
            regexec (&re_gz,  filename, 1, NULL, 0 ) != 0     &&
            strcmp  (filename, "English"           ) != 0     &&
            strcmp  (filename, "Kanji"             ) != 0     &&
            strcmp  (filename, "Names"             ) != 0     &&           
            strcmp  (filename, "Places"            ) != 0     &&        
            strcmp  (filename, "Radicals"          ) != 0     &&     
            strcmp  (filename, "Mix"               ) != 0       )
        {
          dictionarylist_add_dictionary (filename);
        }
      }
    }
    g_dir_close (directory);
}


static gboolean create_mix_dictionary()
{
    DictionaryInfo* mix;
    mix = dictionarylist_get_dictionary_by_name("Mix");

    g_remove (mix->path);
    mix->status = NOT_INSTALLED;

    DictionaryInfo* kanji;
    kanji = dictionarylist_get_dictionary_by_name("Kanji");
    DictionaryInfo* radicals;
    radicals = dictionarylist_get_dictionary_by_name("Radicals");

    char *mpath = mix->path;
    char *kpath = kanji->path;
    char *rpath = radicals->path;

    mix->status = INSTALLING;

    gboolean ret;
    ret = gwaei_io_create_mix_dictionary(mpath, kpath, rpath);
   
    if (ret)
      mix->status = INSTALLED;
    else
      mix->status = ERRORED;

    return ret;
}


static gboolean split_places_from_names_dictionary(GError **error)
{
    DictionaryInfo* di_places;
    di_places = dictionarylist_get_dictionary_by_name("Places");

    DictionaryInfo* di_names;
    di_names = dictionarylist_get_dictionary_by_name("Names");

    if (di_names->status == NOT_INSTALLED) return FALSE;

    char *raw = di_names->sync_path;
    char source[FILENAME_MAX];
    strncpy(source, di_names->path, FILENAME_MAX);
    strncat(source, ".new", FILENAME_MAX - strlen(di_names->path));

    char *names = di_names->path;
    char *places = di_places->path;
    
    gboolean ret = TRUE;

    if (ret)
      ret = gwaei_io_copy_with_encoding(raw, source, "EUC-JP","UTF-8", error);
       
    if (ret)
    {
      remove(names);
      remove(places);
      ret = gwaei_io_split_places_from_names_dictionary (source, names, places);
    }


    if (ret)
    {
      g_remove(source);
      di_places->status = INSTALLED;
      di_names->status  = INSTALLED;
    }
    else
    {
      g_remove(source);
      g_remove(names);
      remove(places);
      di_places->status = ERRORED;
      di_names->status  = ERRORED;
    }
    
    return ret;
}


void dictionarylist_preform_postprocessing_by_name(char* name, GError **error)
{
    DictionaryInfo* di;
    di = dictionarylist_get_dictionary_by_name(name);
    if (di->status != INSTALLING && di->status != UPDATING && di->status != REBUILDING) return;

    int restore_status;
    restore_status = di->status;

    di->status == REBUILDING;

    //Figure out whether to create the Mix dictionary
    if (strcmp(name, "Radicals") == 0)
    {
      di = dictionarylist_get_dictionary_by_name("Kanji");
      if (di->status == INSTALLED) create_mix_dictionary();
    }
    else if (strcmp(name, "Kanji") == 0)
    {
      di = dictionarylist_get_dictionary_by_name("Radicals");
      if (di->status == INSTALLED) create_mix_dictionary();
    }
    else if (strcmp(name, "Mix") == 0)
    {
      di = dictionarylist_get_dictionary_by_id(RADICALS);
      if (di->status == INSTALLED)
      {
        di = dictionarylist_get_dictionary_by_id(KANJI);
        if (di->status == INSTALLED)
          create_mix_dictionary();
      }
    }
    else if(strcmp(name, "Names") == 0)
    {
      split_places_from_names_dictionary(error);
    }

    di->status == restore_status;
}


void dictionarylist_normalize_all_status_from_to(const int OLD, const int NEW)
{
    GList *current = dictionaries->list;
    DictionaryInfo *di;

    while (current != NULL)
    {
      di = (DictionaryInfo*) current->data;
      if (di->status == OLD)
        di->status = NEW;
      current = current->next;
    }
}


void dictionarylist_sync_dictionary (DictionaryInfo *di, GError **error)
{
    GQuark quark;
    quark = g_quark_from_string (GWAEI_GENERIC_ERROR);

    char *path = di->path;
    char *sync_path = di->sync_path;

    if (di->status != INSTALLED || strlen(di->rsync) < 2)
      return;

    printf("*  ");
    printf(gettext("Syncing %s dictionary"), di->name);
    printf("---------------------\n");

    if (system(di->rsync) != 0)
    {
      const char *message = gettext("File read failed");
      if (*error != NULL)
        *error = g_error_new_literal (quark, GWAEI_FILE_ERROR, message);
    }
    
    if (*error == NULL)
      gwaei_io_copy_with_encoding(sync_path, path, "EUC-JP","UTF-8", error);

    //Special dictionary post processing
    if (*error == NULL)
      dictionarylist_preform_postprocessing_by_name(di->name, error);

    if (*error == NULL)
      di->status = UPDATED;
    else
      di->status = ERRORED;

    if (error != NULL && *error != NULL)
    {
      g_error_free (*error);
      *error = NULL;
    }

    printf("\n");
    if (*error == NULL)
      printf("%s\n", gettext("Success"));
}

