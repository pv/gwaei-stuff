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
//! @file src/console/waei.c
//!
//! @brief Main entrance into the program.
//!
//! Main entrance into the program.
//!

#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <glib.h>

#include <waei/waei.h>


WApplication *app;


WApplication* w_app_new (int *argc, char** argv[])
{
    WApplication *temp;

    temp = (WApplication*) malloc (sizeof(WApplication));

    if (temp != NULL)
    {
      setlocale(LC_MESSAGES, "");
      setlocale(LC_CTYPE, "");
      setlocale(LC_COLLATE, "");

      bindtextdomain(PACKAGE, LOCALEDIR);
      bind_textdomain_codeset (PACKAGE, "UTF-8");
      textdomain(PACKAGE);

      g_thread_init (NULL);
      g_type_init ();

      temp->quiet_switch = FALSE;
      temp->exact_switch = FALSE;
      temp->list_switch = FALSE;
      temp->version_switch = FALSE;
#ifdef WITH_NCURSES
      temp->ncurses_switch = FALSE;
#endif
      temp->color_switch = FALSE;

      temp->dictionary_switch_data = NULL;
      temp->install_switch_data = NULL;
      temp->uninstall_switch_data = NULL;
      temp->query_text_data = NULL;
      temp->context = NULL;

      w_app_parse_args (temp, argc, argv);

      temp->engine = NULL;
      temp->prefmanager = lw_prefmanager_new ();
      temp->dictinstlist = lw_dictinstlist_new (temp->prefmanager);
      temp->dictinfolist = lw_dictinfolist_new (20, temp->prefmanager);
    }

    return temp;
}

//!
//! @brief Initializes the memory associated with waei.c
//!
void w_app_parse_args (WApplication* app, int* argc, char** argv[])
{
    //Declarations
    GError *error;
    char *summary_text;
    char *description_text;

    //Initializations
    error = NULL;
    app->context = g_option_context_new (gettext("- A dictionary program for Japanese-English translation."));
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
      { "ncurses", 'n', 0, G_OPTION_ARG_NONE, &(app->ncurses_switch), gettext("Open up the multisearch window"), NULL },
#endif
      { "exact", 'e', 0, G_OPTION_ARG_NONE, &(app->exact_switch), gettext("Do not display less relevant results"), NULL },
      { "quiet", 'q', 0, G_OPTION_ARG_NONE, &(app->quiet_switch), gettext("Display less information"), NULL },
      { "color", 'c', 0, G_OPTION_ARG_NONE, &(app->color_switch), gettext("Display results with color"), NULL },
      { "dictionary", 'd', 0, G_OPTION_ARG_STRING, &(app->dictionary_switch_data), gettext("Search using a chosen dictionary"), NULL },
      { "list", 'l', 0, G_OPTION_ARG_NONE, &(app->list_switch), gettext("Show available dictionaries for searches"), NULL },
      { "install", 'i', 0, G_OPTION_ARG_STRING, &(app->install_switch_data), gettext("Install dictionary"), NULL },
      { "uninstall", 'u', 0, G_OPTION_ARG_STRING, &(app->uninstall_switch_data), gettext("Uninstall dictionary"), NULL },
      { "version", 'v', 0, G_OPTION_ARG_NONE, &(app->version_switch), gettext("Check the waei version information"), NULL },
      { NULL }
    };

    g_option_context_set_description (app->context, description_text);
    g_option_context_set_summary (app->context, summary_text);
    g_option_context_add_main_entries (app->context, entries, PACKAGE);
    g_option_context_parse (app->context, argc, argv, &error);

    if (app->query_text_data != NULL)
      g_free (app->query_text_data);
    app->query_text_data = lw_util_get_query_from_args (*argc, *argv);

    //Cleanup
    g_free (description_text);

    //Error handling
    if (error != NULL)
    {
      fprintf(stderr, "%s\n", error->message);
      g_error_free (error);
      error = NULL;
      exit (EXIT_FAILURE);
    }

}


//!
//! @brief Frees the memory associated with waei.c
//!
void w_app_free (WApplication *app)
{
    lw_dictinfolist_free (app->dictinfolist);
    lw_dictinstlist_free (app->dictinstlist);
    lw_prefmanager_free (app->prefmanager);
    lw_engine_free (app->engine);

    g_free (app->query_text_data);
    g_free (app->dictionary_switch_data);
    g_free (app->uninstall_switch_data);
    g_free (app->install_switch_data);

    g_option_context_free (app->context);

    free (app);
}


//!
//! @brief Equivalent to the main function for many programs.  This is what starts the program
//! @param argc Your argc from your main function
//! @param argv Your array of strings from main
//!
int w_app_start_console (WApplication *app)
{
    //Declarations
    GError *error;
    gboolean resolution;

    //Initializations
    error = NULL;
    resolution = 0;

    //User wants to see what dictionaries are available
    if (app->list_switch)
      w_console_list (app);

    //User wants to see the version of waei
    else if (app->version_switch)
      w_console_about (app);

    //User wants to install a dictionary
    else if (app->install_switch_data != NULL)
      resolution = w_console_install_dictinst (app, &error);

    //User wants to uninstall a dictionary
    else if (app->uninstall_switch_data != NULL)
      resolution = w_console_uninstall_dictinfo (app, &error);

    //User wants to do a search
    else if (app->query_text_data != NULL)
    {
      app->engine = lw_engine_new (
          w_console_append_edict_result_cb,
          w_console_append_kanjidict_result_cb,
          w_console_append_examplesdict_result_cb,
          w_console_append_unknowndict_result_cb,
          w_console_append_less_relevant_header_cb,
          w_console_append_more_relevant_header_cb,
          w_console_prepare_search_cb,
          w_console_cleanup_search_cb
      );
      resolution = w_console_search (app, &error);
    }

    //User didn't specify enough information for an action
    else 
      printf("%s\n", g_option_context_get_help (app->context, TRUE, NULL));

    //Cleanup
    if (error != NULL)
      w_console_handle_error (app, &error);
    else if (app->quiet_switch == FALSE)
      printf("\nDone\n\n");

    return resolution;
}


//!
//! @brief Equivalent to the main function for many programs.  This is what starts the program
//! @param argc Your argc from your main function
//! @param argv Your array of strings from main
//!
int w_app_start_ncurses (WApplication *app)
{
#ifdef WITH_NCURSES
    app->engine = lw_engine_new (
                         nw_output_append_edict_result_cb,
                         nw_output_append_kanjidict_result_cb,
                         nw_output_append_examplesdict_result_cb,
                         nw_output_append_unknowndict_result_cb,
                         nw_output_append_less_relevant_header_cb,
                         nw_output_append_more_relevant_header_cb,
                         nw_output_prepare_search_cb,
                         nw_output_cleanup_search_cb
                        );


    nw_start_ncurses (app);
#endif
    return TRUE;
}


int main (int argc, char *argv[])
{    
    //Declarations
    int resolution;
    app = w_app_new (&argc, &argv);

#ifdef WITH_NCURSES
    //Start the program
    if (app->ncurses_switch)
      resolution = w_app_start_ncurses (app);
    else
#endif
      resolution = w_app_start_console (app);

    w_app_free (app);
    return resolution;
}




