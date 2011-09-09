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
    GError *error;

    temp = (WApplication*) malloc (sizeof(WApplication));
    error = NULL;

    if (temp != NULL)
    {
      w_app_init (temp, argc, argv, &error);

      if (error != NULL)
      {
        w_app_free (temp);
        temp = NULL;
        w_console_handle_error (temp, &error);
      }
    }

    return temp;
}


//!
//! @brief Frees the memory associated with waei.c
//!
void w_app_free (WApplication *app)
{
    w_app_deinit (app);
    free (app);
}


void w_app_init (WApplication* app, int *argc, char** argv[], GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return;

    setlocale(LC_MESSAGES, "");
    setlocale(LC_CTYPE, "");
    setlocale(LC_COLLATE, "");

    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (PACKAGE, "UTF-8");
    textdomain(PACKAGE);

    g_thread_init (NULL);
    g_type_init ();

    app->quiet_switch = FALSE;
    app->exact_switch = FALSE;
    app->list_switch = FALSE;
    app->version_switch = FALSE;
    app->color_switch = FALSE;

    app->dictionary_switch_data = NULL;
    app->install_switch_data = NULL;
    app->uninstall_switch_data = NULL;
    app->query_text_data = NULL;
    app->context = NULL;

    w_app_parse_args (app, argc, argv, error);

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

    app->prefmanager = lw_preferences_new ();
    app->dictinstlist = lw_dictinstlist_new (app->prefmanager);
    app->dictinfolist = lw_dictinfolist_new (20, app->prefmanager);
}


void w_app_deinit (WApplication *app)
{
    lw_dictinfolist_free (app->dictinfolist);
    lw_dictinstlist_free (app->dictinstlist);
    lw_preferences_free (app->prefmanager);
    lw_engine_free (app->engine);

    g_free (app->query_text_data);
    g_free (app->dictionary_switch_data);
    g_free (app->uninstall_switch_data);
    g_free (app->install_switch_data);

    g_option_context_free (app->context);


}


//!
//! @brief Initializes the memory associated with waei.c
//!
void w_app_parse_args (WApplication* app, int* argc, char** argv[], GError **error)
{
    //Sanity check
    if (error != NULL && *error != NULL) return;

    //Declarations
    char *summary_text;
    char *description_text;

    //Initializations
    app->context = g_option_context_new (gettext("- A dictionary program for Japanese-English translation."));
    summary_text = gettext("waei generally outputs directly to the console.");
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
    g_option_context_parse (app->context, argc, argv, error);

    if (app->query_text_data != NULL)
      g_free (app->query_text_data);
    app->query_text_data = lw_util_get_query_from_args (*argc, *argv);

    //Cleanup
    g_free (description_text);
}


//!
//! @brief Equivalent to the main function for many programs.  This is what starts the program
//! @param argc Your argc from your main function
//! @param argv Your array of strings from main
//!
WAppResolution w_app_start_console (WApplication *app)
{
    //Declarations
    GError *error;
    WAppResolution resolution;

    //Initializations
    resolution = W_APP_RESOLUTION_SUCCESS;
    error = NULL;

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
      resolution = w_console_search (app, &error);

    //User didn't specify enough information for an action
    else 
      printf("%s\n", g_option_context_get_help (app->context, TRUE, NULL));

    //Cleanup
    w_console_handle_error (app, &error);

    return resolution;
}


int main (int argc, char *argv[])
{    
    //Declarations
    WAppResolution resolution;

    //Initializations
    app = w_app_new (&argc, &argv);
    resolution = W_APP_RESOLUTION_OUT_OF_MEMORY;

    if (app != NULL)
    {
      resolution = w_app_start_console (app);
      w_app_free (app);
    }

    return resolution;
}




