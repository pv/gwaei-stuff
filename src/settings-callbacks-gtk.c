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
//! @file src/settings-callbacks-gtk.c
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

#include <gwaei/gtk.h>
#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/settings.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>
#include <gwaei/interface.h>
#include <gwaei/preferences.h>



static void *update_thread(void *nothing)
{
    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);
    GError *error = NULL;

    char text[200];

    GwDictInfo* kanji;
    kanji = gw_dictlist_get_dictionary_by_id (GW_DICT_KANJI);
    GwDictInfo* names;
    names = gw_dictlist_get_dictionary_by_id (GW_DICT_NAMES);
    GwDictInfo* radicals;
    radicals = gw_dictlist_get_dictionary_by_id (GW_DICT_RADICALS);

    //Find out how many dictionaries need updating
    gdouble total_dictionary_updates = 0.0;
    gdouble extra_processing_jobs = 0.0;

    GList *dictionarylist = gw_dictlist_get_list();
    GList *updatelist = NULL;
    GwDictInfo* di;

    while (dictionarylist != NULL)
    {
      di = (GwDictInfo*)dictionarylist->data;
      if (di->status == GW_DICT_STATUS_INSTALLED && strlen(di->rsync) > 1)
      {
        updatelist = g_list_append (updatelist, di);
        di->status = GW_DICT_STATUS_UPDATING;
        total_dictionary_updates++;

        if (di->id == GW_DICT_KANJI && radicals->status == GW_DICT_STATUS_INSTALLED)
          extra_processing_jobs++;
        else if (di->id == GW_DICT_NAMES)
          extra_processing_jobs++;
      }
      dictionarylist = dictionarylist->next;
    }

    gdouble dividend;
    dividend = 1.0;
    gdouble divisor;
    divisor = (total_dictionary_updates * 2.0) + extra_processing_jobs;
    gdouble error_deviation;
    error_deviation = 0.0001;
    gdouble increment;
    increment = (dividend / divisor) - error_deviation;
    gdouble progress = 0.0;

    gdk_threads_enter();
    gw_ui_set_install_line_status("update",  "cancel",  NULL);
    gw_ui_update_settings_interface();
    gdk_threads_leave();

    while (updatelist != NULL && gw_ui_get_install_line_status("update") != GW_DICT_STATUS_CANCELING)
    {
      di = (GwDictInfo*) updatelist->data;

      char *path = di->path;
      char *sync_path = di->sync_path;

      if (error == NULL)
      {
        gdk_threads_enter();
        strcpy(text, gettext("Syncing "));
        strcat(text, gettext(di->name));
        strcat(text, gettext(" dictionary with server..."));
        gw_ui_set_progressbar ("update", progress, text);
        gdk_threads_leave();

        if (system (di->rsync) != 0)
        {
          const char *message = gettext("Connection failure\n");
          error = g_error_new_literal (quark, GW_FILE_ERROR, message);
        }
      }
      progress += increment;
      
      if (error == NULL)
      {
        gdk_threads_enter();
        strcpy(text, gettext("Finalizing "));
        strcat(text, gettext(di->name));
        strcat(text, gettext(" dictionary changes..."));
        gw_ui_set_progressbar ("update", progress, text);
        gdk_threads_leave();
        gw_io_copy_with_encoding(sync_path, path, "EUC-JP","UTF-8", &error);
      }
      progress += increment;

      //Special dictionary post processing
      if (error == NULL)
      {
        if (di->id == GW_DICT_KANJI && 
            gw_dictlist_dictionary_get_status_by_id(GW_DICT_RADICALS) == GW_DICT_STATUS_INSTALLED)
        {
          gdk_threads_enter ();
          strcpy(text, gettext("Recreating Mixed dictionary..."));
          gw_ui_set_progressbar ("update", progress, text);
          gdk_threads_leave ();
          gw_dictlist_preform_postprocessing_by_name (di->name, &error);
          progress += increment;
        }
        else if (di->id == GW_DICT_NAMES && error == NULL)
        {
          gdk_threads_enter ();
          strcpy(text, gettext("Resplitting Names dictionary..."));
          gw_ui_set_progressbar ("update", progress, text);
          gw_dictlist_preform_postprocessing_by_name (di->name, &error);
          gdk_threads_leave ();
          progress += increment;
        }
      }

      gdk_threads_enter ();
      if (error == NULL)
        di->status = GW_DICT_STATUS_UPDATED;
      else
        di->status = GW_DICT_STATUS_ERRORED;
      gdk_threads_leave ();

      updatelist = updatelist->next;
    }
    g_list_free (updatelist);

    progress = 1.0;
    gdk_threads_enter ();
    gw_ui_set_progressbar ("update", progress, gettext("Finishing..."));
    gdk_threads_leave ();

    #ifdef G_OS_UNIX
    sleep (0.5);
    #endif

    gdk_threads_enter();
    if(error != NULL)
    {
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_ERRORED, GW_DICT_STATUS_INSTALLED);
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_UPDATING, GW_DICT_STATUS_INSTALLED);
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_UPDATED, GW_DICT_STATUS_INSTALLED);
      gw_ui_set_install_line_status ("update",  "error", error->message);
      g_error_free(error);
      error = NULL;
    }
    else if (gw_ui_get_install_line_status ("update") == GW_DICT_STATUS_CANCELING)
    {
      strcpy(text, gettext("Update was cancelled"));
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_UPDATING, GW_DICT_STATUS_INSTALLED);
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_UPDATED, GW_DICT_STATUS_INSTALLED);
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_CANCELING, GW_DICT_STATUS_INSTALLED);
      gw_ui_set_install_line_status ("update",  "install", text);
    }
    else
    {
      strcpy(text, gettext("Dictionary update finished"));
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_UPDATING, GW_DICT_STATUS_INSTALLED);
      gw_dictlist_normalize_all_status_from_to (GW_DICT_STATUS_UPDATED, GW_DICT_STATUS_INSTALLED);
      gw_ui_set_install_line_status ("update", "remove", text);
    }

    gw_ui_update_settings_interface ();
    gdk_threads_leave ();
}


