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
//! @file src/gtk-main-interface.c
//!
//! @brief Abstraction layer for gtk objects
//!
//! Used as a go between for functions interacting with GUI interface objects.
//! This is the gtk version.
//!

#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>


//Private variables
static GwSearchItem *_progress_feedback_item = NULL;
static gboolean _prev_selection_icon_state = FALSE;
static int _previous_tip = 0;

void _initialize_gtk_builder_translation_hack (void);


//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
void gw_main_initialize ()
{
    gw_common_load_ui_xml ("main.ui");
    _initialize_gtk_builder_translation_hack ();
    gw_tabs_initialize ();
}

void gw_main_free ()
{
  gw_tabs_free ();
}


//!
//! @brief Updates the progress information based on the GwSearchItem info
//!
//! This method isn't called directly, but through the global output funcion pointers
//! set at the beginning of the program.
//!
//! @param item A GwSearchItem pointer to gleam information from.
//!
gboolean gw_ui_update_progress_feedback (gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    int page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    GwSearchItem *item = g_list_nth_data (gw_tabs_get_searchitem_list (), page_num);

    if (item != NULL) 
    {
      g_mutex_lock (item->mutex);
        if (item != _progress_feedback_item || item->current_line != item->progress_feedback_line)
        {
          _progress_feedback_item = item;
          item->progress_feedback_line = item->current_line;
          gw_ui_set_search_progressbar_by_searchitem (item);
          gw_ui_set_total_results_label_by_searchitem (item);
          gw_ui_set_main_window_title_by_searchitem (item);
        }
      g_mutex_unlock (item->mutex);
    }

   return TRUE;
}


//!
//! @brief Sets the query text of the program using the informtion from the searchitem
//!
//! @param item a GwSearchItem argument.
//!
void gw_ui_set_query_entry_text_by_searchitem (GwSearchItem *item)
{
    //Initializations
    char hex_color_string[100];
    GdkColor color;
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    //If there is no search, set the default colors
    if (item == NULL)
    {
      gtk_entry_set_text (GTK_ENTRY (search_entry), "");
      gtk_widget_modify_base (GTK_WIDGET (search_entry), GTK_STATE_NORMAL, NULL);
      gtk_widget_modify_text (GTK_WIDGET (search_entry), GTK_STATE_NORMAL, NULL);
    }
    //There was previously a search, set the match colors from the prefs
    else
    {
      if (item->queryline != NULL && strlen(item->queryline->string) > 0)
      {
        if (strcmp(gtk_entry_get_text (GTK_ENTRY (search_entry)), item->queryline->string) != 0)
        {
          gtk_entry_set_text (GTK_ENTRY (search_entry), item->queryline->string);
          gtk_editable_set_position (GTK_EDITABLE (search_entry), -1);
        }
      }
      else
      {
        gtk_entry_set_text (GTK_ENTRY (search_entry), "");
      }

      //Set the foreground color
      gw_pref_get_string (hex_color_string, GW_SCHEMA_HIGHLIGHT, GW_KEY_MATCH_FG, 100);
      if (gdk_color_parse (hex_color_string, &color) == FALSE)
      {
        gw_pref_reset_value (GW_SCHEMA_HIGHLIGHT, GW_KEY_MATCH_FG);
        return;
      }
      gtk_widget_modify_text (GTK_WIDGET (search_entry), GTK_STATE_NORMAL, &color);

      //Set the background color
      gw_pref_get_string (hex_color_string, GW_SCHEMA_HIGHLIGHT, GW_KEY_MATCH_BG, 100);
      if (gdk_color_parse (hex_color_string, &color) == FALSE)
      {
        gw_pref_reset_value (GW_SCHEMA_HIGHLIGHT, GW_KEY_MATCH_BG);
        return;
      }
      gtk_widget_modify_base (GTK_WIDGET (search_entry), GTK_STATE_NORMAL, &color);
    }
}


//!
//! @brief Sets the main window title text of the program using the informtion from the searchitem
//!
//! @param item a GwSearchItem argument.
//!
void gw_ui_set_main_window_title_by_searchitem (GwSearchItem *item)
{
    GtkBuilder *builder = gw_common_get_builder ();

    //Declarations
    char *full_title = NULL;
    char *base_title = gettext("gWaei Japanese-English Dictionary");
    gboolean required_objects_exist = (item != NULL && item->queryline != NULL);
    GtkWidget *window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
    int relevant;
    int total;

    //Initializations
    if (required_objects_exist)
    {
      char *query = item->queryline->string;
      int relevant = item->total_relevant_results;
      int total = item->total_results;
      full_title = g_strdup_printf ("%s [%d/%d] - %s", query, relevant, total, base_title);
    }

    //Work
    if (required_objects_exist && full_title != NULL)
    {
      gtk_window_set_title (GTK_WINDOW (window), full_title);
      g_free (full_title);
    }
    else
    {
      gtk_window_set_title (GTK_WINDOW (window), base_title);
    }
}


//!
//! @brief Closes the suggestion box.  (Currently unused feature of gWaei)
//!
void gw_ui_close_suggestion_box ()
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *suggestion_hbox;
    suggestion_hbox = GTK_WIDGET (gtk_builder_get_object (builder, "suggestion_hbox"));
    gtk_widget_hide (suggestion_hbox);
}


//!
//! @brief Currently unused function
//!
//! @param string Unknown
//! @param query Unknown
//! @param query_length Unknown
//! @param extension Unknown
//!
void gw_ui_set_information_box_label (const char* string, const char* query, int query_length, const char* extension)
{
    GtkBuilder *builder = gw_common_get_builder ();

    if (query_length > 1)
    {
      char query_short[300];
      strncpy(query_short, query, query_length);
      query_short[query_length] = '\0';

      GtkWidget *suggestion_label;
      suggestion_label = GTK_WIDGET (gtk_builder_get_object (builder, "suggestion_label"));
      char *label_text = g_strdup_printf (string, query_short, extension);
      if (label_text != NULL)
      {
        gtk_label_set_text (GTK_LABEL (suggestion_label), label_text);
        g_free (label_text);

        GtkWidget *suggestion_hbox;
        suggestion_hbox = GTK_WIDGET (gtk_builder_get_object (builder, "suggestion_hbox"));
        gtk_widget_show (suggestion_hbox);
      }
    }
}


//!
//! @brief Currently unused function
//!
//! @param item A GwSearchItem pointer to gleam information from
//!
void gw_ui_verb_check_with_suggestion (GwSearchItem *item)
{
/*
    if (item == NULL || item->queryline == NULL || item->resultline == NULL || item->target != GW_TARGET_RESULTS) return;

    GtkBuilder *builder = gw_common_get_builder ();

    //It's already shown.  No need to do anything.
    GtkWidget *suggestion_hbox;
    suggestion_hbox = GTK_WIDGET (gtk_builder_get_object (builder, "suggestion_hbox"));
    if (gtk_widget_get_visible (suggestion_hbox) == TRUE) return;

    char *query = item->queryline->hira_string;
    GwResultLine *rl = item->resultline;

    if (query[0] == '\0' || rl->kanji_start == NULL)
      return;

    gunichar query_first_letter;
    query_first_letter = g_utf8_get_char (query);
    gunichar result_kanji_first_letter;
    result_kanji_first_letter = g_utf8_get_char (rl->kanji_start);
    gunichar result_furigana_first_letter; 
    if (rl->furigana_start != NULL)
      result_furigana_first_letter = g_utf8_get_char (rl->furigana_start);
    else
      result_furigana_first_letter = result_kanji_first_letter;

    //Make sure the query and the search result start similarly
    if (rl->classification_start == NULL || (query_first_letter != result_kanji_first_letter && query_first_letter != result_furigana_first_letter))
      return;

    GtkWidget *suggestion_label;
    suggestion_label = GTK_WIDGET (gtk_builder_get_object (builder, "suggestion_label"));
    GtkWidget *suggestion_eventbox;
    suggestion_eventbox = GTK_WIDGET (gtk_builder_get_object (builder, "suggestion_eventbox"));
    GtkWidget *window;
    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
    GdkColor fgcolor;
    fgcolor = window->style->fg[GTK_STATE_SELECTED];
    GdkColor bgcolor;
    bgcolor = window->style->bg[GTK_STATE_SELECTED];

    gtk_event_box_set_visible_window (GTK_EVENT_BOX (suggestion_eventbox), TRUE);
    gtk_widget_modify_fg (suggestion_eventbox, GTK_STATE_NORMAL, &fgcolor);
    gtk_widget_modify_bg (suggestion_eventbox, GTK_STATE_NORMAL, &bgcolor);

    //i-adjective stuffs
    if (strstr(rl->classification_start, "adj-i"))
    {
      int nmatch = 1;
      regmatch_t pmatch[nmatch];
      if (regexec (&re_i_adj_past, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a past-form i-adjective.  (Try searching for %s%s?)");
        gw_ui_set_inforation_box_label (message, query, pmatch[0].rm_so, "い");
      }
      else if (regexec (&re_i_adj_negative, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a negative i-adjective.  (Try searching for %s%s?)");
        gw_ui_set_inforation_box_label (message, query, pmatch[0].rm_so, "い");
      }
      else if (regexec (&re_i_adj_te, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a te-form i-adjective.  (Try searching for %s%s?)");
        gw_ui_set_inforation_box_label (message, query, pmatch[0].rm_so, "い");
      }
      else if (regexec (&re_i_adj_causative, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a causitive i-adjective.  (Try searching for %s%s?)");
        gw_ui_set_inforation_box_label (message, query, pmatch[0].rm_so, "い");
      }
      else if (regexec (&re_i_adj_conditional, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a conditional-form i-adjective.  (Try searching for %s%s?)");
        gw_ui_set_inforation_box_label (message, query, pmatch[0].rm_so, "い");
      }
    }

    //na-adjective stuffs
    else if (strstr(rl->classification_start, "adj-na"))
    {
      int nmatch = 1;
      regmatch_t pmatch[nmatch];
      if (regexec(&re_na_adj_past, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a past-form na-adjective.  (Try searching for %s%s?)");
        gw_ui_set_inforation_box_label (message, query, pmatch[0].rm_so, "");
      }
      else if (regexec(&re_na_adj_negative, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a negative na-adjective.  (Try searching for %s%s?)");
        gw_ui_set_inforation_box_label (message, query, pmatch[0].rm_so, "");
      }
      else if (regexec(&re_na_adj_te, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a te-form na-adjective.  (Try searching for %s%s?)");
        gw_ui_set_inforation_box_label (message, query, pmatch[0].rm_so, "");
      }
      else if (regexec(&re_na_adj_causative, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a causitive na-adjective.  (Try searching for %s%s?)");
        gw_ui_set_inforation_box_label (message, query, pmatch[0].rm_so, "");
      }
      else if (regexec(&re_na_adj_conditional, query, nmatch, pmatch, 0) == 0)
      {
        char *message = gettext("Your query is possibly a conditional-form na-adjective.  (Try searching for %s%s?)");
        gw_ui_set_inforation_box_label (message, query, pmatch[0].rm_so, "");
      }
    }
*/
}



