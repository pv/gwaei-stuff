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
//! @file src/searchitem-object.c
//!
//! @brief Search item and history management
//!
//! Functions and objects to create search items and manage them.
//!


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/utilities.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>
#include <gwaei/interface.h>


//!
//! @brief Creates a new GwSearchItem object. 
//!
//! Takes the query and parses it according to the dictionary and TARGET give
//! to it.  Searchitem also stores various variables such as the file
//! it uses and the tallied results.
//!
//! @param query The text to be search for
//! @param dictionary The GwDictInfo object to use
//! @param TARGET The widget to output the results to
//! @return Returns an allocated GwSearchItem object
//!
GwSearchItem* gw_searchitem_new (char* query, GwDictInfo* dictionary,
                                         const int TARGET)
{
  GwSearchItem *temp;

  //Allocate some memory
  if ((temp = malloc(sizeof(struct GwSearchItem))) == NULL) return NULL;

  temp->results_medium = NULL;
  temp->results_low = NULL;
  
  if (TARGET != GW_TARGET_RESULTS &&
      TARGET != GW_TARGET_KANJI   &&
      TARGET != GW_TARGET_CONSOLE       )
    return NULL;

  //Set the internal pointers to the correct global variables
  temp->fd     = NULL;
  temp->status = GW_SEARCH_IDLE;
  temp->scratch_buffer1 = NULL;
  temp->scratch_buffer2 = NULL;
  temp->comparison_buffer = NULL;
  temp->dictionary = dictionary;
  temp->target = TARGET;
  temp->total_relevant_results = 0;
  temp->total_irrelevant_results = 0;
  temp->total_results = 0;
  temp->current_line = 0;
  temp->resultline = NULL;
  temp->backup_resultline = NULL;
  temp->swap_resultline = NULL;
  temp->queryline = gw_queryline_new ();
  if (gw_queryline_parse_string (temp->queryline, query))
      printf("Success creating parsing queryline!\n");

  char *key = GCKEY_GW_LESS_RELEVANT_SHOW; 
  temp->show_less_relevant_results = gw_pref_get_boolean (key, TRUE);


  //Set function pointers
  switch (temp->dictionary->type)
  {
      case GW_DICT_OTHER:
        temp->gw_searchitem_parse_result_string = &gw_resultline_parse_normal_result_string;
        if  (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
          temp->gw_searchitem_append_results_to_output = &gw_console_append_normal_results;
        else
          temp->gw_searchitem_append_results_to_output = &gw_ui_append_normal_results_to_buffer;
        break;
      case GW_DICT_RADICALS:
        temp->gw_searchitem_parse_result_string = &gw_resultline_parse_radical_result_string;
        if  (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
          temp->gw_searchitem_append_results_to_output = &gw_console_append_radical_results;
        else
          temp->gw_searchitem_append_results_to_output = &gw_ui_append_radicals_results_to_buffer;
        break;
      case GW_DICT_KANJI:
        temp->gw_searchitem_parse_result_string = &gw_resultline_parse_kanji_result_string;
        if  (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
          temp->gw_searchitem_append_results_to_output = gw_console_append_kanji_results;
        else
          temp->gw_searchitem_append_results_to_output = &gw_ui_append_kanji_results_to_buffer;
        break;
      case GW_DICT_EXAMPLES:
        temp->gw_searchitem_parse_result_string = &gw_resultline_parse_examples_result_string;
        if  (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
          temp->gw_searchitem_append_results_to_output = &gw_console_append_examples_results;
        else
          temp->gw_searchitem_append_results_to_output = &gw_ui_append_examples_results_to_buffer;
        break;
      default:
        temp->gw_searchitem_parse_result_string = &gw_resultline_parse_unknown_result_string;
        if  (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
          temp->gw_searchitem_append_results_to_output = &gw_console_append_unknown_results;
        else
          temp->gw_searchitem_append_results_to_output = &gw_ui_append_unknown_results_to_buffer;
        break;
  }

  return temp;
}


//!
//! @brief Does variable preparation required before a search
//!
//! The input and output scratch buffers have their memory allocated
//! the current_line integer is reset to 0, the comparison buffer
//! reset to it's initial state, the search status set to
//! SEARCHING, and the file descriptior is opened.
//!
//! @param item The GwSearchItem to its variables prepared
//! @return Returns false on seachitem prep failure.
//!
gboolean gw_searchitem_do_pre_search_prep (GwSearchItem* item)
{
    if (item->scratch_buffer1 != NULL || (item->scratch_buffer1 = malloc (MAX_LINE)) == NULL)
    {
      return FALSE;
    }
    if (item->scratch_buffer2 != NULL || (item->scratch_buffer2 = malloc (MAX_LINE)) == NULL)
    {
      free (item->scratch_buffer1);
      item->scratch_buffer1 = NULL;
      return FALSE;
    }
    if (item->comparison_buffer != NULL || (item->comparison_buffer = malloc (MAX_LINE)) == NULL)
    {
      free (item->scratch_buffer1);
      item->scratch_buffer1 = NULL;
      free (item->scratch_buffer2);
      item->scratch_buffer2 = NULL;
      return FALSE;
    }
    if (item->resultline != NULL || (item->resultline = gw_resultline_new ()) == NULL)
    {
      free (item->comparison_buffer);
      item->comparison_buffer = NULL;
      free (item->scratch_buffer1);
      item->scratch_buffer1 = NULL;
      free (item->scratch_buffer2);
      item->scratch_buffer2 = NULL;
      return FALSE;
    }
    if (item->backup_resultline != NULL || (item->backup_resultline = gw_resultline_new ()) == NULL)
    {
      gw_resultline_free (item->resultline);
      item->resultline = NULL;
      free (item->comparison_buffer);
      item->comparison_buffer = NULL;
      free (item->scratch_buffer1);
      item->scratch_buffer1 = NULL;
      free (item->scratch_buffer2);
      item->scratch_buffer2 = NULL;
      return FALSE;
    }


    //Reset internal variables
    strcpy(item->comparison_buffer, "INITIALSTRING");
    item->current_line = 0;
    item->total_relevant_results = 0;
    item->total_irrelevant_results = 0;
    item->total_results = 0;

    if (item->fd == NULL)
      item->fd = fopen ((item->dictionary)->path, "r");
    item->status = GW_SEARCH_SEARCHING;
    return TRUE;
}


//!
//! @brief Cleanups after a search completes
//!
//! The file descriptior is closed, various variables are
//! reset, and the search status is set to IDLE.
//!
//! @param item The GwSearchItem to its state reset.
//!
void gw_searchitem_do_post_search_clean (GwSearchItem* item)
{
    if (item->fd != NULL)
    {
      fclose(item->fd);
      item->fd = NULL;
    }

    if (item->scratch_buffer1 != NULL)
    {
      free(item->scratch_buffer1);
      item->scratch_buffer1 = NULL;
    }
    if (item->scratch_buffer2 != NULL)
    {
      free(item->scratch_buffer2);
      item->scratch_buffer2 = NULL;
    }
    if (item->comparison_buffer != NULL)
    {
      free(item->comparison_buffer);
      item->comparison_buffer = NULL;
    }
    if (item->resultline != NULL)
    {
      gw_resultline_free (item->resultline);
      item->resultline = NULL;
    }
    if (item->backup_resultline != NULL)
    {
      gw_resultline_free (item->backup_resultline);
      item->backup_resultline = NULL;
    }

    item->status = GW_SEARCH_IDLE;
}


//!
//! @brief Releases a GwSearchItem object from memory. 
//!
//! All of the various interally allocated memory in the GwSearchItem is freed.
//! The file descriptiors and such are made sure to also be closed.
//!
//! @param item The GwSearchItem to have it's memory freed.
//!
void gw_searchitem_free(GwSearchItem* item) {
  int i = 0;
  while (i < item->total_re) {
    regfree(&(item->re_exist[i]));
    regfree(&(item->re_locate[i]));
    regfree(&(item->re_relevance_high[i]));
    regfree(&(item->re_relevance_medium[i]));
    i++;
  }
  gw_searchitem_do_post_search_clean (item);
  free(item);
  item = NULL;
}



//!
//! @brief Comparison function that should be moved to the GwSearchItem file when it matures
//!
//! @param item A GwSearchItem to get search information from
//!
gboolean gw_searchitem_existance_generic_comparison (GwSearchItem *item, const int REGEX_TYPE)
{
    int i;
    int j;
    GwResultLine *rl;
    GwQueryLine *ql;

    rl = item->resultline;
    ql = item->queryline;
    //Compare kanji atoms
    i = 0;
    while (ql->kanji_atom[i])
    {
      if (regexec(&(ql->kanji_regex[REGEX_TYPE][i]), rl->kanji_start, 1, NULL, 0) == 0)
        return TRUE;
      i++;  
    }
    //Compare furigana atoms
    i = 0;
    while (ql->hira_atom[i] && ql->hira_atom[i])
    {
      if (regexec(&(ql->hira_regex[REGEX_TYPE][i]), rl->furigana_start, 1, NULL, 0) == 0)
        return TRUE;
      else if (regexec(&(ql->kata_regex[REGEX_TYPE][i]), rl->furigana_start, 1, NULL, 0) == 0)
        return TRUE;
      i++;  
    }
    //Compare romaji atoms
    i = 0;
    while (ql->roma_atom[i])
    {
      j = 0;
      while (rl->def_start[j])
      {
        if (regexec(&(ql->roma_regex[REGEX_TYPE][i]), rl->def_start[j], 1, NULL, 0) == 0)
          return TRUE;
        j++;
      }
      i++;  
    }
    //Compare mix atoms
    i = 0;
    while (ql->mix_atom[i])
    {
      if (regexec(&(ql->mix_regex[REGEX_TYPE][i]), rl->string, 1, NULL, 0) == 0)
        return TRUE;
      i++;  
    }
    return FALSE;
}