static void *install_thread (gpointer dictionary)
{
    GQuark quark;
    quark = g_quark_from_string (GW_GENERIC_ERROR);
    GError *error = NULL;

    GwDictInfo *di = (GwDictInfo*) dictionary;
    char *name;
    name = g_utf8_strdown(di->name, -1);

    gboolean ret = TRUE;
    char *path = di->path;
    char *sync_path = di->sync_path;
    char *gz_path = di->gz_path;

    if (di->status != GW_DICT_STATUS_NOT_INSTALLED) return;

    //If everything succeeded, update the interface
    di->status = GW_DICT_STATUS_INSTALLING;
    gdk_threads_enter ();
    gw_ui_set_install_line_status(name, "cancel", NULL);
    gw_ui_update_settings_interface();
    gdk_threads_leave ();
    
    char fallback_uri[100];
    gw_util_strncpy_fallback_from_key (fallback_uri, di->gckey, 100);

    char uri[100];
    gw_pref_get_string (uri, di->gckey, fallback_uri, 100);
    printf("%s\n", uri);

    char progressbar_id[100];
    strcpy(progressbar_id, name);
    strcat(progressbar_id, "_install_progressbar");

    //Make sure the download folder exits
    char download_path[FILENAME_MAX] = "";
    gw_util_get_waei_directory(download_path);
    strcat(download_path, G_DIR_SEPARATOR_S);
    strcat(download_path, "download");
    if (ret)
      ret = ((g_mkdir_with_parents(download_path, 0755)) == 0);
    //Copy the file if it is a local file
    if (ret && g_file_test (uri, G_FILE_TEST_IS_REGULAR))
      ret = gw_io_copy_dictionary_file (uri, gz_path);
    //Otherwise attempt to download it
    else if (ret)
    {
      ret = gw_io_download_dictionary_file (uri, gz_path,
                                   gw_ui_update_progressbar, progressbar_id);
      if (ret == FALSE)
      {
        const char *message = gettext("Connection failure\n");
        error = g_error_new_literal (quark, GW_FILE_ERROR, message);
      }
    }

    if (strstr(gz_path, ".gz") && ret && error == NULL)
    {
    printf("gunzipping...\n");
      gdk_threads_enter();
      gw_ui_set_install_line_status(name, "finishing", gettext("Decompressing..."));
      gdk_threads_leave();
      ret = gw_io_gunzip_dictionary_file(gz_path, &error);
    }
    else if (strstr(gz_path, ".zip") && ret && error == NULL)
    {
    printf("unzipping...\n");
      ret = gw_io_unzip_dictionary_file(gz_path, &error);
    }
   
    if (strstr(sync_path, "UTF") == NULL && ret && error == NULL)
    {
      gdk_threads_enter();
      gw_ui_set_install_line_status(name, "finishing", gettext("Converting encoding..."));
      gdk_threads_leave();
      ret = gw_io_copy_with_encoding(sync_path, path, "EUC-JP","UTF-8", &error);
    }
    else
    {
      gw_io_copy_dictionary_file(sync_path, path, &error);
    }

    //Special dictionary post processing
    if (ret && error == NULL)
    {
      gdk_threads_enter();
      gw_ui_set_install_line_status(name, "finishing", gettext("Postprocessing..."));
      gdk_threads_leave();
      gw_dictlist_preform_postprocessing_by_name(di->name, &error);
    }
     
    //Was canceled
    if (error != NULL && gw_ui_get_install_line_status(name) == GW_DICT_STATUS_CANCELING)
    {
      gdk_threads_enter();
      di->status = GW_DICT_STATUS_NOT_INSTALLED;
      gw_ui_set_install_line_status(name, "install", NULL);
      gdk_threads_leave();
      g_error_free(error);
      error = NULL;
    }
    //Errored
    else if (error != NULL)
    {
      gdk_threads_enter();
      di->status = GW_DICT_STATUS_NOT_INSTALLED;
      gw_ui_set_install_line_status(name, "error", error->message);
      gdk_threads_leave();
      g_error_free(error);
      error = NULL;
    }
    //Install was successful
    else
    {
      gdk_threads_enter();
      di->status = GW_DICT_STATUS_INSTALLED;
      di->total_lines =  gw_io_get_total_lines_for_path (di->path);
      gw_ui_set_install_line_status(name, "remove", NULL);
      gdk_threads_leave();
    }

    gdk_threads_enter();
    gw_ui_update_settings_interface();
    rebuild_combobox_dictionary_list();
    gdk_threads_leave();
}