//!
//! \brief Hacks to make the gtkbuilder file from glade 2.6.7 work correctly
//!
//! Likely the problem is that other packages just haven't gotten to suporting
//! it yet. So hopefully we can just clean delete this function in time.
//!
//! @see initialize_gui_interface ()
//!
void _initialize_gtk_builder_translation_hack ()
{
      GtkBuilder *builder = gw_common_get_builder ();
    /*Note: this code doesn't work for adding an accelerator*/
//    gtk_action_set_accel_group (action, accel_group);
//    gtk_action_group_add_action_with_accel (action_group, action, "<control>r");
//    gtk_action_connect_accelerator (action);

    /*Note: but this code does*/
//    gtk_widget_add_accelerator (GTK_WIDGET (
//    gtk_builder_get_object (builder, "word_edge_menuitem")),
//    "activate", accel_group, GDK_b, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    GtkAccelGroup* accel_group;
    GtkWidget *menu;
    accel_group = GTK_ACCEL_GROUP (gtk_builder_get_object (builder, "main_accelgroup"));
    GtkActionGroup* action_group;
    action_group = GTK_ACTION_GROUP (gtk_builder_get_object (builder, "main_actiongroup"));
    const int id_length = 50;
    char id[id_length];
    GtkAction *action;
    GtkWidget *menuitem;


    action = gtk_action_new ("new_tab_action", gettext("_New Tab"), NULL, NULL);
    gtk_action_set_icon_name (action, "stock_new-tab"); 
    g_signal_connect( G_OBJECT (action), "activate", G_CALLBACK (do_new_tab), NULL);
    gtk_action_set_accel_group (action, accel_group);
    gtk_action_group_add_action_with_accel (action_group, action, "<control>t");
    menuitem = GTK_WIDGET (gtk_builder_get_object (builder, "new_tab_menuitem"));
    gtk_activatable_set_related_action (GTK_ACTIVATABLE (menuitem), action);

    char *temp = gettext("Only for Non-Japanese Locals");
    char *temp1  = gettext("When Possible");
    char *temp2  = gettext("Never");
    char *temp3 = gettext("Go to the _Project Homepage");
    char *temp4 = gettext("Get Help on I_RC");
    char *temp5 = gettext("Other Dictionaries");
    char *temp6 = gettext("Main Dictionaries");

    char *temp7 = gettext("Order");
    char *temp8 = gettext("Shortcut");
    char *temp9 = gettext("Dictionary");
    char *temp10 = gettext("_Annotate Strokes");


    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "close_menuitem")), "activate", accel_group, GDK_w, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "quit_menuitem")), "activate", accel_group, GDK_q, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "clear_menuitem")), "activate", accel_group, GDK_l, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "copy_menuitem")), "activate", accel_group, GDK_c, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "paste_menuitem")), "activate", accel_group, GDK_v, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "select_all_menuitem")), "activate", accel_group, GDK_a, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "cut_menuitem")), "activate", accel_group, GDK_x, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    //Edit action
    action = GTK_ACTION (gtk_builder_get_object (builder, "file_edit_action"));
    gtk_action_set_tooltip (action, gettext("Edit the current vocabulary list"));

    //Append action
    action = GTK_ACTION (gtk_builder_get_object (builder, "file_append_action"));
    gtk_action_set_label (action, gettext("A_ppend"));
    gtk_action_set_tooltip (action, gettext("Append the current results to a file"));
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "append_menuitem")), "activate", accel_group, GDK_s, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    //Save as action
    action = GTK_ACTION (gtk_builder_get_object (builder, "file_save_as_action"));
    gtk_action_set_tooltip (action, gettext("Save the current results to a new file"));

    //Print action
    action = GTK_ACTION (gtk_builder_get_object (builder, "file_print_action"));
    gtk_action_set_tooltip (action, gettext("Print the current results"));
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "print_menuitem")), "activate", accel_group, GDK_p, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    //Zoom in action
    action = GTK_ACTION (gtk_builder_get_object (builder, "view_zoom_in_action"));
    gtk_action_set_label (action, gettext("_Enlarge Text"));
    gtk_action_set_short_label (action, gettext("Enlarge"));
    gtk_action_set_tooltip (action, gettext("Enlarge the results text"));
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "zoom_in_menuitem")), "activate", accel_group, GDK_plus, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "zoom_in_menuitem")), "activate", accel_group, GDK_KP_Add, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    //Zoom out action
    action = GTK_ACTION (gtk_builder_get_object (builder, "view_zoom_out_action"));
    gtk_action_set_label (action, gettext("_Shrink Text"));
    gtk_action_set_short_label (action, gettext("Shrink"));
    gtk_action_set_tooltip (action, gettext("Shrink the results text"));
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "zoom_out_menuitem")), "activate", accel_group, GDK_minus, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "zoom_out_menuitem")), "activate", accel_group, GDK_KP_Subtract, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    //Normal size action
    action = GTK_ACTION (gtk_builder_get_object (builder, "view_zoom_100_action"));
    gtk_action_set_label (action, gettext("_Normal Size"));
    gtk_action_set_short_label (action, gettext("Normal"));
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "zoom_100_menuitem")), "activate", accel_group, GDK_0, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "zoom_100_menuitem")), "activate", accel_group, GDK_KP_0, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    //Show toolbar action
    action = GTK_ACTION (gtk_builder_get_object (builder, "view_toggle_toolbar_action"));
    gtk_action_set_label (action, gettext("Show _Toolbar"));

    //Using kanjipad action
    action = GTK_ACTION (gtk_builder_get_object (builder, "insert_kanjipad_action"));
    gtk_action_set_label (action, gettext("Using _Kanjipad"));
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "kanjipad_menuitem")), "activate", accel_group, GDK_k, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    //Using radical search tool action
    action = GTK_ACTION (gtk_builder_get_object (builder, "insert_radicals_action"));
    gtk_action_set_label (action, gettext("Using _Radical Search Tool"));
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "radicals_menuitem")), "activate", accel_group, GDK_r, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);

    //Word boundary action
    action = GTK_ACTION (gtk_builder_get_object (builder, "insert_word_edge_action"));
    gtk_action_set_label (action, gettext("_Word-edge Mark"));
    gtk_action_set_short_label (action, gettext("_Edge"));
    gtk_action_set_tooltip (action, gettext("Insert a word-edge character"));
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "word_edge_menuitem")), "activate", accel_group, GDK_b, GDK_CONTROL_MASK, GTK_ACCEL_VISIBLE);


    //Not Word boundary action
    action = GTK_ACTION (gtk_builder_get_object (builder, "insert_not_word_edge_action"));
    gtk_action_set_label (action, gettext("_Not-word-edge Mark"));
    gtk_action_set_short_label (action, gettext("_Not Edge"));
    gtk_action_set_tooltip (action, gettext("Insert a not-word-edge character"));
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "not_word_edge_menuitem")), "activate", accel_group, GDK_b, (GDK_CONTROL_MASK|GDK_SHIFT_MASK), GTK_ACCEL_VISIBLE);

    //Unknown character action
    action = GTK_ACTION (gtk_builder_get_object (builder, "insert_unknown_character_action"));
    gtk_action_set_label (action, gettext("_Unknown Character"));
    gtk_action_set_short_label (action, gettext("_Unknown"));
    gtk_action_set_tooltip (action, gettext("Insert an unknown character"));

    //Or action
    action = GTK_ACTION (gtk_builder_get_object (builder, "insert_or_action"));
    gtk_action_set_label (action, gettext("_Or Character"));

    //And action
    action = GTK_ACTION (gtk_builder_get_object (builder, "insert_and_action"));
    gtk_action_set_label (action, gettext("_And Character"));

    //Previous
    action = GTK_ACTION (gtk_builder_get_object (builder, "history_back_action"));
    gtk_action_set_label (action, gettext("_Previous Search"));
    gtk_action_set_short_label (action, gettext("Previous"));
    gtk_action_set_tooltip (action, gettext("Go to the previous search"));
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "back_menuitem")), "activate", accel_group, GDK_Left, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

    //Next
    action = GTK_ACTION (gtk_builder_get_object (builder, "history_forward_action"));
    gtk_action_set_label (action, gettext("_Next Search"));
    gtk_action_set_short_label (action, gettext("Next"));
    gtk_action_set_tooltip (action, gettext("Go to the next search"));
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "forward_menuitem")), "activate", accel_group, GDK_Right, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);

    //Help
    action = GTK_ACTION (gtk_builder_get_object (builder, "help_program_action"));
    gtk_action_set_label (action, gettext("_Contents"));
    gtk_action_set_short_label (action, gettext("Help"));
    gtk_action_set_tooltip (action, gettext("Open the help dialog"));
    gtk_widget_add_accelerator (GTK_WIDGET (gtk_builder_get_object (builder, "help_menuitem")), "activate", accel_group, GDK_F1, 0, GTK_ACCEL_VISIBLE);

    //Help glossary
    action = GTK_ACTION (gtk_builder_get_object (builder, "help_glossary_action"));
    gtk_action_set_label (action, gettext("Dictionary _Terminology Glossary"));


    GtkWidget *widget;

    //Spellcheck button
    widget = GTK_WIDGET (gtk_builder_get_object (builder, "spellcheck_toolbutton"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (widget), gettext("Enable spellcheck for searches"));

    //Search Entry
    widget = GTK_WIDGET (gtk_builder_get_object (builder, "search_entry"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (widget), gettext("Input your query here"));

    //Dictionary Combobox
    widget = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_combobox"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (widget), gettext("Select the dictionary you want to use"));

    //Search button
    widget = GTK_WIDGET (gtk_builder_get_object (builder, "search_entry_submit_button"));
    gtk_widget_set_tooltip_text (GTK_WIDGET (widget), gettext("Click to start search"));
}


//!
//! @brief Updates the states of the toolbar buttons etc in the main interface
//!
void gw_ui_update_toolbar_buttons ()
{
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    const int id_length = 50;
    char id[id_length];

    //Delarations
    GtkAction *action;
    GtkWidget *menuitem;
    gboolean enable;

    GwSearchItem* history_search_item = gw_historylist_get_current (GW_TARGET_RESULTS);
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    int page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    int pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
    GwSearchItem *tab_search_item = g_list_nth_data (gw_tabs_get_searchitem_list (), page_num);

    int current_font_magnification;
    current_font_magnification = gw_pref_get_int (GW_SCHEMA_FONT, GW_KEY_FONT_MAGNIFICATION);

    GtkWidget *results_tv = gw_common_get_widget_by_target(GW_TARGET_RESULTS);

    //Update Zoom in sensitivity state
    strncpy(id, "view_zoom_in_action", id_length);
    action = GTK_ACTION (gtk_builder_get_object (builder, id));
    enable = (tab_search_item != NULL && current_font_magnification < GW_MAX_FONT_MAGNIFICATION);
    gtk_action_set_sensitive(action, enable);

    //Update Zoom out sensitivity state
    strncpy(id, "view_zoom_out_action", id_length);
    action = GTK_ACTION (gtk_builder_get_object (builder, id));
    enable = (tab_search_item != NULL && current_font_magnification > GW_MIN_FONT_MAGNIFICATION);
    gtk_action_set_sensitive(action, enable);

    //Update Zoom 100 sensitivity state
    strncpy(id, "view_zoom_100_action", id_length);
    action = GTK_ACTION (gtk_builder_get_object(builder, id));
    enable = (tab_search_item != NULL && current_font_magnification != GW_DEFAULT_FONT_MAGNIFICATION);
    gtk_action_set_sensitive(action, enable);

    //Update Save sensitivity state
    strncpy(id, "file_append_action", id_length);
    action = GTK_ACTION (gtk_builder_get_object(builder, id));
    enable = (tab_search_item != NULL);
    gtk_action_set_sensitive(action, enable);

    //Update Save as sensitivity state
    strncpy(id, "file_save_as_action", id_length);
    action = GTK_ACTION (gtk_builder_get_object(builder, id));
    enable = (tab_search_item != NULL);
    gtk_action_set_sensitive(action, enable);

    //Update Print sensitivity state
    strncpy(id, "file_print_action", id_length);
    action = GTK_ACTION (gtk_builder_get_object(builder, id));
    enable = (tab_search_item != NULL);
    gtk_action_set_sensitive(action, enable);
/*
    //Update radicals search tool menuitem
    strncpy(id, "insert_radicals_action", id_length);
    action = GTK_ACTION (gtk_builder_get_object (builder, id));
    enable = (gw_dictlist_dictionary_get_status_by_id (GW_DICT_ID_RADICALS) == GW_DICT_STATUS_INSTALLED);
    gtk_action_set_sensitive(action, enable);
*/
    //Update back button
    strncpy(id, "history_back_action", id_length);
    action = GTK_ACTION (gtk_builder_get_object (builder, id));
      gtk_action_set_sensitive (action, (gw_historylist_get_back_history (GW_TARGET_RESULTS) != NULL));

    //Update forward button
    strncpy(id, "history_forward_action", id_length);
    action = GTK_ACTION (gtk_builder_get_object (builder, id));
      gtk_action_set_sensitive (action, (gw_historylist_get_forward_history (GW_TARGET_RESULTS) != NULL));

    //Update cut/copy buttons
    gboolean sensitive;
    if (gtk_widget_has_focus (search_entry))
    {
      sensitive = (gtk_editable_get_selection_bounds (GTK_EDITABLE (search_entry), NULL, NULL));
      strncpy(id, "edit_copy_action", id_length);
      action = GTK_ACTION (gtk_builder_get_object (builder, id));
      gtk_action_set_sensitive (action, sensitive);
      strncpy(id, "edit_cut_action", id_length);
      action = GTK_ACTION (gtk_builder_get_object (builder, id));
      gtk_action_set_sensitive (action, sensitive);
    }
    else if (results_tv != NULL && gtk_widget_has_focus (results_tv))
    {
      sensitive = (gw_ui_has_selection_by_target (GW_TARGET_RESULTS));
      strncpy(id, "edit_copy_action", id_length);
      action = GTK_ACTION (gtk_builder_get_object (builder, id));
      gtk_action_set_sensitive (action, sensitive);
      strncpy(id, "edit_cut_action", id_length);
      action = GTK_ACTION (gtk_builder_get_object (builder, id));
      gtk_action_set_sensitive (action, FALSE);
    }

    strncpy(id, "previous_tab_menuitem", id_length);
    menuitem = GTK_WIDGET (gtk_builder_get_object (builder, id));
    gtk_widget_set_sensitive (menuitem, (pages > 1));

    strncpy(id, "next_tab_menuitem", id_length);
    menuitem = GTK_WIDGET (gtk_builder_get_object (builder, id));
    gtk_widget_set_sensitive (menuitem, (pages > 1));
}


//!
//! @brief Set's the progress label of the program using the inforamtion from the searchitem
//!
//! @param item A GwSearchItem pointer to gleam information from
//!
void gw_ui_set_total_results_label_by_searchitem (GwSearchItem* item)
{
    GtkBuilder *builder = gw_common_get_builder ();
    GtkWidget *label = GTK_WIDGET (gtk_builder_get_object(builder, "progress_label"));

    if (item == NULL)
    {
      gtk_label_set_text(GTK_LABEL (label), "");
    }
    else if (item->target != GW_TARGET_RESULTS)
    {
      return;
    }
    else
    {
      //Declarations
      int relevant = item->total_relevant_results;
      int irrelevant = item->total_irrelevant_results;
      int total = item->total_results;

      char *idle_message_none = "";
      char *searching_message_none = gettext("Searching...");

      char *idle_message_total = ngettext("Found %d result", "Found %d results", total);
      char *searching_message_total = ngettext("Searching... %d result", "Searching... %d results", total);

      // TRANSLATORS: relevant what ? It's the number of "relevant" result(s) displayed while or after searching.
      char *message_relevant = ngettext("(%d Relevant)", "(%d Relevant)", relevant);

      char *base_message = NULL;
      char *final_message = NULL;

      //Initializations
      switch (item->status)
      {
        case GW_SEARCH_IDLE:
        case GW_SEARCH_FINISHING:
            if (item->current_line == 0)
              gtk_label_set_text(GTK_LABEL (label), idle_message_none);
            else if (relevant == total)
              final_message = g_strdup_printf (idle_message_total, total);
            else
            {
              base_message = g_strdup_printf ("%s %s", idle_message_total, message_relevant);
              if (base_message != NULL)
                final_message = g_strdup_printf (base_message, total, relevant);
            }
            break;
        case GW_SEARCH_SEARCHING:
            if (item->total_results == 0)
              gtk_label_set_text(GTK_LABEL (label), searching_message_none);
            else if (relevant == total)
              final_message = g_strdup_printf (searching_message_total, total);
            else
            {
              base_message = g_strdup_printf ("%s %s", searching_message_total, message_relevant);
              if (base_message != NULL)
                final_message = g_strdup_printf (base_message, total, relevant);
            }
            break;
      }

      //Finalize
      if (base_message != NULL)
        g_free (base_message);
      if (final_message != NULL)
      {
        gtk_label_set_text(GTK_LABEL (label), final_message);
        g_free (final_message);
      }
    }
}


//!
//! @brief Sets the current dictionary by using the load position
//!
//! @param request Sets the current dictionary by the number here
//!
void gw_ui_set_dictionary (int request)
{
    GtkBuilder *builder = gw_common_get_builder ();

    //Set the correct dictionary in the dictionary list
    if (gw_dictlist_set_selected_by_load_position (request) == NULL)
      return;

    //Time to make sure everything matches up in the gui
    GtkWidget *combobox;
    combobox = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_combobox"));

    GtkMenuShell *shell = NULL;
    shell = GTK_MENU_SHELL (gtk_builder_get_object (builder, "dictionary_popup"));
    if (shell != NULL)
    {
      GList     *children = NULL;
      children = gtk_container_get_children (GTK_CONTAINER (shell));
      GtkWidget *radioitem = g_list_nth_data (children, request);
      if (radioitem != NULL)
      {
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (radioitem), TRUE);
        gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), request);
      }
    }
}


