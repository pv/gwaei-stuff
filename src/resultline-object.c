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
//!  @file src/resultline-object.c
//!
//!  @brief Management of result lines
//!

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/resultline-object.h>



GwResultLine* gw_resultline_new ()
{
    GwResultLine* temp;

    if ((temp = (GwResultLine*) malloc(sizeof(struct GwResultLine))) == NULL) return NULL;

    //A place for a copy of the raw string
    temp->string[0] = '\0';
    
    //General formatting
    temp->def_start[0] = NULL;
    temp->def_total = 0;
    temp->kanji_start = NULL;
    temp->furigana_start = NULL;
    temp->classification_start = NULL;
    strcpy(temp->first, "(1)");
    temp->important = FALSE;

    //Kanji things
    temp->strokes = NULL;
    temp->frequency = NULL;
    temp->readings[0] = NULL;
    temp->readings[1] = NULL;
    temp->meanings = NULL;
    temp->grade = NULL;
    temp->jlpt = NULL;
    temp->kanji = NULL;
    temp->radicals = NULL;

    return temp;
}

void gw_resultline_clear_variables (GwResultLine *temp)
{
    //A place for a copy of the raw string
    temp->string[0] = '\0';
    
    //General formatting
    temp->def_start[0] = NULL;
    temp->def_total = 0;
    temp->kanji_start = NULL;
    temp->furigana_start = NULL;
    temp->classification_start = NULL;
    strcpy(temp->first, "(1)");
    temp->important = FALSE;

    //Kanji things
    temp->strokes = NULL;
    temp->frequency = NULL;
    temp->readings[0] = NULL;
    temp->readings[1] = NULL;
    temp->meanings = NULL;
    temp->grade = NULL;
    temp->jlpt = NULL;
    temp->kanji = NULL;
    temp->radicals = NULL;


}

void gw_resultline_free (GwResultLine *item)
{
    free (item);
}

void gw_resultline_parse_result_string (GwResultLine *line, char *string)
{
    gw_resultline_clear_variables (line);
    strncpy(line->string, string, MAX_LINE);

    char *ptr = line->string;
    char *next = NULL;
    char *nextnext = NULL;
    char *nextnextnext = NULL;
    char *temp = NULL;

    //Remove the final line break
    if ((temp = g_utf8_strchr (line->string, -1, '\n')) != NULL)
    {
        temp--;
        *temp = '\0';
    }

    //Set the kanji pointers
    line->kanji_start = ptr;
    ptr = g_utf8_strchr (ptr, -1, L' ');
    *ptr = '\0';

    //Set the furigana pointer
    ptr++;
    if (g_utf8_get_char(ptr) == L'[')
    {
      ptr = g_utf8_next_char(ptr);
      line->furigana_start = ptr;
      ptr = g_utf8_strchr (ptr, -1, L']');
      *ptr = '\0';
    }
    else
    {
      line->furigana_start = NULL;
      ptr--;
    }


    //Find if there is a type description classification
    temp = ptr;
    temp++;
    temp = g_utf8_strchr (temp, -1, L'/');
    if (g_utf8_get_char(temp + 1) == '(')
    {
      line->classification_start = temp + 2;
      temp = g_utf8_strchr (temp, -1, L')');
      *temp = '\0';
      ptr = temp;
    }

    //Set the definition pointers
    ptr++;
    ptr = g_utf8_next_char(ptr);
    line->def_start[0] = ptr;
    line->number[0] = line->first;
    int i = 1;

    temp = ptr;
    while ((temp = g_utf8_strchr(temp, -1, L'(')) != NULL && i < 50)
    {
      next = g_utf8_next_char (temp);
      nextnext = g_utf8_next_char (next);
      nextnextnext = g_utf8_next_char (nextnext);
      if (*next != '\0' && *nextnext != '\0' &&
          *next == L'1' && *nextnext == L')')
      {
         line->def_start[0] = line->def_start[0] + 4;
      }
      else if (*next != '\0' && *nextnext != '\0' && *nextnextnext != '\0' &&
               *next >= L'1' && *next <= L'9' && (*nextnext == L')' || *nextnextnext == L')'))
      {
         *(temp - 1) = '\0';
         line->number[i] = temp;
         temp = g_utf8_strchr (temp, -1, L')');
         *(temp + 1) = '\0';
         line->def_start[i] = temp + 2;
         i++;
      }
      temp = temp + 2;
    }
    line->def_total = i;
    i--;

    //Get the importance
    //temp = line->def_start[i] + strlen(line->def_start[i]) - 4;
    if ((temp = g_utf8_strchr (line->def_start[i], -1, L'(')) != NULL)
    {
      line->important = (*temp == '(' && *(temp + 1) == 'P' && *(temp + 2) == ')');
      if (line->important) *(temp - 1) = '\0';
    }
}


