#ifndef GW_HISTORYLIST_OBJECT_INCLUDED
#define GW_HISTORYLIST_OBJECT_INCLUDED 
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
//! @file src/include/gwaei/historylist-object.h
//!
//! @brief To be written.
//!
//! To be written.
//!

//!
//! Historylist targets
//!

#include <gwaei/searchitem-object.h>

typedef enum {
  GW_HISTORYLIST_RESULTS,
  GW_HISTORYLIST_KANJI
} GwHistoryListTarget;


//!
//! @brief Primitive for storing search items in intelligent ways
//!
typedef struct GwHistoryList
{
    GList *back;           //!< A GList of past search items
    GList *forward;        //!< A GList where past search items get stacked when the user goes back.
    GwSearchItem *current; //!< The current search before it gets pushed only into a history list.
} GwHistoryList;


/*searchitem methods*/
GwSearchItem* gw_searchitem_new    (char*, GwDictInfo*, int);
void        gw_searchitem_remove (struct GwSearchItem*);

/*Historylist methods*/
GwHistoryList* gw_historylist_new_item(GwHistoryList*, char*, char*);
GwHistoryList* gw_historylist_add_item(GwHistoryList*, GwSearchItem*);
GwHistoryList* gw_historylist_unlink_item(GwHistoryList*);
GwHistoryList* gw_historylist_remove_last_item(GwHistoryList*);
void   gw_historylist_clear(GwHistoryList*, GList**);
void   gw_historylist_shift_item(GwHistoryList*, GList**);

/*Functions*/
gboolean gw_searchitem_do_pre_search_prep (GwSearchItem*);
GwHistoryList* gw_historylist_get_list(const int);
GwSearchItem* gw_historylist_get_current (const int);
GList* gw_historylist_get_combined_history_list (const int);
GList* gw_historylist_get_back_history (const int);
GList* gw_historylist_get_forward_history (const int);

#endif
