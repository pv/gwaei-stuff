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
//! @file addvocabularywindow.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <string.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>
#include <gwaei/addvocabularywindow-private.h>


G_DEFINE_TYPE (GwAddVocabularyWindow, gw_addvocabularywindow, GW_TYPE_WINDOW)

//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* 
gw_addvocabularywindow_new (GtkApplication *application)
{
    g_assert (application != NULL);

    //Declarations
    GwAddVocabularyWindow *window;

    //Initializations
    window = GW_ADDVOCABULARYWINDOW (g_object_new (GW_TYPE_ADDVOCABULARYWINDOW,
                                                   "type",        GTK_WINDOW_TOPLEVEL,
                                                   "application", GW_APPLICATION (application),
                                                   "ui-xml",      "addvocabularywindow.ui",
                                                   NULL));

    return GTK_WINDOW (window);
}


static void 
gw_addvocabularywindow_init (GwAddVocabularyWindow *window)
{
    window->priv = GW_ADDVOCABULARYWINDOW_GET_PRIVATE (window);
    memset(window->priv, 0, sizeof(GwAddVocabularyWindowPrivate));
}


static void 
gw_addvocabularywindow_finalize (GObject *object)
{
  /*
    GwAddVocabularyWindow *window;
    GwAddVocabularyWindowPrivate *priv;

    window = GW_ADDVOCABULARYWINDOW (object);
    priv = window->priv;
    */

    G_OBJECT_CLASS (gw_addvocabularywindow_parent_class)->finalize (object);
}


static void 
gw_addvocabularywindow_constructed (GObject *object)
{
    //Declarations
    GwAddVocabularyWindow *window;
    GwAddVocabularyWindowPrivate *priv;
    GwApplication *application;
    GtkListStore *store;
    GtkTreeModel *model;

    //Chain the parent class
    {
      G_OBJECT_CLASS (gw_addvocabularywindow_parent_class)->constructed (object);
    }

    //Initializations
    window = GW_ADDVOCABULARYWINDOW (object);
    priv = window->priv;
    application = gw_window_get_application (GW_WINDOW (window));
    store = gw_application_get_vocabularyliststore (application);
    model = GTK_TREE_MODEL (store);

    //Set up the gtkbuilder links
    priv->kanji_entry = GTK_ENTRY (gw_window_get_object (GW_WINDOW (window), "kanji_entry"));
    priv->furigana_entry = GTK_ENTRY (gw_window_get_object (GW_WINDOW (window), "furigana_entry"));
    priv->definitions_entry = GTK_ENTRY (gw_window_get_object (GW_WINDOW (window), "definitions_entry"));
    priv->cancel_button = GTK_BUTTON (gw_window_get_object (GW_WINDOW (window), "cancel_button"));
    priv->add_button = GTK_BUTTON (gw_window_get_object (GW_WINDOW (window), "add_button"));
    priv->vocabulary_list_combobox = GTK_COMBO_BOX (gw_window_get_object (GW_WINDOW (window), "vocabulary_list_combobox"));

    //Set up the gtk window
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER_ON_PARENT);
    gtk_window_set_default_size (GTK_WINDOW (window), 400, 100);
    gtk_window_set_icon_name (GTK_WINDOW (window), "gwaei");
    gtk_window_set_title (GTK_WINDOW (window), gettext("Add Vocabulary Word..."));
    gtk_window_set_resizable (GTK_WINDOW (window), TRUE);
    gtk_window_set_type_hint (GTK_WINDOW (window), GDK_WINDOW_TYPE_HINT_DIALOG);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (window), TRUE);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (window), FALSE);
    gtk_window_set_modal (GTK_WINDOW (window), TRUE);

    //Initialize the combobox
    gtk_combo_box_set_model (priv->vocabulary_list_combobox, model); 

    GtkEntry *entry;

    //Remove the default entry since it doesn't seem to be editable for some reason
    entry = GTK_ENTRY (gtk_bin_get_child (GTK_BIN (priv->vocabulary_list_combobox)));
    gtk_widget_destroy (GTK_WIDGET (entry));

    entry = GTK_ENTRY (gtk_entry_new ());
    gtk_widget_show (GTK_WIDGET (entry));
    gtk_combo_box_set_entry_text_column (priv->vocabulary_list_combobox, GW_VOCABULARYLISTSTORE_COLUMN_NAME);
    gtk_container_add (GTK_CONTAINER (priv->vocabulary_list_combobox), GTK_WIDGET (entry));
    gtk_combo_box_set_active (priv->vocabulary_list_combobox, 0);
}


static void
gw_addvocabularywindow_class_init (GwAddVocabularyWindowClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  object_class->constructed = gw_addvocabularywindow_constructed;
  object_class->finalize = gw_addvocabularywindow_finalize;

  g_type_class_add_private (object_class, sizeof (GwAddVocabularyWindowPrivate));
}


const gchar*
gw_addvocabularywindow_get_kanji (GwAddVocabularyWindow *window)
{
  return gtk_entry_get_text (window->priv->kanji_entry);
}

const gchar*
gw_addvocabularywindow_get_furigana (GwAddVocabularyWindow *window)
{
  return gtk_entry_get_text (window->priv->furigana_entry);
}

const gchar*
gw_addvocabularywindow_get_definitions (GwAddVocabularyWindow *window)
{
  return gtk_entry_get_text (window->priv->definitions_entry);
}

GtkListStore*
gw_addvocabularywindow_get_wordstore (GwAddVocabularyWindow *window)
{
    //Declarations
    GwApplication *application;
    GtkEntry *entry;
    const gchar *name;
    GtkListStore *wordstore;
    GtkListStore *liststore;

    //Initializations
    application = gw_window_get_application (GW_WINDOW (window));
    liststore = gw_application_get_vocabularyliststore (application);
    entry = GTK_ENTRY (gtk_bin_get_child (GTK_BIN (window->priv->vocabulary_list_combobox)));
    name = gtk_entry_get_text (entry);
    wordstore = gw_vocabularyliststore_get_wordstore_by_name (GW_VOCABULARYLISTSTORE (liststore), name);

    return wordstore;
}
