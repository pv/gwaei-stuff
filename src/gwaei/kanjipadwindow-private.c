#include <stdlib.h>

#include <gwaei/gwaei.h>
#include <gwaei/kanjipad-private.h>

void gw_kanjipadwindow_private_init (GwKanjipadWindow *window)
{
    GwKanjipadWindowPrivate *priv;

    priv = window->priv;

    priv->drawingarea = NULL;
    priv->candidates = NULL;
    priv->total_candidates = 0;
    priv->strokes = NULL;
    priv->curstroke = NULL;
    priv->surface = NULL;
    priv->ksurface = NULL;
    priv->from_engine = NULL;
    priv->to_engine = NULL;
    priv->kselected[0] = 0;
    priv->kselected[1] = 0;
    priv->instroke = FALSE;
}


void gw_kanjipadwindow_private_finalize (GwKanjipadWindow *window)
{
    //Declarations
    GwKanjipadWindowPrivate *priv;
    GSource *source;
    GError *error;

    //Initializations
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
}
