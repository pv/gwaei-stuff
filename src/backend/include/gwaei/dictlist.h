#ifndef GW_DICTLIST_OBJECT_INCLUDED
#define GW_DICTLIST_OBJECT_INCLUDED
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
//! @file src/include/gwaei/dictlist.h
//!
//! @brief To be written.
//!
//! To be written.
//!

#include <gwaei/dictinfo.h>

#define GW_DICTLIST_MAX_DICTIONARIES 20

//!
//! @brief Primitive for storing lists of dictionaries
//!
struct _GwDictList
{
    GList *list;      //!< GList of the installed dictionaries
    GList *selected;  //!< Pointer to the currently selected dictionary in the GList
    GMutex *mutex;
};
typedef struct _GwDictList GwDictList;


GList* gw_dictlist_get_list (void);
int gw_dictlist_get_total (void);
GList* gw_dictlist_get_selected (void);


void gw_dictlist_initialize (void);
GwDictInfo* gw_dictlist_get_dictinfo_by_name (GwDictEngine, const char*);
GList* gw_dictlist_get_dict_by_load_position (int);
GList* gw_dictlist_set_selected_by_load_position (int);
gboolean gw_dictlist_check_if_loaded_by_name (char*);
void gw_dictlist_update_load_orders (void);

void gw_dictlist_preform_postprocessing_by_name (char*, GError**);
int gw_dictlist_get_total_with_status (GwDictStatus);
void gw_dictlist_load_dictionary_order_from_pref (void);
void gw_dictlist_save_dictionary_order_pref (void);

void gw_dictlist_free ();



#endif
