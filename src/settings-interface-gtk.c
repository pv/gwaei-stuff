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
//! @file src/settings-interface-gtk.c
//!
//! @brief Abstraction layer for gtk objects
//!
//! Used as a go between for functions interacting with GUI interface objects.
//! widgets.
//!


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





char* gw_parse_widget_name (char *output, GtkWidget *widget,
                               gboolean capitalize             )
{
    //Declarations
    const char* input = gtk_widget_get_name(widget);
    const char *input_ptr = input;
    char *output_ptr = output;

    //Copy up to the underscore
    while (*input_ptr != '\0' && *input_ptr != '_') {
      *output_ptr = *input_ptr;
      output_ptr++;
      input_ptr++;
    }
    *output_ptr = '\0';

    //Capitalize the first letter
    if (capitalize == TRUE) output[0] -= 0x20;

    //Finish
    return output;
}


void gw_ui_update_settings_interface()
{
  char id[50];

  //Set the install interface
  GtkWidget *close_button;
  strcpy(id, "settings_close_button");
  close_button = GTK_WIDGET (gtk_builder_get_object(builder, id));

  GtkWidget *install_table;
  strcpy(id, "dictionaries_table");
  install_table = GTK_WIDGET (gtk_builder_get_object(builder, id));

  GtkWidget *advanced_tab;
  strcpy(id, "advanced_tab");
  advanced_tab = GTK_WIDGET (gtk_builder_get_object (builder, id));

  GtkWidget *update_button;
  strcpy(id, "update_install_button");
  update_button = GTK_WIDGET (gtk_builder_get_object(builder, id));

  GtkWidget *updated_button;
  strcpy(id, "update_remove_button");
  updated_button = GTK_WIDGET (gtk_builder_get_object(builder, id));

  GtkWidget *rebuild_button;
  strcpy(id, "force_rebuild_button");
  rebuild_button = GTK_WIDGET (gtk_builder_get_object(builder, id));

  GtkWidget *resplit_button;
  strcpy(id, "force_resplit_button");
  resplit_button = GTK_WIDGET (gtk_builder_get_object(builder, id));


  if (gw_dictlist_get_total_with_status (GW_DICT_STATUS_UPDATING) > 0)
  {
    gtk_widget_set_sensitive (close_button,   FALSE);
    gtk_widget_set_sensitive (install_table,  FALSE);
    gtk_widget_set_sensitive (rebuild_button, FALSE);
    gtk_widget_set_sensitive (resplit_button, FALSE);
    gtk_widget_set_sensitive (update_button,  TRUE );
    gtk_widget_set_sensitive (updated_button, TRUE );
  }
  else if (gw_dictlist_get_total_with_status (GW_DICT_STATUS_INSTALLING) > 0)
  {
    gtk_widget_set_sensitive (close_button,   FALSE);
    gtk_widget_set_sensitive (install_table,  TRUE );
    gtk_widget_set_sensitive (rebuild_button, FALSE);
    gtk_widget_set_sensitive (resplit_button, FALSE);
    gtk_widget_set_sensitive (update_button,  FALSE);
    gtk_widget_set_sensitive (updated_button, FALSE);
  }
  else if (gw_dictlist_get_total_with_status (GW_DICT_STATUS_REBUILDING) > 0)
  {
    gtk_widget_set_sensitive (close_button,   FALSE);
    gtk_widget_set_sensitive (install_table,  FALSE);
    gtk_widget_set_sensitive (rebuild_button, FALSE);
    gtk_widget_set_sensitive (resplit_button, FALSE);
    gtk_widget_set_sensitive (update_button,  FALSE);
    gtk_widget_set_sensitive (updated_button, FALSE);
  }
  else
  {
    gtk_widget_set_sensitive (close_button,   TRUE );
    gtk_widget_set_sensitive (install_table,  TRUE );
    gtk_widget_set_sensitive (update_button,  TRUE );
    gtk_widget_set_sensitive (updated_button, TRUE );

    gboolean state;
    state = (gw_dictlist_dictionary_get_status_by_id(GW_DICT_MIX) == GW_DICT_STATUS_INSTALLED);
    gtk_widget_set_sensitive (rebuild_button, state);
    state = (gw_dictlist_dictionary_get_status_by_id(GW_DICT_NAMES) == GW_DICT_STATUS_INSTALLED);
    gtk_widget_set_sensitive (resplit_button, state);
  }
}

