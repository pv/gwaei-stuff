/******************************************************************************

  FILE:
  src/main-interface-gtk.c

  DESCRIPTION:
  Used as a go between for functions interacting with GUI interface objects.
  This is the gtk version.

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
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gtk.h>
#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/dictionaries.h>
#include <gwaei/history.h>

#include <gwaei/engine.h>
#include <gwaei/callbacks.h>
#include <gwaei/interface.h>
#include <gwaei/preferences.h>


//Convenience pointers
GtkWidget *results_tv   = NULL;
GtkWidget *kanji_tv     = NULL;
GObject   *results_tb   = NULL;
GObject   *kanji_tb     = NULL;
GtkWidget *search_entry = NULL;


GObject* get_gobject_from_target(const int TARGET)
{
    GObject *gobject;
    switch (TARGET)
    {
      case GWAEI_TARGET_RESULTS:
        return results_tb;
      case GWAEI_TARGET_KANJI:
        return kanji_tb;
      default:
        return NULL;
    }
}


GtkWidget* get_widget_from_target(const int TARGET)
{
    GtkWidget *widget;
    switch (TARGET)
    {
      case GWAEI_TARGET_RESULTS:
        return results_tv;
      case GWAEI_TARGET_KANJI:
        return kanji_tv;
      case GWAEI_TARGET_ENTRY:
        return search_entry;
      default:
        return NULL;
    }
}


gboolean gwaei_ui_widget_equals_target (gpointer widget, const int TARGET)
{
    GtkWidget* target;
    target = get_widget_from_target (TARGET);
    return (GTK_WIDGET (widget) == GTK_WIDGET (target));
}


void initialize_global_widget_pointers ()
{
    char id[50];

    //Setup our text view and text buffer references
    strncpy (id, "results_text_view", 50);
    results_tv = GTK_WIDGET (gtk_builder_get_object(builder, id));
    results_tb = G_OBJECT (gtk_text_view_get_buffer(GTK_TEXT_VIEW (results_tv)));
    strncpy (id, "kanji_text_view", 50);
    kanji_tv = GTK_WIDGET (gtk_builder_get_object(builder, id));
    kanji_tb   = G_OBJECT (gtk_text_view_get_buffer(GTK_TEXT_VIEW (kanji_tv)));

    strncpy(id, "search_entry", 50);
    search_entry = GTK_WIDGET (gtk_builder_get_object(builder, id));
}



void initialize_window_attributes(char* window_id)
{
    int leftover;

    GtkWidget *window;
    window = GTK_WIDGET (gtk_builder_get_object(builder, window_id));

    //Setup the unique key for the window
    leftover = MAX_GCONF_KEY;
    char key[leftover];

    strncpy(key, GCPATH_GWAEI, leftover);
    leftover -= strlen(GCPATH_GWAEI);
    strncat(key, "/", leftover);
    leftover -= 1;
    strncat(key, gtk_widget_get_name(window), leftover);
    leftover -= strlen(gtk_widget_get_name(window));

    //Set a pointer at the end of the key for easy access
    char *value;
    value = &key[strlen(key)];

    //Get the stored attributes from pref
    int x, y, width, height;

    strncpy(value, "/x", leftover - strlen("/x"));
    x = gwaei_pref_get_int (key, 0);
    strncpy(value, "/y", leftover - strlen("/y"));
    y = gwaei_pref_get_int(key, 0);
    strncpy(value, "/width", leftover - strlen("/width"));
    width = gwaei_pref_get_int(key, 100);
    strncpy(value, "/height", leftover - strlen("/height"));
    height = gwaei_pref_get_int(key, 100);

    //Apply the x and y if they are within the screen size
    if (x < gdk_screen_width() && y < gdk_screen_height()) {
      gtk_window_move(GTK_WINDOW (window), x, y);
    }

    //Apply the height and width if they are sane
    if ( width  >= 100                 &&
         width  <= gdk_screen_width()  && 
         height >= 100                 && 
         height <= gdk_screen_height()    )
    {
      gtk_window_resize(GTK_WINDOW(window), width, height);
    }
}



void save_window_attributes_and_hide(char* window_id)
{
    GtkWidget *window;
    window = GTK_WIDGET (gtk_builder_get_object(builder, window_id));

    //Get the window attributes
    int x, y, width, height;
    gtk_window_get_position (GTK_WINDOW (window), &x, &y);
    gtk_window_get_size (GTK_WINDOW (window), &width, &height);

    //Hide the widget now because pref can be slow
    gtk_widget_hide(window);

    //Setup our unique key for the window
    int leftover = MAX_GCONF_KEY;
    char key[leftover];

    strncpy(key, GCPATH_GWAEI, leftover);
    leftover -= strlen(GCPATH_GWAEI);
    strncat(key, "/", leftover);
    leftover -= 1;
    strncat(key, gtk_widget_get_name(window), leftover);
    leftover -= strlen(gtk_widget_get_name(window));

    //Set a pointer at the end of the key for easy access
    char *value;
    value = &key[strlen(key)];

    //Start sending the attributes to pref for storage

    strncpy(value, "/x", leftover - strlen("/x"));
    gwaei_pref_set_int (key, x);
    strncpy(value, "/y", leftover - strlen("/y"));
    gwaei_pref_set_int (key, y);
    strncpy(value, "/width", leftover - strlen("/width"));
    gwaei_pref_set_int (key, width);
    strncpy(value, "/height", leftover - strlen("/height"));
    gwaei_pref_set_int (key, height);
}


void gwaei_ui_show_window (char *id)
{
    if (strcmp(id, "main_window") == 0 || strcmp (id, "radicals_window") == 0)
    {
      GtkWidget *window;
      window = GTK_WIDGET (gtk_builder_get_object (builder, id));
      initialize_window_attributes (id);
      gtk_widget_show(window);
      initialize_window_attributes (id);
    }
    else if (strcmp (id, "settings_window") == 0)
    {
      GtkWidget *main_window;
      main_window = GTK_WIDGET (gtk_builder_get_object(builder, "main_window"));
      GtkWidget *settings_window;
      settings_window = GTK_WIDGET (gtk_builder_get_object(builder, id));

      //Show the window
      gtk_window_set_transient_for(GTK_WINDOW (settings_window), GTK_WINDOW (main_window));
      gtk_widget_show(settings_window);
    }
    else if (strcmp (id, "kanjipad_window") == 0)
    {
      kanjipad_set_target_text_widget(search_entry);
      show_kanjipad(builder);
    }
    else
    {
      GtkWidget *window;
      window = GTK_WIDGET (gtk_builder_get_object(builder, id));
      gtk_widget_show(window);
    }
}


void update_toolbar_buttons()
{
    char id[50];

    //Delarations
    GtkWidget *popup, *button;
    GtkAction *action;
    GList* children;
    gboolean enable;

    SearchItem* current;
    current = historylist_get_current (GWAEI_TARGET_RESULTS);

    int current_font_size;
    current_font_size = gwaei_pref_get_int (GCKEY_GWAEI_FONT_SIZE, 12);

    //Update Zoom in sensitivity state
    strcpy(id, "zoom_in_toolbutton");
    button = GTK_WIDGET (gtk_builder_get_object (builder, id));

    strcpy(id, "zoom_in_menuitem");
    action = GTK_ACTION (gtk_builder_get_object (builder, id));

    enable = (current != NULL && current_font_size < MAX_FONT_SIZE);

    gtk_widget_set_sensitive(button, enable);
    gtk_action_set_sensitive(action, enable);

    //Update Zoom out sensitivity state
    strcpy(id, "zoom_out_toolbutton");
    button = GTK_WIDGET (gtk_builder_get_object (builder, id));

    strcpy(id, "zoom_out_menuitem");
    action = GTK_ACTION (gtk_builder_get_object (builder, id));

    enable = (current != NULL && current_font_size > MIN_FONT_SIZE);

    gtk_widget_set_sensitive(button, enable);
    gtk_action_set_sensitive(action, enable);

    //Update Zoom 100 sensitivity state
    strcpy(id, "zoom_100_menuitem");
    action = GTK_ACTION (gtk_builder_get_object(builder, id));

    int default_font_size;
    default_font_size = gwaei_pref_get_default_int (GCKEY_GWAEI_FONT_SIZE, 12);

    enable = (current != NULL && current_font_size != default_font_size);

    gtk_action_set_sensitive(action, enable);

    //Update Save sensitivity state
    strcpy(id, "save_toolbutton");
    button = GTK_WIDGET (gtk_builder_get_object(builder, id));

    strcpy(id, "save_menuitem");
    action = GTK_ACTION (gtk_builder_get_object(builder, id));

    enable = (current != NULL);

    gtk_widget_set_sensitive(button, enable);
    gtk_action_set_sensitive(action, enable);

    //Update Save as sensitivity state
    strcpy(id, "save_as_toolbutton");
    button = GTK_WIDGET (gtk_builder_get_object(builder, id));

    strcpy(id, "save_as_menuitem");
    action = GTK_ACTION (gtk_builder_get_object(builder, id));

    enable = (current != NULL);

    gtk_widget_set_sensitive(button, enable);
    gtk_action_set_sensitive(action, enable);

    //Update Print sensitivity state
    strcpy(id, "print_toolbutton");
    button = GTK_WIDGET (gtk_builder_get_object(builder, id));

    strcpy(id, "print_menuitem");
    action = GTK_ACTION (gtk_builder_get_object(builder, id));

    enable = (current != NULL);

    gtk_widget_set_sensitive(button, enable);
    gtk_action_set_sensitive(action, enable);

    //Update radicals search tool menuitem
    strcpy(id, "radicals_menuitem");
    action = GTK_ACTION (gtk_builder_get_object (builder, id));

    enable = (dictionarylist_dictionary_get_status_by_id (RADICALS) == INSTALLED);
    gtk_action_set_sensitive(action, enable);
}


void gwaei_ui_reinitialize_results_label (SearchItem *item)
{
    if (item->target == GWAEI_TARGET_KANJI) return;

    char id[50];
    GtkWidget *label;
    strcpy(id, "results_label_start");
    label = GTK_WIDGET (gtk_builder_get_object(builder, id));
    gtk_label_set_text(GTK_LABEL (label), gettext("Searching... "));

    GtkWidget *hbox;
    strcpy(id, "results_relevant_hbox");
    hbox = GTK_WIDGET (gtk_builder_get_object(builder, id));
    gtk_widget_hide (GTK_WIDGET (hbox));

    item->total_results = 0;
    item->total_relevant_results = 0;
    gwaei_ui_update_total_results_label(item);
}


void gwaei_ui_update_total_results_label (SearchItem* item)
{
    if (gwaei_util_get_runmode () == GWAEI_CONSOLE_RUNMODE) return;
    if (item->target == GWAEI_TARGET_KANJI) return;

    char id[50];

    //Get the gtk widgets from gtkbuilder
    GtkWidget *results;
    strcpy(id, "results_label_hbox");
    results = GTK_WIDGET (gtk_builder_get_object(builder, id));

    GtkWidget *number;
    strcpy(id, "results_label_number");
    number = GTK_WIDGET (gtk_builder_get_object(builder, id));

    GtkWidget *no_results;
    strcpy(id, "no_results_label");
    no_results = GTK_WIDGET (gtk_builder_get_object(builder, id));

    int number_int = item->total_results;
    char number_string[14];
    gwaei_itoa(number_int, number_string, 14);

    //Finish
    gtk_widget_hide(no_results);
    gtk_label_set_text(GTK_LABEL (number), number_string);
    gtk_widget_show(results);

    if (item->total_results != item->total_relevant_results &&
        item->total_relevant_results != 0                     )
    {
      strcpy(id, "results_relevant_hbox");
      results = GTK_WIDGET (gtk_builder_get_object(builder, id));

      strcpy(id, "results_relevant_number");
      number = GTK_WIDGET (gtk_builder_get_object(builder, id));
      gtk_widget_show (GTK_WIDGET (results));

      number_int = item->total_relevant_results;
      number_string[0] = '\0';
      gwaei_itoa(number_int, number_string, 14);

      gtk_label_set_text(GTK_LABEL (number), number_string);
      gtk_widget_show(results);
    }
}

void gwaei_ui_finalize_total_results_label (SearchItem* item)
{
    if (item->target == GWAEI_TARGET_KANJI) return;

    char id[50];
    GtkWidget *label;
    strcpy(id, "results_label_start");
    label = GTK_WIDGET (gtk_builder_get_object(builder, id));
    gtk_label_set_text(GTK_LABEL (label), gettext("Found "));
}


//
// Menu dictionary combobox section
//

int rebuild_combobox_dictionary_list() 
{
    char id[50];
    GtkWidget *combobox;
    strcpy(id, "dictionary_combobox");
    combobox = GTK_WIDGET (gtk_builder_get_object(builder, id));

    gtk_combo_box_set_active( GTK_COMBO_BOX (combobox), 0);
    while (gtk_combo_box_get_active_text(GTK_COMBO_BOX (combobox)) != NULL)
    {
      gtk_combo_box_remove_text( GTK_COMBO_BOX (combobox), 0);
      gtk_combo_box_set_active( GTK_COMBO_BOX (combobox), 0);
    }

    GList* dictionaries = dictionarylist_get_list ();
    DictionaryInfo *di;
    int i = 0;
    while (dictionaries != NULL)
    {
      di = dictionaries->data;
      if (di->status == INSTALLED)
      {
        if ((dictionarylist_dictionary_get_status_by_id (MIX) == INSTALLED &&
            di->id != MIX &&
            di->id != RADICALS                                               )||
           ((dictionarylist_dictionary_get_status_by_id (MIX) != INSTALLED )))
        {
          gtk_combo_box_append_text(GTK_COMBO_BOX (combobox), di->name);
          i++;
        }
      }
      dictionaries = dictionaries->next;
    }
    gtk_combo_box_set_active( GTK_COMBO_BOX (combobox), 0);

    printf("%d dictionaries are being used.\n", i);
    return i;
}


//
// Menu popups section
//
void gwaei_ui_update_history_menu_popup()
{
    //Get a reference to the history_popup
    char id[50];
    GtkWidget *history_popup;
    strcpy (id, "history_popup");
    history_popup = GTK_WIDGET (gtk_builder_get_object(builder, id));

    GList     *children;
    children = gtk_container_get_children(GTK_CONTAINER (history_popup));
    GList *history_list;

    //Get some references to the back and forward buttons
    GtkWidget *back_menuitem;
    back_menuitem = GTK_WIDGET (children->data);
    children = children->next;
    history_list = historylist_get_back_history (GWAEI_TARGET_RESULTS);
    gtk_widget_set_sensitive (back_menuitem, g_list_length (history_list));

    GtkWidget *forward_menuitem;
    forward_menuitem = GTK_WIDGET (children->data);
    children = children->next;
    history_list = historylist_get_forward_history (GWAEI_TARGET_RESULTS);
    gtk_widget_set_sensitive (forward_menuitem, g_list_length (history_list));

    //Remove all widgets after the back and forward menuitem buttons
    while (children != NULL )
    {
      gtk_widget_destroy(children->data);
      children = g_list_delete_link(children, children);
    }

    //Add a seperator to the end of the history popup
    GtkWidget *menuitem;
    menuitem = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL (history_popup), menuitem);
    gtk_widget_show(menuitem);


    //Declarations
    GtkWidget *label;
    const char *text;
    SearchItem *item;

    children = historylist_get_combined_history_list (GWAEI_HISTORYLIST_RESULTS);
    while (children != NULL)
    {
      item = children->data;

      int leftover = 200;
      char label[leftover];
      /*
      strncpy (label, gettext("Searched for \""), leftover);
      leftover -= 1;
      */
      strncpy (label, item->query, leftover);
      leftover -= strlen (item->query);
      while (leftover > 180)
      {
        strncat (label, " ", leftover);
        leftover -= 1;
      }
      /*
      strncat (label, gettext("\" in the "), leftover);
      leftover -= strlen(gettext("\" in the "));
      strncat (label, item->dictionary->name, leftover);
      leftover -= strlen (item->dictionary->name);
      strncat (label, gettext(" Dictionary"), leftover);
      */

      menuitem = GTK_WIDGET (gtk_menu_item_new_with_label(label));

      //Create the new menuitem
      gtk_menu_shell_append(GTK_MENU_SHELL (history_popup), menuitem);
      gtk_widget_show  (menuitem);
      g_signal_connect (GTK_WIDGET (menuitem), 
                        "activate",
                        G_CALLBACK (do_search_from_history), 
                        item                               );
   
      children = children->next;
    }
}


