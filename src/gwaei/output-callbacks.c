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
//! @file src/gtk/output-callbacks.c
//!
//! @brief Abstraction layer for gtk objects
//!
//! Used as a go between for functions interacting with GUI interface objects.
//! This is the gtk version.
//!


#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>

static GtkWidget* _searchwindow_results_popup_new (char*);
static void _searchwindow_show_popup_cb (GtkWidget*, gpointer);
static void _searchwindow_destroy_popup_cb (GtkWidget*, gpointer);
static void _searchwindow_destroy_text_cb (GtkWidget*, gpointer);
static void _searchwindow_new_tab_with_search_cb (GtkMenuItem*, gpointer);
static void _searchwindow_search_for_searchitem_online_cb (GtkMenuItem*, gpointer);
static void _searchwindow_destroy_tab_menuitem_searchitem_data_cb (GObject*, gpointer);

//!
//! @brief PRIVATE FUNCTION. A Stes the text of the desired mark.
//!
//! @param item A LwSearchItem to gleam information from
//! @param text The desired text to set to the mark
//! @param mark_name The name of the mark to set the new attributes to
//!
//!
static void _set_header (LwSearchItem *item, char* text, char* mark_name)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

  gdk_threads_enter();
    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextMark *mark;
    gint line;
    char *new_text;

    //Initializations
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    mark = gtk_text_buffer_get_mark (buffer, mark_name);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
    line = gtk_text_iter_get_line (&iter);

    //Move the insertion header to the less relevenant section
    if (strcmp(mark_name, "less_relevant_header_mark") == 0)
    {
      GtkTextMark *target_mark;
      GtkTextIter iter;
      target_mark = gtk_text_buffer_get_mark (buffer, "less_rel_content_insertion_mark");
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, target_mark);
      gtk_text_buffer_move_mark_by_name (buffer, "content_insertion_mark", &iter);
    }

    //Update the header text
    new_text = g_strdup_printf ("%s\n", text);
    if (new_text != NULL)
    {
      GtkTextIter end_iter;
      gtk_text_buffer_get_iter_at_line(buffer, &end_iter, line + 1);
      gtk_text_buffer_delete (buffer, &iter, &end_iter);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, new_text, -1, "header", "important", NULL);
      g_free (new_text);
      new_text = NULL;
    }
  gdk_threads_leave();
}


