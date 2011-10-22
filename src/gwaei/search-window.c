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
//! @file search-window.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


//Static declarations
static void _searchwindow_attach_signals (GwSearchWindow*);
static void _searchwindow_remove_signals (GwSearchWindow*);
void _searchwindow_mousedata_init (GwSearchWindowMouseData*);
void _searchwindow_mousedata_deinit (GwSearchWindowMouseData*);
void _searchwindow_keepsearchingdata_init (GwSearchWindowKeepSearchingData*);
void _searchwindow_keepsearchingdata_deinit (GwSearchWindowKeepSearchingData*);


//!
//! @brief Sets up the variables in main-interface.c and main-callbacks.c for use
//!
GtkWindow* gw_searchwindow_new ()
{
    //Declarations
    GtkWindow *window;

    //Initializations
    window = GTK_WINDOW (g_object_new (GW_TYPE_SEARCHWINDOW,
                                       "type", GTK_WINDOW_TOPLEVEL,
                                       NULL));
    if (window != NULL)
    {
      window->priv = GW_WINDOW_GET_PRIVATE (window);
      gw_searchwindow_init (window);
    }

    return window;
}


void gw_searchwindow_init (GwSearchWindow *window)
{
    //Declarations
    GwSearchWindowPrivate *private;

    priv = GW_WINDOW_GET_PRIVATE (window);

    gw_searchwindow_private_init (window);
    gw_window_load_ui_xml (GW_WINDOW (window), "search.ui");
    gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_MOUSE);
}


void gw_searchwindow_finalize (GObject *object)
{
  GwSearchWindow *window;

  window = GW_SEARCHWINDOW (object);

  gw_searchwindow_private_finalize (window);
  G_OBJECT_CLASS (gw_searchwindow_parent_class)->finalize (object);
}


//!
//! @brief Updates the progress information based on the LwSearchItem info
//! @param item A LwSearchItem pointer to gleam information from.
//! @returns Currently always returns TRUE
//!
gboolean gw_searchwindow_update_progress_feedback_timeout (GwSearchWindow *window)
{
    //Sanity checks
    if (window->timeoutid[GW_SEARCHWINDOW_TIMEOUTID_PROGRESS] == 0) return FALSE;
    if (gtk_widget_get_visible (GTK_WIDGET (window->toplevel)) == FALSE) return TRUE;

    //Declarations
    LwSearchItem *item;

    //Initializations
    item = gw_searchwindow_get_current_searchitem (window);

    if (item != NULL) 
    {
      lw_searchitem_lock_mutex (item);
        if (
            item->target != LW_OUTPUTTARGET_KANJI &&
            item->status != LW_SEARCHSTATUS_CANCELING &&
            (item != window->feedbackdata.item ||
             item->current_line != window->feedbackdata.line ||
             item->status != window->feedbackdata.status       )
            )
        {
          gw_searchwindow_set_search_progressbar_by_searchitem (window, item);
          gw_searchwindow_set_total_results_label_by_searchitem (window, item);
          gw_searchwindow_set_title_by_searchitem (window, item);

          window->feedbackdata.item = item;
          window->feedbackdata.line = item->current_line;
          window->feedbackdata.status = item->status;
        }
      lw_searchitem_unlock_mutex (item);
    }

   return TRUE;
}


//!
//! @brief Sets the query text of the program using the informtion from the searchitem
//!
//! @param item a LwSearchItem argument.
//!
void gw_searchwindow_set_entry_text_by_searchitem (GwSearchWindow* window, LwSearchItem *item)
{
    //Declarations
    char hex_color_string[100];
    GdkRGBA color;

    //If there is no window, set the default colors
    if (item == NULL)
    {
      gtk_entry_set_text (window->entry, "");
/*
      gtk_widget_override_background_color (GTK_WIDGET (window->entry), GTK_STATE_NORMAL, NULL);
      gtk_widget_override_color (GTK_WIDGET (window->entry), GTK_STATE_NORMAL, NULL);
*/
    }
    //There was previously a window, set the match colors from the prefs
    else
    {
      if (item->queryline != NULL && strlen(item->queryline->string) > 0)
      {
        if (strcmp(gtk_entry_get_text (window->entry), item->queryline->string) != 0)
        {
          gtk_entry_set_text (window->entry, item->queryline->string);
          gtk_editable_set_position (GTK_EDITABLE (window->entry), -1);
        }
      }
      else
      {
        gtk_entry_set_text (window->entry, "");
      }

      //Set the foreground color
      lw_preferences_get_string_by_schema (app->preferences, hex_color_string, LW_SCHEMA_HIGHLIGHT, LW_KEY_MATCH_FG, 100);
      if (gdk_rgba_parse (&color, hex_color_string) == FALSE)
      {
        lw_preferences_reset_value_by_schema (app->preferences, LW_SCHEMA_HIGHLIGHT, LW_KEY_MATCH_FG);
        return;
      }
      //gtk_widget_override_color (GTK_WIDGET (window->entry), GTK_STATE_NORMAL, &color);

      //Set the background color
      lw_preferences_get_string_by_schema (app->preferences, hex_color_string, LW_SCHEMA_HIGHLIGHT, LW_KEY_MATCH_BG, 100);
      if (gdk_rgba_parse (&color, hex_color_string) == FALSE)
      {
        lw_preferences_reset_value_by_schema (app->preferences, LW_SCHEMA_HIGHLIGHT, LW_KEY_MATCH_BG);
        return;
      }
      //gtk_widget_override_background_color (GTK_WIDGET (window->entry), GTK_STATE_NORMAL, &color);
    }
}


//!
//! @brief Sets the main window title text of the program using the informtion from the searchitem
//!
//! @param item a LwSearchItem argument.
//!
char* gw_searchwindow_get_title_by_searchitem (GwSearchWindow* window, LwSearchItem *item)
{
    //Declarations
    char *title;
    const char *program_name;
    int num_relevant, num_total;
    char *query;

    if (item == NULL || item->queryline == NULL)
    {
      //Initializations
      title = g_strdup (gw_app_get_program_name (app));
    }
    else
    {
      //Initializations
      program_name = gw_app_get_program_name(app);
      query = item->queryline->string;
      num_relevant = item->total_relevant_results;
      num_total = item->total_results;
      title = g_strdup_printf ("%s [%d/%d] - %s", query, num_relevant, num_total, program_name);
    }

    return title;
}

//!
//! @brief Sets the main window title text of the program using the informtion from the searchitem
//!
//! @param item a LwSearchItem argument.
//!
void gw_searchwindow_set_title_by_searchitem (GwSearchWindow* window, LwSearchItem *item)
{
    //Declarations
    char *title;

    //Initializations
    title = gw_searchwindow_get_title_by_searchitem (window, item);

    gtk_window_set_title (window->toplevel, title);

    //Cleanup
    g_free (title);
}


