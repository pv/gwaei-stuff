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
//! @file src/formatting.c
//!
//! @brief Adds general text formatting to strings
//!
//! Functions to add proper formatting to results/queries depending on the
//! context.
//!


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <libintl.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/dictionaries.h>
#include <gwaei/history.h>
#include <gwaei/interface.h>
#include <gwaei/preferences.h>



//!
//! @brief Searches for a regex and returns pointers to matches
//!
//! THIS IS A PRIVATE FUNCTION. Function takes searches for the pattern in a
//! a string and then returns pointers to the beginning and end of matches.
//!
//! @param string a constant string to be searched
//! @param pattern a pattern string to search for
//! @param start a character pointer array for match starts points
//! @param end a character pointer array for match end points
//!
static gboolean locate_boundary_byte_pointers( const char *string, char  *pattern,
                                               char      **start,  char **end      )
{
  regex_t re;
  int status;
  int eflags = REG_EXTENDED | REG_ICASE;
 
  if ((status = regcomp(&re, pattern, eflags)) == 0)
  {
    size_t nmatch = 1;
    regmatch_t pmatch[nmatch];
    if ((status = regexec(&re, string, nmatch, pmatch, 0)) == 0)
    {
      *start = (char*) (string + pmatch[0].rm_so);
      *end = (char*) (string + pmatch[0].rm_eo);
    }
    regfree(&re);
  }
  return !status; 
} 


//!
//! @brief Copies a string while adding some special formatting
//!
//! The formatting added will be to decide of the word will have have hiragara
//! and katakana variations of it searched and if four kanji woulds have the
//! two kanji pieces also searched.
//!
//! @param output Character array the formatting string is copied to
//! @param output Character array to format
//! @param item GwSearchItem to get misc data from
//!
gboolean strcpy_with_query_preformatting (char* output, char* input, GwSearchItem *item)
{
    char buffer[MAX_QUERY];
    strncpy(buffer, input, MAX_QUERY);

    //Load the preformatting preferences from pref
    gboolean hira_kata_conv_pref;
    hira_kata_conv_pref = gw_pref_get_boolean (GCKEY_GWAEI_HIRA_KATA, TRUE);
      
    gboolean kata_hira_conv_pref;
    kata_hira_conv_pref = gw_pref_get_boolean (GCKEY_GWAEI_KATA_HIRA, TRUE);

    int roman_kana_conv_pref;
    roman_kana_conv_pref = gw_pref_get_int (GCKEY_GWAEI_ROMAN_KANA, 2);

    //Load the preformatting preferences from pref
    if (item->dictionary->type == KANJI || item->dictionary->type == RADICALS)
    {
      strcpy(output, buffer);
      return FALSE;
    }
  
    //Hiragana query preprocessing
    else if (hira_kata_conv_pref == TRUE && gw_util_is_hiragana_str(buffer))
    {
      char hira[MAX_QUERY], kata[MAX_QUERY];
      strcpy(hira, buffer);
      strcpy(kata, buffer);
      gw_str_shift_hiragana_to_katakana(kata);

      int leftover;
      leftover = MAX_QUERY;
      strncpy(output, "(", leftover); 
      leftover -= 1;
      strncat(output, hira, leftover);
      leftover -= strlen(hira);
      strncat(output, ")|(", leftover);
      leftover -= 3;
      strncat(output, kata, leftover);
      leftover -= strlen(kata);
      strncat(output, ")", leftover); 

      return TRUE;
    }

    //Katakana query preprocessing
    else if (kata_hira_conv_pref == TRUE && gw_util_is_katakana_str(buffer))
    {
      char hira[MAX_QUERY], kata[MAX_QUERY];
      strcpy(hira, buffer);
      strcpy(kata, buffer);
      gw_str_shift_katakana_to_hiragana(hira);

      int leftover;
      leftover = MAX_QUERY;
      strncpy(output, "(", leftover); 
      leftover -= 1;
      strncat(output, kata, leftover);
      leftover -= strlen(kata);
      strncat(output, ")|(", leftover);
      leftover -= 3;
      strncat(output, hira, leftover);
      leftover -= strlen(hira);
      strncat(output, ")", leftover); 

      return TRUE;
    }

    //Kanji 四字熟語 query preprocessing
    else if (gw_util_is_kanji_str(buffer) && g_utf8_strlen(buffer, -1) == 4)
    {
      char first_half[20];
      char second_half[20];

      char *middle = g_utf8_next_char(g_utf8_next_char(buffer));

      //Split the query into two equal halves
      char *src_ptr = buffer;
      char *dest_ptr = first_half;
      while(src_ptr != middle)
      {
        *dest_ptr = *src_ptr;
        src_ptr++;
        dest_ptr++;
      }
      *dest_ptr = '\0';
            
      src_ptr = middle;
      dest_ptr = second_half;
      while(*src_ptr != '\0')
      {
        *dest_ptr = *src_ptr;
        src_ptr++;
        dest_ptr++;
      }
      *dest_ptr = '\0';

      strcpy(output, "(");
      strcat(output, first_half);
      strcat(output, second_half);
      strcat(output, ")|(");
      strcat(output, first_half);
      strcat(output, ")|(");
      strcat(output, second_half);
      strcat(output, ")");

      return TRUE;
    }

    //Romanji/other query preprocessing
    else if (
             (roman_kana_conv_pref == 0                                 ) || 
             (roman_kana_conv_pref == 2 && is_japanese_locale() == FALSE)
            )
    {
      char *input_ptr = buffer;
      char kana[MAX_QUERY];
      char *kana_ptr = kana;
      *kana_ptr = '\0';

      int leftover;
      leftover = MAX_QUERY;
      while (leftover-- > 0)
      {
        kana_ptr = gw_romaji_to_hiragana (input_ptr, kana_ptr);
        if (kana_ptr == NULL || input_ptr == NULL)
          break;

        input_ptr = gw_next_hiragana_char_from_romaji (input_ptr);
        if (kana_ptr == NULL || input_ptr == NULL)
          break;

        kana_ptr = &kana_ptr[strlen(kana_ptr)];
      }

      gboolean success;
      success = (input_ptr != NULL && strlen(input_ptr) == 0);

      //On success, copy the string to the end of the query
      if (success == TRUE)
      {
        leftover = MAX_QUERY;
        strncpy(output, "(", leftover);
        leftover -= 1;
        strncat(output, buffer, leftover);
        leftover -= strlen(buffer);
        strncat(output, ")|(", leftover);
        leftover -= 3;
        //Add a hiragana version
        strncat(output, kana, leftover);
        leftover -= strlen(kana);

        //Add a katakana version
        strncat(output, ")|(", leftover);
        leftover -= 3;

        gw_str_shift_hiragana_to_katakana(kana);
        strncat(output, kana, leftover);
        leftover -= strlen(kana);
        strncat(output, ")",  leftover);

        return TRUE;
      }
    }

    //No conversions were necissary
    strcpy(output, input);
    return FALSE;
}


