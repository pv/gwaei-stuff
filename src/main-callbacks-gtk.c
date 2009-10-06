/******************************************************************************

  FILE:
  src/callback.c

  DESCRIPTION:
  Callbacks for activities initiated by the user. Most of the gtk code here
  should still be abstracted to the interface C file when possible.

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

#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <libintl.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gtk.h>
#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/dictionaries.h>
#include <gwaei/history.h>

#include <gwaei/printing.h>
#include <gwaei/engine.h>
#include <gwaei/settings.h>
#include <gwaei/interface.h>
#include <gwaei/preferences.h>

#include "kanjipad/kanjipad.h"


G_MODULE_EXPORT void do_settings(GtkWidget *widget, gpointer data)
{
    gwaei_ui_cancel_search_by_target(GWAEI_TARGET_RESULTS);
    gwaei_ui_cancel_search_by_target(GWAEI_TARGET_KANJI);

    //Prepare the interface
    gwaei_settings_initialize_enabled_features_list();
    gwaei_settings_initialize_installed_dictionary_list();
    gwaei_ui_update_settings_interface();

    //get some needed window references
    gwaei_ui_show_window ("settings_window");
}


G_MODULE_EXPORT void do_kanjipad(GtkWidget *widget, gpointer data)
{
    kanjipad_set_target_text_widget(search_entry);
    show_kanjipad (builder);
}


G_MODULE_EXPORT void do_close_kanji_results(GtkWidget *widget, gpointer data)
{ 
    gwaei_close_kanji_results();
}


//
// Functions for looking up a kanji character at the mouse position
//


//Sets the correct cursor when the mouse hovers over a kanji character
G_MODULE_EXPORT gboolean do_get_iter_for_motion (GtkWidget      *widget,
                                                 GdkEventButton *event,
                                                 gpointer        data   )
{
    gunichar unic;
    gint x = event->x;
    gint y = event->y;

    unic = gwaei_get_hovered_character(&x, &y);

    DictionaryInfo *di;
    di = dictionarylist_get_dictionary_by_alias ("Kanji");
  
    // Characters above 0xFF00 represent inserted images
    if (unic > L'ー' && unic < 0xFF00 && di->status == INSTALLED)
      gwaei_ui_set_cursor(GDK_HAND2);
    else
      gwaei_ui_set_cursor(GDK_XTERM);

    return FALSE;
}


gint button_press_x = 0;
gint button_press_y = 0;
G_MODULE_EXPORT gboolean do_get_position_for_button_press (GtkWidget      *widget,
                                                           GdkEventButton *event,
                                                           gpointer        data    )
{
  //Window coordinates
  button_press_x = event->x;
  button_press_y = event->y;

  gwaei_get_hovered_character(&button_press_x, &button_press_y);

  return FALSE;
}


G_MODULE_EXPORT gboolean do_get_iter_for_button_release (GtkWidget      *widget,
                                                         GdkEventButton *event,
                                                         gpointer        data    )
{
  //Window coordinates
  gint x = event->x;
  gint y = event->y;
  gint trailing = 0;
  GtkTextIter start;

  gunichar unic;
  unic = gwaei_get_hovered_character(&x, &y);

  DictionaryInfo *di;
  di = dictionarylist_get_dictionary_by_alias ("Kanji");

  if (di->status == INSTALLED     &&
      abs(button_press_x - x) < 3 &&
      abs(button_press_y - y) < 3   )
  {
    // Characters above 0xFF00 represent inserted images
    if (unic > L'ー' && unic < 0xFF00 )
    {
      gwaei_ui_open_kanji_results();

      //Convert the unicode character into to a utf8 string
      gchar query[7];
      gint length = g_unichar_to_utf8(unic, query);
      query[length] = '\0'; 
      printf("Kanji lookup: %s\n", query);

      //Start the search
      SearchItem *item;
      item = searchitem_new (query, di, GWAEI_TARGET_KANJI);

      gwaei_search_get_results (item);
    }
    else
    {
      gwaei_close_kanji_results();
    }
  }

  return FALSE; 
}


//
//Calbacks for closing a window
//


G_MODULE_EXPORT void do_close (GtkWidget *widget, gpointer data)
{
    const char *id = gtk_widget_get_name (widget);

    if (strcmp (id, "main_window") == 0)
    {
      save_window_attributes_and_hide (id);
      gtk_main_quit ();
    }
    else if (strcmp (id, "radicals_window") == 0)
    {
      save_window_attributes_and_hide (id);
    }
    else if (strcmp (id, "settings_window") == 0)
    {
      if (rebuild_combobox_dictionary_list () > 0)
      {
        gtk_widget_hide (widget);
        update_toolbar_buttons();
      }
      else
      {
        gtk_main_quit();
      }
    }
}


G_MODULE_EXPORT gboolean do_delete_event_action (GtkWidget *widget, gpointer data)
{ 
    do_close(widget, data);
    return TRUE;
}


G_MODULE_EXPORT gboolean do_close_on_escape (GtkWidget *widget,
                             GdkEvent  *event,
                             gpointer  *data   )
{
  guint keyval = ((GdkEventKey*)event)->keyval;
  guint state = ((GdkEventKey*)event)->state;
  guint modifiers = ( 
                      GDK_MOD1_MASK    |
                      GDK_CONTROL_MASK |
                      GDK_SUPER_MASK   |
                      GDK_HYPER_MASK   |
                      GDK_META_MASK      
                    );

  //Make sure no modifier keys are pressed
  if (((state & modifiers) == 0 ) && keyval == GDK_Escape)
  {
    do_close(widget, data);
    return TRUE;
  }
  else
  {
    return FALSE;
  }
}


G_MODULE_EXPORT void do_quit (GtkWidget *widget, gpointer data)
{
    printf("Quitting...\n");
    do_close(widget, data);
    gtk_main_quit ();
}


G_MODULE_EXPORT void do_search_from_history (GtkWidget *widget, gpointer data)
{
    if (gwaei_ui_cancel_search_by_target(GWAEI_TARGET_RESULTS) == FALSE) return;

    HistoryList *hl;
    hl = historylist_get_list(GWAEI_HISTORYLIST_RESULTS);

    SearchItem *item;
    item = (SearchItem*) data;

    if (hl->back != NULL && g_list_find(hl->back, item))
    {
      while (hl->back != NULL && hl->current != item)
        historylist_go_back_by_target (GWAEI_HISTORYLIST_RESULTS);
    }
    else if (hl->forward != NULL && g_list_find(hl->forward, item))
    {
      while (hl->forward != NULL && hl->current != item)
        historylist_go_forward_by_target (GWAEI_HISTORYLIST_RESULTS);
    }

    searchitem_reset_result_counters(hl->current);
    gwaei_search_get_results(hl->current);
    gwaei_ui_update_history_popups();
    update_toolbar_buttons();

    //Set the search string in the GtkEntry
    gwaei_ui_clear_search_entry();
    gwaei_ui_search_entry_insert((hl->current)->query);
    gwaei_ui_text_select_all_by_target(GWAEI_TARGET_ENTRY);
    gwaei_ui_grab_focus_by_target(GWAEI_TARGET_ENTRY);

    //Set the correct dictionary in the gui
    const int id_length = 50;
    char id[id_length];
    GtkWidget *combobox;
    strncpy(id, "dictionary_combobox", id_length);
    combobox = GTK_WIDGET (gtk_builder_get_object (builder, id));
    gtk_combo_box_set_active(GTK_COMBO_BOX (combobox), hl->current->dictionary->load_position);
}


G_MODULE_EXPORT void do_back (GtkWidget *widget, gpointer data)
{
    HistoryList *hl;
    hl = historylist_get_list (GWAEI_HISTORYLIST_RESULTS);
    if (hl->back != NULL)
    {
      do_search_from_history (NULL, hl->back->data);
      printf("Back button clicked\n");
    }
}


G_MODULE_EXPORT void do_forward (GtkWidget *widget, gpointer data)
{
    HistoryList *hl;
    hl = historylist_get_list (GWAEI_HISTORYLIST_RESULTS);
    if (hl->forward != NULL)
    {
      do_search_from_history (NULL, hl->forward->data);
      printf("Forward button clicked\n");
    }
}


G_MODULE_EXPORT void do_save_as (GtkWidget *widget, gpointer data)
{
  //Declarations
  GtkWidget *dialog, *window;
  window = GTK_WIDGET (gtk_builder_get_object(builder, "main_window"));
  dialog = gtk_file_chooser_dialog_new (gettext("Save As"),
				      GTK_WINDOW (window),
				      GTK_FILE_CHOOSER_ACTION_SAVE,
				      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				      NULL);
  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

  //See if the user has already saved.  If they did, reuse the path
  if (save_path[0] == '\0')
  {
      gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "");
      gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), gettext("vocabulary.txt"));
  }
  else
  {
      gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (dialog), save_path);
  }

  //Prepare the text
  gchar *text;
  //Get the region of text to be saved if some text is highlighted
  text = gwaei_ui_get_text_from_text_buffer(GWAEI_TARGET_RESULTS);

  //Run the save as dialog
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
  {
      char *filename;
      filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
      strncpy( save_path,
               gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog)),
               FILENAME_MAX                                               );
      g_free (filename);
      filename = NULL;

      gwaei_io_write_file("w", text);

      GtkWidget *edit_toolbutton;
      edit_toolbutton = GTK_WIDGET (gtk_builder_get_object(builder, "edit_toolbutton"));
      gtk_widget_set_sensitive (edit_toolbutton, TRUE);

      GtkAction *edit_menuitem;
      edit_menuitem = GTK_ACTION (gtk_builder_get_object(builder, "edit_menuitem"));
      gtk_action_set_sensitive(edit_menuitem, TRUE);
  }

  gtk_widget_destroy (dialog);
  g_free(text);
  text = NULL;
  printf("Save as button was clicked\n");
}


G_MODULE_EXPORT void do_save (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog, *window;
  window = GTK_WIDGET (gtk_builder_get_object(builder, "main_window"));

  gchar *text;
  text = gwaei_ui_get_text_from_text_buffer(GWAEI_TARGET_RESULTS);

  //Pop up a save dialog if the user hasn't saved before
  if (save_path[0] == '\0')
  {
    //Setup the save dialog
    dialog = gtk_file_chooser_dialog_new (gettext("Save"),
                GTK_WINDOW (window),
                GTK_FILE_CHOOSER_ACTION_SAVE,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
                NULL);
    gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog), TRUE);

    //Set the default save path
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "");
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog), gettext("vocabulary.txt"));

    //Run the save dialog
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
    {
        char *filename;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
        strncpy( save_path,
                 gtk_file_chooser_get_filename(GTK_FILE_CHOOSER (dialog)),
                 FILENAME_MAX                                              );
        g_free(filename);
        filename = NULL;

        gwaei_io_write_file("a", text);

        GtkWidget *edit_toolbutton;
        edit_toolbutton = GTK_WIDGET (gtk_builder_get_object(builder, "edit_toolbutton"));
        gtk_widget_set_sensitive (edit_toolbutton, TRUE);

        GtkAction *edit_menuitem;
        edit_menuitem = GTK_ACTION (gtk_builder_get_object(builder, "edit_menuitem"));
        gtk_action_set_sensitive(edit_menuitem, TRUE);
    }

    gtk_widget_destroy (dialog);
  }

  //Write the file without opening a dialog
  else
  {
      gwaei_io_write_file("a", text);
  }

  g_free(text);
  text = NULL;
  printf("Save button was clicked\n");
}


//Zoom functions
G_MODULE_EXPORT void do_zoom_in (GtkWidget *widget, gpointer data)
{
    int size;
    size = gwaei_pref_get_int (GCKEY_GWAEI_FONT_SIZE, 12) + 2;
    if (size >= MIN_FONT_SIZE && size <= MAX_FONT_SIZE)
      gwaei_pref_set_int (GCKEY_GWAEI_FONT_SIZE, size);

    printf("Zoom in button was clicked\n");
}

G_MODULE_EXPORT void do_zoom_out (GtkWidget *widget, gpointer data)
{
    int size;
    size = gwaei_pref_get_int (GCKEY_GWAEI_FONT_SIZE, 12) - 2;
    if (size >= MIN_FONT_SIZE | size <= MAX_FONT_SIZE)
      gwaei_pref_set_int (GCKEY_GWAEI_FONT_SIZE, size);

    printf("Zoom out button was clicked\n");
}

G_MODULE_EXPORT void do_zoom_100 (GtkWidget *widget, gpointer data)
{
    int size;
    size = gwaei_pref_get_default_int (GCKEY_GWAEI_FONT_SIZE, 12);
    gwaei_pref_set_int (GCKEY_GWAEI_FONT_SIZE, size);

    printf ("Zoom 100 button was clicked\n");
}


G_MODULE_EXPORT void do_less_relevant_results_toggle (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gwaei_pref_get_boolean (GCKEY_GWAEI_LESS_RELEVANT_SHOW, TRUE);
    gwaei_pref_set_boolean (GCKEY_GWAEI_LESS_RELEVANT_SHOW, !state);

    printf("Toggle less relevent results button was clicked\n");
}



G_MODULE_EXPORT void do_toolbar_toggle (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gwaei_pref_get_boolean (GCKEY_GWAEI_TOOLBAR_SHOW, TRUE);
    gwaei_pref_set_boolean (GCKEY_GWAEI_TOOLBAR_SHOW, !state);

    printf("Toolbar toggle button was clicked\n");
}



G_MODULE_EXPORT void do_dictionary_changed_action (GtkWidget *widget, gpointer data)
{
    //Declarations
    const char id_length = 50;
    char id[id_length];
    GList *list = NULL;
    GtkMenuShell *shell = NULL;
    strncpy (id, "dictionary_popup", id_length);
    shell = GTK_MENU_SHELL (gtk_builder_get_object (builder, id));

    //Action depending on the source
    int active = 0;
    if (strcmp(G_OBJECT_TYPE_NAME(widget), "GtkComboBox") == 0 )
    {
      active = gtk_combo_box_get_active(GTK_COMBO_BOX (widget));
    }
    else if (strcmp(G_OBJECT_TYPE_NAME(widget), "GtkRadioMenuItem") == 0 )
    {
      list = gtk_container_get_children (GTK_CONTAINER (shell));
      while (list != NULL && gtk_check_menu_item_get_active( GTK_CHECK_MENU_ITEM (list->data)) == FALSE)
      {
        list = g_list_next(list);
        active++;
      }
    }

    //Finish up
    gwaei_ui_set_dictionary(active);
    gwaei_ui_grab_focus_by_target(GWAEI_TARGET_ENTRY);
}



G_MODULE_EXPORT void do_select_all (GtkWidget *widget, gpointer data)
{
    guint TARGET;
    TARGET = gwaei_ui_get_current_widget_focus("main_window");

    gwaei_ui_text_select_all_by_target(TARGET);

    printf("Select All button was clicked\n");
}



G_MODULE_EXPORT void do_paste (GtkWidget *widget, gpointer data)
{
    guint TARGET;
    TARGET = gwaei_ui_get_current_widget_focus("main_window");
    gwaei_ui_paste_text(TARGET);

    printf("Paste button was clicked\n");
}



G_MODULE_EXPORT void do_cut (GtkWidget *widget, gpointer data)
{
    guint TARGET;
    TARGET = gwaei_ui_get_current_widget_focus("main_window");
    gwaei_ui_cut_text(TARGET);

    printf("Cut button was clicked\n");
}



G_MODULE_EXPORT void do_copy (GtkWidget *widget, gpointer data)
{
    guint TARGET;
    TARGET = gwaei_ui_get_current_widget_focus("main_window");
    gwaei_ui_copy_text(TARGET);

    printf("Copy button was clicked\n");
}



gulong copy_handler_id = 0;
gulong cut_handler_id = 0;
gulong paste_handler_id = 0;
gulong select_all_handler_id = 0;

G_MODULE_EXPORT gboolean do_update_clipboard_on_focus_change (GtkWidget        *widget, 
                                              GtkDirectionType  arg1,
                                              gpointer          data   ) 
{
    guint TARGET;
    TARGET = gwaei_ui_get_current_widget_focus("main_window");

    //Set up the references to the actions
    GtkAction *copy_action, *cut_action, *paste_action, *select_all_action;

    char id[50];
    strncpy(id, "edit_copy_action", 50);
    copy_action       = GTK_ACTION (gtk_builder_get_object (builder, id));
    strncpy(id, "edit_cut_action", 50);
    cut_action        = GTK_ACTION (gtk_builder_get_object (builder, id));
    strncpy(id, "edit_paste_action", 50);
    paste_action      = GTK_ACTION (gtk_builder_get_object (builder, id));
    strncpy(id, "edit_select_all_action", 50);
    select_all_action = GTK_ACTION (gtk_builder_get_object (builder, id));

    //Disconnected old handlers
    if (copy_handler_id != 0)
    {
      g_signal_handler_disconnect( copy_action, copy_handler_id );
      copy_handler_id = 0;
    }
    if (cut_handler_id != 0)
    {
      g_signal_handler_disconnect( cut_action, cut_handler_id );
      cut_handler_id = 0;
    }
    if (paste_handler_id != 0)
    {
      g_signal_handler_disconnect( paste_action, paste_handler_id );
      paste_handler_id = 0;
    }
    if (select_all_handler_id != 0)
    {
      g_signal_handler_disconnect( select_all_action, select_all_handler_id );
      select_all_handler_id = 0;
    }
                                          
    //Create new ones pointed at the correct widget
    copy_handler_id = g_signal_connect       ( copy_action,
                                               "activate",
                                               G_CALLBACK (do_copy),
                                               widget                      );
    cut_handler_id = g_signal_connect        ( cut_action,
                                               "activate",
                                               G_CALLBACK (do_cut),
                                               widget                      );
    paste_handler_id = g_signal_connect      ( paste_action,
                                               "activate",
                                               G_CALLBACK (do_paste),
                                               widget                      );
    select_all_handler_id = g_signal_connect ( select_all_action,
                                               "activate",
                                               G_CALLBACK (do_select_all),
                                               widget                      );


    //Correct the sensitive states of the menuitems
    if (gwaei_ui_widget_equals_target (data, GWAEI_TARGET_RESULTS) ||
        gwaei_ui_widget_equals_target (data, GWAEI_TARGET_KANJI)     )
    {
      gtk_action_set_sensitive(GTK_ACTION (cut_action), FALSE);
      gtk_action_set_sensitive(GTK_ACTION (paste_action), FALSE);
    }
    else
    {
      gtk_action_set_sensitive(GTK_ACTION (cut_action), TRUE);
      gtk_action_set_sensitive(GTK_ACTION (paste_action), TRUE);
    }

    return FALSE;
}


G_MODULE_EXPORT void do_print (GtkWidget *widget, gpointer data)
{
    gwaei_ui_cancel_search_by_target(GWAEI_TARGET_RESULTS);
    gwaei_print();
    printf("Print button was clicked\n");
}


G_MODULE_EXPORT void do_radical_search_tool(GtkWidget *widget, gpointer data)
{
    char id[50];

    GtkWidget *hbox;
    strncpy(id, "strokes_hbox", 50);
    hbox = GTK_WIDGET (gtk_builder_get_object(builder, id));

    GtkWidget *spinbutton;
    strcpy(id, "strokes_spinbutton");
    spinbutton = GTK_WIDGET (gtk_builder_get_object(builder, id));
    gtk_spin_button_set_value (GTK_SPIN_BUTTON (spinbutton), 1.0);


    //Hide the togglebox if the Mix dictionary is not present
    if (dictionarylist_dictionary_get_status_by_id (MIX) != INSTALLED)
      gtk_widget_hide (hbox); 
    else
      gtk_widget_show (hbox); 

    //Show the window
    gwaei_ui_show_window ("radicals_window");
}

G_MODULE_EXPORT void do_edit(GtkWidget *widget, gpointer data)
{
    char *uri = g_build_filename("file://", save_path, NULL);

    GError *err = NULL;
    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &err);
    if (err != NULL)
      g_error_free(err);

    g_free(uri);
}



G_MODULE_EXPORT void do_help(GtkWidget *widget, gpointer data)
{
    GError *err = NULL;
    gtk_show_uri (NULL, "ghelp:gwaei", gtk_get_current_event_time (), &err);
    if (err != NULL)
      g_error_free(err);
}


G_MODULE_EXPORT void do_glossary(GtkWidget *widget, gpointer data)
{
    char *uri = g_build_filename("ghelp://", DATADIR2, "gnome", "help", "gwaei", "C", "glossary.xml", NULL);

    GError *err = NULL;
    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &err);
    if (err != NULL)
      g_error_free(err);

    g_free(uri);
}


G_MODULE_EXPORT void do_about(GtkWidget *widget, gpointer data)
{
    char pixbuf_path[FILENAME_MAX];
    strcpy(pixbuf_path, DATADIR2);
    strcat(pixbuf_path, "/");
    strcat(pixbuf_path, PACKAGE);
    strcat(pixbuf_path, "/logo.png");

    //Generate the translator credits in such a way it is easily localizable
    char *italian = gettext("Italian: ");
    char *japanese = gettext("Japanese: ");
    char *spanish = gettext("Spanish: ");
    char translator_credits[10000];
    strcpy(translator_credits, "");

    strcat(translator_credits, italian);
    strcat(translator_credits, "Fabrizio Sabatini\n");
    strcat(translator_credits, japanese);
    strcat(translator_credits, "Zachary Dovel\n");
    strcat(translator_credits, spanish);
    strcat(translator_credits, "4packed\n");

    GdkPixbuf *logo;
    if ( (logo = gdk_pixbuf_new_from_file ( pixbuf_path,    NULL)) == NULL &&
         (logo = gdk_pixbuf_new_from_file ( "img/logo.png", NULL)) == NULL    )
    {
      printf("Was unable to load the gwaei logo.\n");
    }

  GtkWidget *about = g_object_new (GTK_TYPE_ABOUT_DIALOG,
               "program-name", "gWaei", 
               "version", VERSION,
               "copyright", gettext("gWaei (C) 2008-2009 Zachary Dovel\nKanjipad backend (C) 2002 Owen Taylor\nJStroke backend (C) 1997 Robert Wells"),
               "comments", gettext("Program for Japanese translation and reference. The\ndictionaries are supplied by Jim Breen's WWWJDIC."),
               "license", "This software is GPL Licensed.\n\ngWaei is free software: you can redistribute it and/or modify\nit under the terms of the GNU General Public License as published by\n the Free Software Foundation, either version 3 of the License, or\n(at your option) any later version.\n\ngWaei is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License\nalong with gWaei.  If not, see <http://www.gnu.org/licenses/>.",
               "logo", logo,
               "translator-credits", translator_credits,
               NULL);
    gtk_dialog_run(GTK_DIALOG (about));
    g_object_unref (logo);
    gtk_widget_destroy (about);
}


G_MODULE_EXPORT void do_cycle_dictionaries_forward (GtkWidget *widget, gpointer data)
{
    gwaei_ui_cycle_dictionaries(TRUE);
}


G_MODULE_EXPORT void do_cycle_dictionaries_backward (GtkWidget *widget, gpointer data)
{
    gwaei_ui_cycle_dictionaries(FALSE);
}


G_MODULE_EXPORT gboolean do_focus_change_on_key_press (GtkWidget *widget,
                                                       GdkEvent  *event,
                                                       gpointer  *focus  )
{
    guint state = ((GdkEventKey*)event)->state;
    guint keyval = ((GdkEventKey*)event)->keyval;
    guint modifiers = ( 
                        GDK_MOD1_MASK    |
                        GDK_CONTROL_MASK |
                        GDK_SUPER_MASK   |
                        GDK_HYPER_MASK   |
                        GDK_META_MASK      |
                        GDK_Meta_L    |
                        GDK_Meta_R    |
                        GDK_Alt_L     |
                        GDK_Alt_R
                      );

    //Make sure no modifier keys are pressed
    if (
          (state & modifiers) == 0   &&
          keyval != GDK_Tab          &&
          keyval != GDK_ISO_Left_Tab &&
          keyval != GDK_Shift_L      &&
          keyval != GDK_Shift_R      &&
          keyval != GDK_Control_L    &&
          keyval != GDK_Control_R    &&
          keyval != GDK_Caps_Lock    &&
          keyval != GDK_Shift_Lock   &&
          keyval != GDK_Meta_L       &&
          keyval != GDK_Meta_R       &&
          keyval != GDK_Alt_L        &&
          keyval != GDK_Alt_R        &&
          keyval != GDK_Super_L      &&
          keyval != GDK_Super_R      &&
          keyval != GDK_Hyper_L      &&
          keyval != GDK_Hyper_R      &&
          keyval != GDK_Num_Lock     &&
          keyval != GDK_Scroll_Lock  &&
          keyval != GDK_Pause        &&
          keyval != GDK_Home         &&
          keyval != GDK_End
       )
    {
      //Change focus to the text view if is an arrow key or page key
      if ( 
           ( keyval == GDK_Up        ||
             keyval == GDK_Down      ||
             keyval == GDK_Page_Up   ||
             keyval == GDK_Page_Down   
           ) &&
           (
             !gwaei_ui_widget_equals_target (widget, GWAEI_TARGET_RESULTS)
           )
         )
      {
        gwaei_ui_text_select_none_by_target (GWAEI_TARGET_ENTRY);
        gwaei_ui_grab_focus_by_target (GWAEI_TARGET_RESULTS);
        return TRUE;
      }

      //Change focus to the entry if other key
      else if (
                keyval != GDK_Up        &&
                keyval != GDK_Down      &&
                keyval != GDK_Page_Up   &&
                keyval != GDK_Page_Down &&
                !gwaei_ui_widget_equals_target (widget, GWAEI_TARGET_ENTRY)
              )
      {
        gwaei_ui_text_select_all_by_target (GWAEI_TARGET_ENTRY);
        gwaei_ui_grab_focus_by_target(GWAEI_TARGET_ENTRY);
        return TRUE;
      }
    }
    return FALSE;

return FALSE;
}


G_MODULE_EXPORT void do_search (GtkWidget *widget, gpointer data)
{
    HistoryList* hl = historylist_get_list(GWAEI_HISTORYLIST_RESULTS);

    GList *list = dictionarylist_get_selected();
    DictionaryInfo *dictionary = list->data;

    gchar query[MAX_QUERY];
    gwaei_ui_strcpy_from_widget(query, MAX_QUERY, GWAEI_TARGET_ENTRY);

    char *gckey = GCKEY_GWAEI_LESS_RELEVANT_SHOW; 
    gboolean show_less_relevant = gwaei_pref_get_boolean (gckey, TRUE);

    if (strlen (query) == 0)
      return;

    if (hl->current != NULL &&
        strcmp (query, hl->current->query) == 0 &&
        dictionary->id == hl->current->dictionary->id &&
        show_less_relevant == hl->current->show_less_relevant_results)
      return;

    if (gwaei_ui_cancel_search_by_target(GWAEI_TARGET_RESULTS) == FALSE)
      return;

    if (hl->current != NULL && (hl->current)->results_found) 
    {
      historylist_add_searchitem_to_history(GWAEI_HISTORYLIST_RESULTS, hl->current);
      hl->current = NULL;
      gwaei_ui_update_history_popups();
    }
    else if (hl->current != NULL)
    {
      searchitem_free (hl->current);
      hl->current = NULL;
    }
    
    //in add_to_history() rather than here
    hl->current = searchitem_new (query, dictionary, GWAEI_TARGET_RESULTS);
    if (hl->current == NULL)
    {
      g_warning("There was an error creating the searchitem variable.  I will cancel this search.  Please eat some cheese.\n");
      return;
    }

    //Start the search
    gwaei_search_get_results(hl->current);

    //Set the colors of the entry to the current match highlight colors
    char key[100];
    char *key_ptr;
    strcpy(key, GCPATH_GWAEI);
    strcat(key, "/highlighting/match");
    key_ptr = &key[strlen(key)];
    char fg_color[100], bg_color[100], fallback[100];
    char *ret;
    strcpy(key_ptr, "_foreground");
    gwaei_util_strncpy_fallback_from_key (fallback, key, 100);
    ret = gwaei_pref_get_string (fg_color, key, fallback, 100);
    if (IS_HEXCOLOR(fg_color) == FALSE)
    {
      if (ret != NULL) gwaei_pref_set_string (key, fallback);
      strncpy(fg_color, fallback, 100);
    }
    strcpy(key_ptr, "_background");
    gwaei_util_strncpy_fallback_from_key (fallback, key, 100);
    ret = gwaei_pref_get_string (bg_color, key, fallback, 100);
    if (IS_HEXCOLOR(bg_color) == FALSE)
    {
      if (ret != NULL) gwaei_pref_set_string (key, fallback);
      strncpy(bg_color, fallback, 100);
    }
    GdkColor forground, background;
    gdk_color_parse (fg_color, &forground);
    gdk_color_parse (bg_color, &background);
    gtk_widget_modify_base(GTK_WIDGET (search_entry), GTK_STATE_NORMAL, &background);
    gtk_widget_modify_text(GTK_WIDGET (search_entry), GTK_STATE_NORMAL, &forground);

    //Update the toolbar buttons
    update_toolbar_buttons();
}


G_MODULE_EXPORT void do_insert_unknown_character(GtkWidget *widget, gpointer data)
{
    gwaei_ui_search_entry_insert (".");
}


G_MODULE_EXPORT void do_insert_word_edge(GtkWidget *widget, gpointer data)
{
    gwaei_ui_search_entry_insert ("\\b");
}


G_MODULE_EXPORT void do_insert_not_word_edge(GtkWidget *widget, gpointer data)
{
    gwaei_ui_search_entry_insert ("\\B");
}


G_MODULE_EXPORT void do_insert_and(GtkWidget *widget, gpointer data)
{
    gwaei_ui_search_entry_insert ("&");
}


G_MODULE_EXPORT void do_insert_or(GtkWidget *widget, gpointer data)
{
    gwaei_ui_search_entry_insert ("|");
}


G_MODULE_EXPORT void do_clear_search(GtkWidget *widget, gpointer data)
{
    gwaei_ui_clear_search_entry ();
    gwaei_ui_grab_focus_by_target (GWAEI_TARGET_ENTRY);
}

    
G_MODULE_EXPORT void do_update_styles(GtkWidget *widget, gpointer data)
{
    GtkWidget *window, *view;
    GdkColor color;

    window = GTK_WIDGET (gtk_builder_get_object (builder, "main_window"));
    view   = GTK_WIDGET (gtk_builder_get_object (builder, "kanji_text_view"));
    color  = window->style->bg[GTK_STATE_NORMAL];

    g_signal_handlers_block_by_func(widget, do_update_styles, NULL);
    gtk_widget_modify_base (view, GTK_STATE_NORMAL, &color);
    g_signal_handlers_unblock_by_func(widget, do_update_styles, NULL);
}


G_MODULE_EXPORT void do_open_dictionary_folder(GtkWidget *widget, gpointer data) 
{
    char directory[FILENAME_MAX];
    get_waei_directory(directory);
    printf("%s\n",directory);

    char *uri = g_build_filename ("file://", directory, NULL);

    GError *err = NULL;
    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &err);
    if (err != NULL)
      g_error_free(err);

    g_free(uri);
}


//!
//! \brief Moves the cursor on screen
//!
//! This function is used as a callback during scrolling to keep the cursor
//! on the screen.  The effect is, if the user types a letter by accident, the
//! buffer won't suddenly jump all the way to the bottom (where the cursor
//! normally is after all these operations.)
//!
G_MODULE_EXPORT void do_move_cursor_onscreen (GtkWidget *widget, gpointer data)
{
    gtk_text_view_place_cursor_onscreen (GTK_TEXT_VIEW (widget));
}


//!
//! \brief Drag an drop code for starting searches
//!
//! Uses the dropped text to automatically start a search.  It generally works
//! really well except for it can't get text from it's own buffer for some
//! reason...
//!
G_MODULE_EXPORT gboolean drag_motion_1 (GtkWidget      *widget,
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


G_MODULE_EXPORT void drag_leave_1 (GtkWidget      *widget,
                                   GdkDragContext *drag_context,
                                   guint           time,
                                   gpointer        user_data) 
{
    gtk_drag_unhighlight (widget);
}


G_MODULE_EXPORT gboolean drag_drop_1 (GtkWidget      *widget,
                                      GdkDragContext *drag_context,
                                      gint            x,
                                      gint            y,
                                      guint           time,
                                      gpointer        user_data)  
{
    GdkAtom target;
    target = gtk_drag_dest_find_target (widget, drag_context, NULL);
    gtk_drag_get_data (widget, drag_context, target, time);
}


G_MODULE_EXPORT void search_drag_data_recieved (GtkWidget        *widget,
                                                GdkDragContext   *drag_context,
                                                gint              x,
                                                gint              y,
                                                GtkSelectionData *data,
                                                guint             info,
                                                guint             time,
                                                gpointer          user_data    )
{
    if (strcmp (gtk_widget_get_name (widget), "search_entry") == 0)
      return;

    GtkWidget *entry;
    entry = GTK_WIDGET (search_entry);

    char* text = gtk_selection_data_get_text (data);

    if ((data->length >= 0) && (data->format == 8) && text != NULL)
    {
      g_free (text);

      do_clear_search(entry, NULL);
      gtk_entry_set_text(GTK_ENTRY (entry), data->data);
      do_search(NULL, NULL);

      drag_context->action = GDK_ACTION_COPY;
      gtk_drag_finish(drag_context, TRUE, FALSE, time);
    }
    else
    {
      gtk_drag_finish(drag_context, FALSE, FALSE, time);
    }
}


//!
//! \brief Hides/shows buttons depending on search entry text
//!
//! Currently this function just hides and shown the clear icon depending if
//! there is any text in the entry.  Previously, this would also set the search
//! button in it's insensitive state also.
//!
G_MODULE_EXPORT void do_update_button_states_based_on_entry_text (GtkWidget *widget,
                                                                  gpointer   data   )
{
    int length = gtk_entry_get_text_length (GTK_ENTRY (search_entry));

    //Show the clear icon when approprate
    if (length > 0)
      gtk_entry_set_icon_from_stock(GTK_ENTRY (search_entry), GTK_ENTRY_ICON_SECONDARY, GTK_STOCK_CLEAR);
    else
      gtk_entry_set_icon_from_stock(GTK_ENTRY (search_entry), GTK_ENTRY_ICON_SECONDARY, NULL);

    //Return widget colors back to normal
    gtk_widget_modify_base(GTK_WIDGET (search_entry), GTK_STATE_NORMAL, NULL);
    gtk_widget_modify_text(GTK_WIDGET (search_entry), GTK_STATE_NORMAL, NULL);
}



