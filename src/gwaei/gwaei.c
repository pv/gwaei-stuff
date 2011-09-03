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
      setlocale(LC_MESSAGES, "");
      setlocale(LC_CTYPE, "");
      setlocale(LC_COLLATE, "");

      bindtextdomain(PACKAGE, LOCALEDIR);
      bind_textdomain_codeset (PACKAGE, "UTF-8");
      textdomain(PACKAGE);

      g_thread_init (NULL);
      g_type_init ();
      gdk_threads_init ();
      gtk_init (argc, argv);

      temp->arg_dictionary = NULL;
      temp->arg_query = NULL;
#ifdef WITH_LIBUNIQUE
      temp->arg_new_window = FALSE;
#endif
      temp->arg_version_switch = FALSE;
      temp->windowlist = NULL;
      temp->last_focused = NULL;

      gw_app_parse_args (temp, argc, argv);

      temp->engine = lw_engine_new (
                           gw_output_append_edict_results_cb,
                           gw_output_append_kanjidict_results_cb,
                           gw_output_append_examplesdict_results_cb,
                           gw_output_append_unknowndict_results_cb,
                           gw_output_append_less_relevant_header_cb,
                           gw_output_append_more_relevant_header_cb,
                           gw_output_prepare_search_cb,
                           gw_output_cleanup_search_cb
                          );


      temp->prefmanager = lw_prefmanager_new ();
      temp->dictinfolist = gw_dictinfolist_new (20, temp->prefmanager);
      temp->block_new_searches = 0;

      temp->tagtable = gw_texttagtable_new ();
      lw_prefmanager_add_change_listener_by_schema (temp->prefmanager, LW_SCHEMA_HIGHLIGHT, LW_KEY_MATCH_FG, gw_app_sync_tag_cb, temp);
      lw_prefmanager_add_change_listener_by_schema (temp->prefmanager, LW_SCHEMA_HIGHLIGHT, LW_KEY_MATCH_BG, gw_app_sync_tag_cb, temp);
      lw_prefmanager_add_change_listener_by_schema (temp->prefmanager, LW_SCHEMA_HIGHLIGHT, LW_KEY_HEADER_FG, gw_app_sync_tag_cb, temp);
      lw_prefmanager_add_change_listener_by_schema (temp->prefmanager, LW_SCHEMA_HIGHLIGHT, LW_KEY_HEADER_BG, gw_app_sync_tag_cb, temp);
      lw_prefmanager_add_change_listener_by_schema (temp->prefmanager, LW_SCHEMA_HIGHLIGHT, LW_KEY_COMMENT_FG, gw_app_sync_tag_cb, temp);

      #ifdef WITH_LIBUNIQUE
      gw_libunique_initialize (temp->arg_new_window, temp->arg_dictionary, temp->arg_query);
      #endif

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
      window = GW_WINDOW (iter->data);
      if (window != NULL)
      {
        gw_window_destroy (window);
        iter->data = NULL;
      }
    }
    g_list_free (app->windowlist);

    gw_dictinfolist_free (app->dictinfolist);

    #ifdef WITH_LIBUNIQUE
    gw_libunique_free ();
    #endif

    g_option_context_free (app->context);
    g_free(app->arg_query);
    lw_engine_free (app->engine);
    lw_prefmanager_free (app->prefmanager);

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
      { "new-window", 'n', 0, G_OPTION_ARG_NONE, &(app->arg_new_window), gettext("Open a new search window"), NULL },
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
      gw_app_handle_error (app, NULL, FALSE, &error);
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
GwApplicationResolution gw_app_run (GwApplication *app)
{
    //Declarations
    LwDictInfo *di;
    GwSearchWindow *window;

    window = GW_SEARCHWINDOW (gw_app_show_window (app, GW_WINDOW_SEARCH, NULL, FALSE));

    gw_searchwindow_update_history_popups (window);

    //Show the settings dialog if no dictionaries are installed
    if (lw_dictinfolist_get_total (LW_DICTINFOLIST (app->dictinfolist)) == 0) {
      gw_app_show_window (app, GW_WINDOW_SETTINGS, GW_WINDOW (window), FALSE);
    }

    //Set the initial dictionary
    if ((di = lw_dictinfolist_get_dictinfo_fuzzy (LW_DICTINFOLIST (app->dictinfolist), app->arg_dictionary)) != NULL)
    {
      gw_searchwindow_set_dictionary (window, di->load_position);
    }

    //Set the initial query text if it was passed as an argument to the program
    if (app->arg_query != NULL)
    {
      gtk_entry_set_text (window->entry, app->arg_query);
      gw_searchwindow_search_cb (GTK_WIDGET (window->entry), window->toplevel);
    }

    //Enter the main loop
    gdk_threads_enter();
      gtk_main ();
    gdk_threads_leave();

    return GW_APP_RESOLUTION_NO_ERRORS;
}