//!
//! @brief Set's the progress label of the program using the inforamtion from the searchitem
//!
//! @param item A LwSearchItem pointer to gleam information from
//!
void gw_searchwindow_set_total_results_label_by_searchitem (GwSearchWindow *window, LwSearchItem* item)
{
    //Declarations
    GtkWidget *label;
    
    //Initializations
    label = GTK_WIDGET (gtk_builder_get_object(window->builder, "progress_label"));

    if (item == NULL)
    {
      gtk_label_set_text(GTK_LABEL (label), "");
    }
    else if (item->target != LW_OUTPUTTARGET_RESULTS)
    {
      return;
    }
    else
    {
      //Declarations
      int relevant = item->total_relevant_results;
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
        case LW_SEARCHSTATUS_IDLE:
            if (item->current_line == 0)
              gtk_label_set_text (GTK_LABEL (label), idle_message_none);
            else if (relevant == total)
              final_message = g_strdup_printf (idle_message_total, total);
            else
            {
              base_message = g_strdup_printf ("%s %s", idle_message_total, message_relevant);
              if (base_message != NULL)
                final_message = g_strdup_printf (base_message, total, relevant);
            }
            break;
        case LW_SEARCHSTATUS_SEARCHING:
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
        case LW_SEARCHSTATUS_CANCELING:
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
void gw_searchwindow_set_dictionary (GwSearchWindow *window, int request)
{
    LwDictInfo *di;
    GtkMenuShell *shell;
    GList *list;
    GtkWidget *radioitem;

    di = lw_dictinfolist_get_dictinfo_by_load_position (LW_DICTINFOLIST (app->dictinfolist), request);
    if (di == NULL) return;

    window->dictinfo = di;

    //Make sure the correct radio menuitem is selected
    shell = GTK_MENU_SHELL (gtk_builder_get_object (window->builder, "dictionary_popup"));
    if (shell != NULL)
    {
      list = gtk_container_get_children (GTK_CONTAINER (shell));
      radioitem = GTK_WIDGET (g_list_nth_data (list, request));
      g_signal_handlers_block_by_func (radioitem, gw_searchwindow_dictionary_radio_changed_cb, window->toplevel);
      if (g_list_length (list) > 3 && radioitem != NULL)
      {
        gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (radioitem), TRUE);
      }
      g_signal_handlers_unblock_by_func (radioitem, gw_searchwindow_dictionary_radio_changed_cb, window->toplevel);
      g_list_free (list);
    }

    //Make sure the correct combobox item is selected
    g_signal_handlers_block_by_func (window->combobox, gw_searchwindow_dictionary_combobox_changed_cb, NULL);
    gtk_combo_box_set_active (window->combobox, request);
    g_signal_handlers_unblock_by_func (window->combobox, gw_searchwindow_dictionary_combobox_changed_cb, NULL);
}


//!
//! @brief Uses a LwSearchItem to set the currently active dictionary
//!
//! This function is greatly useful for doing searches from the history.
//!
//! @param item A lnSearchItem to gleam information from
//!
void gw_searchwindow_set_dictionary_by_searchitem (GwSearchWindow *window, LwSearchItem *item)
{
    if (item == NULL)
      gw_searchwindow_set_dictionary (window, 0);
    else if (item->dictionary != NULL)
      gw_searchwindow_set_dictionary (window, item->dictionary->load_position);
}


LwDictInfo* gw_searchwindow_get_dictionary (GwSearchWindow* window)
{
    return window->dictinfo;
}


//!
//! @brief Updates the status of the window progressbar
//!
//! @param item A LwSearchItem to gleam information from
//!
void gw_searchwindow_set_search_progressbar_by_searchitem (GwSearchWindow *window, LwSearchItem *item)
{
    //Declarations
    GtkWidget *progressbar;
    GtkWidget *statusbar;
    long current;
    long total;
    double fraction;

    //Initializations
    progressbar = GTK_WIDGET (gtk_builder_get_object (window->builder, "search_progressbar"));
    statusbar = GTK_WIDGET (gtk_builder_get_object (window->builder, "statusbar"));
    current = 0;
    total = 0;

    if (item != NULL && item->dictionary != NULL)
    {
      current = item->current_line;
      total = item->dictionary->total_lines;
    }

    if (current == 0) fraction = 0.0;
    else fraction = (double)current / (double)total;

    if (item == NULL ||
        item->dictionary == NULL ||
        total == 0 ||
        fraction >= (1.0 - 0.00001) ||
        item->status == LW_SEARCHSTATUS_IDLE ||
        item->status == LW_SEARCHSTATUS_CANCELING   )
    {
      gtk_entry_set_progress_fraction (GTK_ENTRY (window->entry), 0.0);
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progressbar), 0.0);
    }
    else if (gtk_widget_get_visible (statusbar))
    {
      gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progressbar), fraction);
      gtk_entry_set_progress_fraction (GTK_ENTRY (window->entry), 0.0);
    }
    else
    {
      gtk_entry_set_progress_fraction (GTK_ENTRY (window->entry), fraction);
    }
}


//!
//! @brief Updates the history menu popup menu
//!
void gw_searchwindow_update_history_menu_popup (GwSearchWindow *window)
{
    //Declarations
    char *id;
    GtkMenuShell *shell;
    GList *children;
    GList *list;
    GList *iter;
    LwSearchItem *item;
    GtkWidget *menuitem;

    //Initializations
    id = "history_popup";
    shell = GTK_MENU_SHELL (gtk_builder_get_object(window->builder, id));
    children = gtk_container_get_children (GTK_CONTAINER (shell));
    iter = children;

    //Skip over the back/forward buttons
    if (iter != NULL) iter = g_list_next (iter);
    if (iter != NULL) iter = g_list_next (iter);

    //Remove all widgets after the back and forward menuitem buttons
    while (iter != NULL)
    {
      gtk_widget_destroy (GTK_WIDGET (iter->data));
      iter = iter->next;
    }
    g_list_free (children);


    list = lw_history_get_combined_list (window->history);

    //Add a separator if there are some items in history
    if (list != NULL)
    {
      //Add a seperator to the end of the history popup
      menuitem = GTK_WIDGET (gtk_separator_menu_item_new());
      gtk_menu_shell_append (GTK_MENU_SHELL (shell), menuitem);
      gtk_widget_show (menuitem);
    }

    //Fill the history items
    for (iter = list; iter != NULL; iter = iter->next)
    {
      item = LW_SEARCHITEM (iter->data);

      GtkWidget *menu_item, *accel_label, *label;

      accel_label = gtk_label_new (item->dictionary->longname);
      gtk_widget_set_sensitive (GTK_WIDGET (accel_label), FALSE);
      label = gtk_label_new (item->queryline->string);

      GtkWidget *hbox;
      hbox = gtk_hbox_new (FALSE, 0);

      menu_item = gtk_menu_item_new();
      g_signal_connect (GTK_WIDGET (menu_item), 
                        "activate",
                        G_CALLBACK (gw_searchwindow_search_from_history_cb), 
                        window->toplevel                                    );

      gtk_menu_shell_append(GTK_MENU_SHELL (shell), GTK_WIDGET (menu_item));
      gtk_container_add (GTK_CONTAINER (menu_item), hbox);
      gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
      gtk_box_pack_end (GTK_BOX (hbox), accel_label, FALSE, FALSE, 0);

      gtk_widget_show(label);
      gtk_widget_show(accel_label);
      gtk_widget_show(hbox);
      gtk_widget_show(menu_item);
    }
    g_list_free (list);
}


//!
//! @brief PRIVATE FUNCTION. Populate the menu item lists for the back and forward buttons
//!
//! @param id Id of the popuplist
//! @param list history list to compair against
//!
static void _rebuild_history_button_popup (GwSearchWindow *window, char* id, GList* list)
{
    //Get a reference to the history_popup
    GtkWidget *popup;
    popup = GTK_WIDGET (gtk_builder_get_object(window->builder, id));

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
    gtk_widget_show (menuitem);

    //Declarations
    LwSearchItem *item;

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
                        G_CALLBACK (gw_searchwindow_search_from_history_cb), 
                        window->toplevel                                    );
   
      children = children->next;
    }
}