void gw_ui_set_dictionary_source(const char* id, const char* value)
{
    GtkWidget *widget;
    widget = GTK_WIDGET (gtk_builder_get_object(builder, id));

    g_signal_handlers_block_by_func(widget, do_source_entry_changed_action, NULL);
    gtk_entry_set_text(GTK_ENTRY (widget), value);
    g_signal_handlers_unblock_by_func(widget, do_source_entry_changed_action, NULL);
}


//Sets the status of an individual feature
void gw_ui_set_feature_line_status(char* name, char* status)
{
    char id[100];
    strcpy(id, name);
    char* suffix = &id[strlen(id)];

    //Get the widget pointers
    GtkWidget *label        = NULL;
    GtkWidget *icon_enabled = NULL;
    GtkWidget *icon_warning = NULL;

    strcpy(suffix, "_enabled_label");
    label = GTK_WIDGET (gtk_builder_get_object(builder, id));
    strcpy(suffix, "_enabled_icon");
    icon_enabled = GTK_WIDGET (gtk_builder_get_object(builder, id));
    if (strcmp(name, "general") == 0) {
      strcpy(suffix, "_warning_icon");
      icon_warning = GTK_WIDGET (gtk_builder_get_object(builder, id));
    }

    //Set the show/hide states of the widgets
    if (strcmp(status, "enabled") == 0) {
      if (icon_warning != NULL)
        gtk_widget_hide( icon_warning );
      gtk_widget_show( icon_enabled );
      gtk_label_set_text( GTK_LABEL (label), gettext("Enabled"));
    }
    else if (strcmp(status, "disabled") == 0) {
      gtk_widget_hide( icon_enabled );
      if (icon_warning != NULL)
        gtk_widget_show( icon_warning );
      gtk_label_set_text( GTK_LABEL (label), gettext("Disabled"));
    }
}

int gw_ui_get_install_line_status(char *name)
{
    GtkWidget *button;

    char id[100];
    strcpy(id, name);
    char *suffix = &id[strlen(id)];

    strcpy(suffix, "_install_button");
    button = GTK_WIDGET (gtk_builder_get_object (builder, id));
    if (GTK_WIDGET_VISIBLE (button) == TRUE)
      return GW_DICT_STATUS_NOT_INSTALLED;

    strcpy(suffix, "_remove_button");
    button = GTK_WIDGET (gtk_builder_get_object (builder, id));
    if (GTK_WIDGET_VISIBLE (button) == TRUE)
      return GW_DICT_STATUS_INSTALLED;

    strcpy(suffix, "_cancel_button");
    button = GTK_WIDGET (gtk_builder_get_object (builder, id));
    if (GTK_WIDGET_VISIBLE (button) == TRUE && GTK_WIDGET_SENSITIVE(button) == TRUE)
      return GW_DICT_STATUS_INSTALLING;
    if (GTK_WIDGET_VISIBLE (button) == TRUE && GTK_WIDGET_SENSITIVE (button) == FALSE)
      return GW_DICT_STATUS_CANCELING;

}