//!
//! @brief Copies a string while adding some special formatting
//!
//! This function parses a string, adding delimiters for search atoms and then
//! writes the edited string to the output.
//!
//! @param output Character array the formatting string is copied to
//! @param output Character array to format
//! @param item GwSearchItem to get misc data from
//!
void strcpy_with_query_formatting (char* output, char* input, GwSearchItem *item)
{
    //Searching in the kanji sidebar only look for a matching first character
    if (item->target == GWAEI_TARGET_KANJI)
    {
      strcpy(output, "^(");
      strcat(output, input);
      strcat(output, ")");
      strcat(output, DELIMITOR_STR);
    }

    //General Radical and kanji searches look for every single atom separated by
    //the delimitor
    else if (item->dictionary->type == KANJI || item->dictionary->type == RADICALS)
    {
      //Radical and kanji searches don't use regex
      //so the search should be cleaned before sending.

      char *output_ptr = &output[0];

      //s = start
      char *s = NULL;
      //e = end
      char *e = NULL;

      s = &input[0];

      //copy the Kanji characters
      while(*s != '\0')
      {
        if (g_utf8_get_char(s) > L'ン') { // 0x30A1 = 'ァ'
          e = g_utf8_next_char(s);
          while (s != e) {
            *output_ptr = *s;
            s++;
            output_ptr++;
          }
          *output_ptr = DELIMITOR_CHR;
          output_ptr++;
        }
        else {
          s = g_utf8_next_char(s);
        }
      }
      *output_ptr = '\0';

      //copy the Grade search atom
      if (locate_boundary_byte_pointers(input, "G[0-9]{1,2}", &s, &e))
      {
        *output_ptr = ' ';
        output_ptr++;
        while(s != e)
        {
          *output_ptr = *s;
          output_ptr++;
          s++;
        }
        *output_ptr = ' ';
        output_ptr++;
        *output_ptr = DELIMITOR_CHR;
        output_ptr++;
      }
      *output_ptr = '\0';

      //copy the Stroke search atom
      if (locate_boundary_byte_pointers(input, "S[0-9]{1,2}", &s, &e))
      {
        *output_ptr = ' ';
        output_ptr++;
        while(s != e)
        {
          *output_ptr = *s;
          output_ptr++;
          s++;
        }
        *output_ptr = ' ';
        output_ptr++;
        *output_ptr = DELIMITOR_CHR;
        output_ptr++;
      }
      *output_ptr = '\0';

      //copy the Frequency search atom
      if (locate_boundary_byte_pointers(input, "F[0-9]{1,8}", &s, &e))
      {
        *output_ptr = ' ';
        output_ptr++;
        while(s != e)
        {
          *output_ptr = *s;
          output_ptr++;
          s++;
        }
        *output_ptr = ' ';
        output_ptr++;
        *output_ptr = DELIMITOR_CHR;
        output_ptr++;
      }
      *output_ptr = '\0';

      //copy the JLPT search atom
      if (locate_boundary_byte_pointers(input, "J[0-4]", &s, &e)){
        *output_ptr = ' ';
        output_ptr++;
        while(s != e)
        {
          *output_ptr = *s;
          output_ptr++;
          s++;
        }
        *output_ptr = ' ';
        output_ptr++;
        *output_ptr = DELIMITOR_CHR;
        output_ptr++;
      }
      *output_ptr = '\0';

      //copy the English search atom
      if (locate_boundary_byte_pointers(input, "[A-Za-z][a-z ]{1,20}", &s, &e))
      {
        while(s != e)
        {
          *output_ptr = *s;
          output_ptr++;
          s++;
        }
        *output_ptr = DELIMITOR_CHR;
        output_ptr++;
      }

      //Finalize the string
      *output_ptr = '\0';

      //copy the hirakana/kanakana search atom
      char exp[1000];
      strcpy(exp, "[(");
      strcat(exp, HIRAGANA);
      strcat(exp, "|");
      strcat(exp, KATAKANA);
      strcat(exp, ")]+");
      if (locate_boundary_byte_pointers(input, exp, &s, &e) && (e - s) >= 3 )
      {
        while(s != e && s != '\0')
        {
          *output_ptr = *s;
          output_ptr++;
          s++;
        }
        *output_ptr = DELIMITOR_CHR;
        output_ptr++;
      }

      //Finalize the string
      *output_ptr = '\0';
    }


    //Query setup for general searches
    else
    {
      //Copy the string to output
      strcpy(output, input);
     
      //Truncate at the first possible user delimitor
      char* user_delimitor_ptr = &output[0]; 
      while (*user_delimitor_ptr != '\0' && *user_delimitor_ptr != DELIMITOR_CHR)
      {
        if (*user_delimitor_ptr == '&')
          *user_delimitor_ptr = DELIMITOR_CHR;
        user_delimitor_ptr++;
      }
      *user_delimitor_ptr = '\0'; 

      //convert any '&' symbols to delimitors
      user_delimitor_ptr = &output[0]; 
      while (*user_delimitor_ptr != '\0')
      {
        if (*user_delimitor_ptr == '&')
          *user_delimitor_ptr = DELIMITOR_CHR;
        user_delimitor_ptr++;
      }
      *user_delimitor_ptr = '\0'; 
      
      //Add our own delimitor
      strcat(output, DELIMITOR_STR);
    }
}