//!
//! @brief PRIVATE FUNCTION. Applies a tag to a section of text
//!
//! @param line An integer showing the line in the buffer to tag
//! @param start_offset the ending character in the line to highlight
//! @param end_offset The ending character in the line to highlight
//! @param item A LwSearchItem to get general information from
//!
static void _add_match_highlights (gint line, gint start_offset, gint end_offset, LwSearchItem* item)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    LwQueryLine *ql;
    int match_start_byte_offset;
    int match_end_byte_offset;
    int match_character_offset;
    GtkTextIter start_iter;
    GtkTextIter end_iter;
    char *text;
    GRegex *re;
    GRegex ***iter;
    GMatchInfo *match_info;

    //Initializations
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    ql = item->queryline;
    gtk_text_buffer_get_iter_at_line_offset (buffer, &start_iter, line, start_offset);
    gtk_text_buffer_get_iter_at_line_offset (buffer, &end_iter, line, end_offset);
    text = gtk_text_buffer_get_slice (buffer, &start_iter, &end_iter, FALSE);

    //Look for kanji atoms
    for (iter = ql->re_kanji; *iter != NULL && **iter != NULL; iter++)
    {
      re = (*iter)[LW_RELEVANCE_LOCATE];
      if (g_regex_match (re, text, 0, &match_info))
      { 
        while (g_match_info_matches (match_info))
        {
          g_match_info_fetch_pos (match_info, 0, &match_start_byte_offset, &match_end_byte_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_start_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (buffer, &start_iter, line, match_character_offset + start_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_end_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (buffer, &end_iter, line, match_character_offset + start_offset);
          gtk_text_buffer_apply_tag_by_name (buffer, "match", &start_iter, &end_iter);
          g_match_info_next (match_info, NULL);
        }
        g_match_info_free (match_info);
      }
    }

    //Look for furigana atoms
    for (iter = ql->re_furi; *iter != NULL && **iter != NULL; iter++)
    {
      re = (*iter)[LW_RELEVANCE_LOCATE];
      if (g_regex_match (re, text, 0, &match_info))
      { 
        while (g_match_info_matches (match_info))
        {
          g_match_info_fetch_pos (match_info, 0, &match_start_byte_offset, &match_end_byte_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_start_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (buffer, &start_iter, line, match_character_offset + start_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_end_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (buffer, &end_iter, line, match_character_offset + start_offset);
          gtk_text_buffer_apply_tag_by_name (buffer, "match", &start_iter, &end_iter);
          g_match_info_next (match_info, NULL);
        }
        g_match_info_free (match_info);
      }
    }

    //Look for romaji atoms
    for (iter = ql->re_roma; *iter != NULL; iter++)
    {
      re = (*iter)[LW_RELEVANCE_LOCATE];
      if (re != NULL && g_regex_match (re, text, 0, &match_info))
      { 
        while (g_match_info_matches (match_info))
        {
          g_match_info_fetch_pos (match_info, 0, &match_start_byte_offset, &match_end_byte_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_start_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (buffer, &start_iter, line, match_character_offset + start_offset);
          match_character_offset = g_utf8_pointer_to_offset (text, text + match_end_byte_offset);
          gtk_text_buffer_get_iter_at_line_offset (buffer, &end_iter, line, match_character_offset + start_offset);
          gtk_text_buffer_apply_tag_by_name (buffer, "match", &start_iter, &end_iter);
          g_match_info_next (match_info, NULL);
        }
        g_match_info_free (match_info);
      }
    }

    //Cleanup
    g_free (text);
}


//!
//! @brief PRIVATE FUNCTION. Moves the content insertion mark to another mark's spot
//!
//! @param item A LwSearchItem pointer to gleam information from
//! @param name The name of the mark to move the content insertion mark to
//!
static void _shift_stay_mark (LwSearchItem *item, char *name)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextMark *mark;
    GtkTextIter iter;

    //Initializations
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    mark = gtk_text_buffer_get_mark (buffer, "content_insertion_mark");
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);

    if ((mark = gtk_text_buffer_get_mark (buffer, name)) == NULL)
      gtk_text_buffer_create_mark (buffer, name, &iter, TRUE);
    else
      gtk_text_buffer_move_mark (buffer, mark, &iter);
}


//!
//! @brief PRIVATE FUNCTION.  Updates the position of a mark to accomidate new results.
//!
//! @param item A LwSearchItem to gleam information from.
//! @param stay_name The name of the mark that stays in place before the new result.
//! @param append_name The name of the mark that moves to the end after the new result is added.
//!
static void _shift_append_mark (LwSearchItem *item, char *stay_name, char *append_name)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextMark *stay_mark, *append_mark;

    //Initializations
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    stay_mark = gtk_text_buffer_get_mark (buffer, stay_name);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, stay_mark);

    if ((append_mark = gtk_text_buffer_get_mark (buffer, append_name)) == NULL)
      gtk_text_buffer_create_mark (buffer, append_name, &iter, FALSE);
    else
      gtk_text_buffer_move_mark (buffer, append_mark, &iter);
}


//!
//! @brief PRIVATE FUNCTION.  When adding a result to the buffer, it just adds the the kanji/hiragana section
//!
//! This function is made to help readability of edict results since there is a lot of repeating.
//!
//! @param item A LwSearchItem pointer to use for sdata.
//!
static void _append_def_same_to_buffer (LwSearchItem* item)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    LwResultLine* resultline;
    GtkTextMark *mark;

    //Initializations
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    resultline = item->resultline;

    _shift_append_mark (item, "previous_result", "new_result");
    if ((mark = gtk_text_buffer_get_mark (buffer, "previous_result")) != NULL)
    {
      GtkTextIter iter;
      int line, start_offset, end_offset;
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
      line = gtk_text_iter_get_line (&iter);
      start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, " /", -1, "important", NULL);
      gtk_text_buffer_insert (buffer, &iter, " ", -1);
      //Kanji
      if (resultline->kanji_start != NULL)
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->kanji_start, -1, "important", NULL);
      //Furigana
      if (resultline->furigana_start != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, " [", -1, "important", NULL);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->furigana_start, -1, "important", NULL);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, "]", -1, "important", NULL);
      }
      //Other info
      if (resultline->classification_start != NULL)
      {
        gtk_text_buffer_insert (buffer, &iter, " ", -1);
        GtkTextIter copy = iter;
        gtk_text_iter_backward_char (&copy);
        gtk_text_buffer_remove_tag_by_name (buffer, "important", &copy, &iter);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->classification_start, -1, "gray", "italic", NULL);
      }
      if (resultline->important == TRUE)
      {
        gtk_text_buffer_insert                   (buffer, &iter, " ", -1);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Pop"), -1, "small", NULL);
      }
      end_offset = gtk_text_iter_get_line_offset (&iter);
      _add_match_highlights (line, start_offset, end_offset, item);
    }
}