//Sets the install status of an individual dictionary
void gw_ui_set_install_line_status(char *name, char *status, char *message)
{
    GtkWidget *install_button, *remove_button, *cancel_button;
    GtkWidget *hbox, *label, *icon_installed, *icon_errored, *progressbar;
    GtkWidget *advanced_hbox;

    char id[100];
    strcpy(id, name);
    char *suffix = &id[strlen(id)];

    strcpy(suffix, "_install_button");
    install_button = GTK_WIDGET (gtk_builder_get_object (builder, id));

    strcpy(suffix, "_remove_button");
    remove_button = GTK_WIDGET (gtk_builder_get_object (builder, id));

    strcpy(suffix, "_cancel_button");
    cancel_button = GTK_WIDGET (gtk_builder_get_object (builder, id));

    strcpy(suffix, "_install_progressbar");
    progressbar = GTK_WIDGET (gtk_builder_get_object (builder, id));

    strcpy(suffix, "_install_hbox");
    hbox = GTK_WIDGET (gtk_builder_get_object (builder, id));

    strcpy(suffix, "_install_label");
    label = GTK_WIDGET (gtk_builder_get_object (builder, id));

    strcpy(suffix, "_icon_installed");
    icon_installed = GTK_WIDGET (gtk_builder_get_object (builder, id));

    strcpy(suffix, "_icon_errored");
    icon_errored = GTK_WIDGET (gtk_builder_get_object (builder, id));

    //advanced settings items
    strcpy(suffix, "_advanced_hbox");
    if (strcmp(name, "update") != 0)
      advanced_hbox = GTK_WIDGET (gtk_builder_get_object (builder, id));

    if (strcmp(status, "finishing") == 0)
    {
      gtk_widget_hide(remove_button);
      gtk_widget_show(cancel_button);
      gtk_widget_set_sensitive(cancel_button, FALSE);
      gtk_widget_hide(install_button);

      gtk_label_set_text(GTK_LABEL (label), gettext("Finishing...")); 
      gtk_widget_show(label);
      gtk_widget_hide(icon_errored);
      gtk_widget_hide(icon_installed);
      gtk_widget_hide(progressbar);
      gtk_widget_show(hbox);
    }
    if (strcmp(status, "cancelling") == 0)
    {
      gtk_widget_hide(remove_button);
      gtk_widget_show(cancel_button);
      gtk_widget_set_sensitive(cancel_button, FALSE);
      gtk_widget_hide(install_button);

      gtk_label_set_text(GTK_LABEL (label), gettext("Cancelling...")); 
      gtk_widget_show(label);
      gtk_widget_hide(icon_errored);
      gtk_widget_hide(icon_installed);
      gtk_widget_hide(progressbar);
      gtk_widget_show(hbox);
    }
    else if (strcmp(status, "cancel") == 0)
    {
      gtk_widget_hide(remove_button);
      gtk_widget_show(cancel_button);
      gtk_widget_set_sensitive(cancel_button, TRUE);
      gtk_widget_hide(install_button);

      char text[100];
      if (message != NULL)
      {
        strcpy(text, message);
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR (progressbar), text);
      }
      else
      {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR (progressbar), 0.0);
      }
      gtk_widget_hide(hbox);
      gtk_widget_show(progressbar);

      if (strcmp(name, "update") != 0)
        gtk_widget_set_sensitive(advanced_hbox, FALSE);
    }
    else if (strcmp(status, "install") == 0)
    {
      gtk_widget_hide(remove_button);
      gtk_widget_hide(cancel_button);
      gtk_widget_show(install_button);

      char text[100];
      if (message != NULL)
      {
        strcpy(text, message);
      }
      else {
        strcpy(text, gettext("Not Installed"));
      }
      gtk_label_set_text(GTK_LABEL (label), text); 
      gtk_widget_show(label);
      gtk_widget_hide(icon_errored);
      gtk_widget_hide(icon_installed);
      gtk_widget_hide(progressbar);
      gtk_widget_show(hbox);

      if (strcmp(name, "update") != 0)
        gtk_widget_set_sensitive(advanced_hbox, TRUE);
    }
    else if (strcmp(status, "remove") == 0)
    {
      gtk_widget_show(remove_button);
      gtk_widget_hide(cancel_button);  
      gtk_widget_hide(install_button);

      char text[100];
      if (message != NULL)
      {
        strcpy(text, message);
        gtk_label_set_text(GTK_LABEL (label), text); 
      }
      else
      {
        gtk_label_set_text(GTK_LABEL (label), gettext("Installed")); 
      }

      gtk_widget_show(label);
      gtk_widget_hide(icon_errored);
      gtk_widget_show(icon_installed);
      gtk_widget_hide(progressbar);
      gtk_widget_show(hbox);

      if (strcmp(name, "update") != 0)
        gtk_widget_set_sensitive(advanced_hbox, FALSE);
    }
    else if (strcmp(status, "error") == 0)
    {
      //Prepare the message text
      char text[100];
      strcpy(text, gettext("Errored"));
      if (message != NULL)
      {
        strcat(text, ": ");
        strcat(text, message);
      }

      gtk_widget_hide(remove_button);
      gtk_widget_hide(cancel_button);  
      gtk_widget_show(install_button);

      gtk_widget_show(label);
      gtk_label_set_text(GTK_LABEL (label), text); 
      gtk_widget_show(icon_errored);
      gtk_widget_hide(icon_installed);
      gtk_widget_hide(progressbar);
      gtk_widget_show(hbox);

      if (strcmp(name, "update") != 0)
        gtk_widget_set_sensitive(advanced_hbox, TRUE);
    }

    GtkWidget *button;
    gboolean sensitive;

    sensitive = (rsync_exists && gw_dictlist_get_total() &&
                 gw_dictlist_get_total_with_status(GW_DICT_STATUS_INSTALLING) == 0);

    strcpy(id, "update_install_button");
    button = GTK_WIDGET (gtk_builder_get_object(builder, id));
    gtk_widget_set_sensitive(button, sensitive);

    strcpy(id, "update_remove_button");
    button = GTK_WIDGET (gtk_builder_get_object(builder, id));
    gtk_widget_set_sensitive(button, sensitive);
}


