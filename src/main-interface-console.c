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
//! @file src/main-interface-console.c
//!
//! @brief Abstraction layer for the console
//!
//! Used as a go between for functions interacting with the console.
//!


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libintl.h>
#include <regex.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/resultline.h>
#include <gwaei/regex.h>
#include <gwaei/dictionaries.h>
#include <gwaei/history.h>
#include <gwaei/engine.h>
#include <gwaei/preferences.h>

gboolean quiet_switch = FALSE;
gboolean exact_switch = FALSE;

void gw_console_uninstall_dictionary_by_name(char *name)
{
    GwDictInfo* di;
    di = gw_dictlist_get_dictionary_by_name (name);
    if (di == NULL) return;

    gw_io_delete_dictionary_file(di);
    di->status = GW_DICT_STATUS_NOT_INSTALLED;
    di->load_position = -1;

    if (di->id == GW_DICT_KANJI || di->id == GW_DICT_RADICALS)
      gw_console_uninstall_dictionary_by_name ("Mix");
    else if (di->id == GW_DICT_NAMES)
      gw_console_uninstall_dictionary_by_name ("Places");
}


gboolean gw_console_install_dictionary_by_name(char *name)
{
    GwDictInfo* di;
    di = gw_dictlist_get_dictionary_by_alias(name);

    char *path = di->path;
    char *sync_path = di->sync_path;
    char *gz_path = di->gz_path;

    if (di->status != GW_DICT_STATUS_NOT_INSTALLED || di->id == GW_DICT_PLACES) return FALSE;
    di->status = GW_DICT_STATUS_INSTALLING;
    
    char fallback_uri[100];
    gw_util_strncpy_fallback_from_key (fallback_uri, di->gckey, 100);
 
    char uri[100];
    gw_pref_get_string (uri, di->gckey, fallback_uri, 100);

    //Make sure the download folder exits
    char download_path[FILENAME_MAX] = "";
    gw_util_get_waei_directory(download_path);
    strcat(download_path, G_DIR_SEPARATOR_S);
    strcat(download_path, "download");

    gboolean ret = TRUE;

    if (ret)
      ret = ((g_mkdir_with_parents(download_path, 0755)) == 0);

    //Copy the file if it is a local file
    if (ret && g_file_test (uri, G_FILE_TEST_IS_REGULAR))
    {
      printf("  %s\n", gettext("Coping file..."));
      ret = gw_io_copy_dictionary_file (uri, gz_path, NULL);
    }
    else if (ret)
    {
      printf("* %s\n", gettext("Downloading file..."));
      ret = gw_io_download_dictionary_file (uri, gz_path, NULL, NULL);
    }

    if (ret)
    {
      printf("* %s\n", gettext("Gunzipping file..."));
      ret = gw_io_gunzip_dictionary_file(gz_path, NULL);
    }
   
    if (ret)
    {
      printf("* %s\n",gettext("Converting encoding..."));
      ret = gw_io_copy_with_encoding(sync_path, path, "EUC-JP","UTF-8");
    }

    if (ret)
      di->status = GW_DICT_STATUS_INSTALLED;
    else
      di->status = GW_DICT_STATUS_NOT_INSTALLED;

    //Special dictionary post processing
    if (ret)
    {
      printf("* %s\n", gettext("Postprocessing..."));
      gw_dictlist_preform_postprocessing_by_name(name);
    }

    return ret;
}


static void print_about_program ()
{
    printf ("[1;31m%s Version %s[0m[1m with ", PACKAGE, VERSION);
    if (strcmp (INTERFACE, "NONE") == 0)
      printf ("no");
    else if (strcmp (INTERFACE, "gwaei"))
      printf("the Gnome");
    printf (" front end compiled in.\n\n");
    printf ("[0mCheck for the latest updates at <http://gwaei.sourceforge.net/>\n");
    printf ("Code Copyright (C) 2009 Zachary Dovel[0m\n\n");

    printf ("License:\n");
    printf ("Copyright (C) 2008 Free Software Foundation, Inc.\nLicense GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\n");
}


static void print_search_start_banner(char *query, char *dictionary)
{
    printf("[1m");
    printf("%s", gettext("Searching for \""));
    printf("[1;31m%s[0m[1m", query);
    printf("%s", gettext("\" in the "));
    printf("[1;31m%s[0m[1m", dictionary);
    printf("%s", gettext(" dictionary..."));
    printf("[0m\n\n");

    #ifdef G_OS_UNIX
    sleep(1);
    #endif
}

