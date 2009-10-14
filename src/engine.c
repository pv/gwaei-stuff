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
//! @file src/engine.c
//!
//! @brief search logic
//!
//! This file controls the backend of searches (usually initiated by the
//! do_search command.) get_results is the gatekeeper to stream_results.
//! get_results sets everything up that needs to be correct and double checked.
//! stream_results is called on a timer by gmainloop until it finished.  It then
//! cleans up after after the things set up in get_results.  If another search is
//! started before the previous is finished, get_results puts out a stop request
//! to stream_results and then waits for it to finish.
//!

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>
#include <gwaei/engine.h>
#include <gwaei/utilities.h>
#include <gwaei/formatting.h>

static gboolean less_relevant_title_inserted = FALSE;

//!
//! @brief Sends a result directly out to the output.
//!
//! THIS IS A PRIVATE FUNCTION. Function takes the current working result of the
//! search thread and outputs it to the approprate output, be that a terminal or
//! a text widget.
//! are below it.
//!
//! @param item a GwSearchItem to get the result from
//!
static void append_result_to_output (GwSearchItem *item, GwResultLine *UNUSED)
{
    if (item->dictionary->type == GW_DICT_OTHER && gw_util_get_runmode() == GW_GTK_RUNMODE)
    {
      gboolean furigana_exists, kanji_exists;
      gboolean same_def_totals, same_first_def, same_furigana, same_kanji, skip;
      kanji_exists = (item->resultline->kanji_start != NULL && item->backup_resultline->kanji_start != NULL);
      furigana_exists = (item->resultline->furigana_start != NULL && item->backup_resultline->furigana_start != NULL);
      if (item->resultline->kanji_start == NULL || item->backup_resultline->kanji_start == NULL)
      {
        skip = TRUE;
      }
      else
      {
        same_def_totals = (item->resultline->def_total == item->backup_resultline->def_total);
        same_first_def = (strcmp(item->resultline->def_start[0], item->backup_resultline->def_start[0]) == 0);
        same_furigana = (!furigana_exists ||strcmp(item->resultline->furigana_start, item->backup_resultline->furigana_start) == 0);
        same_kanji = (!kanji_exists || strcmp(item->resultline->kanji_start, item->backup_resultline->kanji_start) == 0);
        skip = FALSE;
      }

      //Begin comparison if possible
      if (!skip && ((same_def_totals && same_first_def) || (same_kanji && same_furigana)))
        gw_ui_append_def_same_to_buffer (item, TRUE);
      else
        (*item->gw_searchitem_append_results_to_output)(item, (!skip && same_kanji));
    }
    else
      (*item->gw_searchitem_append_results_to_output)(item, TRUE);
}