//The layout of this function is specifically for a libcurl callback
int gw_ui_update_progressbar (void   *id,
                              double  dltotal,
                              double  dlnow,
                              double  ultotal,
                              double  ulnow   )
{
    gdk_threads_enter();

    GtkWidget *progressbar;
    progressbar = GTK_WIDGET (gtk_builder_get_object(builder, (char*) id));

    if (GTK_WIDGET_VISIBLE (progressbar) == TRUE) {
      if (dlnow == 0.0) {
        gtk_progress_bar_pulse (GTK_PROGRESS_BAR (progressbar));  
        gtk_progress_bar_set_text (GTK_PROGRESS_BAR (progressbar), " ");
      }
      else {
        double ratio = dlnow / dltotal;
        char *text = gettext("Downloading...");
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR (progressbar), ratio);
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR (progressbar), text);
      }

      gdk_threads_leave ();
      return FALSE;
    }

    gdk_threads_leave ();
    return TRUE;
}


void gw_ui_set_progressbar (char *name, double percent, char *message)
{
    char id[50];
    strcpy (id, name);
    strcat(id, "_install_progressbar");

    GtkWidget *progressbar;
    progressbar = GTK_WIDGET (gtk_builder_get_object (builder, id));

    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR (progressbar), percent);

    if (message != NULL)
      gtk_progress_bar_set_text(GTK_PROGRESS_BAR (progressbar), message);
}

