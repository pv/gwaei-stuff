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
//! @file src/radicals-callbacks-gtk.c
//!
//! @brief To be written.
//!
//! To be written.
//!


#include <regex.h>
#include <string.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/definitions.h>
#include <gwaei/utilities.h>
#include <gwaei/dictionaries.h>
#include <gwaei/history.h>
#include <gwaei/engine.h>
#include <gwaei/interface.h>



G_MODULE_EXPORT void do_radical_clear (GtkWidget *widget, gpointer data)
{
  gw_ui_deselect_all_radicals ();
  gw_ui_set_strokes_checkbox_state (FALSE);

  //Make the search stop that automatically starts
  gw_ui_cancel_search_by_target(GWAEI_TARGET_RESULTS);
}


G_MODULE_EXPORT void do_radical_search (GtkWidget *widget, gpointer data)
{
    GwHistoryList* hl = historylist_get_list(GWAEI_HISTORYLIST_RESULTS);   

    int leftover = 250;

    char query[leftover];
    query[0] = '\0';

    gw_ui_strcpy_all_selected_radicals (query, &leftover);
    gw_ui_strcpy_prefered_stroke_count (query, &leftover);

    if (strlen(query) == 0) return;

    gw_ui_clear_search_entry ();
    gw_ui_search_entry_insert (query);
    gw_ui_text_select_all_by_target (GWAEI_TARGET_ENTRY);

    GwDictInfo *dictionary;
    dictionary = gw_dictlist_get_dictionary_by_alias ("Radicals");

    if (gw_ui_cancel_search_by_target(GWAEI_TARGET_RESULTS) == FALSE)
      return;

    if (hl->current != NULL && (hl->current)->total_results > 0) 
    {
      historylist_add_searchitem_to_history(GWAEI_HISTORYLIST_RESULTS, hl->current);
      hl->current = NULL;
      gw_ui_update_history_popups();
    }
    else if (hl->current != NULL)
    {
      gw_searchitem_free (hl->current);
      hl->current = NULL;
    }
 
    hl->current = gw_searchitem_new(query, dictionary, GWAEI_TARGET_RESULTS);

    //Start the search
    gw_search_get_results (hl->current);
}


G_MODULE_EXPORT void do_radical_kanji_stroke_checkbox_update (GtkWidget *widget, gpointer data)
{
    gw_ui_update_strokes_checkbox_state ();

    //Start the search
    do_radical_search (NULL, NULL);
}


