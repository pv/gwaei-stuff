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
//! @file vocabularywindow-callbacks.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/vocabularywindow-private.h>


G_MODULE_EXPORT void
gw_vocabularywindow_cell_edited_cb (GtkCellRendererText *renderer,
                                    gchar               *path_string,
                                    gchar               *new_text,
                                    gpointer             data       )
{
}


G_MODULE_EXPORT void
gw_vocabularywindow_list_add_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    if (window == NULL) return FALSE;
    priv = window->priv;
}

G_MODULE_EXPORT void
gw_vocabularywindow_list_remove_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    if (window == NULL) return FALSE;
    priv = window->priv;
}


G_MODULE_EXPORT void
gw_vocabularywindow_item_add_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    if (window == NULL) return FALSE;
    priv = window->priv;
}


G_MODULE_EXPORT void
gw_vocabularywindow_item_remove_cb (GtkWidget *widget, gpointer data)
{
    GwVocabularyWindow *window;
    GwVocabularyWindowPrivate *priv;

    window = GW_SEARCHWINDOW (gtk_widget_get_ancestor (GTK_WIDGET (data), GW_TYPE_SEARCHWINDOW));
    if (window == NULL) return FALSE;
    priv = window->priv;
}

