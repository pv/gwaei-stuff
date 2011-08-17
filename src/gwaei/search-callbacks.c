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
//! @file src/gtk/main-callbacks.c
//!
//! @brief Abstraction layer for gtk callbacks
//!
//! Callbacks for activities initiated by the user. Most of the gtk code here
//! should still be abstracted to the interface C file when possible.
//!


#include <string.h>
#include <stdlib.h>
#include <libintl.h>

#include <gdk/gdkkeysyms.h>
#include <gdk/gdk.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>

//!
//! @brief Sets the cursor type depending on the character hovered
//!
//! If the character hovered is a kanji character, the hand turns into a
//! pointer in order to show that the selection is clickable. It will open
//! the kanji sidebar using gw_searchwindow_get_position_for_button_press_cb () and
//! gw_searchwindow_get_iter_for_button_release_cb ().
//! 
//! @see gw_searchwindow_get_position_for_button_press_cb ()
//! @see gw_searchwindow_get_iter_for_button_release_cb ()
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean gw_searchwindow_get_iter_for_motion_cb (GtkWidget      *widget,
                                                                 GdkEventButton *event,
                                                                 gpointer        data   )
{
/*
    //Declarations
    gunichar unic;
    GtkTextIter iter, start, end;
    GwSearchWindow *window;
    gint x;
    gint y;
    GtkTextView *view;
    GtkWidget *tooltip_window;
    LwDictInfo *di;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return FALSE;
    x = event->x;
    y = event->y;
    unic = gw_searchwindow_get_hovered_character (window, &x, &y, &iter);

    //get word
    if (gtk_text_iter_starts_word (&iter) == FALSE)
      gtk_text_iter_backward_word_start (&iter);
    start = iter;
    if (gtk_text_iter_ends_word (&iter) == FALSE)
      gtk_text_iter_forward_word_end (&iter);
    end = iter;
    window->mousedata.hovered_word = gtk_text_iter_get_visible_slice (&start, &end);

    di = lw_dictinfolist_get_dictinfo (LW_DICTINFOLIST (app->dictinfolist), LW_DICTTYPE_KANJI, "Kanji");
    if (di == NULL) return FALSE;
  
    // Characters above 0xFF00 represent inserted images
    if (unic > L'ー' && unic < 0xFF00)
      gw_searchwindow_set_cursor (window, GDK_HAND2);
    else
      gw_searchwindow_set_cursor (window, GDK_XTERM);

    view = gw_searchwindow_get_current_textview (window);
    tooltip_window = GTK_WIDGET (gtk_widget_get_tooltip_window (GTK_WIDGET (view)));

    if (tooltip_window != NULL && window->mousedata.button_character != unic) 
    {
      gtk_widget_destroy (tooltip_window);
      gtk_widget_set_tooltip_window (GTK_WIDGET (view), NULL);
    }
*/
    return FALSE;
}


//!
//! @brief Gets the position of the cursor click and stores it
//!
//! The function stores the location of the button press, but takes no action
//! by itself.  gw_searchwindow_get_iter_for_button_release_cb () uses the saved x and y
//! coordinates and determines if an action should be taken then.
//! 
//! @see gw_searchwindow_get_iter_for_button_release_cb ()
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean gw_searchwindow_get_position_for_button_press_cb (GtkWidget      *widget,
                                                                           GdkEventButton *event,
                                                                           gpointer        data    )
{
    //Declarations
    GtkTextIter iter;
    GwSearchWindow *window;
    int x;
    int y;

    //Window coordinates
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return FALSE;
    window->mousedata.button_press_x = event->x;
    window->mousedata.button_press_y = event->y;
    x = event->x;
    y = event->y;

    gw_searchwindow_get_hovered_character (window, &x, &y, &iter);

    return FALSE;
}


//!
//! @brief Gets the position of the cursor click then opens the kanji sidebar
//!
//! Compares the x and y coordinates fetch by gw_searchwindow_get_position_for_button_press_cb
//! for the cursor, and if the difference is below a certain threshhold,
//! decides if the user wants to open the kanji character under cursor in the
//! kanji sidebar or not.
//! 
//! @see gw_searchwindow_get_position_for_button_press_cb ()
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean gw_searchwindow_get_iter_for_button_release_cb (GtkWidget      *widget,
                                                                         GdkEventButton *event,
                                                                         gpointer        data    )
{
/*
    //Declarations
    GwSearchWindow *window;
    gint x;
    gint y;
    gint trailing;
    GtkTextIter iter;
    gunichar unic;
    LwDictInfo *di;
    GError *error;
    GtkTextView *view;
    GtkWidget *tooltip_window;

    //Initializations
    x = event->x;
    y = event->y;
    trailing = 0;
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return FALSE;
    unic = gw_searchwindow_get_hovered_character (window, &x, &y, &iter);
    di = lw_dictinfolist_get_dictinfo (LW_DICTINFOLIST (app->dictinfolist), LW_DICTTYPE_KANJI, "Kanji");
    error = NULL;


    //Sanity cehck
    if (di == NULL) return FALSE;

    if (abs (window->mousedata.button_press_x - x) < 3 && abs (window->mousedata.button_press_y - y) < 3)
    {
      // Characters above 0xFF00 represent inserted images
      if (unic > L'ー' && unic < 0xFF00 )
      {
        //Convert the unicode character into to a utf8 string
        gchar query[7];
        gint length = g_unichar_to_utf8 (unic, query);
        query[length] = '\0'; 


        view = gw_searchwindow_get_current_textview (window);
        tooltip_window = GTK_WIDGET (gtk_widget_get_tooltip_window (GTK_WIDGET (view)));

        if (tooltip_window == NULL) {
          window->mousedata.button_character = unic;
          tooltip_window = gtk_window_new (GTK_WINDOW_POPUP);
          gtk_window_set_skip_taskbar_hint (GTK_WINDOW (tooltip_window), TRUE);
          gtk_window_set_skip_pager_hint (GTK_WINDOW (tooltip_window), TRUE);
          gtk_window_set_accept_focus (GTK_WINDOW (tooltip_window), FALSE);
          gtk_widget_set_tooltip_window (GTK_WIDGET (view), GTK_WINDOW (tooltip_window));
          gtk_window_set_transient_for (GTK_WINDOW (tooltip_window), NULL);
          gtk_window_set_type_hint (GTK_WINDOW (tooltip_window), GDK_WINDOW_TYPE_HINT_TOOLTIP);
          gtk_widget_set_name (GTK_WIDGET (tooltip_window), "gtk-tooltip");
        }
        if (tooltip_window != NULL) {
          window->mousedata.button_character = unic;

          //Start the search
          if (window->mousedata.item != NULL)
          {
            gw_searchwindow_cancel_search_by_searchitem (window, window->mousedata.item);
            window->mousedata.item = NULL;
          }

          window->mousedata.item = lw_searchitem_new (query, di, LW_OUTPUTTARGET_KANJI, app->prefmanager, &error);
          lw_engine_get_results (app->engine, window->mousedata.item, TRUE, FALSE);

          g_thread_join (window->mousedata.item->thread); 

          int winx, winy;
          gtk_window_get_position (GTK_WINDOW (tooltip_window), &winx, &winy);
          gtk_window_move (GTK_WINDOW (tooltip_window), event->x_root + winx - 3, event->y_root + winy - 3);
          gtk_widget_show_now (GTK_WIDGET (tooltip_window));
        }
      }
      else {
        view = gw_searchwindow_get_current_textview (window);
        tooltip_window = GTK_WIDGET (gtk_widget_get_tooltip_window (GTK_WIDGET (view)));
        if (tooltip_window != NULL && window->mousedata.button_character != unic) 
        {
          gtk_widget_set_tooltip_window (GTK_WIDGET (view), NULL);
          gtk_widget_destroy (tooltip_window);
        }
      }

      if (error != NULL)
      {
        fprintf(stderr, "%s\n", error->message);
        g_error_free (error);
      }
    }
*/
    return FALSE; 
}


//!
//! @brief Closes the window passed throught the widget pointer
//!
//! This function closes the window passed through the widget pointer.
//! Depending if it is a specific window, it will save it's coordinates
//! or take other special actions before closing.
//! 
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_close_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    int pages;
    
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    pages = gtk_notebook_get_n_pages (window->notebook);

    if (pages > 1)
      gw_searchwindow_remove_current_tab_cb (widget, data);
    else
      gw_app_destroy_window (app, GW_WINDOW_SEARCH, NULL);
}


