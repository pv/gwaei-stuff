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
//! @file gwaei.c
//!
//! @brief To be written
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
#include <gwaei/application-private.h>


G_DEFINE_TYPE (GwApplication, gw_application, GW_TYPE_APPLICATION);

//!
//! @brief creates a new instance of the gwaei applicaiton
//!
GtkApplication* gw_application_new (int* argc, char** argv[])
{
    //Declarations
    GwApplication *application;

    //Initializations
    application = GW_APPLICATION (g_object_new (GW_TYPE_APPLICATION, NULL));

    if (application != NULL)
    {
      application->priv = GW_APPLICATION_GET_PRIVATE (application);
    }

    return GTK_APPLICATION (application);
}


static void gw_application_init (GwApplication *app)
{
    gw_application_private_init (app);
}


static void gw_application_finalize (GObject *object)
{
    //Declarations
    GwApplication *app;
    GList *iter;
    GtkWidget *widget;

    app = GW_APPLICATION (object);

    gw_application_private_finalize (app);
    G_OBJECT_CLASS (gw_application_parent_class)->finalize (object);
}


static void
gw_application_class_init (GwApplicationClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = gw_application_finalize;

  g_type_class_add_private (object_class, sizeof (GwApplicationPrivate));
}


//!
//! @brief Loads the arguments from the command line into the app instance
//!

void gw_application_parse_args (GwApplication *app, int *argc, char** argv[])
{
    GwApplicationPrivate *priv;

    priv = GW_APPLICATION_GET_PRIVATE (app);

    if (priv->arg_dictionary != NULL) g_free (priv->arg_dictionary);
    if (priv->arg_query != NULL) g_free (priv->arg_query);

    GOptionEntry entries[] =
    {
      { "dictionary", 'd', 0, G_OPTION_ARG_STRING, &(priv->arg_dictionary), gettext("Choose the dictionary to use"), "English" },
      { "version", 'v', 0, G_OPTION_ARG_NONE, &(priv->arg_version_switch), gettext("Check the gWaei version information"), NULL },
      { NULL }
    };

    //Program flags setup
    GError *error = NULL;
    priv->context = g_option_context_new (gettext("- A dictionary program for Japanese-English translation."));
    g_option_context_add_main_entries (priv->context, entries, PACKAGE);
    g_option_context_add_group (priv->context, gtk_get_option_group (TRUE));
    g_option_context_parse (priv->context, argc, argv, &error);

    if (error != NULL)
    {
      gw_application_handle_error (app, NULL, FALSE, &error);
      exit(1);
    }

    //Get the query after the flags have been parsed out
    priv->arg_query = lw_util_get_query_from_args (*argc, *argv);
}


//!
//! @brief Prints to the terminal the about message for the program.
//!
void gw_application_print_about (GwApplication *app)
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
GwApplicationResolution gw_application_run (GwApplication *app)
{
  /*
    //Declarations
    LwDictInfo *di;
    GwSearchWindow *window;

    window = GW_SEARCHWINDOW (gw_application_show_window (app, GW_WINDOW_SEARCH, NULL, FALSE));

    gw_searchwindow_update_history_popups (window);

    //Show the settings dialog if no dictionaries are installed
    if (lw_dictinfolist_get_total (LW_DICTINFOLIST (app->dictinfolist)) == 0)
    {
      gw_application_show_window (app, GW_WINDOW_SETTINGS, GW_WINDOW (window), FALSE);
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
    */
}


void gw_application_quit (GwApplication *app)
{
  /*
    gw_application_block_searches (app);

    //Declarations
    GList *iter;
    GwWindow *window;
    
    //Clear all of the windows *before* quiting the gtk main loop to avoid a gdk_thread_enter lock
    for (iter = app->windowlist; iter != NULL; iter = iter->next)
    {
      window = GW_WINDOW (iter->data);
      if (window != NULL)
      {
        gw_window_destroy (window);
      }
      app->windowlist = g_list_delete_link (app->windowlist, iter);
    }

    gtk_main_quit ();
    gw_application_unblock_searches (app);
    */
}


//!
//! @brief Returns the program name.  It should not be freed or modified
//! @returns A constanst string representing the program name
//!
const char* gw_application_get_program_name (GwApplication *app) 
{
  return gettext("gWaei Japanese-English Dictionary");
}


void gw_application_cancel_all_searches (GwApplication *app)
{
  /*
    GList *iter;
    GwWindow *window;

    for (iter = app->windowlist; iter != NULL; iter = iter->next)
    {
      window = GW_WINDOW (iter->data);
      if (window != NULL && window->type == GW_WINDOW_SEARCH)
        gw_searchwindow_cancel_all_searches (GW_SEARCHWINDOW (window));
    }
    */
}


//!
//!  @brief Will attempt to get the window of the specified type which is most at the front
//!
GwWindow* gw_application_get_window_by_type (GwApplication *app, GType TYPE)
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

/*
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
*/
    return window;
}


//!
//! @brief Gets a GwWindow from the application's windowlist
//! @param app A GwApplication instance to work on
//! @param TYPE The window type to get
//! @param widget A widget from the window so you can get a specific instance.  If NULL, you cet the first window to match the GwWindowType
//!
GwWindow* gw_application_get_window_by_widget (GwApplication *app, GtkWidget *widget)
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

/*
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
*/
    return window;
}


void gw_application_block_searches (GwApplication *app)
{
  GwApplicationPrivate *priv;

  priv = GW_APPLICATION_GET_PRIVATE (app);

  priv->block_new_searches++;
  gw_application_cancel_all_searches (app);
}


void gw_application_unblock_searches (GwApplication *app)
{
  GwApplicationPrivate *priv;

  priv = GW_APPLICATION_GET_PRIVATE (app);

  if (priv->block_new_searches > 0)
    priv->block_new_searches--;
}

gboolean gw_application_can_start_search (GwApplication *app)
{
  GwApplicationPrivate *priv;

  priv = GW_APPLICATION_GET_PRIVATE (app);

  return (priv->block_new_searches == 0);
}


void gw_application_handle_error (GwApplication *app, GtkWindow *transient_for, gboolean show_dialog, GError **error)
{
  /*
    //Sanity checks
    if (error == NULL || *error == NULL) return;

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
    */
}


/*
void gw_application_set_last_focused_searchwindow (GwApplication *app, GwSearchWindow *window)
{
   app->last_focused = window; 
}
*/


/*
GwSearchWindow* gw_application_get_last_focused_searchwindow (GwApplication *app)
{
   GwSearchWindow *window;

   if (app->last_focused != NULL)
     window = app->last_focused;
   else
     window = GW_SEARCHWINDOW (gw_application_get_window_by_type (app, GW_WINDOW_SEARCH));

   return window;
}
*/


LwPreferences* gw_application_get_preferences (GwApplication *app)
{
  GwApplicationPrivate *priv;

  priv = GW_APPLICATION_GET_PRIVATE (app);

  return priv->preferences;
}


GwDictInfoList* gw_application_get_dictinfolist (GwApplication *app)
{
  GwApplicationPrivate *priv;

  priv = GW_APPLICATION_GET_PRIVATE (app);

  return priv->dictinfolist;
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

