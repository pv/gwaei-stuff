#ifndef GW_SEARCHITEM_OBJECT_INCLUDED
#define GW_SEARCHITEM_OBJECT_INCLUDED
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
//! @file src/include/gwaei/searchitem.h
//!
//! @brief To be written.
//!
//! To be written.
//!

//!
//! Historylist targets
//!

#include <stdio.h>

#include <gwaei/queryline.h>
#include <gwaei/resultline.h>
#include <gwaei/dictinfo.h>


#define GW_HISTORY_TIME_TO_RELEVANCE 20

//!
//! Search status types
//!
typedef enum
{
  GW_SEARCH_IDLE,
  GW_SEARCH_SEARCHING,
  GW_SEARCH_FINISHING,
  GW_SEARCH_CANCELING
} GwSearchState;

typedef enum
{
  GW_RESULT_HIGH_RELEVANCE,
  GW_RESULT_MEDIUM_RELEVANCE,
  GW_RESULT_LOW_RELEVANCE
} GwResultRelevance;

typedef enum
{
  GW_TARGET_RESULTS,
  GW_TARGET_KANJI,
  GW_TARGET_ENTRY,
  GW_TARGET_CONSOLE
} GwTargetOutput;


//!
//! @brief Primitive for storing search item information
//!
//Object
struct _GwSearchItem {
    GwQueryLine* queryline;                 //!< Result line to store parsed result
    GwDictInfo* dictionary;                 //!< Pointer to the dictionary used

    FILE* fd;                               //!< File descriptor for file search position
    GThread *thread;                        //!< Thread the search is processed in
    GMutex *mutex;                          //!< Mutext to help ensure threadsafe operation

    GwSearchState status;                   //!< Used to test if a search is in progress.
    char *scratch_buffer;                   //!< Scratch space
    GwTargetOutput target;                  //!< What gui element should be outputted to
    long current_line;                      //!< Current line in the dictionary file
    long progress_feedback_line;            //!< Recorderd previous line for determining when to update the progresse
    int history_relevance_idle_timer;       //!< Helps determine if something is added to the history or not
    gboolean show_only_exact_matches;

    int total_relevant_results;             //!< Total results guessed to be highly relevant to the query
    int total_irrelevant_results;           //!< Total results guessed to be vaguely relevant to the query
    int total_results;                      //!< Total results returned from the search

    GList *results_medium;                  //!< Buffer storing mediumly relevant result for later display
    GList *results_low;                     //!< Buffer storing lowly relevant result for later display

    GwResultLine* resultline;               //!< Result line to store parsed result
    GwResultLine* backup_resultline;        //!< Result line kept for comparison purposes from previosu result line
    GwResultLine* swap_resultline;          //!< Swap space for swapping result line and backup_resultline

    void (*gw_searchitem_parse_result_string)(GwResultLine*);                              //!< function pointer
    void (*gw_searchitem_ui_append_results_to_output)(struct _GwSearchItem*);              //!< function pointer
    void (*gw_searchitem_ui_append_less_relevant_header_to_output)(struct _GwSearchItem*); //!< function pointer
    void (*gw_searchitem_ui_append_more_relevant_header_to_output)(struct _GwSearchItem*); //!< function pointer
    void (*gw_searchitem_ui_pre_search_prep)(struct _GwSearchItem*);                       //!< function pointer
    void (*gw_searchitem_ui_after_search_cleanup)(struct _GwSearchItem*);                  //!< function pointer

    gpointer* target_tb;                 //!< Pointer to a buffer that stays constant unlike when the target attribute is used
    gpointer* target_tv;                 //!< Pointer to a buffer that stays constant unlike when the target attribute is used
};
typedef struct _GwSearchItem GwSearchItem;

//Methods
GwSearchItem* gw_searchitem_new (char*, GwDictInfo*, const int);

void gw_searchitem_do_post_search_clean (GwSearchItem*);
gboolean gw_searchitem_do_pre_search_prep (GwSearchItem*);

gboolean gw_searchitem_existance_generic_comparison (GwSearchItem*, const int);


void gw_searchitem_free (GwSearchItem*);


#endif