void gw_app_quit (GwApplication *app)
{
    gw_app_block_searches (app);
    gtk_main_quit ();
    gw_app_unblock_searches (app);
}


//!
//! @brief Destroys a window, removing it from the window list
//! @param app The GwApplication instance
//! @param widget A widget from the window so you can get a specific instance.  If NULL, you cet the first window to match the GwWindowType
//!
void gw_app_destroy_window (GwApplication *app, GwWindow *window)
{
    //Sanity check
    g_assert (window != NULL);

    //Declarations
    GList *iter;
    
    //Remove it from the window list
    for (iter = app->windowlist; iter != NULL; iter = iter->next)
    {
      if (GW_WINDOW (iter->data) == window)
      {
        app->windowlist = g_list_delete_link (app->windowlist, iter);
        break;
      }
    }

    //Destroy the window itself
    if (window != NULL)
    {
      gw_window_destroy (window);
    }

    //Quit the app if that was the last search window
    if (gw_app_get_window_by_type (app, GW_WINDOW_SEARCH) == NULL)
    {
      gw_app_quit (app);
    }
}


//!
//! @brief Shows a window if it is available, otherwise creating it if it isn't or it is forced
//! @param app A GwApplication instance to work on
//! @param TYPE The window type to create
//! @param force_new Force a new instance even if a window of that type already exists
//! @returns Returns a pointer to the GwWindow that was shown
//!
GwWindow* gw_app_show_window (GwApplication *app, const GwWindowType TYPE, GwWindow *transient_for, gboolean force_new)
{
    //Declarations
    GwWindow *window;
    GList *link;

    //Initializations
    window = gw_app_get_window_by_type (app, TYPE);

    if (window == NULL || force_new)
    {
      app->windowlist = g_list_append (app->windowlist, NULL);
      link = g_list_find (app->windowlist, NULL);
      window = gw_window_new (TYPE, transient_for, link);
      if (window != NULL)
        gtk_widget_show (GTK_WIDGET (window->toplevel));
      else
        app->windowlist = g_list_delete_link (app->windowlist, link);
    }
    else
    {
      gw_window_set_transient_for (window, transient_for);
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
    GwWindow *window;

    for (iter = app->windowlist; iter != NULL; iter = iter->next)
    {
      window = GW_WINDOW (iter->data);
      if (window != NULL && window->type == GW_WINDOW_SEARCH)
        gw_searchwindow_cancel_all_searches (GW_SEARCHWINDOW (window));
    }
}


//!
//!  @brief Will attempt to get the window of the specified type which is most at the front
//!
GwWindow* gw_app_get_window_by_type (GwApplication *app, const GwWindowType TYPE)
{
    //Declarations
    GList *iter;
    GwWindow *window;
    GwWindow *active;
    GwWindow *fuzzy;
    GtkWindow *toplevel;

    //Initializations
    iter = app->windowlist;
    window = NULL;
    fuzzy = NULL;
    toplevel = NULL;
    active = NULL;

    for (iter = app->windowlist; iter != NULL; iter = iter->next)
    {
      fuzzy = GW_WINDOW (iter->data);
      active = GW_WINDOW (iter->data);

      if (fuzzy == NULL)
      {
        continue;
      }
      if (active->type == TYPE && gtk_window_is_active (active->toplevel))
      {
        window = active;
        break;
      }
      if (fuzzy->type == TYPE)
      {
        window = fuzzy;
      }
    }

    return window;
}


//!
//! @brief Gets a GwWindow from the application's windowlist
//! @param app A GwApplication instance to work on
//! @param TYPE The window type to get
//! @param widget A widget from the window so you can get a specific instance.  If NULL, you cet the first window to match the GwWindowType
//!
GwWindow* gw_app_get_window_by_widget (GwApplication *app, GtkWidget *widget)
{
    //Declarations
    GList *iter;
    GwWindow *window;
    GwWindow *active;
    GwWindow *fuzzy;
    GtkWindow *toplevel;

    //Initializations
    window = NULL;
    fuzzy = NULL;
    toplevel = NULL;
    active = NULL;

    for (iter = app->windowlist; iter != NULL; iter = iter->next)
    {
      fuzzy = GW_WINDOW (iter->data);
      toplevel = GTK_WINDOW (gtk_widget_get_toplevel (widget));

      if (fuzzy == NULL)
      {
        continue;
      }
      else if (fuzzy->toplevel == toplevel)
      {
        window = fuzzy;
        break;
      }
    }

    return window;
}


int main (int argc, char *argv[])
{    
    int resolution;
    app = gw_app_new (&argc, &argv);

    if (app->arg_version_switch)
      gw_app_print_about (app);
    else
      resolution = gw_app_run (app);

    gw_app_free (app);

    return resolution;
}


//!
//! @brief Adds the tags to stylize the buffer text
//!
GtkTextTagTable* gw_texttagtable_new ()
{
    GtkTextTagTable *temp;
    GtkTextTag *tag;

    temp = gtk_text_tag_table_new ();

    if (temp != NULL)
    {
      tag = gtk_text_tag_new ("italic");
      g_object_set (tag, "style", PANGO_STYLE_ITALIC, NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("gray");
      g_object_set (tag, "foreground", "#888888", NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("smaller");
      g_object_set (tag, "size", "smaller", NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("small");
      g_object_set (tag, "font", "Serif 6", NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("important");
      g_object_set (tag, "weight", PANGO_WEIGHT_BOLD, NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("larger");
      g_object_set (tag, "font", "Sans 20", NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("large");
      g_object_set (tag, "font", "Serif 40", NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("center");
      g_object_set (tag, "justification", GTK_JUSTIFY_LEFT, NULL);
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("comment");
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("match");
      gtk_text_tag_table_add (temp, tag);

      tag = gtk_text_tag_new ("header");
      gtk_text_tag_table_add (temp, tag);
    }

    return temp;
}


//!
//! @brief Resets the color tags according to the preferences
//!
void gw_app_sync_tag_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    char hex[10];
    GdkRGBA color;
    gchar **pair;
    GtkTextTag *tag;
    GwApplication *app;

    app = GW_APPLICATION (data);

    //Parse the color
    lw_prefmanager_get_string (hex, settings, key, 10);
    if (gdk_rgba_parse (&color, hex) == FALSE)
    {
      fprintf(stderr, "color failed %s\n", hex);
      lw_prefmanager_reset_value_by_schema (app->prefmanager, LW_SCHEMA_HIGHLIGHT, key);
      return;
    }

    //Update the tag 
    pair = g_strsplit (key, "-", 2);
    if (pair != NULL && pair[0] != NULL && pair[1] != NULL)
    {
      tag = gtk_text_tag_table_lookup (app->tagtable, pair[0]);
      g_object_set (G_OBJECT (tag), pair[1], hex, NULL);
      g_strfreev (pair);
    }
}


void gw_app_block_searches (GwApplication *app)
{
  app->block_new_searches++;
  gw_app_cancel_all_searches (app);
}


void gw_app_unblock_searches (GwApplication *app)
{
  if (app->block_new_searches > 0)
    app->block_new_searches--;
}

gboolean gw_app_can_start_search (GwApplication *app)
{
  return (app->block_new_searches == 0);
}


void gw_app_handle_error (GwApplication *app, GwWindow *transient_for, gboolean show_dialog, GError **error)
{
    //Sanity checks
    if (error != NULL && *error != NULL) return;

    //Declarations
    GtkWidget *dialog;
    GtkWindow *parent;
    gint response;

    if (transient_for == NULL)
      parent = NULL;
    else
      parent = transient_for->toplevel;

    //Handle the error
    if (show_dialog)
    {
      dialog = gtk_message_dialog_new_with_markup (parent,
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "<b>%s</b>\n\n%s",
                                                   "An Error Occured",
                                                   (*error)->message
                                                  );
      g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
      gtk_widget_show_all (GTK_WIDGET (dialog));
      response = gtk_dialog_run (GTK_DIALOG (dialog));
    }
    else
    {
      fprintf(stderr, "ERROR: %s\n", (*error)->message);
    }

    //Cleanup
    g_error_free (*error);
    *error = NULL;
}


void gw_app_set_last_focused_searchwindow (GwApplication *app, GwSearchWindow *window)
{
   app->last_focused = window; 
}


GwSearchWindow* gw_app_get_last_focused_searchwindow (GwApplication *app)
{
   GwSearchWindow *window;

   if (app->last_focused != NULL)
     window = app->last_focused;
   else
     window = GW_SEARCHWINDOW (gw_app_get_window_by_type (app, GW_WINDOW_SEARCH));

   return window;
}


