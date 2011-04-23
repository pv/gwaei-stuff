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
//! @file src/console-main-interface.c
//!
//! @brief Abstraction layer for the console
//!
//! Used as a go between for functions interacting with the console.
//!


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <libintl.h>

#include <glib.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>

static int _console_progress_cb (double percent, gpointer data);

//!
//! @brief Print the "less relevant" header where necessary.
//!
void gw_console_append_less_relevant_header_to_output(GwSearchItem *item)
{
    printf("\n[0;31m***[0m[1m%s[0;31m***************************[0m\n\n\n", gettext("Other Results"));
}


//!
//! @brief Print the "no result" message where necessary.
//!
void gw_console_no_result (GwSearchItem *item)
{
    printf("%s\n\n", gettext("No results found!"));
}


//!
//! @brief Callback function for printing a status message to the console
//!
//! @param message String of message to be shown.
//! @param percent An integer of how much a progress bar should be filled.  1-100.
//! @param data Currently unused gpointer.
//!
static int _print_message_to_console (char *message, int percent, gpointer data)
{
    if (message != NULL) printf("%s", message);
    if (percent > -1 && percent < 101) printf("%d%s", percent, "%");
    if (message != NULL || (percent > -1 && percent < 101)) printf("\n");

    return FALSE;
}


//!
//! @brief Uninstalls the named dictionary, deleting it.
//!
//! @param name A string of the name of the dictionary to uninstall.
//!
void gw_console_uninstall_dictinst (GwDictInst *di)
{
    //Declarations
    GError *error;

    //Initializations
    error = NULL;

    //gw_dictinst_uninstall (di, _console_progress_cb, &error);
    printf(gettext("Finished\n"));

    //Cleanup
    g_error_free (error);
    error = NULL;
}


//!
//! @brief Installs the named dictionary, deleting it.
//!
//! @param name A string of the name of the dictionary to install.
//!
gboolean gw_console_install_dictinst (GwDictInst *di)
{
/*
    printf(gettext("Trying to install the %s...\n"), di->long_name);

    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);
    GError *error = NULL;

    if (di->status != GW_DICT_STATUS_NOT_INSTALLED) return FALSE;

    gw_io_install_dictinfo (di, &_print_message_to_console, NULL, TRUE, &error);

    //Install auxillery dictionaries when appropriate
    if (error == NULL && di->id == GW_DICT_ID_KANJI)
    {
      di = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_RADICALS);
      gw_io_install_dictinfo (di, &_print_message_to_console, NULL, TRUE, &error);
    }
    else if (error == NULL && di->id == GW_DICT_ID_RADICALS)
    {
      di = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_KANJI);
      gw_io_install_dictinfo (di, &_print_message_to_console, NULL, TRUE, &error);
    }

    //Print final messages
    if (error != NULL)
    {
      printf("%s\n", error->message);
		  g_error_free (error);
      error = NULL;
      return FALSE;
    }
    else
    {
      printf(gettext("Finished\n"));
    }
*/
    return TRUE;
}


//!
//! @brief Prints to the terminal the about message for the program.
//!
void gw_console_about ()
{
#ifdef WITH_GTK
    printf ("gWaei version %s with the Gnome font end compiled in.", VERSION);
#elif WITH_QT
    printf ("gWaei version %s with the QT font end compiled in.", VERSION);
#else
    printf ("gWaei version %s with no end compiled in.", VERSION);
#endif

    printf ("\n\n");

    printf ("Check for the latest updates at <http://gwaei.sourceforge.net/>\n");
    printf ("Code Copyright (C) 2009-2010 Zachary Dovel\n\n");

    printf ("License:\n");
    printf ("Copyright (C) 2008 Free Software Foundation, Inc.\nLicense GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\n");
}


