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
//! @file src/include/gwaei/history.h
//!
//! @brief To be written.
//!
//! To be written.
//!

//!
//! Historylist targets
//!
enum historylist_targets
{
  GW_HISTORYLIST_RESULTS,
  GW_HISTORYLIST_KANJI
};

//!
//! Search status types
//!
enum search_states
{
  GW_SEARCH_IDLE,
  GW_SEARCH_SEARCHING,
  GW_SEARCH_GW_DICT_STATUS_CANCELING
};


//!
//! @brief Primitive for storing search item information
//!
typedef struct GwSearchItem {
    char query[MAX_QUERY];                  //!< Query of the search
    GwDictInfo* dictionary;                 //!< Pointer to the dictionary used

    FILE* fd;                               //!< File descriptor for file search position
    int status;                             //!< Used to test if a search is in progress.
    char *scratch_buffer1;                  //!< Scratch space
    char *scratch_buffer2;                  //!< Scratch space
    char *comparison_buffer;                //!< Saves the previously loaded result for comparison
    int target;                             //!< What gui element should be outputted to
    long current_line;                      //!< Current line in teh dictionary file
    gboolean show_less_relevant_results;    //!< Saved search display format

    int total_relevant_results;             //!< Total results guessed to be highly relevant to the query
    int total_irrelevant_results;           //!< Total results guessed to be vaguely relevant to the query
    int total_results;                      //!< Total results returned from the search

    regex_t re_exist[MAX_QUERY];            //!< Parsed regex atoms for checking of a needle exists
    regex_t re_locate[MAX_QUERY];           //!< Parsed regex atoms for pulling the needle out of the haystack.

    regex_t re_relevance_medium[MAX_QUERY]; //!< Parsed regex atoms for testing medium relevance on a result
    regex_t re_relevance_high[MAX_QUERY];   //!< Parsed regex atoms for testing high relevance on a result
    GList *results_medium;                  //!< Buffer storing mediumly relevant result for later display
    GList *results_low;                     //!< Buffer storing lowly relevant result for later display
    int total_re;                           //!< The total regex atoms that were created.
} GwSearchItem;


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