void gw_resultline_parse_kanji_result_string (GwResultLine *line, char *string)
{
    gw_resultline_clear_variables (line);
    strncpy(line->string, string, MAX_LINE);

    //First generate the grade, stroke, frequency, and jplt fields
    char *start[4], *end[4];
    gboolean found[4];
    char *ptr = line->string;

    if ((found[0] = gw_regex_locate_boundary_byte_pointers(ptr, " G[0-9]{1,2} ", &start[0], &end[0])))
      line->grade = start[0] + 2;
    else
      line->grade = NULL;

    if ((found[1] =gw_regex_locate_boundary_byte_pointers(ptr, " S[0-9]{1,2} ", &start[1], &end[1])))
      line->strokes = start[1] + 2;
    else
      line->strokes = NULL;

    if ((found[2] =gw_regex_locate_boundary_byte_pointers(ptr, " F[0-9]{1,5} ", &start[2], &end[2])))
      line->frequency = start[2] + 2;
    else
      line->frequency = NULL;

    if ((found[3] = gw_regex_locate_boundary_byte_pointers(ptr, " J[0-9]{1,1} ", &start[3], &end[3])))
      line->jlpt = start[3] + 2;
    else
      line->jlpt = NULL;

    //Get the kanji character
    line->kanji = ptr;
    while (g_utf8_get_char(ptr) != L' ') {
      ptr = g_utf8_next_char(ptr);
    }
    *ptr = '\0';
    ptr++;

    //Test if the radicals information is present
    if(g_utf8_get_char(ptr) > 3040)
    {
      line->radicals = ptr;
      while(g_utf8_get_char(ptr) > 3040 || g_utf8_get_char(ptr) == L' ')
      {
        ptr = g_utf8_next_char(ptr);
      }
      *(ptr - 1) = '\0';
    }

    //Go to the readings section
    while (g_utf8_get_char(ptr) < 3041 && *ptr != '\0')
      ptr = g_utf8_next_char (ptr);
    line->readings[0] = ptr;

    //Copy the rest of the data
    char *next = ptr;
    while (*ptr != '\0' && (next = g_utf8_next_char(ptr)) != NULL && g_utf8_get_char(next) != '{')
    {
      //The strange T1 character between kana readings
      if (g_utf8_get_char (ptr) == L'T' && g_utf8_get_char(next) == L'1') {
        *(ptr - 1) = '\0';
        line->readings[1] = next + 2;
      }
      ptr = next;
    }
    *ptr = '\0';
    line->meanings = next;

    ptr++;
    if ((ptr = g_utf8_strrchr (ptr, -1, '\n')))
      *ptr = '\0';

    if (found[0]) *end[0] = '\0';
    if (found[1]) *end[1] = '\0';
    if (found[2]) *end[2] = '\0';
    if (found[3]) *end[3] = '\0';
}


void gw_resultline_parse_radical_result_string (GwResultLine *line, char *string)
{
    gw_resultline_clear_variables (line);
    strncpy(line->string, string, MAX_LINE);

    //First generate the grade, stroke, frequency, and jplt fields
    line->kanji = line->string;

    char *temp = NULL;

    if (temp = g_utf8_strchr (line->string, -1, L'\n'))
    {
      *temp = '\0';
    }

    if (temp = g_utf8_strchr (line->string, -1, L':'))
    {
       *temp = '\0';
       line->radicals = temp + 1;
    }
}


//!
//! @brief Parses a string for an example format string
//!
//! @param line line
//! @param string string
//!
void gw_resultline_parse_examples_result_string (GwResultLine *line, char *string)
{
    gw_resultline_clear_variables (line);
    strncpy(line->string, string, MAX_LINE);

    //First generate the grade, stroke, frequency, and jplt fields
    line->kanji = line->string;

    char *temp = NULL;
    char *eraser = NULL;
    int i = 0;

    //Example sentence:    A:日本語English:B:読み解説
    temp = line->string;
    while (temp = g_utf8_strchr (temp, -1, L':'))
    {
      //Get the letter bullet 
      if (g_utf8_get_char(temp - 1) == L'A' || g_utf8_get_char(temp - 1) == L'B')
      {
         line->number[i] = temp - 1;
         *temp = '\0';
         temp++;
         temp++;
         line->def_start[i] = temp;
         i++;
         if (eraser = g_utf8_strchr (temp, -1, L'\t'))
         {
             temp = eraser;
             *temp = '\0';
             line->number[i] = temp;
             temp++;
             line->def_start[i] = temp;
             i++;
         }
      }
      //Get the example
      else
      {
         *temp = '\0';
         temp++;
      }
    }
    line->def_start[i] = NULL;
    line->number[i] = NULL;
}

void gw_resultline_parse_unknown_result_string (GwResultLine *line, char *string)
{
    gw_resultline_clear_variables (line);
    strncpy(line->string, string, MAX_LINE);
    char *temp = NULL;
    if (temp = g_utf8_strchr (line->string, -1, L'\n'))
    {
      *temp = '\0';
    }
}
