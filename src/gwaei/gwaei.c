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
//! @file src/gwaei.c
//!
//! @brief Main entrance into the program.
//!
//! Main entrance into the program.
//!

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <gio/gio.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>

GwApplication *app;

//!
//! @brief creates a new instance of the gwaei applicaiton
//!
GwApplication* gw_app_new (int* argc, char** argv[])
{
    GwApplication* temp;

    temp = (GwApplication*) malloc(sizeof(GwApplication));

    if (temp != NULL)
    {
      bindtextdomain(PACKAGE, GWAEI_LOCALEDIR);
      bind_textdomain_codeset(PACKAGE, "UTF-8");
      textdomain(PACKAGE);

      gdk_threads_init();
      gtk_init (argc, argv);

      temp->arg_dictionary = NULL;
      temp->arg_query = NULL;
#ifdef WITH_LIBUNIQUE
      temp->arg_new_instance = FALSE;
#endif
      temp->arg_version_switch = FALSE;

      gw_app_parse_args (temp, argc, argv);

      lw_engine_initialize (
                           gw_output_append_edict_results_cb,
                           gw_output_append_kanjidict_results_cb,
                           gw_output_append_examplesdict_results_cb,
                           gw_output_append_unknowndict_results_cb,
                           gw_output_append_less_relevant_header_cb,
                           gw_output_append_more_relevant_header_cb,
                           gw_output_pre_search_prep_cb,
                           gw_output_after_search_cleanup_cb
                          );


      temp->prefmanager = lw_prefmanager_new ();
      temp->dictinstlist = lw_dictinstlist_new (temp->prefmanager);
      temp->history = lw_historylist_new (20);

/*
      #ifdef WITH_LIBUNIQUE
      gw_libunique_initialize (temp->arg_new_instance, temp->arg_dictionary, temp->arg_query);
      #endif

      gw_dictionarymanager_initialize ();
      gw_dictionaryinstall_initialize ();
      gw_installprogress_initialize ();
      gw_kanjipad_initialize ();
*/

#ifdef ENABLE_WIN32
      GtkSettings *settings;
      settings = gtk_settings_get_default ();
      g_object_set (settings, "gtk-theme-name", "MS-Windows", NULL);
      g_object_set (settings, "gtk-menu-images", FALSE, NULL);
      g_object_set (settings, "gtk-button-images", FALSE, NULL);
      g_object_set (settings, "gtk-cursor-blink", FALSE, NULL);
      g_object_set (settings, "gtk-alternative-button-order", TRUE, NULL);
      g_object_unref (settings);
#endif
    }

    return temp;
}


void gw_app_free (GwApplication *app)
{
    //Declarations
    GList *iter;
    GwWindow *window;
    
    //Close all the open windows
    for (iter = app->windowlist; iter != NULL; iter = iter->next)
    {
      window = (GwWindow*) iter->data;
      if (window != NULL)
      {
        gw_window_destroy (window);
        iter->data = NULL;
      }
    }
    g_list_free (app->windowlist);

    gw_dictionarymanager_free (app->dictionarymanager);
/*
    #ifdef WITH_LIBUNIQUE
    gw_libunique_free ();
    #endif

    //ADD CLOSE TO CLOSE/DETSROY WINDOWS ON CLOSE
    lw_engine_free ();
*/
    g_option_context_free (app->context);
    g_free(app->arg_query);

    free (app);
}


//!
//! @brief Loads the arguments from the command line into the app instance
//!

void gw_app_parse_args (GwApplication *app, int *argc, char** argv[])
{
    if (app->arg_dictionary != NULL) g_free (app->arg_dictionary);
    if (app->arg_query != NULL) g_free (app->arg_query);

    GOptionEntry entries[] =
    {
      { "dictionary", 'd', 0, G_OPTION_ARG_STRING, &(app->arg_dictionary), gettext("Choose the dictionary to use"), "English" },
#ifdef WITH_LIBUNIQUE
      { "new-instance", 'n', 0, G_OPTION_ARG_NONE, &(app->arg_new_instance), gettext("Open a new instance of gWaei"), NULL },
#endif
      { "version", 'v', 0, G_OPTION_ARG_NONE, &(app->arg_version_switch), gettext("Check the gWaei version information"), NULL },
      { NULL }
    };

    //Program flags setup
    GError *error = NULL;
    app->context = g_option_context_new (gettext("- A dictionary program for Japanese-English translation."));
    g_option_context_add_main_entries (app->context, entries, PACKAGE);
    g_option_context_add_group (app->context, gtk_get_option_group (TRUE));
    g_option_context_parse (app->context, argc, argv, &error);

    if (error != NULL)
    {
      gw_common_handle_error (&error, NULL, FALSE);
      exit(1);
    }

    //Get the query after the flags have been parsed out
    app->arg_query = lw_util_get_query_from_args (*argc, *argv);
}



//!
//! @brief Prints to the terminal the about message for the program.
//!
void gw_app_print_about (GwApplication *app)
{
    printf ("gWaei version %s", VERSION);

    printf ("\n\n");

    printf ("Check for the latest updates at <http://gwaei.sourceforge.net/>\n");
    printf ("Code Copyright (C) 2009-2011 Zachary Dovel\n\n");

    printf ("License:\n");
    printf ("Copyright (C) 2008 Free Software Foundation, Inc.\nLicense GPLv3+: "
            "GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>\nThis"
            " is free software: you are free to change and redistribute it.\nThe"
            "re is NO WARRANTY, to the extent permitted by law.\n\n"             );
}


