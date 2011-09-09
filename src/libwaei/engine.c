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
//! @file engine.c
//!

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <glib.h>

#include <libwaei/libwaei.h>

#include <libwaei/engine-data.h>


void lw_engine_append_result (LwEngine* engine, LwSearchItem* item)
{
    switch (item->dictionary->type)
    {
      case LW_DICTTYPE_EDICT:
        (engine->append_edict_result_cb) (item);
        break;
      case LW_DICTTYPE_KANJI:
        (engine->append_kanjidict_result_cb) (item);
        break;
      case LW_DICTTYPE_EXAMPLES:
        (engine->append_examplesdict_result_cb) (item);
        break;
      case LW_DICTTYPE_UNKNOWN:
        (engine->append_unknowndict_result_cb) (item);
        break;
      default:
        g_assert_not_reached ();
        break;
    }
}


void lw_engine_append_less_relevant_header (LwEngine *engine, LwSearchItem *item)
{
  (engine->append_less_relevant_header_cb) (item);
}


void lw_engine_append_more_relevant_header (LwEngine *engine, LwSearchItem *item)
{
  (engine->append_more_relevant_header_cb) (item);
}


//!
//! @brief Sets up and allocateds memory for the specific LwEngine search
//! @param engine The LwEngine to use
//! @param item The LwSearchItem to prepare
//! @param exact Whether the search should display less relevant results
//!
gpointer lw_engine_prepare_search (LwEngine *engine, LwSearchItem *item, gboolean exact)
{
    LwEngineData *data;

    if (lw_searchitem_prepare_search (item) == FALSE)
      return NULL;

    (engine->prepare_search_cb) (item);

    data = lw_enginedata_new (engine, item, exact);

    return data;
}


//!
//! @brief Frees memory allocated for the specific LwEngine search
//! @param data  A pointer to a LwEngineData object
//!
void lw_engine_cleanup_search (gpointer data)
{
  LwEngineData *enginedata;
  LwEngine *engine;
  LwSearchItem *item;

  enginedata = LW_ENGINEDATA (data);
  engine = enginedata->engine;
  item = enginedata->item;

  (engine->cleanup_search_cb) (item);
  lw_searchitem_cleanup_search (item);

  lw_enginedata_free (data);
}


//!
//! @brief Gets a stored result in a search item and posts it to the output.
//!
//! THIS IS A PRIVATE FUNCTION. The memory is allocated and tthis function makes
//! sure to cleanly free it and then post it to the approprate output, be it the
//! terminal or a text buffer widget.
//!
//! @param item a LwSearchItem
//! @param results the result stored in a GList to free
//!
static void _append_stored_result_to_output (LwEngine *engine, LwSearchItem *item, GList **results)
{
    //Swap the lines
    item->swap_resultline = item->backup_resultline;
    item->backup_resultline = item->resultline;
    item->resultline = item->swap_resultline;
    item->swap_resultline = NULL;

    //Replace the current result line with the stored one
    if (item->resultline != NULL)
      lw_resultline_free (item->resultline);
    item->resultline = (LwResultLine*)(*results)->data;
    *results = g_list_delete_link(*results, *results);
 
    //Append to the buffer 
    if (item->status == LW_SEARCHSTATUS_SEARCHING)
    {
      lw_engine_append_result (engine, item);
    }
}


//!
//! @brief Find the relevance of a returned result
//!
//! THIS IS A PRIVATE FUNCTION. Function uses the stored relevance regrex
//! expressions in the LwSearchItem to get the relevance of a returned result.  It
//! then returns the answer to the caller in the form of an int.
//!
//! @param text a string to check the relevance of
//! @param item a search item to grab the regrexes from
//! @return Returns one of the integers: LOW_RELEVANCE, MEDIUM_RELEVANCE, or HIGH_RELEVANCE.
//!
static int _get_relevance (LwSearchItem *item) {
    if (lw_searchitem_run_comparison (item, LW_RELEVANCE_HIGH))
      return LW_RELEVANCE_HIGH;
    else if (lw_searchitem_run_comparison (item, LW_RELEVANCE_MEDIUM))
      return LW_RELEVANCE_MEDIUM;
    else
      return LW_RELEVANCE_LOW;
}


