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
//! @file src/settings.c
//!
//! @brief To be written
//!
//! To be written.
//!


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

/*
    GtkWidget *label;
    label = GTK_WIDGET (gtk_builder_get_object(builder, "update_install_label"));
    if (rsync_exists)
      gtk_label_set_text(GTK_LABEL (label), gettext("Requires rsync to be installed"));
      */
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
}


