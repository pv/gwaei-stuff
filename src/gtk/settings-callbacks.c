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
//! @file src/gtk-settings-callbacks.c
//!
//! @brief Abstraction layer for gtk callbacks
//!
//! Callbacks for activities initiated by the user. Most of the gtk code here
//! should still be abstracted to the interface C file when possible.
//!


#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>


//!
//! @brief GUI install thread for a dictionary
//!
//! @param dictionary A gpointer to a dictionary
//!
static void *install_thread (gpointer data)
{
/*
    //Preparations
    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);
    GError *error = NULL;

    GwUiDictInstallLine *il = (GwUiDictInstallLine*) data;
    GwDictInfo *di = (GwDictInfo*) il->di;

    di->source_uri = (char*) gtk_entry_get_text (GTK_ENTRY (il->source_uri_entry));

    //Weird hacksh code for the radicals dictionary
    char radicals_source[100];
    radicals_source[0] = '\0';
    if (di->id == GW_DICT_ID_RADICALS)
    {
      gw_pref_get_string_by_schema (radicals_source, GW_SCHEMA_DICTIONARY, GW_KEY_RADICALS_SOURCE, 100);
      di->source_uri = radicals_source;
    }

    if (di->status != GW_DICT_STATUS_NOT_INSTALLED) return FALSE;

    //Preparatation complete, it's showtime
    gdk_threads_enter ();
    di->status = GW_DICT_STATUS_INSTALLING;
    gw_ui_dict_install_set_action_button (il, GTK_STOCK_CANCEL, TRUE);
    gw_ui_update_settings_interface();
    di->status = GW_DICT_STATUS_NOT_INSTALLED;
    gdk_threads_leave ();


    //Offload the install work to the io function
    gw_io_install_dictinfo (di, &gw_ui_update_progressbar, data, FALSE, &error);


    //Finish up
    if (di->status == GW_DICT_STATUS_CANCELING)
    {
      gdk_threads_enter();
        di->status = GW_DICT_STATUS_NOT_INSTALLED;
      gdk_threads_leave();
      g_error_free(error);
      error = NULL;
      do_dictionary_remove (il->action_button, il);
    }
    //Errored
    else if (error != NULL)
    {
      do_dictionary_remove (il->action_button, il);
      gdk_threads_enter();
        di->status = GW_DICT_STATUS_NOT_INSTALLED;
        gw_ui_dict_install_set_message (il, GTK_STOCK_DIALOG_ERROR, error->message);
        gw_ui_dict_install_set_action_button (il, GTK_STOCK_ADD, TRUE);
      gdk_threads_leave();
      g_error_free (error);
      error = NULL;
    }
    //Install was successful
    else
    {
      gdk_threads_enter();
        di->status = GW_DICT_STATUS_INSTALLED;
        di->total_lines =  gw_io_get_total_lines_for_path (di->path);
      gdk_threads_leave();
      //If statement to reduce flicker, between the Kanji/Radical dictionary install
      if (di->id != GW_DICT_ID_KANJI)
      {
        gdk_threads_enter();
          gw_ui_dict_install_set_action_button (il, GTK_STOCK_DELETE, TRUE);
          gw_ui_dict_install_set_message (il, GTK_STOCK_APPLY, gettext("Installed"));
        gdk_threads_leave();
      }

      //Special case where the Radicals dictionary is installed right after the Kanji one.
      if (di->id == GW_DICT_ID_KANJI)
      {
        gdk_threads_enter();
          gw_ui_dict_install_set_message (il, NULL, gettext("Installing..."));
        gdk_threads_leave();
        GwDictInfo *radicals_dict = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_RADICALS);
        GwDictInfo *kanji_dict = gw_dictlist_get_dictinfo_by_id (GW_DICT_ID_KANJI);
        il->di = radicals_dict;
        install_thread (data);
        il->di = kanji_dict;
      }
    }

    gw_ui_update_settings_interface();
    rebuild_combobox_dictionary_list();
*/
}


//!
//! @brief Callback to toggle the hiragana-katakana conversion setting for the search entry
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_hira_kata_conv_toggled_cb (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_HIRA_KATA);
    gw_pref_set_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_HIRA_KATA, !state);
}


