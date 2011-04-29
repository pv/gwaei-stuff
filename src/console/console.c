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
//! @file src/console/console.c
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
//! @brief Uninstalls the named dictionary, deleting it.
//!
//! @param name A string of the name of the dictionary to uninstall.
//!
gboolean gw_console_uninstall_dictinfo (const char *FUZZY, GError **error)
{
    //Declarations
    GwDictInfo *di;

    //Initializations
    di = gw_dictinfolist_get_dictinfo_fuzzy (FUZZY);

    if (di != NULL)
    {
      printf(gettext("Uninstalling %s...\n"), di->longname);
      gw_dictinfo_uninstall (di, gw_console_uninstall_progress_cb, error);
    }
    else
    {
      printf("\n%s was not found!\n\n", FUZZY);
      gw_console_print_available_dictionaries ();
    }

    return (*error == NULL);
}


//!
//! @brief Installs the named dictionary, deleting it.
//!
//! @param name A string of the name of the dictionary to install.
//!
gboolean gw_console_install_dictinst (const char *FUZZY, GError **error)
{
    //Declarations
    GwDictInst *di;

    //Initializations
    di = gw_dictinstlist_get_dictinst_fuzzy (FUZZY);

    if (di != NULL)
    {
      printf(gettext("Installing %s...\n"), di->longname);
      gw_dictinst_install (di, gw_console_install_progress_cb, error);
    }
    else
    {
      printf("\n%s was not found!\n\n", FUZZY);
      gw_console_print_installable_dictionaries ();
    }

    return (*error == NULL);
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
void gw_console_print_installable_dictionaries ()
{
    printf(gettext("Installable dictionaries are:\n"));

    //Declarations
    int i;
    int j;
    GList *iter;
    GwDictInst* di;

    //Initializations
    i = 0; 
    iter = gw_dictinstlist_get_list ();

    while (iter != NULL)
    {
      di = (GwDictInst*) iter->data;
      if (gw_dictinst_data_is_valid (di))
      {
        printf("  %s", di->filename);
        for (j = strlen(di->filename); j < 20; j++) printf(" ");
        printf("(AKA: %s)\n", di->longname);
        i++;
      }
      iter = iter->next;
    }

    if (i == 0)
    {
      printf("  %s\n", gettext("none"));
    }
}


//!
//! @brief Not yet written
//!
void gw_console_print_available_dictionaries ()
{
    //Declarations
    int i;
    int j;
    GwDictInfo* di;
    GList *iter;

    //Initializations
    i = 0;
    j = 0;
	  iter = gw_dictinfolist_get_list();

    printf(gettext("Available dictionaries are:\n"));

    while (iter != NULL) {
      di = iter->data;
      printf("  %s", di->filename);
      for (j = strlen(di->filename); j < 20; j++) printf(" ");
      printf("(AKA: %s)\n", di->longname);
      i++;
      iter = iter->next;
    }

    if (i == 0)
    {
      printf("  %s\n", gettext("none"));
    }
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


int gw_console_uninstall_progress_cb (double fraction, gpointer data)
{
  //Declarations
  GwDictInfo *di;
  char *uri;

  //Initializations
  di = data;
  uri = g_build_filename (gw_util_get_directory_for_engine (di->engine), di->filename, NULL);

  printf("Removing %s...\n", uri);

  g_free (uri);

  return FALSE;
}





int gw_console_install_progress_cb (double fraction, gpointer data)
{
  //Declarations
  GwDictInst *di;
  char *status;
  int current_percent;
  int previous_percent;

  //Initializations
  di = data;
  current_percent = (int) (100.0 * fraction); 
  previous_percent = (int) (100.0 * di->progress); 
  if (di->progress <= 1.0) di->progress = fraction;

  //Update the dictinst progress state only when the delta is large enough
  if (current_percent - previous_percent >= 1 && di->progress < 1.0)
  {
    status = gw_dictinst_get_status_string (di, TRUE);
    printf("\r  %s", status);
    g_free (status);
  }

  //Display the final progress 
  else if (di->progress == 1.0)
  {
    status = gw_dictinst_get_status_string (di, TRUE);
    printf("\r  %s\n", status);
    g_free (status);
    di->progress = 1.1;
  }

  return FALSE;
}


//!
//! @brief Lists the available and installable dictionaries
//!
void gw_console_list ()
{
    gw_console_print_available_dictionaries ();
    gw_console_print_installable_dictionaries ();
}


//!
//! @brief If the GError is set, it prints it and frees the memory
//! @param error A pointer to a gerror pointer
//!
void gw_console_handle_error (GError **error)
{
    if (*error != NULL)
    {
      printf("Error: %s\n", (*error)->message);
      g_error_free (*error);
      *error = NULL;
    }
}


gboolean gw_console_search (char* query, char* fuzzy, gboolean quiet, gboolean exact, GError **error)
{
    //Sanity check
    if (*error != NULL) return FALSE;

    //Declarations
    GwSearchItem *item;
    char *message_total;
    char *message_relevant;
    GwDictInfo *di;

    //Initializations
    di = gw_dictinfolist_get_dictinfo_fuzzy (fuzzy);
    if (di == NULL)
    {
      printf (gettext("Requested dictionary not found!\n"));
      return FALSE;
    }

    item = gw_searchitem_new (query, di, GW_TARGET_CONSOLE);
    if (item == NULL)
    {
      printf(gettext("Query parse error\n"));
      return FALSE;
    }

    //Print the search intro
    if (!quiet)
    {
      gw_console_start_banner (query, di->longname);
    }

    //Print the results
    gw_engine_get_results (item, FALSE, exact);

    //Print final header
    if (quiet == FALSE)
    {
      message_total = ngettext("Found %d result", "Found %d results", item->total_results);
      message_relevant = ngettext("(%d Relevant)", "(%d Relevant)", item->total_relevant_results);
      printf(message_total, item->total_results);
      if (item->total_relevant_results != item->total_results)
        printf(message_relevant, item->total_relevant_results);
      printf("\n");
    }

    //Cleanup
    gw_searchitem_free (item);

    return TRUE;
}