//!
//! @brief Preforms the action the window manager close event
//!
//! This function currently acts as a proxy for the gw_searchwindow_close_cb () function.
//! 
//! @see gw_searchwindow_close_cb ()
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//! @return Always returns true
//!
G_MODULE_EXPORT gboolean gw_searchwindow_delete_event_action_cb (GtkWidget *widget, gpointer data)
{ 
    gw_searchwindow_close_cb (widget, data);
    return TRUE;
}


//!
//! @brief Quits out of the application
//! 
//! @see gw_app_quit ()
//! @param widget GtkWidget pointer to the window to close
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_quit_cb (GtkWidget *widget, gpointer data)
{
    gw_app_quit (app);
}


//!
//! @brief Preforms a search from the history.
//!
//! The function uses the gpointer data to fetch a LwSearchItem that was pased
//! to the function for the search.  It will reflow the back and forward
//! history lists so the LwSearchItem is in the current position of the
//! Historylist.
//! 
//! @see gw_searchwindow_search_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached LwSearchItem variable
//!
G_MODULE_EXPORT void gw_searchwindow_search_from_history_cb (GtkWidget *widget, gpointer data)
{
/*
    LwSearchItem *item;
    GwSearchWindow *window;
    GwSearchData *sdata;
    LwHistoryList *history;

    history = app->history;
    item = (LwSearchItem*) data;
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;

    gw_searchwindow_guarantee_first_tab (window);

    //Update the textview pointed to in case the user switched tabs
    sdata = lw_searchitem_get_data (item);
    sdata->view = gw_searchwindow_get_current_textview (window);
    
    //Checks to make sure everything is sane
    if (gw_searchwindow_cancel_search_for_current_tab (window) == FALSE)
    {
      fprintf(stderr, "CANCEL SEARCH FOR CURRENT TAB RETURNED FALSE\n");
      return;
    }

    //Start setting things up;
    if (history->back != NULL && g_list_find (history->back, item))
    {
      while (history->back != NULL && history->current != item)
        lw_historylist_go_back (history);
    }
    else if (history->forward != NULL && g_list_find (history->forward, item))
    {
      while (history->forward != NULL && history->current != item)
        lw_historylist_go_forward (history);
    }

    //Set tab text
    gw_searchwindow_set_current_tab_text (window, item->queryline->string);

    //Add tab reference to searchitem
    gw_searchwindow_set_current_searchitem (window, item);
    lw_engine_get_results (app->engine, item, TRUE, FALSE);
    gw_searchwindow_update_history_popups (window);
    gw_searchwindow_update_toolbar_buttons (window);

    //Set the search string in the GtkEntry
    gw_searchwindow_clear_search_entry (window);
    gw_searchwindow_entry_insert (window, item->queryline->string);
    gw_searchwindow_select_all_by_target (window, LW_OUTPUTTARGET_ENTRY);
    gtk_widget_grab_focus (GTK_WIDGET (window->entry));

    //Set the correct dictionary in the gui
    gtk_combo_box_set_active (window->combobox, item->dictionary->load_position);
*/
}


//!
//! @brief Goes back one step in the search history
//! 
//! This function checks the top of the back historylist and uses the
//! LwSearchItem in it to invoke gw_searchwindow_search_from_history_cb () using it.
//!
//! @see gw_searchwindow_search_from_history_cb ()
//! @see gw_searchwindow_forward_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached LwSearchItem variable
//!
G_MODULE_EXPORT void gw_searchwindow_back_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));

    if (lw_historylist_has_back (window->history))
    {
      gw_searchwindow_search_from_history_cb (NULL, window->history->back->data);
    }
}


//!
//! @brief Goes forward one step in the search history
//! 
//! This function checks the top of the forward historylist and uses the
//! LwSearchItem in it to invoke gw_searchwindow_search_from_history_cb () using it.
//!
//! @see gw_searchwindow_search_from_history_cb ()
//! @see gw_searchwindow_back_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data pointer to a specially attached LwSearchItem variable
//!
G_MODULE_EXPORT void gw_searchwindow_forward_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));

    if (lw_historylist_has_forward (window->history))
    {
      gw_searchwindow_search_from_history_cb (NULL, window->history->forward->data);
    }
}


//!
//! @brief Saves the current search results to a file
//! 
//! The function gets the current contents of the results text view and saves
//! it to a file, overwriting it if it already exists.  If part of the results
//! are highlighted, only that gets saved.
//!
//! @see gw_searchwindow_save_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_save_as_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    const gchar *path;
    GtkWidget *dialog;
    GtkAction *edit;
    gchar *text;
    gchar *temp;
    GError *error;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;
    path = lw_io_get_savepath ();
    temp = NULL;
    text = gw_searchwindow_get_text_by_target (window, LW_OUTPUTTARGET_RESULTS);
    dialog = gtk_file_chooser_dialog_new (gettext ("Save As"),
                GTK_WINDOW (window->toplevel),
                GTK_FILE_CHOOSER_ACTION_SAVE,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                NULL);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);
    error = NULL;

    //Set the default save path if none is set
    if (path == NULL)
    {
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "");
        gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), gettext ("vocabulary.txt"));
    }
    //Otherwise use the already existing one
    else
    {
        gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), path);
    }

    //Run the save as dialog
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        //Set the new savepath
        temp = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        lw_io_set_savepath (temp);
        g_free (temp);
        temp = NULL;
        path = lw_io_get_savepath ();

        lw_io_write_file (path, "w", text, NULL, NULL, &error);

        edit = GTK_ACTION (gtk_builder_get_object (window->builder, "file_edit_action"));
        gtk_action_set_sensitive (edit, TRUE);
    }

    //Cleanup
    gtk_widget_destroy (dialog);
    g_free (text);
    text = NULL;
    gw_common_handle_error (&error, GTK_WINDOW (window->toplevel), TRUE);
}


//!
//! @brief Appends the current search results to a file
//! 
//! The function gets the current contents of the results text view and appends
//! it to a file.  If the user has already saved once, it will automatically
//! keep appending to the same file. If part of the results are highlighted,
//! only that gets appended.
//!
//! @see gw_searchwindow_save_as_cb ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//0
G_MODULE_EXPORT void gw_searchwindow_save_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    gchar *text;
    const gchar *path;
    GError *error;
    GwSearchWindow* window;

    //Initializations
    path = lw_io_get_savepath ();
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;
    error = NULL;

    //Sanity check for an empty save path
    if (path == NULL || *path == '\0')
    {
      gw_searchwindow_save_as_cb (widget, data);
      return;
    }

    //Carry out the save
    text = gw_searchwindow_get_text_by_target (window, LW_OUTPUTTARGET_RESULTS);
    lw_io_write_file (path, "a", text, NULL, NULL, &error);
    g_free (text);
    text = NULL;

    gw_common_handle_error (&error, GTK_WINDOW (window->toplevel), TRUE);
}


//!
//! @brief Makes the text in the text buffer enlarge
//! 
//! Determines if the text size is smaller than the max possible text size,
//! and then sets the pref in gconf which will trigger the font size setting
//! function.
//!
//! @see gw_searchwindow_set_font()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_zoom_in_cb (GtkWidget *widget, gpointer data)
{
    int size;
    size = lw_prefmanager_get_int_by_schema (app->prefmanager, LW_SCHEMA_FONT, LW_KEY_FONT_MAGNIFICATION) + GW_FONT_ZOOM_STEP;
    if (size <= GW_MAX_FONT_MAGNIFICATION)
    {
      lw_prefmanager_set_int_by_schema (app->prefmanager, LW_SCHEMA_FONT, LW_KEY_FONT_MAGNIFICATION, size);
      printf("BREAK zoom in\n");
    }
}


//!
//! @brief Makes the text in the text buffer shrink
//! 
//! Determines if the text size is larger than the min possible text size,
//! and then sets the pref in gconf which will trigger the font size setting
//! function.
//!
//! @see gw_searchwindow_set_font()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_zoom_out_cb (GtkWidget *widget, gpointer data)
{
    int size;
    size = lw_prefmanager_get_int_by_schema (app->prefmanager, LW_SCHEMA_FONT, LW_KEY_FONT_MAGNIFICATION) - GW_FONT_ZOOM_STEP;
    if (size >= GW_MIN_FONT_MAGNIFICATION)
    {
      lw_prefmanager_set_int_by_schema (app->prefmanager, LW_SCHEMA_FONT, LW_KEY_FONT_MAGNIFICATION, size);
      printf("BREAK zoom out\n");
    }
}