//!
//! @brief Appends an edict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param item A LwSearchItem to gleam information from.
//!
void gw_output_append_edict_results_cb (LwSearchItem *item)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextMark *mark;
    GtkTextIter iter;
    gboolean furigana_exists, kanji_exists;
    gboolean same_def_totals, same_first_def, same_furigana, same_kanji, skip;
    gboolean remove_last_linebreak;
    int line, start_offset, end_offset;
    LwResultLine* resultline;

    char *popup_text;
    GtkButton* button;
    GtkLabel *label;
    GtkTextChildAnchor *anchor;

  gdk_threads_enter();

    //Initializations
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    kanji_exists = (item->resultline->kanji_start != NULL && item->backup_resultline->kanji_start != NULL);
    furigana_exists = (item->resultline->furigana_start != NULL && item->backup_resultline->furigana_start != NULL);
    resultline = item->resultline;
    popup_text = NULL;

    if (item->resultline->kanji_start == NULL || item->backup_resultline->kanji_start == NULL)
    {
      skip = TRUE;
    }
    else
    {
      same_def_totals = (item->resultline->def_total == item->backup_resultline->def_total);
      same_first_def = (strcmp(item->resultline->def_start[0], item->backup_resultline->def_start[0]) == 0);
      same_furigana = (!furigana_exists || strcmp(item->resultline->furigana_start, item->backup_resultline->furigana_start) == 0);
      same_kanji = (!kanji_exists || strcmp(item->resultline->kanji_start, item->backup_resultline->kanji_start) == 0);
      skip = FALSE;
    }

    //Begin comparison if possible
    if (!skip && ((same_def_totals) || (same_kanji && same_furigana)) && same_first_def)
    {
      //_append_def_same_to_buffer (item);
      gdk_threads_leave ();
      return;
    }

    //Start output
    remove_last_linebreak = (!skip && same_kanji && same_first_def);
    mark = gtk_text_buffer_get_mark (buffer, "content_insertion_mark");

    if (remove_last_linebreak)
    {
      GtkTextIter si, ei;
      GtkTextMark *temp_mark;
      if ((temp_mark = gtk_text_buffer_get_mark (buffer, "previous_result")) && gtk_text_buffer_get_mark (buffer, "note_mark") == NULL)
      {
        gtk_text_buffer_get_iter_at_mark (buffer, &si, temp_mark);
        gtk_text_buffer_create_mark (buffer, "note_mark", &si, TRUE);
        gtk_text_buffer_get_iter_at_line (buffer, &ei, gtk_text_iter_get_line (&si) + 1);
        gtk_text_buffer_delete (buffer, &si, &ei);
      }
      gtk_text_buffer_get_iter_at_mark (buffer, &ei, mark);
      gtk_text_buffer_get_iter_at_line (buffer, &si, gtk_text_iter_get_line (&ei) - 1);
      gtk_text_buffer_delete(buffer, &si, &ei);
    }
    else
    {
      GtkTextMark *temp_mark;
      if ((temp_mark = gtk_text_buffer_get_mark (buffer, "note_mark")) != NULL)
         gtk_text_buffer_delete_mark (buffer, temp_mark);
    }

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
    line = gtk_text_iter_get_line (&iter);

    //Kanji
    if (resultline->kanji_start != NULL)
    {
      if (popup_text == NULL) popup_text = g_strdup (resultline->kanji_start);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->kanji_start, -1, "important", NULL);
    }
    //Furigana
    if (resultline->furigana_start != NULL)
    {
      if (popup_text == NULL) popup_text = g_strdup (resultline->furigana_start);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, " [", -1, "important", NULL);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->furigana_start, -1, "important", NULL);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, "]", -1, "important", NULL);
    }
    //Other info
    if (resultline->classification_start != NULL)
    {
      gtk_text_buffer_insert (buffer, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->classification_start, -1, "gray", "italic", NULL);
    }
    if (resultline->important == TRUE)
    {
      gtk_text_buffer_insert (buffer, &iter, " ", -1);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Pop"), -1, "small", NULL);
    }


    //Insert popup button
    if (popup_text != NULL)
    {
      gtk_text_buffer_insert (buffer, &iter, "   ", -1);
      button = GTK_BUTTON (gtk_button_new ());
      g_signal_connect (G_OBJECT (button), "clicked", G_CALLBACK (_searchwindow_show_popup_cb), popup_text);
      g_signal_connect (G_OBJECT (button), "destroy", G_CALLBACK (_searchwindow_destroy_text_cb), popup_text);
      label = GTK_LABEL (gtk_label_new (NULL));
      gtk_label_set_markup (label, "<small><small>▼</small></small>");
      gtk_button_set_relief (button, GTK_RELIEF_NONE);
      gtk_container_add (GTK_CONTAINER (button), GTK_WIDGET (label));
      gtk_widget_show (GTK_WIDGET (button));
      gtk_widget_show (GTK_WIDGET (label));
      anchor = gtk_text_buffer_create_child_anchor (buffer, &iter);
      gtk_text_view_add_child_at_anchor (view, GTK_WIDGET (button), anchor);
    }


    _shift_stay_mark (item, "previous_result");
    start_offset = 0;
    end_offset = gtk_text_iter_get_line_offset (&iter);

    if (!remove_last_linebreak) gtk_text_buffer_insert (buffer, &iter, "\n", -1);
    _add_match_highlights (line, start_offset, end_offset, item);

    //Definitions
    int i = 0;
    while (resultline->def_start[i] != NULL)
    {
      gtk_text_buffer_insert (buffer, &iter, "      ", -1);

      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->number[i], -1, "comment", NULL);
      gtk_text_buffer_insert                   (buffer, &iter, " ", -1);
      gtk_text_buffer_insert                   (buffer, &iter, resultline->def_start[i], -1);
      end_offset = gtk_text_iter_get_line_offset (&iter);
      line = gtk_text_iter_get_line (&iter);
      _add_match_highlights (line, start_offset, end_offset, item);
      gtk_text_buffer_insert                   (buffer, &iter, "\n", -1);
      i++;
    }
    gtk_text_buffer_insert (buffer, &iter, "\n", -1);

  gdk_threads_leave();
}