//!
//! @brief Convenience function to update both the back and forward histories etc
//!
void gw_searchwindow_update_history_popups (GwSearchWindow* window)
{
    GList* list;
    const char *id;
    GtkAction *action;

    gw_searchwindow_update_history_menu_popup (window);

    list = lw_history_get_forward_list (window->history);
    _rebuild_history_button_popup(window, "forward_popup", list);

    list = lw_history_get_back_list (window->history);
    _rebuild_history_button_popup(window, "back_popup", list);

    //Update back button
    id = "history_back_action";
    action = GTK_ACTION (gtk_builder_get_object (window->builder, id));
    gtk_action_set_sensitive (action, lw_history_has_back (window->history));

    //Update forward button
    id = "history_forward_action";
    action = GTK_ACTION (gtk_builder_get_object (window->builder, id));
    gtk_action_set_sensitive (action, lw_history_has_forward (window->history));
}


//!
//! @brief Sets the style of the toolbar [icons/text/both/both-horizontal]
//!
//! @param request The name of the style
//!
void gw_searchwindow_set_toolbar_style (GwSearchWindow *window, const char *request) 
{
    GtkToolbarStyle style;

    if (strcmp(request, "text") == 0)
      style = GTK_TOOLBAR_TEXT;
    else if (strcmp(request, "icons") == 0)
      style = GTK_TOOLBAR_ICONS;
    else if (strcmp(request, "both-horiz") == 0)
      style = GTK_TOOLBAR_BOTH_HORIZ;
    else
      style = GTK_TOOLBAR_BOTH;

    gtk_toolbar_set_style (window->toolbar, style);
}


//!
//! @brief Appends some text to the text buffer
//!
//! @param item A LwSearchItem to gleam information from
//! @param text The text to append to the buffer
//! @param tag1 A tag to apply to the text or NULL
//! @param tag2 A tag to apply to the text or NULL
//! @param start_line Returns the start line of the text inserted
//! @param end_line Returns the end line of the text inserted
//!
void gw_searchwindow_append_to_buffer (GwSearchWindow *window, LwSearchItem *item, char *text, char *tag1,
                                       char *tag2, int *start_line, int *end_line)
{
    if (item == NULL) return;

    //Assertain the target text buffer
    GwSearchData *sd;
    GtkTextView *view;
    GtkTextBuffer *buffer;

    sd = (GwSearchData*) lw_searchitem_get_data (item);
    view = GTK_TEXT_VIEW (sd->view);
    if (view == NULL) return;
    buffer = gtk_text_view_get_buffer (view);
    if (buffer == NULL) return;

    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    if (start_line != NULL)
      *start_line = gtk_text_iter_get_line(&iter);

    if (tag1 == NULL && tag2 == NULL)
      gtk_text_buffer_insert (buffer, &iter, text, -1);
    else if (tag2 == NULL)
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, text, -1, tag1, NULL);
    else if (tag1 == NULL)
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, text, -1, tag2, NULL);
    else
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, text, -1, tag1, tag2, NULL);

    gtk_text_buffer_get_end_iter (buffer, &iter);
    if (end_line != NULL)
      *end_line = gtk_text_iter_get_line(&iter);
}


//!
//! @brief Performs initializations absolutely necessary before a window can take place
//!
//! Correctly the pointer in the LwSearchItem to the correct textbuffer and moves marks
//!
//! @param item A LwSearchItem to gleam information from
//!
void gw_searchwindow_initialize_buffer_by_searchitem (GwSearchWindow *window, LwSearchItem *item)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Make sure searches done from the history are pointing at a valid target
    GwSearchData *data;
    GtkTextView *view;
    GtkTextBuffer *buffer;

    data = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (data->view);
    buffer = gtk_text_view_get_buffer (view);

    if (view == NULL || buffer == NULL) return;

    gtk_text_buffer_set_text (buffer, "", -1);

    if (item->target == LW_OUTPUTTARGET_RESULTS)
    {
      //Clear the target text buffer
      GtkTextIter iter;
      gtk_text_buffer_get_end_iter (buffer, &iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, "\n", -1, "small", NULL);

      gtk_text_buffer_get_end_iter (buffer, &iter);
      gtk_text_buffer_create_mark (buffer, "more_relevant_header_mark", &iter, TRUE);
      gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

      gtk_text_buffer_get_end_iter (buffer, &iter);
      gtk_text_buffer_insert (buffer, &iter, "\n", -1);
      gtk_text_buffer_get_end_iter (buffer, &iter); gtk_text_iter_backward_line (&iter);
      gtk_text_buffer_create_mark (buffer, "more_rel_content_insertion_mark", &iter, FALSE);
      gtk_text_buffer_create_mark (buffer, "content_insertion_mark", &iter, FALSE);

      gtk_text_buffer_get_end_iter (buffer, &iter);
      gtk_text_buffer_create_mark (buffer, "less_relevant_header_mark", &iter, TRUE);
      gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);

      gtk_text_buffer_get_end_iter (buffer, &iter);
      gtk_text_buffer_insert (buffer, &iter, "\n", -1);
      gtk_text_buffer_get_end_iter (buffer, &iter); gtk_text_iter_backward_line (&iter);
      gtk_text_buffer_create_mark (buffer, "less_rel_content_insertion_mark", &iter, FALSE);

      gtk_text_buffer_get_end_iter (buffer, &iter);
      gtk_text_buffer_insert (buffer, &iter, "\n\n\n", -1);
      gtk_text_buffer_get_end_iter (buffer, &iter);
      gtk_text_buffer_create_mark (buffer, "footer_insertion_mark", &iter, FALSE);

      gw_searchwindow_set_total_results_label_by_searchitem (window, item);
    }
}


//!
//! @brief Inserts text into the window entry
//!
//! @param text The text to insert
//!
void gw_searchwindow_entry_insert (GwSearchWindow *window, char* text)
{
    glong length;
    length = strlen (text);

    gint start, end;
    gtk_editable_get_selection_bounds (GTK_EDITABLE (window->entry), &start, &end);
    gtk_editable_delete_text (GTK_EDITABLE (window->entry), start, end);

    gtk_editable_insert_text(GTK_EDITABLE (window->entry), text, length, &start);
    gtk_editable_set_position (GTK_EDITABLE (window->entry), start);
}


//!
//! @brief Selects all text in a target
//!
//! @param TARGET The widget where to select all text
//!
void gw_searchwindow_select_all_by_target (GwSearchWindow* window, const LwOutputTarget TARGET)
{
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;

    //GtkEntry
    if (TARGET == LW_OUTPUTTARGET_ENTRY)
    {
      gtk_editable_select_region (GTK_EDITABLE (window->entry), 0,-1);
    }

    //GtkTextView
    else if (TARGET == LW_OUTPUTTARGET_RESULTS ||
             TARGET == LW_OUTPUTTARGET_KANJI     )
    {
      view = gw_searchwindow_get_current_textview (window);
      buffer = gtk_text_view_get_buffer (view);

      gtk_text_buffer_get_start_iter (buffer, &start);
      gtk_text_buffer_get_end_iter (buffer, &end);

      gtk_text_buffer_select_range (buffer, &start, &end);
    }
}


