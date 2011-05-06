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
//! @file src/gtk-main-interface-sexy.c
//!
//! @brief Replaces the search GtkEntry with a SexySpellEntry.
//!
//! File used for implimenting a libsexy entry as the search query.  Libsexy
//! is nice because it allows spell checking compatibility.  Because some day
//! I may want to remove the dependeny, this code mingles with the other files
//! minimally.  One the gw_libsexy_initialize () command should be used if you want
//! to use the libsexy search entry.
//!


#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>
#include <libsexy/sexy.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>


//!
//! @brief Turns on or aff spellcheck given specific conditions
//!
//! The function has to make sure that the user wants spellcheck, and if 
//! the query is convertable to hiragana if the conversion pref is turned on.
//!
//! @param widget Unused GtkWidget
//! @param data Unused gpointer
//!
void do_conditionally_enable_spellcheck (GtkWidget *widget, gpointer data)
{
   GtkBuilder *builder = gw_common_get_builder ();

   GtkWidget *search_entry = GTK_WIDGET (gw_common_get_widget_by_target (GW_TARGET_ENTRY));

   gboolean spellcheck_pref;
   spellcheck_pref = gw_pref_get_boolean (GW_SCHEMA_BASE, GW_KEY_SPELLCHECK);
   int rk_conv_pref;
   rk_conv_pref = gw_pref_get_int (GW_SCHEMA_BASE, GW_KEY_ROMAN_KANA);
   gboolean want_conv;
   want_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !gw_util_is_japanese_locale()));

   char *text = gtk_editable_get_chars (GTK_EDITABLE (search_entry), 0, -1);
   if (text == NULL) return;

   char kana[MAX_QUERY];
   gboolean is_convertable_to_hiragana;
   is_convertable_to_hiragana = (want_conv && gw_util_str_roma_to_hira (text, kana, MAX_QUERY));

   if (gw_util_all_chars_are_in_range (text, L' ', L'|') == TRUE &&
       is_convertable_to_hiragana == FALSE                     &&
       spellcheck_pref == TRUE                                   )
   {
     //Default to the english dictionary when using the english-japanese dict
     GError *err;
     err = NULL;
     GList *list = gw_dictinfolist_get_selected();
     GwDictInfo *di = (GwDictInfo*) list->data;

     if ((strcmp ("Radicals", di->filename) == 0) ||
         (strcmp ("Names",    di->filename) == 0)   )
     {
       //No spell checking needed
       sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (search_entry), FALSE);
     }
     else if ((strcmp ("English",  di->filename) == 0) ||
              (strcmp ("Kanji",    di->filename) == 0) ||
              (strcmp ("Examples", di->filename) == 0) ||
              (strcmp ("Places",   di->filename) == 0)   )
     {
       //Set the spellchecked language to english
       if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (search_entry), "en") == FALSE)
         sexy_spell_entry_activate_language (SEXY_SPELL_ENTRY (search_entry), "en", &err);
       if (err != NULL)
       {
         g_error_free (err);
         err = NULL;
       }
       sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (search_entry), TRUE);
 
       if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (search_entry), "fr") == TRUE)
         sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (search_entry), "fr");
       if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (search_entry), "de") == TRUE)
         sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (search_entry), "de");
       if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (search_entry), "es") == TRUE)
         sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (search_entry), "es");
       if (sexy_spell_entry_language_is_active(SEXY_SPELL_ENTRY (search_entry), "it") == TRUE)
         sexy_spell_entry_deactivate_language (SEXY_SPELL_ENTRY (search_entry), "it");
     }
     //Default to the system default otherwise
     else
     {
       //Set the system default language
       sexy_spell_entry_activate_default_languages (SEXY_SPELL_ENTRY (search_entry));
       sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (search_entry), TRUE);
     }
   }
   else
   {
     sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (search_entry), FALSE);
   }
   g_free (text);
}


//!
//! @brief Conitionally enables search when focus enters the searh entry
//!
//! @param widget Unused GtkWidget
//! @param data Unused gpointer
//! @see do_enable_spellcheck_when_focus_out ()
//! @return always returns false
//!
gboolean do_enable_spellcheck_when_focus_in (GtkWidget *widget, gpointer data)
{
    GtkWidget *search_entry = GTK_WIDGET (gw_common_get_widget_by_target (GW_TARGET_ENTRY));
    do_conditionally_enable_spellcheck (NULL, NULL);
    return FALSE;
}


//!
//! @brief Disables spell check wehn the focus isn't in the search entry
//!
//! The though behind is is the red underline is distracting when the results
//! are being searched through by the user.
//!
//! @param widget Unused GtkWidget
//! @param data Unused gpointer
//! @see do_enable_spellcheck_when_focus_in ()
//! @return always returns false
//!
gboolean do_disable_spellcheck_when_focus_out (GtkWidget *widget, gpointer data)
{
    GtkWidget *search_entry = GTK_WIDGET (gw_common_get_widget_by_target (GW_TARGET_ENTRY));
    sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (search_entry), FALSE);
    return FALSE;
}