//!
//! @brief Uses a GwSearchItem to set the currently active dictionary
//!
//! This function is greatly useful for doing searches from the history.
//!
//! @param item A GwSearchItem to gleam information from
//!
void gw_ui_set_dictionary_by_searchitem (GwSearchItem *item)
{
    if (item != NULL && item->dictionary != NULL)
      gw_ui_set_dictionary (item->dictionary->load_position);
}


//!
//! @brief Loads up the dictionaries for the GUI
//!
int rebuild_combobox_dictionary_list () 
{
/*
    gw_settings_increment_order_list_processes ();

    //Parse the string
    char order[5000];
    char new_order[5000];
    GwDictInfo* di = NULL;
    gw_pref_get_string (order, GW_SCHEMA_DICTIONARY, GW_KEY_LOAD_ORDER, 5000);

    char *names[50];
    char *mix_name = NULL, *kanji_name = NULL, *radicals_name = NULL;
    names[0] = order;
    int i = 0;
    while ((names[i + 1] = g_utf8_strchr (names[i], -1, L',')) && i < 50)
    {
      i++;
      *names[i] = '\0';
      names[i]++;
    }
    names[i + 1] = NULL;

    //Add any missing dictionaries
    int j = 0;
    GList *list = gw_dictlist_get_list ();
    while (list != NULL)
    {
      di = list->data;
      j = 0;
      while (names[j] != NULL && strcmp(di->name, names[j]) != 0)
        j++;

      if (names[j] == NULL && j > 0 && di->status == GW_DICT_STATUS_INSTALLED)
      {
        names[j] = names[j - 1];
        while (*names[j] != '\0') names[j]++;
        names[j]++;
        strcpy(names[j], di->name);
        names[j + 1] = NULL;
      }
      list = g_list_next (list);
    }

    //Remove not installed dictionaries from the list
    i = 0;
    while (names[i] != NULL)
    {
      di = gw_dictlist_get_dictinfo_by_name (names[i]);
      if (di == NULL)
        *names[i] = '\0';
      i++;
    }

    //Collapse the holes
    j = 0;
    i = 0;
    new_order[0] = '\0';
    while (names[i] != NULL && names[j] != NULL)
    {
      if (*names[j] == '\0')
      {
        j++;
      }
      else if (*names[j] != '\0')
      {
        names[i] = names[j];
        strcat(new_order, names[j]);
        strcat(new_order, ",");
        i++;
        j++;
      }
    }
    new_order[strlen(new_order) - 1] = '\0';
    names[i] = NULL;
    gw_pref_set_string (GW_SCHEMA_DICTIONARY, GW_KEY_LOAD_ORDER, new_order);

    //Initialize variables
    const int id_length = 50;
    char id[id_length];

    GtkListStore *list_store;
    list_store = GTK_LIST_STORE (gtk_builder_get_object (builder, "list_store_dictionaries"));

    gtk_list_store_clear (list_store);

    GSList* group = NULL;
    GtkAccelGroup* accel_group;
    accel_group = GTK_ACCEL_GROUP (gtk_builder_get_object (builder, "main_accelgroup"));
    GtkWidget *item = NULL;
    i = 0;


    //Remove all widgets after the back and forward menuitem buttons
    GtkMenuShell *shell = NULL;
    shell = GTK_MENU_SHELL (gtk_builder_get_object (builder, "dictionary_popup"));
    if (shell != NULL)
    {
      GList     *children = NULL;
      children = gtk_container_get_children (GTK_CONTAINER (shell));
      while (children != NULL )
      {
        gtk_widget_destroy(children->data);
        children = g_list_delete_link(children, children);
      }
    }

    printf("\n");

    //Start filling in the new items
    GwDictInfo *di_alias, *di_name;
    GtkTreeIter iter;
    j = 0;
    char *dictionary_name = NULL;
    char *long_name = NULL;
    char *icon_name = NULL;
    char *shortcut_name = NULL;
    char *order_number = NULL;
    char *favorite_icon = "emblem-favorite";
    while (names[i] != NULL)
    {
      di_alias = gw_dictlist_get_dictinfo_by_alias (names[i]);
      di_name = gw_dictlist_get_dictinfo_by_name (names[i]);
      di = di_alias;
      if (strcmp(di_alias->name, di_name->name) == 0 && di_alias->status == GW_DICT_STATUS_INSTALLED)
      {
        printf("%d %s\n", j, di_alias->long_name);

        dictionary_name = di_alias->short_name;
        long_name = di_alias->long_name;
        if (j == 0)
          icon_name = favorite_icon;
        else
          icon_name = NULL;
        if (j < 10)
          shortcut_name = g_strdup_printf ("Alt-%d", j + 1);

        order_number = g_strdup_printf ("%d", j + 1);

        //Refill the combobox
        gtk_list_store_append (GTK_LIST_STORE (list_store), &iter);
        gtk_list_store_set (list_store, &iter, 0, dictionary_name, 1, icon_name, 2, order_number, 3, shortcut_name, 4, long_name, -1);

        //Free allocated momory
        if (shortcut_name != NULL)
        {
          g_free (shortcut_name);
          shortcut_name = NULL;
        }
        if (order_number != NULL)
        {
          g_free (order_number);
          order_number = NULL;
        }

        //Refill the menu
        item = GTK_WIDGET (gtk_radio_menu_item_new_with_label (group, di->long_name));
        group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (item));
        gtk_menu_shell_append (GTK_MENU_SHELL (shell),  GTK_WIDGET (item));
        if (i == 0) gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
        g_signal_connect(G_OBJECT (item), "toggled", G_CALLBACK (do_dictionary_changed_action), NULL);
        if (j + 1 < 10) gtk_widget_add_accelerator (GTK_WIDGET (item), "activate", accel_group, (GDK_0 + j + 1), GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
        gtk_widget_show (item);

        di->load_position = j;
        j++;
      }

      i++;
    }

    //Set the combobox to the first item
    GtkWidget *combobox;
    combobox = GTK_WIDGET (gtk_builder_get_object(builder, "dictionary_combobox"));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);

    //Fill in the other menu items
    item = GTK_WIDGET (gtk_separator_menu_item_new());
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (item));
    gtk_widget_show (GTK_WIDGET (item));

    item = GTK_WIDGET (gtk_menu_item_new_with_mnemonic(gettext("_Cycle Up")));
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (item));
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (do_cycle_dictionaries_backward), NULL);
    gtk_widget_add_accelerator (GTK_WIDGET (item), "activate", accel_group, GDK_Up, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_show (GTK_WIDGET (item));

    item = GTK_WIDGET (gtk_menu_item_new_with_mnemonic(gettext("Cycle _Down")));
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (item));
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (do_cycle_dictionaries_forward), NULL);
    gtk_widget_add_accelerator (GTK_WIDGET (item), "activate", accel_group, GDK_Down, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_show (GTK_WIDGET (item));

    gw_ui_update_settings_interface();
    gw_settings_decrement_order_list_processes ();

    //Finish
    printf(ngettext("%d dictionary is being used.", "%d dictionaries are being used.",j), j);
    printf("\n\n");
    return j;
    */
return 0;
}


//!
//! @brief Updates the status of the search progressbar
//!
//! @param item A GwSearchItem to gleam information from
//!
void gw_ui_set_search_progressbar_by_searchitem (GwSearchItem *item)
{
  /*
    GtkWidget *progress = GTK_WIDGET (gtk_builder_get_object(builder, "search_progressbar"));
    long current = 0;
    long total = 0;

    if (item != NULL && item->dictionary != NULL)
    {
      current = item->current_line;
      total = item->dictionary->total_lines;
    }

    if (total == 0) {
      gtk_progress_bar_pulse (GTK_PROGRESS_BAR (progress));
      gtk_progress_bar_set_pulse_step (GTK_PROGRESS_BAR (progress), 0.05);
    }

    else if (item == NULL || item->dictionary == NULL || total == 0 || ((double)current/(double)total) > 1.0 || item->status == GW_SEARCH_IDLE || item->status == GW_SEARCH_FINISHING)
    {
      gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR (progress), 0.0);
      gtk_widget_show (GTK_WIDGET (progress));
    }
    else
    {
      gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR (progress), ((double)current/(double)total));
      gtk_widget_show (GTK_WIDGET (progress));
    }
    */
}


//!
//! @brief Updates the history menu popup menu
//!
void gw_ui_update_history_menu_popup()
{
    GtkBuilder *builder = gw_common_get_builder ();

    const int id_length = 50;
    char id[id_length];

    GtkMenuShell *shell;
    strncpy (id, "history_popup", id_length);
    shell = GTK_MENU_SHELL (gtk_builder_get_object(builder, id));

    GList     *children = NULL;
    children = gtk_container_get_children (GTK_CONTAINER (shell));

    //Skip over the back/forward buttons
    if (children != NULL) children = g_list_next(children);
    if (children != NULL) children = g_list_next(children);

    //Remove all widgets after the back and forward menuitem buttons
    while (children != NULL )
    {
      gtk_widget_destroy(children->data);
      children = g_list_delete_link(children, children);
    }

    //Declarations
    GtkWidget *label;
    const char *text;
    GwSearchItem *item;
    GtkWidget *menuitem;

    children = gw_historylist_get_combined_history_list (GW_HISTORYLIST_RESULTS);

    //Add a separator if there are some items in history
    if (children != NULL)
    {
      //Add a seperator to the end of the history popup
      menuitem = GTK_WIDGET (gtk_separator_menu_item_new());
      gtk_menu_shell_append (GTK_MENU_SHELL (shell), menuitem);
      gtk_widget_show (menuitem);
    }

    //Fill the history items
    while (children != NULL)
    {
      item = children->data;

      GtkWidget *menu_item, *accel_label, *label;

      accel_label = gtk_label_new (item->dictionary->long_name);
      gtk_widget_set_sensitive (GTK_WIDGET (accel_label), FALSE);
      label = gtk_label_new (item->queryline->string);

      GtkWidget *hbox;
      hbox = gtk_hbox_new (FALSE, 0);

      menu_item = gtk_menu_item_new();
      g_signal_connect (GTK_WIDGET (menu_item), 
                        "activate",
                        G_CALLBACK (do_search_from_history), 
                        item                               );

      gtk_menu_shell_append(GTK_MENU_SHELL (shell), GTK_WIDGET (menu_item));
      gtk_container_add (GTK_CONTAINER (menu_item), hbox);
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
      gtk_box_pack_end (GTK_BOX (hbox), accel_label, FALSE, FALSE, 0);

      gtk_widget_show(label);
      gtk_widget_show(accel_label);
      gtk_widget_show(hbox);
      gtk_widget_show(menu_item);
      children = children->next;
    }
    g_list_free (children);
}


//!
//! @brief PRIVATE FUNCTION. Populate the menu item lists for the back and forward buttons
//!
//! @param id Id of the popuplist
//! @param list history list to compair against
//!
static void _rebuild_history_button_popup(char* id, GList* list)
{
    GtkBuilder *builder = gw_common_get_builder ();

    //Get a reference to the history_popup
    GtkWidget *popup;
    popup = GTK_WIDGET (gtk_builder_get_object(builder, id));

    GList     *children;
    children = gtk_container_get_children(GTK_CONTAINER (popup));

    //Remove all widgets after the back and forward menuitem buttons
    while (children != NULL )
    {
      gtk_widget_destroy(children->data);
      children = g_list_delete_link(children, children);
    }

    //Add a seperator to the end of the history popup
    GtkWidget *menuitem;
    menuitem = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL (popup), menuitem);
    gtk_widget_show(menuitem);

    //Declarations
    GtkWidget *label;
    const char *text;
    GwSearchItem *item;

    children = list;
    while (children != NULL)
    {
      item = children->data;

      //Ensure a minimuim width of the menu
      int leftover = 200;
      char label[leftover];
      strncpy (label, item->queryline->string, leftover);
      leftover -= strlen (item->queryline->string);
      while (leftover > 180)
      {
        strncat (label, " ", leftover);
        leftover -= 1;
      }

      menuitem = GTK_WIDGET (gtk_menu_item_new_with_label(label));

      //Create the new menuitem
      gtk_menu_shell_append(GTK_MENU_SHELL (popup), menuitem);
      gtk_widget_show  (menuitem);
      g_signal_connect (GTK_WIDGET (menuitem), 
                        "activate",
                        G_CALLBACK (do_search_from_history), 
                        item                               );
   
      children = children->next;
    }
}


//!
//! @brief Convenience function to update both the back and forward histories etc
//!
void gw_ui_update_history_popups ()
{
    GList* list = NULL;

    gw_ui_update_history_menu_popup();
    list = gw_historylist_get_forward_history (GW_HISTORYLIST_RESULTS);
    _rebuild_history_button_popup("forward_popup", list);
    list = gw_historylist_get_back_history (GW_HISTORYLIST_RESULTS);
    _rebuild_history_button_popup("back_popup", list);
}