//Populate the menu item lists for the back and forward buttons
void rebuild_history_button_popup(char* id, GList* list) {
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
    SearchItem *item;

    children = list;
    while (children != NULL)
    {
      item = children->data;

      int leftover = 200;
      char label[leftover];
      /*
      strncpy (label, gettext("Searched for \""), leftover);
      leftover -= 1;
      */
      strncpy (label, item->query, leftover);
      leftover -= strlen (item->query);
      while (leftover > 180)
      {
        strncat (label, " ", leftover);
        leftover -= 1;
      }
      /*
      strncat (label, gettext("\" in the "), leftover);
      leftover -= strlen(gettext("\" in the "));
      strncat (label, item->dictionary->name, leftover);
      leftover -= strlen (item->dictionary->name);
      strncat (label, gettext(" Dictionary"), leftover);
      */

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

void gwaei_ui_rebuild_back_history_popup() {
    
    GList* list;
    list = historylist_get_back_history (GWAEI_HISTORYLIST_RESULTS);
    rebuild_history_button_popup("back_popup", list);

    GtkWidget *button;
    button= GTK_WIDGET (gtk_builder_get_object (builder, "back_toolbutton"));
    gtk_widget_set_sensitive (button, (g_list_length(list) > 0));
}

void gwaei_ui_rebuild_forward_history_popup() {
    GList* list;
    list = historylist_get_forward_history (GWAEI_HISTORYLIST_RESULTS);
    rebuild_history_button_popup("forward_popup", list);

    GtkWidget *button;
    button = GTK_WIDGET (gtk_builder_get_object (builder, "forward_toolbutton"));
    gtk_widget_set_sensitive (button, (g_list_length(list) > 0));
}


void gwaei_ui_update_history_popups()
{
  gwaei_ui_update_history_menu_popup();
  gwaei_ui_rebuild_back_history_popup();
  gwaei_ui_rebuild_forward_history_popup();
}


void gwaei_ui_set_font(char *family, int size)
{
    //Convert the int version of size to a string
    char digit2 = ((size/10) + 48);
    char digit1 = ((size%10) + 48);
    char digits[] = { digit2, digit1, '\0' };

    //Concatinate it all together
    char font[100];
    strncpy(font, family, 100 - 4);
    strcat(font, " ");
    strcat(font, digits);

    PangoFontDescription *desc;

    if ((desc = pango_font_description_from_string (font)) != NULL)
    {
      gtk_widget_modify_font (GTK_WIDGET (results_tv), desc);
      gtk_widget_modify_font (GTK_WIDGET (kanji_tv), desc);
      pango_font_description_free (desc);
    }

    desc = NULL;
}

void gwaei_ui_set_toolbar_style(char *request) 
{
    char id[50];

    GtkWidget *toolbar;
    strcpy(id, "toolbar");
    toolbar = GTK_WIDGET (gtk_builder_get_object(builder, id));

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


void gwaei_ui_set_toolbar_show(gboolean request)
{
    char id[50];
    
    GtkWidget *toolbar;
    strcpy(id, "toolbar");
    toolbar = GTK_WIDGET (gtk_builder_get_object(builder, id));

    if (request == TRUE)
      gtk_widget_show(toolbar);
    else
      gtk_widget_hide(toolbar);

    GObject *action;
    strcpy(id, "toolbar_toggle_menuitem");
    action = G_OBJECT (gtk_builder_get_object(builder,id));

    g_signal_handlers_block_by_func(action, do_toolbar_toggle, NULL);
    gtk_toggle_action_set_active(GTK_TOGGLE_ACTION (action), request);
    g_signal_handlers_unblock_by_func(action, do_toolbar_toggle, NULL);
}


void gwaei_ui_set_less_relevant_show(gboolean show)
{
  char id[50];

  GObject *action;
  strcpy(id, "less_relevant_results_toggle_menuitem");
  action = G_OBJECT (gtk_builder_get_object(builder, id));

  g_signal_handlers_block_by_func(action, do_less_relevant_results_toggle, NULL);
  gtk_toggle_action_set_active(GTK_TOGGLE_ACTION (action), show);
  g_signal_handlers_unblock_by_func(action, do_less_relevant_results_toggle, NULL);

  less_relevant_results_show = show;
}


void gwaei_ui_set_romanji_kana_conv(int request)
{
  char id[50];

  GtkWidget *widget;
  strcpy(id, "query_romanji_to_kana");
  widget = GTK_WIDGET (gtk_builder_get_object(builder, id));

  g_signal_handlers_block_by_func(widget, do_romanji_kana_conv_change, NULL);
  gtk_combo_box_set_active(GTK_COMBO_BOX (widget), request);
  g_signal_handlers_unblock_by_func(widget, do_romanji_kana_conv_change, NULL);
}


void gwaei_ui_set_hiragana_katakana_pref(GtkWidget *widget, gpointer data)
{
  gboolean active;
  active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (widget));

  char key[100];
  char *key_ptr;
  strcpy(key, GCPATH_GWAEI);
  strcat(key, "/");
  strcat(key, gtk_widget_get_name(widget));

  gwaei_pref_set_boolean (key, active);
}


void gwaei_ui_set_hiragana_katakana_conv(gboolean request)
{
    char id[50];

    GtkWidget *widget;
    strcpy(id, "query_hiragana_to_katakana");
    widget = GTK_WIDGET (gtk_builder_get_object(builder, id));

    g_signal_handlers_block_by_func(widget, do_hiragana_katakana_conv_toggle, NULL); 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (widget), request);
    g_signal_handlers_unblock_by_func(widget, do_hiragana_katakana_conv_toggle, NULL); 
}


void gwaei_ui_set_katakana_hiragana_conv(gboolean request)
{
    char id[50];

    GtkWidget *widget;
    strcpy(id, "query_katakana_to_hiragana");
    widget = GTK_WIDGET (gtk_builder_get_object(builder, id));

    g_signal_handlers_block_by_func(widget, do_hiragana_katakana_conv_toggle, NULL); 
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (widget), request);
    g_signal_handlers_unblock_by_func(widget, do_hiragana_katakana_conv_toggle, NULL); 
}




