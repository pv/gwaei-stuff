/******************************************************************************

  FILE:
  src/interface.c

  DESCRIPTION:
  Used as a go between for functions interacting with GUI interface objects.
  widgets.

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


void gwaei_ui_update_settings_interface()
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


  if (dictionarylist_get_total_with_status (UPDATING) > 0)
  {
    gtk_widget_set_sensitive (close_button,   FALSE);
    gtk_widget_set_sensitive (install_table,  FALSE);
    gtk_widget_set_sensitive (rebuild_button,  FALSE);
    gtk_widget_set_sensitive (resplit_button,    FALSE);
    gtk_widget_set_sensitive (update_button,  TRUE );
    gtk_widget_set_sensitive (updated_button, TRUE );
  }
  else if (dictionarylist_get_total_with_status (INSTALLING) > 0)
  {
    gtk_widget_set_sensitive (close_button,   FALSE);
    gtk_widget_set_sensitive (install_table,  TRUE );
    gtk_widget_set_sensitive (rebuild_button,  FALSE);
    gtk_widget_set_sensitive (resplit_button,    FALSE);
    gtk_widget_set_sensitive (update_button,  FALSE);
    gtk_widget_set_sensitive (updated_button, FALSE);
  }
  else if (dictionarylist_get_total_with_status (REBUILDING) > 0)
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
    state = (dictionarylist_dictionary_get_status_by_id(MIX) == INSTALLED);
    gtk_widget_set_sensitive (rebuild_button, state);
    state = (dictionarylist_dictionary_get_status_by_id(NAMES) == INSTALLED);
    gtk_widget_set_sensitive (resplit_button, state);
  }
}

void gwaei_ui_set_dictionary_source(const char* id, const char* value)
{
    GtkWidget *widget;
    widget = GTK_WIDGET (gtk_builder_get_object(builder, id));

    g_signal_handlers_block_by_func(widget, do_source_entry_changed_action, NULL);
    gtk_entry_set_text(GTK_ENTRY (widget), value);
    g_signal_handlers_unblock_by_func(widget, do_source_entry_changed_action, NULL);
}


//Sets the status of an individual feature
void gwaei_ui_set_feature_line_status(char* name, char* status)
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

int gwaei_ui_get_install_line_status(char *name)
{
    GtkWidget *button;

    char id[100];
    strcpy(id, name);
    char *suffix = &id[strlen(id)];

    strcpy(suffix, "_install_button");
    button = GTK_WIDGET (gtk_builder_get_object (builder, id));
    if (GTK_WIDGET_VISIBLE (button) == TRUE)
      return NOT_INSTALLED;

    strcpy(suffix, "_remove_button");
    button = GTK_WIDGET (gtk_builder_get_object (builder, id));
    if (GTK_WIDGET_VISIBLE (button) == TRUE)
      return INSTALLED;

    strcpy(suffix, "_cancel_button");
    button = GTK_WIDGET (gtk_builder_get_object (builder, id));
    if (GTK_WIDGET_VISIBLE (button) == TRUE && GTK_WIDGET_SENSITIVE(button) == TRUE)
      return INSTALLING;
    if (GTK_WIDGET_VISIBLE (button) == TRUE && GTK_WIDGET_SENSITIVE (button) == FALSE)
      return CANCELING;

}

//Sets the install status of an individual dictionary
void gwaei_ui_set_install_line_status(char *name, char *status, char *message)
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

    sensitive = (rsync_exists && dictionarylist_get_total() &&
                 dictionarylist_get_total_with_status(INSTALLING) == 0);

    strcpy(id, "update_install_button");
    button = GTK_WIDGET (gtk_builder_get_object(builder, id));
    gtk_widget_set_sensitive(button, sensitive);

    strcpy(id, "update_remove_button");
    button = GTK_WIDGET (gtk_builder_get_object(builder, id));
    gtk_widget_set_sensitive(button, sensitive);
}


//The layout of this function is specifically for a libcurl callback
int gwaei_ui_update_progressbar (void   *id,
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


void gwaei_ui_set_progressbar (char *name, double percent, char *message)
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


void gwaei_settings_combobox_hack ()
{
    //Rebuild the combobox since gtkbuilder doesn't seem to localize it...
    GtkWidget *combobox;
    combobox = GTK_WIDGET (gtk_builder_get_object(builder, "query_romanji_to_kana"));
    int active = gtk_combo_box_get_active (GTK_COMBO_BOX(combobox));
    gtk_combo_box_append_text(GTK_COMBO_BOX (combobox), gettext("When Possible"));
    gtk_combo_box_append_text(GTK_COMBO_BOX (combobox), gettext("Never"));
    gtk_combo_box_append_text(GTK_COMBO_BOX (combobox), gettext("Only for Non-Japanese Locales"));
    gtk_combo_box_remove_text (GTK_COMBO_BOX(combobox), 0);
    gtk_combo_box_remove_text (GTK_COMBO_BOX(combobox), 0);
    gtk_combo_box_remove_text (GTK_COMBO_BOX(combobox), 0);
    gtk_combo_box_set_active (GTK_COMBO_BOX(combobox), active);
}