G_MODULE_EXPORT void do_hiragana_katakana_conv_toggle (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gw_pref_get_boolean (GCKEY_GW_HIRA_KATA, TRUE);
    gw_pref_set_boolean (GCKEY_GW_HIRA_KATA, !state);
}


G_MODULE_EXPORT void do_katakana_hiragana_conv_toggle (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gw_pref_get_boolean (GCKEY_GW_KATA_HIRA, TRUE);
    gw_pref_set_boolean (GCKEY_GW_KATA_HIRA, !state);
}


G_MODULE_EXPORT void do_spellcheck_toggle (GtkWidget *widget, gpointer data)
{
    gboolean state;
    state = gw_pref_get_boolean (GCKEY_GW_SPELLCHECK, TRUE);
    gw_pref_set_boolean (GCKEY_GW_SPELLCHECK, !state);
}


G_MODULE_EXPORT void do_romaji_kana_conv_change (GtkWidget *widget, gpointer data)
{
    int active;
    active = gtk_combo_box_get_active(GTK_COMBO_BOX (widget));
    gw_pref_set_int (GCKEY_GW_ROMAN_KANA, active);
}


G_MODULE_EXPORT void do_set_color_to_swatch (GtkWidget *widget, gpointer data)
{
    GdkColor color;
    gtk_color_button_get_color(GTK_COLOR_BUTTON (widget), &color);

    guint red = color.red & 0xFF00;
    guint green = color.green & 0xFF00;
    guint blue = color.blue & 0xFF00;

    int hex_integer = (red << 8) | (green) | (blue >> 8);

    char hex_string[10];
    if (gw_util_itohexstr(hex_string, hex_integer) == TRUE)
      printf("hex string: %s\n", hex_string);

    char key[100];
    char *key_ptr;
    strcpy(key, GCPATH_GW);
    strcat(key, "/highlighting/");
    key_ptr = &key[strlen(key)];
    strcpy(key_ptr, gtk_widget_get_name(widget));

    gw_pref_set_string (key, hex_string);
}


