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
  char *key = GCKEY_GW_LESS_RELEVANT_SHOW; 
  temp->show_less_relevant_results = gw_pref_get_boolean (key, TRUE);


  //Set function pointers
  switch (temp->dictionary->type)
  {
      case GW_DICT_OTHER:
        temp->gw_searchitem_parse_result_string = &gw_resultline_parse_result_string;
        if  (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
          temp->gw_searchitem_append_results_to_output = &gw_console_append_normal_results;
        else
          temp->gw_searchitem_append_results_to_output = &gw_ui_append_results_to_buffer;
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
          ;
        else
          temp->gw_searchitem_append_results_to_output = &gw_ui_append_examples_results_to_buffer;
        break;
      default:
        temp->gw_searchitem_parse_result_string = &gw_resultline_parse_unknown_result_string;
        if  (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
          ;
        else
        temp->gw_searchitem_append_results_to_output = &gw_ui_append_unknown_results_to_buffer;
        break;
  }


  //Create the compiled regular expression
  int eflags_exist    = REG_EXTENDED | REG_ICASE | REG_NOSUB;
  int eflags_relevant = REG_EXTENDED | REG_ICASE | REG_NOSUB;
  int eflags_locate   = REG_EXTENDED | REG_ICASE;

  //Create the needed regex for searching and locating
  char query_temp[MAX_QUERY];
  gw_fmt_strcpy_with_query_preformatting (temp->query, query, temp);
  gw_fmt_strcpy_with_query_formatting(query_temp, temp->query, temp);
  if (strlen(query_temp) == 0) return NULL;

  char expression[(MAX_QUERY * 4) + 150];
  char *query_ptr = &query_temp[strlen(query_temp)];
  temp->total_re = 0;

  //The loop compiles a regex for every item between the delimitors
  while (query_ptr != &query_temp[0] && temp->total_re < MAX_QUERY)
  {
    *(query_ptr - 1) = '\0'; //removes the trailing slash

    do {
      query_ptr = g_utf8_prev_char(query_ptr);
    } while (query_ptr != &query_temp[0] && *(query_ptr - 1) != DELIMITOR_CHR);

    //Create Regular Expression for Match exists
    if (regcomp(&((temp->re_exist)[temp->total_re]), query_ptr, eflags_exist) != 0) {
      int j = 0;
      while (j < temp->total_re - 1)
        regfree(&(temp->re_exist[j]));
      free(temp);
      temp = NULL;
      return NULL;
    }

    //Create Regular Expression for Locate the match
    if (regcomp(&((temp->re_locate)[temp->total_re]), query_ptr, eflags_locate) != 0) {
      int j = 0;
      while (j < temp->total_re - 1)
        regfree(&(temp->re_locate[j]));
      free(temp);
      temp = NULL;
      return NULL;
    }

    gunichar test_char = g_utf8_get_char(query_ptr);
    if (test_char == L'(')
    {
      test_char = g_utf8_get_char(g_utf8_next_char(query_ptr));
    }

    //Prepare the string expression for high relevance
    //Kanji version
    if (temp->dictionary->type == GW_DICT_EXAMPLES)
    {
      if (test_char > L'ン')
      {
        strcpy(expression, "(");
        strcat(expression, query_ptr);
        strcat(expression, ")");
      }
      else
      {
        strcpy(expression, "(\\b(");
        strcat(expression, query_ptr);
        strcat(expression, ")\\b)");
      }
    }
    else if (test_char > L'ン') {
        strcpy(expression, "((^無)|(^不)|(^非)|(^)|(^お)|(^御))(");
        strcat(expression, query_ptr);
        strcat(expression, ")((\\])|(\\))|(\\})|( ))");
    }
    //Katakana/Hiragana version
    else if (test_char > L'ぁ')
    {
        strcpy(expression, "((^)|(\\[)|(\\()|(\\{)|( )|(^お))(");
        strcat(expression, query_ptr);
        strcat(expression, ")((\\])|(\\))|(\\})|( ))");
    }
    //Romanji version
    else {
        strcpy(expression, "\\{(");
        strcat(expression, query_ptr);
        strcat(expression, ")\\}|(\\) |/)((to )|(to be )|())(");
        strcat(expression, query_ptr);
        strcat(expression, ")(( \\([^/]+\\)/)|(/))|(\\[)(");
        strcat(expression, query_ptr);
        strcat(expression, ")(\\])|^(");
        strcat(expression, query_ptr);
        strcat(expression, ")\\b");
    }

    //Create Regular Expression for high relevance
    if (regcomp(&((temp->re_relevance_high)[temp->total_re]), expression, eflags_relevant) != 0) 
    {
      int j = 0;
      while (j < temp->total_re - 1) {
  
        regfree(&(temp->re_locate[j]));
      }
      free(temp);
      return NULL;
    }


    //Prepare the string expression for medium relevance
    //Two character Kanji version
    /*
    if ( g_utf8_strlen (query_ptr, -1) == 2 &&
         g_utf8_get_char(query_ptr) > L'ン' && 
         g_utf8_get_char(g_utf8_next_char(query_ptr)) > L'ン' )
    {
      // (^query..|^..query\b)
      strcpy(expression, "(^");
      strcat(expression, query_ptr);
      strcat(expression, "..\\b|^..");
      strcat(expression, query_ptr);
      strcat(expression, "\\b)");
    }
    //Katakana/Hiragana/Kanji version
    else */
    if (test_char >= L'ぁ')
    {
      strcpy(expression, "((^)|(\\[)|(\\()|(\\{)|( )|(お)|(を)|(に)|(で)|(は)|(と))(");
      /*
      strcpy(expression, "(((\\()|(\\[)|(\\{)|( ))(");
      */
      strcat(expression, query_ptr);
      /*
      strcat(expression, "))|((");
      strcat(expression, query_ptr);
      strcat(expression, ")((\\()|(\\[)|(\\{)|( )))");
      */
      strcat(expression, ")((で)|(が)|(の)|(を)|(に)|(で)|(は)|(と)|(\\])|(\\))|(\\})|( ))");
    }
    //Romanji version
    else
    {
      strcpy(expression, "(\\b(");
      strcat(expression, query_ptr);
      strcat(expression, ")\\b|^(");
      strcat(expression, query_ptr);
      strcat(expression, "))");
    }

    //Create Regular Expression for medium relevance
    if (regcomp(&((temp->re_relevance_medium)[temp->total_re]), expression, eflags_relevant) != 0) {
      int j = 0;
      while (j < temp->total_re - 1) {
        regfree(&(temp->re_relevance_medium[j]));
        regfree(&(temp->re_relevance_high[j]));
        regfree(&(temp->re_locate[j]));
      }
      free(temp);
      return NULL;
    }

    temp->total_re++;
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