//!
//! @brief Resets the text size to the default in the text buffers
//! 
//! The function acts gconf for the default font size from the schema, and then
//! sets it, which makes gconf call the font size setting function since the
//! stored value changed.
//!
//! @see gw_searchwindow_set_font()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_zoom_100_cb (GtkWidget *widget, gpointer data)
{
    lw_prefmanager_reset_value_by_schema (app->prefmanager, LW_SCHEMA_FONT, LW_KEY_FONT_MAGNIFICATION);
}


//!
//! @brief Sets the less relevant results show boolean
//! 
//! Makes the gconf pref match the current state of the triggering widget.
//! Each separate LwSearchItem stores this individually, so even if you flip
//! this, you will need to do a new search if you want to change how things are
//! displayed.
//!
//! @see gw_searchwindow_set_less_relevant_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_less_relevant_results_toggle_cb (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = lw_prefmanager_get_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_LESS_RELEVANT_SHOW);
    lw_prefmanager_set_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_LESS_RELEVANT_SHOW, !state);
}


G_MODULE_EXPORT void gw_searchwindow_dictionary_combobox_changed_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    int active;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;
    active = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));

    gw_searchwindow_set_dictionary (window, active);

    gtk_widget_grab_focus (GTK_WIDGET (window->entry));
}

//!
//! @brief Changes the selected dictionary in the dictionarylist
//! 
//! This function makes the selected dictionary in the dictionarylist match
//! the dictionary of the widget that was modified.  The selected dictionary
//! in the dictionarylist acts as the central reference for the GUI.
//!
//! @param widget pointer to the GtkWidget that changed dictionaries
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_dictionary_radio_changed_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GList *list;
    GList *iter;
    GtkMenuShell *shell;
    GtkCheckMenuItem *check_menu_item;
    int active;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    shell = GTK_MENU_SHELL (gtk_builder_get_object (window->builder, "dictionary_popup"));
    active = 0;

    list = gtk_container_get_children (GTK_CONTAINER (shell));
    for (iter = list; iter != NULL; iter = iter->next)
    {
      check_menu_item = GTK_CHECK_MENU_ITEM (iter->data);
      if (gtk_check_menu_item_get_active (check_menu_item) == TRUE)
        break;
      active++;
    }
    g_list_free (list);

    //Finish up
    gw_searchwindow_set_dictionary (window, active);
}


//!
//! @brief Selects all the text in the current widget
//! 
//! This function makes the selected dictionary in the dictionarylist match
//! the dictionary of the widget that was modified.  The selected dictionary
//! in the dictionarylist acts as the central reference for the GUI.
//!
//! @see gw_searchwindow_cut_cb ()
//! @see gw_searchwindow_copy_cb ()
//! @see gw_searchwindow_paste_cb ()
//! @see gw_searchwindow_update_clipboard_on_focus_change_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_select_all_cb (GtkWidget *widget, gpointer data)
{
    guint TARGET;
    GwSearchWindow *window;

    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;
    TARGET = gw_searchwindow_get_current_target_focus (window);

    gw_searchwindow_select_all_by_target (window, TARGET);
}


//!
//! @brief Pastes text into the current widget
//! 
//! This function makes the selected dictionary in the dictionarylist match
//! the dictionary of the widget that was modified.  The selected dictionary
//! in the dictionarylist acts as the central reference for the GUI.
//!
//! @see gw_searchwindow_cut_cb ()
//! @see gw_searchwindow_copy_cb ()
//! @see gw_searchwindow_select_all_cb ()
//! @see gw_searchwindow_update_clipboard_on_focus_change_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_paste_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    guint TARGET;

    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    TARGET = gw_searchwindow_get_current_target_focus (window);

    gw_searchwindow_paste_text (window, TARGET);
}


//!
//! @brief Cuts text from the current widget
//! 
//! This function makes the selected dictionary in the dictionarylist match
//! the dictionary of the widget that was modified.  The selected dictionary
//! in the dictionarylist acts as the central reference for the GUI.
//!
//! @see gw_searchwindow_paste_cb ()
//! @see gw_searchwindow_copy_cb ()
//! @see gw_searchwindow_select_all_cb ()
//! @see gw_searchwindow_update_clipboard_on_focus_change_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_cut_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    guint TARGET;

    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    TARGET = gw_searchwindow_get_current_target_focus (window);

    gw_searchwindow_cut_text (window, TARGET);
}


//!
//! @brief Pastes text into the current widget
//! 
//! This function makes the selected dictionary in the dictionarylist match
//! the dictionary of the widget that was modified.  The selected dictionary
//! in the dictionarylist acts as the central reference for the GUI.
//!
//! @see gw_searchwindow_cut_cb ()
//! @see gw_searchwindow_paste_cb ()
//! @see gw_searchwindow_select_all_cb ()
//! @see gw_searchwindow_update_clipboard_on_focus_change_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_copy_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    guint TARGET;

    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    TARGET = gw_searchwindow_get_current_target_focus (window);

    gw_searchwindow_copy_text (window, TARGET);
}


//!
//! @brief Manages the required changes for focus in different elements
//! 
//! Depending if the object's text is editable or not, the clipboard will
//! update button states approprately and connect the signal handlers to the
//! approprate widgets.
//!
//! @see gw_searchwindow_cut_cb ()
//! @see gw_searchwindow_copy_cb ()
//! @see gw_searchwindow_paste_cb ()
//! @see gw_searchwindow_select_all_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean gw_searchwindow_update_clipboard_on_focus_change_cb (GtkWidget        *widget, 
                                                                              GtkDirectionType  arg1,
                                                                              gpointer          data   ) 
{
    GwSearchWindow *window;
    GtkAction *copy_action, *cut_action, *paste_action, *select_all_action;
    GtkTextView *view;
    char *id;

    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return FALSE;
    view = gw_searchwindow_get_current_textview (window);
    id = "edit_copy_action";
    copy_action = GTK_ACTION (gtk_builder_get_object (window->builder, id));
    id = "edit_cut_action";
    cut_action = GTK_ACTION (gtk_builder_get_object (window->builder, id));
    id = "edit_paste_action";
    paste_action = GTK_ACTION (gtk_builder_get_object (window->builder, id));
    id = "edit_select_all_action";
    select_all_action = GTK_ACTION (gtk_builder_get_object (window->builder, id));

    //Disconnected old handlers
    if (window->signalids[GW_SEARCHWINDOW_SIGNALID_COPY] != 0)
    {
      g_signal_handler_disconnect (copy_action, window->signalids[GW_SEARCHWINDOW_SIGNALID_COPY]);
      window->signalids[GW_SEARCHWINDOW_SIGNALID_COPY] = 0;
    }
    if (window->signalids[GW_SEARCHWINDOW_SIGNALID_CUT] != 0)
    {
      g_signal_handler_disconnect (cut_action, window->signalids[GW_SEARCHWINDOW_SIGNALID_CUT]);
      window->signalids[GW_SEARCHWINDOW_SIGNALID_CUT] = 0;
    }
    if (window->signalids[GW_SEARCHWINDOW_SIGNALID_PASTE] != 0)
    {
      g_signal_handler_disconnect (paste_action, window->signalids[GW_SEARCHWINDOW_SIGNALID_PASTE]);
      window->signalids[GW_SEARCHWINDOW_SIGNALID_PASTE] = 0;
    }
    if (window->signalids[GW_SEARCHWINDOW_SIGNALID_SELECT_ALL] != 0)
    {
      g_signal_handler_disconnect (select_all_action, window->signalids[GW_SEARCHWINDOW_SIGNALID_SELECT_ALL]);
      window->signalids[GW_SEARCHWINDOW_SIGNALID_SELECT_ALL] = 0;
    }
                                          
    //Create new ones pointed at the correct widget
    window->signalids[GW_SEARCHWINDOW_SIGNALID_COPY] = g_signal_connect (
          copy_action,
          "activate",
          G_CALLBACK (gw_searchwindow_copy_cb),
          widget
    );
    window->signalids[GW_SEARCHWINDOW_SIGNALID_CUT] = g_signal_connect (
          cut_action,
          "activate",
          G_CALLBACK (gw_searchwindow_cut_cb),
          widget
    );
    window->signalids[GW_SEARCHWINDOW_SIGNALID_PASTE] = g_signal_connect (
          paste_action,
          "activate",
          G_CALLBACK (gw_searchwindow_paste_cb),
          widget
    );
    window->signalids[GW_SEARCHWINDOW_SIGNALID_SELECT_ALL] = g_signal_connect (
          select_all_action,
          "activate",
          G_CALLBACK (gw_searchwindow_select_all_cb),
          widget
    );


    //Correct the sensitive state to paste
    if (GTK_WIDGET (data) == GTK_WIDGET (view))
      gtk_action_set_sensitive (GTK_ACTION (paste_action), FALSE);
    else
      gtk_action_set_sensitive (GTK_ACTION (paste_action), TRUE);

    return FALSE;
}