G_MODULE_EXPORT void do_color_reset_for_swatches (GtkWidget *widget, gpointer data)
{
    char key[100];
    char *key_ptr;
    strcpy(key, GCPATH_GW);
    strcat(key, "/highlighting/");
    key_ptr = &key[strlen(key)];

    char fallback[100];
    int i;

    int total_key = 5;
    char key_id [][100] = {
                             "match_foreground",
                             "match_background",
                             "header_foreground",
                             "header_background",
                             "comment_foreground"
                          };

    for (i = 0; i < total_key; i++)
    {
      strcpy(key_ptr, key_id[i]);
      gw_util_strncpy_fallback_from_key (fallback, key, 100);
      const char *string = gw_pref_get_default_string (key, fallback);
      if (string != NULL)
        gw_pref_set_string (key, string);
    }

    gw_ui_reload_tagtable_tags();
}



G_MODULE_EXPORT void do_dictionary_remove (GtkWidget* widget, gpointer* dictionary)
{
    char name[100];
    name[0] = '\0';
    gw_parse_widget_name (name, widget, TRUE);
    gw_console_uninstall_dictionary_by_name (name);

    name[0] = '\0';
    gw_parse_widget_name (name, widget, FALSE);
    gw_ui_set_install_line_status (name, "install", NULL);
    rebuild_combobox_dictionary_list();
}


G_MODULE_EXPORT void do_cancel_dictionary_install (GtkWidget *widget, gpointer data)
{
    char name[100];

    name[0] = '\0';
    gw_parse_widget_name (name, widget, FALSE);
    gw_ui_set_install_line_status (name, "cancelling", NULL);
}


G_MODULE_EXPORT void do_dictionary_install (GtkWidget *widget, gpointer data)
{
    //Make sure the files are clean
    do_dictionary_remove(widget, data);

    //Create the name string
    char name[100];
    gw_parse_widget_name(name, widget, TRUE);

    GwDictInfo *dictionary;
    dictionary = gw_dictlist_get_dictionary_by_alias (name);

    //Create the thread
    if (g_thread_create(&install_thread, dictionary, FALSE, NULL) == NULL) {
      g_warning("couldn't create the thread");
      return;
    }
}


G_MODULE_EXPORT void do_toggle_advanced_show(GtkWidget *widget, gpointer data)
{
    //show the one advanced hbox if it should be
    gboolean expanded;
    expanded = gtk_expander_get_expanded(GTK_EXPANDER (widget));

    //Reset all advanced boxes to their default closed states
    static char *expander_name[] = {
      "english_expander",
      "kanji_expander",
      "names_expander",
      "radicals_expander",
      "examples_expander",
      "french_expander",
      "german_expander",
      "spanish_expander"
    };
    static char *hbox_name[] = {
      "english_advanced_hbox",
      "kanji_advanced_hbox",
      "names_advanced_hbox",
      "radicals_advanced_hbox",
      "examples_advanced_hbox",
      "french_advanced_hbox",
      "german_advanced_hbox",
      "spanish_advanced_hbox"
    };

    int i;
    GtkWidget *hbox, *expander;
    for (i = 0; i < 8; i++) {
      hbox = GTK_WIDGET (gtk_builder_get_object(builder, hbox_name[i]));
      gtk_widget_hide(hbox);
      expander = GTK_WIDGET (gtk_builder_get_object(builder, expander_name[i]));

      g_signal_handlers_block_by_func(widget, do_toggle_advanced_show, NULL);
      gtk_expander_set_expanded(GTK_EXPANDER (expander), FALSE);
      g_signal_handlers_unblock_by_func(widget, do_toggle_advanced_show, NULL);
    }

    if (!expanded) {
      char id[100];
      gw_parse_widget_name(id, widget, FALSE);
      strcat(id, "_advanced_hbox");

      GtkWidget *target_advanced_hbox;
      target_advanced_hbox = GTK_WIDGET (gtk_builder_get_object(builder, id));
      gtk_widget_show(target_advanced_hbox);
    }

    else {
      g_signal_handlers_block_by_func(widget, do_toggle_advanced_show, NULL);
      gtk_expander_set_expanded(GTK_EXPANDER (widget), TRUE);
      g_signal_handlers_unblock_by_func(widget, do_toggle_advanced_show, NULL);
    }
}


G_MODULE_EXPORT void do_source_entry_changed_action (GtkWidget *widget, gpointer data)
{
    //Prepare some variables
    char name[100];
    gw_parse_widget_name(name, widget, FALSE);

    char key[100];
    strcpy(key, GCPATH_GW);
    strcat(key, "/dictionary/");
    strcat(key, name);
    strcat(key, "_source");

    char value[FILENAME_MAX];
    strcpy(value, gtk_entry_get_text(GTK_ENTRY (widget)));

    gw_pref_set_string (key, value);
}