static void print_installable_dictionaries()
{
    if (quiet_switch == FALSE)
    {
      printf("[1;31m");
      printf("%s ", gettext("Installable:"));
      printf("[0m");
    }

    int i = 0; 

    GwDictInfo* di;
    GList *list = gw_dictlist_get_list();
    while (list != NULL)
    {
      di = list->data;
      if (di->gckey[0] != '\0' && di->status == GW_DICT_STATUS_NOT_INSTALLED)
      {
        if (i != 0) printf(" ");
        printf("%s", di->name);
        i++;
      }
      list = list->next;
    }

    if (i == 0 && quiet_switch == FALSE)
      printf(gettext("All dictionaries are already installed"));

    printf ("\n");
}

static void print_uninstallable_dictionaries()
{
    if (quiet_switch == FALSE)
    {
      printf("[1;31m");
      printf("%s ", gettext("Uninstallable:"));
      printf("[0m");
    }

    int i = 0; 

    GwDictInfo* di;
    GList *list = gw_dictlist_get_list();
    while (list != NULL)
    {
      di = list->data;
      if (di->status == GW_DICT_STATUS_INSTALLED)
      {
        if (i != 0) printf(" ");
        printf("%s", di->name);
        i++;
      }
      list = list->next;
    }

    if (i == 0 && quiet_switch == FALSE)
      printf(gettext("There are no dictionaries installed"));

    printf ("\n");
}


static void print_available_dictionaries()
{
    if (quiet_switch == FALSE)
    {
      printf("[1;31m");
      printf("%s ", gettext("Available:"));
      printf("[0m");
    }

    int i = 0; 

    GwDictInfo* di;
    GList *list = gw_dictlist_get_list();
    while (list != NULL)
    {
      di = list->data;
      if (di->status == GW_DICT_STATUS_INSTALLED)
      {
        if (i != 0) printf(" ");
        printf("%s", di->name);
        i++;
      }
      list = list->next;
    }

    if (i == 0)
      printf(gettext(" There are no dictionaries available"));

    printf ("\n");
}


static void print_help_message()
{
    printf("%s\n\n", gettext("waei [options]... pattern"));
    printf("%s\n", gettext("This is a Japanese-English dictionary program that allows regex style searches.\nThe dictionaries include: English, Places, Names, Radicals and Kanji. Periods\ncan be used in place of unknown kanji."));

    printf("\n%s\n", gettext("OPTIONS:"));
    printf("%s\n", gettext("  -e, --exact                "
                   "Do not display less relevant results"));

    printf("%s\n", gettext("  -d, --dictionary name      "
                   "Search using a chosen dictionary"));

    printf("%s\n", gettext("  -l, --list                 "
                   "Show available dictionaries for searches"));

    printf("%s\n", gettext("  -i, --install              "
                   "List installable dictionaries"));

    printf("%s\n", gettext("  -i, --install dictionary   "
                   "Install dictionary"));

    printf("%s\n", gettext("  -u, --uninstall            "
                   "List uninstallable dictionaries"));

    printf("%s\n", gettext("  -u, --uninstall dictionary "
                   "Uninstall dictionary"));

    printf("%s\n", gettext("  -s, --sync                 "
                   "Sync installed dictionaries using rsync"));

    printf("%s\n", gettext("  -v, --version              "
                   "Check the waei version info"));

    printf("%s\n", gettext("  -q, --quiet                "
                   "Display less information"));

    printf("%s\n", gettext("  -h, --help                 "
                   "Display this help"));

    printf("\n%s\n", gettext("EXAMPLES:"));

    printf("%s\n", gettext("  waei English               "
                   "Search for the english word English"));

    printf("%s\n", gettext("  waei \"cats&dogs\"           "
                   "Search for results containing cats and dogs"));

    printf("%s\n", gettext("  waei \"cats|dogs\"           "
                   "Search for results containing cats or dogs"));

    printf("%s\n", gettext("  waei cats dogs             "
                   "Search for results containing \"cats dogs\""));

    printf("%s日本語%s日本語\n", gettext("  waei "), gettext("                "
                    "Search for the Japanese word "));

    printf("%s日本%s日本%s\n", gettext("  waei -e "), gettext("               "
                    "Search for "), gettext(" ignoring similar results"));

    printf("%s日.語%s\n", gettext("  waei "), gettext("                 "
                   "When you don't know the middle character"));

    printf("%s伍%s\n", gettext("  waei -d Kanji "), gettext("           "
                   "Find a kanji character in the kanji database"));

    printf("%s田中%s\n", gettext("  waei -d Names "), gettext("         "
                   "Look up a name in the name database"));

    printf("%s秋葉原%s\n", gettext("  waei -d Places "), gettext("      "
                   "Look up a place in the place database"));
}