//!
//! @brief Opens the saved vocab list in your default editor
//! 
//! If the user saved a vocab list using the save as or append functions, this
//! action becomes available where the file is opened in the user's default
//! text editor.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_edit_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    char *uri;
    GError *error;
    const char *savepath;
    GwSearchWindow *window;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;
    savepath = lw_io_get_savepath ();
    uri = g_build_filename ("file://", savepath, NULL);
    error = NULL;

    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &error);

    gw_common_handle_error (&error, GTK_WINDOW (window->toplevel), TRUE);

    //Cleanup
    g_free (uri);
}


//!
//! @brief Sends the user to the gWaei irc channel for help
//! 
//! The IRC uri should open in the user's default IRC client.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_irc_channel_cb (GtkWidget *widget, gpointer data)
{
    //Initializations
    GError *error;
    GwSearchWindow *window;

    //Declarations
    error = NULL;
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;

    gtk_show_uri (NULL, "irc://irc.freenode.net/gWaei", gtk_get_current_event_time (), &error);

    //Cleanup
    gw_common_handle_error (&error, GTK_WINDOW (window->toplevel), TRUE);
}


//!
//! @brief Sends the user to the gWaei homepage for whatever they need
//! 
//! The homepage should open in their default browser.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_homepage_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GError *error;
    GwSearchWindow *window;

    //Initializations
    error = NULL;
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;

    gtk_show_uri (NULL, "http://gwaei.sourceforge.net/", gtk_get_current_event_time (), &error);

    //Cleanup
    gw_common_handle_error (&error, GTK_WINDOW (window->toplevel), TRUE);
}


//!
//! @brief Opens the gWaei help documentation
//!
//! The gWaei help documentation is opened in the user's default help program.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_help_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GError *error;
    GwSearchWindow *window;

    //Initializations
    error = NULL;
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;

    gtk_show_uri (NULL, "ghelp:gwaei", gtk_get_current_event_time (), &error);

    //Cleanup
    gw_common_handle_error (&error, GTK_WINDOW (window->toplevel), TRUE);
}


//!
//! @brief Opens the gWaei dictionary glossary help documentation
//!
//! The gWaei dictionary glossary help documentation is opened in the user's
//! default help program.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_glossary_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    char *uri;
    GError *error;
    GwSearchWindow *window;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;
    uri = g_build_filename ("ghelp://", DATADIR2, "gnome", "help", "gwaei", "C", "glossary.xml", NULL);
    error = NULL;

    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &error);

    //Cleanup
    gw_common_handle_error (&error, GTK_WINDOW (window->toplevel), TRUE);
    g_free (uri);
}


//!
//! @brief Opens the gWaei about dialog
//!
//! The gWaei help dialog is displayed, showing the credits of everyone who has
//! helped to make this program possible.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_about_cb (GtkWidget *widget, gpointer data)
{
    char *global_path = DATADIR2 G_DIR_SEPARATOR_S PACKAGE G_DIR_SEPARATOR_S "logo.png";
    char *local_path = ".." G_DIR_SEPARATOR_S "share" G_DIR_SEPARATOR_S PACKAGE G_DIR_SEPARATOR_S "logo.png";

    char *programmer_credits[] = 
    {
      "Zachary Dovel <pizzach@gmail.com>",
      "Fabrizio Sabatini",
      NULL
    };

    GdkPixbuf *logo;
    if ( (logo = gdk_pixbuf_new_from_file (global_path,    NULL)) == NULL &&
         (logo = gdk_pixbuf_new_from_file (local_path, NULL)) == NULL    )
    {
      printf ("Was unable to load the gwaei logo.\n");
    }

    GtkWidget *about = g_object_new (GTK_TYPE_ABOUT_DIALOG,
               "program-name", "gWaei", 
               "version", VERSION,
               "copyright", "gWaei (C) 2008-2010 Zachary Dovel\nKanjipad backend (C) 2002 Owen Taylor\nJStroke backend (C) 1997 Robert Wells",
               "comments", gettext("Program for Japanese translation and reference. The\ndictionaries are supplied by Jim Breen's WWWJDIC.\nSpecial thanks to the maker of GJITEN who served as an inspiration.\n Dedicated to Chuus"),
               "license", "This software is GPL Licensed.\n\ngWaei is free software: you can redistribute it and/or modify\nit under the terms of the GNU General Public License as published by\n the Free Software Foundation, either version 3 of the License, or\n(at your option) any later version.\n\ngWaei is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License\nalong with gWaei.  If not, see <http://www.gnu.org/licenses/>.",
               "logo", logo,
               // TRANSLATORS: You can add your own name to the translation of this field, it will be displayed in the "about" box when gwaei is run in your language
               "translator-credits", gettext("translator-credits"),
               "authors", programmer_credits,
               "website", "http://gwaei.sourceforge.net/",
               NULL);
    gtk_dialog_run (GTK_DIALOG (about));
    g_object_unref (logo);
    gtk_widget_destroy (about);
}


//!
//! @brief Cycles the active dictionaries down the list
//!
//! This function cycles the dictionaries down the list.  If it reaches the
//! end, it will loop back to the top.
//!
//! @see gw_searchwindow_cycle_dictionaries_backward_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_cycle_dictionaries_forward_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;
    gw_searchwindow_cycle_dictionaries (window, TRUE);
}


//!
//! @brief Cycles the active dictionaries up the list
//!
//! This function cycles the dictionaries up the list.  If it reaches the
//! end, it will loop back to the bottom.
//!
//! @see gw_searchwindow_cycle_dictionaries_forward_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_cycle_dictionaries_backward_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;
    gw_searchwindow_cycle_dictionaries (window, FALSE);
}


//!
//! @brief Update the special key press status
//!
//! Currently used to determine if a search should be opened in a new tab.
//!
//! @param widget Unused GtkWidget pointer
//! @param event the event data to get the specific key that had it's status modified
//! @param data Currently unused gpointer
//! @return Always returns FALSE
//!
G_MODULE_EXPORT gboolean gw_searchwindow_key_press_modify_status_update_cb (GtkWidget *widget,
                                                                            GdkEvent  *event,
                                                                            gpointer  *data  )
{
/*
    //Declarations
    GwSearchWindow *window;
    GtkTextView *view;
    GtkWidget *tooltip_window;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return FALSE;
    view = gw_searchwindow_get_current_textview (window);
    tooltip_window = GTK_WIDGET (gtk_widget_get_tooltip_window (GTK_WIDGET (view)));

    if (tooltip_window != NULL) 
    {
      gtk_widget_destroy (tooltip_window);
      gtk_widget_set_tooltip_window (GTK_WIDGET (view), NULL);
    }

    guint keyval = ((GdkEventKey*)event)->keyval;

    if ((keyval == GDK_KEY_ISO_Enter || keyval == GDK_KEY_Return) && gtk_widget_is_focus (GTK_WIDGET (window->entry)))
    {
      gtk_widget_activate (GTK_WIDGET (window->entry));
      return FALSE;
    }

    if (keyval == GDK_KEY_Shift_L || keyval == GDK_KEY_Shift_R || keyval == GDK_KEY_ISO_Next_Group || keyval == GDK_KEY_ISO_Prev_Group)
    {
      window->new_tab = TRUE;
    }
*/
    return FALSE;
}


//!
//! @brief Update the special key release status
//!
//! Currently used to determine if a search should be opened in a new tab.
//!
//! @param widget Unused GtkWidget pointer
//! @param event the event data to get the specific key that had it's status modified
//! @param data Currently unused gpointer
//! @return Always returns FALSE
//!
G_MODULE_EXPORT gboolean gw_searchwindow_key_release_modify_status_update_cb (GtkWidget *widget,
                                                                      GdkEvent  *event,
                                                                      gpointer  *data  )
{
    //Declarations
    GwSearchWindow *window;
    guint keyval;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return FALSE;
    keyval = ((GdkEventKey*)event)->keyval;

    if (keyval == GDK_KEY_Shift_L || keyval == GDK_KEY_Shift_R || keyval == GDK_KEY_ISO_Next_Group || keyval == GDK_KEY_ISO_Prev_Group)
    {
      window->new_tab = FALSE;
    }

    return FALSE;
}


