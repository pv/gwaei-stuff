/******************************************************************************

  FILE:
  src/engine.c

  DESCRIPTION:
  This file controls the backend of searches (usually initiated by the
  do_search command.) get_results is the gatekeeper to stream_results.
  get_results sets everything up that needs to be correct and double checked.
  stream_results is called on a timer by gmainloop until it finished.  It then
  cleans up after after the things set up in get_results.  If another search is
  started before the previous is finished, get_results puts out a stop request
  to stream_results and then waits for it to finish.

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/dictionaries.h>
#include <gwaei/history.h>
#include <gwaei/engine.h>
#include <gwaei/utilities.h>
#include <gwaei/formatting.h>

gboolean less_relevant_title_inserted = FALSE;
gboolean less_relevant_results_show = TRUE;


//Private convenience function
void append_result_to_output (SearchItem *item)
{
    if (gwaei_util_get_runmode() == GWAEI_CONSOLE_RUNMODE)
    {
      printf("%s", item->output);
    }
    else
    {
      int start, end;
      gwaei_ui_append_to_buffer (item->target, item->output,
                                 NULL, NULL, &start, &end  );
      gwaei_ui_add_results_tagging (start, end, item);
    }
}


//
//Private convenience functions
//

void append_less_relevant_header_to_output(SearchItem *item)
{
    if (gwaei_util_get_runmode() == GWAEI_CONSOLE_RUNMODE)
    {
      printf("\n[0;31m***[0m[1m%s[0;31m***************************[0m\n\n\n", gettext("Other Results"));
    }
    else
    {
      char *tag1 = "header";
      char *tag2 = "important";
      gwaei_ui_append_to_buffer(item->target, "\n\n", tag1, tag2, NULL, NULL);
      gwaei_ui_append_to_buffer(item->target, gettext("Other Results"),
                             tag1, tag2, NULL, NULL                       );
      gwaei_ui_append_to_buffer(item->target, "\n\n", tag1, tag2, NULL, NULL);
    }
}


void add_group_formatting (SearchItem* item)
{
    char *input = item->input;
    char *comparison_buffer = item->comparison_buffer;

    //Special case for the initial string
    char temp[MAX_LINE];
    if (strcmp(comparison_buffer, "INITIALSTRING") == 0) {
        if (strlen(input) > 1)input[strlen(input) - 1] = '\0';
        comparison_buffer[0] = '\0';
        char* position1 = strchr(input, '[');
        if (position1 != NULL)              
        {
        strncpy(comparison_buffer, input, (int)(position1 - input));
        comparison_buffer[(int)(position1 - input)] = '\0';
        }
        return;
    }

    //Code to remove duplicate kanji in the beginnig of a result /////
    input[strlen(input) - 1] = '\0';
    char* position1 = strchr(input, '[');
    if (position1 == 0)
    {
        strcpy(temp, "\n");
        strncat(temp, input, MAX_LINE - 1);
        strncpy(input, temp, MAX_LINE);
    }
    else if (strstr(input, comparison_buffer) != input || comparison_buffer[0] == '\0')
    { //Changed
      strncpy(comparison_buffer, input, (int)(position1 - input));
      comparison_buffer[(int)(position1 - input)] = '\0';
      strcpy(temp, "\n");
      strncat(temp, input, MAX_LINE - 1);
      strncpy(input, temp, MAX_LINE);
    }
    else
    { //Didn't change
       char* position2 = input;
       while (position2 < position1)
       {
         if (position1 - position2 > 3)
         {
           
           *position2 = "　"[0];
           position2++;
           *position2 = "　"[1];
           position2++;
           *position2 = "　"[2];
           position2++;
         }
         else
         {
           *position2 = ' ';
           position2++;
         }
       }
    }
}


void append_stored_result_to_output(SearchItem *item, GList **results)
{
    if (less_relevant_results_show || item->total_relevant_results == 0)
    {
      if (gwaei_util_get_runmode() == GWAEI_CONSOLE_RUNMODE)
      {
        strcpy(item->input, (char*)(*results)->data);
        add_group_formatting (item);
        printf("%s", item->input);
      }
      else if (item->status != CANCELING)
      {
          int start, end;
          strcpy(item->input, (char*)(*results)->data);
          add_group_formatting (item);
          gwaei_ui_append_to_buffer (item->target, item->input,
                                     NULL, NULL, &start, &end);
          gwaei_ui_add_results_tagging (start, end, item);
      }
    }

    free(((*results)->data));
    *results = g_list_delete_link(*results, *results);
}


//Quantifier to figure out how relevent a returned item is
int get_relevance (char* text, SearchItem *item) {
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


gboolean stream_results_thread (gpointer data)
{
    SearchItem *item = data;
    char *dictionary = item->dictionary->name;
    int dictionary_type = item->dictionary->type;
    int chunk = 0;

    //We loop, processing lines of the file until the max chunk size has been
    //reached or we reach the end of the file or a cancel request is recieved.
    while (chunk < MAX_CHUNK               &&
           item->status != GWAEI_SEARCH_CANCELING &&
           fgets(item->input, MAX_LINE, item->fd) != NULL)
    {
      chunk++;
      item->current_line++;


      //Commented input in the dictionary...we should skip over it
      if(item->input[0] == '#' || g_utf8_get_char(item->input) == L'？') 
      { } 


      //Search engine for the kanji sidebar 
      else if (item->target == GWAEI_TARGET_KANJI)
      {
        if (regexec(&(item->re_exist[0]), item->input, 1, NULL, 0) == 0)
        {
          strcpy_with_kanji_formatting(item->output, item->input);
          item->output[strlen(item->output) - 1] = '\0';
          append_result_to_output(item);
          chunk = 0;
        }
      }


      //Search engine for kanji and radicals
      else if (dictionary_type == KANJI || dictionary_type == RADICALS)
      {
        gboolean missing_an_atom = FALSE;
        //Search for existance of every atom in the query
        int i;
        for (i = 0; i < item->total_re && !missing_an_atom; i++)
          if (regexec(&(item->re_exist[i]), item->input, 1, NULL, 0) != 0)
            missing_an_atom = TRUE;

        if (!missing_an_atom)
        {
          int relevance = get_relevance(item->input, item);
          char *result = NULL;
          switch(relevance)
          {
            case HIGH_RELEVANCE:
                item->total_relevant_results++;
                item->total_results++;
                gwaei_ui_update_total_results_label(item);
                if (dictionary_type == KANJI)
                  strcpy_with_kanji_formatting(item->output, item->input);
                else
                  strcpy(item->output, item->input);
                append_result_to_output(item);
                break;
            case MEDIUM_RELEVANCE:
                item->total_irrelevant_results++;
                if (result = (char*)malloc(MAX_LINE))
                {
                  if (dictionary_type == KANJI)
                    strcpy_with_kanji_formatting(result, item->input);
                  else
                    strcpy_with_kanji_formatting(result, item->input);
                  item->results_medium =  g_list_append(item->results_medium, result);
                }
                break;
            default:
                item->total_irrelevant_results++;
                if (result = (char*)malloc(MAX_LINE))
                {
                  if (dictionary_type == KANJI)
                    strcpy_with_kanji_formatting(result, item->input);
                  else
                    strcpy(result, item->input);
                  item->results_low = g_list_append(item->results_low, result);
                }
                break;
          }
        }
      }

      //Search engine for other dictionaries
      else
      {
        //Search for existance of every atom in the query.
        int i;
        gboolean missing_an_atom = FALSE;
        for(i = 0; i < item->total_re && !missing_an_atom; i++)
          if (regexec(&(item->re_exist[i]), item->input, 1, NULL, 0) != 0)
            missing_an_atom = TRUE;

        //Results match, add to the text buffer
        if (!missing_an_atom)
        {
          int relevance = get_relevance(item->input, item);
          char *result = NULL;
          switch(relevance)
          {
            case HIGH_RELEVANCE:
                item->total_results++;
                item->total_relevant_results++;
                gwaei_ui_update_total_results_label(item);
                add_group_formatting (item);
                strcpy_with_general_formatting(item->input, item->output);
                append_result_to_output(item);
                break;
            case MEDIUM_RELEVANCE:
                if ( item->total_irrelevant_results < MAX_MEDIUM_IRRELIVENT_RESULTS &&
                     (result = (char*)malloc(MAX_LINE)) )
                {
                  item->total_irrelevant_results++;
                  strcpy_with_general_formatting(item->input, result);
                  item->results_medium =  g_list_append(item->results_medium, result);
                }
                break;
            default:
                if ( item->total_irrelevant_results < MAX_LOW_IRRELIVENT_RESULTS &&
                     (result = (char*)malloc(MAX_LINE)))
                {
                  item->total_irrelevant_results++;
                  strcpy_with_general_formatting(item->input, result);
                  item->results_low = g_list_append(item->results_low, result);
                }
                break;
          }
        }
      }
      continue;
    }

    //Update the progressbar
    if (item->target == GWAEI_TARGET_RESULTS)
      gwaei_ui_update_search_progressbar (item->current_line, item->dictionary->total_lines);

    //If the chunk reached the max chunk size, there is still file left to load
    if ( chunk == MAX_CHUNK ) {
      return TRUE;
    }

    //Insert the less relevant title header if needed
    if ( less_relevant_results_show    &&
         !less_relevant_title_inserted &&
         item->total_relevant_results > 0    &&
         (item->results_medium != NULL || item->results_low != NULL) )
    {
      append_less_relevant_header_to_output(item);
      less_relevant_title_inserted = TRUE;
    }

    //Append the medium relevent results
    if (item->results_medium != NULL) {
      for (chunk = 0; item->results_medium != NULL && chunk < MAX_CHUNK; chunk++) {
        item->total_results++;
        append_stored_result_to_output(item, &(item->results_medium));
      }
      gwaei_ui_update_total_results_label(item);
      return TRUE;
    }

    //Append the least relevent results
    if (item->results_low != NULL) {
      for (chunk = 0; item->results_low != NULL && chunk < MAX_CHUNK; chunk++) {
        item->total_results++;
        append_stored_result_to_output(item, &(item->results_low));
      }
      gwaei_ui_update_total_results_label(item);
      return TRUE;
    }

    //Finish up
    if (item->total_results == 0 &&
        item->target != GWAEI_TARGET_KANJI &&
        item->status == GWAEI_SEARCH_SEARCHING)
    {
      if (gwaei_util_get_runmode () == GWAEI_CONSOLE_RUNMODE)
      {
        printf("%s\n\n", gettext("No results found!"));
      }
      else
      {
        gwaei_ui_clear_buffer_by_target (GWAEI_TARGET_RESULTS);
        gwaei_ui_display_no_results_found_page();
      }
      item->results_found = FALSE;
    }
    
    return FALSE;
}


gboolean stream_results_cleanup (gpointer data)
{
    SearchItem *item = data;
    searchitem_do_post_search_clean (item);
    less_relevant_title_inserted = FALSE;
    if (gwaei_util_get_runmode () == GWAEI_CONSOLE_RUNMODE)
    {
    }
    else
    {
      gwaei_ui_finalize_total_results_label (item);
      if (item->target == GWAEI_TARGET_RESULTS)
        gwaei_ui_update_search_progressbar (0, 0);
    }
     
    //Correct for background toggling of the boolean
    char *key = GCKEY_GWAEI_LESS_RELEVANT_SHOW; 
    less_relevant_results_show = gwaei_pref_get_boolean (key, TRUE);
}



//Intermediates searches
void gwaei_search_get_results (SearchItem *item)
{
    if (regexec(&re_kanji,   item->dictionary->name, 1, NULL, 0) == 0 ||
        regexec(&re_radical, item->dictionary->name, 1, NULL, 0) == 0 ||
        regexec(&re_mix,     item->dictionary->name, 1, NULL, 0) == 0   )
      less_relevant_results_show = TRUE;

    if (gwaei_util_get_runmode () != GWAEI_CONSOLE_RUNMODE)
      gwaei_ui_clear_buffer_by_target (item->target);

    if (searchitem_is_prepared (item) == FALSE)
      if (searchitem_do_pre_search_prep (item) == FALSE)
      {
        searchitem_free(item);
        return;
      }

    if (item->target == GWAEI_TARGET_RESULTS)
      gwaei_close_kanji_results();


    if (gwaei_util_get_runmode () == GWAEI_CONSOLE_RUNMODE)
    {
      while (stream_results_thread(item))
        ;
      stream_results_cleanup(item);
    }
    else
    {
      gwaei_ui_reinitialize_results_label (item);
      g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, 1,
                          (GSourceFunc)stream_results_thread, item,
                          (GDestroyNotify)stream_results_cleanup     );
    }
}