//!
//! @brief Preforms the brute work of the search
//!
//! THIS IS A PRIVATE FUNCTION. This function returns true until it finishes
//! searching the whole file.  It works in specified chunks before going back to
//! the thread to help improve speed.  
//!
//! @param data A LwSearchItem to search with
//! @return Returns true when the search isn't finished yet.
//!
static gpointer _stream_results_thread (gpointer data)
{
    //Declarations
    LwEngineData *enginedata;
    LwSearchItem *item;
    LwEngine *engine;
    gboolean show_only_exact_matches;

    //Initializations
    enginedata = LW_ENGINEDATA (data);
    engine = LW_ENGINE (enginedata->engine);
    item = LW_SEARCHITEM (enginedata->item);
    show_only_exact_matches = enginedata->exact;

    if (item == NULL || item->fd == NULL) return NULL;
    char *line_pointer = NULL;

    lw_searchitem_lock_mutex (item);

    //We loop, processing lines of the file until the max chunk size has been
    //reached or we reach the end of the file or a cancel request is recieved.
    while ((line_pointer = fgets(item->resultline->string, LW_IO_MAX_FGETS_LINE, item->fd)) != NULL &&
           item->status != LW_SEARCHSTATUS_CANCELING)
    {
      //Give a chance for something else to run
      lw_searchitem_unlock_mutex (item);
      lw_searchitem_lock_mutex (item);

      item->current_line++;

      //Commented input in the dictionary...we should skip over it
      if(item->resultline->string[0] == '#' || g_utf8_get_char(item->resultline->string) == L'？') 
      {
        continue;
      }
      else if (item->resultline->string[0] == 'A' && item->resultline->string[1] == ':' &&
               fgets(item->scratch_buffer, LW_IO_MAX_FGETS_LINE, item->fd) != NULL             )
      {
        char *eraser = NULL;
        if ((eraser = g_utf8_strchr (item->resultline->string, -1, L'\n')) != NULL) { *eraser = '\0'; }
        if ((eraser = g_utf8_strchr (item->scratch_buffer, -1, L'\n')) != NULL) { *eraser = '\0'; }
        if ((eraser = g_utf8_strrchr (item->resultline->string, -1, L'#')) != NULL) { *eraser = '\0'; }
        strcat(item->resultline->string, ":");
        strcat(item->resultline->string, item->scratch_buffer);
      }
      lw_searchitem_parse_result_string (item);


      //Results match, add to the text buffer
      if (lw_searchitem_run_comparison (item, LW_RELEVANCE_LOW))
      {
        int relevance = _get_relevance (item);
        switch(relevance)
        {
          case LW_RELEVANCE_HIGH:
              
              if (item->total_relevant_results < LW_MAX_HIGH_RELEVENT_RESULTS)
              {
                item->total_results++;
                item->total_relevant_results++;
                if (item->target != LW_OUTPUTTARGET_KANJI)
                  lw_engine_append_more_relevant_header (engine, item);
                lw_engine_append_result (engine, item);

                //Swap the result lines
                item->swap_resultline = item->backup_resultline;
                item->backup_resultline = item->resultline;
                item->resultline = item->swap_resultline;
                item->swap_resultline = NULL;
              }
              break;
          case LW_RELEVANCE_MEDIUM:
              if (item->total_irrelevant_results < LW_MAX_MEDIUM_IRRELEVENT_RESULTS &&
                  !show_only_exact_matches && 
                   (item->swap_resultline = lw_resultline_new ()) != NULL && item->target != LW_OUTPUTTARGET_KANJI)
              {
                //Store the result line and create an empty one in its place
                item->total_irrelevant_results++;
                item->results_medium =  g_list_append (item->results_medium, item->resultline);
                item->resultline = item->swap_resultline;
                item->swap_resultline = NULL;
              }
              break;
          default:
              if (item->total_irrelevant_results < LW_MAX_LOW_IRRELEVENT_RESULTS &&
                    !show_only_exact_matches && 
                   (item->swap_resultline = lw_resultline_new ()) != NULL && item->target != LW_OUTPUTTARGET_KANJI)
              {
                //Store the result line and create an empty one in its place
                item->total_irrelevant_results++;
                item->results_low = g_list_append (item->results_low, item->resultline);
                item->resultline = item->swap_resultline;
                item->swap_resultline = NULL;
              }
              break;
        }
      }
    }

    //Make sure the more relevant header banner is visible
    if (item->status != LW_SEARCHSTATUS_CANCELING)
    {
      if (item->target != LW_OUTPUTTARGET_KANJI && item->total_results > 0)
        lw_engine_append_more_relevant_header (engine, item);

      if (item->results_medium != NULL || item->results_low != NULL)
        lw_engine_append_less_relevant_header (engine, item);
    }

    //Append the medium relevent results
    while (item->results_medium != NULL)
    {
      item->total_results++;
      _append_stored_result_to_output (engine, item, &(item->results_medium));

      //Give a chance for something else to run
      lw_searchitem_unlock_mutex (item);
      lw_searchitem_lock_mutex (item);
    }

    //Append the least relevent results
    while (item->results_low != NULL)
    {
      item->total_results++;
      _append_stored_result_to_output (engine, item, &(item->results_low));

      //Give a chance for something else to run
      lw_searchitem_unlock_mutex (item);
      lw_searchitem_lock_mutex (item);
    }

    //Cleanup
    lw_engine_cleanup_search (data);

    lw_searchitem_unlock_mutex (item);

    return NULL;
}