void gw_settings_initialize_enabled_features_list()
{
    //General search
    if (gw_dictlist_get_total_with_status(GW_DICT_STATUS_INSTALLED) > 0)
      gw_ui_set_feature_line_status("general", "enabled");
    else
      gw_ui_set_feature_line_status("general", "disabled");

    //Combined dictionary
    if (gw_dictlist_dictionary_get_status_by_id(GW_DICT_MIX) == GW_DICT_STATUS_INSTALLED)
      gw_ui_set_feature_line_status("mix", "enabled");
    else
      gw_ui_set_feature_line_status("mix", "disabled");

    //Radical search tool
    if (gw_dictlist_dictionary_get_status_by_id(GW_DICT_RADICALS) == GW_DICT_STATUS_INSTALLED)
      gw_ui_set_feature_line_status("radical", "enabled");
    else
      gw_ui_set_feature_line_status("radical", "disabled");

    //Kanji lookup tool
    if (gw_dictlist_dictionary_get_status_by_id(GW_DICT_KANJI) == GW_DICT_STATUS_INSTALLED)
      gw_ui_set_feature_line_status("kanji", "enabled");
    else
      gw_ui_set_feature_line_status("kanji", "disabled");

    GtkWidget *label;
    label = GTK_WIDGET (gtk_builder_get_object(builder, "update_install_label"));
    if (rsync_exists)
      gtk_label_set_text(GTK_LABEL (label), gettext("Requires rsync to be installed"));
}


void gw_settings_initialize_installed_dictionary_list() 
{
    if (gw_dictlist_dictionary_get_status_by_id(GW_DICT_ENGLISH) == GW_DICT_STATUS_INSTALLED)
      gw_ui_set_install_line_status("english", "remove", NULL);
    else
      gw_ui_set_install_line_status("english", "install", NULL);

    if (gw_dictlist_dictionary_get_status_by_id(GW_DICT_KANJI) == GW_DICT_STATUS_INSTALLED)
      gw_ui_set_install_line_status("kanji", "remove", NULL);
    else
      gw_ui_set_install_line_status("kanji", "install", NULL);

    if (gw_dictlist_dictionary_get_status_by_id(GW_DICT_NAMES) == GW_DICT_STATUS_INSTALLED)
      gw_ui_set_install_line_status("names", "remove", NULL);
    else
      gw_ui_set_install_line_status("names", "install", NULL);

    if (gw_dictlist_dictionary_get_status_by_id(GW_DICT_RADICALS) == GW_DICT_STATUS_INSTALLED)
      gw_ui_set_install_line_status("radicals", "remove", NULL);
    else
      gw_ui_set_install_line_status("radicals", "install", NULL);

    if (gw_dictlist_dictionary_get_status_by_id(GW_DICT_EXAMPLES) == GW_DICT_STATUS_INSTALLED)
      gw_ui_set_install_line_status("examples", "remove", NULL);
    else
      gw_ui_set_install_line_status("examples", "install", NULL);
}