//!
//! @brief Deselects all text in a target
//!
//! @param TARGET The widget where to deselect all text
//!
void gw_searchwindow_select_none_by_target (GwSearchWindow *window, LwOutputTarget TARGET)
{
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter start, end;

    //GtkEntry
    if (TARGET == LW_OUTPUTTARGET_ENTRY)
    {
      gtk_editable_select_region (GTK_EDITABLE (window->entry), -1,-1);
    }

    //GtkTextView
    else if (TARGET == LW_OUTPUTTARGET_RESULTS ||
             TARGET == LW_OUTPUTTARGET_KANJI     )
    {
      view = gw_searchwindow_get_current_textview (window);
      buffer = gtk_text_view_get_buffer (view);

      gtk_text_buffer_get_end_iter (buffer, &start);
      gtk_text_buffer_get_end_iter (buffer, &end);

      gtk_text_buffer_select_range (buffer, &start, &end);
    }
}


//!
//! @brief Returns the target id corresponding to what widget has focus
//!
//! @param window_id The window to check the widgets against 
//!
LwOutputTarget gw_searchwindow_get_current_target_focus (GwSearchWindow* window)
{
    //Declarations
    GtkWidget *widget;
    GtkTextView *view;
    GtkEntry *entry;
    LwOutputTarget target;

    //Initializations
    widget = GTK_WIDGET (gtk_window_get_focus (window->toplevel)); 
    view = gw_searchwindow_get_current_textview (window);
    entry = window->entry;

    if (widget == GTK_WIDGET (view))
      target = LW_OUTPUTTARGET_RESULTS;
    else if (widget == GTK_WIDGET (entry))
      target = LW_OUTPUTTARGET_ENTRY;
    else
      target = LW_OUTPUTTARGET_INVALID;

    return target;
}


//!
//! @brief Copy Text into the target output
//!
//! TARGET LwOutputTarget to specify where the text should come from
//!
void gw_searchwindow_copy_text (GwSearchWindow* window, LwOutputTarget TARGET)
{
    GtkClipboard *clipbd;
    GtkTextView *view;
    GtkTextBuffer *buffer;

    view = gw_searchwindow_get_current_textview (window);
    buffer = gtk_text_view_get_buffer (view);

    switch (TARGET)
    {
      case LW_OUTPUTTARGET_ENTRY:
        gtk_editable_copy_clipboard (GTK_EDITABLE (window->entry));
        break;
      case LW_OUTPUTTARGET_RESULTS:
        clipbd = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
        gtk_text_buffer_copy_clipboard (buffer, clipbd);
        break;
      case LW_OUTPUTTARGET_KANJI:
      case LW_OUTPUTTARGET_VOCABULARY:
      case LW_OUTPUTTARGET_INVALID:
        break;
    }
}


//!
//! @brief Cut Text into the target output
//!
//! TARGET LwOutputTarget to specify where the text should come from
//!
void gw_searchwindow_cut_text (GwSearchWindow *window, LwOutputTarget TARGET)
{
    switch (TARGET)
    {
      case LW_OUTPUTTARGET_ENTRY:
        gtk_editable_cut_clipboard (GTK_EDITABLE (window->entry));
        break;
      case LW_OUTPUTTARGET_RESULTS:
      case LW_OUTPUTTARGET_KANJI:
      case LW_OUTPUTTARGET_VOCABULARY:
      case LW_OUTPUTTARGET_INVALID:
        break;
    }
}


//!
//! @brief Pastes Text into the target output
//!
//! TARGET LwOutputTarget to specify where the text should go
//!
void gw_searchwindow_paste_text (GwSearchWindow* window, LwOutputTarget TARGET)
{
    switch (TARGET)
    {
      case LW_OUTPUTTARGET_ENTRY:
        gtk_editable_paste_clipboard (GTK_EDITABLE (window->entry));
        break;
      case LW_OUTPUTTARGET_RESULTS:
      case LW_OUTPUTTARGET_KANJI:
      case LW_OUTPUTTARGET_VOCABULARY:
      case LW_OUTPUTTARGET_INVALID:
        break;
    }
}



//!
//! @brief Returns the slice of characters between to line numbers in the target output buffer
//!
//! @param TARGET The LwOutputTarget to get the text slice from
//! @param sl The start line number
//! @param el The end line number
//!
char* gw_searchwindow_buffer_get_text_slice_by_target (GwSearchWindow* window, LwOutputTarget TARGET, int sl, int el)
{
    //Assertain the target text buffer
    GtkTextView *view;
    GtkTextBuffer *buffer;

    view = gw_searchwindow_get_current_textview (window);
    buffer = gtk_text_view_get_buffer (view);

    //Set up the text
    GtkTextIter si, ei;
    gtk_text_buffer_get_iter_at_line (buffer, &si, sl);
    gtk_text_buffer_get_iter_at_line (buffer, &ei, el);

    return gtk_text_buffer_get_slice (buffer, &si, &ei, TRUE);
}


//!
//! @brief Returns the character currently under the cursor in the main results window
//!
//! @param x Pointer to the x coordinate
//! @param y Pointer to the y coordinate
//!
//! @return Returns the character that is being moused over
//!
gunichar gw_searchwindow_get_hovered_character (GwSearchWindow* window, int *x, int *y, GtkTextIter *start)
{
    //Declarations;
    gint trailing;
    GtkTextView* view;

    //Initializations
    trailing = 0;
    view = gw_searchwindow_get_current_textview (window);

    gtk_text_view_window_to_buffer_coords (view, GTK_TEXT_WINDOW_TEXT, *x, *y, x, y);
    gtk_text_view_get_iter_at_position (view, start, &trailing, *x, *y);

    return gtk_text_iter_get_char (start);
} 


//!
//! @brief A convenience function to set the gdk cursor
//!
//! @param GdkCursorType The prefered cursor to set
//!
void gw_searchwindow_set_cursor (GwSearchWindow* window, GdkCursorType CURSOR)
{
    //Declarations
    GdkWindow* gdkwindow;
    GtkTextView *view;
    GdkCursor* cursor;

    //Initializations
    view = gw_searchwindow_get_current_textview (window);
    gdkwindow = gtk_text_view_get_window (view, GTK_TEXT_WINDOW_TEXT);
    cursor = gdk_cursor_new (CURSOR);

    gdk_window_set_cursor (gdkwindow, cursor);
    gdk_cursor_unref (cursor);
}