//!
//! @brief Appends a kanjidict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param item A LwSearchItem to gleam information from.
//!
void gw_output_append_kanjidict_results_cb (LwSearchItem *item)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    GtkTextIter iter;
    GtkTextMark *mark;
    LwResultLine* resultline;
    int line, start_offset, end_offset;

    //Initializations
    resultline = item->resultline;
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);

    if (item->target == LW_OUTPUTTARGET_RESULTS)
    {
  gdk_threads_enter();
      mark = gtk_text_buffer_get_mark (buffer, "content_insertion_mark");
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);

      //Kanji
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->kanji, -1, "large", "center", NULL);
      if (item->target == LW_OUTPUTTARGET_RESULTS) gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, " ", -1, "large", "center", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
      if (item->target == LW_OUTPUTTARGET_RESULTS)
        _add_match_highlights (line, start_offset, end_offset, item);

      gtk_text_buffer_insert (buffer, &iter, "\n", -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);

      //Radicals
      if (resultline->radicals != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Radicals:"), -1, "important", NULL);
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert (buffer, &iter, resultline->radicals, -1);
        gtk_text_buffer_insert (buffer, &iter, " ", -1);
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        if (item->target == LW_OUTPUTTARGET_RESULTS)
          _add_match_highlights (line, start_offset, end_offset, item);

        gtk_text_buffer_insert (buffer, &iter, "\n", -1);
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);