//!
//! @brief Copies a string while adding some special formatting
//!
//! Adds the formatting to a returned search result so it becomes
//! more readable at the output.
//!
//! @param output Character array the formatting string is copied to
//! @param output Character array to format
//! @param item GwSearchItem to get misc data from
//!
void strcpy_with_general_formatting(char *output, char *input, GwSearchItem *item) 
{
    char *input_ptr = &input[0];
    char *output_ptr = &output[0];
    while(*input_ptr != DELIMITOR_CHR && *input_ptr != '\0')
    {
      *output_ptr = *input_ptr;
      input_ptr++;
      output_ptr++;
    }

    while(*input_ptr != '\0')
    {
      *output_ptr = *input_ptr;
      input_ptr++;
      output_ptr++;
    }

    *output_ptr = '\n';
    output_ptr++;
    *output_ptr = '\0';
}


//!
//! @brief Copies a string while adding some special formatting
//!
//! Adds the formatting to a returned search result so it becomes
//! more readable at the output.  This version is used for kanji dictionary
//! searches.
//!
//! @param output Character array the formatting string is copied to
//! @param output Character array to format
//! @param item GwSearchItem to get misc data from
//!
void strcpy_with_kanji_formatting(char *output, char *input, GwSearchItem *item)
{
    //First generate the grade, stroke, frequency, and jplt fields
    char *start, *end;

    char grade[50]     = "";
    char stroke[50]    = "";
    char frequency[50] = "";
    char jplt[50]      = "";

    if (locate_boundary_byte_pointers(input, " G[0-9]{1,2} ", &start, &end))
    {
      strcat(grade, gettext("Grade:"));
      strncat(grade, start + 2, (end - start - 2));
    }

    if (locate_boundary_byte_pointers(input, " S[0-9]{1,2} ", &start, &end))
    {
      strcat(stroke, gettext("Stroke:"));
      strncat(stroke, start + 2, (end - start - 2));
    }

    if (locate_boundary_byte_pointers(input, " F[0-9]{1,5} ", &start, &end))
    {
      strcat(frequency, gettext("Freq:"));
      strncat(frequency, start + 2, (end - start - 2));
    }

    if (locate_boundary_byte_pointers(input, " J[0-9]{1,1} ", &start, &end))
    {
      strcat(jplt, gettext("JLPT:"));
      strncat(jplt, start + 2, (end - start - 2));
    }

    //Prepare to write the output

    char *input_ptr  = &input[0];
    char *output_ptr = &output[0];
    
    //Copy the kanji character
    while (*input_ptr != ' ') {
      *output_ptr = *input_ptr;
      output_ptr++;
      input_ptr++;
    }
    *output_ptr = ' ';
    output_ptr++;

    *output_ptr = '\n';
     output_ptr++;
    *output_ptr = '\0';

    input_ptr++;
    //Test if the radicals information is present
    if(g_utf8_get_char(input_ptr) > 3040)
    {
      strcat(output, gettext("Radicals:"));
      output_ptr = &output[strlen(output)];
      while(g_utf8_get_char(input_ptr) > 3040 || g_utf8_get_char(input_ptr) == L' ')
      {
        *output_ptr = *input_ptr;
        output_ptr++;
        input_ptr++;
      }
      *output_ptr = '\n';
      output_ptr++;
    }
    *output_ptr = '\0';

    //Copy the prepared info blobs
    strcat(output, grade);

    if (strlen(grade) > 1 && strlen(stroke) > 1)
      strcat(output, " ");
    strcat(output, stroke);

    if ((strlen(grade) > 1 || strlen(stroke) > 1) && strlen(frequency) > 1)
      strcat(output, " ");
    strcat(output, frequency);

    if ((strlen(grade) > 1 || strlen(stroke) > 1 || strlen(frequency) > 1) && strlen(jplt) > 1)
      strcat(output, " ");
    strcat(output, jplt);

    strcat(output, "\n");
    strcat(output, gettext("Readings:"));

    //correct the positioning of the pointers
    while (g_utf8_get_char(input_ptr) < 3041 && *input_ptr != '\0')
      input_ptr++;
    while (*output_ptr != '\0')
      output_ptr++;

    //Copy the rest of the data
    while (*input_ptr != '\0') {
      *output_ptr = *input_ptr;
      output_ptr++;
      input_ptr++;
      
      //The strange T1 character between kana readings
      if (*input_ptr == 'T' && *(input_ptr + 1) == '1') {
        input_ptr = input_ptr + 3;
      }
      //Add a carrage return before the english translations
      else if (g_utf8_get_char(g_utf8_prev_char(input_ptr)) >= 3041 && *(input_ptr + 1) == '{')
      {
        *output_ptr = '\0';
        strcat(output, "\n");
        strcat(output, gettext("Meanings:"));
        output_ptr = output_ptr + strlen(gettext("Meanings:")) + 1;
        input_ptr++;
      }
    }
    *output_ptr= '\n';
    output_ptr++;
    *output_ptr= '\0';
}


//!
//! @brief Adds group formatting to a string
//!
//! If this function detects similar kanji between strings, it will over write
//! the kanji with spaces and remove vertical white space between the results.
//!
//! @param item GwSearchItem to get the result to add formatting to
//!
void add_group_formatting (GwSearchItem* item)
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

    //Code to remove duplicate kanji in the beginning of a result /////
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