//!
//! @brief Function handles automatic focus changes on key presses
//!
//! When the user types a letter, the focus will move to the search entry and
//! auto-highlight the results so you can start typing immediately.  If the
//! user hits an arrow key or pageup/pagedown, the focus will move to the
//! search results so they can scroll them.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//! @return Always returns false
//!
G_MODULE_EXPORT gboolean gw_searchwindow_focus_change_on_key_press_cb (GtkWidget *widget,
                                                       GdkEvent  *event,
                                                       gpointer  *focus  )
{
    GwSearchWindow *window;
    guint state;
    guint keyval;
    guint modifiers;
    GtkTextView *view;

    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return FALSE;
    state = ((GdkEventKey*)event)->state;
    keyval = ((GdkEventKey*)event)->keyval;
    modifiers = ( 
       GDK_MOD1_MASK    |
       GDK_CONTROL_MASK |
       GDK_SUPER_MASK   |
       GDK_HYPER_MASK   |
       GDK_META_MASK    |
       GDK_KEY_Meta_L   |
       GDK_KEY_Meta_R   |
       GDK_KEY_Alt_L    |
       GDK_KEY_Alt_R
    );
    view = gw_searchwindow_get_current_textview (window);

    //Make sure no modifier keys are pressed
    if (
          (state & modifiers) == 0   &&
          keyval != GDK_KEY_Tab          &&
          keyval != GDK_KEY_ISO_Left_Tab &&
          keyval != GDK_KEY_Shift_L      &&
          keyval != GDK_KEY_Shift_R      &&
          keyval != GDK_KEY_Control_L    &&
          keyval != GDK_KEY_Control_R    &&
          keyval != GDK_KEY_Caps_Lock    &&
          keyval != GDK_KEY_Shift_Lock   &&
          keyval != GDK_KEY_Meta_L       &&
          keyval != GDK_KEY_Meta_R       &&
          keyval != GDK_KEY_Alt_L        &&
          keyval != GDK_KEY_Alt_R        &&
          keyval != GDK_KEY_Super_L      &&
          keyval != GDK_KEY_Super_R      &&
          keyval != GDK_KEY_Hyper_L      &&
          keyval != GDK_KEY_Hyper_R      &&
          keyval != GDK_KEY_Num_Lock     &&
          keyval != GDK_KEY_Scroll_Lock  &&
          keyval != GDK_KEY_Pause        &&
          keyval != GDK_KEY_Home         &&
          keyval != GDK_KEY_End
       )
    {
      //Change focus to the text view if is an arrow key or page key
      if ( 
           ( keyval == GDK_KEY_Up        ||
             keyval == GDK_KEY_Down      ||
             keyval == GDK_KEY_Page_Up   ||
             keyval == GDK_KEY_Page_Down   
           ) &&
           (
             widget != GTK_WIDGET (view)
           )
         )
      {
        gw_searchwindow_select_none_by_target (window, LW_OUTPUTTARGET_ENTRY);
        gtk_widget_grab_focus (GTK_WIDGET (view));
        return TRUE;
      }

      //Change focus to the entry if other key
      else if (
                keyval != GDK_KEY_Up        &&
                keyval != GDK_KEY_Down      &&
                keyval != GDK_KEY_Page_Up   &&
                keyval != GDK_KEY_Page_Down &&
                widget != GTK_WIDGET (window->entry)
              )
      {
        gw_searchwindow_select_all_by_target (window, LW_OUTPUTTARGET_ENTRY);
        gtk_widget_grab_focus (GTK_WIDGET (window->entry));
        return TRUE;
      }
    }

    return FALSE;
}


//!
//! @brief Initiates a search on the user's typed query
//!
//! This function does the needed work to check the query for basic
//! correctness, shift the previously completed search to the history list,
//! creates the searchitem, and then initiates the search.
//!
//! @see gw_searchwindow_search_from_history_cb ()
//! @see lw_search_get_results ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_search_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    char query[50];
    LwSearchItem *item;
    LwSearchItem *new_item;
    LwDictInfo *di;
    GError *error;
    GwSearchData *sdata;
    GtkTextView *view;

    //Initializations
    error = NULL;
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;
    strncpy (query, gtk_entry_get_text (window->entry), 50);
    item = gw_searchwindow_get_current_searchitem (window);
    di = gw_searchwindow_get_dictionary (window);
    gw_searchwindow_guarantee_first_tab (window);

    //Cancel all searches if the search bar is empty
    if (strlen(query) == 0 || di == NULL) 
    {
      gw_searchwindow_cancel_search_by_searchitem (window, item);
      return;
    }

    view = gw_searchwindow_get_current_textview (window);
    new_item = lw_searchitem_new (query, di, LW_OUTPUTTARGET_RESULTS, app->prefmanager, &error);
    sdata = gw_searchdata_new (view, window);
    lw_searchitem_set_data (new_item, sdata, LW_SEARCHITEM_DATA_FREE_FUNC (gw_searchdata_free));

    //Check for problems, and quit if there are
    if (error != NULL ||
        new_item == NULL ||
        lw_searchitem_is_equal (item, new_item) ||
        !gw_searchwindow_cancel_search_by_searchitem (window, item)
       )
    {
      lw_searchitem_increment_history_relevance_timer (item);
      if (new_item != NULL)
        lw_searchitem_free (new_item);

      if (error != NULL)
      {
        fprintf(stderr, "%s\n", error->message);
        g_error_free (error);
      }

      return;
    }

    //Push the previous searchitem or replace it with the new one
    if (item != NULL && lw_searchitem_has_history_relevance (item))
      lw_historylist_add_searchitem (window->history, item);
    else if (item != NULL)
      lw_searchitem_free (item);

    gw_searchwindow_start_search (window, new_item);
}


//!
//! @brief Inserts an unknown regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert a period
//! wherever the cursor presently is in the search entry.
//!
//! @see gw_searchwindow_insert_word_edge_cb ()
//! @see gw_searchwindow_insert_not_word_edge_cb ()
//! @see gw_searchwindow_insert_and_cb ()
//! @see gw_searchwindow_insert_or_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_insert_unknown_character_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    gw_searchwindow_entry_insert (window, ".");
}


//!
//! @brief Inserts an a word-boundary regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert \\b
//! wherever the cursor presently is in the search entry.
//!
//! @see gw_searchwindow_insert_unknown_character_cb ()
//! @see gw_searchwindow_insert_not_word_edge_cb ()
//! @see gw_searchwindow_insert_and_cb ()
//! @see gw_searchwindow_insert_or_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_insert_word_edge_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    gw_searchwindow_entry_insert (window, "\\b");
}


//!
//! @brief Inserts an a not-word-boundary regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert \\B
//! wherever the cursor presently is in the search entry.
//!
//! @see gw_searchwindow_insert_unknown_character_cb ()
//! @see gw_searchwindow_insert_word_edge_cb ()
//! @see gw_searchwindow_insert_and_cb ()
//! @see gw_searchwindow_insert_or_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_insert_not_word_edge_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    gw_searchwindow_entry_insert (window, "\\B");
}


//!
//! @brief Inserts an an and regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert &
//! wherever the cursor presently is in the search entry.
//!
//! @see gw_searchwindow_insert_unknown_character_cb ()
//! @see gw_searchwindow_insert_word_edge_cb ()
//! @see gw_searchwindow_insert_not_word_edge_cb ()
//! @see gw_searchwindow_insert_or_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_insert_and_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    gw_searchwindow_entry_insert (window, "&");
}


//!
//! @brief Inserts an an or regex character into the entry
//!
//! Used to help users discover regex searches.  It just insert |
//! wherever the cursor presently is in the search entry.
//!
//! @see gw_searchwindow_insert_unknown_character_cb ()
//! @see gw_searchwindow_insert_word_edge_cb ()
//! @see gw_searchwindow_insert_not_word_edge_cb ()
//! @see gw_searchwindow_insert_and_cb ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_insert_or_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    gw_searchwindow_entry_insert (window, "|");
}


//!
//! @brief Clears the search entry and moves the focus to it
//!
//! This function acts as a quick way for the user to get back to the search
//! entry and do another search whereever they are.
//!
//! @see gw_searchwindow_clear_search_entry ()
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_clear_search_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    gw_searchwindow_clear_search_entry (window);
    gtk_widget_grab_focus (GTK_WIDGET (window->entry));
}


//!
//! @brief Opens the dictionary folder using the user's default file browser
//!
//! The dictionary folder that is opened is generally in "~/.waei".
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_open_dictionary_folder_cb (GtkWidget *widget, gpointer data) 
{
    //Declarations
    GwSearchWindow *window;
    const char *directory;
    char *uri;
    GError *error;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;
    directory = lw_util_build_filename (LW_PATH_DICTIONARY, NULL);
    uri = g_build_filename ("file://", directory, NULL);
    error = NULL;

    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &error);

    gw_common_handle_error (&error, GTK_WINDOW (window->toplevel), TRUE);

    g_free (uri);
}