//!
//! @brief Equivalent to the main function for many programs.  This is what starts the program
//!
//! @param argc Your argc from your main function
//! @param argv Your array of strings from main
//!
void gw_app_run (GwApplication *app)
{
    //Declarations
    LwDictInfo *di;
    GwSearchWindow *window;

    window = (GwSearchWindow*) gw_app_show_window (app, GW_WINDOW_SEARCH, FALSE);

    gw_searchwindow_update_history_popups (window);

    //Show the settings dialog if no dictionaries are installed
    if (lw_dictinfolist_get_total ((LwDictInfoList*) app->dictionarymanager) == 0) {
      gw_app_show_window (app, GW_WINDOW_SETTINGS, FALSE);
    }

    //Set the initial dictionary
    if ((di = lw_dictinfolist_get_dictinfo_fuzzy ((LwDictInfoList*) app->dictionarymanager, app->arg_dictionary)) != NULL)
    {
      gw_searchwindow_set_dictionary (window, di->load_position);
    }

    //Set the initial query text if it was passed as an argument to the program
    if (app->arg_query != NULL)
    {
      gtk_entry_set_text (window->entry, app->arg_query);
    }

    //Enter the main loop
    gdk_threads_enter();
      gtk_main ();
    gdk_threads_leave();
}


void gw_app_quit (GwApplication *app)
{
    gtk_main_quit ();
}


//!
//! @brief Destroys a window, removing it from the window list
//! @param app The GwApplication instance
//! @param TYPE the GwWindowType to remove
//! @param widget A widget from the window so you can get a specific instance.  If NULL, you cet the first window to match the GwWindowType
//!
void gw_app_destroy_window (GwApplication *app, const GwWindowType TYPE, GtkWidget *widget)
{
    //Declarations
    GwWindow* window;
    GList *iter;
    
    //Initializations
    window = gw_app_get_window (app, TYPE, widget);
    iter = app->windowlist;

    while (iter != NULL)
    {
      if (iter->data == window)
      {
        app->windowlist = g_list_delete_link (app->windowlist, iter);
        break;
      }
      iter = iter->next;
    }

    if (window != NULL)
    {
      gw_window_destroy (window);
    }
}


//!
//! @brief Shows a window if it is available, otherwise creating it if it isn't or it is forced
//! @param app A GwApplication instance to work on
//! @param TYPE The window type to create
//! @param force_new Force a new instance even if a window of that type already exists
//! @returns Returns a pointer to the GwWindow that was shown
//!
GwWindow* gw_app_show_window (GwApplication *app, const GwWindowType TYPE, gboolean force_new) {
    //Declarations
    GwWindow *window;

    //Initializations
    window = gw_app_get_window (app, TYPE, NULL);

    if (window == NULL || force_new)
    {
      window = gw_window_new (TYPE);
      if (window != NULL)
      {
        app->windowlist = g_list_append (app->windowlist, window);
        gtk_widget_show (GTK_WIDGET (window->toplevel));
      }
    }
    else
    {
//      gw_window_update_parent (window);
      gtk_window_present (GTK_WINDOW (window->toplevel));
    }

    return window;
}


//!
//! @brief Returns the program name.  It should not be freed or modified
//! @returns A constanst string representing the program name
//!
const char* gw_app_get_program_name (GwApplication *app) 
{
  return gettext("gWaei Japanese-English Dictionary");
}


void gw_app_cancel_all_searches (GwApplication *app)
{
    GList *iter;
    GwSearchWindow *window;

    iter = app->windowlist;

    while (iter != NULL)
    {
      window = (GwSearchWindow*) iter->data;
      gw_searchwindow_cancel_all_searches (window);
      iter = iter->next;
    }
}


//!
//! @brief Gets a GwWindow from the application's windowlist
//! @param app A GwApplication instance to work on
//! @param TYPE The window type to get
//! @param widget A widget from the window so you can get a specific instance.  If NULL, you cet the first window to match the GwWindowType
//!
GwWindow* gw_app_get_window (GwApplication *app, const GwWindowType TYPE, GtkWidget *widget)
{
    //Declarations
    GList *iter;
    GwWindow *window;
    GtkWidget *toplevel;

    //Initializations
    iter = app->windowlist;
    window = NULL;
    toplevel = NULL;

    while (iter != NULL && window == NULL)
    {
      window = iter->data;
      if (widget != NULL)
        toplevel = gtk_widget_get_toplevel (widget);
      if (window->type != TYPE || (widget != NULL && window->toplevel != GTK_WINDOW (toplevel)))
        window = NULL;
      iter = iter->next;
    }

    return window;
}


int main (int argc, char *argv[])
{    
    lw_initialize (&argc, argv);
    app = gw_app_new (&argc, &argv);

    if (app->arg_version_switch)
      gw_app_print_about (app);
    else
      gw_app_run (app);

    gw_app_free (app);
    lw_free();
    gw_app_free (app);

    return EXIT_SUCCESS;
}

