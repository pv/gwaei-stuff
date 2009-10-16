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
//! @file src/include/gwaei/searchitem-object.h
//!
//! @brief To be written.
//!
//! To be written.
//!

//!
//! Historylist targets
//!

#include <gwaei/resultline-object.h>


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
    char raw_query[MAX_QUERY];              //!< Query of the search
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

    GwResultLine* resultline;               //!< Result line to store parsed result
    GwResultLine* backup_resultline;        //!< Result line kept for comparison purposes from previosu result line
    GwResultLine* swap_resultline;          //!< Swap space for swapping result line and backup_resultline


    void (*gw_searchitem_parse_result_string)(GwResultLine*, char*);
    void (*gw_searchitem_append_results_to_output)(struct GwSearchItem*, gboolean);
} GwSearchItem;