void gwaei_ui_set_color_to_swatch(const char *widget_id, guint r, guint g, guint b)
{
    GtkWidget *widget;
    widget = GTK_WIDGET (gtk_builder_get_object(builder, widget_id));

    GdkColor color;
    color.pixel = -1;
    color.red = r;
    color.green = g;
    color.blue = b;

    g_signal_handlers_block_by_func(widget, do_set_color_to_swatch, NULL);
    gtk_color_button_set_color(GTK_COLOR_BUTTON (widget), &color);
    g_signal_handlers_unblock_by_func(widget, do_set_color_to_swatch, NULL);
}


void gwaei_ui_append_to_buffer (const int TARGET, char *text, char *tag1,
                                char *tag2, int *start_line, int *end_line)
{
    //Assertain the target text buffer
    GObject *tb;
    tb = get_gobject_from_target(TARGET);

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


void gwaei_ui_append_image_to_buffer (const int TARGET, char *name)
{
    //Insert the pixbuf
    int leftover = FILENAME_MAX;
    char pixbuf_path[leftover];
    strncpy(pixbuf_path, DATADIR2, leftover);
    leftover -= strlen(DATADIR2);
    strncat(pixbuf_path, G_DIR_SEPARATOR_S, leftover);
    leftover -= 1;
    strncat(pixbuf_path, PACKAGE, leftover);
    leftover -= strlen(PACKAGE);
    strncat(pixbuf_path, G_DIR_SEPARATOR_S, leftover);
    leftover -= 1;
    strncat(pixbuf_path, name, leftover);

    GdkPixbuf *pixbuf;
    if ((pixbuf = gdk_pixbuf_new_from_file ( pixbuf_path, NULL)) == NULL)
    {
      g_assert("gWaei was unable to find the character.png image\n");
    }

    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(results_tb), &iter);
    gtk_text_buffer_insert_pixbuf (GTK_TEXT_BUFFER (results_tb), &iter, pixbuf);
    g_object_unref(pixbuf);
}