//!
//! @brief Start a dictionary search
//! @param engine The LwEngine object to use to output the results
//! @param item a LwSearchItem argument to calculate results
//! @param create_thread Whether the search should run in a new thread.
//! @param exact Whether to show only exact matches for this search
//!
void lw_engine_get_results (LwEngine *engine, LwSearchItem *item, gboolean create_thread, gboolean exact)
{
    gpointer data;

    data = lw_engine_prepare_search (engine, item, exact);

    if (data != NULL)
    {
      if (create_thread)
      {
        item->thread = g_thread_create ((GThreadFunc) _stream_results_thread, (gpointer) data, TRUE, NULL);
        if (item->thread == NULL)
        {
          fprintf(stderr, "Couldn't create the thread");
        }
      }
      else
      {
        item->thread = NULL;
        _stream_results_thread ((gpointer) data);
      }
    }
}


//!
//! @brief Creates a new LwEngine object
//! @param append_edict_result_cb A callback function to output edict results
//! @param append_kanjidict_result_cb A callback function to output kanjidict results
//! @param append_examplesdict_result_cb A callback function to output examplesdict results
//! @param append_unknowndict_result_cb A callback function to output unknown dictionary results
//! @param append_less_relevant_header_cb A callback function to output the less relevant results header
//! @param append_more_relevant_header_cb A callback function to output he more relevant results header
//! @param prepare_search_cb A callback function to prepare data in a LwSearchItem before a search
//! @param cleanup_search_cb A callback function to cleanup data in a LwSearchItem after a search
//! @return An allocated LwEngine that will be needed to be freed by lw_engine_free.
//!
LwEngine* lw_engine_new (
    void (*append_edict_result_cb)(LwSearchItem*),
    void (*append_kanjidict_result_cb)(LwSearchItem*),
    void (*append_examplesdict_result_cb)(LwSearchItem*),
    void (*append_unknowndict_result_cb)(LwSearchItem*),
    void (*append_less_relevant_header_cb)(LwSearchItem*),
    void (*append_more_relevant_header_cb)(LwSearchItem*),
    void (*prepare_search_cb)(LwSearchItem*),
    void (*cleanup_search_cb)(LwSearchItem*)
)
{
    lw_regex_initialize ();

    LwEngine *temp;

    temp = (LwEngine*) malloc(sizeof(LwEngine));

    if (temp != NULL)
    {
      temp->append_edict_result_cb = append_edict_result_cb;
      temp->append_kanjidict_result_cb = append_kanjidict_result_cb;
      temp->append_examplesdict_result_cb =  append_examplesdict_result_cb;
      temp->append_unknowndict_result_cb = append_unknowndict_result_cb;

      temp->append_less_relevant_header_cb = append_less_relevant_header_cb;
      temp->append_more_relevant_header_cb = append_more_relevant_header_cb;
      temp->prepare_search_cb = prepare_search_cb;
      temp->cleanup_search_cb = cleanup_search_cb;
    }

    return temp;
}


//!
//! @brief Frees a LwEngine object
//! @param engine the LwEngine to free the memory of
//!
void lw_engine_free (LwEngine *engine)
{
    free (engine);
    lw_regex_free ();
}


