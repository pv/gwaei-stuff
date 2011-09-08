/******************************************************************************
    AUTHOR:
    KanjiPad - Japanese handwriting recognition front end
    Copyright (C) 1997 Owen Taylor
    File heavily modified and updated by Zachary Dovel.

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
//! @file kanjipad-window.c
//!
//! @brief To be written
//!


#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>

#define BUFLEN 256


static void _kanjipadwindow_initialize_engine (GwKanjipadWindow*);
static gboolean _kanjipadwindow_engine_input_handler (GIOChannel*, GIOCondition, gpointer);


//!
//! @brief Sets up kanjipad, aquiring any needed resources
//!
GwKanjipadWindow* gw_kanjipadwindow_new (GwSearchWindow* transient_for, GList *link)
{
    GwKanjipadWindow *temp;

    temp = (GwKanjipadWindow*) malloc(sizeof(GwKanjipadWindow));

    if (temp != NULL)
    {
      gw_window_init (GW_WINDOW (temp), GW_WINDOW_KANJIPAD, "kanjipad.ui", "kanjipad_window", link);
      gw_kanjipadwindow_init (temp, transient_for);
    }

    return temp;
}


//!
//! @brief Frees any resources taken by the initialization of kanjipad
//!
void gw_kanjipadwindow_destroy (GwKanjipadWindow *window)
{
    gw_kanjipadwindow_deinit (window);
    gw_window_deinit (GW_WINDOW (window));

    free (window);
}


void gw_kanjipadwindow_init (GwKanjipadWindow *window, GwSearchWindow* transient_for)
{
      window->drawingarea = GTK_DRAWING_AREA (gtk_builder_get_object (window->builder, "kdrawing_area"));
      window->candidates = GTK_DRAWING_AREA (gtk_builder_get_object (window->builder, "kguesses"));

      window->total_candidates = 0;
      window->strokes = NULL;
      window->curstroke = NULL;
      window->surface = NULL;
      window->ksurface = NULL;
      window->from_engine = NULL;
      window->to_engine = NULL;
      window->kselected[0] = 0;
      window->kselected[1] = 0;
      window->instroke = FALSE;

      gw_kanjipadwindow_initialize_drawingarea (window);
      gw_kanjipadwindow_initialize_candidates (window);

      _kanjipadwindow_initialize_engine (window);

      gw_window_set_transient_for (GW_WINDOW (window), GW_WINDOW (transient_for));
}


void gw_kanjipadwindow_deinit (GwKanjipadWindow *window)
{
    gtk_widget_hide (GTK_WIDGET (window->toplevel));

    //Declarations
    GSource *source;
    GError *error;

    //Initializations
    error = NULL;

    if (g_main_current_source () != NULL &&
        !g_source_is_destroyed (g_main_current_source ()) &&
        window->iowatchid > 0
       )
    {
      source = g_main_context_find_source_by_id (NULL, window->iowatchid);
      if (source != NULL)
      {
        g_source_destroy (source);
      }
    }
    window->iowatchid = 0;

    if (error == NULL) 
    {
      g_io_channel_shutdown (window->from_engine, FALSE, &error);
      g_io_channel_unref (window->from_engine);
      window->from_engine = NULL;
    }

    if (error == NULL)
    {
      g_io_channel_shutdown (window->to_engine, FALSE, &error);
      g_io_channel_unref (window->to_engine);
      window->to_engine = NULL;
    }

    g_spawn_close_pid (window->engine_pid);

    if (error != NULL)
    {
      fprintf(stderr, "Errored: %s\n", error->message);
      exit(EXIT_FAILURE);
    }
}


//!
//! @brief Open the connection to the engine
//!
static void _kanjipadwindow_initialize_engine (GwKanjipadWindow *window)
{
    //Declarations
    char *dir;
    char *path;
    char *argv[2];
    GError *error;
    int stdin_fd;
    int stdout_fd;

    //Initializations
    error = NULL;
    dir = g_get_current_dir ();
#ifdef G_OS_WIN32
    path = g_build_filename (dir, "..", "lib", PACKAGE, "kpengine.exe", NULL);
#else
    path = g_build_filename (LIBDIR, PACKAGE, "kpengine", NULL);
#endif
    argv[0] = path;
    argv[1] = NULL;

    if (!g_file_test(argv[0], G_FILE_TEST_EXISTS)) 
    {
      fprintf(stderr, "Error: Can't find kpengine at %s\n", argv[0]);
      exit (EXIT_FAILURE);
    }

    if (!g_spawn_async_with_pipes (NULL, /* working directory */
           argv, NULL,  /* argv, envp */
           0,
           NULL, NULL,  /* child_setup */
           (gpointer)&window->engine_pid,   /* child pid */
           &stdin_fd, &stdout_fd, NULL,
           &error))
    {
      gw_app_handle_error (app, NULL, FALSE, &error);
      exit (EXIT_FAILURE);
    }

    if (!(window->to_engine = g_io_channel_unix_new (stdin_fd)))
      g_error ("Couldn't create pipe to child process: %s", g_strerror(errno));
    if (!(window->from_engine = g_io_channel_unix_new (stdout_fd)))
      g_error ("Couldn't create pipe from child process: %s", g_strerror(errno));

    window->iowatchid = g_io_add_watch (window->from_engine, G_IO_IN, _kanjipadwindow_engine_input_handler, window);

    //Cleanup
    g_free(path);
    g_free(dir);
}


//!
//! @brief To be written
//!
static gboolean _kanjipadwindow_engine_input_handler (GIOChannel *source, GIOCondition condition, gpointer data)
{
    static gchar *p;
    static gchar *line;
    GError *error;
    GIOStatus status;
    int i;
    GwKanjipadWindow *window;

    window = GW_KANJIPADWINDOW (data);
    error = NULL;
    status = g_io_channel_read_line (window->from_engine, &line, NULL, NULL, &error);

    switch (status)
    {
      case G_IO_STATUS_ERROR:
        fprintf (stderr, "Error reading from engine: %s\n", error->message);
        exit(EXIT_FAILURE);
        break;
      case G_IO_STATUS_NORMAL:
        break;
      case G_IO_STATUS_EOF:
        fprintf (stderr, "Engine no longer exists");
        exit (EXIT_FAILURE);
        break;
      case G_IO_STATUS_AGAIN:
        g_assert_not_reached ();
        break;
    }

    if (line[0] == 'K')
    {
      unsigned int t1, t2;
      p = line + 1;
      for (i = 0; i < GW_KANJIPADWINDOW_MAX_GUESSES; i++)
      {
        while (*p && isspace(*p)) p++;
        if (!*p || sscanf(p, "%2x%2x", &t1, &t2) != 2)
        {
            i--;
            break;
        }
        window->kanji_candidates[i][0] = t1;
        window->kanji_candidates[i][1] = t2;
        while (*p && !isspace(*p)) p++;
      }
      window->total_candidates = i + 1;

      gw_kanjipadwindow_draw_candidates (window);
    }

    g_free (line);

    return TRUE;
}