/*
        GwRadicalsWindow *radicals;
        radicals = 
        gw_radiccalswindow_set_button_sensitive_when_label_is (resultline->radicals);
*/
      }

      //Readings
      if (resultline->readings[0] != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Readings:"), -1, "important", NULL);
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert (buffer, &iter, resultline->readings[0], -1);
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        if (item->target == LW_OUTPUTTARGET_RESULTS)
          _add_match_highlights (line, start_offset, end_offset, item);
        gtk_text_buffer_insert (buffer, &iter, "\n", -1);
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
      }
      if (resultline->readings[1] != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Name:"), -1, "important", NULL);
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert (buffer, &iter, resultline->readings[1], -1);
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        if (item->target == LW_OUTPUTTARGET_RESULTS)
          _add_match_highlights (line, start_offset, end_offset, item);
        gtk_text_buffer_insert (buffer, &iter, "\n", -1);
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
      }
      if (resultline->readings[2] != NULL)
      {
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Radical Name:"), -1, "important", NULL);
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
        gtk_text_buffer_insert (buffer, &iter, resultline->readings[2], -1);
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
        if (item->target == LW_OUTPUTTARGET_RESULTS)
          _add_match_highlights (line, start_offset, end_offset, item);
        gtk_text_buffer_insert (buffer, &iter, "\n", -1);
        gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
      }


      //etc
      gboolean line_started = FALSE;
      if (resultline->strokes)
      {
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Stroke:"), -1, "important", NULL);
        gtk_text_buffer_insert (buffer, &iter, resultline->strokes, -1);
        line_started = TRUE;
      }
      if (resultline->frequency)
      {
        if (line_started) gtk_text_buffer_insert (buffer, &iter, " ", -1);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Freq:"), -1, "important", NULL);
        gtk_text_buffer_insert (buffer, &iter, resultline->frequency, -1);
        gtk_text_buffer_insert (buffer, &iter, " ", -1);
      }
      if (resultline->grade)
      {
        if (line_started) gtk_text_buffer_insert (buffer, &iter, " ", -1);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Grade:"), -1, "important", NULL);
        gtk_text_buffer_insert (buffer, &iter, resultline->grade, -1);
      }
      if (resultline->jlpt)
      {
        if (line_started) gtk_text_buffer_insert (buffer, &iter, " ", -1);
        gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("JLPT:"), -1, "important", NULL);
        gtk_text_buffer_insert (buffer, &iter, resultline->jlpt, -1);
      }

      gtk_text_buffer_insert (buffer, &iter, "\n", -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);

      //Meanings
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("Meanings:"), -1, "important", NULL);
      gtk_text_buffer_insert (buffer, &iter, resultline->meanings, -1);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      if (item->target == LW_OUTPUTTARGET_RESULTS)
        _add_match_highlights (line, start_offset, end_offset, item);

      gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
  gdk_threads_leave ();
    }
    
    if (item->target == LW_OUTPUTTARGET_KANJI)
    {
      char *markup;
      char *new;
      char *base;
      char *linebreak;
      gboolean first = TRUE;

      markup = g_strdup ("");
      new = NULL;
      base = NULL;
      linebreak = NULL;

      if (resultline->radicals) {
        first = FALSE;
        base = markup;
        new = g_markup_printf_escaped ("<b>%s </b>%s\n", gettext("Radicals:"), resultline->radicals);
        markup = g_strjoin ("", base, new, NULL);
        g_free (base);
        base = NULL;
        g_free (new);
        new = NULL;
      }
      if (resultline->readings[0]) {
        first = FALSE;
        base = markup;
        new = g_markup_printf_escaped ("<b>%s </b>%s\n", gettext("Readings:"), resultline->readings[0]);
        markup = g_strjoin ("", base, new, NULL);
        g_free (base);
        base = NULL;
        g_free (new);
        new = NULL;
      } 
      if (resultline->readings[1]) {
        first = FALSE;
        base = markup;
        new = g_markup_printf_escaped ("<b>%s </b>%s\n", gettext("Readings:"), resultline->readings[1]);
        markup = g_strjoin ("", base, new, NULL);
        g_free (base);
        base = NULL;
        g_free (new);
        new = NULL;
      }
      if (resultline->readings[2]) {
        first = FALSE;
        base = markup;
        new = g_markup_printf_escaped ("<b>%s: </b>%s\n", gettext("Radical Name"), resultline->readings[2]);
        markup = g_strjoin ("", base, new, NULL);
        g_free (base);
        base = NULL;
        g_free (new);
        new = NULL;
      }

      if (resultline->strokes) {
        base = markup;
        new = g_markup_printf_escaped ("<b>%s </b>%s   ", gettext("Stroke:"), resultline->strokes);
        markup = g_strjoin ("", base, new, NULL);
        g_free (base);
        base = NULL;
        g_free (new);
        new = NULL;
      }
      if (resultline->frequency) {
        base = markup;
        new = g_markup_printf_escaped ("<b>%s </b>%s   ", gettext("Freq:"), resultline->frequency);
        markup = g_strjoin ("", base, new, NULL);
        g_free (base);
        base = NULL;
        g_free (new);
        new = NULL;
      }
      if (resultline->grade) {
        base = markup;
        new = g_markup_printf_escaped ("<b>%s </b>%s   ", gettext("Grade:"), resultline->grade);
        markup = g_strjoin ("", base, new, NULL);
        g_free (base);
        base = NULL;
        g_free (new);
        new = NULL;
      }
      if (resultline->jlpt) {
        base = markup;
        new = g_markup_printf_escaped ("<b>%s </b>%s   ", gettext("JLPT:"), resultline->jlpt);
        markup = g_strjoin ("", base, new, NULL);
        g_free (base);
        base = NULL;
        g_free (new);
        new = NULL;
      }

      base = markup;
      new = g_markup_printf_escaped ("\n");
      markup = g_strjoin ("", base, new, NULL);
      g_free (base);
      base = NULL;
      g_free (new);
      new = NULL;

      if (resultline->meanings) {
        base = markup;
        new = g_markup_printf_escaped ("<b>%s </b>%s", gettext("Meanings:"), resultline->meanings);
        markup = g_strjoin ("", base, new, NULL);
        g_free (base);
        base = NULL;
        g_free (new);
        new = NULL;
      }

      //Declarations
      GtkWidget *child;
      GtkWidget *label;
      GtkWidget *window;
      GtkWidget *hbox;
      char *markup2;

      markup2 = g_markup_printf_escaped ("<span font=\"KanjiStrokeOrders 100\">%s</span>", resultline->kanji);
      window = GTK_WIDGET (gtk_widget_get_tooltip_window (GTK_WIDGET (view)));

      if (window != NULL) {
        child = gtk_bin_get_child (GTK_BIN (window));
        if (child != NULL) gtk_widget_destroy (GTK_WIDGET (child));

        hbox = GTK_WIDGET (gtk_hbox_new (FALSE, 3));
        gtk_container_set_border_width (GTK_CONTAINER (hbox), 10);
        gtk_container_add (GTK_CONTAINER (window), GTK_WIDGET (hbox));

        label = GTK_WIDGET (gtk_label_new (NULL));
        gtk_label_set_markup (GTK_LABEL (label), markup2);
        gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, TRUE, 0);
        gtk_label_set_selectable (GTK_LABEL (label), TRUE);
        gtk_widget_set_can_focus (GTK_WIDGET (label), FALSE);

        label = GTK_WIDGET (gtk_label_new (NULL));
        gtk_label_set_markup (GTK_LABEL (label), markup);
        gtk_box_pack_start (GTK_BOX (hbox), GTK_WIDGET (label), FALSE, TRUE, 0);
        gtk_label_set_selectable (GTK_LABEL (label), TRUE);
        gtk_widget_set_can_focus (GTK_WIDGET (label), FALSE);

        gtk_widget_show_all (hbox);

      }

      g_free (markup);
      g_free (markup2);
    }
}