//!
//! @brief Sets the no results page to the output buffer
//!
//! @param item A LwSearchItem pointer to gleam information from
//!
void gw_searchwindow_display_no_results_found_page (GwSearchWindow *window, LwSearchItem *item)
{
    if (item->status == LW_SEARCHSTATUS_CANCELING) return; 

    gint32 temp = g_random_int_range (0,9);
    while (temp == window->previous_tip)
      temp = g_random_int_range (0,9);
    const gint32 TIP_NUMBER = temp;
    window->previous_tip = temp;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkWidget *image = NULL;
    GtkTextIter iter;
    GtkTextChildAnchor *anchor = NULL;
    GtkWidget *label = NULL;
    GtkWidget *hbox = NULL;
    char *body = NULL;
    const char *query_text;
    int i = 0;
    GtkWidget *button = NULL;
    char *markup = NULL;
    LwDictInfo *di_selected;
    LwDictInfo *di;
    GwSearchData *sdata;

    sdata = (GwSearchData*) lw_searchitem_get_data (item);
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    query_text = gtk_entry_get_text (GTK_ENTRY (window->entry));
    di_selected = gw_searchwindow_get_dictionary (window);

  gdk_threads_enter ();
    gtk_text_buffer_set_text (buffer, "", -1);

    //Add the title
    gw_searchwindow_append_to_buffer (window, item, "\n", "small", NULL, NULL, NULL);


    //Set the header message
    hbox = gtk_hbox_new (FALSE, 10);
    gtk_text_buffer_get_end_iter (buffer, &iter);
    anchor = gtk_text_buffer_create_child_anchor (buffer, &iter);
    gtk_text_view_add_child_at_anchor (GTK_TEXT_VIEW (view), hbox, anchor);
    gtk_widget_show (hbox);

    image = gtk_image_new_from_stock (GTK_STOCK_FIND, GTK_ICON_SIZE_DIALOG);
    gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (image));
    gtk_widget_show (image);

    label = gtk_label_new (NULL);
    char *message = NULL;
    // TRANSLATORS: The argument is the dictionary long name
    message = g_strdup_printf(gettext("Nothing found in the %s!"), di_selected->longname);
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
    gw_searchwindow_append_to_buffer (window, item, "\n\n\n", NULL, NULL, NULL, NULL);


    if (lw_dictinfolist_get_total (LW_DICTINFOLIST (app->dictinfolist)) > 1)
    {
      //Add label for links
      hbox = gtk_hbox_new (FALSE, 0);
      gtk_text_buffer_get_end_iter (buffer, &iter);
      anchor = gtk_text_buffer_create_child_anchor (buffer, &iter);
      gtk_text_view_add_child_at_anchor (view, hbox, anchor);
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
      di = lw_dictinfolist_get_dictinfo_by_load_position (LW_DICTINFOLIST (app->dictinfolist), 0);

      while ((di = lw_dictinfolist_get_dictinfo_by_load_position (LW_DICTINFOLIST (app->dictinfolist), i)) != NULL)
      {
        if (di != di_selected)
        {
          button = gtk_button_new_with_label (di->shortname);
          g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (gw_searchwindow_no_results_search_for_dictionary_cb), di);
          gtk_container_add (GTK_CONTAINER (hbox), GTK_WIDGET (button));
          gtk_widget_show (GTK_WIDGET (button));
        }
        i++;
      }

      gw_searchwindow_append_to_buffer (window, item, "\n", NULL, NULL, NULL, NULL);
    }

    //Add label for links
    hbox = gtk_hbox_new (FALSE, 0);
    gtk_text_buffer_get_end_iter (buffer, &iter);
    anchor = gtk_text_buffer_create_child_anchor (buffer, &iter);
    gtk_text_view_add_child_at_anchor (view, hbox, anchor);
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
      "Google", "http://www.google.com/window?q=%s", "google.png",
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




    gw_searchwindow_append_to_buffer (window, item, "\n\n\n", NULL, NULL, NULL, NULL);




    //Insert the instruction text
    char *tip_header_str = NULL;
    tip_header_str = g_strdup_printf (gettext("gWaei Usage Tip #%d: "), (TIP_NUMBER + 1));
    if (tip_header_str != NULL)
    {
      gw_searchwindow_append_to_buffer (window, item, tip_header_str,
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
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Inputting Unknown Kanji"),
                                "header", "important", NULL, NULL         );
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        if (body != NULL)
        {
          gw_searchwindow_append_to_buffer (window, item, body,
                                  NULL, NULL, NULL, NULL);
          g_free (body);
          body = NULL;
        }
        break;

     case 1:
        //Tip 2
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Getting More Exact Matches"),
                                "important", "header", NULL, NULL         );
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Use the Word-edge Mark and the Not-word-edge Mark from the "
                                "insert menu to get more relevant results.  fish\\b will return results "
                                "like fish and selfish , but not fisherman"),
                                NULL, NULL, NULL, NULL);
        break;

     case 2:
        //Tip 3
        body = g_strdup_printf (gettext("Use the And Character or Or Character to window for "
                                "results that contain a combination of words that might not be "
                                "right next to each other.  cats&dogs will return only results "
                                "that contain both the words cats and dogs like %s does."),
                                "犬猫");
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Searching for Multiple Words"),
                                "important", "header", NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL);
        if (body != NULL)
        {
          gw_searchwindow_append_to_buffer (window, item, body,
                                  NULL, NULL, NULL, NULL);
          g_free (body);
          body = NULL;
        }
        break;

     case 3:
        //Tip 4
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Make a Vocabulary List"),
                                "important", "header", NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Specific sections of results can be printed or saved by "
                                "dragging the mouse to highlight them.  Using this in combination "
                                "with the Append command from the File menu or toolbar, quick and "
                                "easy creation of a vocabulary lists is possible."),
                                NULL, NULL, NULL, NULL);
        break;

     case 4:
        //Tip 5
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Why Use the Mouse?"),
                                "important", "header", NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Typing something will move the focus to the window input "
                                "box.  Hitting the Up or Down arrow key will move the focus to the "
                                "results pane so you can scroll the results.  Hitting Alt-Up or "
                                "Alt-Down will cycle the currently installed dictionaries."),
                                NULL, NULL, NULL, NULL         );
        break;

     case 5:
        //Tip 6
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Get Ready for the JLPT"),
                                "important", "header", NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_searchwindow_append_to_buffer (window, item,
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
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Just drag words in!"),
                                "important", "header", NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("If you drag and drop a highlighted word into gWaei's "
                                "window result box, gWaei will automatically start a window "
                                "using that text.  This can be a nice way to quickly look up words "
                                "while browsing webpages. "),
                                NULL, NULL, NULL, NULL         );

        break;

     case 7:
        //Tip 8
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("What does (adj-i) mean?"),
                                "important", "header", NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_searchwindow_append_to_buffer (window, item,
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
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Books are Heavy"),
                                "important", "header", NULL, NULL);
        gw_searchwindow_append_to_buffer (window, item,
                                "\n\n",
                                NULL, NULL, NULL, NULL         );
        gw_searchwindow_append_to_buffer (window, item,
                                gettext("Aways wear a construction helmet when working with books.  "
                                "They are dangerous heavy objects that can at any point fall on and "
                                "injure you.  Please all urge all of your friends to, too.  They will "
                                "thank you later.  Really."),
                                NULL, NULL, NULL, NULL         );
       break;
    }

    gw_searchwindow_append_to_buffer (window, item,
                               "\n\n",
                               NULL, NULL, NULL, NULL         );
gdk_threads_leave ();
}


//!
//! @brief Cycles the dictionaries forward or backward, looping when it reaches the end
//!
//! @param cycle_forward A boolean to choose the cycle direction
//!
void gw_searchwindow_cycle_dictionaries (GwSearchWindow* window, gboolean cycle_forward)
{
    int increment;

    if (cycle_forward)
      increment = 1;
    else
      increment = -1;

    //Declarations
    GtkTreeIter iter;
    gint active;
    gboolean set;

    active = gtk_combo_box_get_active (window->combobox);
    set = FALSE;

    if ((active = active + increment) == -1)
    {
      do {
        active++;
        gtk_combo_box_set_active (window->combobox, active);
        set = gtk_combo_box_get_active_iter (window->combobox, &iter);
      } while (set);
      active--;
      gtk_combo_box_set_active (window->combobox, active);
    }
    else
    {
      gtk_combo_box_set_active (window->combobox, active);
      set = gtk_combo_box_get_active_iter (window->combobox, &iter);
      if (!set)
        gtk_combo_box_set_active (window->combobox, 0);
    }
}