void gwaei_ui_clear_buffer_by_target (const int TARGET)
{
    //Assertain the target text buffer
    GObject *tb;
    tb = get_gobject_from_target (TARGET);

    if (TARGET != GWAEI_TARGET_KANJI) {
      //Clear the target text buffer
      gtk_text_buffer_set_text (GTK_TEXT_BUFFER (tb), "", -1);
      gwaei_ui_append_to_buffer (TARGET, "\n", "small", NULL, NULL, NULL);
    }
    else {
      //Clear the target text buffer
      gtk_text_buffer_set_text (GTK_TEXT_BUFFER (tb), "", -1);
    }
}


void gwaei_ui_search_entry_insert(char* text)
{
    GtkWidget *entry;
    entry = search_entry;

    glong length;
    length = strlen (text);

    gint start, end;
    gtk_editable_get_selection_bounds (GTK_EDITABLE (entry), &start, &end);
    gtk_editable_delete_text (GTK_EDITABLE (entry), start, end);

    gtk_editable_insert_text(GTK_EDITABLE (entry), text, length, &start);
    gtk_editable_set_position (GTK_EDITABLE (entry), start);
}


void gwaei_ui_grab_focus_by_target (const int TARGET)
{
    GtkWidget* widget;
    widget = get_widget_from_target(TARGET);
    gtk_widget_grab_focus(widget);
}


void gwaei_ui_clear_search_entry()
{
    GtkWidget *entry;
    entry = search_entry;

    gint start, end;
    gtk_editable_select_region (GTK_EDITABLE (entry), 0, -1);
    gtk_editable_get_selection_bounds (GTK_EDITABLE (entry), &start, &end);
    gtk_editable_delete_text (GTK_EDITABLE(entry), start, end);
}


void gwaei_ui_strcpy_from_widget(char* output, int MAX, int TARGET)
{
    //GtkEntry
    if (TARGET == GWAEI_TARGET_ENTRY)
    {
      GtkWidget *entry;
      entry = search_entry;

      strncpy(output, gtk_entry_get_text (GTK_ENTRY (entry)), MAX);
    }
  /*
    //GtkTextView
    else if (TARGET = GWAEI_TARGET_RESULTS | TARGET = GWAEI_TARGET_KANJI)
    {
      GObject *tb;
      switch (TARGET)
      {
        case GWAEI_TARGET_RESULTS:
          tb = results_tb;
          break;
        case GWAEI_TARGET_KANJI:
          tb = kanji_tb;
          break;
      }
      GtkTextIter start, end;

      gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER (tb), &start);
      gtk_text_buffer_get_end_iter( GTK_TEXT_BUFFER (tb), &end);
      gtk_text_buffer_select_range (GTK_TEXT_BUFFER (tb), &start, &end);
    }
  */
}


void gwaei_ui_text_select_all_by_target (int TARGET)
{
    //GtkEntry
    if (TARGET == GWAEI_TARGET_ENTRY)
    {
      GtkWidget *entry;
      entry = search_entry;

      gtk_editable_select_region (GTK_EDITABLE (entry), 0,-1);
    }

    //GtkTextView
    else if (TARGET == GWAEI_TARGET_RESULTS ||
             TARGET == GWAEI_TARGET_KANJI     )
    {
      //Assertain the target text buffer
      GObject *tb;
      tb = get_gobject_from_target(TARGET);

      GtkTextIter start, end;
      gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER (tb), &start);
      gtk_text_buffer_get_end_iter( GTK_TEXT_BUFFER (tb), &end);

      gtk_text_buffer_select_range (GTK_TEXT_BUFFER (tb), &start, &end);
    }
}


void gwaei_ui_text_select_none_by_target (int TARGET)
{
    //GtkEntry
    if (TARGET == GWAEI_TARGET_ENTRY)
    {
      GtkWidget *entry;
      entry = search_entry;

      gtk_editable_select_region (GTK_EDITABLE (entry), -1,-1);
    }

    //GtkTextView
    else if (TARGET == GWAEI_TARGET_RESULTS ||
             TARGET == GWAEI_TARGET_KANJI     )
    {
      //Assertain the target text buffer
      GObject *tb;
      tb = get_gobject_from_target(TARGET);

      GtkTextIter start, end;
      gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER (tb), &start);
      gtk_text_buffer_get_end_iter( GTK_TEXT_BUFFER (tb), &end);

      gtk_text_buffer_select_range (GTK_TEXT_BUFFER (tb), &start, &end);
    }
}


