#include <gwaei/gwaei.h>
#include <gwaei/window-private.h>

void gw_window_private_init (GwWindow *window)
{
    GwWindowPrivate *priv;

    priv = window->priv;

    priv->builder = gtk_builder_new ();
    priv->application = NULL;
    priv->ui_xml = NULL;
    priv->toplevel = NULL;
}

void gw_window_private_finalize (GwWindow *window)
{
    GwWindowPrivate *priv;

    priv = window->priv;

    if (priv->builder != NULL) g_object_unref (priv->builder);
    if (priv->ui_xml != NULL) g_free (priv->ui_xml);
    priv->toplevel = NULL;
}

