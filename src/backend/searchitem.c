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

#include <gwaei/backend.h>

static gboolean _query_is_sane (const char* query)
{
    //Declarations
    char *q;
    gboolean is_sane;

    //Initializations
    q = gw_util_prepare_query (query, TRUE); 
    is_sane = TRUE;

    //Tests
    if (strlen (q) == 0)
      is_sane = FALSE;

    if (g_str_has_prefix (q, "|") || g_str_has_prefix (q, "&")) 
      is_sane = FALSE;
    if (g_str_has_suffix (q, "\\") || g_str_has_suffix (q, "|") || g_str_has_suffix (q, "&")) 
      is_sane = FALSE;

    const char *ptr;
    int count;

    count = 0;

    for (ptr = q; *ptr != '\0'; ptr = g_utf8_next_char(ptr))
    {
      if (*ptr == '\\' && *(ptr + 1) != '\0')
      {
        if (*(ptr + 1) == '.'  ||
            *(ptr + 1) == '?'  || 
            *(ptr + 1) == '['  || 
            *(ptr + 1) == ']'  || 
            *(ptr + 1) == '^'  || 
            *(ptr + 1) == '$'  || 
            *(ptr + 1) == '|'  ||
            *(ptr + 1) == '*'  || 
            *(ptr + 1) == '+'  || 
            *(ptr + 1) == '('  || 
            *(ptr + 1) == '\\' || 
            *(ptr + 1) == ')'    )
          count++;
        ptr += 2;
      }
      else if (*ptr == '.'  || 
               *ptr == '?'  || 
               *ptr == '['  || 
               *ptr == ']'  || 
               *ptr == '^'  || 
               *ptr == '$'  || 
               *ptr == '|'  ||
               *ptr == '*'  || 
               *ptr == '+'  || 
               *ptr == '\\' ||
               *ptr == ' ' ||
               *ptr == '('  || 
               *ptr == ')'    )
      {
        ptr += 1;
      }
      else
      {
        count++;
        ptr += 1;
      }
    }

    if (count == 0) 
      is_sane = FALSE;

    g_free (q);

    return is_sane;
}


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
GwSearchItem* gw_searchitem_new (char* query, GwDictInfo* dictionary, const int TARGET)
{
    if (!_query_is_sane (query)) return NULL;

    GwSearchItem *temp;

    //Allocate some memory
    if ((temp = malloc(sizeof(GwSearchItem))) == NULL) return NULL;

    temp->results_medium = NULL;
    temp->results_low = NULL;
    temp->thread = NULL;
    temp->mutex = g_mutex_new ();
    
    if (TARGET != GW_TARGET_RESULTS &&
        TARGET != GW_TARGET_KANJI   &&
        TARGET != GW_TARGET_CONSOLE       )
      return NULL;

    //Set the internal pointers to the correct global variables
    temp->fd     = NULL;
    temp->status = GW_SEARCH_IDLE;
    temp->scratch_buffer = NULL;
    temp->dictionary = dictionary;
    temp->target = TARGET;
    temp->target_tb = NULL;
    temp->target_tv = NULL;
    temp->total_relevant_results = 0;
    temp->total_irrelevant_results = 0;
    temp->total_results = 0;
    temp->current_line = 0;
    temp->resultline = NULL;
    temp->backup_resultline = NULL;
    temp->swap_resultline = NULL;
    temp->queryline = gw_queryline_new ();
    temp->history_relevance_idle_timer = 0;
    temp->show_only_exact_matches = FALSE;

    //Set function pointers
    switch (temp->dictionary->engine)
    {
        case GW_ENGINE_EDICT:
          if (!gw_queryline_parse_edict_string (temp->queryline, query)) return NULL;
          temp->gw_searchitem_parse_result_string = &gw_resultline_parse_edict_result_string;
          temp->gw_searchitem_ui_append_results_to_output = gw_engine_get_append_edict_results_func ();
          break;
        case GW_ENGINE_KANJI:
          if (!gw_queryline_parse_kanjidict_string (temp->queryline, query)) return NULL;
          temp->gw_searchitem_parse_result_string = &gw_resultline_parse_kanjidict_result_string;
          temp->gw_searchitem_ui_append_results_to_output = gw_engine_get_append_kanjidict_results_func ();
          break;
        case GW_ENGINE_EXAMPLES:
          if (!gw_queryline_parse_exampledict_string (temp->queryline, query)) return NULL;
          temp->gw_searchitem_parse_result_string = &gw_resultline_parse_examplesdict_result_string;
          temp->gw_searchitem_ui_append_results_to_output = gw_engine_get_append_examplesdict_results_func ();
        break;
        default:
          if (!gw_queryline_parse_edict_string (temp->queryline, query)) return NULL;
          temp->gw_searchitem_parse_result_string = &gw_resultline_parse_unknowndict_result_string;
          temp->gw_searchitem_ui_append_results_to_output = gw_engine_get_append_unknowndict_results_func ();
          break;
    }
    temp->gw_searchitem_ui_append_less_relevant_header_to_output = gw_engine_get_append_less_relevant_header_func ();
    temp->gw_searchitem_ui_append_more_relevant_header_to_output = gw_engine_get_append_more_relevant_header_func ();
    temp->gw_searchitem_ui_pre_search_prep = gw_engine_get_pre_search_prep_func ();
    temp->gw_searchitem_ui_after_search_cleanup = gw_engine_get_after_search_cleanup_func ();

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
    if (item->scratch_buffer != NULL || (item->scratch_buffer = malloc (GW_IO_MAX_FGETS_LINE)) == NULL)
    {
      return FALSE;
    }
    if (item->resultline != NULL || (item->resultline = gw_resultline_new ()) == NULL)
    {
      free (item->scratch_buffer);
      item->scratch_buffer = NULL;
      return FALSE;
    }
    if (item->backup_resultline != NULL || (item->backup_resultline = gw_resultline_new ()) == NULL)
    {
      gw_resultline_free (item->resultline);
      item->resultline = NULL;
      free (item->scratch_buffer);
      item->scratch_buffer = NULL;
      return FALSE;
    }

    //Reset internal variables
    item->current_line = 0;
    item->progress_feedback_line = 0;
    item->total_relevant_results = 0;
    item->total_irrelevant_results = 0;
    item->total_results = 0;
    item->thread = NULL;

    if (item->fd == NULL)
    {
      const char *directory = gw_util_get_directory_for_engine (item->dictionary->engine);
      const char *filename = item->dictionary->filename;
      char *path = g_build_filename (directory, filename, NULL);
      item->fd = fopen (path, "r");
      g_free (path);
      path = NULL;
    }
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

    if (item->scratch_buffer != NULL)
    {
      free(item->scratch_buffer);
      item->scratch_buffer = NULL;
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

    //item->thread = NULL;  This code creates multithreading problems
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
void gw_searchitem_free (GwSearchItem* item)
{
  if (item == NULL) return;

  if (item->thread != NULL) 
  {
    item->status = GW_SEARCH_CANCELING;
    g_thread_join(item->thread);
    item->thread = NULL;
    g_mutex_free (item->mutex);
    item->mutex = NULL;
  }
  gw_searchitem_do_post_search_clean (item);
  free (item->queryline);
  free (item);
  item = NULL;
}


//!
//! @brief Comparison function that should be moved to the GwSearchItem file when it matures
//!
//! @param item A GwSearchItem to get search information from
//! @param REGEX_TYPE A constant int representing the regex type to test
//!
gboolean gw_searchitem_existance_generic_comparison (GwSearchItem *item, const int REGEX_TYPE)
{
    GwResultLine *rl;
    GwQueryLine *ql;
    rl = item->resultline;
    ql = item->queryline;

    //Kanji radical dictionary search
    int i = 0;
    if (item->dictionary->engine == GW_ENGINE_KANJI)
    {
      gboolean strokes_check_passed = TRUE;
      gboolean frequency_check_passed = TRUE;
      gboolean grade_check_passed = TRUE;
      gboolean jlpt_check_passed = TRUE;
      gboolean romaji_high_check_passed = TRUE;
      gboolean romaji_check_passed = TRUE;
      gboolean furigana_check_passed = TRUE;
      gboolean kanji_check_passed = TRUE;
      gboolean radical_check_passed = TRUE;

      //Calculate the strokes check
      if (ql->strokes_total > 0)
      {
        if (rl->strokes == NULL || regexec(&(ql->strokes_regex[REGEX_TYPE][i]), rl->strokes, 1, NULL, 0) != 0)
          strokes_check_passed = FALSE;
      }

      //Calculate the frequency check
      if (ql->frequency_total > 0)
      {
        if (rl->frequency == NULL || regexec(&(ql->frequency_regex[REGEX_TYPE][i]), rl->frequency, 1, NULL, 0) != 0)
          frequency_check_passed = FALSE;
      }

      //Calculate the grade check
      if (ql->grade_total > 0)
      {
        if (rl->grade == NULL || regexec(&(ql->grade_regex[REGEX_TYPE][i]), rl->grade, 1, NULL, 0) != 0)
          grade_check_passed = FALSE;
      }

      //Calculate the jlpt check
      if (ql->jlpt_total > 0)
      {
        if (rl->jlpt == NULL || regexec(&(ql->jlpt_regex[REGEX_TYPE][i]), rl->jlpt, 1, NULL, 0) != 0)
          jlpt_check_passed = FALSE;
      }

      //Calculate the romaji check
      if (ql->roma_total > 0 && rl->meanings != NULL)
      {
        if (regexec(&(ql->roma_regex[GW_QUERYLINE_HIGH][i]), rl->meanings, 1, NULL, 0) != 0)
          romaji_high_check_passed = FALSE;
      }

      //Calculate the romaji check
      if (ql->roma_total > 0 && rl->meanings != NULL)
      {
        if (regexec(&(ql->roma_regex[REGEX_TYPE][i]), rl->meanings, 1, NULL, 0) != 0)
          romaji_check_passed = FALSE;
      }

      //Calculate the furigana check
      if (ql->furi_total > 0 && rl->readings[0] != NULL)
      {
        if (regexec(&(ql->furi_regex[REGEX_TYPE][i]), rl->readings[0], 1, NULL, 0) != 0)
          furigana_check_passed = FALSE;
      }

      //Calculate the kanji check
      for (i = 0; i < ql->kanji_total && rl->kanji != NULL; i++)
      {
        if (regexec(&(ql->kanji_regex[REGEX_TYPE][i]), rl->kanji, 1, NULL, 0) != 0)
          kanji_check_passed = FALSE;
      }

      //Calculate the radical check
      if (rl->radicals == NULL)
      {
        radical_check_passed = FALSE;
      }
      for (i = 0; i < ql->kanji_total && radical_check_passed; i++)
      {
        if (regexec(&(ql->kanji_regex[REGEX_TYPE][i]), rl->radicals, 1, NULL, 0) != 0)
          radical_check_passed = FALSE;
      }

      //Return our results
      if (REGEX_TYPE == GW_QUERYLINE_HIGH)
      {
        return (kanji_check_passed && romaji_high_check_passed);
      }
      else
      {
        return (strokes_check_passed &&
                frequency_check_passed &&
                grade_check_passed &&
                jlpt_check_passed &&
                romaji_check_passed &&
                furigana_check_passed &&
                (radical_check_passed | kanji_check_passed));
      }
    }
    //Standard dictionary search
    else
    {
      int i;
      int j;
      //Compare kanji atoms
      i = 0;
      while (i < ql->kanji_total && rl->kanji_start != NULL)
      {
        if (regexec(&(ql->kanji_regex[REGEX_TYPE][i]), rl->kanji_start, 1, NULL, 0) != 0)
	  break;
        i++;  
      }
      if (i > 0 && i == ql->kanji_total) return TRUE;

      //Compare furigana atoms
      i = 0;
      while (i < ql->furi_total && rl->furigana_start != NULL)
      {
        if (regexec(&(ql->furi_regex[REGEX_TYPE][i]), rl->furigana_start, 1, NULL, 0) != 0)
	  break;
        i++;  
      }
      if (i > 0 && i == ql->furi_total) return TRUE;

      //Compare furigana atoms
      i = 0;
      while (i < ql->furi_total && rl->kanji_start != NULL && rl->furigana_start == NULL)
      {
        if (regexec(&(ql->furi_regex[REGEX_TYPE][i]), rl->kanji_start, 1, NULL, 0) != 0)
	  break;
        i++;  
      }
      if (i > 0 && i == ql->furi_total) return TRUE;

      //Compare romaji atoms
      j = 0;
      while (rl->def_start[j] != NULL)
      {
        i = 0;
        while (i < ql->roma_total)
        {
          if (regexec(&(ql->roma_regex[REGEX_TYPE][i]), rl->def_start[j], 1, NULL, 0) != 0)
	    break;
          i++;
        }
      	if (i > 0 && i == ql->roma_total) return TRUE;
        j++;  
      }

      //Compare word classification atoms
      i = 0;
      while (i < ql->roma_total && rl->classification_start != NULL)
      {
        if (regexec(&(ql->roma_regex[REGEX_TYPE][i]), rl->classification_start, 1, NULL, 0) != 0)
          break;
        i++;  
      }
      if (i > 0 && i == ql->roma_total) return TRUE;

      return FALSE;

      //Compare mix atoms
      i = 0;
      while (i < ql->mix_total && rl->string != NULL)
      {
        if (regexec(&(ql->mix_regex[REGEX_TYPE][i]), rl->string, 1, NULL, 0) == 0)
          return TRUE;
        i++;  
      }

      return FALSE;
    }
}


//!
//! @brief comparison function for determining if two GwSearchItems are equal
//!
gboolean gw_searchitem_is_equal (GwSearchItem *item1, GwSearchItem *item2)
{
  //Declarations
  gboolean queries_are_equal;
  gboolean dictionaries_are_equal;

  //Sanity checks
  if (item1 == item2) return TRUE;
  if (item1 == NULL) return FALSE;
  if (item2 == NULL) return FALSE;

  g_mutex_lock (item1->mutex);
  g_mutex_lock (item2->mutex);

  //Initializations
  queries_are_equal = (strcmp(item1->queryline->string, item2->queryline->string) == 0);
  dictionaries_are_equal = (item1->dictionary == item2->dictionary);

  g_mutex_unlock (item1->mutex);
  g_mutex_unlock (item2->mutex);

  return (queries_are_equal && dictionaries_are_equal);
}


//!
//! @brief a method for incrementing an internal integer for determining if a result set has worth
//!
void gw_searchitem_increment_history_relevance_timer (GwSearchItem *item)
{
  if (item != NULL && item->history_relevance_idle_timer < GW_HISTORY_TIME_TO_RELEVANCE)
    item->history_relevance_idle_timer++;
}


//!
//! @brief Checks if the relevant timer has passed a threshold
//!
gboolean gw_searchitem_has_history_relevance (GwSearchItem *item)
{
  return (item != NULL && item->total_results && item->history_relevance_idle_timer >= GW_HISTORY_TIME_TO_RELEVANCE);
}