guint gwaei_ui_get_current_widget_focus (char *window_id)
{
    GtkWidget *window;
    window = GTK_WIDGET (gtk_builder_get_object (builder, window_id));

    GtkWidget *widget;
    widget = GTK_WIDGET (gtk_window_get_focus (GTK_WINDOW (window))); 

    if (widget == results_tv)
      return GWAEI_TARGET_RESULTS;
    if (widget == kanji_tv)
      return GWAEI_TARGET_KANJI;
    if (widget == search_entry)
      return GWAEI_TARGET_ENTRY;
    else
      return -1;
}


void gwaei_ui_copy_text(guint TARGET)
{
    GtkClipboard *clipbd;

    switch (TARGET)
    {
      case GWAEI_TARGET_ENTRY:
        gtk_editable_copy_clipboard (GTK_EDITABLE (search_entry));
        break;
      case GWAEI_TARGET_RESULTS:
        clipbd = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
        gtk_text_buffer_copy_clipboard (GTK_TEXT_BUFFER (results_tb), clipbd);
        break;
      case GWAEI_TARGET_KANJI:
        clipbd = gtk_clipboard_get (GDK_SELECTION_CLIPBOARD);
        gtk_text_buffer_copy_clipboard (GTK_TEXT_BUFFER (kanji_tb), clipbd);
        break;
    }
}


void gwaei_ui_cut_text(guint TARGET)
{
    switch (TARGET)
    {
      case GWAEI_TARGET_ENTRY:
        gtk_editable_cut_clipboard (GTK_EDITABLE (search_entry));
        break;
    }
}


void gwaei_ui_paste_text(guint TARGET)
{
    switch (TARGET)
    {
      case GWAEI_TARGET_ENTRY:
        gtk_editable_paste_clipboard (GTK_EDITABLE (search_entry));
        break;
    }
}


gboolean gwaei_ui_load_gtk_builder_xml(const char *name) {
    char local_path[FILENAME_MAX];
    strcpy(local_path, "xml");
    strcat(local_path, G_DIR_SEPARATOR_S);
    strcat(local_path, name);

    char global_path[FILENAME_MAX]; 
    strcpy(global_path, DATADIR2);
    strcat(global_path, G_DIR_SEPARATOR_S);
    strcat(global_path, PACKAGE);
    strcat(global_path, G_DIR_SEPARATOR_S);
    strcat(global_path, name);

    if ( gtk_builder_add_from_file (builder, local_path,  NULL) ||
         gtk_builder_add_from_file (builder, global_path, NULL)    )
    {
      gtk_builder_connect_signals (builder, NULL);
      return TRUE;
    }

    printf("Could not find needed xml files\n");
    return FALSE;
}


void initialize_history_popups()
{
    //Initialize the history popup
    GtkWidget *menubar;
    menubar = GTK_WIDGET (gtk_builder_get_object(builder, "menubar"));

    GList *children;
    children = gtk_container_get_children(GTK_CONTAINER (menubar));

    while (children->next != NULL &&
           strcmp(gtk_widget_get_name(children->data), "history_menu") != 0)
    {
      children = children->next;
    }

    GtkWidget *submenu;
    submenu = GTK_WIDGET (gtk_builder_get_object(builder, "history_popup"));

    gtk_menu_item_set_submenu (children->data, submenu);
}



/////////
//  Tag handling
/////////////////////////////////////////////////


#define fg "foreground"
#define bg "background"
#define IS_HEXCOLOR(color) (regexec(&re_hexcolor, (color), 1, NULL, 0) == 0)

gboolean gwaei_ui_set_color_to_tagtable (char    *id,     int      TARGET,
                                         gboolean set_fg, gboolean set_bg )
{
    GtkTextTag *tag;

    //Assertain the target text buffer
    GObject *tb;
    tb = get_gobject_from_target(TARGET);

    //Load the tag table
    GtkTextTagTable *table;
    table = gtk_text_buffer_get_tag_table(GTK_TEXT_BUFFER (tb));

    //Load the set colors in the preferences
    char key[100];
    char *key_ptr;
    strcpy(key, GCPATH_GWAEI);
    strcat(key, "/highlighting/");
    strcat(key, id);
    key_ptr = &key[strlen(key)];

    char fg_color[100];
    char bg_color[100];
    char fallback[100];
    char *ret;

    //Load the prefered tag colors from pref
    if (set_fg)
    {
      strcpy(key_ptr, "_foreground");
      gwaei_util_strncpy_fallback_from_key (fallback, key, 100);
      ret = gwaei_pref_get_string (fg_color, key, fallback, 100);
      if (IS_HEXCOLOR(fg_color) == FALSE)
      {
        if (ret != NULL) gwaei_pref_set_string (key, fallback);
        strncpy(fg_color, fallback, 100);
      }
    }

    if (set_bg)
    {
      strcpy(key_ptr, "_background");
      gwaei_util_strncpy_fallback_from_key (fallback, key, 100);
      ret = gwaei_pref_get_string (bg_color, key, fallback, 100);
      if (IS_HEXCOLOR(bg_color) == FALSE)
      {
        if (ret != NULL) gwaei_pref_set_string (key, fallback);
        strncpy(bg_color, fallback, 100);
      }
    }

    //Remove the previous tag from the table if it exists
    if ((tag = gtk_text_tag_table_lookup(GTK_TEXT_TAG_TABLE (table), id)) != NULL)
      gtk_text_tag_table_remove(GTK_TEXT_TAG_TABLE (table), GTK_TEXT_TAG (tag));

    //Insert the new tag into the table
    if (set_fg && set_bg)
      tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (tb), id,
                                        fg, fg_color, bg, bg_color, NULL );
    else if (set_fg)
      tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (tb), id,
                                        fg, fg_color, NULL               );
    else if (set_bg)
      tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (tb), id,
                                        bg, bg_color, NULL               );

    return TRUE;
}



void  gwaei_ui_set_tag_to_tagtable (char *id,   int      TARGET,
                                    char *atr,  gpointer val    )
{
    //Assertain the target text buffer
    GObject *tb;
    tb = get_gobject_from_target(TARGET);

    //Important tag (usually bold)
    GtkTextTag *tag;
    tag = gtk_text_buffer_create_tag (GTK_TEXT_BUFFER (tb), id, atr, val, NULL);
}


char* gwaei_ui_get_text_slice_from_buffer (int TARGET, int sl, int el)
{
    //Assertain the target text buffer
    GObject *tb;
    tb = get_gobject_from_target(TARGET);

    //Set up the text
    GtkTextIter si, ei;
    gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (tb), &si, sl);
    gtk_text_buffer_get_iter_at_line (GTK_TEXT_BUFFER (tb), &ei, el);

    return gtk_text_buffer_get_slice (GTK_TEXT_BUFFER (tb), &si, &ei, TRUE);
}


void gwaei_ui_apply_tag_to_text (int TARGET, char tag[],
                                 int sl, int so, int el, int eo)
{
    //Assertain the target text buffer
    GObject *tb;
    tb = get_gobject_from_target(TARGET);

    //Load the text iters
    GtkTextIter si;
    gtk_text_buffer_get_iter_at_line_offset (GTK_TEXT_BUFFER (tb), &si, sl, so);

    GtkTextIter ei;
    gtk_text_buffer_get_iter_at_line_offset (GTK_TEXT_BUFFER (tb), &ei, el, eo);

    //Apply the tag
    gtk_text_buffer_apply_tag_by_name (GTK_TEXT_BUFFER (tb), tag, &si, &ei);
}