//!
//! @brief Sets the requested font with magnification applied
//!
//! @param font_description_string The font with the font size
//! @param font_magnification And describing how to enlarge or shrink the font
//!
void gw_ui_set_font (char *font_description_string, int *font_magnification)
{
    GtkBuilder *builder = gw_common_get_builder ();

    gboolean use_global_font_setting = gw_pref_get_boolean (GW_SCHEMA_FONT, GW_KEY_FONT_USE_GLOBAL_FONT);
    char *new_font_description_string = NULL;
    char font_family[100];
    int font_size = 0;

    //Get the font family
    if (font_description_string == NULL)
    {
      if (use_global_font_setting)
        strcpy(font_family, "Sans 10");
//        gw_pref_get_string (font_family, GW_SCHEMA_GNOME_INTERFACE, GW_KEY_DOCUMENT_FONT_NAME, GW_DEFAULT_FONT, 100);
      else
        gw_pref_get_string (font_family, GW_SCHEMA_FONT, GW_KEY_FONT_CUSTOM_FONT, 100);
    }
    else
      strcpy (font_family, font_description_string);

    //Get the font size
    char *pos = strrchr (font_family, ' ');
    if (pos != NULL)
    {
      *pos = '\0';
      pos++;
      font_size = (int) g_ascii_strtoll (pos, NULL, 10);
    }
    else
    {
      font_size = GW_DEFAULT_FONT_SIZE;
    }

    //Add the magnification in to the font size
    if (font_magnification == NULL)
      font_size += gw_pref_get_int (GW_SCHEMA_FONT, GW_KEY_FONT_MAGNIFICATION);
    else
      font_size += *font_magnification;

    //Make sure the font size is sane
    if (font_size < GW_MIN_FONT_SIZE)
      font_size = GW_MIN_FONT_SIZE;
    else if (font_size > GW_MAX_FONT_SIZE)
      font_size = GW_MAX_FONT_SIZE;

    //Assemble the font description
    new_font_description_string = g_strdup_printf("%s %d", font_family, font_size);

    //Set it
    PangoFontDescription *desc;
    desc = pango_font_description_from_string (new_font_description_string);
    if (desc != NULL && new_font_description_string != NULL)
    {
      GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
      GtkWidget *textview = NULL;
      GtkWidget *scrolledwindow = NULL;
      int i = 0;
      while ((scrolledwindow = GTK_WIDGET (gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), i))) != NULL)
      {
        textview = GTK_WIDGET (gtk_bin_get_child (GTK_BIN (scrolledwindow)));
        gtk_widget_modify_font (GTK_WIDGET (textview), desc);
        i++;
      }
      pango_font_description_free (desc);
    }

    //Cleanup
    if (desc != NULL)
    {
      desc = NULL;
    }
    if (new_font_description_string != NULL)
    {
      g_free (new_font_description_string);
      new_font_description_string = NULL;
    }
}


//!
//! @brief Sets the style of the toolbar [icons/text/both/both-horizontal]
//!
//! @param request The name of the style
//!
void gw_ui_set_toolbar_style (const char *request) 
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *toolbar;
    toolbar = GTK_WIDGET (gtk_builder_get_object(builder, "toolbar"));

    GtkToolbarStyle style;
    if (strcmp(request, "text") == 0)
      style = GTK_TOOLBAR_TEXT;
    else if (strcmp(request, "icons") == 0)
      style = GTK_TOOLBAR_ICONS;
    else if (strcmp(request, "both-horiz") == 0)
      style = GTK_TOOLBAR_BOTH_HORIZ;
    else
      style = GTK_TOOLBAR_BOTH;

    gtk_toolbar_set_style(GTK_TOOLBAR (toolbar), style);
}


//!
//! @brief Sets the checkbox to show or hide the toolbar
//!
//! @param request How to set the toolbar
//!
void gw_ui_set_toolbar_show (gboolean request)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *toolbar;
    toolbar = GTK_WIDGET (gtk_builder_get_object(builder, "toolbar"));

    if (request == TRUE)
      gtk_widget_show(toolbar);
    else
      gtk_widget_hide(toolbar);

    GtkAction *action;
    action = GTK_ACTION (gtk_builder_get_object (builder, "view_toggle_toolbar_action"));

    g_signal_handlers_block_by_func (action, do_toolbar_toggle, NULL);
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), request);
    g_signal_handlers_unblock_by_func (action, do_toolbar_toggle, NULL);
}


//!
//! @brief Sets the checkbox state of the roma-kana conversion pref
//!
//! @param request How to set the preference
//!
void gw_ui_set_romaji_kana_conv (int request)
{
  GtkBuilder *builder = gw_common_get_builder ();

  GtkWidget *widget;
  widget = GTK_WIDGET (gtk_builder_get_object(builder, "query_romaji_to_kana"));

  gtk_combo_box_set_active(GTK_COMBO_BOX (widget), request);
}


//!
//! @brief Sets the checkbox state of the hira-kata conversion pref
//!
//! @param request How to set the preference
//!
void gw_ui_set_hiragana_katakana_conv (gboolean request)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *widget;
    if (widget = GTK_WIDGET (gtk_builder_get_object(builder, "query_hiragana_to_katakana")))
    {
      g_signal_handlers_block_by_func (widget, do_hiragana_katakana_conv_toggle, NULL);
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (widget), request);
      g_signal_handlers_unblock_by_func (widget, do_hiragana_katakana_conv_toggle, NULL);
    }
}


//!
//! @brief Sets the katakana-hiragana conversion checkbox being mindful to disable the event handlers
//!
//! @param request A boolean to use to set the checkbox state.
//!
void gw_ui_set_katakana_hiragana_conv (gboolean request)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *widget;
    if (widget = GTK_WIDGET (gtk_builder_get_object(builder, "query_katakana_to_hiragana")))
    {
      g_signal_handlers_block_by_func (widget, do_katakana_hiragana_conv_toggle, NULL);
      
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (widget), request);
      g_signal_handlers_unblock_by_func (widget, do_katakana_hiragana_conv_toggle, NULL);
    }
}


//!
//! @brief Sets the color to a switch minding to disable the event handlers on it
//!
//! @param widget_id The id of the widget to get.
//! @param hex_color_string The color to attempt to set.
//!
void gw_ui_set_color_to_swatch (const char *widget_id, const char *hex_color_string)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *widget;
    widget = GTK_WIDGET (gtk_builder_get_object (builder, widget_id));

    GdkColor color;
    if (gdk_color_parse (hex_color_string, &color) == TRUE)
    {
      gtk_color_button_set_color (GTK_COLOR_BUTTON (widget), &color);
    }
}


//!
//! @brief Appends some text to the text buffer
//!
//! @param item A GwSearchItem to gleam information from
//! @param text The text to append to the buffer
//! @param tag1 A tag to apply to the text or NULL
//! @param tag2 A tag to apply to the text or NULL
//! @param start_line Returns the start line of the text inserted
//! @param end_line Returns the end line of the text inserted
//!
void gw_ui_append_to_buffer (GwSearchItem *item, char *text, char *tag1,
                             char *tag2, int *start_line, int *end_line)
{
    //Assertain the target text buffer
    GObject *tb;
    if (item == NULL || item->target_tb == NULL) return;
    tb = G_OBJECT (item->target_tb);

    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER (tb), &iter);
    if (start_line != NULL)
      *start_line = gtk_text_iter_get_line(&iter);

    if (tag1 == NULL && tag2 == NULL)
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (tb),
                              &iter, text, -1          );
    else if (tag2 == NULL)
      gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (tb),
                                                &iter, text, -1,
                                                tag1, NULL            );
    else if (tag1 == NULL)
      gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (tb),
                                                &iter, text, -1,
                                                tag2, NULL            );
    else
      gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (tb),
                                                &iter, text, -1,
                                                tag1, tag2, NULL   );

    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER (tb), &iter);
    if (end_line != NULL)
      *end_line = gtk_text_iter_get_line(&iter);
}


//!
//! @brief Performs initializations absolutely necessary before a search can take place
//!
//! Correctly the pointer in the GwSearchItem to the correct textbuffer and moves marks
//!
//! @param item A GwSearchItem to gleam information from
//!
void gw_ui_initialize_buffer_by_searchitem (GwSearchItem *item)
{
    //Make sure searches done from the history are pointing at a valid target
    item->target_tb = (gpointer) gw_common_get_gobject_by_target (item->target);
    item->target_tv = (gpointer) gw_common_get_widget_by_target (item->target);

    if (item->target_tb == NULL || item->target_tv == NULL) return;

    gtk_text_buffer_set_text (GTK_TEXT_BUFFER (item->target_tb), "", -1);

    if (item->target == GW_TARGET_RESULTS)
    {

      //Assertain the target text buffer
      GObject *tb = G_OBJECT (item->target_tb);

      //Clear the target text buffer
      GtkTextIter iter;
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter);
      gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (tb), &iter, "\n", -1, "small", NULL);

      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter);
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "more_relevant_header_mark", &iter, TRUE);
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (tb), &iter, "\n\n", -1);

      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter);
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (tb), &iter, "\n", -1);
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter); gtk_text_iter_backward_line (&iter);
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "more_rel_content_insertion_mark", &iter, FALSE);
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "content_insertion_mark", &iter, FALSE);

      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter);
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "less_relevant_header_mark", &iter, TRUE);
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (tb), &iter, "\n\n", -1);

      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter);
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (tb), &iter, "\n", -1);
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter); gtk_text_iter_backward_line (&iter);
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "less_rel_content_insertion_mark", &iter, FALSE);

      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter);
      gtk_text_buffer_insert (GTK_TEXT_BUFFER (tb), &iter, "\n\n\n", -1);
      gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &iter);
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "footer_insertion_mark", &iter, FALSE);

      gw_ui_set_total_results_label_by_searchitem (item);
    }
}


//!
//! @brief Inserts text into the search entry
//!
//! @param text The text to insert
//!
void gw_ui_search_entry_insert (char* text)
{
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    glong length;
    length = strlen (text);

    gint start, end;
    gtk_editable_get_selection_bounds (GTK_EDITABLE (search_entry), &start, &end);
    gtk_editable_delete_text (GTK_EDITABLE (search_entry), start, end);

    gtk_editable_insert_text(GTK_EDITABLE (search_entry), text, length, &start);
    gtk_editable_set_position (GTK_EDITABLE (search_entry), start);
}


//!
//! @brief Sets the focus to a specific target widget
//! 
//! @param TARGET A GwTargetOutput specifying a specific target
//!
void gw_ui_grab_focus_by_target (GwTargetOutput TARGET)
{
    GtkWidget* widget;
    widget = gw_common_get_widget_by_target(TARGET);
    gtk_widget_grab_focus(widget);
}


//!
//! @brief Clears the search entry
//!
void gw_ui_clear_search_entry ()
{
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    gint start, end;
    gtk_editable_select_region (GTK_EDITABLE (search_entry), 0, -1);
    gtk_editable_get_selection_bounds (GTK_EDITABLE (search_entry), &start, &end);
    gtk_editable_delete_text (GTK_EDITABLE (search_entry), start, end);
}


//!
//! @brief Copies the text held by a text type widget using a strncpy like structure
//!
//! @param output The char pointer to copy the string to
//! @param TARGET The widget to copy the text from identified by a target
//! @param MAX The Max characters to copy
//!
void gw_ui_strncpy_text_from_widget_by_target (char* output, GwTargetOutput TARGET, int MAX)
{
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    //GtkEntry
    if (TARGET == GW_TARGET_ENTRY)
    {
      GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);
      strncpy(output, gtk_entry_get_text (GTK_ENTRY (search_entry)), MAX);
    }
  /*
    //GtkTextView
    else if (TARGET = GW_TARGET_RESULTS | TARGET = GW_TARGET_KANJI)
    {
      GObject *tb;
      switch (TARGET)
      {
        case GW_TARGET_RESULTS:
          tb = results_tb;
          break;
      }
      GtkTextIter start, end;

      gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER (tb), &start);
      gtk_text_buffer_get_end_iter( GTK_TEXT_BUFFER (tb), &end);
      gtk_text_buffer_select_range (GTK_TEXT_BUFFER (tb), &start, &end);
    }
  */
}


//!
//! @brief Selects all text in a target
//!
//! @param TARGET The widget where to select all text
//!
void gw_ui_text_select_all_by_target (GwTargetOutput TARGET)
{
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    //GtkEntry
    if (TARGET == GW_TARGET_ENTRY)
    {
      gtk_editable_select_region (GTK_EDITABLE (search_entry), 0,-1);
    }

    //GtkTextView
    else if (TARGET == GW_TARGET_RESULTS ||
             TARGET == GW_TARGET_KANJI     )
    {
      //Assertain the target text buffer
      GObject *tb;
      tb = gw_common_get_gobject_by_target (TARGET);

      GtkTextIter start, end;
      gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER (tb), &start);
      gtk_text_buffer_get_end_iter( GTK_TEXT_BUFFER (tb), &end);

      gtk_text_buffer_select_range (GTK_TEXT_BUFFER (tb), &start, &end);
    }
}


//!
//! @brief Deselects all text in a target
//!
//! @param TARGET The widget where to deselect all text
//!
void gw_ui_text_select_none_by_target (GwTargetOutput TARGET)
{
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    //GtkEntry
    if (TARGET == GW_TARGET_ENTRY)
    {
      gtk_editable_select_region (GTK_EDITABLE (search_entry), -1,-1);
    }

    //GtkTextView
    else if (TARGET == GW_TARGET_RESULTS ||
             TARGET == GW_TARGET_KANJI     )
    {
      //Assertain the target text buffer
      GObject *tb;
      tb = gw_common_get_gobject_by_target (TARGET);

      GtkTextIter start, end;
      gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER (tb), &start);
      gtk_text_buffer_get_end_iter( GTK_TEXT_BUFFER (tb), &end);

      gtk_text_buffer_select_range (GTK_TEXT_BUFFER (tb), &start, &end);
    }
}