//!
//! @brief Does the work of outputing the more relevant header.
//!
//! THIS IS A PRIVATE FUNCTION. The function gives a label for the user to see
//! and tells how many results are below it.
//!
//! @param item a GwSearchItem to get the result numbers from
//!
static void append_more_relevant_header_to_output(GwSearchItem *item)
{
    if (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
    {
    }
    else
    {
      char number[14];
      gw_util_itoa(item->total_relevant_results, number, 14);

      char text[100];
      strncpy(text, gettext("Main Results "), 100);
      strncat(text, number, 100 - strlen(text));

      gw_ui_set_header (item, text, "more_relevant_header_mark");
    }
}


//!
//! @brief Does the work of outputing the less relevant header
//!
//! THIS IS A PRIVATE FUNCTION. The function gives a label for the user to see
//! and tells how many results are below it.
//!
//! @param item a GwSearchItem to get the result numbers from
//!
static void append_less_relevant_header_to_output(GwSearchItem *item)
{
    if (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
    {
      printf("\n[0;31m***[0m[1m%s[0;31m***************************[0m\n\n\n", gettext("Other Results"));
    }
    else
    {
      char number[14];
      gw_util_itoa(item->total_irrelevant_results, number, 14);

      char text[100];
      strncpy(text, gettext("Other Results "), 100);
      strncat(text, number, 100 - strlen(text));

      char *tag1 = "header";
      char *tag2 = "important";

      gw_ui_append_to_buffer(item->target, "\n", tag1, tag2, NULL, NULL);
      gw_ui_set_header (item, text, "less_relevant_header_mark");
      gw_ui_append_to_buffer(item->target, "\n", tag1, tag2, NULL, NULL);
    }
}


//!
//! @brief Gets a stored result in a search item and posts it to the output.
//!
//! THIS IS A PRIVATE FUNCTION. The memory is allocated and tthis function makes
//! sure to cleanly free it and then post it to the approprate output, be it the
//! terminal or a text buffer widget.
//!
//! @param item a GwSearchItem
//! @param results the result stored in a GList to free
//!
static void append_stored_result_to_output (GwSearchItem *item, GList **results, GwResultLine *UNUSED)
{
    if (item->show_less_relevant_results || item->total_relevant_results == 0)
    {
      (*item->gw_searchitem_parse_result_string)(item->resultline, (char*)(*results)->data);
      append_result_to_output (item, item->resultline);
    }

    free(((*results)->data));
    *results = g_list_delete_link(*results, *results);
}


//!
//! @brief Find the relevance of a returned result
//!
//! THIS IS A PRIVATE FUNCTION. Function uses the stored relevance regrex
//! expressions in the GwSearchItem to get the relevance of a returned result.  It
//! then returns the answer to the caller in the form of an int.
//!
//! @param text a string to check the relevance of
//! @param item a search item to grab the regrexes from
//! @return Returns one of the integers: LOW_RELEVANCE, MEDIUM_RELEVANCE, or HIGH_RELEVANCE.
//!
static int get_relevance (char* text, GwSearchItem *item) {
    int i;

    //The search results is freakin' gold :-D
    for (i = 0; i < item->total_re; i++)
      if (regexec(&(item->re_relevance_high[i]), text, 1, NULL, 0) == 0)
        return HIGH_RELEVANCE;

    //Blarg.  A search result that may come in useful. :-)
    for (i = 0; i < item->total_re; i++)
      if (regexec(&(item->re_relevance_medium[i]), text, 1, NULL, 0) == 0)
        return MEDIUM_RELEVANCE;

    //Search result wasn't relevent. :-(
    return LOW_RELEVANCE;
}


//!
//! @brief Preforms the brute work of the search
//!
//! THIS IS A PRIVATE FUNCTION. This function returns true until it finishes
//! searching the whole file.  It works in specified chunks before going back to
//! the thread to help improve speed.  
//!
//! @param data A GwSearchItem to search with
//! @return Returns true when the search isn't finished yet.
//!
static gboolean stream_results_thread (GwSearchItem *item)
{
    char *dictionary = item->dictionary->name;
    int dictionary_type = item->dictionary->type;
    int chunk = 0;

    //We loop, processing lines of the file until the max chunk size has been
    //reached or we reach the end of the file or a cancel request is recieved.
    while (chunk < MAX_CHUNK               &&
           item->status != GW_SEARCH_GW_DICT_STATUS_CANCELING &&
           fgets(item->scratch_buffer1, MAX_LINE, item->fd) != NULL)
    {
      chunk++;
      item->current_line++;

      if (item->scratch_buffer1[0] == 'A' && item->scratch_buffer1[1] == ':' &&
          fgets(item->scratch_buffer2, MAX_LINE, item->fd) != NULL             )
      {
        char *eraser = NULL;
        if (eraser = g_utf8_strchr (item->scratch_buffer1, -1, L'\n')) { *eraser = '\0'; }
        if (eraser = g_utf8_strchr (item->scratch_buffer2, -1, L'\n')) { *eraser = '\0'; }
        if (eraser = g_utf8_strrchr (item->scratch_buffer1, -1, L'#')) { *eraser = '\0'; }
        strcat(item->scratch_buffer1, ":");
        strcat(item->scratch_buffer1, item->scratch_buffer2);
      }
      //Commented input in the dictionary...we should skip over it
      if(item->scratch_buffer1[0] == '#' || g_utf8_get_char(item->scratch_buffer1) == L'？') 
      { } 
      //Search engine for the kanji sidebar 
      else if (item->target == GW_TARGET_KANJI)
      {
        if (regexec(&(item->re_exist[0]), item->scratch_buffer1, 1, NULL, 0) == 0)
        {
          (*item->gw_searchitem_parse_result_string)(item->resultline, item->scratch_buffer1);
          append_result_to_output (item, item->resultline);
          chunk = 0;
        }
      }

      //Search engine for other dictionaries
      else
      {
        //Search for existance of every atom in the query.
        int i;
        gboolean missing_an_atom = FALSE;
        for(i = 0; i < item->total_re && !missing_an_atom; i++)
          if (regexec(&(item->re_exist[i]), item->scratch_buffer1, 1, NULL, 0) != 0)
            missing_an_atom = TRUE;

        //Results match, add to the text buffer
        if (!missing_an_atom)
        {
          int relevance = get_relevance(item->scratch_buffer1, item);
          char *result = NULL;
          switch(relevance)
          {
            case HIGH_RELEVANCE:
                item->total_results++;
                item->total_relevant_results++;
                append_more_relevant_header_to_output(item);
                gw_ui_update_total_results_label(item);
                if (item->resultline != NULL)
                {
                  //Pull a switcheroo
                  item->swap_resultline = item->backup_resultline;
                  item->backup_resultline = item->resultline;
                  item->resultline = item->swap_resultline;
                  item->swap_resultline = NULL;
                }
                (*item->gw_searchitem_parse_result_string)(item->resultline, item->scratch_buffer1);
                append_result_to_output(item, item->resultline);
                break;
            case MEDIUM_RELEVANCE:
                if ((item->dictionary->type == GW_DICT_KANJI || item->total_irrelevant_results < MAX_MEDIUM_IRRELIVENT_RESULTS) &&
                     (result = (char*)malloc(strlen(item->scratch_buffer1) + 2)))
                {
                  item->total_irrelevant_results++;
                  strcpy(result, item->scratch_buffer1);
                  item->results_medium =  g_list_append(item->results_medium, result);
                }
                break;
            default:
                if ((item->dictionary->type == GW_DICT_KANJI || item->total_irrelevant_results < MAX_LOW_IRRELIVENT_RESULTS) &&
                     (result = (char*)malloc(strlen(item->scratch_buffer1) + 2)))
                {
                  item->total_irrelevant_results++;
                  strcpy(result, item->scratch_buffer1);
                  item->results_low = g_list_append(item->results_low, result);
                }
                break;
          }
        }
      }
      continue;
    }

    //Update the progressbar
    if (item->target == GW_TARGET_RESULTS)
      gw_ui_update_search_progressbar (item->current_line, item->dictionary->total_lines);

    //If the chunk reached the max chunk size, there is still file left to load
    if ( chunk == MAX_CHUNK ) {
      return TRUE;
    }

    //Insert the less relevant title header if needed
    if ( item->show_less_relevant_results    &&
         !less_relevant_title_inserted &&
         (item->results_medium != NULL || item->results_low != NULL) )
    {
      append_less_relevant_header_to_output(item);
      less_relevant_title_inserted = TRUE;
    }

    //Append the medium relevent results
    if (item->results_medium != NULL) {
      for (chunk = 0; item->results_medium != NULL && chunk < MAX_CHUNK; chunk++) {
        item->total_results++;
        item->swap_resultline = item->backup_resultline;
        item->backup_resultline = item->resultline;
        item->resultline = item->swap_resultline;
        item->swap_resultline = NULL;
        append_stored_result_to_output(item, &(item->results_medium), item->resultline);
      }
      gw_ui_update_total_results_label(item);
      return TRUE;
    }

    //Append the least relevent results
    if (item->results_low != NULL) {
      for (chunk = 0; item->results_low != NULL && chunk < MAX_CHUNK; chunk++) {
        item->total_results++;
        item->swap_resultline = item->backup_resultline;
        item->backup_resultline = item->resultline;
        item->resultline = item->swap_resultline;
        item->swap_resultline = NULL;
        append_stored_result_to_output(item, &(item->results_low), item->resultline);
      }
      gw_ui_update_total_results_label(item);
      return TRUE;
    }

    //Finish up
    if (item->total_results == 0 &&
        item->target != GW_TARGET_KANJI &&
        item->status == GW_SEARCH_SEARCHING)
    {
      if (gw_util_get_runmode () == GW_CONSOLE_RUNMODE)
      {
        printf("%s\n\n", gettext("No results found!"));
      }
      else
      {
        gw_ui_clear_buffer_by_target (GW_TARGET_RESULTS);
        gw_ui_display_no_results_found_page();
      }
    }
    
    return FALSE;
}


//!
//! @brief Preforms necessary cleanups after the search thread finishes
//!
//! THIS IS A PRIVATE FUNCTION. The calls to this function are made by
//! gw_search_get_results.  Do not call this function directly.
//!
//! @see gw_search_get_results()
//! @param data A GwSearchItem to clean up the data of
//! @return currently unused
//!
static gboolean stream_results_cleanup (GwSearchItem *item)
{
    gw_searchitem_do_post_search_clean (item);
    less_relevant_title_inserted = FALSE;
    if (gw_util_get_runmode () == GW_CONSOLE_RUNMODE)
    {
    }
    else
    {
      gw_ui_remove_whitespace_from_buffer (GW_TARGET_RESULTS);
      gw_ui_finalize_total_results_label (item);
      if (item->target == GW_TARGET_RESULTS)
        gw_ui_update_search_progressbar (0, 0);
    }     
}


//!
//! @brief Start a dictionary search
//!
//! This is the entry point for starting a search.  It handles setting up the
//! query, checking things that need to be checked before the final go, and
//! initializing the search loop or thread.
//!
//! @param item a GwSearchItem argument.
//!
void gw_search_get_results (GwSearchItem *item)
{
    //Misc preparations
    if (item->target != GW_TARGET_CONSOLE &&
        (item->dictionary->type == GW_DICT_KANJI || item->dictionary->type == GW_DICT_RADICALS))
      item->show_less_relevant_results = TRUE;

    if (gw_util_get_runmode () != GW_CONSOLE_RUNMODE)
      gw_ui_initialize_buffer_by_target (item->target);

    if (gw_searchitem_do_pre_search_prep (item) == FALSE)
    {
      gw_searchitem_free(item);
      return;
    }

    if (item->target == GW_TARGET_RESULTS)
      gw_ui_close_kanji_results();


    //Start the search
    if (gw_util_get_runmode () == GW_CONSOLE_RUNMODE)
    {
      while (stream_results_thread(item))
        ;
      stream_results_cleanup(item);
    }
    else
    {
      gw_ui_reinitialize_results_label (item);
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 1,
                          (GSourceFunc)stream_results_thread, item,
                          (GDestroyNotify)stream_results_cleanup     );
    }
}