//!
//! @brief Callback to toggle the katakana-hiragana conversion setting for the search entry
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_kata_hira_conv_toggled_cb (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_KATA_HIRA);
    gw_pref_set_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_KATA_HIRA, !state);
}


//!
//! @brief Callback to toggle spellcheck in the search entry
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_spellcheck_toggled_cb (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_SPELLCHECK);
    gw_pref_set_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_SPELLCHECK, !state);
}


//!
//! @brief Callback to toggle romaji-kana conversion
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_romaji_kana_conv_changed_cb (GtkWidget *widget, gpointer data)
{
    int active;
    active = gtk_combo_box_get_active(GTK_COMBO_BOX (widget));
    gw_pref_set_int_by_schema (GW_SCHEMA_BASE, GW_KEY_ROMAN_KANA, active);
}


//!
//! @brief Callback to set the user selected color to the color swatch for text highlighting
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_swatch_color_set_cb (GtkWidget *widget, gpointer data)
{
    //Initializations
    GdkColor color;
    gtk_color_button_get_color(GTK_COLOR_BUTTON (widget), &color);
    char *hex_color_string = NULL;
    hex_color_string = gdk_color_to_string (&color);
    char *pref_key = NULL;
    pref_key = g_strdup_printf ("%s", gtk_buildable_get_name (GTK_BUILDABLE (widget)));
    char *letter = strchr(pref_key, '_');
    if (letter == NULL) return;
    *letter = '-';

    //Set the color inthe prefs
    if (pref_key != NULL && hex_color_string != NULL)
    {
      gw_pref_set_string_by_schema (GW_SCHEMA_HIGHLIGHT, pref_key, hex_color_string);
    }

    //Cleanup
    if (pref_key != NULL)
    {
      g_free (pref_key);
      pref_key = NULL;
    }
    if (hex_color_string != NULL)
    {
      g_free (hex_color_string);
      hex_color_string = NULL;
    }
}


//!
//! @brief Callback to reset all the colors for all the swatches to the default in the preferences
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_swatch_color_reset_cb (GtkWidget *widget, gpointer data)
{
    //Initializations
    char fallback[100];
    int i = 0;
    char *pref_key[] = {
      GW_KEY_MATCH_FG,
      GW_KEY_MATCH_BG,
      GW_KEY_HEADER_FG,
      GW_KEY_HEADER_BG,
      GW_KEY_COMMENT_FG,
      NULL
    };

    //Start setting the default values
    for (i = 0; pref_key[i] != NULL; i++)
    {
      gw_pref_reset_value_by_schema (GW_SCHEMA_HIGHLIGHT, pref_key[i]);
    }

    //gw_ui_buffer_reload_tagtable_tags();
}


//!
//! @brief Sets the preference key for the global font usage
//! 
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_use_global_document_font_toggled_cb (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gw_pref_get_boolean_by_schema (GW_SCHEMA_FONT, GW_KEY_FONT_USE_GLOBAL_FONT);
    gw_pref_set_boolean_by_schema (GW_SCHEMA_FONT, GW_KEY_FONT_USE_GLOBAL_FONT, !state);
}


//!
//! @brief Sets the preference key for the new custom document font
//! 
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_custom_document_font_set_cb (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *button = GTK_WIDGET (gtk_builder_get_object (builder, "custom_font_fontbutton"));
    const char *font_description_string = gtk_font_button_get_font_name (GTK_FONT_BUTTON (button));
    gw_pref_set_string_by_schema (GW_SCHEMA_FONT, GW_KEY_FONT_CUSTOM_FONT, font_description_string);
}


//!
//! @brief Removes a dictionary
//! 
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_settings_remove_dictionary_cb (GtkWidget *widget, gpointer data)
{
   GList *iter;

   //Clear the search history it is broken since the dictionaries may be now missing
   gw_historylist_free ();
   gw_historylist_initialize ();
   gw_ui_update_history_popups ();
   gw_tabs_set_searchitem (NULL);
   for (iter = gw_tabs_get_searchitem_list(); iter != NULL; iter = iter->next)
   {
     gw_searchitem_free (iter->data);
     iter->data = NULL;
   }

}