//!
//! @brief This function prints the start banner
//!
//! This function prints the start banner in both
//! simple and ncurses interface.
//!
//! @param query The query string we are searching
//! @param dictionary The name (string) of the dictionary used
//!
void gw_console_start_banner (char *query, char *dictionary)
{
    // TRANSLATORS: 'Searching for "${query}" in ${dictionary long name}'
    printf(gettext("Searching for \"%s\" in %s...\n"), query, dictionary);
    printf("\n");
}


//!
//! @brief Prints out the yet uninstalled available dictionaries.
//!
static void _print_installable_dictionaries ()
{
/*
    if (_quiet_switch == FALSE)
    {
      printf(gettext("Installable dictionaries are:\n"));
    }

    int i = 0; 

    GwDictInfo* di;
    GList *list = gw_dictlist_get_list ();
    while (list != NULL)
    {
      di = (GwDictInfo*) list->data;
      if (di->gskey != NULL && di->status == GW_DICT_STATUS_NOT_INSTALLED &&
          di->id != GW_DICT_ID_RADICALS && di->id != GW_DICT_ID_MIX            )
      {
        printf("  %s (AKA: %s)\n", di->name, di->long_name);
        i++;
      }
      list = list->next;
    }

    if (i == 0)
      printf("  %s\n", gettext("none"));

    printf ("\n");
*/
}


//!
//! @brief Not yet written
//!
static void _print_available_dictionaries ()
{
/*
    int i = 0;
    GwDictInfo* di;
    GList *list;

	  if (_quiet_switch == FALSE)
    {
    	printf(gettext("Available dictionaries are:\n"));
    }

	  list = gw_dictlist_get_list();
    while (list != NULL) {
      di = list->data;
      if (di->status == GW_DICT_STATUS_INSTALLED && di->id != GW_DICT_ID_RADICALS && di->id != GW_DICT_ID_MIX) {
        printf("  %s (AKA: %s)\n", di->name, di->long_name);
        i++;
      }
      list = list->next;
    }

    if (i == 0)
      printf("  %s\n", gettext("none"));

    printf ("\n");
*/
}