//!
//! @brief Callback action for when preference key changes
//!
//! @param client The preference client
//! @param cnxn_id Unknown
//! @param entry The preference entry object
//! @param data Usere data passed to the function
//!
void do_spellcheck_pref_changed_action (GSettings *settings,
                                        gchar *key,
                                        gpointer data       )
{
    g_signal_handlers_block_by_func (settings, do_spellcheck_pref_changed_action, NULL);
    gboolean value = g_settings_get_boolean (settings, key);
    gw_libsexy_ui_set_spellcheck (value);
    g_signal_handlers_unblock_by_func (settings, do_spellcheck_pref_changed_action, NULL);
}


//!
//! @brief Sets the gui widgets consistently to the requested state
//!
//! The function makes sure that both of the widgets in the gui are the same
//! when the user clicks a one of them to change the settings.
//!
//! @param request the requested state for spellchecking widgets
//!
void gw_libsexy_ui_set_spellcheck (gboolean request)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *pref_checkbox, *toolbar_button;
    pref_checkbox = GTK_WIDGET (gtk_builder_get_object(builder, "query_spellcheck"));
    toolbar_button = GTK_WIDGET (gtk_builder_get_object(builder, "spellcheck_toolbutton"));
    g_signal_handlers_block_by_func (pref_checkbox, do_spellcheck_toggle, NULL);
    g_signal_handlers_block_by_func (toolbar_button, do_spellcheck_toggle, NULL);

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (pref_checkbox), request);
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON (toolbar_button), request);
    do_conditionally_enable_spellcheck (NULL, NULL);

    g_signal_handlers_unblock_by_func (pref_checkbox, do_spellcheck_toggle, NULL);
    g_signal_handlers_unblock_by_func (toolbar_button, do_spellcheck_toggle, NULL);
}


//!
//! @brief Sets up the sexy text entry
//!
//! The function takes the time to get rid of the gtkentry and replace it with
//! a SexySpellEntry.  In the process, it makes sure all of the signals are
//! connected to the new widget. It also make hidden spellcheck toolbutton
//! and checkbox visible.
//!
void gw_libsexy_initialize ()
{
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *container = GTK_WIDGET (gtk_builder_get_object (builder, "search_entry_container"));
    GtkWidget *entry = gtk_bin_get_child (GTK_BIN (container));

    //Make the hidden spellcheck toolbutton appear
    GtkWidget *spellcheck_toolbutton;
    spellcheck_toolbutton = GTK_WIDGET (gtk_builder_get_object (builder, "spellcheck_toolbutton"));
    gtk_widget_show (spellcheck_toolbutton);

    //Swap the original entry for the libsexy one
    gchar *tooltip_text = gtk_widget_get_tooltip_text (entry);
    gtk_widget_destroy (entry);
    entry = NULL;

    entry = GTK_WIDGET (sexy_spell_entry_new ());

    gtk_container_add (GTK_CONTAINER (container), entry);
    gtk_buildable_set_name (GTK_BUILDABLE (entry), "search_entry");

    if (tooltip_text != NULL)
    {
      gtk_widget_set_tooltip_text (entry, tooltip_text);
      g_free (tooltip_text);
    }

    gtk_entry_set_max_length (GTK_ENTRY (entry), 50);
    gtk_widget_show (entry);

    GtkWidget* results_tv = gw_common_get_widget_by_target (GW_TARGET_RESULTS);
    GtkWidget* kanji_tv = gw_common_get_widget_by_target (GW_TARGET_KANJI);

    //Mimic original callbacks from the original search entry
    g_signal_connect (G_OBJECT (entry), "activate", G_CALLBACK (do_search), NULL);
    g_signal_connect (G_OBJECT (entry), "focus-in-event", G_CALLBACK (do_update_clipboard_on_focus_change), entry);
    g_signal_connect (G_OBJECT (entry), "key-press-event", G_CALLBACK (do_focus_change_on_key_press), NULL);
    g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (do_update_button_states_based_on_entry_text), NULL);
    g_signal_connect (G_OBJECT (entry), "icon-release", G_CALLBACK (do_clear_search), NULL);

    //New callbacks specifically for libsexy
    g_signal_connect (G_OBJECT (entry), "changed", G_CALLBACK (do_conditionally_enable_spellcheck), NULL);

    //Show the spellcheck checkbox in the preferences
    GtkWidget *spellcheck_checkbox;
    spellcheck_checkbox = GTK_WIDGET (gtk_builder_get_object (builder, "query_spellcheck"));
    gtk_widget_show (spellcheck_checkbox);


    gw_pref_add_change_listener (GW_SCHEMA_BASE, GW_KEY_SPELLCHECK, do_spellcheck_pref_changed_action, NULL);
}

void gw_libsexy_free ()
{

}