void gw_ui_update_dictionary_orders ()
{
    //Parse the the names of the dictionary list
    char order[5000];
    gw_pref_get_string (order, GCKEY_GW_LOAD_ORDER, GW_LOAD_ORDER_FALLBACK, 5000);
    char *dictionaries[50];
    dictionaries[0] = order;
    int i = 0;
    while ((dictionaries[i + 1] = g_utf8_strchr (dictionaries[i], -1, L',')) && i < 50)
    {
      i++;
      *dictionaries[i] = '\0';
      dictionaries[i]++;
    }
    dictionaries[i + 1] = '\0';

    GtkWidget *label, *container, *dictionary, *move_up_button, *move_down_button, *button_box, *button_image, *number_label, *eventbox;
    GtkWidget *icon_image;
    container = GTK_WIDGET (gtk_builder_get_object (builder, "organize_dictionary_list_hbox"));
    GList *list;
    //Clear out old buttons
    list = gtk_container_get_children (GTK_CONTAINER (container));
    while (list != NULL)
    {
      gtk_widget_destroy (GTK_WIDGET(list->data));
      list = gtk_container_get_children (GTK_CONTAINER (container));
    }

    //Add new buttons
    i = 0;
    char *markup;
    GwDictInfo* di = NULL;
    while (dictionaries[i] != NULL)
    {
      eventbox = gtk_event_box_new();
      if (i == 0)
        icon_image = gtk_image_new_from_icon_name ("emblem-favorite", GTK_ICON_SIZE_LARGE_TOOLBAR);
      else
        icon_image = gtk_image_new_from_stock (GTK_STOCK_YES, GTK_ICON_SIZE_MENU);

      number_label = GTK_WIDGET (gtk_label_new (NULL));
      markup = g_markup_printf_escaped ("<span weight=\"bold\">%d</span>", i + 1);
      gtk_label_set_markup (GTK_LABEL (number_label), markup);
      g_free (markup);

      label = GTK_WIDGET (gtk_label_new (NULL));
      if (di = gw_dictlist_get_dictionary_by_name (dictionaries[i]))
        markup = g_markup_printf_escaped ("<span size=\"larger\">%s</span>", di->long_name);
      else
        markup = g_markup_printf_escaped ("<span size=\"larger\">%s</span>", dictionaries[i]);

      gtk_label_set_markup (GTK_LABEL (label), markup);
      g_free (markup);

      dictionary = GTK_WIDGET (gtk_hbox_new (TRUE, 5));
      gtk_container_set_border_width (GTK_CONTAINER (dictionary), 10);
      button_box = GTK_WIDGET (gtk_hbox_new (TRUE, 5));

      move_up_button = GTK_WIDGET (gtk_button_new ());
      gtk_button_set_relief (GTK_BUTTON (move_up_button), GTK_RELIEF_NONE);
      button_image = GTK_WIDGET (gtk_image_new_from_stock (GTK_STOCK_GO_UP, GTK_ICON_SIZE_MENU));
      gtk_container_add (GTK_CONTAINER (move_up_button), button_image);
      if (i == 0) gtk_widget_set_sensitive (move_up_button, FALSE);
      g_signal_connect( G_OBJECT (move_up_button),       "clicked",
                        G_CALLBACK (do_move_dictionary_up), GINT_TO_POINTER(i));

      move_down_button = GTK_WIDGET (gtk_button_new ());
      gtk_button_set_relief (GTK_BUTTON (move_down_button), GTK_RELIEF_NONE);
      button_image = GTK_WIDGET (gtk_image_new_from_stock (GTK_STOCK_GO_DOWN, GTK_ICON_SIZE_MENU));
      gtk_container_add (GTK_CONTAINER (move_down_button), button_image);
      g_signal_connect( G_OBJECT (move_down_button),       "clicked",
                        G_CALLBACK (do_move_dictionary_down), GINT_TO_POINTER(i));
      
      gtk_box_pack_start (GTK_BOX (button_box), move_up_button, FALSE, FALSE, 0);
      gtk_box_pack_start (GTK_BOX (button_box), move_down_button, FALSE, FALSE, 0);
      gtk_box_pack_end (GTK_BOX (dictionary), button_box, FALSE, FALSE, 5);

      gtk_box_pack_start (GTK_BOX (dictionary), icon_image, FALSE, FALSE, 5);
      gtk_box_pack_start (GTK_BOX (dictionary), number_label, FALSE, FALSE, 5);
      GtkWidget *temp = GTK_WIDGET (gtk_hbox_new (FALSE, 5));
      gtk_box_pack_start (GTK_BOX (temp), label, FALSE, FALSE, 5);
      gtk_box_pack_start (GTK_BOX (dictionary), temp, TRUE, TRUE, 5);

      if (i % 2)
      {
        gtk_container_add (GTK_CONTAINER (eventbox), dictionary);
        gtk_box_pack_start (GTK_BOX (container), eventbox, FALSE,FALSE, 0);
        gtk_widget_show_all (eventbox);
      }
      else
      {
        gtk_box_pack_start (GTK_BOX (container), dictionary, FALSE, FALSE, 0);
        gtk_widget_show_all (dictionary);
      }
      
      i++;
    }
    gtk_widget_set_sensitive (move_down_button, FALSE);
}
