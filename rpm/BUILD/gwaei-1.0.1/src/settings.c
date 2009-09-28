/******************************************************************************

  FILE:
  src/settings.c

  DESCRIPTION:
  To be written.

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
#include <stdlib.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>
#include <curl/curl.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/dictionaries.h>
#include <gwaei/history.h>
#include <gwaei/settings.h>
#include <gwaei/interface.h>
#include <gwaei/main.h>
#include <gwaei/settings.h>


char* gwaei_parse_widget_name (char *output, GtkWidget *widget,
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


void gwaei_settings_initialize_enabled_features_list()
{
    //General search
    if (dictionarylist_get_total_with_status(INSTALLED) > 0)
      gwaei_ui_set_feature_line_status("general", "enabled");
    else
      gwaei_ui_set_feature_line_status("general", "disabled");

    //Combined dictionary
    if (dictionarylist_dictionary_get_status_by_id(MIX) == INSTALLED)
      gwaei_ui_set_feature_line_status("mix", "enabled");
    else
      gwaei_ui_set_feature_line_status("mix", "disabled");

    //Radical search tool
    if (dictionarylist_dictionary_get_status_by_id(RADICALS) == INSTALLED)
      gwaei_ui_set_feature_line_status("radical", "enabled");
    else
      gwaei_ui_set_feature_line_status("radical", "disabled");

    //Kanji lookup tool
    if (dictionarylist_dictionary_get_status_by_id(KANJI) == INSTALLED)
      gwaei_ui_set_feature_line_status("kanji", "enabled");
    else
      gwaei_ui_set_feature_line_status("kanji", "disabled");

/*
    GtkWidget *label;
    label = GTK_WIDGET (gtk_builder_get_object(builder, "update_install_label"));
    if (rsync_exists)
      gtk_label_set_text(GTK_LABEL (label), gettext("Requires rsync to be installed"));
      */
}



void gwaei_settings_initialize_installed_dictionary_list() 
{
    if (dictionarylist_dictionary_get_status_by_id(ENGLISH) == INSTALLED)
      gwaei_ui_set_install_line_status("english", "remove", NULL);
    else
      gwaei_ui_set_install_line_status("english", "install", NULL);

    if (dictionarylist_dictionary_get_status_by_id(KANJI) == INSTALLED)
      gwaei_ui_set_install_line_status("kanji", "remove", NULL);
    else
      gwaei_ui_set_install_line_status("kanji", "install", NULL);

    if (dictionarylist_dictionary_get_status_by_id(NAMES) == INSTALLED)
      gwaei_ui_set_install_line_status("names", "remove", NULL);
    else
      gwaei_ui_set_install_line_status("names", "install", NULL);

    if (dictionarylist_dictionary_get_status_by_id(RADICALS) == INSTALLED)
      gwaei_ui_set_install_line_status("radicals", "remove", NULL);
    else
      gwaei_ui_set_install_line_status("radicals", "install", NULL);
}