//!
//! @brief Sets the drag icon to the cursor if the widget is dragged over
//!
//! Part of a group of four functions to handle drag drops of text into
//! the main text buffer which will initialize a search based on that text.
//!
//! @see gw_searchwindow_search_drag_data_recieved_cb ()
//! @see gw_searchwindow_drag_leave_1_cb ()
//! @see gw_searchwindow_drag_drop_1_cb ()
//! @return Always returns true
//!
G_MODULE_EXPORT gboolean gw_searchwindow_drag_motion_1_cb (GtkWidget      *widget,
                                                   GdkDragContext *drag_context,
                                                   gint            x,
                                                   gint            y,
                                                   guint           time,
                                                   gpointer        user_data)
{
    gdk_drag_status (drag_context, GDK_ACTION_COPY, time);
    gtk_drag_highlight (widget);
    return TRUE;
}


//!
//! @brief Resets the gui when the drag leaves the widget area
//!
//! Part of a group of four functions to handle drag drops of text into
//! the main text buffer which will initialize a search based on that text.
//!
//! @see gw_searchwindow_search_drag_data_recieved_cb ()
//! @see gw_searchwindow_drag_drop_1_cb ()
//! @see gw_searchwindow_drag_motion_1_cb ()
//!
G_MODULE_EXPORT void gw_searchwindow_drag_leave_1_cb (GtkWidget      *widget,
                                      GdkDragContext *drag_context,
                                      guint           time,
                                      gpointer        user_data) 
{
    gtk_drag_unhighlight (widget);
}


//!
//! @brief Tells the widget to recieve the dragged data
//!
//! Part of a group of four functions to handle drag drops of text into
//! the main text buffer which will initialize a search based on that text.
//!
//! @see gw_searchwindow_search_drag_data_recieved_cb ()
//! @see gw_searchwindow_drag_leave_1_cb ()
//! @see gw_searchwindow_drag_motion_1_cb ()
//! @return Always returns true
//!
G_MODULE_EXPORT gboolean gw_searchwindow_drag_drop_1_cb (GtkWidget      *widget,
                                         GdkDragContext *drag_context,
                                         gint            x,
                                         gint            y,
                                         guint           time,
                                         gpointer        user_data)  
{
    GdkAtom target;
    target = gtk_drag_dest_find_target (widget, drag_context, NULL);
    gtk_drag_get_data (widget, drag_context, target, time);
    return TRUE;
}


//!
//! @brief The widget recieves the data and starts a search based on it.
//!
//! Part of a group of four functions to handle drag drops of text into
//! the main text buffer which will initialize a search based on that text.
//!
//! @see gw_searchwindow_drag_leave_1_cb ()
//! @see gw_searchwindow_drag_drop_1_cb ()
//! @see gw_searchwindow_drag_motion_1_cb ()
//!
G_MODULE_EXPORT void gw_searchwindow_search_drag_data_recieved_cb (GtkWidget        *widget,
                                                                   GdkDragContext   *drag_context,
                                                                   gint              x,
                                                                   gint              y,
                                                                   GtkSelectionData *data,
                                                                   guint             info,
                                                                   guint             time,
                                                                   gpointer          user_data    )
{
    //Sanity checks
    if (widget == NULL) return;
    const char *name = gtk_buildable_get_name (GTK_BUILDABLE (widget));
    if (name == NULL || strcmp (name, "search_entry") == 0)
      return;

    //Declarations
    GwSearchWindow *window;
    char* text;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;
    text = (char*) gtk_selection_data_get_text (data);   

    if (text != NULL && strlen(text) > 0)
    {
      gw_searchwindow_clear_search_cb (widget, data);
      gtk_entry_set_text (window->entry, text);
      gw_searchwindow_search_cb (widget, data);

      gdk_drag_status (drag_context, GDK_ACTION_COPY, time);
      gtk_drag_finish (drag_context, TRUE, FALSE, time);
    }
    else
    {
      gtk_drag_finish (drag_context, FALSE, FALSE, time);
    }

    //Cleanup
    if (text != NULL) g_free (text);
}


