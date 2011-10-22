#include <gwaei/gwaei.h>
#include <gwaei/windowprivate.h>

void gw_window_private_init (GwWindowPrivate *priv)
{
    priv->builder = gtk_builder_new ();
}

void gw_window_private_finalize (GwWindowPrivate *priv)
{
    g_object_unref (priv->builder);
}