//!
//! @brief Appends a examplesdict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly
//!
//! @param item A LwSearchItem to gleam information from.
//!
void gw_output_append_examplesdict_results_cb (LwSearchItem *item)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    LwResultLine* resultline;
    int line, start_offset, end_offset;
    GtkTextMark *mark;
    GtkTextIter iter;

  gdk_threads_enter();

    //Initializations
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    view = GTK_TEXT_VIEW (sdata->view);
    buffer = gtk_text_view_get_buffer (view);
    resultline = item->resultline;
    mark = gtk_text_buffer_get_mark (buffer, "content_insertion_mark");
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);

    if (resultline->def_start[0] != NULL)
    {
      // TRANSLATORS: The "E" stands for "English"
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("E:\t"), -1, "important", "comment", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->def_start[0], -1, "important", NULL, NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
      _add_match_highlights (line, start_offset, end_offset, item);
    }

    if (resultline->kanji_start != NULL)
    {
      // TRANSLATORS: The "J" stands for "Japanese"
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("\nJ:\t"), -1, "important", "comment", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->kanji_start, -1, NULL, NULL, NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
      _add_match_highlights (line, start_offset, end_offset, item);
    }

    if (resultline->furigana_start != NULL)
    {
      // TRANSLATORS: The "D" stands for "Detail"
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, gettext("\nD:\t"), -1, "important", "comment", NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_insert_with_tags_by_name (buffer, &iter, resultline->furigana_start, -1, NULL, NULL, NULL);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
      gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
      _add_match_highlights (line, start_offset, end_offset, item);
    }

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
    gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
  gdk_threads_leave();
}


//!
//! @brief Appends a examplesdict style result to the buffer, adding nice formatting.
//!
//! This is a part of a set of functions used for the global output function pointers and
//! isn't used directly.  This is the fallback safe function for unknown dictionaries.
//!
//! @param item A LwSearchItem to gleam information from.
//!
void gw_output_append_unknowndict_results_cb (LwSearchItem *item)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Definitions
    GwSearchData *sdata;
    GtkTextView *view;
    GtkTextBuffer *buffer;
    LwResultLine *resultline;
    GtkTextIter iter;
    GtkTextMark *mark;
    int line, start_offset, end_offset;

  gdk_threads_enter();

    //Initializations
    resultline = item->resultline;
    mark = gtk_text_buffer_get_mark (buffer, "content_insertion_mark");

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); start_offset = gtk_text_iter_get_line_offset (&iter);
    gtk_text_buffer_insert (buffer, &iter, resultline->string, -1);
    if (item->target == LW_OUTPUTTARGET_RESULTS) gtk_text_buffer_insert (buffer, &iter, " ", -1);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); end_offset = gtk_text_iter_get_line_offset (&iter);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
    _add_match_highlights (line, start_offset, end_offset, item);

    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark);
    gtk_text_buffer_insert (buffer, &iter, "\n\n", -1);
    gtk_text_buffer_get_iter_at_mark (buffer, &iter, mark); line = gtk_text_iter_get_line (&iter);
  gdk_threads_leave();
}


//!
//! @brief Add an header to irrelevant "other" results with number of matches
//!
void gw_output_append_less_relevant_header_cb (LwSearchItem *item)
{
    //Declarations
    int irrelevant;
    char *message;

    //Initializations
    irrelevant = item->total_irrelevant_results;
    message = g_strdup_printf (ngettext("Other Result %d", "Other Results %d", irrelevant), irrelevant);

    if (message != NULL)
    {
      _set_header (item, message, "less_relevant_header_mark");
      g_free (message);
    }
}


//!
//! @brief Add an header to relevant "main" results with number of matches
//!
void gw_output_append_more_relevant_header_cb (LwSearchItem *item)
{
    //Declarations
    int relevant;
    char *message;

    //Initializations
    relevant = item->total_relevant_results;
    message = g_strdup_printf (ngettext("Main Result %d", "Main Results %d", relevant), relevant);

    if (message != NULL)
    {
      _set_header (item, message, "more_relevant_header_mark");
      g_free (message);
    }
}