gunichar gwaei_get_hovered_character(int *x, int *y)
{
    gint trailing = 0;

    gtk_text_view_window_to_buffer_coords (GTK_TEXT_VIEW(results_tv), 
                                           GTK_TEXT_WINDOW_TEXT, 
                                           *x,  *y, x, y             );

    GtkTextIter start;
    gtk_text_view_get_iter_at_position (GTK_TEXT_VIEW (results_tv),
                                        &start, &trailing,
                                        *x, *y                      );

    return gtk_text_iter_get_char (&start);
} 


void gwaei_ui_set_cursor(const int CURSOR)
{
    GdkWindow* gdk_window;
    gdk_window = gtk_text_view_get_window( GTK_TEXT_VIEW (results_tv), 
                                           GTK_TEXT_WINDOW_TEXT        );
    GdkCursor* cursor;
    cursor = gdk_cursor_new(CURSOR);
    gdk_window_set_cursor (gdk_window, cursor);
    gdk_cursor_unref (cursor);
}


void gwaei_open_kanji_results()
{
      //Open the hpane so the kanji results are visible
      gint window_height, window_width;

      GtkWidget *window;
      window = GTK_WIDGET (gtk_builder_get_object( builder, "main_window" ));
      gtk_window_get_size (GTK_WINDOW (window), &window_width, &window_height);

      GtkWidget *hpaned;
      hpaned= GTK_WIDGET (gtk_builder_get_object( builder, "results_hpaned" ));
      gtk_paned_set_position (GTK_PANED (hpaned), (window_width - 200)); 
}


void gwaei_close_kanji_results()
{
    gint window_height, window_width;

    GtkWidget *window;
    window = GTK_WIDGET (gtk_builder_get_object( builder, "main_window" ));
    gtk_window_get_size (GTK_WINDOW (window), &window_width, &window_height);

    GtkWidget *hpaned;
    hpaned = GTK_WIDGET (gtk_builder_get_object( builder, "results_hpaned" ));
    gtk_paned_set_position (GTK_PANED (hpaned), window_width); 
}



char* locate_offset( char *string, char *line_start, regex_t *re_locate,
                     gint *start,  gint    *end                          )
{
    //Force regex to stop searching at line breaks
    char *string_ptr = string;
    char temp;
    while(*string_ptr != '\n' && *string_ptr != '\0')
      string_ptr++;
    temp = *string_ptr;
    *string_ptr = '\0';

    size_t nmatch = 1;
    regmatch_t pmatch[nmatch];

    int status;
    if ((status = regexec(re_locate, string, 1, pmatch, 0)) == 0)
    {
      *start = g_utf8_pointer_to_offset (line_start, string + pmatch[0].rm_so);
      *end = g_utf8_pointer_to_offset (line_start, string + pmatch[0].rm_eo);
      *string_ptr = temp;
      return (string + pmatch[0].rm_eo);
    }
    else
    {
      *string_ptr = temp;
      return NULL;
    }
}

void gwaei_ui_remove_all_tags (SearchItem *item)
{
    if (item->total_results == 0) return;

    GObject* tb;
    tb = get_gobject_from_target(item->target);

    GtkTextIter start;
    gtk_text_buffer_get_start_iter (GTK_TEXT_BUFFER (tb), &start);
    GtkTextIter end;
    gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &end);

    gtk_text_buffer_remove_all_tags (GTK_TEXT_BUFFER (tb), &start, &end);
}


void gwaei_ui_add_results_tagging ( gint sl, gint el, SearchItem* item )
{
  char *text, *c, *temp_c;
  gboolean is_end_quote, is_end_parenth;
  gint cl, co;
  gint match_so, match_eo;
  gint parenth_so, parenth_eo;
  gint desc_so, desc_eo;
  gint target = item->target;
  gint re;

  for(re = 0; re < item->total_re; re++) {
    //Set up the text
    text = gwaei_ui_get_text_slice_from_buffer (target, sl, el);
    c = &text[0];

    //Position tracking variables
    cl = sl;
    co = 0;
    is_end_quote = FALSE;
    is_end_parenth = FALSE;

    while (*c != '\0' && (cl < el || *c != '\n') ) {
      //Highlight query matches whenever we hit a newline
      if (c == &text[0] || *(c - 1) == '\n') {
         //Backup the c position
         temp_c = c;

         while ((temp_c = locate_offset( temp_c, c, &item->re_locate[re],
                                         &match_so, &match_eo)) != NULL)
         {
           gwaei_ui_apply_tag_to_text (target, "match", cl, match_so,
                                                        cl, match_eo );
         }
      }

      //
      //Stuff for the kanji section
      //
      if (regexec(&re_kanji, item->dictionary->name, 1, NULL, 0) == 0 ||
          strcmp("Mix", item->dictionary->name) == 0                    )
      {
        char *nextchar;
        char *nextnextchar;
        if (*c != '\0')
          nextchar = g_utf8_next_char(c);
        else
          nextchar = NULL;
        if (*nextchar != '\0')
          nextnextchar = g_utf8_next_char(nextchar);
        else
          nextchar = NULL;

        //Highlight single kanji lines
        if (g_utf8_get_char(c) > L'カ' && nextchar != NULL &&
            *(nextchar) == '\n' && nextnextchar != NULL    &&
            *(nextnextchar) != '{' && co == 0             )
        {
           gwaei_ui_apply_tag_to_text (target, "large", cl, 0, cl, 1);
           gwaei_ui_apply_tag_to_text (target, "header", cl, 0, cl, 1);
        }
        //Bold kanji description titles
        if (*c == ':')
        {
          desc_eo = co;

          char *temp_c = c;
          char temp_offset = co;
          while (  (temp_c - 1) != text && *(temp_c - 1) != ' ' &&
                  *(temp_c - 1) != '\n' &&   temp_offset > 0        )
          {
            temp_c--;
            temp_offset--;
          }

          desc_so = temp_offset;
          gwaei_ui_apply_tag_to_text (target, "important", cl, desc_so,
                                                           cl, desc_eo);
        }
      }

      //Add highlighting to word description (adjective, verb type etc...)
      if (*c == '(' && *(c - 1) == '/') {
        is_end_parenth = TRUE;
        parenth_so = co;
      }
      else if (*c == ')' && *(c + 1) != '\0' && *(c + 1) != '(' && *(c + 2) != '(' && is_end_parenth == TRUE) {
        is_end_parenth = FALSE;
        if (*(c + 1) == '/' || *(c + 1) == ' ') {
          parenth_eo = co + 1;
          gwaei_ui_apply_tag_to_text (target, "comment", cl, parenth_so,
                                                         cl, parenth_eo);
        }
      }
      
      //Make the Japanese half of the results bold
      if (*c == '\n') {

        if (is_end_quote == TRUE) {
          is_end_quote = FALSE;
        }
        cl++;
        co = 0;
        c = g_utf8_next_char(c);
      }
      else {
        if (*c == '/') {
          is_end_parenth = FALSE;
          if ( is_end_quote == FALSE &&
               (regexec(&re_kanji, item->dictionary->name, 1, NULL, 0) != 0) )
          {
            is_end_quote = TRUE;
            gwaei_ui_apply_tag_to_text (target, "important", cl, 0, cl, co);
          }
        }
        co++;
        c = g_utf8_next_char(c);
      }
    }

    //Cleanup
    g_free(text);
  }
}