G_MODULE_EXPORT void do_dictionary_source_reset(GtkWidget *widget, gpointer data) {
    //Prepare some variables
    char name[100];
    gw_parse_widget_name (name, widget, FALSE);

    char key[100];
    strcpy (key, GCPATH_GW);
    strcat (key, "/dictionary/");
    strcat (key, name);
    strcat (key, "_source");

    const char *string = gw_pref_get_default_string (key, NULL);
    if (string != NULL)
      gw_pref_set_string (key, string);
}


G_MODULE_EXPORT void do_dictionary_source_browse(GtkWidget *widget, gpointer data)
{
    //Declarations
    GtkWidget *dialog, *window;
    window = GTK_WIDGET (gtk_builder_get_object(builder, "settings_window"));
    dialog = gtk_file_chooser_dialog_new (gettext("Dictionary File Select"),
                GTK_WINDOW (window),
                GTK_FILE_CHOOSER_ACTION_OPEN,
                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
                NULL);
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (dialog), "");

    //Run the open as dialog
    if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename;
        filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

        const char *widget_id = gtk_widget_get_name (widget);

        gw_ui_set_dictionary_source(widget_id, filename);

        g_free (filename);
        filename = NULL;
    }

    gtk_widget_destroy (dialog);
}


G_MODULE_EXPORT void do_update_installed_dictionaries(GtkWidget *widget, gpointer data)
{
    char name[100];
    gw_parse_widget_name(name, widget, TRUE);

    GwDictInfo *dictionary;
    dictionary = gw_dictlist_get_dictionary_by_alias (name);

    //Create the thread
    if (g_thread_create(&update_thread, NULL, FALSE, NULL) == NULL) {
      g_warning("couldn't create the thread");
      return;
    }
}


G_MODULE_EXPORT void do_cancel_update_installed_dictionaries(GtkWidget *widget, gpointer data)
{
    gw_ui_set_install_line_status("update", "cancelling", NULL);
}


G_MODULE_EXPORT void do_force_names_resplit(GtkWidget *widget, gpointer data)
{
    GError *error = NULL;

    GwDictInfo* di;
    di = gw_dictlist_get_dictionary_by_alias("Names");

    di->status = GW_DICT_STATUS_REBUILDING;
    gw_dictlist_preform_postprocessing_by_name("Names", &error);
    di->status = GW_DICT_STATUS_INSTALLED;

    if (error != NULL)
    {
      g_error_free (error);
    }
}


G_MODULE_EXPORT void do_force_mix_rebuild(GtkWidget *widget, gpointer data)
{
    GError *error = NULL;

    GwDictInfo* di;
    di = gw_dictlist_get_dictionary_by_alias("Mix");

    if (gw_dictlist_dictionary_get_status_by_id (GW_DICT_KANJI)    == GW_DICT_STATUS_INSTALLED &&
        gw_dictlist_dictionary_get_status_by_id (GW_DICT_RADICALS) == GW_DICT_STATUS_INSTALLED   )
    {
      di->status = GW_DICT_STATUS_REBUILDING;
      gw_dictlist_preform_postprocessing_by_name("Mix", &error);
      di->status = GW_DICT_STATUS_INSTALLED;
    }
    if (error != NULL)
    {
      g_error_free (error);
    }
}


G_MODULE_EXPORT void do_other_dictionaries_help (GtkWidget *widget, gpointer data)
{
    char *uri = "http://gwaei.sourceforge.net/dictionaries.html";

    GError *err = NULL;
    gtk_show_uri (NULL, uri, gtk_get_current_event_time (), &err);
    if (err != NULL)
      g_error_free(err);
}


G_MODULE_EXPORT void do_switch_displayed_dictionaries_action (GtkWidget *widget, gpointer data)
{
    int active;
    GtkWidget *main_dictionaries_table, *other_dictionaries_table;

    active = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
    main_dictionaries_table = GTK_WIDGET (gtk_builder_get_object (builder, "dictionaries_table"));
    other_dictionaries_table = GTK_WIDGET (gtk_builder_get_object (builder, "other_dictionaries_table"));

    gtk_widget_hide (other_dictionaries_table);
    gtk_widget_hide (main_dictionaries_table);

    if (active == 0)
       gtk_widget_show (main_dictionaries_table);
    else if (active == 1)
       gtk_widget_show (other_dictionaries_table);
}


