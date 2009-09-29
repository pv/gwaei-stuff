/*****************************************************************************

  FILE:
  src/sexy.c

  DESCRIPTION:
  File used for implimenting a libsexy entry as the search query.  Libsexy
  is nice because it allows spell checking compatibility.  Because some day
  I may want to remove the dependeny, this code mingles with the other files
  minimally.  One the initialize_sexy() command should be used if you want
  to use the libsexy search entry.

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


#include <stdlib.h>
#include <regex.h>
#include <string.h>

#include <gtk/gtk.h>
#include <libsexy/sexy.h>

#include <gwaei/gtk.h>
#include <gwaei/sexy.h>
#include <gwaei/definitions.h>
#include <gwaei/utilities.h>
#include <gwaei/dictionaries.h>
#include <gwaei/history.h>
#include <gwaei/interface.h>
#include <gwaei/callbacks.h>
#include <gwaei/preferences.h>


//Events for enabling/disabling spellchecking if the query only contains romanji

void do_conditionally_enable_spellcheck (GtkWidget *widget, gpointer data)
{
     char id[50];

     GtkWidget *entry;
     entry = search_entry;

     gboolean spellcheck_pref;
     spellcheck_pref = gwaei_pref_get_boolean (GCKEY_GWAEI_SPELLCHECK, TRUE);

     char *text = gtk_editable_get_chars (GTK_EDITABLE (entry), 0, -1);
     if (text == NULL) return;

     char *input_ptr = text;
     char kana[MAX_QUERY];
     char *kana_ptr = kana;
     *kana_ptr = '\0';

     int leftover;
     leftover = MAX_QUERY;
     while (leftover-- > 0)
     {
       kana_ptr = gwaei_romanji_to_hiragana (input_ptr, kana_ptr);
       if (kana_ptr == NULL || input_ptr == NULL)
         break;
       input_ptr = gwaei_next_hiragana_char_from_romanji (input_ptr);
       if (kana_ptr == NULL || input_ptr == NULL)
         break;

       kana_ptr = &kana_ptr[strlen(kana_ptr)];
     }

     gboolean is_convertable_to_hiragana;
     is_convertable_to_hiragana= (input_ptr != NULL && strlen (input_ptr) == 0);

     if (gwaei_all_chars_are_in_range (text, L' ', L'|') == TRUE &&
         is_convertable_to_hiragana == FALSE                     &&
         spellcheck_pref == TRUE                                   )
     {
       GtkWidget *combobox ;
       strcpy (id, "dictionary_combobox");
       combobox = GTK_WIDGET (gtk_builder_get_object (builder, id));

       char *active;
       active = gtk_combo_box_get_active_text (GTK_COMBO_BOX (combobox));

       //Default to the english dictionary when using the english-japanese dict
       if (strcmp (active, "English") == 0 || strcmp (active, "english") == 0)
       {
          //Set the spellchecked language to english
          GError *err;
          err = NULL;
          sexy_spell_entry_activate_language (SEXY_SPELL_ENTRY (entry), "en", &err);
          if (err != NULL)
          {
            g_error_free (err);
            err = NULL;
          }

          sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (entry), TRUE);
       }

       //Default to the system default otherwise
       else
       {
          //Set the system default language
          sexy_spell_entry_activate_default_languages (SEXY_SPELL_ENTRY (entry));
          sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (entry), TRUE);
       }
     }
     else
     {
       sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (entry), FALSE);
     }
     g_free (text);
}


gboolean do_enable_spellcheck_when_focus_in (GtkWidget *widget, gpointer data)
{
      do_conditionally_enable_spellcheck (NULL, NULL);
      return FALSE;
}


gboolean do_disable_spellcheck_when_focus_out (GtkWidget *widget, gpointer data)
{
    sexy_spell_entry_set_checked (SEXY_SPELL_ENTRY (search_entry), FALSE);
    return FALSE;
}


void gwaei_sexy_initialize_libsexy ()
{
    char id[50];


    //Make the hidden spellcheck toolbutton appear
    GtkWidget *spellcheck_toolbutton;
    strcpy (id, "spellcheck_toolbutton");
    spellcheck_toolbutton = GTK_WIDGET (gtk_builder_get_object (builder, id));
    gtk_widget_show (spellcheck_toolbutton);


    //Swap the original entry for the libsexy one
    GtkWidget *entry;
    strcpy (id, "search_entry");
    entry = GTK_WIDGET (gtk_builder_get_object (builder, id));

    gchar *tooltip_text = gtk_widget_get_tooltip_text (entry);
    gtk_widget_destroy (entry);

    entry = GTK_WIDGET (sexy_spell_entry_new ());
    search_entry = entry;

    GtkWidget *entry_container;
    strcpy(id, "search_entry_container");
    entry_container = GTK_WIDGET (gtk_builder_get_object (builder, id));

    gtk_container_add (GTK_CONTAINER (entry_container), entry);
    gtk_widget_set_name (entry, "search_entry");
    if (tooltip_text != NULL)
    {
      gtk_widget_set_tooltip_text (entry, tooltip_text);
      g_free (tooltip_text);
    }
    gtk_entry_set_max_length (GTK_ENTRY (entry), 50);
    gtk_widget_show (entry);

    //Mimic original callbacks from the original search entry
    g_signal_connect( G_OBJECT (entry),       "activate",
                      G_CALLBACK (do_search), NULL);
    g_signal_connect( G_OBJECT (entry),       "drag-data-received",
                      G_CALLBACK (search_drag_data_recieved), NULL);
    g_signal_connect( G_OBJECT (entry),       "focus-in-event",
                      G_CALLBACK (do_update_clipboard_on_focus_change), entry);
    g_signal_connect( G_OBJECT (entry),       "key-press-event",
                      G_CALLBACK (do_switch_dictionaries_on_tab_press), NULL);
    g_signal_connect( G_OBJECT (entry),       "key-press-event",
                      G_CALLBACK (do_focus_change_on_key_press), NULL);
    g_signal_connect( G_OBJECT (entry),       "key-press-event",
                      G_CALLBACK (do_switch_dictionaries_quickkey_action), NULL);
    g_signal_connect( G_OBJECT (entry),       "changed",
                      G_CALLBACK (do_update_button_states_based_on_entry_text), NULL);
    g_signal_connect( G_OBJECT (entry),       "icon-release",
                      G_CALLBACK (do_clear_search), NULL);



    //New callbacks specifically for libsexy
    g_signal_connect( G_OBJECT (entry), "changed",
                      G_CALLBACK (do_conditionally_enable_spellcheck), NULL);
    g_signal_connect( G_OBJECT (entry), "focus-in-event",
                      G_CALLBACK (do_enable_spellcheck_when_focus_in), entry);
    g_signal_connect( G_OBJECT (results_tv), "focus-in-event",
                      G_CALLBACK (do_disable_spellcheck_when_focus_out), entry);
    g_signal_connect( G_OBJECT (kanji_tv), "focus-in-event",
                      G_CALLBACK (do_disable_spellcheck_when_focus_out), entry);

    //Show the spellcheck checkbox in the preferences
    GtkWidget *spellcheck_checkbox;
    strcpy (id, "query_spellcheck");
    spellcheck_checkbox = GTK_WIDGET (gtk_builder_get_object (builder, id));
    gtk_widget_show (spellcheck_checkbox);

}

void gwaei_sexy_ui_set_spellcheck(gboolean request)
{
    char id[50];

    GtkWidget *pref_checkbox, *toolbar_button;
    strcpy(id, "query_spellcheck");
    pref_checkbox = GTK_WIDGET (gtk_builder_get_object(builder, id));
    strcpy(id, "spellcheck_toolbutton");
    toolbar_button = GTK_WIDGET (gtk_builder_get_object(builder, id));

    g_signal_handlers_block_by_func(pref_checkbox, do_spellcheck_toggle, NULL); 
    g_signal_handlers_block_by_func(toolbar_button, do_spellcheck_toggle, NULL); 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (pref_checkbox), request);
    gtk_toggle_tool_button_set_active(GTK_TOGGLE_TOOL_BUTTON (toolbar_button), request);
    g_signal_handlers_unblock_by_func(pref_checkbox, do_spellcheck_toggle, NULL); 
    g_signal_handlers_unblock_by_func(toolbar_button, do_spellcheck_toggle, NULL); 
    do_conditionally_enable_spellcheck (NULL, NULL);
}
