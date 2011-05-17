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
//! @file src/gwaei/frontend.c
//!
//! @brief Entry point for the gtk frontend
//!

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>


static gchar   *_arg_dictionary = NULL;
static gboolean _arg_exact = FALSE;
static gboolean _arg_new_instance = FALSE;
static GOptionContext *_context = NULL;


void gw_frontend_initialize (int* argc, char* argv[])
{
    bindtextdomain(PACKAGE, GWAEI_LOCALEDIR);
    bind_textdomain_codeset(PACKAGE, "UTF-8");
    textdomain(PACKAGE);

    gdk_threads_init();
    gtk_init (argc, &argv);

    GOptionEntry entries[] =
    {
      { "dictionary", 'd', 0, G_OPTION_ARG_STRING, &_arg_dictionary, gettext("Choose the dictionary to use"), "English" },
#ifdef WITH_LIBUNIQUE
      { "new-instance", 'n', 0, G_OPTION_ARG_NONE, &_arg_new_instance, gettext("Open a new instance of gWaei"), NULL },
#endif
      { NULL }
    };

    //Program flags setup
    GError *error = NULL;
    _context = g_option_context_new (gettext("- A dictionary program for Japanese-English translation."));
    g_option_context_add_main_entries (_context, entries, PACKAGE);
    g_option_context_add_group (_context, gtk_get_option_group (TRUE));
    g_option_context_parse (_context, argc, &argv, &error);

    gw_engine_initialize (
                         gw_ui_append_edict_results_to_buffer,
                         gw_ui_append_kanjidict_results_to_buffer,
                         gw_ui_append_examplesdict_results_to_buffer,
                         gw_ui_append_unknowndict_results_to_buffer,
                         gw_ui_append_less_relevant_header_to_output,
                         gw_ui_append_more_relevant_header_to_output,
                         gw_ui_pre_search_prep,
                         gw_ui_after_search_cleanup
                        );


    gw_common_initialize ();
    gw_main_initialize ();
    gw_settings_initialize();

    #ifdef WITH_LIBUNIQUE
    gw_libunique_initialize (_arg_new_instance);
    #endif

    gw_radsearchtool_initialize ();

    gdk_threads_enter ();
    gdk_threads_leave ();

    gw_dictionarymanager_initialize ();
    gw_dictionaryinstall_initialize ();
    gw_installprogress_initialize ();
}


//!
//! @brief Equivalent to the main function for many programs.  This is what starts the program
//!
//! @param argc Your argc from your main function
//! @param argv Your array of strings from main
//!
void gw_frontend_start_gtk (int argc, char* argv[])
{
    //Declarations
    char *query;
    GtkWidget* entry;
    GwDictInfo *di;

    gw_ui_update_history_popups ();
    gw_common_show_window ("main_window");

    //Show the settings dialog if no dictionaries are installed
    if (gw_dictinfolist_get_total () == 0) {
      gw_main_settings_cb (NULL, GINT_TO_POINTER (1));
    }

    //Set the initial focus to the search bar
    gw_ui_grab_focus_by_target (GW_TARGET_ENTRY);

    //Set the initial dictionary
    if (_arg_dictionary != NULL) printf("%s\n", _arg_dictionary);
    if ((di = gw_dictinfolist_get_dictinfo_fuzzy (_arg_dictionary)) != NULL)
    {
      gw_ui_set_dictionary (di->load_position);
    }

    //Set the initial query text if it was passed as an argument to the program
    if ((query = gw_util_get_query_from_args (argc, argv)) != NULL)
    {
      entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);
      gtk_entry_set_text (GTK_ENTRY (entry), query);
      g_free(query);
    }

    //Enter the main loop
    gdk_threads_enter();

      //Add timers
      g_timeout_add_full (G_PRIORITY_LOW, 200, (GSourceFunc) gw_ui_keep_searching, NULL, NULL);
      g_timeout_add_full (G_PRIORITY_LOW, 200, (GSourceFunc) gw_ui_update_progress_feedback, NULL, NULL);
      //g_timeout_add_full (G_PRIORITY_LOW, 1000, (GSourceFunc) gw_update_icons_for_selection, NULL, NULL);

      gtk_main ();

    gdk_threads_leave();
}

void gw_frontend_free ()
{
    gw_dictionaryinstall_free ();
    gw_dictionarymanager_free ();
    #ifdef WITH_LIBUNIQUE
    gw_libunique_free ();
    #endif
    gw_settings_free ();
    gw_radsearchtool_free ();
    gw_main_free ();
    gw_engine_free ();
    gw_common_free ();
    g_option_context_free (_context);
}