//!
//! @brief CONSOLE Main function
//!
//! This function will NOT use GError because it have
//! nobody to report to.
//!
//! @param argc Standard argc from main
//! @param argv Standard argv from main
//!
void initialize_console_interface (int argc, char **argv)
{
  /*
    GwDictInfo *di = NULL;
    GError *error = NULL;

    if (_list_switch)
    {
      print_available_dictionaries ();
      print_installable_dictionaries();
      exit (EXIT_SUCCESS);
    }

    //User wants to see the version of waei
    if (_version_switch)
    {
      print_about_program ();
      exit (EXIT_SUCCESS);
    }

    //User wants to sync dictionaries
    else if (rsync_switch)
    {
      printf("");
      printf(gettext("Syncing possible installed dictionaries..."));
      printf("[0m\n");

      GwDictInfo* di;
      GList *list = gw_dictlist_get_list();

      while (list != NULL && error == NULL) {
        di = list->data;
        gw_dictlist_sync_dictionary (di, &error);
        list = list->next;
      }

      if (error == NULL)
      {
        printf("");
        printf(gettext("Finished"));
        printf("[0m\n");
      }
      else
      {
        printf("");
        printf("%s", error->message);
        printf("[0m\n");
        g_error_free (error);
        error = NULL;
      }

      return;
    }

    //User wants to install dictionary
    if (_install_switch_data != NULL)
    {
      di = gw_dictlist_get_dictinfo_by_name (_install_switch_data);
      if (di != NULL && di->status != GW_DICT_STATUS_NOT_INSTALLED && di->gskey[0] != '\0')
      {
        printf(gettext("%s is already Installed. "), di->long_name);
        print_installable_dictionaries();
      }
      else if (di == NULL || di->gskey[0] == '\0' || di->id == GW_DICT_ID_RADICALS || di->id == GW_DICT_ID_MIX)
      {
        printf(gettext("%s is not installable with this mechanism. "), _install_switch_data);
        print_installable_dictionaries();
      }
      else if (di != NULL)
        gw_console_install_dictinfo (di);
      exit (EXIT_SUCCESS);
    }

    //User wants to uninstall dictionary
    if (_uninstall_switch_data != NULL)
    {
      di = gw_dictlist_get_dictinfo_by_name (_uninstall_switch_data);
      if (di != NULL)
      {
        if (di->status = GW_DICT_STATUS_INSTALLED && di->id != GW_DICT_ID_RADICALS && di->id != GW_DICT_ID_MIX)
          gw_console_uninstall_dictinfo (di);
        else
        {
          printf(gettext("The %s is not installed. "), di->long_name);
          print_available_dictionaries();
        }
      }
      else
      {
        // TRANSLATORS: The "%s" stands for the value provided by the user to the "waei uninstall"
        printf(gettext("%s is not installed. "), _uninstall_switch_data);
        print_available_dictionaries();
      }
      exit (EXIT_SUCCESS);
    }

    //We weren't interupted by any switches! Now to the search....

    //Set dictionary
    if (_dictionary_switch_data == NULL)
      di = gw_dictlist_get_dictinfo_by_alias ("English");
    else
      di = gw_dictlist_get_dictinfo_by_alias (_dictionary_switch_data);
    if (di == NULL || di->status != GW_DICT_STATUS_INSTALLED)
    {
      printf (gettext("Requested dictionary not found!\n"));
      exit (EXIT_FAILURE);
    }

    //Set query text
    if (argc > 1 && _query_text_data == NULL)
    {
      _query_text_data = gw_util_strdup_args_to_query (argc, argv);
      if (_query_text_data == NULL)
      {
        printf ("Memory error creating initial query string!\n");
        exit (EXIT_FAILURE);
      }
    }
    else
    {
      //No query means to print the help screen
      char *help_text = g_option_context_get_help (context, TRUE, NULL);
      if (help_text != NULL)
      {
        printf("%s\n", help_text);
        g_free (help_text);
        help_text = NULL;
      }
      exit (EXIT_SUCCESS);
    }

    //Set the output generic functions
    gw_console_initialize_interface_output_generics ();

    // Run some checks and transformation on a user inputed string before using it
   	char* sane_query = gw_util_prepare_query (_query_text_data, FALSE);
   	if (sane_query == NULL)
   	{
   	  printf(gettext("Query prepare error\n"));
   	  exit (EXIT_FAILURE);
   	}
   	g_stpcpy(_query_text_data, sane_query);
   	g_free(sane_query);
   	sane_query = NULL;

    //Print the search intro
    if (!_quiet_switch)
    {
      gw_console_start_banner (_query_text_data, di->long_name);
    }

    //Start the search
    GwSearchItem *item;
    item = gw_searchitem_new (_query_text_data, di, GW_TARGET_CONSOLE);
    if (item == NULL)
    {
      printf(gettext("Query parse error\n"));
      exit (EXIT_FAILURE);
    }

    //Print the number of results
    gw_search_get_results (item, FALSE, _exact_switch);

    //Final header
    if (_quiet_switch == FALSE)
    {
      char *message_total = ngettext("Found %d result", "Found %d results", item->total_results);
      char *message_relevant = ngettext("(%d Relevant)", "(%d Relevant)", item->total_relevant_results);
      printf(message_total, item->total_results);
      if (item->total_relevant_results != item->total_results)
        printf(message_relevant, item->total_relevant_results);
      printf("\n");
    }

    //Cleanup
    gw_searchitem_free (item);
    if (_query_text_data != NULL)
    {
      g_free (_query_text_data);
      _query_text_data = NULL;
    }

*/
    //Finish
    exit (EXIT_SUCCESS);
}


