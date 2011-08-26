#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


G_MODULE_EXPORT void gw_installprogress_cancel_cb (GtkWidget *widget, gpointer data)
{
    GwInstallProgressWindow *window;
    GwSettingsWindow *settingswindow;

    window = GW_INSTALLPROGRESSWINDOW (gw_app_get_window_by_widget (app, GTK_WIDGET (data)));
    if (window == NULL) return;
    settingswindow = GW_SETTINGSWINDOW (window->transient_for);

    lw_dictinstlist_set_cancel_operations (settingswindow->dictinstlist, TRUE);
}

