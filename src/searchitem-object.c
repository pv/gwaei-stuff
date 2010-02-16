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
GwSearchItem* gw_searchitem_new (char* query, GwDictInfo* dictionary, const int TARGET)
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
  temp->scratch_buffer = NULL;
  temp->dictionary = dictionary;
  temp->target = TARGET;
  temp->target_tb = (gpointer) get_gobject_from_target(TARGET);
  temp->total_relevant_results = 0;
  temp->total_irrelevant_results = 0;
  temp->total_results = 0;
  temp->current_line = 0;
  temp->resultline = NULL;
  temp->backup_resultline = NULL;
  temp->swap_resultline = NULL;
  temp->queryline = gw_queryline_new ();

  char *key = GCKEY_GW_LESS_RELEVANT_SHOW; 
  temp->show_less_relevant_results = gw_pref_get_boolean (key, TRUE);

  //Set function pointers
  switch (temp->dictionary->type)
  {
      case GW_DICT_TYPE_EDICT:
        if (!gw_queryline_parse_edict_string (temp->queryline, query)) return;
        temp->gw_searchitem_parse_result_string = &gw_resultline_parse_edict_result_string;
        if  (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
          temp->gw_searchitem_append_results_to_output = &gw_console_append_edict_results;
        else
          temp->gw_searchitem_append_results_to_output = &gw_ui_append_edict_results_to_buffer;
        break;
      case GW_DICT_TYPE_RADICALS:
        if (!gw_queryline_parse_edict_string (temp->queryline, query)) return;
        temp->gw_searchitem_parse_result_string = &gw_resultline_parse_radicaldict_result_string;
        if  (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
          temp->gw_searchitem_append_results_to_output = &gw_console_append_radicalsdict_results;
        else
          temp->gw_searchitem_append_results_to_output = &gw_ui_append_radicalsdict_results_to_buffer;
        break;
      case GW_DICT_TYPE_KANJI:
        if (!gw_queryline_parse_kanjidict_string (temp->queryline, query)) return;
        temp->gw_searchitem_parse_result_string = &gw_resultline_parse_kanjidict_result_string;
        if  (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
          temp->gw_searchitem_append_results_to_output = gw_console_append_kanjidict_results;
        else
          temp->gw_searchitem_append_results_to_output = &gw_ui_append_kanjidict_results_to_buffer;
        break;
      case GW_DICT_TYPE_EXAMPLES:
        if (!gw_queryline_parse_exampledict_string (temp->queryline, query)) return;
        temp->gw_searchitem_parse_result_string = &gw_resultline_parse_examplesdict_result_string;
        if  (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
          temp->gw_searchitem_append_results_to_output = &gw_console_append_examplesdict_results;
        else
          temp->gw_searchitem_append_results_to_output = &gw_ui_append_examplesdict_results_to_buffer;
        break;
      default:
        if (!gw_queryline_parse_edict_string (temp->queryline, query)) return;
        temp->gw_searchitem_parse_result_string = &gw_resultline_parse_unknowndict_result_string;
        if  (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
          temp->gw_searchitem_append_results_to_output = &gw_console_append_unknowndict_results;
        else
          temp->gw_searchitem_append_results_to_output = &gw_ui_append_unknowndict_results_to_buffer;
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
    if (item->scratch_buffer != NULL || (item->scratch_buffer = malloc (MAX_LINE)) == NULL)
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
  gw_ui_cancel_search_by_searchitem (item);
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
    int roma_check = 0, furi_check = 0;

    //Kanji radical dictionary search
    int i = 0;
    if (item->dictionary->type == GW_DICT_TYPE_KANJI || item->dictionary->type == GW_DICT_TYPE_RADICALS)
    {
      if (ql->strokes_total > 0 && rl->strokes != NULL)
        if (regexec(&(ql->strokes_regex[REGEX_TYPE][i]), rl->strokes, 1, NULL, 0) != 0)
          return FALSE;
      if (ql->frequency_total > 0 && rl->frequency != NULL)
        if (regexec(&(ql->frequency_regex[REGEX_TYPE][i]), rl->frequency, 1, NULL, 0) != 0)
          return FALSE;
      if (ql->grade_total > 0 && rl->grade != NULL)
        if (regexec(&(ql->grade_regex[REGEX_TYPE][i]), rl->grade, 1, NULL, 0) != 0)
          return FALSE;
      if (ql->jlpt_total > 0 && rl->jlpt != NULL)
        if (regexec(&(ql->jlpt_regex[REGEX_TYPE][i]), rl->jlpt, 1, NULL, 0) != 0)
          return FALSE;
      if (ql->roma_total > 0 && rl->meanings != NULL)
      {
        roma_check = 1;
        if (regexec(&(ql->roma_regex[REGEX_TYPE][i]), rl->meanings, 1, NULL, 0) == 0)
          roma_check = 2;
      }
      if (ql->furi_total > 0 && rl->readings[0] != NULL)
      {
        furi_check = 1;
        if (regexec(&(ql->furi_regex[REGEX_TYPE][i]), rl->readings[0], 1, NULL, 0) == 0)
          furi_check = 2;
      }

      //Trick code to allow romaji converted to furigana test to pass for the query...
      if ((roma_check == 1 && furi_check == 2) || roma_check == 2 && furi_check == 1) return TRUE;
      else if ((roma_check == 1 || furi_check == 1)) return FALSE;

      gboolean found_kanji = FALSE, found_radical = FALSE;
      for (i = 0; i < ql->kanji_total && !found_kanji; i++)
      {
        found_kanji = (rl->kanji != NULL && regexec(&(ql->kanji_regex[REGEX_TYPE][i]), rl->kanji, 1, NULL, 0) == 0);
        i++;
      }
      for (i = 0; i < ql->kanji_total && !found_radical; i++)
      {
        found_radical = (rl->radicals != NULL && regexec(&(ql->kanji_regex[REGEX_TYPE][i]), rl->radicals, 1, NULL, 0) == 0);
        i++;
      }

      //Make sure found radicals do not force a result into relevance
      if (REGEX_TYPE == GW_QUERYLINE_HIGH && found_kanji == FALSE) return FALSE;

      return (found_kanji | found_radical || roma_check == 2 || furi_check == 2);
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
          //return TRUE;
        i++;  
      }
      if (i > 0 && i == ql->kanji_total) return TRUE;

      //Compare furigana atoms
      i = 0;
      while (i < ql->furi_total && rl->furigana_start != NULL)
      {
        if (regexec(&(ql->furi_regex[REGEX_TYPE][i]), rl->furigana_start, 1, NULL, 0) != 0)
	  break;
          //return TRUE;
        i++;  
      }
      if (i > 0 && i == ql->furi_total) return TRUE;

      //Compare romaji atoms
      i = 0;
      while (i < ql->roma_total)
      {
        j = 0;
        while (rl->def_start[j] != NULL)
        {
          if (regexec(&(ql->roma_regex[REGEX_TYPE][i]), rl->def_start[j], 1, NULL, 0) != 0)
	    break;
            //return TRUE;
          j++;
        }
      	if (j > 0 && j == ql->roma_total) return TRUE;
        i++;  
      }

      //Compare word classification atoms
      i = 0;
      while (i < ql->roma_total && rl->classification_start != NULL)
      {
        if (regexec(&(ql->roma_regex[REGEX_TYPE][i]), rl->classification_start, 1, NULL, 0) != 0)
          break;
          //return TRUE;
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

