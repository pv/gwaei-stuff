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

#include <waei/waei.h>


//!
//! @brief Print the "no result" message where necessary.
//!
void w_console_no_result (LwSearchItem *item)
{
    printf("%s\n\n", gettext("No results found!"));
}


//!
//! @brief Uninstalls the named dictionary, deleting it.
//!
//! @param name A string of the name of the dictionary to uninstall.
//!
WAppResolution w_console_uninstall_dictinfo (WApplication* app, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return W_APP_RESOLUTION_DICTIONARY_UNINSTALL_ERROR;

    //Declarations
    LwDictInfo *di;
    WAppResolution resolution;

    //Initializations
    di = lw_dictinfolist_get_dictinfo_fuzzy (app->dictinfolist, app->uninstall_switch_data);
    resolution = W_APP_RESOLUTION_SUCCESS;

    if (di != NULL)
    {
      printf(gettext("Uninstalling %s...\n"), di->longname);
      lw_dictinfo_uninstall (di, w_console_uninstall_progress_cb, error);
    }
    else
    {
      printf("\n%s was not found!\n\n", app->uninstall_switch_data);
      w_console_print_available_dictionaries (app);
    }

    if (*error != NULL)
    {
      resolution = W_APP_RESOLUTION_DICTIONARY_UNINSTALL_ERROR;
    }

    return resolution;
}


//!
//! @brief Installs the named dictionary, deleting it.
//!
//! @param name A string of the name of the dictionary to install.
//!
WAppResolution w_console_install_dictinst (WApplication *app, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return W_APP_RESOLUTION_DICTIONARY_INSTALL_ERROR;

    //Declarations
    LwDictInst *di;
    WAppResolution resolution;

    //Initializations
    di = lw_dictinstlist_get_dictinst_fuzzy (app->dictinstlist, app->install_switch_data);
    resolution = W_APP_RESOLUTION_SUCCESS;

    if (di != NULL)
    {
      printf(gettext("Installing %s...\n"), di->longname);
      lw_dictinst_install (di, w_console_install_progress_cb, di, error);
    }
    else
    {
      printf("\n%s was not found!\n\n", app->install_switch_data);
      w_console_print_installable_dictionaries (app);
    }

    if (*error != NULL)
    {
      resolution = W_APP_RESOLUTION_DICTIONARY_INSTALL_ERROR;
    }

    return resolution;
}


//!
//! @brief Prints to the terminal the about message for the program.
//!
void w_console_about (WApplication* app)
{
    printf ("waei version %s", VERSION);

    printf ("\n\n");

    printf ("Check for the latest updates at <http://gwaei.sourceforge.net/>\n");
    printf ("Code Copyright (C) 2009-2011 Zachary Dovel\n\n");

    printf ("License:\n");
    printf ("Copyright (C) 2008 Free Software Foundation, Inc.\nLicense GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\nThis is free software: you are free to change and redistribute it.\nThere is NO WARRANTY, to the extent permitted by law.\n\n");
}


//!
//! @brief Prints out the yet uninstalled available dictionaries.
//!
void w_console_print_installable_dictionaries (WApplication *app)
{
    printf(gettext("Installable dictionaries are:\n"));

    //Declarations
    int i;
    int j;
    GList *iter;
    LwDictInst* di;

    //Initializations
    i = 0; 
    iter = app->dictinstlist->list;

    while (iter != NULL)
    {
      di = (LwDictInst*) iter->data;
      if (lw_dictinst_data_is_valid (di))
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
void w_console_print_available_dictionaries (WApplication *app)
{
    //Declarations
    int i;
    int j;
    LwDictInfo* di;
    GList *iter;

    //Initializations
    i = 0;
    j = 0;
	  iter = app->dictinstlist->list;

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
//! @brief Lists the available and installable dictionaries
//!
void w_console_list (WApplication *app)
{
    w_console_print_available_dictionaries (app);
    w_console_print_installable_dictionaries (app);
}


//!
//! @brief If the GError is set, it prints it and frees the memory
//! @param error A pointer to a gerror pointer
//!
void w_console_handle_error (WApplication* app, GError **error)
{
    if (error != NULL && *error != NULL)
    {
      fprintf(stderr, "Error: %s\n", (*error)->message);
      g_error_free (*error);
      *error = NULL;
    }
}


WAppResolution w_console_search (WApplication *app, GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return FALSE;

    //Declarations
    LwSearchItem *item;
    char *message_total;
    char *message_relevant;
    LwDictInfo *di;
    WAppResolution resolution;

    //Initializations
    di = lw_dictinfolist_get_dictinfo_fuzzy (app->dictinfolist, app->dictionary_switch_data);
    item = lw_searchitem_new (app->query_text_data, di, LW_OUTPUTTARGET_RESULTS, app->prefmanager, error);
    resolution = W_APP_RESOLUTION_SUCCESS;

    //Sanity checks
    if (di == NULL)
    {
      resolution = W_APP_RESOLUTION_INVALID_DICTIONARY;
      fprintf (stderr, gettext("Requested dictionary not found!\n"));
      return resolution;
    }

    if (item == NULL)
    {
      resolution = W_APP_RESOLUTION_INVALID_QUERY;
      return resolution;
    }

    //Print the search intro
    if (!app->quiet_switch)
    {
      // TRANSLATORS: 'Searching for "${query}" in ${dictionary long name}'
      printf(gettext("Searching for \"%s\" in %s...\n"), app->query_text_data, di->longname);
      printf("\n");
    }

    //Print the results
    lw_engine_get_results (app->engine, item, FALSE, app->exact_switch);

    //Print final header
    if (app->quiet_switch == FALSE)
    {
      message_total = ngettext("Found %d result", "Found %d results", item->total_results);
      message_relevant = ngettext("(%d Relevant)", "(%d Relevant)", item->total_relevant_results);
      printf(message_total, item->total_results);
      if (item->total_relevant_results != item->total_results)
        printf(message_relevant, item->total_relevant_results);
      printf("\n");
    }

    //Cleanup
    lw_searchitem_free (item);

    return resolution;
}
