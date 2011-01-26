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
static GOptionEntry _entries[] =
{
  { "dictionary", 'd', 0, G_OPTION_ARG_STRING, &_arg_dictionary, "Choose the dictionary to use", "English" },
#ifdef WITH_LIBUNIQUE
  { "new-instance", 'n', 0, G_OPTION_ARG_NONE, &_arg_new_instance, "Open a new instance of gWaei", NULL },
#endif
  { NULL }
};



void gw_frontend_initialize (int argc, char* argv[])
{
    gdk_threads_init();
    gtk_init (&argc, &argv);

    //Program flags setup
    GError *error = NULL;
    GOptionContext *context = g_option_context_new (gettext("- A dictionary program for Japanese-English translation."));
    g_option_context_add_main_entries (context, _entries, PACKAGE);
    g_option_context_add_group (context, gtk_get_option_group (TRUE));
    g_option_context_parse (context, &argc, &argv, &error);
    g_option_context_free (context);

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
    gw_kanjipad_initialize ();
    gdk_threads_leave ();

    #ifdef WITH_LIBSEXY
    gw_libsexy_initialize ();
    #endif

    gw_dictionary_manager_initialize ();
}


//!
//! @brief Equivalent to the main function for many programs.  This is what starts the program
//!
//! @param argc Your argc from your main function
//! @param argv Your array of strings from main
//!
void gw_frontend_start_gtk (int argc, char* argv[])
{
    gw_ui_update_history_popups ();
    gw_common_show_window ("main_window");

    //Spring up the prefs dialog if no dictionaries are installed
    //gw_settings_initialize_installed_dictionary_list ();
    if (gw_dictlist_get_total_with_status (GW_DICT_STATUS_INSTALLED) == 0) {
      do_settings(NULL, GINT_TO_POINTER (1));
    }

    //Set the initial focus to the search bar
    gw_ui_grab_focus_by_target (GW_TARGET_ENTRY);

/*
    //Set the initial dictionary
    if (_arg_dictionary != NULL)
    {
      GwDictInfo *di = gw_dictlist_get_dictinfo_by_alias (_arg_dictionary);
      if (di != NULL)
      {
        gw_ui_set_dictionary (di->load_position);
      }
    }
*/

    //Set the initial query text
    if (argc > 1)
    {
      //Setup the query text
      char *text = NULL;
      int i = 0;
      int length = 0;
      for (i = 0; i < argc; i++) length += strlen (argv[i]) + 1;
      text = (char*) malloc(length * sizeof(char) + 1);
      g_assert (text != NULL);
      for (i = 0; i < argc; i++) { strcat(text, argv[i]); strcat(text, " "); }
      text[strlen(text) - 1] = '\0';
      GtkWidget* entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);
      gtk_entry_set_text (GTK_ENTRY (entry), text);
      do_search (NULL, NULL);
      g_free (text);
      text = NULL;
    }


    //Enter the main loop
    gdk_threads_enter();

      //Add timers
      //g_timeout_add_full (G_PRIORITY_LOW, 100, (GSourceFunc)_keep_searching, NULL, NULL);
      g_timeout_add_full (G_PRIORITY_LOW, 100, (GSourceFunc) gw_ui_update_progress_feedback, NULL, NULL);
      g_timeout_add_full (G_PRIORITY_LOW, 1000, (GSourceFunc) gw_update_icons_for_selection, NULL, NULL);

      gtk_main ();

    gdk_threads_leave();
}

void gw_frontend_free ()
{
    gw_libsexy_free ();
    gw_libunique_free ();
    gw_kanjipad_free ();
    gw_settings_free ();
    gw_radsearchtool_free ();
    gw_main_free ();
    gw_engine_free ();
    gw_common_free ();
}

