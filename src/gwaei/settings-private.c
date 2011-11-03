#include <gwaei/gwaei.h>
#include <gwaei/settings-private.h>

void gw_settingswindow_private_init (GwSettingsWindow *window)
{
    GwSettingsWindowPrivate *priv;

    priv = GW_SETTINGSWINDOW_GET_PRIVATE (window);

    //Initializations
    priv->dictinstlist = NULL;
}

void gw_settingswindow_private_finalize (GwSettingsWindow *window)
{
    GwSettingsWindowPrivate *priv;

    priv = GW_SETTINGSWINDOW_GET_PRIVATE (window);

    if (priv->dictinstlist != NULL)
    {
      lw_dictinstlist_free (priv->dictinstlist);
      priv->dictinstlist = NULL;
    }
}
