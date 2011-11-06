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
//! @file console-callbacks.c
//!
//! @brief Abstraction layer for the console
//!
//! Used as a go between for functions interacting with the console.
//!


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <glib.h>

#include <waei/waei.h>

static gboolean _group_index_changed = FALSE;
static int _previous_percent = -1;

//!
//! @brief Not yet written
//!
void w_console_append_edict_result_cb (LwSearchItem *item)
{
    //Definitions
    int cont = 0;
    LwResultLine *resultline = item->resultline;

    //Kanji
    if (app->color_switch)
      printf("[32m%s", resultline->kanji_start);
    else
      printf("%s", resultline->kanji_start);
    //Furigana
    if (resultline->furigana_start)
      printf(" [%s]", resultline->furigana_start);
    //Other info
    if (resultline->classification_start)
    {
      if (app->color_switch)
        printf("[0m %s", resultline->classification_start);
      else
        printf("%s", resultline->classification_start);
    }
    //Important Flag
    if (resultline->important)
    {
      if (app->color_switch)
        printf("[0m %s", "P");
      else
        printf("%s", "P");
    }

    printf("\n");
    while (cont < resultline->def_total)
    {
      if (app->color_switch)
        printf("[0m      [35m%s [0m%s\n", resultline->number[cont], resultline->def_start[cont]);
      else
        printf("      %s %s\n", resultline->number[cont], resultline->def_start[cont]);
      cont++;
    }
    printf("\n");
}


//!
//! @brief Not yet written
//!
void w_console_append_kanjidict_result_cb (LwSearchItem *item)
{
    if (item == NULL) return;

    char line_started = FALSE;
      LwResultLine *resultline = item->resultline;

    //Kanji
    if (app->color_switch)
      printf("[32;1m%s[0m\n", resultline->kanji);
    else
      printf("%s\n", resultline->kanji);

    if (resultline->radicals)
      printf("%s%s\n", gettext("Radicals:"), resultline->radicals);

    if (resultline->strokes)
    {
      line_started = TRUE;
      printf("%s%s", gettext("Stroke:"), resultline->strokes);
    }

    if (resultline->frequency)
    {
      if (line_started)
        printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("Freq:"), resultline->frequency);
    }

    if (resultline->grade)
    {
      if (line_started)
        printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("Grade:"), resultline->grade);
    }

    if (resultline->jlpt)
    {
      if (line_started)
        printf(" ");
      line_started = TRUE;
      printf("%s%s", gettext("JLPT:"), resultline->jlpt);
    }

    if (line_started)
      printf("\n");

    if (resultline->readings[0])
      printf("%s%s\n", gettext("Readings:"), resultline->readings[0]);
    if (resultline->readings[1])
      printf("%s%s\n", gettext("Name:"), resultline->readings[1]);
    if (resultline->readings[2])
      printf("%s%s\n", gettext("Radical Name:"), resultline->readings[2]);

    if (resultline->meanings)
      printf("%s%s\n", gettext("Meanings:"), resultline->meanings);
    printf("\n");
}


//!
//! @brief Not yet written
//!
void w_console_append_examplesdict_result_cb (LwSearchItem *item)
{
    if (item == NULL) return;

    LwResultLine *resultline = item->resultline;


    if (resultline->def_start[0] != NULL)
    {
      if (app->color_switch)
        printf ("[32;1m%s[0m", gettext("E:\t"));
      else
        printf ("%s", gettext("E:\t"));
      printf ("%s", resultline->def_start[0]);
    }

    if (resultline->kanji_start != NULL)
    {
      if (app->color_switch)
        printf ("[32;1m%s[0m", gettext("\nJ:\t"));
      else
        printf ("%s", gettext("\nJ:\t"));
      printf ("%s", resultline->kanji_start);
    }

    if (resultline->furigana_start != NULL)
    {
      if (app->color_switch)
        printf("[32;1m%s[0m", gettext("\nD:\t"));
      else
        printf("%s", gettext("\nD:\t"));
      printf("%s", resultline->furigana_start);
    }

    printf("\n\n");
}


//!
//! @brief Not yet written
//!
void w_console_append_unknowndict_result_cb (LwSearchItem *item)
{
    if (item == NULL) return;

    printf("%s\n", item->resultline->string);
}


//!
//! @brief Not yet written
//!
void w_console_append_more_relevant_header_cb (LwSearchItem *item)
{
}


//!
//! @brief Not yet written
//!
void w_console_prepare_search_cb (LwSearchItem *item)
{
}


//!
//! @brief Not yet written
//!
void w_console_cleanup_search_cb (LwSearchItem *item)
{
    //Finish up
    if (item->total_results == 0 &&
        item->target != LW_OUTPUTTARGET_KANJI &&
        item->status == LW_SEARCHSTATUS_SEARCHING)
    {
      w_console_no_result (item);
    }
}


int w_console_uninstall_progress_cb (double fraction, gpointer data)
{
  //Declarations
  LwDictInfo *di;
  char *uri;

  //Initializations
  di = data;
  uri = lw_util_build_filename_by_dicttype (di->type, di->filename);

  printf("Removing %s...\n", uri);

  g_free (uri);

  return FALSE;
}


int w_console_install_progress_cb (double fraction, gpointer data)
{
  //Declarations
  LwDictInst *di;
  char *status;
  double current_fraction;
  int current_percent;

  //Initializations
  di = data;
  current_fraction = lw_dictinst_get_process_progress (di, fraction);
  current_percent = (int) (100.0 * current_fraction); 

  //Update the dictinst progress state only when the delta is large enough
  if (current_percent < 100 && _group_index_changed)
  {
    _group_index_changed = FALSE;
    printf("\n");
  }
  else if (current_percent == 100)
  {
    _group_index_changed = TRUE;
  }

  status = lw_dictinst_get_status_string (di, TRUE);
  printf("\r [%d%] %s", current_percent, status);
  _previous_percent = current_percent;
  g_free (status);

  return FALSE;
}


//!
//! @brief Print the "less relevant" header where necessary.
//!
void w_console_append_less_relevant_header_cb (LwSearchItem *item)
{
    if (app->color_switch)
      printf("\n[0;31m***[0m[1m%s[0;31m***************************[0m\n\n\n", gettext("Other Results"));
    else
      printf("\n***%s***************************\n\n\n", gettext("Other Results"));
}