//!
//! @brief Returns the target id corresponding to what widget has focus
//!
//! @param window_id The window to check the widgets against 
//!
guint gw_ui_get_current_target_focus (char *window_id)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *window = GTK_WIDGET (gtk_builder_get_object (builder, window_id));
    GtkWidget *widget = GTK_WIDGET (gtk_window_get_focus (GTK_WINDOW (window))); 
    GtkWidget* results_tv = gw_common_get_widget_by_target(GW_TARGET_RESULTS);
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    if (widget == results_tv)
      return GW_TARGET_RESULTS;
    if (widget == search_entry)
      return GW_TARGET_ENTRY;
    else
      return -1;
}


//!
//! @brief Copy Text into the target output
//!
//! TARGET GwTargetOutput to specify where the text should come from
//!
void gw_ui_copy_text (GwTargetOutput TARGET)
{
    GtkClipboard *clipbd;
    GObject *results_tb = gw_common_get_gobject_by_target (GW_TARGET_RESULTS);
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    switch (TARGET)
    {
      case GW_TARGET_ENTRY:
        gtk_editable_copy_clipboard (GTK_EDITABLE (search_entry));
        break;
      case GW_TARGET_RESULTS:
        clipbd = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
        gtk_text_buffer_copy_clipboard (GTK_TEXT_BUFFER (results_tb), clipbd);
        break;
    }
}


//!
//! @brief Cut Text into the target output
//!
//! TARGET GwTargetOutput to specify where the text should come from
//!
void gw_ui_cut_text (GwTargetOutput TARGET)
{
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    switch (TARGET)
    {
      case GW_TARGET_ENTRY:
        gtk_editable_cut_clipboard (GTK_EDITABLE (search_entry));
        break;
    }
}


//!
//! @brief Pastes Text into the target output
//!
//! TARGET GwTargetOutput to specify where the text should go
//!
void gw_ui_paste_text (GwTargetOutput TARGET)
{
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);

    switch (TARGET)
    {
      case GW_TARGET_ENTRY:
        gtk_editable_paste_clipboard (GTK_EDITABLE (search_entry));
        break;
    }
}


//!
//! @brief Sets or updates an existing tag to the buffer
//!
//! @param id String representing the name of the tag
//! @param A constant int representing the target buffer
//! @param set_fg Boolean whether to set the foreground color or not
//! @param set_bg Boolean whether to set the background color or not
//!
gboolean gw_ui_set_color_to_tagtable (char    *id,     GwTargetOutput TARGET,
                                      gboolean set_fg, gboolean set_bg  )
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkTextTag *tag;

    //Yes, we're going to update the colors for all tabs
    GObject *tb;
    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    GtkWidget *textview = NULL;
    GtkWidget *scrolledwindow = NULL;

    int i = 0;
    while ((scrolledwindow = GTK_WIDGET (gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), i))) != NULL)
    {
      textview = GTK_WIDGET (gtk_bin_get_child (GTK_BIN (scrolledwindow)));
      tb = G_OBJECT (gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview)));

      //Load the tag table
      GtkTextTagTable *table;
      table = gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER (tb));

      //Load the set colors in the preferences
      char *key = NULL;
      char fg_color[100];
      char bg_color[100];
      char fallback_color[100];

      GdkColor color;

      //Set the foreground color and reset if the value is odd
      if (set_fg)
      {
        key = g_strdup_printf ("%s-foreground", id);
        if (key != NULL)
        {
          gw_pref_get_string (fg_color, GW_SCHEMA_HIGHLIGHT, key, 100);
          if (gdk_color_parse (fg_color, &color) == FALSE)
          {
            printf("color failed %s\n", fg_color);
            gw_pref_reset_value (GW_SCHEMA_HIGHLIGHT, key);
            g_free (key);
            key = NULL;
            return FALSE;
          }
          g_free (key);
          key = NULL;
        }
      }

      //Set the background color and reset if the value is odd
      if (set_bg)
      {
        key = g_strdup_printf ("%s-background", id);
        if (key != NULL)
        {
          gw_pref_get_string (bg_color, GW_SCHEMA_HIGHLIGHT, key, 100);
          if (gdk_color_parse (bg_color, &color) == FALSE)
          {
            printf("color failed %s\n", bg_color);
            gw_pref_reset_value (GW_SCHEMA_HIGHLIGHT, key);
            g_free (key);
            key = NULL;
            return FALSE;
          }
          g_free (key);
          key = NULL;
        }
      }


      //Insert the new tag into the table
      if ((tag = gtk_text_tag_table_lookup (GTK_TEXT_TAG_TABLE (table), id)) == NULL)
      {
        if (set_fg && set_bg)
          tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (tb), id,
                                            "foreground", fg_color, "background", bg_color, NULL );
        else if (set_fg)
          tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (tb), id,
                                            "foreground", fg_color, NULL               );
        else if (set_bg)
          tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (tb), id,
                                            "background", bg_color, NULL               );
      }
      //Update the tags
      else
      {
        GValue fg_value = {0}, bg_value = {0};
        tag = gtk_text_tag_table_lookup (GTK_TEXT_TAG_TABLE (table), id);
        if (set_fg)
        {
          g_value_init (&fg_value, G_TYPE_STRING);
          g_value_set_string (&fg_value, fg_color);
          g_object_set_property (G_OBJECT (tag), "foreground", &fg_value);
        }
        if (set_bg)
        {
          g_value_init (&bg_value, G_TYPE_STRING);
          g_value_set_string (&bg_value, bg_color);
          g_object_set_property (G_OBJECT (tag), "background", &bg_value);
        }
      }
      i++;
    }
    return TRUE;
}


//!
//! PRIVATE FUNCTION. Sets a single tag to the tagtable of the output buffer
//!
//! @param id The id of the tag
//! @param TARGET The GWTargetOutput of the buffer
//! @param atr The attribute to set
//! @param val the value to set to the attribute
//!
static void gw_ui_set_tag_to_tagtable (char *id,  GwTargetOutput TARGET,
                                       char *atr, gpointer val          )
{
    //Assertain the target text buffer
    GObject *tb;
    tb = gw_common_get_gobject_by_target (TARGET);

    GtkTextTagTable* table = gtk_text_buffer_get_tag_table (GTK_TEXT_BUFFER (tb)); 
    GtkTextTag* tag = gtk_text_tag_table_lookup (table, id);

    if (tag == NULL)
    {
      tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (tb), id, atr, val, NULL);
    }
}


//!
//! @brief Returns the slice of characters between to line numbers in the target output buffer
//!
//! @param TARGET The GwTargetOutput to get the text slice from
//! @param sl The start line number
//! @param el The end line number
//!
char* gw_ui_buffer_get_text_slice_by_target (GwTargetOutput TARGET, int sl, int el)
{
    //Assertain the target text buffer
    GObject *tb;
    tb = gw_common_get_gobject_by_target (TARGET);

    //Set up the text
    GtkTextIter si, ei;
    gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (tb), &si, sl);
    gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (tb), &ei, el);

    return gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (tb), &si, &ei, TRUE);
}


//!
//! @brief Returns the character currently under the cursor in the main results window
//!
//! @param x Pointer to the x coordinate
//! @param y Pointer to the y coordinate
//!
//! @return Returns the character that is being moused over
//!
gunichar gw_ui_get_hovered_character (int *x, int *y, GtkTextIter *start)
{
    gint trailing = 0;
    GtkWidget* results_tv = gw_common_get_widget_by_target (GW_TARGET_RESULTS);

    gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW(results_tv), 
                                           GTK_TEXT_WINDOW_TEXT, 
                                           *x,  *y, x, y             );

    gtk_text_view_get_iter_at_position (GTK_TEXT_VIEW (results_tv),
                                        start, &trailing,
                                        *x, *y                      );

    return gtk_text_iter_get_char (start);
} 


//!
//! @brief A convenience function to set the gdk cursor
//!
//! @param GdkCursorType The prefered cursor to set
//!
void gw_ui_set_cursor (GdkCursorType CURSOR)
{
    GtkWidget* results_tv = gw_common_get_widget_by_target (GW_TARGET_RESULTS);

    GdkWindow* gdk_window;
    gdk_window = gtk_text_view_get_window (GTK_TEXT_VIEW (results_tv), 
                                           GTK_TEXT_WINDOW_TEXT        );
    GdkCursor* cursor;
    cursor = gdk_cursor_new (CURSOR);
    gdk_window_set_cursor (gdk_window, cursor);
    gdk_cursor_unref (cursor);
}


//!
//! @brief Sets the no results page to the output buffer
//!
//! @param item A GwSearchItem pointer to gleam information from
//!
void gw_ui_display_no_results_found_page (GwSearchItem *item)
{
    if (item->status == GW_SEARCH_CANCELING) return; 

    gint32 temp = g_random_int_range (0,9);
    while (temp == _previous_tip)
      temp = g_random_int_range (0,9);
    const gint32 TIP_NUMBER = temp;
    _previous_tip = temp;
    GtkTextView *tv = GTK_TEXT_VIEW (item->target_tv);
    GtkTextBuffer *tb = GTK_TEXT_BUFFER (item->target_tb);
    GtkWidget *image = NULL;
    GtkTextIter iter;
    GtkTextChildAnchor *anchor = NULL;
    GtkWidget *label = NULL;
    GtkWidget *hbox = NULL;
    char *body = NULL;
    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);
    const char *query_text = gtk_entry_get_text (GTK_ENTRY (search_entry));
    GList *list = NULL;
    int i = 0;
    GtkWidget *button = NULL;
    char *markup = NULL;
    list = gw_dictlist_get_selected();
    GwDictInfo *di_selected = list->data;