void gwaei_ui_add_all_tags (SearchItem *item)
{
    if (item->total_results == 0) return;

    GObject* tb;
    tb = get_gobject_from_target(item->target);

    gint sl;
    sl = 0;

    GtkTextIter end;
    gtk_text_buffer_get_end_iter (GTK_TEXT_BUFFER (tb), &end);
    gint el;
    el = gtk_text_iter_get_line (&end);

    gwaei_ui_add_results_tagging (sl, el, item);
}


gint32 gwaei_previous_tip = 0;
void gwaei_ui_display_no_results_found_page()
{
    gint32 temp = g_random_int_range(0,9);
    while (temp == gwaei_previous_tip)
      temp = g_random_int_range(0,9);
    const gint32 TIP_NUMBER = temp;
    gwaei_previous_tip = temp;
    char tip_number_str[5];
    gwaei_itoa((TIP_NUMBER + 1), tip_number_str, 5);


    //Add the title
    gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS, gettext("No results found!"),
                            "important", "larger", NULL, NULL    );


    //Linebreak before the image
    gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS, "\n", NULL, NULL, NULL, NULL);

    char image_name[100];
    strcpy(image_name, "character");
    if (TIP_NUMBER == 8)
      strcat(image_name, "3");
    strcat(image_name, ".png");
    
    gwaei_ui_append_image_to_buffer(GWAEI_TARGET_RESULTS, image_name);


    //Insert the instruction text
    gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS, "\n\n", NULL, NULL, NULL, NULL);

    gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS, gettext("gWaei Usage Tip #"),
                            "important", NULL, NULL, NULL         );
    gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS, tip_number_str,
                            "important", NULL, NULL, NULL         );
    gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS, gettext(": "),
                            "important", NULL, NULL, NULL         );
                            
    switch (TIP_NUMBER)
    {
      case 0:
        //Tip 1
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Inputting Unknown Kanji"),
                                   "header", "important", NULL, NULL         );
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "\n\n",
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Use the "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Unknown Character"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" from the Insert menu or toolbar in place of unknown Kanji.  "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "日.語",
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" will return results like "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "日本語",
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(".\n\nKanjipad"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" is another option for inputting Kanji characters.  Because of how the innards of Kanjipad works, drawing with the correct number of strokes and drawing the strokes in the correct direction is very important."),
                                   NULL, NULL, NULL, NULL         );
        break;

     case 1:
        //Tip 2
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Getting More Exact Matches"),
                                   "important", "header", NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "\n\n",
                                   NULL, NULL, NULL, NULL         );


        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Use the "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Word-Boundary Character"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" and the "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Not-Word-Boundary Character"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" from the insert menu to get more relevant results.  "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("fish\\b"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" will return results like "),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "fish",
                                   "important", "match", NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" and "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "sel",
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "fish",
                                   "important", "match", NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(", but not "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "fish",
                                   "important", "match", NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "erman", 
                                   "important", NULL, NULL, NULL         );

        break;

     case 2:
        //Tip 3
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Searching for Multiple Words"),
                                   "important", "header", NULL, NULL);

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "\n\n",
                                   NULL, NULL, NULL, NULL         );


        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Use the "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("And Character"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" or "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Or Character"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" to search for results that contain a combination of words that might not be right next to each other.  "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("cats&dogs"), 
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" will return only results that contain both the words cats and dogs like "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "犬猫", 
                                   "important", "match", NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   " does.", 
                                   NULL, NULL, NULL, NULL         );
        break;

     case 3:
        //Tip 4
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Make a Vocabulary List"),
                                   "important", "header", NULL, NULL);

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "\n\n",
                                   NULL, NULL, NULL, NULL         );


        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
            gettext("Specific sections of results can be printed or saved by dragging the mouse to highlight them.  Using this in combination with the "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Append"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" command from the File menu or toolbar, quick and easy creation of a vocabulary lists are possible."),
                                   NULL, NULL, NULL, NULL         );
        break;

     case 4:
        //Tip 5
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Why Use the Mouse?"),
                                   "important", "header", NULL, NULL);
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "\n\n",
                                   NULL, NULL, NULL, NULL         );


        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Typing something"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" will move the focus to the search input box.  Hitting the "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("up or down arrow key"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" will move the focus to the results pane so you can scroll the results.  Hitting the "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("tab key"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" will cycle the currently installed dictionaries."),
                                   NULL, NULL, NULL, NULL         );

        break;

     case 5:
        //Tip 6
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Get Ready for the JLPT"),
                                   "important", "header", NULL, NULL);
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "\n\n",
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("The Kanji dictionary has some hidden features.  One such one is the ability to filter out Kanji that don't meet a certain criteria.  If you are planning on taking the Japanese Language Proficiency Test, using the phrase "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("J#"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" will filter out Kanji not of that level for easy study.  For example, "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("J4"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" will only show Kanji that appears on the forth level test.\n\nAlso of interest, the phrase "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("G#"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" will filter out Kanji for the grade level a Japanese person would study it at in school."),
                                   NULL, NULL, NULL, NULL         );

        break;

     case 6:
        //Tip 7
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Just drag words in!"),
                                "important", "header", NULL, NULL);
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "\n\n",
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("If you drag and drop a highlighted word into "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("gWaei"),
                                   "important", NULL, NULL, NULL         );
            
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("'s search query input, "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("gWaei"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" will automatically start a search using that text.  This can be a nice way to quickly look up words while browsing webpages."),
                                   NULL, NULL, NULL, NULL         );

        break;

     case 7:
        //Tip 8
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("What does (adj-i) mean?"),
                                   "important", "header", NULL, NULL);
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "\n\n",
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("It is part of the terminalogy used by the EDICT group of dictionaries to categorize words.  Some are obvious, but there are a number that there is no way to know the meaning other than by looking it up.\n\n"),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("gWaei"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" includes some of the EDICT documentation in its help manual.  Click the "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Dictionary Terminology Glossary menuitem"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" in the "),
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Help menu"),
                                   "important", NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext(" to get to it."),
                                   NULL, NULL, NULL, NULL         );

        break;

     case 8:
        //Tip 9
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Books are Heavy"),
                                   "important", "header", NULL, NULL);
        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   "\n\n",
                                   NULL, NULL, NULL, NULL         );

        gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                                   gettext("Aways wear a construction helmet when working with books.  They are dangerous heavy objects that can at any point fall on and injure you.  Please all urge all of your friends to, too.  They will thank you later.  Really."),
                                   NULL, NULL, NULL, NULL         );
       break;
    }

    gwaei_ui_append_to_buffer (GWAEI_TARGET_RESULTS,
                               "\n\n",
                               NULL, NULL, NULL, NULL         );
}


const char* gwaei_ui_get_active_dictionary ()
{
    char id[50];
    GtkWidget *combobox;
    strcpy(id, "dictionary_combobox");
    combobox = GTK_WIDGET (gtk_builder_get_object(builder, id));
    return (gtk_combo_box_get_active_text(GTK_COMBO_BOX(combobox)));
}