//!
//! @brief  Returns the unfreeable text from a gtk widget by target
//!
//! The fancy thing about this function is it will only return the
//! highlighted text if some is highlighted.
//!
//! @param TARGET a LwOutputTarget to get the data from
//!
char* gw_searchwindow_get_text_by_target (GwSearchWindow *window, LwOutputTarget TARGET)
{
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter s, e;

    view = gw_searchwindow_get_current_textview (window);
    buffer = gtk_text_view_get_buffer (view);

    if (gtk_text_buffer_get_has_selection (buffer))
    {
      gtk_text_buffer_get_selection_bounds (buffer, &s, &e);
    }
    //Get the region of text to be saved if no text is highlighted
    else
    {
      gtk_text_buffer_get_start_iter (buffer, &s);
      gtk_text_buffer_get_end_iter (buffer, &e);
    }
    return gtk_text_buffer_get_text (buffer, &s, &e, FALSE);
}



//!
//! @brief A simple window initiater function made to be looped by a timer
//!
//! @param data An unused gpointer.  It should always be NULL
//!
gboolean gw_searchwindow_keep_searching_timeout (GwSearchWindow *window)
{
    //Sanity check
    if (!gw_app_can_start_search (app)) return TRUE;
    if (!window->keepsearchingdata.enabled) return TRUE;
    if (window->timeoutid[GW_SEARCHWINDOW_TIMEOUTID_KEEP_SEARCHING] == 0) return FALSE;

    //Declarations
    const char *query;

    //Initializations
    query = gtk_entry_get_text (GTK_ENTRY (window->entry));
    if (window->keepsearchingdata.query == NULL) window->keepsearchingdata.query = g_strdup ("");

    if (window->keepsearchingdata.delay >= GW_SEARCHWINDOW_KEEP_SEARCHING_MAX_DELAY || strlen(query) == 0)
    {
      window->keepsearchingdata.delay = 0;
      gtk_widget_activate (GTK_WIDGET (window->entry));
    }
    else
    {
      if (strcmp(window->keepsearchingdata.query, query) == 0)
      {
        window->keepsearchingdata.delay++;
      }
      else
      {
        if (window->keepsearchingdata.query != NULL)
          g_free (window->keepsearchingdata.query);
        window->keepsearchingdata.query = g_strdup (query);
        window->keepsearchingdata.delay = 0;
      }
    }
    
    return TRUE;
}


//!
//! @brief Abstraction function to find out if some text is selected
//!
//! It gets the requested text buffer and then returns if it has text selected
//! or not.
//!
//! @param TARGET A LwOutputTarget
//!
gboolean gw_searchwindow_has_selection_by_target (GwSearchWindow *window, LwOutputTarget TARGET)
{
    //Declarations
    GtkTextView *view;
    GtkTextBuffer *buffer;

    //Initializations
    view = gw_searchwindow_get_current_textview (window);
    buffer = gtk_text_view_get_buffer (view);

    return (buffer != NULL && gtk_text_buffer_get_has_selection (buffer));
}


void gw_searchwindow_cancel_search_by_searchitem (GwSearchWindow *window, LwSearchItem *item)
{
    gdk_threads_leave ();
    lw_searchitem_cancel_search (item);
    gdk_threads_enter ();
}


//!
//! @brief Cancels all searches in all currently open tabs
//!
void gw_searchwindow_cancel_all_searches (GwSearchWindow *window)
{
    //Declarations
    GList *iter;
    LwSearchItem *item;

    for (iter = window->tablist; iter != NULL; iter = iter->next)
    {
      item = LW_SEARCHITEM (iter->data);
      gw_searchwindow_cancel_search_by_searchitem (window, item);
    }

    gw_searchwindow_cancel_search_by_searchitem (window, window->mousedata.item);
}


//!
//! @brief Cancels the search of the tab number
//! @param page_num The page number of the tab to cancel the search of
//!
void gw_searchwindow_cancel_search_by_tab_number (GwSearchWindow *window, const int page_num)
{
    //Declarations
    LwSearchItem *item;

    //Initializations
    item = LW_SEARCHITEM (g_list_nth_data (window->tablist, page_num));

    //Sanity check
    if (item == NULL) return;

    gw_searchwindow_cancel_search_by_searchitem (window, item);
}


//!
//! @brief Cancels the search of the currently visibile tab
//!
void gw_searchwindow_cancel_search_for_current_tab (GwSearchWindow *window)
{
    //Declarations
    int page_num;

    //Initializations
    page_num = gtk_notebook_get_current_page (window->notebook);

    gw_searchwindow_cancel_search_by_tab_number (window, page_num);
}


//!
//! @brief Cancels a search by identifying matching gpointer
//! @param container A pointer to the top-most widget in the desired tab to cancel the search of.
//!
void gw_searchwindow_cancel_search_by_content (GwSearchWindow *window, gpointer container)
{
    //Declarations
    int position;
    LwSearchItem *item;

    //Initializations
    position = gtk_notebook_page_num (window->notebook, container);

    //Sanity check
    if (position == -1) return;

    item = LW_SEARCHITEM (g_list_nth_data (window->tablist, position));

    //Sanity check
    if (item == NULL) return;

    gw_searchwindow_cancel_search_by_searchitem (window, item);
}


GtkTextView* gw_searchwindow_get_current_textview (GwSearchWindow *window)
{
    //Sanity check
    g_assert (window != NULL);

    //Declarations
    int page_num;
    GtkScrolledWindow *scrolledwindow;
    GtkTextView *view;

    //Initializations
    view = NULL;
    page_num = gtk_notebook_get_current_page (window->notebook);
    scrolledwindow = GTK_SCROLLED_WINDOW (gtk_notebook_get_nth_page (window->notebook, page_num));
    if (scrolledwindow != NULL)
      view = GTK_TEXT_VIEW (gtk_bin_get_child (GTK_BIN (scrolledwindow)));

    return view;
}


//!
//! @brief Makes sure that at least one tab is available to output search results.
//!
void gw_searchwindow_guarantee_first_tab (GwSearchWindow *window)
{
    //Declarations
    int pages;

    //Initializations
    pages = gtk_notebook_get_n_pages (window->notebook);

    if (pages == 0)
    {
      gw_searchwindow_new_tab (window);
      gw_searchwindow_sync_current_searchitem (window);
    }
}


//!
//! @brief Sets the title text of the current tab.
//! @param TEXT The text to set to the tab
//!
void gw_searchwindow_set_tab_text_by_searchitem (GwSearchWindow *window, LwSearchItem *item)
{
    //Declarations
    int page_num;
    GtkWidget *container;
    GtkWidget *hbox;
    GtkWidget *vbox;
    GList *hchildren;
    GList *vchildren;
    GtkWidget *label;
    const char *text;
    GList *iter;

    if (item == NULL)
    {
      page_num = 0;
      text = gettext("(Empty)");
      for (iter = window->tablist; iter != NULL; iter = iter->next)
      {
        if (iter->data == NULL)
        {
          container = gtk_notebook_get_nth_page (window->notebook, page_num);
          hbox = GTK_WIDGET (gtk_notebook_get_tab_label(window->notebook, GTK_WIDGET (container)));
          hchildren = gtk_container_get_children (GTK_CONTAINER (hbox));
          vbox = GTK_WIDGET (hchildren->data);
          vchildren = gtk_container_get_children (GTK_CONTAINER (vbox));
          label = GTK_WIDGET (vchildren->data);
          gtk_label_set_text (GTK_LABEL (label), text);

          g_list_free (hchildren);
          g_list_free (vchildren);
        }
        page_num++;
      }
    }
    else
    {
      page_num = g_list_index (window->tablist, item);
      g_assert (page_num != -1);
      container = gtk_notebook_get_nth_page (window->notebook, page_num);
      hbox = GTK_WIDGET (gtk_notebook_get_tab_label (window->notebook, GTK_WIDGET (container)));
      hchildren = gtk_container_get_children (GTK_CONTAINER (hbox));
      vbox = GTK_WIDGET (hchildren->data);
      vchildren = gtk_container_get_children (GTK_CONTAINER (vbox));
      label = GTK_WIDGET (vchildren->data);
      g_assert (item->queryline != NULL);
      text = item->queryline->string;

      gtk_label_set_text (GTK_LABEL (label), text);

      //Cleanup
      g_list_free (hchildren);
      g_list_free (vchildren);
    }
}


