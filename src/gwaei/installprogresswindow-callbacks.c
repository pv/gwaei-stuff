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
//! @file installprogress-callbacks.c
//!
//! @brief To be written
//!

#include <string.h>
#include <stdlib.h>

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


G_MODULE_EXPORT int gw_installprogresswindow_update_dictinst_cb (double fraction, gpointer data)
{
    //Declarations
    GwInstallProgressWindow *window;

    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (data);

    g_mutex_lock (window->mutex); 
    window->install_fraction = lw_dictinst_get_total_progress (window->di, fraction);
    g_mutex_unlock (window->mutex);

    return 0;
}


//!
//! @brief Callback to update the install dialog progress.  The data passed to it should be
//!        in the form of a LwDictInst.  If it is NULL, the progress window will be closed.
//!
G_MODULE_EXPORT gboolean gw_installprogresswindow_update_ui_timeout (gpointer data)
{
    //Sanity check
    g_assert (data != NULL);

    //Declarations
    GwInstallProgressWindow *window;
    GwSettingsWindow *settingswindow;
    LwDictInst *di;
    GList *iter;
    int current_to_install;
    int total_to_install;
    char *text_installing;
    char *text_installing_markup;
    char *text_left;
    char *text_left_markup;
    char *text_progressbar;

    //Initializations
    window = GW_INSTALLPROGRESSWINDOW (data);
    settingswindow = GW_SETTINGSWINDOW (window->transient_for);
    current_to_install = 0;
    total_to_install = 0;

    //The install is complete close the window
    if (window->di == NULL)
    {
      gw_app_destroy_window (app, GW_WINDOW (window));
      return FALSE;
    }

    g_mutex_lock (window->mutex);

    //Calculate the number of dictionaries left to install
    for (iter = settingswindow->dictinstlist->list; iter != NULL; iter = iter->next)
    {
      di = LW_DICTINST (iter->data);
      if (di != NULL && di->selected)
      {
        current_to_install++;
      }
      if (iter->data == window->di) break;
    }

    //Calculate the number of dictionaries left to install
    for (iter = settingswindow->dictinstlist->list; iter != NULL; iter = iter->next)
    {
      di = LW_DICTINST (iter->data);
      if (di->selected)
      {
        total_to_install++;
      }
    }
    
    di = window->di;

    text_progressbar =  g_markup_printf_escaped (gettext("Installing %s..."), di->filename);
    text_left = g_strdup_printf (gettext("Installing dictionary %d of %d..."), current_to_install, total_to_install);
    text_left_markup = g_markup_printf_escaped ("<big><b>%s</b></big>", text_left);
    text_installing = lw_dictinst_get_status_string (di, TRUE);
    text_installing_markup = g_markup_printf_escaped ("<small>%s</small>", text_installing);

    gtk_label_set_markup (window->label, text_left_markup);
    gtk_label_set_markup (window->sublabel, text_installing_markup);
    gtk_progress_bar_set_fraction (window->progressbar, window->install_fraction);
    gtk_progress_bar_set_text (window->progressbar, text_progressbar);

    g_mutex_unlock (window->mutex);

    //Cleanup
    g_free (text_progressbar);
    g_free (text_left);
    g_free (text_left_markup);
    g_free (text_installing);
    g_free (text_installing_markup);

    return TRUE;
}