//!
//! @brief Not yet written
//!
void gw_console_append_edict_results_to_buffer (GwSearchItem *item)
{
    //Definitions
    int cont = 0;
    GwResultLine *resultline = item->resultline;

    //Kanji
    printf("[32m%s", resultline->kanji_start);
    //Furigana
    if (resultline->furigana_start)
      printf(" [%s]", resultline->furigana_start);
    //Other info
    if (resultline->classification_start)
      printf("[0m %s", resultline->classification_start);
    //Important Flag
    if (resultline->important)
      printf("[0m %s", "P");

    printf("\n");
    while (cont < resultline->def_total)
    {
      printf("[0m      [35m%s [0m%s\n", resultline->number[cont], resultline->def_start[cont]);
      cont++;
    }
    printf("\n");
}


//!
//! @brief Not yet written
//!
void gw_console_append_kanjidict_results_to_buffer (GwSearchItem *item)
{
    if (item == NULL) return;

    char line_started = FALSE;
      GwResultLine *resultline = item->resultline;

    //Kanji
    printf("[32;1m%s[0m\n", resultline->kanji);

    if (resultline->radicals)
      printf("%s%s\n", gettext("Radicals:"), resultline->radicals);

    if (resultline->strokes)
    {
      line_started = TRUE;
      printf("%s%s", gettext("Stroke:"), resultline->strokes);
    }

    if (resultline->frequency)
    {
      if (line_started)
        printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("Freq:"), resultline->frequency);
    }

    if (resultline->grade)
    {
      if (line_started)
        printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("Grade:"), resultline->grade);
    }

    if (resultline->jlpt)
    {
      if (line_started)
        printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("JLPT:"), resultline->jlpt);
    }

    if (line_started)
      printf("\n");

    if (resultline->readings[0])
      printf("%s%s\n", gettext("Readings:"), resultline->readings[0]);
    if (resultline->readings[1])
      printf("%s%s\n", gettext("Name:"), resultline->readings[1]);
    if (resultline->readings[2])
      printf("%s%s\n", gettext("Radical Name:"), resultline->readings[2]);

    if (resultline->meanings)
      printf("%s%s\n", gettext("Meanings:"), resultline->meanings);
    printf("\n");
}


//!
//! @brief Not yet written
//!
void gw_console_append_examplesdict_results_to_buffer (GwSearchItem *item)
{
    if (item == NULL) return;

    GwResultLine *resultline = item->resultline;


    if (resultline->def_start[0] != NULL)
    {
      printf ("[32;1m%s[0m", gettext("E:\t"));
      printf ("%s", resultline->def_start[0]);
    }

    if (resultline->kanji_start != NULL)
    {
      printf ("[32;1m%s[0m", gettext("\nJ:\t"));
      printf ("%s", resultline->kanji_start);
    }

    if (resultline->furigana_start != NULL)
    {
      printf("[32;1m%s[0m", gettext("\nD:\t"));
      printf("%s", resultline->furigana_start);
    }

    printf("\n\n");
}


//!
//! @brief Not yet written
//!
void gw_console_append_unknowndict_results_to_buffer (GwSearchItem *item)
{
    if (item == NULL) return;

    printf("%s\n", item->resultline->string);
}


//!
//! @brief Not yet written
//!
void gw_console_append_more_relevant_header_to_output (GwSearchItem *item)
{
}


//!
//! @brief Not yet written
//!
void gw_console_pre_search_prep (GwSearchItem *item)
{
}

//!
//! @brief Not yet written
//!
void gw_console_after_search_cleanup (GwSearchItem *item)
{
    //Finish up
    if (item->total_results == 0 &&
        item->target != GW_TARGET_KANJI &&
        item->status == GW_SEARCH_SEARCHING)
    {
      gw_console_no_result(item);
    }
}


static int _console_progress_cb (double percent, gpointer data)
{
  //Declarations
  GwDictInst *di;
  char *status;

  //Initializations
  di = data;
  status = gw_dictinst_get_status_string (di, TRUE);

  //Body
  printf("%s\n", status);

  //Cleanup
  g_free (status);
  status = NULL;
}

//!
//! @brief Lists the currently installed and installable dictionaries
//!
void gw_console_list ()
{
    _print_available_dictionaries ();
    _print_installable_dictionaries();
}