//!
//! @brief Creats a new tab.  The focus and other details are handled by gw_tabs_new_cb ()
//!
int gw_searchwindow_new_tab (GwSearchWindow *window)
{
    //Declarations
    GtkWidget *scrolledwindow;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;

    //Initializations
    scrolledwindow = GTK_WIDGET (gtk_scrolled_window_new (NULL, NULL));
    buffer = GTK_TEXT_BUFFER (gtk_text_buffer_new (app->tagtable));
    view = GTK_TEXT_VIEW (gtk_text_view_new_with_buffer (buffer));

    //Set up the text buffer
    gtk_text_buffer_get_start_iter (buffer, &iter);
    gtk_text_buffer_create_mark (buffer, "more_relevant_header_mark", &iter, TRUE);
    gtk_text_buffer_create_mark (buffer, "less_relevant_header_mark", &iter, TRUE);
    gtk_text_buffer_create_mark (buffer, "less_rel_content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (buffer, "more_rel_content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (buffer, "content_insertion_mark", &iter, FALSE);
    gtk_text_buffer_create_mark (buffer, "footer_insertion_mark", &iter, FALSE);

    //Set up the text view
    gtk_text_view_set_right_margin (view, 10);
    gtk_text_view_set_left_margin (view, 10);
    gtk_text_view_set_cursor_visible (view, FALSE); 
    gtk_text_view_set_editable (view, FALSE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
    gtk_text_view_set_wrap_mode (view, GTK_WRAP_WORD);

    g_signal_connect (G_OBJECT (view), "drag_motion", G_CALLBACK (gw_searchwindow_drag_motion_1_cb), window->toplevel);
    g_signal_connect (G_OBJECT (view), "button_press_event", G_CALLBACK (gw_searchwindow_get_position_for_button_press_cb), window->toplevel);
    g_signal_connect (G_OBJECT (view), "motion_notify_event", G_CALLBACK (gw_searchwindow_get_iter_for_motion_cb), window->toplevel);
    g_signal_connect (G_OBJECT (view), "drag_drop", G_CALLBACK (gw_searchwindow_drag_drop_1_cb), window->toplevel);
    g_signal_connect (G_OBJECT (view), "button_release_event", G_CALLBACK (gw_searchwindow_get_iter_for_button_release_cb), window->toplevel);
    g_signal_connect (G_OBJECT (view), "drag_leave", G_CALLBACK (gw_searchwindow_drag_leave_1_cb), window->toplevel);
    g_signal_connect (G_OBJECT (view), "drag_data_received", G_CALLBACK (gw_searchwindow_search_drag_data_recieved_cb), window->toplevel);
    g_signal_connect (G_OBJECT (view), "key_press_event", G_CALLBACK (gw_searchwindow_focus_change_on_key_press_cb), window->toplevel);
    g_signal_connect (G_OBJECT (view), "scroll_event", G_CALLBACK (gw_searchwindow_scroll_or_zoom_cb), window->toplevel);


    gtk_container_add (GTK_CONTAINER (scrolledwindow), GTK_WIDGET (view));
    gtk_widget_show_all (GTK_WIDGET (scrolledwindow));

    //Create the tab label
    GtkWidget *vbox;
    GtkWidget *hbox;
    GtkWidget *label;
    GtkWidget *close_button;
    GtkWidget *button_image;
    GtkCssProvider *provider;
    char *style_data;
    GtkStyleContext *context;

    //Initializations
    hbox = GTK_WIDGET (gtk_hbox_new(FALSE, 3));
    label = GTK_WIDGET (gtk_label_new (NULL));
    close_button = GTK_WIDGET (gtk_button_new ());
    button_image = GTK_WIDGET (gtk_image_new_from_stock (GTK_STOCK_CLOSE, GTK_ICON_SIZE_MENU));
    style_data = "* {\n"
                 "-GtkButton-default-border : 0;\n"
                 "-GtkButton-default-outside-border : 0;\n"
                 "-GtkButton-inner-border: 0;\n"
                 "-GtkWidget-focus-line-width : 0;\n"
                 "-GtkWidget-focus-padding : 0;\n"
                 "padding: 0;\n"
                 "}";
    provider = gtk_css_provider_new ();
    context = gtk_widget_get_style_context (close_button);

    //Set up the button
    gtk_button_set_relief (GTK_BUTTON (close_button), GTK_RELIEF_NONE);
    gtk_button_set_focus_on_click (GTK_BUTTON (close_button), FALSE);
    gtk_container_set_border_width (GTK_CONTAINER (close_button), 0);
    gtk_misc_set_padding (GTK_MISC (button_image), 0, 0);
    gtk_widget_set_size_request (GTK_WIDGET (button_image), 14, 14);
    gtk_css_provider_load_from_data (provider,  style_data, strlen(style_data), NULL); 
    gtk_style_context_add_provider (context, GTK_STYLE_PROVIDER (provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref (provider);

    //Put all the elements together
    gtk_container_add (GTK_CONTAINER (close_button), button_image);
    g_signal_connect (G_OBJECT (close_button), "clicked", G_CALLBACK (gw_searchwindow_remove_tab_cb), scrolledwindow);
    vbox = GTK_WIDGET (gtk_vbox_new(FALSE, 0));
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 1);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
    vbox = GTK_WIDGET (gtk_vbox_new(FALSE, 0));
    gtk_box_pack_start (GTK_BOX (vbox), close_button, FALSE, FALSE, 1);
    gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
    gtk_widget_show_all (GTK_WIDGET (hbox));

    //Initializations
    int current;
    int position;

    //Initializations
    current = gtk_notebook_get_current_page (window->notebook);
    position = gtk_notebook_append_page (window->notebook, scrolledwindow, hbox);
    window->tablist = g_list_append (window->tablist, NULL);

    //Put everything together
//    gtk_notebook_set_tab_reorderable (window->notebook, scrolledwindow, TRUE);
    gw_searchwindow_set_font (window);
    gtk_notebook_set_current_page (window->notebook, position);
    gw_searchwindow_set_entry_text_by_searchitem (window, NULL);
    gtk_widget_grab_focus (GTK_WIDGET (window->entry));
    gw_searchwindow_set_current_searchitem (window, NULL);

    return position;
}


void gw_searchwindow_remove_tab (GwSearchWindow *window, int index)
{
    //Sanity check
    g_assert (window != NULL && index > -1);

    //Declarations
    int pages;
    GList *iter;
    LwSearchItem *item;

    //Initializations
    pages = gtk_notebook_get_n_pages (window->notebook);

    //Sanity check
    if (pages <= 1) {
      return;
    }

    gw_searchwindow_cancel_search_by_tab_number (window, index);

    iter = g_list_nth (window->tablist, index);
    if (iter != NULL)
    {
      item = LW_SEARCHITEM (iter->data);
      if (lw_searchitem_has_history_relevance (item, window->keepsearchingdata.enabled))
      {
        lw_history_add_searchitem (window->history, item);
        gw_searchwindow_update_history_popups (window);
      }
      else if (item != NULL)
      {
        lw_searchitem_free (item);
      }
    }
    window->tablist = g_list_delete_link (window->tablist, iter);

    gtk_notebook_remove_page (window->notebook, index);
    gtk_widget_grab_focus (GTK_WIDGET (window->entry));
    gw_searchwindow_sync_current_searchitem (window);
}


void gw_searchwindow_sync_current_searchitem (GwSearchWindow *window)
{
  //Declarations
  LwSearchItem *item;
  
  //Initializations
  item = gw_searchwindow_get_current_searchitem (window);

  gw_searchwindow_set_current_searchitem (window, item);
}


//!
//! @brief The searchwindow searchitem should be set when a new search takes place
//!        using a newly made searchitem.
//!
void gw_searchwindow_set_current_searchitem (GwSearchWindow *window, LwSearchItem *item)
{
    //Declarations
    GtkAction *action;
    GtkLabel *label;
    gboolean enable;
    const char *id;
    GList *link;
    int page_num;

    //Initializations
    page_num = gtk_notebook_get_current_page (window->notebook);
    if (page_num == -1) return;
    link = g_list_nth (window->tablist, page_num);
    if (link == NULL) return;
    link->data = item;

    //Update the window to match the searchitem data
    gw_searchwindow_set_tab_text_by_searchitem (window, item);
    gw_searchwindow_set_dictionary_by_searchitem (window, item);
    gw_searchwindow_set_entry_text_by_searchitem (window, item);
    gw_searchwindow_set_title_by_searchitem (window, item);
    gw_searchwindow_set_total_results_label_by_searchitem (window, item);
    gw_searchwindow_set_search_progressbar_by_searchitem (window, item);

    //Update Save sensitivity state
    id = "file_append_action";
    action = GTK_ACTION (gtk_builder_get_object (window->builder, id));
    enable = (item != NULL);
    gtk_action_set_sensitive (action, enable);

    //Update Save as sensitivity state
    id = "file_save_as_action";
    action = GTK_ACTION (gtk_builder_get_object (window->builder, id));
    enable = (item != NULL);
    gtk_action_set_sensitive (action, enable);

    //Update Print sensitivity state
    id = "file_print_action";
    action = GTK_ACTION (gtk_builder_get_object (window->builder, id));
    enable = (item != NULL);
    gtk_action_set_sensitive (action, enable);

    //Update Print preview sensitivity state
    id = "file_print_preview_action";
    action = GTK_ACTION (gtk_builder_get_object (window->builder, id));
    enable = (item != NULL);
    gtk_action_set_sensitive (action, enable);

    //Set the label's mnemonic widget since glade doesn't seem to want to
    id = "search_entry_label";
    label = GTK_LABEL (gtk_builder_get_object (window->builder, id));
    gtk_label_set_mnemonic_widget (label, GTK_WIDGET (window->entry));
}


//!
//! @brief The searchwindow searchitem will be the one currently loaded in the current tab
//!
LwSearchItem* gw_searchwindow_get_current_searchitem (GwSearchWindow *window)
{
    LwSearchItem *item;
    int page_num;

    page_num = gtk_notebook_get_current_page (window->notebook);
    if (page_num == -1)
      item = NULL;
    else
      item = LW_SEARCHITEM (g_list_nth_data (window->tablist, page_num));

    return item;
}


void gw_searchwindow_start_search (GwSearchWindow *window, LwSearchItem* item)
{
    //Sanity check
    g_assert (window != NULL && item != NULL);

    if (!gw_app_can_start_search (app)) return;

    gw_searchwindow_guarantee_first_tab (window);
    gw_searchwindow_set_current_searchitem (window, item);
    lw_engine_get_results (app->engine, item, TRUE, FALSE);
    gw_searchwindow_update_history_popups (window);
}


//!
//! @brief Sets the requested font with magnification applied
//!
void gw_searchwindow_set_font (GwSearchWindow *window)
{
    //Declarations
    GtkTextView *view;
    gboolean use_global_font_setting;
    int size;
    int magnification;
    char font[50];
    PangoFontDescription *desc;
    int i;
    GtkContainer *container;
    const char *id;
    GtkAction *action;
    gboolean enable;

    //Initializations
    use_global_font_setting = lw_preferences_get_boolean_by_schema (app->preferences, LW_SCHEMA_FONT, LW_KEY_FONT_USE_GLOBAL_FONT);
    magnification = lw_preferences_get_int_by_schema (app->preferences, LW_SCHEMA_FONT, LW_KEY_FONT_MAGNIFICATION);

    if (use_global_font_setting)
      lw_preferences_get_string_by_schema (app->preferences, font, LW_SCHEMA_GNOME_INTERFACE, LW_KEY_DOCUMENT_FONT_NAME, 50);
    else
      lw_preferences_get_string_by_schema (app->preferences, font, LW_SCHEMA_FONT, LW_KEY_FONT_CUSTOM_FONT, 50);

    desc = pango_font_description_from_string (font);
    if (desc != NULL)
    {
      if (pango_font_description_get_size_is_absolute (desc))
        size = pango_font_description_get_size (desc) + magnification;
      else
        size = PANGO_PIXELS (pango_font_description_get_size (desc)) + magnification;

      //Make sure the font size is sane
      if (size < GW_MIN_FONT_SIZE)
        size = GW_MIN_FONT_SIZE;
      else if (size > GW_MAX_FONT_SIZE)
        size = GW_MAX_FONT_SIZE;

      window->font_size = size;

      pango_font_description_set_size (desc, size * PANGO_SCALE);

      //Set it
      i = 0;
      while ((container = GTK_CONTAINER (gtk_notebook_get_nth_page (window->notebook, i))) != NULL)
      {
        view = GTK_TEXT_VIEW (gtk_bin_get_child (GTK_BIN (container)));
        if (view != NULL)
          gtk_widget_override_font (GTK_WIDGET (view), desc);
        i++;
      }

      //Cleanup
      pango_font_description_free (desc);

      //Update Zoom in sensitivity state
      id = "view_zoom_in_action";
      action = GTK_ACTION (gtk_builder_get_object (window->builder, id));
      enable = (magnification < GW_MAX_FONT_MAGNIFICATION);
      gtk_action_set_sensitive (action, enable);

      //Update Zoom out sensitivity state
      id = "view_zoom_out_action";
      action = GTK_ACTION (gtk_builder_get_object (window->builder, id));
      enable = (magnification > GW_MIN_FONT_MAGNIFICATION);
      gtk_action_set_sensitive (action, enable);

      //Update Zoom 100 sensitivity state
      id = "view_zoom_100_action";
      action = GTK_ACTION (gtk_builder_get_object(window->builder, id));
      enable = (magnification != GW_DEFAULT_FONT_MAGNIFICATION);
      gtk_action_set_sensitive (action, enable);
    }
}