gdk_threads_enter ();
    //Add the title
    gw_ui_append_to_buffer (item, "\n", "small", NULL, NULL, NULL);


    //Set the header message
    hbox = gtk_hbox_new (FALSE, 10);
    gtk_text_buffer_get_end_iter (tb, &iter);
    anchor = gtk_text_buffer_create_child_anchor (tb, &iter);
    gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (tv), hbox, anchor);
    gtk_widget_show (hbox);

    image = gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_DIALOG);
    gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (image));
    gtk_widget_show (image);

    label = gtk_label_new (NULL);
    char *message = NULL;
    // TRANSLATORS: The argument is the dictionary long name
    message = g_strdup_printf(gettext("Nothing found in the %s!"), di_selected->long_name);
    if (message != NULL)
    {
      markup = g_markup_printf_escaped ("<big><big><b>%s</b></big></big>", message);
      if (markup != NULL)
      {
        gtk_label_set_markup (GTK_LABEL (label), markup);
        gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (label));
        gtk_widget_show (label);
        g_free (markup);
        markup = NULL;
      }
      g_free (message);
      message = NULL;
    }


    //Linebreak after the image
    gw_ui_append_to_buffer (item, "\n\n\n", NULL, NULL, NULL, NULL);


    if (gw_dictlist_get_total_with_status (GW_DICT_STATUS_INSTALLED) > 1)
    {
      //Add label for links
      hbox = gtk_hbox_new (FALSE, 0);
      gtk_text_buffer_get_end_iter (tb, &iter);
      anchor = gtk_text_buffer_create_child_anchor (tb, &iter);
      gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (tv), hbox, anchor);
      gtk_widget_show (hbox);

      label = gtk_label_new (NULL);
      markup = g_markup_printf_escaped ("<b>%s</b>", gettext("Search Other Dictionary: "));
      if (markup != NULL)
      {
        gtk_label_set_markup (GTK_LABEL (label), markup);
        gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (label));
        gtk_widget_show (label);
        g_free (markup);
        markup = NULL;
      }

      //Add internal dictionary links
      i = 0;
      list = gw_dictlist_get_dict_by_load_position (0);
      GwDictInfo *di = list->data;

      char text[MAX_QUERY];
      while ((list = gw_dictlist_get_dict_by_load_position(i)) != NULL)
      {
        di = list->data;
        strncpy(text, query_text, MAX_QUERY);
        if (di != NULL && di != di_selected)
        {
          button = gtk_button_new_with_label (di->short_name);
          g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (do_no_results_search_for_dictionary), di);
          gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (button));
          gtk_widget_show (GTK_WIDGET (button));
        }
        i++;
      }

      gw_ui_append_to_buffer (item, "\n", NULL, NULL, NULL, NULL);
    }

    //Add label for links
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_text_buffer_get_end_iter (tb, &iter);
    anchor = gtk_text_buffer_create_child_anchor (tb, &iter);
    gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (tv), hbox, anchor);
    gtk_widget_show (hbox);

    label = gtk_label_new (NULL);
    markup = g_markup_printf_escaped ("<b>%s</b>", gettext("Search Online: "));
    if (markup != NULL)
    {
      gtk_label_set_markup (GTK_LABEL (label), markup);
      gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (label));
      gtk_widget_show (label);
      g_free (markup);
      markup = NULL;
    }


    //Add links
    char *website_url_menuitems[] = {
      "Google", "http://www.google.com/search?q=%s", "google.png",
      "Goo", "http://dictionary.goo.ne.jp/srch/all/%s/m0u/", "goo.png",
      "Wikipedia", "http://www.wikipedia.org/wiki/%s", "wikipedia.png",
      NULL, NULL, NULL
    };
    i = 0;
    while (website_url_menuitems[i] != NULL)
    {
      //Create handy variables
      char *name = website_url_menuitems[i];
      char *url = g_strdup_printf(website_url_menuitems[i + 1], query_text);
      char *icon_path = website_url_menuitems[i + 2];
      char *path = g_build_filename (DATADIR2, PACKAGE, icon_path, NULL);
      image = NULL;

      //Start creating
      button = gtk_link_button_new_with_label (url, name);
      if (path != NULL)
      {
        image = gtk_image_new_from_file (path);
        //Gtk doesn't use the image anymore by default so we are removing
        //if (image != NULL) gtk_button_set_image (GTK_BUTTON (button), image);
        g_free (path);
        path = NULL;
      }
      gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (button));
      gtk_widget_show (button);
      i += 3;
    }




    gw_ui_append_to_buffer (item, "\n\n\n", NULL, NULL, NULL, NULL);




    //Insert the instruction text
    char *tip_header_str = NULL;
    tip_header_str = g_strdup_printf (gettext("gWaei Usage Tip #%d: "), (TIP_NUMBER + 1));
    if (tip_header_str != NULL)
    {
      gw_ui_append_to_buffer (item, tip_header_str,
                              "important", NULL, NULL, NULL         );
      g_free (tip_header_str);
      tip_header_str = NULL;
    }
                            
    switch (TIP_NUMBER)
    {
      case 0:
        //Tip 1
        body = g_strdup_printf (gettext("Use the Unknown Character from the Insert menu or toolbar in "
                                "place of unknown Kanji. %s will return results like %s.\n\nKanjipad "
                                "is another option for inputting Kanji characters.  Because of how the "
                                "innards of Kanjipad works, drawing with the correct number of strokes "
                                "and drawing the strokes in the correct direction is very important."),
                                "日.語", "日本語");
        gw_ui_append_to_buffer (item,
                                gettext("Inputting Unknown Kanji"),
                                "header", "important", NULL, NULL         );
        gw_ui_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        if (body != NULL)
        {
          gw_ui_append_to_buffer (item, body,
                                  NULL, NULL, NULL, NULL);
          g_free (body);
          body = NULL;
        }
        break;

     case 1:
        //Tip 2
        gw_ui_append_to_buffer (item,
                                gettext("Getting More Exact Matches"),
                                "important", "header", NULL, NULL         );
        gw_ui_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL);
        gw_ui_append_to_buffer (item,
                                gettext("Use the Word-edge Mark and the Not-word-edge Mark from the "
                                "insert menu to get more relevant results.  fish\\b will return results "
                                "like fish and selfish , but not fisherman"),
                                NULL, NULL, NULL, NULL);
        break;

     case 2:
        //Tip 3
        body = g_strdup_printf (gettext("Use the And Character or Or Character to search for "
                                "results that contain a combination of words that might not be "
                                "right next to each other.  cats&dogs will return only results "
                                "that contain both the words cats and dogs like %s does."),
                                "犬猫");
        gw_ui_append_to_buffer (item,
                                gettext("Searching for Multiple Words"),
                                "important", "header", NULL, NULL);
        gw_ui_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL);
        if (body != NULL)
        {
          gw_ui_append_to_buffer (item, body,
                                  NULL, NULL, NULL, NULL);
          g_free (body);
          body = NULL;
        }
        break;

     case 3:
        //Tip 4
        gw_ui_append_to_buffer (item,
                                gettext("Make a Vocabulary List"),
                                "important", "header", NULL, NULL);
        gw_ui_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL);
        gw_ui_append_to_buffer (item,
                                gettext("Specific sections of results can be printed or saved by "
                                "dragging the mouse to highlight them.  Using this in combination "
                                "with the Append command from the File menu or toolbar, quick and "
                                "easy creation of a vocabulary lists is possible."),
                                NULL, NULL, NULL, NULL);
        break;

     case 4:
        //Tip 5
        gw_ui_append_to_buffer (item,
                                gettext("Why Use the Mouse?"),
                                "important", "header", NULL, NULL);
        gw_ui_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_ui_append_to_buffer (item,
                                gettext("Typing something will move the focus to the search input "
                                "box.  Hitting the Up or Down arrow key will move the focus to the "
                                "results pane so you can scroll the results.  Hitting Alt-Up or "
                                "Alt-Down will cycle the currently installed dictionaries."),
                                NULL, NULL, NULL, NULL         );
        break;

     case 5:
        //Tip 6
        gw_ui_append_to_buffer (item,
                                gettext("Get Ready for the JLPT"),
                                "important", "header", NULL, NULL);
        gw_ui_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_ui_append_to_buffer (item,
                                gettext("The Kanji dictionary has some hidden features.  One such "
                                "one is the ability to filter out Kanji that don't meet a certain "
                                "criteria.  If you are planning on taking the Japanese Language "
                                "Proficiency Test, using the phrase J# will filter out Kanji not of "
                                "that level for easy study.  For example, J4 will only show Kanji "
                                "that appears on the forth level test.\n\nAlso of interest, the "
                                "phrase G# will filter out Kanji for the grade level a Japanese "
                                "person would study it at in school."),
                                NULL, NULL, NULL, NULL         );
        break;

     case 6:
        //Tip 7
        gw_ui_append_to_buffer (item,
                                gettext("Just drag words in!"),
                                "important", "header", NULL, NULL);
        gw_ui_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_ui_append_to_buffer (item,
                                gettext("If you drag and drop a highlighted word into gWaei's "
                                "search result box, gWaei will automatically start a search "
                                "using that text.  This can be a nice way to quickly look up words "
                                "while browsing webpages. "),
                                NULL, NULL, NULL, NULL         );

        break;

     case 7:
        //Tip 8
        gw_ui_append_to_buffer (item,
                                gettext("What does (adj-i) mean?"),
                                "important", "header", NULL, NULL);
        gw_ui_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_ui_append_to_buffer (item,
                                gettext("It is part of the terminalogy used by the EDICT group of "
                                "dictionaries to categorize words.  Some are obvious, but there are "
                                "a number that there is no way to know the meaning other than by looking "
                                "it up.\n\ngWaei includes some of the EDICT documentation in its help "
                                "manual.  Click the Dictionary Terminology Glossary menuitem in the "
                                "Help menu to get to it."),
                                NULL, NULL, NULL, NULL         );
        break;

     case 8:
        //Tip 9
        gw_ui_append_to_buffer (item,
                                gettext("Books are Heavy"),
                                "important", "header", NULL, NULL);
        gw_ui_append_to_buffer (item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_ui_append_to_buffer (item,
                                gettext("Aways wear a construction helmet when working with books.  "
                                "They are dangerous heavy objects that can at any point fall on and "
                                "injure you.  Please all urge all of your friends to, too.  They will "
                                "thank you later.  Really."),
                                NULL, NULL, NULL, NULL         );
       break;
    }

    gw_ui_append_to_buffer (item,
                               "\n\n",
                               NULL, NULL, NULL, NULL         );
gdk_threads_leave ();
}


//!
//! @brief Cycles the dictionaries forward or backward, looping when it reaches the end
//!
//! @param cycle_forward A boolean to choose the cycle direction
//!
void gw_ui_cycle_dictionaries (gboolean cycle_forward)
{
    GtkBuilder *builder = gw_common_get_builder ();

    int increment;

    if (cycle_forward)
      increment = 1;
    else
      increment = -1;

    //Declarations
    GtkWidget *combobox;
    combobox = GTK_WIDGET (gtk_builder_get_object(builder, "dictionary_combobox"));

    gint active;
    active = gtk_combo_box_get_active (GTK_COMBO_BOX (combobox));
    GtkTreeIter iter;
    gboolean set = FALSE;

    if ((active = active + increment) == -1)
    {
      do {
        active++;
        gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), active);
        set = gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter);
      } while (set);
      active--;
      gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), active);
    }
    else
    {
      gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), active);
      set = gtk_combo_box_get_active_iter (GTK_COMBO_BOX (combobox), &iter);
      if (!set)
        gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);
    }
}


//!
//! @brief  Returns the unfreeable text from a gtk widget by target
//!
//! The fancy thing about this function is it will only return the
//! highlighted text if some is highlighted.
//!
//! @param TARGET a GwTargetOutput to get the data from
//!
char* gw_ui_buffer_get_text_by_target (GwTargetOutput TARGET)
{
    GObject* tb;
    tb = gw_common_get_gobject_by_target (TARGET);

    GtkTextIter s, e;
    if (gtk_text_buffer_get_has_selection (GTK_TEXT_BUFFER (tb)))
    {
      gtk_text_buffer_get_selection_bounds(GTK_TEXT_BUFFER (tb), &s, &e);
    }
    //Get the region of text to be saved if no text is highlighted
    else
    {
      gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER (tb), &s);
      gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER (tb), &e);
    }
    return gtk_text_buffer_get_text(GTK_TEXT_BUFFER (tb), &s, &e, FALSE);
}


//!
//! @brief Resets the color tags according to the preferences
//!
void gw_ui_buffer_reload_tagtable_tags ()
{
    gw_ui_set_color_to_tagtable ("comment", GW_TARGET_RESULTS, TRUE, FALSE);
    gw_ui_set_color_to_tagtable ("comment", GW_TARGET_KANJI,   TRUE, FALSE);

    gw_ui_set_color_to_tagtable ("match",   GW_TARGET_RESULTS, TRUE, TRUE );
    gw_ui_set_color_to_tagtable ("match",   GW_TARGET_KANJI,   FALSE,FALSE);

    gw_ui_set_color_to_tagtable ("header",  GW_TARGET_RESULTS, TRUE, FALSE);
    gw_ui_set_color_to_tagtable ("header",  GW_TARGET_KANJI,   TRUE, FALSE);

    GtkWidget *search_entry = gw_common_get_widget_by_target (GW_TARGET_ENTRY);
    gtk_widget_modify_base (GTK_WIDGET (search_entry), GTK_STATE_NORMAL, NULL);
    gtk_widget_modify_text (GTK_WIDGET (search_entry), GTK_STATE_NORMAL, NULL);
}


//!
//! @brief Adds the tags to stylize the buffer text
//!
void gw_ui_buffer_initialize_tags ()
{
    gw_ui_set_tag_to_tagtable ("italic", GW_TARGET_RESULTS,
                                  "style", GINT_TO_POINTER(PANGO_STYLE_ITALIC));
    gw_ui_set_tag_to_tagtable ("gray", GW_TARGET_RESULTS,
                                  "foreground",    "#888888");
    gw_ui_set_tag_to_tagtable ("smaller", GW_TARGET_RESULTS,
                                  "size",    "smaller");
    //Important tag (usually bold)
    gw_ui_set_tag_to_tagtable ("important", GW_TARGET_RESULTS,
                                  "weight",    GINT_TO_POINTER(PANGO_WEIGHT_BOLD));

    //Larger tag
    gw_ui_set_tag_to_tagtable ("larger", GW_TARGET_RESULTS, "font", "sans 20");

    //Large tag
    gw_ui_set_tag_to_tagtable ("large", GW_TARGET_RESULTS, "font", "serif 40");

    gw_ui_set_tag_to_tagtable ("center", GW_TARGET_RESULTS, "justification", GINT_TO_POINTER(GTK_JUSTIFY_LEFT));

    //Small tag
    gw_ui_set_tag_to_tagtable ("small", GW_TARGET_RESULTS,  "font", "serif 6");

    gw_ui_buffer_reload_tagtable_tags();
}


//!
//! @brief Creates the initial marks needed for the text buffer
//!
//! tb gpointer to the textbuffer to add the marks to
//!
void gw_ui_buffer_initialize_marks (gpointer tb)
{
    GtkTextIter iter;

    gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (tb), &iter);
    gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "more_relevant_header_mark", &iter, TRUE);
    gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "less_relevant_header_mark", &iter, TRUE);
    gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "less_rel_content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "more_rel_content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (tb), "footer_insertion_mark", &iter, FALSE);
}


//!
//! @brief PRIVATE FUNCTION. A Stes the text of the desired mark.
//!
//! @param item A GwSearchItem to gleam information from
//! @param text The desired text to set to the mark
//! @param mark_name The name of the mark to set the new attributes to
//!
//!
static void _set_header (GwSearchItem *item, char* text, char* mark_name)
{
  gdk_threads_enter();
    GObject *results_tb = G_OBJECT (item->target_tb);

    GtkTextIter iter;
    GtkTextMark *mark;
    gint line;

    mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (results_tb), mark_name);
    gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (results_tb), &iter, mark);
    line = gtk_text_iter_get_line (&iter);

    //Move the insertion header to the less relevenant section
    if (strcmp(mark_name, "less_relevant_header_mark") == 0)
    {
      GtkTextMark *target_mark;
      GtkTextIter iter;
      target_mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (results_tb), "less_rel_content_insertion_mark");
      gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (results_tb), &iter, target_mark);
      gtk_text_buffer_move_mark_by_name (GTK_TEXT_BUFFER (results_tb), "content_insertion_mark", &iter);
    }

    //Update the header text
    char *new_text = g_strdup_printf ("%s\n", text);
    if (new_text != NULL)
    {
      GtkTextIter end_iter;
      gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER (results_tb), &end_iter, line + 1);
      gtk_text_buffer_delete (GTK_TEXT_BUFFER (results_tb), &iter, &end_iter);
      gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (results_tb), &iter, mark);
      gtk_text_buffer_insert_with_tags_by_name (GTK_TEXT_BUFFER (results_tb), &iter, new_text, -1, "header", "important", NULL);
      g_free (new_text);
      new_text = NULL;
    }
  gdk_threads_leave();
}


//!
//! @brief A simple search initiater function made to be looped by a timer
//!
//! @param data An unused gpointer.  It should always be NULL
//!
gboolean gw_ui_keep_searching (gpointer data)
{
    do_search (NULL, NULL);
    return TRUE;
}


//!
//! @brief Finds out if some text is selected and updates the buttons accordingly
//!
//! When text is found selected, some buttons become sensitive and some have the
//! label change.  This tells the user they can save/print sections of the
//! results.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
gboolean gw_update_icons_for_selection (gpointer data) 
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkAction *action = NULL;
    gboolean has_selection = gw_ui_has_selection_by_target (GW_TARGET_RESULTS);

    //Set the special buttons
    if (!_prev_selection_icon_state && has_selection)
    {
      _prev_selection_icon_state = TRUE;
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_append_action"));
      gtk_action_set_label (action, gettext("A_ppend Selected"));
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_save_as_action"));
      gtk_action_set_label (action, gettext("Save Selected _As"));
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_print_action"));
      gtk_action_set_label (action, gettext("_Print Selected"));
    }
    //Reset the buttons to their normal states
    else if (_prev_selection_icon_state == TRUE && !has_selection)
    {
      _prev_selection_icon_state = FALSE;
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_append_action"));
      gtk_action_set_label (action, gettext("A_ppend"));
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_save_as_action"));
      gtk_action_set_label (action, NULL);
      action = GTK_ACTION (gtk_builder_get_object (builder, "file_print_action"));
      gtk_action_set_label (action, NULL);
    }
    gw_ui_update_toolbar_buttons();
    return TRUE; 
}




