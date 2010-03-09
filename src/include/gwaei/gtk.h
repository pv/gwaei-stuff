#define GW_GTK_INCLUDED
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
//! @file src/include/gtk.h
//!
//! @brief Abstraction layer for gtk objects
//!
//! Used as a go between for functions interacting with GUI interface objects.
//! This is the gtk version.
//!


GtkBuilder *builder;
GtkWidget *search_entry, *kanji_tv;
GtkWidget* get_widget_from_target(const int);

struct GwUiDictInstallLine {
    GtkWidget *status_icon;
    GtkWidget *status_message;
    GtkWidget *status_progressbar;
    GtkWidget *status_hbox;
    GtkWidget *message_hbox;

    GtkWidget *action_button;
    GtkWidget *action_button_hbox;

    GtkWidget *source_uri_entry;
    GtkWidget *source_browse_button;
    GtkWidget *source_reset_button;
    GtkWidget *source_hbox;

    GtkWidget *advanced_expander;
    GtkWidget *advanced_hbox;

    GwDictInfo *di;
};
typedef struct GwUiDictInstallLine GwUiDictInstallLine;



