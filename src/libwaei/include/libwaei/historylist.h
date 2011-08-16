#ifndef LW_HISTORYLIST_INCLUDED
#define LW_HISTORYLIST_INCLUDED 
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
//! @file src/include/libwaei/historylist.h
//!
//! @brief To be written.
//!
//! To be written.
//!

//!
//! Historylist targets
//!

#define LW_HISTORYLIST(object) (LwHistoryList*) object
#include <libwaei/searchitem.h>


//!
//! @brief Primitive for storing search items in intelligent ways
//!
struct _LwHistoryList {
    GList *back;           //!< A GList of past search items
    GList *forward;        //!< A GList where past search items get stacked when the user goes back.
    int max;
};
typedef struct _LwHistoryList LwHistoryList;

LwHistoryList* lw_historylist_new (const int);
void lw_historylist_free (LwHistoryList*);

//Methods
GList* lw_historylist_get_back_list (LwHistoryList*);
GList* lw_historylist_get_forward_list (LwHistoryList*);
GList* lw_historylist_get_combined_list (LwHistoryList*);
void lw_historylist_clear_forward_list (LwHistoryList*);
void lw_historylist_clear_back_list (LwHistoryList*);

void lw_historylist_add_searchitem (LwHistoryList*, LwSearchItem*);

gboolean lw_historylist_has_back (LwHistoryList*);
gboolean lw_historylist_has_forward (LwHistoryList*);
LwSearchItem* lw_historylist_go_back (LwHistoryList*, LwSearchItem*);
LwSearchItem* lw_historylist_go_forward (LwHistoryList*, LwSearchItem*);


#endif
