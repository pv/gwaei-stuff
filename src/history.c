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
//! @file src/history.c
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
#include <gwaei/dictionaries.h>
#include <gwaei/history.h>

static GwHistoryList *results_history;
static GwHistoryList *kanji_history;


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
  char *key = GCKEY_GW_LESS_RELEVANT_SHOW; 
  temp->show_less_relevant_results = gw_pref_get_boolean (key, TRUE);

  //Create the compiled regular expression
  int eflags_exist    = REG_EXTENDED | REG_ICASE | REG_NOSUB;
  int eflags_relevant = REG_EXTENDED | REG_ICASE | REG_NOSUB;
  int eflags_locate   = REG_EXTENDED | REG_ICASE;

  //Create the needed regex for searching and locating
  char query_temp[MAX_QUERY];
  strcpy_with_query_preformatting (temp->query, query, temp);
  strcpy_with_query_formatting(query_temp, temp->query, temp);
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
    if (test_char > L'ン') {
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
//! @brief Returns the private historylist of the history.c file
//!
//! The two options here are GW_HISTORYLIST_RESULTS to get the results
//! history list and GW_HISTORYLIST_KANJI to get the kanji history list.
//!
//! @param TARGET The target who's history list we want.
//!
GwHistoryList* gw_historylist_get_list(const int TARGET)
{
    if (TARGET == GW_HISTORYLIST_RESULTS)
      return results_history;
    else if (TARGET == GW_HISTORYLIST_KANJI)
      return kanji_history;
    else
      return NULL;
}


//!
//! @brief Creates a new GwHistoryList object. 
//!
//! Creates a new history list object which null pointers
//! a a back, forward history list and a current GwSearchItem.
//!
//! @return Returns the allocated GwHistoryList object.
//!
GwHistoryList* gw_historylist_new()
{
    GwHistoryList *temp;
    if ((temp = malloc(sizeof(struct GwSearchItem))) != NULL)
    {
      temp->back = NULL;
      temp->forward = NULL;
      temp->current = NULL;
    }

    return temp;
}


//!
//! @brief Clears the forward history of the desired target.
//!
//! This function was designed in mind with the user case of
//! hitting the back button multiple times, then doing a new
//! search.  It frees all the search items, then deletes the links
//! in the list.
//!
//! @return Returns the allocated GwHistoryList object.
//!
void gw_historylist_clear_forward_history(const int TARGET)
{
    GwHistoryList *hl = gw_historylist_get_list (TARGET);

    while (hl->forward != NULL)
    {
      gw_searchitem_free((hl->forward)->data);
      hl->forward = g_list_delete_link(hl->forward, hl->forward);
    }
}


//!
//! @brief Gets the back history of the target history list
//!
//! @see gw_historylist_get_forward_history ()
//! @see gw_historylist_get_current ()
//! @return Returns a GList containing the GwSearchItem back history
//!
GList* gw_historylist_get_back_history (const int TARGET)
{
    GwHistoryList *list = gw_historylist_get_list (TARGET);
    return list->back;
}


//!
//! @brief Gets the forward history of the target history list
//!
//! @see gw_historylist_get_back_history ()
//! @see gw_historylist_get_current ()
//! @return Returns a GList containing the GwSearchItem forward history
//!
GList* gw_historylist_get_forward_history (const int TARGET)
{
    GwHistoryList *list = gw_historylist_get_list (TARGET);
    return list->forward;
}


//!
//! @brief Gets the current search item of the user
//!
//! This is the search item of the current search.  It doesn't get lumped
//! into a history list until the user hits the back button or does another
//! search. At program start, it has the special value of null which causes
//! many GUI elements to become disabled.
//!
//! @see gw_historylist_get_back_history ()
//! @see gw_historylist_get_forward_history ()
//! @return Returns a GList containing the GwSearchItem forward history
//!
GwSearchItem* gw_historylist_get_current (const int TARGET)
{
    GwHistoryList *list = gw_historylist_get_list (TARGET);
    return list->current;
}


//!
//! @brief Concatinates together a copy of the back and forward histories
//!
//! This function was made with the idea of easily preparing a history list
//! for a history menu which doesn't care about separating each list.
//!
//! @see gw_historylist_get_back_history ()
//! @see gw_historylist_get_forward_history ()
//! @return Returns an allocated GList containing the back and forward history
//!
GList* gw_historylist_get_combined_history_list (const int TARGET)
{
    GwHistoryList *hl = gw_historylist_get_list (TARGET);
    GList *back_copy = g_list_copy (hl->back);

    GList *out = NULL;
    out = g_list_copy (hl->forward);
    out = g_list_reverse (out);
    out = g_list_concat (out, back_copy);

    return out;
}


//!
//! @brief Moves an item to the back history
//!
//! The current variable has its GwSearchItem moved into the backhistory list.  The
//! forward history is also cleared at this time.
//!
void gw_historylist_add_searchitem_to_history(const int TARGET, GwSearchItem *item)
{ 
    GwHistoryList *hl = gw_historylist_get_list (TARGET);
    gw_historylist_clear_forward_history(TARGET);

    if (g_list_length(hl->back) >= 20)
    {
      GList* last = g_list_last (hl->back); 
      gw_searchitem_free(last->data);
      hl->back = g_list_delete_link(hl->back, last);
    }
    hl->back = g_list_prepend(hl->back, item);
}


//!
//! @brief Flopps the history stack 1 item in the desired direction
//!
//! Data is shifted between the forward, current, and back variables. If current is
//! null, the list just fills it in rather than shifting the data around.
//!
static void shift_history_by_target(const int TARGET, GList **from, GList **to)
{
    GwHistoryList *hl = gw_historylist_get_list (TARGET);
    GwSearchItem **current = &(hl->current);

    //Handle the current searchitem if it exists
    if (*current != NULL)
    {
      if ((*current)->total_results)
        *to = g_list_prepend (*to, *current);
      else
        gw_searchitem_free (*current);
      *current = NULL;
    }

    //Shift the top back searchitem to current (which is now empty)
    GList *item = *from;
    *from = g_list_remove_link (*from, item);
    *current = item->data;

    if (g_list_length (*from) == 0)
      *from = NULL;
}


//!
//! @brief Go back 1 in history
//!
//! @param TARGET the target that should have it's history list adjusted
//!
void gw_historylist_go_back_by_target (const int TARGET)
{ 
    GwHistoryList *hl = gw_historylist_get_list (TARGET);
    shift_history_by_target (TARGET, &(hl->back), &(hl->forward));
}


//!
//! @brief Go formward 1 in history
//!
//! @param TARGET the target that should have it's history list adjusted
//!
void gw_historylist_go_forward_by_target (const int TARGET)
{ 
    GwHistoryList *hl = gw_historylist_get_list (TARGET);
    shift_history_by_target (TARGET, &(hl->forward), &(hl->back));
}


//!
//! @brief Prepare the historylists for the desired widgets
//!
//! Currently there is the results history list and the mostly unused
//! kanji history list for the sidebar.
//!
void gw_history_initialize_history() {
    results_history = gw_historylist_new();
    kanji_history   = gw_historylist_new();
}