void gwaei_ui_set_active_dictionary_by_name (char* requested_name)
{
    //Get a reference to the combobox and it's text contents
    char id[50];
    GtkWidget *combobox;
    strncpy (id, "dictionary_combobox", 50);
    combobox = GTK_WIDGET (gtk_builder_get_object(builder, id));

    char name[MAX_DICTIONARY];
    if (strcmp (requested_name, "Mix") == 0)
      strncpy(name, "Kanji", MAX_DICTIONARY);
    else
      strncpy(name, requested_name, MAX_DICTIONARY);


    //Set the dictionary in the GtkCombobox
    int i = 0;
    while (gwaei_ui_get_active_dictionary () != NULL             &&
           strcmp (gwaei_ui_get_active_dictionary (), name) != 0   )
    {
      gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), i);
      i++;
    }

    if (gwaei_ui_get_active_dictionary () == NULL)
      gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);
}


void gwaei_ui_next_dictionary()
{
    //Declarations
    char id[50];
    char *active_text;
    GtkWidget *combobox;
    strcpy (id, "dictionary_combobox");
    combobox = GTK_WIDGET (gtk_builder_get_object(builder, id));

    gint active;
    active = gtk_combo_box_get_active( GTK_COMBO_BOX (combobox) );
}


void gwaei_ui_cycle_dictionaries(gboolean cycle_forward)
{
    char id[50];
    int increment;

    if (cycle_forward)
      increment = 1;
    else
      increment = -1;

    //Declarations
    GtkWidget *combobox;
    strcpy (id, "dictionary_combobox");
    combobox = GTK_WIDGET (gtk_builder_get_object(builder, id));

    gint active;
    active = gtk_combo_box_get_active( GTK_COMBO_BOX (combobox) );
    char *active_text;

    if ((active = active + increment) == -1)
    {
      do {
        active++;
        gtk_combo_box_set_active( GTK_COMBO_BOX (combobox), active);
        active_text = gtk_combo_box_get_active_text(GTK_COMBO_BOX (combobox));
      } while (active_text != NULL);
      active--;
      gtk_combo_box_set_active( GTK_COMBO_BOX (combobox), active);
    }
    else
    {
      gtk_combo_box_set_active( GTK_COMBO_BOX (combobox), active);
      active_text = gtk_combo_box_get_active_text(GTK_COMBO_BOX (combobox));
      if ( active_text == NULL )
        gtk_combo_box_set_active( GTK_COMBO_BOX (combobox), 0 );
    }
}


void gwaei_ui_cycle_dictionaries_forward ()
{
    gwaei_ui_cycle_dictionaries(TRUE);
}


void gwaei_ui_cycle_dictionaries_backward ()
{
    gwaei_ui_cycle_dictionaries(FALSE);
}


char* gwaei_ui_get_text_from_text_buffer(const int TARGET)
{
  GObject* tb;
  tb = get_gobject_from_target(TARGET);

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


void gwaei_reload_tagtable_tags()
{
  HistoryList* hl;
  hl = historylist_get_list (GWAEI_HISTORYLIST_RESULTS);

  if (hl != NULL && hl->current != NULL)
    gwaei_ui_remove_all_tags (hl->current);

  gwaei_ui_set_color_to_tagtable ("comment", GWAEI_TARGET_RESULTS, TRUE, FALSE);
  gwaei_ui_set_color_to_tagtable ("comment", GWAEI_TARGET_KANJI,   TRUE, FALSE);

  gwaei_ui_set_color_to_tagtable ("match",   GWAEI_TARGET_RESULTS, TRUE, TRUE );
  gwaei_ui_set_color_to_tagtable ("match",   GWAEI_TARGET_KANJI,   TRUE, TRUE );

  gwaei_ui_set_color_to_tagtable ("header",  GWAEI_TARGET_RESULTS, TRUE, FALSE);
  gwaei_ui_set_color_to_tagtable ("header",  GWAEI_TARGET_KANJI,   TRUE, TRUE );

  if (hl != NULL && hl->current != NULL)
    gwaei_ui_add_all_tags (hl->current);
}


void gwaei_initialize_tags()
{
    //Important tag (usually bold)
    gwaei_ui_set_tag_to_tagtable ("important", GWAEI_TARGET_RESULTS,
                                  "weight",    GINT_TO_POINTER(PANGO_WEIGHT_BOLD));
    gwaei_ui_set_tag_to_tagtable ("important", GWAEI_TARGET_KANJI,
                                  "weight",    GINT_TO_POINTER(PANGO_WEIGHT_BOLD));

    //Larger tag
    gwaei_ui_set_tag_to_tagtable ("larger", GWAEI_TARGET_RESULTS, "font", "sans 20");

    //Large tag
    gwaei_ui_set_tag_to_tagtable ("large", GWAEI_TARGET_RESULTS, "font", "serif 30");
    gwaei_ui_set_tag_to_tagtable ("large", GWAEI_TARGET_KANJI,   "font", "serif 40");

    //Small tag
    gwaei_ui_set_tag_to_tagtable ("small", GWAEI_TARGET_RESULTS,  "font", "serif 6");
    gwaei_ui_set_tag_to_tagtable ("small", GWAEI_TARGET_KANJI,    "font", "serif 6");

    gwaei_reload_tagtable_tags();
}


void initialize_gui_interface(int *argc, char ***argv)
{
    //Initialize some libraries
    gdk_threads_init();
    gtk_init (argc, argv);

    //Setup the main windows xml
    builder = gtk_builder_new ();
    gwaei_ui_load_gtk_builder_xml("main.xml");
    gwaei_ui_load_gtk_builder_xml("radicals.xml");
    gwaei_ui_load_gtk_builder_xml("settings.xml");
    gwaei_ui_load_gtk_builder_xml("kanjipad.xml");

    //Initialize some component and variables
    initialize_global_widget_pointers();
    gwaei_initialize_tags();
    initialize_kanjipad();
    initialize_history_popups();

    gwaei_sexy_initialize_libsexy();
    gwaei_ui_update_history_popups();
    gwaei_ui_show_window ("main_window");
    gwaei_prefs_initialize_preferences();
    gwaei_settings_combobox_hack ();

    if (rebuild_combobox_dictionary_list() == 0) {
      do_settings(NULL, NULL);
    }

    //Set the initial focus to the search bar
    gwaei_ui_grab_focus_by_target (GWAEI_TARGET_ENTRY);

    //Enter the main loop
    gdk_threads_enter();
    gtk_main ();
    gdk_threads_leave();
}


gboolean gwaei_ui_cancel_search_by_target(const int TARGET)
{
    HistoryList* hl = historylist_get_list(GWAEI_HISTORYLIST_RESULTS);
    SearchItem *item = hl->current;

    if (item != NULL && item->status == GWAEI_SEARCH_CANCELING) return FALSE;

    if (item == NULL || item->status == GWAEI_SEARCH_IDLE) return TRUE;

    item->status = GWAEI_SEARCH_CANCELING;

    while (item->status != GWAEI_SEARCH_IDLE)
    {
      gdk_threads_leave();
      g_main_context_iteration (NULL, FALSE);
      gdk_threads_enter();
    }
    return TRUE;
}
