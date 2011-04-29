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
//! @file src/console/frontend.c
//!
//! @brief Entry point for the gtk frontend
//!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <glib.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>

static gboolean _quiet_switch = FALSE;
static gboolean _exact_switch = FALSE;
static gboolean _list_switch = FALSE;
static gboolean _version_switch = FALSE;

static char* _dictionary_switch_data = NULL;
static char* _install_switch_data = NULL;
static char* _uninstall_switch_data = NULL;
static char* _query_text_data = NULL;
static GOptionContext *_context = NULL;


//!
//! @brief Initializes the memory associated with frontend.c
//!
void gw_frontend_initialize (int* argc, char* argv[])
{
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "UTF-8");
    textdomain(PACKAGE);

    //Declarations
    GError *error;
    char *summary_text;
    char *description_text;

    //Initializations
    error = NULL;
    _context = g_option_context_new (gettext("- A dictionary program for Japanese-English translation."));
    summary_text = gettext("waei generally outputs directly to the console.  "
                           "If you want to do multiple\nsearches, please start"
                           " gWaei with the -n switch for the multisearch mode.");
    description_text = g_strdup_printf(
        gettext(
           "Examples:\n"
           "  waei English               Search for the english word English\n"
           "  waei \"cats&dogs\"           Search for results containing cats and dogs\n"
           "  waei \"cats|dogs\"           Search for results containing cats or dogs\n"
           "  waei cats dogs             Search for results containing \"cats dogs\"\n"
           "  waei %s                Search for the Japanese word %s\n"
           "  waei -e %s               Search for %s and ignore similar results\n"
           "  waei %s                 When you don't know a kanji character\n"
           "  waei -d Kanji %s           Find a kanji character in the kanji dictionary\n"
           "  waei -d Names %s       Look up a name in the names dictionary\n"
           "  waei -d Places %s       Look up a place in the places dictionary"
         )
         , "にほん", "にほん", "日本", "日本", "日.語", "魚", "Miyabe", "Tokyo"
    );
    GOptionEntry entries[] = {
#ifdef WITH_NCURSES
      { "ncurses", 'n', 0, G_OPTION_ARG_NONE, &_ncurses_switch, gettext("Open up the multisearch window"), NULL },
#endif
      { "exact", 'e', 0, G_OPTION_ARG_NONE, &_exact_switch, gettext("Do not display less relevant results"), NULL },
      { "quiet", 'q', 0, G_OPTION_ARG_NONE, &_quiet_switch, gettext("Display less information"), NULL },
      { "dictionary", 'd', 0, G_OPTION_ARG_STRING, &_dictionary_switch_data, gettext("Search using a chosen dictionary"), NULL },
      { "list", 'l', 0, G_OPTION_ARG_NONE, &_list_switch, gettext("Show available dictionaries for searches"), NULL },
      { "install", 'i', 0, G_OPTION_ARG_STRING, &_install_switch_data, gettext("Install dictionary"), NULL },
      { "uninstall", 'u', 0, G_OPTION_ARG_STRING, &_uninstall_switch_data, gettext("Uninstall dictionary"), NULL },
      { "version", 'v', 0, G_OPTION_ARG_NONE, &_version_switch, gettext("Check the waei version information"), NULL },
      { NULL }
    };

    g_option_context_set_description (_context, description_text);
    g_option_context_set_summary (_context, summary_text);
    g_option_context_add_main_entries (_context, entries, PACKAGE);
    g_option_context_parse (_context, argc, &argv, &error);

    _query_text_data = gw_util_get_query_from_args (*argc, argv);

    gw_engine_initialize (
                         gw_console_append_edict_results_to_buffer,
                         gw_console_append_kanjidict_results_to_buffer,
                         gw_console_append_examplesdict_results_to_buffer,
                         gw_console_append_unknowndict_results_to_buffer,
                         gw_console_append_less_relevant_header_to_output,
                         gw_console_append_more_relevant_header_to_output,
                         gw_console_pre_search_prep,
                         gw_console_after_search_cleanup
                        );

    //Cleanup
    g_free (description_text);

    //Error handling
    if (error != NULL)
    {
      printf("%s\n", error->message);
      g_error_free (error);
      error = NULL;
      exit (EXIT_FAILURE);
    }
}


//!
//! @brief Equivalent to the main function for many programs.  This is what starts the program
//! @param argc Your argc from your main function
//! @param argv Your array of strings from main
//!
int gw_frontend_start_console (int argc, char* argv[])
{
    //Declarations
    GError *error;
    gboolean resolution;

    //Initializations
    error = NULL;
    resolution = 0;

    //User wants to see what dictionaries are available
    if (_list_switch)
      gw_console_list ();

    //User wants to see the version of waei
    else if (_version_switch)
      gw_console_about ();

    //User wants to install a dictionary
    else if (_install_switch_data != NULL)
      resolution = gw_console_install_dictinst (_install_switch_data, &error);

    //User wants to uninstall a dictionary
    else if (_uninstall_switch_data != NULL)
      resolution = gw_console_uninstall_dictinfo (_uninstall_switch_data, &error);

    //User wants to do a search
    else if (_query_text_data != NULL)
      resolution = gw_console_search (_query_text_data, _dictionary_switch_data, _quiet_switch, _exact_switch, &error);

    //User didn't specify enough information for an action
    else 
      printf("%s\n", g_option_context_get_help (_context, TRUE, NULL));

    //Cleanup
    if (error != NULL)
      gw_console_handle_error (&error);
    else
      printf("\nDone\n\n");

    return resolution;
}


//!
//! @brief Frees the memory associated with frontend.c
//!
void gw_frontend_free ()
{
    gw_engine_free ();
    free (_query_text_data);
    g_option_context_free (_context);
}

