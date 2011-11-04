#include <gwaei/gwaei.h>
#include <gwaei/dictionaryinstall-private.h>

void gw_dictionaryinstallwindow_private_init (GwDictionaryInstallWindow *window)
{
}


void gw_dictionaryinstallwindow_private_finalize (GwDictionaryInstallWindow *window)
{
    GwDictionaryInstallWindowPrivate *priv;

    priv = window->priv;

    g_object_unref (priv->encoding_store);
    g_object_unref (priv->compression_store);
    g_object_unref (priv->engine_store);
    g_object_unref (priv->dictionary_store);
}