//!
//! @brief Sets up the interface before each search begins
//! @param item A LwSearchItem pointer to get information from
//!
void gw_output_pre_search_prep_cb (LwSearchItem *item)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;

    //Initializations
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));
    sdata->view = gw_searchwindow_get_current_textview (sdata->window);

    if (item->target == LW_OUTPUTTARGET_RESULTS)
      gw_searchwindow_initialize_buffer_by_searchitem (sdata->window, item);
}


//!
//! @brief The details to be taken care of after a search is finished
//! @param item A LwSearchItem pointer to get information from
//!
void gw_output_after_search_cleanup_cb (LwSearchItem *item)
{
    //Sanity check
    g_assert (lw_searchitem_has_data (item));

    //Declarations
    GwSearchData *sdata;

    //Initializations
    sdata = GW_SEARCHDATA (lw_searchitem_get_data (item));

    //Finish up
    if (item->total_results == 0 &&
        item->target != LW_OUTPUTTARGET_KANJI &&
        item->status != LW_SEARCHSTATUS_CANCELING)
    {
      gw_searchwindow_display_no_results_found_page (sdata->window, item);
    }
}


//!
//! @brief Populates the main contextual menu with search options
//!
//! @param view The GtkTexView that was right clicked
//! @param menu The Popup menu to populate
//! @param data  Currently unused gpointer containing user data
//!
static GtkWidget* _searchwindow_results_popup_new (char* query_text)
{
    if (query_text == NULL) return NULL;

    //Declarations
    GwSearchWindow *window;
    GtkWidget *menu;
    LwSearchItem *item = NULL;
    LwDictInfo *di = NULL;
    char *menu_text = NULL;
    GtkWidget *menuitem = NULL;
//    GtkWidget *menuimage = NULL;
    GtkTextIter start_iter, end_iter;
    window = GW_SEARCHWINDOW (gw_app_get_window_by_type (app, GW_WINDOW_SEARCH));
    if (window == NULL) return NULL;
    int i = 0;

    //Initializations
    menu = gtk_menu_new ();
    char *website_url_menuitems[] = {
      "Wikipedia", "http://www.wikipedia.org/wiki/%s", "wikipedia.png",
      "Goo.ne.jp", "http://dictionary.goo.ne.jp/srch/all/%s/m0u/", "goo.png",
      "Google.com", "http://www.google.com/search?q=%s", "google.png",
      NULL, NULL, NULL
    };

    //Add internal dictionary links
    i = 0;
    while ((di = lw_dictinfolist_get_dictinfo_by_load_position (LW_DICTINFOLIST (app->dictinfolist), i)) != NULL)
    {
      if (di != NULL && (item = lw_searchitem_new (query_text, di, LW_OUTPUTTARGET_RESULTS, app->prefmanager, NULL)) != NULL)
      {
        menu_text = g_strdup_printf ("%s", di->longname);
        if (menu_text != NULL)
        {
          menuitem = GTK_WIDGET (gtk_image_menu_item_new_with_label (menu_text));
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (_searchwindow_new_tab_with_search_cb), item);
          g_signal_connect (G_OBJECT (menuitem), "destroy",  G_CALLBACK (_searchwindow_destroy_tab_menuitem_searchitem_data_cb), item);
          gtk_menu_shell_append (GTK_MENU_SHELL (menu), GTK_WIDGET (menuitem));
          gtk_widget_show (GTK_WIDGET (menuitem));
//          gtk_widget_show (GTK_WIDGET (menuimage));
          g_free (menu_text);
          menu_text = NULL;
        }
      }
      i++;
    }

    //Separator
    menuitem = gtk_separator_menu_item_new ();
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), GTK_WIDGET (menuitem));
    gtk_widget_show (GTK_WIDGET (menuitem));

    //Add weblinks
    i = 0;
    di =  lw_dictinfolist_get_dictinfo_by_load_position (LW_DICTINFOLIST (app->dictinfolist), 0);
    while (website_url_menuitems[i] != NULL)
    {
      if (di != NULL && (item = lw_searchitem_new (query_text, di, LW_OUTPUTTARGET_RESULTS, app->prefmanager, NULL)) != NULL)
      {
        //Create handy variables
        char *name = website_url_menuitems[i];
        char *url = g_strdup_printf(website_url_menuitems[i + 1], query_text);
        if (url != NULL)
        {
          g_free (item->queryline->string);
          item->queryline->string = g_strdup (url);
          g_free (url);
          url = NULL;
        }
        char *icon_path = website_url_menuitems[i + 2];

        //Start creating
        menu_text = g_strdup_printf ("%s", name);
        if (menu_text != NULL)
        {
          menuitem = GTK_WIDGET (gtk_image_menu_item_new_with_label (menu_text));
          char *path = g_build_filename (DATADIR2, PACKAGE, icon_path, NULL);
          if (path != NULL)
          {
//            menuimage = gtk_image_new_from_file (path);
//            gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (menuitem), GTK_WIDGET (menuimage));
            g_free (path);
            path = NULL;
          }
          g_signal_connect (G_OBJECT (menuitem), "activate", G_CALLBACK (_searchwindow_search_for_searchitem_online_cb), item);
          g_signal_connect (G_OBJECT (menuitem), "destroy",  G_CALLBACK (_searchwindow_destroy_tab_menuitem_searchitem_data_cb), item);
          gtk_menu_shell_append (GTK_MENU_SHELL (menu), GTK_WIDGET (menuitem));
          gtk_widget_show (GTK_WIDGET (menuitem));
//          gtk_widget_show (GTK_WIDGET (menuimage));
          g_free (menu_text);
          menu_text = NULL;
        }
      }
      i += 3;
    }

    return menu;
}

