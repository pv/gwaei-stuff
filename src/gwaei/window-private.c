#include <gwaei/gwaei.h>
#include <gwaei/windowprivate.h>

void gw_window_private_init (GwWindow *window)
{
    GwWindowPrivate *priv;

    priv = GW_WINDOW_GET_PRIVATE (window);
    priv->builder = gtk_builder_new ();
    priv->application = NULL;
}

void gw_window_private_finalize (GwWindow *window)
{
    GwWindowPrivate *priv;

    priv = GW_WINDOW_GET_PRIVATE (window);
    g_object_unref (priv->builder);
}