//!
//! @brief Hides/shows buttons depending on search entry text
//!
//! Currently this function just hides and shows the clear icon depending if
//! there is any text in the entry.  Previously, this would also set the search
//! button in it's insensitive state also.
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_update_button_states_based_on_entry_text_cb (GtkWidget *widget,
                                                                                  gpointer   data   )
{
    //Declarations
    GwSearchWindow *window;
    int length;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;
    length = gtk_entry_get_text_length (GTK_ENTRY (window->entry));

    //Show the clear icon when approprate
    if (length > 0)
      gtk_entry_set_icon_from_stock (GTK_ENTRY (window->entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
    else
      gtk_entry_set_icon_from_stock (GTK_ENTRY (window->entry), GTK_ENTRY_ICON_SECONDARY, NULL);

/*
    //Return widget colors back to normal
    gtk_widget_override_background_color (GTK_WIDGET (window->entry), GTK_STATE_NORMAL, NULL);
    gtk_widget_override_color (GTK_WIDGET (window->entry), GTK_STATE_NORMAL, NULL);
*/
}


//!
//! @brief Populates the main contextual menu with search options
//!
//! @param view The GtkTexView that was right clicked
//! @param menu The Popup menu to populate
//! @param data  Currently unused gpointer containing user data
//!
void gw_searchwindow_populate_popup_with_search_options_cb (GtkTextView *view, GtkMenu *menu, gpointer data)
{
/*
    //Declarations
    GwSearchWindow *window;
    LwSearchItem *item = NULL;
    LwDictInfo *di = NULL;
    char *menu_text = NULL;
    GtkWidget *menuitem = NULL;
    GtkWidget *menuimage = NULL;
    char *query_text = NULL;
    char *search_for_menuitem_text;
    char *websearch_for_menuitem_text;
    char *othersearch_for_menuitem_text;
    GtkTextBuffer *buffer;
    GtkTextIter start_iter, end_iter;
    LwDictInfo *di_selected = NULL;
    window = (GwSearchWindow*) gw_app_get_window (app, GW_WINDOW_SEARCH, GTK_WIDGET (view));
    if (window == NULL) return;
    int i = 0;

    if (window->mousedata.hovered_word == NULL) return;

    //Initializations
    buffer = gtk_text_view_get_buffer (view);
    // TRANSLATORS: The first variable is the expression to look for, the second is the dictionary full name
    search_for_menuitem_text = gettext("Search for \"%s\" in the %s");
    // TRANSLATORS: The variable is the expression to look for
    othersearch_for_menuitem_text = gettext("Search for \"%s\" in a Different Dictionary");
    // TRANSLATORS: The variable is the expression to look for
    websearch_for_menuitem_text = gettext("Cross-reference \"%s\" Online");

    menuitem = gtk_separator_menu_item_new ();
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), GTK_WIDGET (menuitem));
    gtk_widget_show (GTK_WIDGET (menuitem));
    if (gtk_text_buffer_get_has_selection (buffer))
    {
      gtk_text_buffer_get_selection_bounds (buffer, &start_iter, &end_iter);
      query_text = gtk_text_buffer_get_text (buffer, &start_iter, &end_iter, FALSE);
      if (g_utf8_strchr(query_text, -1, L'\n') != NULL) query_text = NULL;
    }
    if (query_text == NULL)
    {
      query_text = window->mousedata.hovered_word;
    }
    di_selected = gw_dictinfolist_get_selected_dictinfo (app->dictinfolist);


    //Add webpage links
    di =  lw_dictinfolist_get_dictinfo_by_load_position (LW_DICTINFOLIST (app->dictinfolist), 0);
    char *website_url_menuitems[] = {
      "Wikipedia", "http://www.wikipedia.org/wiki/%s", "wikipedia.png",
      "Goo.ne.jp", "http://dictionary.goo.ne.jp/srch/all/%s/m0u/", "goo.png",
      "Google.com", "http://www.google.com/search?q=%s", "google.png",
      NULL, NULL, NULL
    };

    //Setup the web submenu
    GtkWidget *web_menu = NULL;
    GtkWidget *web_menuitem = NULL;
    menu_text = g_strdup_printf (websearch_for_menuitem_text, query_text);
    if (menu_text != NULL)
    {
      web_menu = gtk_menu_new();
      web_menuitem = gtk_menu_item_new_with_label (menu_text);
      g_free (menu_text);
      menu_text = NULL;
    }
    else
    {
      web_menuitem = gtk_menu_item_new_with_label ("crossreference on the web");
    }
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), GTK_WIDGET (web_menuitem));
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (web_menuitem), GTK_WIDGET (web_menu));
    gtk_widget_show (web_menuitem);
    gtk_widget_show (web_menu);

    i = 0;
    while (website_url_menuitems[i] != NULL)
    {
      if (di != NULL && (item = lw_searchitem_new (query_text, di, LW_OUTPUTTARGET_RESULTS, app->prefmanager, NULL)) != NULL)
      {
        //Create handy variables
        char *name = website_url_menuitems[i];
        char *url = g_strdup_printf(website_url_menuitems[i + 1], query_text);
        if (url != NULL)
        {
          g_free (item->queryline->string);
          item->queryline->string = g_strdup (url);
          g_free (url);
          url = NULL;
        }
        char *icon_path = website_url_menuitems[i + 2];

        //Start creating
        menu_text = g_strdup_printf ("%s", name);
        if (menu_text != NULL)
        {
          menuitem = GTK_WIDGET (gtk_image_menu_item_new_with_label (menu_text));
          char *path = g_build_filename (DATADIR2, PACKAGE, icon_path, NULL);
          if (path != NULL)
          {
            menuimage = gtk_image_new_from_file (path);
            gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), GTK_WIDGET (menuimage));
            g_free (path);
            path = NULL;
          }
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (gw_searchwindow_search_for_searchitem_online_cb), item);
          g_signal_connect (G_OBJECT (menuitem), "destroy",  G_CALLBACK (gw_searchwindow_destroy_tab_menuitem_searchitem_data_cb), item);
          gtk_menu_shell_append (GTK_MENU_SHELL (web_menu), GTK_WIDGET (menuitem));
          gtk_widget_show (GTK_WIDGET (menuitem));
          gtk_widget_show (GTK_WIDGET (menuimage));
          g_free (menu_text);
          menu_text = NULL;
        }
      }
      i += 3;
    }


    //Setup the submenu
    GtkWidget *dictionaries_menu = NULL;
    GtkWidget *dictionaries_menuitem = NULL;
    menu_text = g_strdup_printf (othersearch_for_menuitem_text, query_text);
    if (menu_text != NULL)
    {
      dictionaries_menu = gtk_menu_new();
      dictionaries_menuitem = gtk_menu_item_new_with_label (menu_text);
      g_free (menu_text);
      menu_text = NULL;
    }
    else
    {
      dictionaries_menuitem = gtk_menu_item_new_with_label ("Search for this in a different dictionary");
    }
    gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), GTK_WIDGET (dictionaries_menuitem));
    gtk_menu_item_set_submenu (GTK_MENU_ITEM (dictionaries_menuitem), GTK_WIDGET (dictionaries_menu));
    gtk_widget_show (dictionaries_menuitem);
    gtk_widget_show (dictionaries_menu);

    //Add internal dictionary links
    i = 0;
    while ((di = lw_dictinfolist_get_dictinfo_by_load_position (LW_DICTINFOLIST (app->dictinfolist), i)) != NULL)
    {
      if (di != NULL && (item = lw_searchitem_new (query_text, di, LW_OUTPUTTARGET_RESULTS, app->prefmanager, NULL)) != NULL)
      {
        if (di == di_selected)
        {
          menu_text = g_strdup_printf (search_for_menuitem_text, item->queryline->string, di->longname);
          if (menu_text != NULL)
          {
            menuitem = GTK_WIDGET (gtk_image_menu_item_new_with_label (menu_text));
            menuimage = gtk_image_new_from_icon_name ("stock_new-tab", GTK_ICON_SIZE_MENU);
            gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), GTK_WIDGET (menuimage));
            g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (gw_searchwindow_new_tab_with_search_cb), item);
            gtk_menu_shell_prepend (GTK_MENU_SHELL (menu), GTK_WIDGET (menuitem));
            gtk_widget_show (GTK_WIDGET (menuitem));
            gtk_widget_show (GTK_WIDGET (menuimage));
            g_free (menu_text);
            menu_text = NULL;
          }
        }
        menu_text = g_strdup_printf ("%s", di->longname);
        if (menu_text != NULL)
        {
          menuitem = GTK_WIDGET (gtk_image_menu_item_new_with_label (menu_text));
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (gw_searchwindow_new_tab_with_search_cb), item);
          g_signal_connect (G_OBJECT (menuitem), "destroy",  G_CALLBACK (gw_searchwindow_destroy_tab_menuitem_searchitem_data_cb), item);
          gtk_menu_shell_append (GTK_MENU_SHELL (dictionaries_menu), GTK_WIDGET (menuitem));
          gtk_widget_show (GTK_WIDGET (menuitem));
          gtk_widget_show (GTK_WIDGET (menuimage));
          g_free (menu_text);
          menu_text = NULL;
        }
      }
      i++;
    }
    */
}


//!
//! @brief Searches for the word in a webbrower in an external dictionary
//! 
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_search_for_searchitem_online_cb (GtkWidget *widget, gpointer data)
{
    LwSearchItem *item = (LwSearchItem*) data;
    GError *error;
    GwSearchWindow *window;

    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;

    if (item != NULL)
    {
      error = NULL;
      gtk_show_uri (NULL, item->queryline->string, gtk_get_current_event_time (), &error);

      gw_common_handle_error (&error, GTK_WINDOW (window->toplevel), TRUE);
    }
}

//!
//! @brief Emulates web browsers font size control with (ctrl + wheel)
//!
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT gboolean gw_searchwindow_scroll_or_zoom_cb (GtkWidget *widget, GdkEventScroll *event, gpointer data)
{
    // If "control" is being pressed
    if( (event->state & GDK_CONTROL_MASK) == GDK_CONTROL_MASK )
    {
	// On wheel direction up ~ zoom out
	if(event->direction == GDK_SCROLL_UP)
	{
	  gw_searchwindow_zoom_out_cb (widget, data);
	  return TRUE; // dont propagate event, no scroll
	}

	// On wheel direction down ~ zoom in
	if(event->direction == GDK_SCROLL_DOWN)
	{
	  gw_searchwindow_zoom_in_cb (widget, data);
	  return TRUE; // dont propagate event, no scroll
	}
    }

    // return false and propagate event for regular scroll
    return FALSE;
}


//!
//! @brief Append a tag to the end of the tags
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_new_tab_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    int position;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    position = gw_searchwindow_new_tab (window);

    gtk_notebook_set_current_page (window->notebook, position);
    gw_searchwindow_set_entry_text_by_searchitem (window, NULL);
    gtk_widget_grab_focus (GTK_WIDGET (window->entry));
    gw_searchwindow_set_dictionary(window, 0);
    gw_searchwindow_set_current_searchitem (window, NULL);

    gw_searchwindow_update_tab_appearance (window);
}


//!
//! @brief Remove the tab where the close button is clicked
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_remove_tab_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    int pages;
    int page_num;
    GList *iter;
    LwSearchItem *item;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    pages = gtk_notebook_get_n_pages (window->notebook);
    page_num = gtk_notebook_page_num (window->notebook, GTK_WIDGET (data));

    //Sanity check
    if (pages <= 1) {
      return;
    }

    gw_searchwindow_cancel_search_by_tab_number (window, page_num);
    gtk_notebook_remove_page (window->notebook, page_num);

    iter = g_list_nth (window->tablist, page_num);
    if (iter != NULL)
    {
      item = LW_SEARCHITEM (iter->data);
      if (item != NULL && lw_searchitem_has_history_relevance (item))
      {
        lw_historylist_add_searchitem (window->history, item);
        gw_searchwindow_update_history_popups (window);
      }
      else if (item != NULL)
      {
        lw_searchitem_free (item);
        gtk_widget_grab_focus (GTK_WIDGET (window->entry));
      }
    }
    window->tablist = g_list_delete_link (window->tablist, iter);

    gw_searchwindow_update_tab_appearance (window);

    if (pages == 1) gtk_widget_grab_focus (GTK_WIDGET (window->entry)); 
}