static void _searchwindow_show_popup_cb (GtkWidget *widget, gpointer data)
{
    GtkMenu *popup;
    char *popup_text;

    popup_text = (char*) data;
    popup = GTK_MENU (_searchwindow_results_popup_new (popup_text));
    gtk_menu_attach_to_widget (GTK_MENU (popup), GTK_WIDGET (widget), NULL);

    gtk_menu_popup (popup, NULL, NULL, NULL, NULL, 0, gtk_get_current_event_time ());

    g_signal_connect_after (G_OBJECT (widget), "destroy", G_CALLBACK (_searchwindow_destroy_popup_cb), popup);
}


static void _searchwindow_destroy_popup_cb (GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy (GTK_WIDGET (data));
}


static void _searchwindow_destroy_text_cb (GtkWidget *widget, gpointer data)
{
    g_free (data);
}


//!
//! @brief Frees the LwSearchItem memory that is attached to the activate tab callback
//! @param widget Currently unused widget pointe
//! @param data gpointer to a LwSearchItem to be freed
//!
static void _searchwindow_destroy_tab_menuitem_searchitem_data_cb (GObject *object, gpointer data)
{
    //Declarations
    LwSearchItem *item;

    //Initializations
    item = LW_SEARCHITEM (data);

    if (item != NULL)
    {
      lw_searchitem_free (item);
      item = NULL;
    }
}


//!
//! @brief Searches for the word in a webbrower in an external dictionary
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
static void _searchwindow_search_for_searchitem_online_cb (GtkMenuItem *widget, gpointer data)
{
    LwSearchItem *item;
    GError *error;
    GwSearchWindow *window;
    GtkTextView *view;
    GwSearchData *sdata;

    window = GW_SEARCHWINDOW (gw_app_get_window_by_type (app, GW_WINDOW_SEARCH));
    if (window == NULL) return;
    item = LW_SEARCHITEM (data);
    if (item != NULL)
    {
      view = gw_searchwindow_get_current_textview (window);
      sdata = gw_searchdata_new (view, window);
      lw_searchitem_set_data (item, sdata, LW_SEARCHITEM_DATA_FREE_FUNC (gw_searchdata_free));
      error = NULL;

      gtk_show_uri (NULL, item->queryline->string, gtk_get_current_event_time (), &error);
      gw_common_handle_error (&error, GTK_WINDOW (window->toplevel), TRUE);
    }
}


//!
//! @brief Sets up an initites a new search in a new tab
//!
//! @param widget Currently unused widget pointer
//! @param data A gpointer to a LwSearchItem that hold the search information
//!
static void _searchwindow_new_tab_with_search_cb (GtkMenuItem *widget, gpointer data)
{
    if (!gw_app_can_start_search (app)) return;

    //Declarations
    GwSearchWindow *window;
    LwSearchItem *item;
    LwSearchItem *item_new;
    GtkTextView *view;
    GwSearchData *sdata;
    int index;

    //Initializations
    window = GW_SEARCHWINDOW (gw_app_get_window_by_type (app, GW_WINDOW_SEARCH));
    if (window == NULL) return;
    item = LW_SEARCHITEM (data);
    item_new = lw_searchitem_new (item->queryline->string, item->dictionary, item->target, app->prefmanager, NULL);
    if (item_new != NULL)
    {
      view = gw_searchwindow_get_current_textview (window);
      sdata = gw_searchdata_new (view, window);
      lw_searchitem_set_data (item_new, sdata, LW_SEARCHITEM_DATA_FREE_FUNC (gw_searchdata_free));

      index = gw_searchwindow_new_tab (window);
      gtk_notebook_set_current_page (window->notebook, index);
      gw_searchwindow_start_search (window, item_new);
    }
}