static gboolean is_switch (char *arg, char *short_switch, char *long_switch)
{
  return (strcmp(arg, short_switch) == 0 || strcmp(arg, long_switch) == 0);
}


void initialize_console_interface(int argc, char **argv)
{
  GError *error = NULL;

  int i;
  int leftover;
  char query[MAX_QUERY];

  GwDictInfo *di;
  di = gw_dictlist_get_dictionary_by_alias("English");

  char *args[argc];
  int total_args;

  //Filter out modification arguments
  i = 1;
  total_args = 0;
  while (i < argc)
  {
    if (is_switch (argv[i], "-e", "--exact"))
    {
      exact_switch = TRUE;
    }

    else if (is_switch (argv[i], "-q", "--quiet"))
    {
      quiet_switch = TRUE;
    }

    else if (is_switch (argv[i], "-d", "--dictionary"))
    {
      i++;
      di = gw_dictlist_get_dictionary_by_alias(argv[i]);
    }

    else
    {
      args[total_args] = argv[i];
      total_args++;
    }
    i++;
  }


  //User requests help
  if (total_args == 0 || (total_args == 1 && is_switch (argv[1], "-h", "--help")))
  {
    print_help_message();
  }

  //User wants to see the available dictionaries
  else if (total_args == 1 && is_switch (args[0], "-l", "--list"))
  {
    print_available_dictionaries ();
  }

  //User wants to see the version of waei
  else if (total_args == 1 && is_switch (args[0], "-v", "--version"))
  {
    print_about_program ();
  }

  //Show installable dictionaries
  else if (total_args == 1 && is_switch (args[0], "-i", "--install"))
  {
    print_installable_dictionaries();
  }

  //User wants to install dictionary
  else if (total_args == 2 && is_switch (args[0], "-i", "--install"))
  {
    printf(gettext("%sTrying to install %s%s%s...%s"), "[1m", "[1;31m", args[1], "[0m[1m", "\n[0m");

    di = gw_dictlist_get_dictionary_by_alias(args[1]);

    if (di != NULL && di->status != GW_DICT_STATUS_NOT_INSTALLED && di->gckey[0] != '\0')
    {
      //printf("[1m");
      printf("%s\n", gettext("Already Installed"));
      //printf("[0m");
    }
    else if (di == NULL || di->gckey[0] == '\0')
    {
      //printf("[1m");
      printf("%s\n", gettext("That dictionary is not installable with this mechanism"));
      //printf("[0m");
      exit (1);
    }
    else if (gw_console_install_dictionary_by_name (args[1]))
    {
      printf("[1m");
      printf(gettext("Finished"));
      printf("[0m\n");
    }
    else
    {
      printf("[1m");
      printf(gettext("Failed"));
      printf("[0m\n");
    }
  }

  //Show uninstallable dictionaries
  else if (total_args == 1 && is_switch (args[0], "-u", "--uninstall"))
  {
    print_uninstallable_dictionaries();
  }

  //User wants to uninstall dictionary
  else if (total_args == 2 && is_switch (args[0], "-u", "--uninstall"))
  {
    printf(gettext("%sTrying to uninstall %s%s%s...%s"), "[1m", "[1;31m", args[1], "[0m[1m", "\n[0m");

    if (gw_dictlist_check_if_loaded_by_name(argv[2]))
    {
      gw_console_uninstall_dictionary_by_name(argv[2]);

      printf("[1m");
      printf(gettext("Finished"));
      printf("[0m\n");
    }
    else
    {
      printf("%s\n", gettext("Is not installed"));
    }
  }


  //User wants to sync dictionaries
  else if (total_args == 1 && is_switch (args[0], "-s", "--sync"))
  {
    printf("[1m");
    printf(gettext("Syncing possible installed dictionaries..."));
    printf("[0m\n");

    GwDictInfo* di;
    GList *list = gw_dictlist_get_list();
    while (list != NULL && error == NULL)
    {
      di = list->data;
      gw_dictlist_sync_dictionary (di, &error);
      list = list->next;
    }

    if (error == NULL)
    {
      printf("[1m");
      printf(gettext("Finished"));
      printf("[0m\n");
    }
    else
    {
      printf("[1m");
      printf("%s", error->message);
      printf("[0m\n");
      g_error_free (error);
      error = NULL;
    }
  }


  //Make sure the selected dictionary exists
  else if (di == NULL || di->status != GW_DICT_STATUS_INSTALLED)
  {
    printf(gettext("Requested dictionary not found!\n"));
  }

  //Start the search
  else
  {
    //Collect the query terms
    i = 0;
    query[0] = '\0';
    leftover = MAX_QUERY;

    while (leftover > 0 && i < total_args)
    {
       strncat(query, args[i], leftover);
       leftover -= strlen (args[i]);
       i++;
       if (i != total_args)
       {
         strncat (query, " ", leftover);
         leftover -= 1;
       }
    }

    //Start the search
    if (quiet_switch == FALSE)
      print_search_start_banner(query, di->name);

    GwSearchItem *item;
    item = gw_searchitem_new(query, di, GW_TARGET_CONSOLE);
    item->show_less_relevant_results = !exact_switch;

    if (item != NULL )
    {
      gw_search_get_results (item);

      if (quiet_switch == FALSE)
      {
        printf("[1m");
        printf("\n%s", gettext("Found "));
        printf("%d", item->total_results);
        printf("%s", gettext(" Results"));

        if (item->total_relevant_results != item->total_results)
        {
          printf("%s", gettext(" ("));
          printf("%d", item->total_relevant_results);
          printf("%s", gettext(" Relevant)"));
        }
        printf("[0m\n");
      }
    }
    else
    {
      printf(gettext("Results seem to have incorrect formatting. Did you "
                     "close all of your\nparenthesis?  You may want to tr"
                     "y quotes too.\n"));
    }
  }
}


