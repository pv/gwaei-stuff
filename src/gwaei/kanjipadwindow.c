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
//! @file kanjipadwindow.c
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
#include <gwaei/kanjipadwindow-private.h>

#define BUFLEN 256


static void _kanjipadwindow_initialize_engine (GwKanjipadWindow*);
static gboolean _kanjipadwindow_engine_input_handler (GIOChannel*, GIOCondition, gpointer);

G_DEFINE_TYPE (GwKanjipadWindow, gw_kanjipadwindow, GW_TYPE_WINDOW)

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* gw_kanjipadwindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwKanjipadWindow *window;

    //Initializations
    window = GW_KANJIPADWINDOW (g_object_new (GW_TYPE_KANJIPADWINDOW,
                                            "type",        GTK_WINDOW_TOPLEVEL,
                                            "application", GW_APPLICATION (application),
                                            "ui-xml",      "kanjipadwindow.ui",
                                            NULL));

    return GTK_WINDOW (window);
}


void gw_kanjipadwindow_init (GwKanjipadWindow *window)
{
    window->priv = GW_KANJIPADWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwKanjipadWindowPrivate));
}


void gw_kanjipadwindow_finalize (GObject *object)
{
    GwKanjipadWindow *window;
    GwKanjipadWindowPrivate *priv;
    GSource *source;
    GError *error;

    window = GW_KANJIPADWINDOW (object);
    priv = window->priv;
    error = NULL;

    if (g_main_current_source () != NULL &&
        !g_source_is_destroyed (g_main_current_source ()) &&
        priv->iowatchid > 0
       )
    {
      source = g_main_context_find_source_by_id (NULL, priv->iowatchid);
      if (source != NULL)
      {
        g_source_destroy (source);
      }
    }
    priv->iowatchid = 0;

    if (error == NULL) 
    {
      g_io_channel_shutdown (priv->from_engine, FALSE, &error);
      g_io_channel_unref (priv->from_engine);
      priv->from_engine = NULL;
    }

    if (error == NULL)
    {
      g_io_channel_shutdown (priv->to_engine, FALSE, &error);
      g_io_channel_unref (priv->to_engine);
      priv->to_engine = NULL;
    }

    g_spawn_close_pid (priv->engine_pid);

    if (error != NULL)
    {
      fprintf(stderr, "Errored: %s\n", error->message);
      exit(EXIT_FAILURE);
    }

    G_OBJECT_CLASS (gw_kanjipadwindow_parent_class)->finalize (object);
}


static void gw_kanjipadwindow_constructed (GObject *object)
{
    GwKanjipadWindow *window;
    GwKanjipadWindowPrivate *priv;
    GtkAccelGroup *accelgroup;
    GtkWidget *widget;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_kanjipadwindow_parent_class)->constructed (object);
    }

    window = GW_KANJIPADWINDOW (object);
    priv = window->priv;
    accelgroup = gw_window_get_accel_group (GW_WINDOW (window));

    gtk_window_set_title (GTK_WINDOW (window), gettext("gWaei Kanjipad"));
    gtk_window_set_resizable (GTK_WINDOW (window), FALSE);
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_UTILITY);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (window), TRUE);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");

    priv->drawingarea = GTK_DRAWING_AREA (gw_window_get_object (GW_WINDOW (window), "kdrawing_area"));
    priv->candidates = GTK_DRAWING_AREA (gw_window_get_object (GW_WINDOW (window), "kguesses"));

    gw_kanjipadwindow_initialize_drawingarea (window);
    gw_kanjipadwindow_initialize_candidates (window);
    _kanjipadwindow_initialize_engine (window);

    widget = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "close_button"));
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_W), GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", 
      accelgroup, (GDK_KEY_Escape), 0, GTK_ACCEL_VISIBLE);

}


static void
gw_kanjipadwindow_class_init (GwKanjipadWindowClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gw_kanjipadwindow_constructed;
  object_class->finalize = gw_kanjipadwindow_finalize;

  g_type_class_add_private (object_class, sizeof (GwKanjipadWindowPrivate));
}


//!
//! @brief Open the connection to the engine
//!
static void _kanjipadwindow_initialize_engine (GwKanjipadWindow *window)
{
    //Declarations
    GwApplication *application;
    GwKanjipadWindowPrivate *priv;
    char *dir;
    char *path;
    char *argv[2];
    GError *error;
    int stdin_fd;
    int stdout_fd;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    priv = window->priv;
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
           (gpointer)&priv->engine_pid,   /* child pid */
           &stdin_fd, &stdout_fd, NULL,
           &error))
    {
      gw_application_handle_error (application, NULL, FALSE, &error);
      exit (EXIT_FAILURE);
    }

    if (!(priv->to_engine = g_io_channel_unix_new (stdin_fd)))
      g_error ("Couldn't create pipe to child process: %s", g_strerror(errno));
    if (!(priv->from_engine = g_io_channel_unix_new (stdout_fd)))
      g_error ("Couldn't create pipe from child process: %s", g_strerror(errno));

    priv->iowatchid = g_io_add_watch (priv->from_engine, G_IO_IN, _kanjipadwindow_engine_input_handler, window);

    //Cleanup
    g_free(path);
    g_free(dir);
}


//!
//! @brief To be written
//!
static gboolean _kanjipadwindow_engine_input_handler (GIOChannel *source, GIOCondition condition, gpointer data)
{
    GwKanjipadWindow *window;
    GwKanjipadWindowPrivate *priv;
    static gchar *p;
    static gchar *line;
    GError *error;
    GIOStatus status;
    int i;

    window = GW_KANJIPADWINDOW (data);
    priv = window->priv;
    error = NULL;
    status = g_io_channel_read_line (priv->from_engine, &line, NULL, NULL, &error);

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
        priv->kanji_candidates[i][0] = t1;
        priv->kanji_candidates[i][1] = t2;
        while (*p && !isspace(*p)) p++;
      }
      priv->total_candidates = i + 1;

      gw_kanjipadwindow_draw_candidates (window);
    }

    g_free (line);

    return TRUE;
}

