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
//! @file src/historylist-object.c
//!
//! @brief Search item and history management
//!
//! Functions and objects to create search items and manage them.
//!


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <libwaei/libwaei.h>


//!
//! @brief Creates a new LwHistoryList object. 
//!
//! Creates a new history list object which null pointers
//! a a back, forward history list and a current LwSearchItem.
//!
//! @return Returns the allocated LwHistoryList object.
//!
LwHistoryList* lw_historylist_new (const int MAX)
{
    LwHistoryList *temp;

    temp = (LwHistoryList*) malloc(sizeof(LwHistoryList));

    if (temp != NULL)
    {
      temp->back = NULL;
      temp->forward = NULL;
      temp->current = NULL;
      temp->max = MAX;
    }

    return temp;
}

void lw_historylist_free (LwHistoryList *list)
{
    //Free the results history list
    lw_historylist_clear_forward_list (list);
    lw_historylist_clear_back_list (list);
    lw_historylist_set_current_searchitem (list, NULL);
}


//!
//! @brief Clears the forward history of the desired target.
//!
void lw_historylist_clear_forward_list (LwHistoryList *list)
{
    //Declarations
    LwSearchItem *item;
    GList *iter;

    //Free the data of the list
    for (iter = list->forward; iter != NULL; iter = iter->next)
    {
      item = (LwSearchItem*) iter->data;
      if (item != NULL)
        lw_searchitem_free (item);
      iter->data = NULL;
    }

    //Free the list itself
    g_list_free (list->forward);
    list->forward = NULL;
}


//!
//! @brief Clears the back history of the desired target.
//!
void lw_historylist_clear_back_list (LwHistoryList *list)
{
    //Declarations
    LwSearchItem *item;
    GList *iter;

    //Free the data of the list
    for (iter = list->back; iter != NULL; iter = iter->next)
    {
      item = (LwSearchItem*) iter->data;
      if (item != NULL)
        lw_searchitem_free (item);
      iter->data = NULL;
    }

    //Free the list itself
    g_list_free (list->back);
    list->back = NULL;
}




//!
//! @brief Gets the back history of the target history list
//!
//! @see lw_historylist_get_forward_history ()
//! @see lw_historylist_get_current ()
//! @return Returns a GList containing the LwSearchItem back history
//!
GList* lw_historylist_get_back_list (LwHistoryList *list)
{
    return list->back;
}


//!
//! @brief Gets the forward history of the target history list
//!
//! @see lw_historylist_get_back_history ()
//! @see lw_historylist_get_current ()
//! @return Returns a GList containing the LwSearchItem forward history
//!
GList* lw_historylist_get_forward_list (LwHistoryList *list)
{
    return list->forward;
}


//!
//! @brief Gets the current search item of the user
//! @see lw_historylist_get_back_list ()
//! @see lw_historylist_get_forward_list ()
//! @return Returns a GList containing the LwSearchItem forward history
//!
LwSearchItem* lw_historylist_get_current_searchitem (LwHistoryList *list)
{
    return list->current;
}


//!
//! @brief Sets the current searchitem replacing the current one if it is there
//!
void lw_historylist_set_current_searchitem (LwHistoryList *list, LwSearchItem *item)
{
    if (list->current != NULL)
      lw_searchitem_free (list->current);
      
    list->current = item;
}


//!
//! @brief Concatinates together a copy of the back and forward histories
//!
//! This function was made with the idea of easily preparing a history list
//! for a history menu which doesn't care about separating each list.
//!
//! @see lw_historylist_get_back_list ()
//! @see lw_historylist_get_forward_list ()
//! @return Returns an allocated GList containing the back and forward history
//!
GList* lw_historylist_get_combined_list (LwHistoryList *list)
{
    //Declarations
    GList *combined;
    
    //Initializations
    combined = NULL;
    combined = g_list_copy (list->forward);
    combined = g_list_reverse (combined);
    combined = g_list_concat (combined, g_list_copy (list->back));

    return combined;
}


//!
//! @brief Moves an item to the back history
//!
//! The current variable has its LwSearchItem moved into the backhistory list.  The
//! forward history is also cleared at this time.
//!
void lw_historylist_add_searchitem (LwHistoryList *list, LwSearchItem *item)
{ 
    //Declarations
    GList *link;

    //Clear the forward history
    lw_historylist_clear_forward_list (list);

    //Shift current to the back history
    if (list->current != NULL)
    {
      list->back = g_list_prepend (list->back, list->current);
      list->current = NULL;
    }

    //Make sure the list hasn't gotten too long
    if (g_list_length(list->back) >= list->max)
    {
      link = g_list_last (list->back); 
      lw_searchitem_free (link->data);
      link->data = NULL;
      list->back = g_list_delete_link (list->back, link);
    }

    //Add the new current
    list->current = item;
}


//!
//! @brief Returns true if it is possible to go forward on a history list
//!
gboolean lw_historylist_has_forward (LwHistoryList *list)
{
    return (g_list_length (list->forward) > 0);
}


//!
//! @brief Returns true if it is possible to go back on a history list
//!
gboolean lw_historylist_has_back (LwHistoryList *list)
{
    return (g_list_length (list->back) > 0);
}


//!
//! @brief Go back 1 in history
//!
//! @param TARGET the target that should have it's history list adjusted
//!
gboolean lw_historylist_go_back (LwHistoryList *list)
{ 
    //Sanity check
    if (!lw_historylist_has_back (list)) return FALSE;

    //Declarations
    GList *link;

    //Move current onto the forward stack 
    if (list->current != NULL)
    {
      list->forward = g_list_append (list->forward, list->current);
      list->current = NULL;
    }

    //Make back to the current
    link = g_list_last (list->back); 
    list->current = link->data;
    list->back = g_list_delete_link (list->back, link);

    return TRUE;
}


//!
//! @brief Go formward 1 in history
//!
//! @param TARGET the target that should have it's history list adjusted
//!
gboolean lw_historylist_go_forward (LwHistoryList *list)
{ 
    //Sanity check
    if (!lw_historylist_has_forward (list)) return FALSE;

    //Declarations
    GList *link;

    //Move current onto the back stack 
    if (list->current != NULL)
    {
      list->back = g_list_append (list->back, list->current);
      list->current = NULL;
    }

    //Make forward to the current
    link = g_list_last (list->forward); 
    list->current = link->data;
    list->forward = g_list_delete_link (list->forward, link);

    return TRUE;
}