void gw_console_append_normal_results (GwSearchItem *item, GwResultLine *resultline)
{
    //Kanji
    printf("%s", resultline->kanji_start);
    //Furigana
    if (resultline->furigana_start)
    {
      printf(" [%s]", resultline->furigana_start);
    }
    //Other info
    if (resultline->classification_start)
    {
      printf(" %s", resultline->classification_start);
    }
    if (resultline->important)
    {
      printf(" %s", "P");
    }
    printf("\n");
    //Definitions
    int i = 0;
    while (i < resultline->def_total)
    {
      printf("      %s %s\n", resultline->number[i], resultline->def_start[i]);
      i++;
    }
    printf("\n");

}

void gw_console_append_kanji_results (GwSearchItem *item, GwResultLine *resultline)
{
    //Kanji
    printf("%s\n", resultline->kanji);
    if (resultline->radicals) printf("%s%s\n", gettext("Radicals:"), resultline->radicals);

    char line_started = FALSE;
    if (resultline->strokes)
    {
      line_started = TRUE;
      printf("%s%s", gettext("Stroke:"), resultline->strokes);
    }
    if (resultline->frequency)
    {
      if (line_started) printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("Freq:"), resultline->frequency);
    }
    if (resultline->grade)
    {
      if (line_started) printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("Grade:"), resultline->grade);
    }
    if (resultline->jlpt)
    {
      if (line_started) printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("JLPT:"), resultline->jlpt);
    }
    if (line_started) printf("\n");
    if (resultline->readings[0]) printf("%s%s", gettext("Readings:"), resultline->readings[0]);
    if (resultline->readings[1]) printf("%s", resultline->readings[1]);
    printf("\n");

    if (resultline->meanings) printf("%s%s\n", gettext("Meanings:"), resultline->meanings);
    printf("\n");

}

void gw_console_append_radical_results (GwSearchItem *item, GwResultLine *resultline)
{
    printf("%s : %s", resultline->kanji, resultline->radicals);
}