G_MODULE_EXPORT void do_move_dictionary_up (GtkWidget *widget, gpointer data)
{
    printf("Moving up...\n");
    //Get element number
    int requested_move = GPOINTER_TO_INT(data);

    //Parse the string
    char order[5000];
    gw_pref_get_string (order, GCKEY_GW_LOAD_ORDER, GW_LOAD_ORDER_FALLBACK, 5000);
    char *long_name_list[50];
    char **condensed_name_list[50];
    long_name_list[0] = order;
    int i = 0;
    int j = 0;
    while ((long_name_list[i + 1] = g_utf8_strchr (long_name_list[i], -1, L',')) && i < 50)
    {
      i++;
      *long_name_list[i] = '\0';
      long_name_list[i]++;
    }
    long_name_list[i + 1] = NULL;

    i = 0;
    j = 0;
    GwDictInfo *di1, *di2;
    while (long_name_list[i] != NULL && long_name_list[j] != NULL)
    {
      di1 = gw_dictlist_get_dictionary_by_name (long_name_list[j]);
      di2 = gw_dictlist_get_dictionary_by_alias (long_name_list[j]);
      if (strcmp(di1->name, di2->name) == 0 && di2->status == GW_DICT_STATUS_INSTALLED)
      {
        condensed_name_list[i] = &long_name_list[j];
        i++; j++;
      }
      else
      {
        j++;
      }
        
    }
    condensed_name_list[i] = NULL;

    //Pull the switcheroo
    if (requested_move > 0)
    {
      char *temp;
      temp = *condensed_name_list[requested_move - 1];
      *condensed_name_list[requested_move - 1] = *condensed_name_list[requested_move];
      *condensed_name_list[requested_move] = temp;
    }

    i = 0;
    char output[5000];
    output[0] = '\0';
    while (long_name_list[i] != NULL)
    {
      strcat (output, long_name_list[i]);
      strcat (output, ",");
      i++;
    }
    output[strlen(output) - 1] = '\0';
    gw_pref_set_string (GCKEY_GW_LOAD_ORDER, output);
}


G_MODULE_EXPORT void do_move_dictionary_down (GtkWidget *widget, gpointer data)
{
    printf("Moving down...\n");
    //Get element number
    int requested_move = GPOINTER_TO_INT(data);

    //Parse the string
    char order[5000];
    gw_pref_get_string (order, GCKEY_GW_LOAD_ORDER, GW_LOAD_ORDER_FALLBACK, 5000);
    char *long_name_list[50];
    char **condensed_name_list[50];
    long_name_list[0] = order;
    int i = 0;
    int j = 0;
    while ((long_name_list[i + 1] = g_utf8_strchr (long_name_list[i], -1, L',')) && i < 50)
    {
      i++;
      *long_name_list[i] = '\0';
      long_name_list[i]++;
    }
    long_name_list[i + 1] = NULL;

    i = 0;
    j = 0;
    GwDictInfo *di1, *di2;
    while (long_name_list[i] != NULL && long_name_list[j] != NULL)
    {
      di1 = gw_dictlist_get_dictionary_by_name (long_name_list[j]);
      di2 = gw_dictlist_get_dictionary_by_alias (long_name_list[j]);
      if (strcmp(di1->name, di2->name) == 0 && di2->status == GW_DICT_STATUS_INSTALLED)
      {
        condensed_name_list[i] = &long_name_list[j];
        i++; j++;
      }
      else
      {
        j++;
      }
        
    }
    condensed_name_list[i] = NULL;

    //Pull the switcheroo
    if (requested_move < i)
    {
      char *temp;
      temp = *condensed_name_list[requested_move + 1];
      *condensed_name_list[requested_move + 1] = *condensed_name_list[requested_move];
      *condensed_name_list[requested_move] = temp;
    }

    i = 0;
    char output[5000];
    output[0] = '\0';
    while (long_name_list[i] != NULL)
    {
      strcat (output, long_name_list[i]);
      strcat (output, ",");
      i++;
    }
    output[strlen(output) - 1] = '\0';
    gw_pref_set_string (GCKEY_GW_LOAD_ORDER, output);
}