//!
//! @brief Uses a searchitem to cancel a search
//!
//! @param item A GwSearchItem to gleam information from
//!
gboolean gw_ui_cancel_search_by_searchitem (GwSearchItem *item)
{
    if (item == NULL) return TRUE;
    
    g_mutex_lock (item->mutex);

      //Sanity check 1
      if (item->status == GW_SEARCH_IDLE) 
      {
        g_mutex_unlock (item->mutex);
        return TRUE;
      }

      //Sanity check 2
      if(item != NULL && item->status == GW_SEARCH_CANCELING) 
      {
        g_mutex_unlock (item->mutex);
        return FALSE;
      }

      //Sanity check 3
      if (item->thread == NULL)
      {
        item->status = GW_SEARCH_IDLE;
        g_mutex_unlock (item->mutex);
        return FALSE;
      }

      //Do the cancel operation
      item->status = GW_SEARCH_CANCELING;

    g_mutex_unlock (item->mutex);

    g_thread_join(item->thread);
    item->thread = NULL;

    return FALSE;
}


//!
//! @brief Cancels a search by identifying matching gpointer
//!
//! @param container A pointer to the top-most widget in the desired tab to cancel the search of.
//!
gboolean gw_ui_cancel_search_by_tab_content (gpointer container)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    int position = gtk_notebook_page_num (GTK_NOTEBOOK (notebook), container);
    if (position == -1) return FALSE;

    GList *list = gw_tabs_get_searchitem_list ();
    GwSearchItem *item = g_list_nth_data (list, position);
    if (item == NULL) return TRUE;

    gboolean result = gw_ui_cancel_search_by_searchitem (item);
    return result;
}


//!
//! @brief Cancels all searches in all currently open tabs
//!
void gw_ui_tab_cancel_all_searches ()
{
    GList *list = gw_tabs_get_searchitem_list ();
    GwSearchItem *item = NULL;
    while (list != NULL)
    {
      item = (GwSearchItem*) list->data;
      gw_ui_cancel_search_by_searchitem (item);
      list = list->next;
    }
}


//!
//! @brief Cancels the search of the tab number
//!
//! @param page_num The page number of the tab to cancel the search of
//!
gboolean gw_ui_cancel_search_by_tab_number (const int page_num)
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    GtkWidget *content = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num);
    if (content == NULL) return TRUE;

    gboolean result = gw_ui_cancel_search_by_tab_content (content);
    return result;
}


//!
//! @brief Cancels the search of the currently visibile tab
//!
gboolean gw_ui_cancel_search_for_current_tab ()
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *notebook = GTK_WIDGET (gtk_builder_get_object (builder, "notebook"));
    int page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (notebook));
    gboolean result = gw_ui_cancel_search_by_tab_number (page_num);
    return result;
}


//!
//! @brief Cancels a search using a GwTargetOutput as identification
//!
gboolean gw_ui_cancel_search_by_target (GwTargetOutput TARGET)
{
    if (TARGET == GW_TARGET_KANJI)
    {
      GwHistoryList* hl = gw_historylist_get_list(GW_HISTORYLIST_RESULTS);
      GwSearchItem *item = hl->current;
      return  gw_ui_cancel_search_by_searchitem (item);
    }
}


//!
//! @brief Abstraction function to find out if some text is selected
//!
//! It gets the requested text buffer and then returns if it has text selected
//! or not.
//!
//! @param TARGET A GwTargetOutput
//!
gboolean gw_ui_has_selection_by_target (GwTargetOutput TARGET)
{
    GObject* tb = gw_common_get_gobject_by_target (TARGET);
    return (tb != NULL && gtk_text_buffer_get_has_selection (GTK_TEXT_BUFFER (tb)));
}


//!
//! @brief PRIVATE FUNCTION. Applies a tag to a section of text
//!
//! @param line An integer showing the line in the buffer to tag
//! @param start_offset the ending character in the line to highlight
//! @param end_offset The ending character in the line to highlight
//! @param item A GwSearchItem to get general information from
//!
static void _add_match_highlights (gint line, gint start_offset, gint end_offset, GwSearchItem* item)
{
    GtkTextBuffer *tb;
    tb = GTK_TEXT_BUFFER (item->target_tb);
    GwQueryLine *ql = item->queryline;
    
    int i;
    int match_so, match_eo;
    GtkTextIter si, ei;
    gtk_text_buffer_get_iter_at_line_offset (tb, &si, line, start_offset);
    gtk_text_buffer_get_iter_at_line_offset (tb, &ei, line, end_offset);
    char *text = gtk_text_buffer_get_slice (tb, &si, &ei, FALSE);
    char *pos = text;

    //Look for kanji atoms
    for(i = 0; i < ql->kanji_total; i++) {
       pos = text;
       while ((pos = gw_regex_locate_offset (pos, text, &(ql->kanji_regex[GW_QUERYLINE_LOCATE][i]), &match_so, &match_eo)) != NULL )
       {
          gtk_text_buffer_get_iter_at_line_offset (tb, &si, line, match_so + start_offset);
          gtk_text_buffer_get_iter_at_line_offset (tb, &ei, line, match_eo + start_offset);
          gtk_text_buffer_apply_tag_by_name (tb, "match", &si, &ei);
       }
    }
    //Look for furigana atoms
    for(i = 0; i < ql->furi_total; i++) {
       pos = text;
       while ((pos = gw_regex_locate_offset (pos, text, &(ql->furi_regex[GW_QUERYLINE_LOCATE][i]), &match_so, &match_eo)) != NULL )
       {
          gtk_text_buffer_get_iter_at_line_offset (tb, &si, line, match_so + start_offset);
          gtk_text_buffer_get_iter_at_line_offset (tb, &ei, line, match_eo + start_offset);
          gtk_text_buffer_apply_tag_by_name (tb, "match", &si, &ei);
       }
    }
    //Look for romaji atoms
    for(i = 0; i < ql->roma_total; i++) {
       pos = text;
       while ((pos = gw_regex_locate_offset (pos, text, &(ql->roma_regex[GW_QUERYLINE_LOCATE][i]), &match_so, &match_eo)) != NULL )
       {
          gtk_text_buffer_get_iter_at_line_offset (tb, &si, line, match_so + start_offset);
          gtk_text_buffer_get_iter_at_line_offset (tb, &ei, line, match_eo + start_offset);
          gtk_text_buffer_apply_tag_by_name (tb, "match", &si, &ei);
       }
    }
    g_free (text);
}


//!
//! @brief PRIVATE FUNCTION. Moves the content insertion mark to another mark's spot
//!
//! @param item A GwSearchItem pointer to gleam information from
//! @param name The name of the mark to move the content insertion mark to
//!
static void _shift_stay_mark (GwSearchItem *item, char *name)
{
    GObject *results_tb;
    results_tb = G_OBJECT (item->target_tb);

    GtkTextMark *mark;
    GtkTextIter iter;
    mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (results_tb), "content_insertion_mark");
    gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (results_tb), &iter, mark);
    if ((mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (results_tb), name)) == NULL)
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (results_tb), name, &iter, TRUE);
    else
      gtk_text_buffer_move_mark (GTK_TEXT_BUFFER (results_tb), mark, &iter);
}


//!
//! @brief PRIVATE FUNCTION.  Updates the position of a mark to accomidate new results.
//!
//! @param item A GwSearchItem to gleam information from.
//! @param stay_name The name of the mark that stays in place before the new result.
//! @param append_name The name of the mark that moves to the end after the new result is added.
//!
static void _shift_append_mark (GwSearchItem *item, char *stay_name, char *append_name)
{
    GObject *results_tb;
    results_tb = G_OBJECT (item->target_tb);

    GtkTextIter iter;
    GtkTextMark *stay_mark, *append_mark;

    stay_mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (results_tb), stay_name);
    gtk_text_buffer_get_iter_at_mark (GTK_TEXT_BUFFER (results_tb), &iter, stay_mark);

    if ((append_mark = gtk_text_buffer_get_mark (GTK_TEXT_BUFFER (results_tb), append_name)) == NULL)
      gtk_text_buffer_create_mark (GTK_TEXT_BUFFER (results_tb), append_name, &iter, FALSE);
    else
      gtk_text_buffer_move_mark (GTK_TEXT_BUFFER (results_tb), append_mark, &iter);
}


//!
//! @brief PRIVATE FUNCTION.  When adding a result to the buffer, it just adds the the kanji/hiragana section
//!
//! This function is made to help readability of edict results since there is a lot of repeating.
//!
//! @param item A GwSearchItem pointer to use for data.
//!
static void _append_def_same_to_buffer (GwSearchItem* item)
{
    GwResultLine* resultline = item->resultline;

    GtkTextBuffer *tb = GTK_TEXT_BUFFER (item->target_tb);

    _shift_append_mark (item, "previous_result", "new_result");
    GtkTextMark *mark;
    if ((mark = gtk_text_buffer_get_mark (tb, "previous_result")) != NULL)
    {
      GtkTextIter iter;
      int line, start_offset, end_offset;
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark);
      line = gtk_text_iter_get_line (&iter);
      start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, " /", -1, "important", NULL);
      gtk_text_buffer_insert (tb, &iter, " ", -1);
      //Kanji
      if (resultline->kanji_start != NULL)
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, resultline->kanji_start, -1, "important", NULL);
      //Furigana
      if (resultline->furigana_start != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, " [", -1, "important", NULL);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, resultline->furigana_start, -1, "important", NULL);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, "]", -1, "important", NULL);
      }
      //Other info
      if (resultline->classification_start != NULL)
      {
        gtk_text_buffer_insert                   (tb, &iter, " ", -1);
        GtkTextIter copy = iter;
        gtk_text_iter_backward_char (&copy);
        gtk_text_buffer_remove_tag_by_name (tb, "important", &copy, &iter);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, resultline->classification_start, -1, "gray", "italic", NULL);
      }
      if (resultline->important == TRUE)
      {
        gtk_text_buffer_insert                   (tb, &iter, " ", -1);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Pop"), -1, "small", NULL);
      }
      end_offset = gtk_text_iter_get_line_offset (&iter);
      _add_match_highlights (line, start_offset, end_offset, item);
    }
}


//!
//! @brief Appends an edict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param item A GwSearchItem to gleam information from.
//!
void gw_ui_append_edict_results_to_buffer (GwSearchItem *item)
{
  gdk_threads_enter();
    //Some checks
    gboolean furigana_exists, kanji_exists;
    gboolean same_def_totals, same_first_def, same_furigana, same_kanji, skip;
    kanji_exists = (item->resultline->kanji_start != NULL && item->backup_resultline->kanji_start != NULL);
    furigana_exists = (item->resultline->furigana_start != NULL && item->backup_resultline->furigana_start != NULL);
    if (item->resultline->kanji_start == NULL || item->backup_resultline->kanji_start == NULL)
    {
      skip = TRUE;
    }
    else
    {
      same_def_totals = (item->resultline->def_total == item->backup_resultline->def_total);
      same_first_def = (strcmp(item->resultline->def_start[0], item->backup_resultline->def_start[0]) == 0);
      same_furigana = (!furigana_exists ||strcmp(item->resultline->furigana_start, item->backup_resultline->furigana_start) == 0);
      same_kanji = (!kanji_exists || strcmp(item->resultline->kanji_start, item->backup_resultline->kanji_start) == 0);
      skip = FALSE;
    }

    //Begin comparison if possible
    if (!skip && ((same_def_totals) || (same_kanji && same_furigana)) && same_first_def)
    {
      _append_def_same_to_buffer (item);
      gdk_threads_leave ();
      return;
    }

    gboolean remove_last_linebreak = (!skip && same_kanji && same_first_def);

    //Start output
    GwResultLine* rl = item->resultline;

    GtkTextBuffer *tb = GTK_TEXT_BUFFER (item->target_tb);
    GtkTextMark *mark;
    mark = gtk_text_buffer_get_mark (tb, "content_insertion_mark");

    if (remove_last_linebreak)
    {
      GtkTextIter si, ei;
      GtkTextMark *temp_mark;
      if ((temp_mark = gtk_text_buffer_get_mark (tb, "previous_result")) && gtk_text_buffer_get_mark (tb, "note_mark") == NULL)
      {
        gtk_text_buffer_get_iter_at_mark (tb, &si, temp_mark);
        gtk_text_buffer_create_mark (tb, "note_mark", &si, TRUE);
        gtk_text_buffer_get_iter_at_line (tb, &ei, gtk_text_iter_get_line (&si) + 1);
        gtk_text_buffer_delete (tb, &si, &ei);
      }
      gtk_text_buffer_get_iter_at_mark (tb, &ei, mark);
      gtk_text_buffer_get_iter_at_line (tb, &si, gtk_text_iter_get_line (&ei) - 1);
      gtk_text_buffer_delete(tb, &si, &ei);
    }
    else
    {
      GtkTextMark *temp_mark;
      if (temp_mark = gtk_text_buffer_get_mark (tb, "note_mark"))
         gtk_text_buffer_delete_mark (tb, temp_mark);
    }

    int line, start_offset, end_offset;
    GtkTextIter iter;
    gtk_text_buffer_get_iter_at_mark (tb, &iter, mark);
    line = gtk_text_iter_get_line (&iter);
    //Kanji
    if (rl->kanji_start != NULL)
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, rl->kanji_start, -1, "important", NULL);
    //Furigana
    if (rl->furigana_start != NULL)
    {
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, " [", -1, "important", NULL);
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, rl->furigana_start, -1, "important", NULL);
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, "]", -1, "important", NULL);
    }
    //Other info
    if (rl->classification_start != NULL)
    {
      gtk_text_buffer_insert (tb, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, rl->classification_start, -1, "gray", "italic", NULL);
    }
    if (rl->important == TRUE)
    {
      gtk_text_buffer_insert (tb, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Pop"), -1, "small", NULL);
    }
    _shift_stay_mark (item, "previous_result");
    start_offset = 0;
    end_offset = gtk_text_iter_get_line_offset (&iter);
    if (!remove_last_linebreak) gtk_text_buffer_insert (tb, &iter, "\n", -1);
    _add_match_highlights (line, start_offset, end_offset, item);

    //Definitions
    int i = 0;
    while (rl->def_start[i] != NULL)
    {
      gtk_text_buffer_insert (tb, &iter, "      ", -1);

      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, rl->number[i], -1, "comment", NULL);
      gtk_text_buffer_insert                   (tb, &iter, " ", -1);
      gtk_text_buffer_insert                   (tb, &iter, rl->def_start[i], -1);
      end_offset = gtk_text_iter_get_line_offset (&iter);
      line = gtk_text_iter_get_line (&iter);
      _add_match_highlights (line, start_offset, end_offset, item);
      gtk_text_buffer_insert                   (tb, &iter, "\n", -1);
      i++;
    }
    gtk_text_buffer_insert (tb, &iter, "\n", -1);

  gdk_threads_leave();
}