//!
//! @brief Remove the tab where the close button is clicked
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_remove_current_tab_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    int pages;
    int page_num;
    GList *link;
    LwSearchItem *item;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    pages = gtk_notebook_get_n_pages (window->notebook);
    page_num = gtk_notebook_get_current_page (window->notebook);
    link = g_list_nth (window->tablist, page_num);

    if (pages <= 1)
    {
      return;
    }

    gw_searchwindow_cancel_search_by_tab_number (window, page_num);

    if (link != NULL)
    {
      item = LW_SEARCHITEM (link->data);
      if (item != NULL)
      {
        if (lw_searchitem_has_history_relevance (item))
        {
          lw_historylist_add_searchitem (window->history, item);
          gw_searchwindow_update_history_popups (window);
        }
        else
        {
          lw_searchitem_free (item);
          item = NULL;
        }
      }
    }
    window->tablist = g_list_delete_link (window->tablist, link);

    gtk_notebook_remove_page (window->notebook, page_num);
    gw_searchwindow_update_tab_appearance (window);

    if (pages == 1) gtk_widget_grab_focus (GTK_WIDGET (window->entry));
}


//!
//! @brief Do the side actions required when a tab switch takes place
//!
//! Various side elements should be updated when at tab switch occurs
//! such as the progress bar, querybar, dictionry selection etc.
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_switch_tab_cb (GtkNotebook *notebook, GtkWidget *page, int page_num, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    LwSearchItem *item;

    //Initializations
    window = (GwSearchWindow*) gw_app_get_window (app, GW_WINDOW_SEARCH, GTK_WIDGET (notebook));
    if (window == NULL) return;
    item =  LW_SEARCHITEM (g_list_nth_data (window->tablist, page_num));

    gw_searchwindow_update_tab_appearance_by_searchitem (window, item);
}


//!
//! @brief Cycles to the next tab 
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_next_tab_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    int page_num;
    LwSearchItem *item;

    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    page_num = gtk_notebook_get_current_page (window->notebook);
    item = LW_SEARCHITEM (g_list_nth_data (window->tablist, page_num));

    gtk_notebook_next_page (window->notebook);
    gw_searchwindow_update_tab_appearance (window);
}


//!
//! @brief Cycles to the previous tab 
//!
//! @param widget Currently unused widget pointer
//! @param data Currently unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_previous_tab_cb (GtkWidget *widget, gpointer data)
{
    GwSearchWindow *window;
    int page_num;
    LwSearchItem *item;

    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, NULL));
    if (window == NULL) return;
    page_num = gtk_notebook_get_current_page (window->notebook);
    item = LW_SEARCHITEM (g_list_nth_data (window->tablist, page_num));

    gtk_notebook_prev_page (window->notebook);
    gw_searchwindow_update_tab_appearance (window);
}


//!
//! @brief Sets up an initites a new search in a new tab
//!
//! @param widget Currently unused widget pointer
//! @param data A gpointer to a LwSearchItem that hold the search information
//!
G_MODULE_EXPORT void gw_searchwindow_new_tab_with_search_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    LwSearchItem *item;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;
    item = (LwSearchItem*) data;

    if (item != NULL)
    {
      gw_searchwindow_new_tab_cb (widget, data);
      gw_searchwindow_set_dictionary_by_searchitem (window, item);
      gw_searchwindow_set_entry_text_by_searchitem (window, item);
      gw_searchwindow_search_cb (widget, data);
    }
}


G_MODULE_EXPORT void gw_searchwindow_no_results_search_for_dictionary_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    LwDictInfo* di;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));
    if (window == NULL) return;
    di = (LwDictInfo*) data;

    gw_searchwindow_set_dictionary (window, di->load_position);
}

//!
//! @brief Frees the LwSearchItem memory that is attached to the activate tab callback
//!
//! @param widget Currently unused widget pointe
//! @param data gpointer to a LwSearchItem to be freed
//!
G_MODULE_EXPORT void gw_searchwindow_destroy_tab_menuitem_searchitem_data_cb (GObject *object, gpointer data)
{
    //Declarations
    LwSearchItem *item;

    //Initializations
    item = (LwSearchItem*) data;

    if (item != NULL)
    {
      lw_searchitem_free (item);
      item = NULL;
    }
}


//!
//! @brief Sets the show toolbar boolean to match the widget
//! 
//! Makes the gconf pref match the current state of the triggering widget.
//! The gconf value changed callback then updates the state of the toolbar
//! to match the pref.
//!
//! @see gw_searchwindow_set_toolbar_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_toolbar_show_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    gboolean request;

    //Initializations
    request = lw_prefmanager_get_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_TOOLBAR_SHOW);

    lw_prefmanager_set_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_TOOLBAR_SHOW, !request);
}


//!
//! @brief Syncs the gui to the preference settinging.  It should be attached to the gsettings object
//!
G_MODULE_EXPORT void gw_searchwindow_sync_toolbar_show_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GtkAction *action;
    gboolean request;

    //Initializations
    window = GW_SEARCHWINDOW (data);
    action = GTK_ACTION (gtk_builder_get_object (window->builder, "view_toggle_toolbar_action"));
    request = lw_prefmanager_get_boolean (settings, key);

    if (request == TRUE)
      gtk_widget_show (GTK_WIDGET (window->toolbar));
    else
      gtk_widget_hide (GTK_WIDGET (window->toolbar));

    g_signal_handlers_block_by_func (action, gw_searchwindow_toolbar_show_toggled_cb, NULL);
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), request);
    g_signal_handlers_unblock_by_func (action, gw_searchwindow_toolbar_show_toggled_cb, NULL);
}


//!
//! @brief Sets the show toolbar boolean to match the widget
//! @see gw_searchwindow_set_toolbar_show ()
//! @param widget Unused GtkWidget pointer.
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_searchwindow_statusbar_show_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    gboolean request;

    //Initializations
    request = lw_prefmanager_get_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_STATUSBAR_SHOW);

    lw_prefmanager_set_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_STATUSBAR_SHOW, !request);
}


//!
//! @brief Sets the checkbox to show or hide the statusbar
//!
//! @param request How to set the statusbar
//!
G_MODULE_EXPORT void gw_searchwindow_sync_statusbar_show_cb (GSettings *settings, gchar *key, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GtkAction *action;
    gboolean request;

    //Initializations
    window = GW_SEARCHWINDOW (data);
    action = GTK_ACTION (gtk_builder_get_object (window->builder, "view_toggle_statusbar_action"));
    request = lw_prefmanager_get_boolean (settings, key);

    if (request == TRUE)
      gtk_widget_show (window->statusbar);
    else
      gtk_widget_hide (window->statusbar);

    g_signal_handlers_block_by_func (action, gw_searchwindow_statusbar_show_toggled_cb, NULL);
    gtk_toggle_action_set_active (GTK_TOGGLE_ACTION (action), request);
    g_signal_handlers_unblock_by_func (action, gw_searchwindow_statusbar_show_toggled_cb, NULL);
}


//!
//! @brief Sets the requested font with magnification applied
//!
G_MODULE_EXPORT void gw_searchwindow_sync_font_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    //Declarations
    GwSearchWindow *window;

    //Initializations
    window = GW_SEARCHWINDOW (data);

    gw_searchwindow_set_font (window);
}


//!
//! @brief Callback to toggle spellcheck in the search entry
//!
//! @param widget Unused pointer to a GtkWidget
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void gw_spellcheck_toggled_cb (GtkWidget *widget, gpointer data)
{
    //Declarations
    gboolean state;
    GwSearchWindow *window;

    //Initializations
    state = lw_prefmanager_get_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK);
    window = GW_SEARCHWINDOW (gw_app_get_window (app, GW_WINDOW_SEARCH, widget));

    lw_prefmanager_set_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK, !state);

    if (window->spellcheck->query_text != NULL) g_free (window->spellcheck->query_text);
    window->spellcheck->query_text = g_strdup ("FORCE UPDATE");
}


//!
//! @brief Sets the gui widgets consistently to the requested state
//!
//! The function makes sure that both of the widgets in the gui are the same
//! when the user clicks a one of them to change the settings.
//!
//! @param request the requested state for spellchecking widgets
//!
void gw_searchwindow_sync_spellcheck (GSettings *settings, gchar *Key, gpointer data)
{
    //Declarations
    GwSearchWindow *window;
    GtkWidget *toolbutton;
    gboolean request;

    //Initializations
    window = GW_SEARCHWINDOW (data);
    toolbutton = GTK_WIDGET (gtk_builder_get_object (window->builder, "spellcheck_toolbutton"));
    request = lw_prefmanager_get_boolean_by_schema (app->prefmanager, LW_SCHEMA_BASE, LW_KEY_SPELLCHECK);

    g_signal_handlers_block_by_func (toolbutton, gw_spellcheck_toggled_cb, NULL);

    gtk_toggle_tool_button_set_active (GTK_TOGGLE_TOOL_BUTTON (toolbutton), request);

    g_signal_handlers_unblock_by_func (toolbutton, gw_spellcheck_toggled_cb, NULL);
}