//!
//! @brief Appends a kanjidict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param item A GwSearchItem to gleam information from.
//!
void gw_ui_append_kanjidict_results_to_buffer (GwSearchItem *item)
{
    GwResultLine* resultline = item->resultline;
    GtkTextBuffer *tb = NULL;
    GtkWidget *tv = NULL;

    if (item->target == GW_TARGET_RESULTS)
    {
  gdk_threads_enter();
      tb = GTK_TEXT_BUFFER (item->target_tb);
      GtkTextIter iter;
      GtkTextMark *mark;
      mark = gtk_text_buffer_get_mark (tb, "content_insertion_mark");
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark);
      int line, start_offset, end_offset;

      //Kanji
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, resultline->kanji, -1, "large", "center", NULL);
      if (item->target == GW_TARGET_RESULTS) gtk_text_buffer_insert_with_tags_by_name (tb, &iter, " ", -1, "large", "center", NULL);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
      if (item->target == GW_TARGET_RESULTS)
        _add_match_highlights (line, start_offset, end_offset, item);

      gtk_text_buffer_insert (tb, &iter, "\n", -1);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);

      //Radicals
      if (resultline->radicals != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Radicals:"), -1, "important", NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert (tb, &iter, resultline->radicals, -1);
        gtk_text_buffer_insert (tb, &iter, " ", -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        if (item->target == GW_TARGET_RESULTS)
          _add_match_highlights (line, start_offset, end_offset, item);

        gtk_text_buffer_insert (tb, &iter, "\n", -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);

        gw_radsearchtool_set_button_sensitive_when_label_is (resultline->radicals);
      }

      //Readings
      if (resultline->readings[0] != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Readings:"), -1, "important", NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert (tb, &iter, resultline->readings[0], -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        if (item->target == GW_TARGET_RESULTS)
          _add_match_highlights (line, start_offset, end_offset, item);
        gtk_text_buffer_insert (tb, &iter, "\n", -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
      }
      if (resultline->readings[1] != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Name:"), -1, "important", NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert (tb, &iter, resultline->readings[1], -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        if (item->target == GW_TARGET_RESULTS)
          _add_match_highlights (line, start_offset, end_offset, item);
        gtk_text_buffer_insert (tb, &iter, "\n", -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
      }
      if (resultline->readings[2] != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Radical Name:"), -1, "important", NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert (tb, &iter, resultline->readings[2], -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        if (item->target == GW_TARGET_RESULTS)
          _add_match_highlights (line, start_offset, end_offset, item);
        gtk_text_buffer_insert (tb, &iter, "\n", -1);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
      }


      //etc
      gboolean line_started = FALSE;
      if (resultline->strokes)
      {
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Stroke:"), -1, "important", NULL);
        gtk_text_buffer_insert (tb, &iter, resultline->strokes, -1);
        line_started = TRUE;
      }
      if (resultline->frequency)
      {
        if (line_started) gtk_text_buffer_insert (tb, &iter, " ", -1);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Freq:"), -1, "important", NULL);
        gtk_text_buffer_insert (tb, &iter, resultline->frequency, -1);
        gtk_text_buffer_insert (tb, &iter, " ", -1);
      }
      if (resultline->grade)
      {
        if (line_started) gtk_text_buffer_insert (tb, &iter, " ", -1);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Grade:"), -1, "important", NULL);
        gtk_text_buffer_insert (tb, &iter, resultline->grade, -1);
      }
      if (resultline->jlpt)
      {
        if (line_started) gtk_text_buffer_insert (tb, &iter, " ", -1);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("JLPT:"), -1, "important", NULL);
        gtk_text_buffer_insert (tb, &iter, resultline->jlpt, -1);
      }

      gtk_text_buffer_insert (tb, &iter, "\n", -1);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);

      //Meanings
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("Meanings:"), -1, "important", NULL);
      gtk_text_buffer_insert (tb, &iter, resultline->meanings, -1);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      if (item->target == GW_TARGET_RESULTS)
        _add_match_highlights (line, start_offset, end_offset, item);

      gtk_text_buffer_insert (tb, &iter, "\n\n", -1);
  gdk_threads_leave ();
    }
    
    if (item->target == GW_TARGET_KANJI && (tv = GTK_WIDGET (gw_common_get_widget_by_target (GW_TARGET_RESULTS))) != NULL)
    {
      char markup[1000];
      markup[0] = '\0';

      if (resultline->radicals) {
        strcat(markup, "\n<b>");
        strcat(markup, gettext("Radicals:"));
        strcat(markup, " ");
        strcat(markup, "</b>");
        strcat(markup, resultline->radicals);
        strcat(markup, " ");
      }
      if (resultline->readings[0]) {
        strcat(markup, "\n<b>");
        strcat(markup, gettext("Readings:"));
        strcat(markup, " ");
        strcat(markup, "</b>");
        strcat(markup, resultline->readings[0]);
        strcat(markup, " ");
      } 
      if (resultline->readings[1]) {
        strcat(markup, "\n<b>");
        strcat(markup, gettext("Name:"));
        strcat(markup, " ");
        strcat(markup, "</b>");
        strcat(markup, resultline->readings[1]);
        strcat(markup, " ");
      }
      if (resultline->readings[2]) {
        strcat(markup, "\n<b>");
        strcat(markup, gettext("Radical Name:"));
        strcat(markup, " ");
        strcat(markup, "</b>");
        strcat(markup, resultline->readings[2]);
        strcat(markup, " ");
      }
      strcat(markup, "\n");
      if (resultline->strokes) {
        strcat(markup, "<b>");
        strcat(markup, gettext("Stroke:"));
        strcat(markup, "</b> ");
        strcat(markup, resultline->strokes);
        strcat(markup, " ");
      }
      if (resultline->frequency) {
        strcat(markup, "<b>");
        strcat(markup, gettext("Freq:"));
        strcat(markup, "</b> ");
        strcat(markup, resultline->frequency);
        strcat(markup, " ");
      }
      if (resultline->grade) {
        strcat(markup, "<b>");
        strcat(markup, gettext("Grade:"));
        strcat(markup, "</b> ");
        strcat(markup, resultline->grade);
        strcat(markup, " ");
      }
      if (resultline->jlpt) {
        strcat(markup, "<b>");
        strcat(markup, gettext("JLPT:"));
        strcat(markup, "</b> ");
        strcat(markup, resultline->jlpt);
        strcat(markup, " ");
      }
      if (resultline->meanings) {
        strcat(markup, "\n<b>");
        strcat(markup, gettext("Meanings:"));
        strcat(markup, "</b> ");
        strcat(markup, resultline->meanings);
        strcat(markup, " ");
      }

      char markup2[1000];
      markup2[0] = '\0';

      strcat(markup2, "<span font=\"KanjiStrokeOrders 80\">");
      strcat(markup2, resultline->kanji);
      strcat(markup2, "</span>");


      int x, y, width, height;
      GtkWidget *window = GTK_WIDGET (gtk_widget_get_tooltip_window (tv));
      if (window != NULL) {
        GtkWidget *hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 3));
        gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (hbox));

        GtkWidget *label = GTK_WIDGET (gtk_label_new (NULL));
        gtk_label_set_markup (GTK_LABEL (label), markup2);
        gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);

        label = GTK_WIDGET (gtk_label_new (NULL));
        gtk_label_set_markup (GTK_LABEL (label), markup);
        gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, FALSE, 0);
      }
    }
}


//!
//! @brief Appends a examplesdict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param item A GwSearchItem to gleam information from.
//!
void gw_ui_append_examplesdict_results_to_buffer (GwSearchItem *item)
{
  gdk_threads_enter();
      GwResultLine* resultline = item->resultline;
      GtkTextBuffer *tb = GTK_TEXT_BUFFER (item->target_tb);

      int line, start_offset, end_offset;
      GtkTextMark *mark;
      mark = gtk_text_buffer_get_mark (tb, "content_insertion_mark");
      GtkTextIter iter;
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark);

      if (resultline->def_start[0] != NULL)
      {
        // TRANSLATORS: The "E" stands for "English"
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("E:\t"), -1, "important", "comment", NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, resultline->def_start[0], -1, "important", NULL, NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
        _add_match_highlights (line, start_offset, end_offset, item);
      }

      if (resultline->kanji_start != NULL)
      {
        // TRANSLATORS: The "J" stands for "Japanese"
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("\nJ:\t"), -1, "important", "comment", NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, resultline->kanji_start, -1, NULL, NULL, NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
        _add_match_highlights (line, start_offset, end_offset, item);
      }

      if (resultline->furigana_start != NULL)
      {
        // TRANSLATORS: The "D" stands for "Detail"
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, gettext("\nD:\t"), -1, "important", "comment", NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert_with_tags_by_name (tb, &iter, resultline->furigana_start, -1, NULL, NULL, NULL);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
        _add_match_highlights (line, start_offset, end_offset, item);
      }

      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark);
      gtk_text_buffer_insert (tb, &iter, "\n\n", -1);
  gdk_threads_leave();
}


//!
//! @brief Appends a examplesdict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly.  This is the fallback safe function for unknown dictionaries.
//!
//! @param item A GwSearchItem to gleam information from.
//!
void gw_ui_append_unknowndict_results_to_buffer (GwSearchItem *item)
{
  gdk_threads_enter();
      GwResultLine* resultline = item->resultline;
      GtkTextBuffer *tb = GTK_TEXT_BUFFER (item->target_tb);
      GtkTextIter iter;
      GtkTextMark *mark;
      mark = gtk_text_buffer_get_mark (tb, "content_insertion_mark");

      int line, start_offset, end_offset;

      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert (tb, &iter, resultline->string, -1);
      if (item->target == GW_TARGET_RESULTS) gtk_text_buffer_insert (tb, &iter, " ", -1);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
      _add_match_highlights (line, start_offset, end_offset, item);

      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark);
      gtk_text_buffer_insert (tb, &iter, "\n\n", -1);
      gtk_text_buffer_get_iter_at_mark (tb, &iter, mark); line = gtk_text_iter_get_line (&iter);
  gdk_threads_leave();
}


//!
//! @brief Sets the no results page to the buffer
//!
//! What is special about this no results page is it also gives usage tips.
//!
//! @param item A GwSearchItem pointer to gleam information from.
//!
void gw_ui_no_result (GwSearchItem *item)
{
  gdk_threads_enter ();
    gtk_text_buffer_set_text (GTK_TEXT_BUFFER (item->target_tb), "", -1);
  gdk_threads_leave ();
    gw_ui_display_no_results_found_page (item);
}


//!
//! @brief Add an header to irrelevant "other" results with number of matches
//!
void gw_ui_append_less_relevant_header_to_output (GwSearchItem *item)
{
    int irrelevant = item->total_irrelevant_results;
    char *message = g_strdup_printf (ngettext("Other Result %d", "Other Results %d", irrelevant), irrelevant);
    if (message != NULL)
    {
      _set_header (item, message, "less_relevant_header_mark");
      g_free (message);
    }
}


//!
//! @brief Add an header to relevant "main" results with number of matches
//!
void gw_ui_append_more_relevant_header_to_output (GwSearchItem *item)
{
    int relevant = item->total_relevant_results;
    char *message = g_strdup_printf (ngettext("Main Result %d", "Main Results %d", relevant), relevant);
    if (message != NULL)
    {
      _set_header (item, message, "more_relevant_header_mark");
      g_free (message);
    }
}


//!
//! @brief Sets up the interface before each search begins
//!
//! @param item A GwSearchItem pointer to get information from
//!
void gw_ui_pre_search_prep (GwSearchItem *item)
{
    gw_ui_initialize_buffer_by_searchitem (item);
    item->target_tb = (gpointer) gw_common_get_gobject_by_target (item->target);
    item->target_tv = (gpointer) gw_common_get_widget_by_target (item->target);
}


//!
//! @brief The details to be taken care of after a search is finished
//!
//! This is the function that takes care of things such as hiding progressbars,
//! changing action verbs to past verbs (Searching... vs Found) and for displaying
//! "no results found" pages.  Before this function is called, the searchitem search
//! status changes from GW_SEARCH_SEARCHING to GW_SEARCH_FINISHING.
//!
//! @param item A GwSearchItem pointer to get information from
//!
void gw_ui_after_search_cleanup (GwSearchItem *item)
{
    //Finish up
    if (item->total_results == 0 &&
        item->target != GW_TARGET_KANJI && item->status != GW_SEARCH_CANCELING)
    {
      gw_ui_no_result (item);
    }
}

