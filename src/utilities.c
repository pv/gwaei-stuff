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
//! @file src/utilities.c
//!
//! @brief Generic utility functions
//!
//! Holds some basic functions that are extremely handy for gWaei.
//!


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/utilities.h>

static int run_mode;

void gw_util_initialize_runmode (char* call)
{
   char *call_ptr = &call[strlen(call)];
   while (call_ptr != call && !G_IS_DIR_SEPARATOR (*call_ptr))
     call_ptr--;
   if (G_IS_DIR_SEPARATOR (*call_ptr))
     call_ptr++;

   if (strcmp (call_ptr, "waei") == 0 || strcmp (INTERFACE, "NONE") == 0)
     run_mode = GW_CONSOLE_RUNMODE;
   else if (strcmp (call_ptr, "gwaei") == 0)
     run_mode = GW_GTK_RUNMODE;
   else if (strcmp (call_ptr, "kwaei") == 0)
     run_mode = GW_QT_RUNMODE;
}

int gw_util_get_runmode ()
{
    return run_mode;
}


//A fairly fast int to acsii char funciton
gboolean gw_util_itoa (int input, char *output, const int MAX)
{
  //digit_ptr is used to write and track the number
  char buffer[MAX];
  char *digit_ptr = &buffer[MAX - 1];

  *digit_ptr = '\0';
  //Write out the digits starting at the end of of the buffer
  do
  {
    digit_ptr--;
    *digit_ptr = input % 10;
    *digit_ptr += 48;
    input /= 10;
  } while (input != 0 && digit_ptr != &output[0]);

  //Shift the digits to the beginning of the string
  strcpy(output, digit_ptr);

  //Send return status
  if (input != 0 && digit_ptr == &output[0])
    return FALSE;
  else
    return TRUE;
}

guint gw_util_2digithexstrtoint(char hex_char_digit_2, char hex_char_digit_1)
{
    int digit1;
    if (hex_char_digit_1 <= '9')
      digit1 = hex_char_digit_1 - '0';
    else
      digit1 = hex_char_digit_1 - 'A' + 10;

    int digit2;
    if (hex_char_digit_2 <= '9')
      digit2 = hex_char_digit_2 - '0';
    else 
      digit2 = hex_char_digit_2 - 'A' + 10;

    return ((digit2 << 12) | (digit1 << 8));
}


gboolean gw_util_itohexstr (char *color_string, guint color_integer)
{
    char *color_string_ptr = color_string;
    *color_string_ptr = '#';
    color_string_ptr++;
    color_string_ptr = color_string_ptr + 6;
    *color_string_ptr = '\0';
    color_string_ptr--;

    int color_integer_copy = color_integer;
    int remainder = 0;

    int i = 0;
    while (i < 6)
    {
      remainder = color_integer_copy % 16;
      color_integer_copy = color_integer_copy / 16;

      if (remainder < 10)
        *color_string_ptr = '0' + remainder;
      else
        *color_string_ptr = 'A' + (remainder - 10);
      i++;
      color_string_ptr--;
    }

    return TRUE;
}


//Returns the string where the waei directory should be.  If it doesn't exist, it creates it
char* gw_util_get_waei_directory(char *buffer) 
{
    const char* home = g_get_home_dir ();
    char* directory = g_build_filename (home, ".waei", NULL);
    strcpy (buffer, directory);
    strcat (buffer, G_DIR_SEPARATOR_S);
    g_free (directory);

    //Make sure the ~/.waei directory exits, return NULL on error
    if(g_mkdir_with_parents (buffer, 0755) == -1)
      return NULL;
    else
      return buffer;
}



//Check if characters are romaji, katakana, hiragana, or kanji
gboolean gw_util_all_chars_are_in_range (char input[],
                                         int  start_unic_boundary,
                                         int  end_unic_boundary   )
{
  //Setup
  char *input_ptr;
  input_ptr = input;

  gunichar unic;
  unic = g_utf8_get_char(input_ptr);

  gboolean is_in_boundary;
  is_in_boundary = (unic >= start_unic_boundary && unic <= end_unic_boundary);

  //Check
  while (*input_ptr != '\0' && is_in_boundary)
  {
    input_ptr = g_utf8_next_char(input_ptr);
    unic = g_utf8_get_char(input_ptr);
    is_in_boundary = (unic >= start_unic_boundary && unic <= end_unic_boundary);
  }

  //Return Results
  return (input[0] != '\0' && *input_ptr == '\0');
}


//Convenience functions
gboolean gw_util_is_hiragana_str (char input[])
{
    return gw_util_all_chars_are_in_range (input, L'ぁ', L'ん');
}

gboolean gw_util_is_katakana_str (char input[])
{
    return gw_util_all_chars_are_in_range (input, L'ァ', L'ー');
}

gboolean gw_util_is_kanji_str (char input[])
{
    return gw_util_all_chars_are_in_range (input, L'ー', 0xFF00);
}

gboolean gw_util_is_romaji_str (char input[])
{
    return gw_util_all_chars_are_in_range (input, L'A', L'z');
}


//Shift characters between hiragana and katakana
void gw_util_shift_all_chars_in_str_by (char input[], int shift)
{
    //Setup
    char *input_ptr;
    input_ptr = input;

    char output[MAX_QUERY];
    char *output_ptr;
    output_ptr = output;

    gunichar unic;
    unic = g_utf8_get_char(input_ptr);

    gint offset = 0;

    //Start the conversion
    while (*input_ptr != '\0')
    {
      if (unic == L'ー')
        offset = g_unichar_to_utf8((unic), output_ptr);
      else
        offset = g_unichar_to_utf8((unic + shift), output_ptr);
      output_ptr = output_ptr + offset;

      input_ptr = g_utf8_next_char(input_ptr);
      unic = g_utf8_get_char(input_ptr);
    }
    *output_ptr = *input_ptr;

    strcpy(input, output);
}


//
//Convenience functions
//


void gw_util_str_shift_hira_to_kata (char input[])
{
    gw_util_shift_all_chars_in_str_by (input, (L'ア' - L'あ'));
}

void gw_util_str_shift_kata_to_hira (char input[])
{
    gw_util_shift_all_chars_in_str_by (input, (L'あ' - L'ア'));
}


//
//Functions for managing romaji to hiragana conversions
//


char* gw_util_next_hira_char_from_roma (char *input)
{
    char *input_ptr;
    input_ptr = input;

    int total_n = 0;

    //Cautiously up a character
    if (*input_ptr == 'n')
      total_n++;
    if (*input_ptr != '\0')
      input_ptr++;
    if (*input_ptr == '\0')
      return input_ptr;

    //Loop until we hit a hiragana character ending
    while ( (
            //General conditional for hiragana processing
              *(input_ptr    ) != '\0' &&
              *(input_ptr - 1) != 'a'  &&
              *(input_ptr - 1) != 'i'  &&
              *(input_ptr - 1) != 'u'  &&
              *(input_ptr - 1) != 'e'  &&
              *(input_ptr - 1) != 'o'  &&
              *(input_ptr - 1) != *input_ptr  &&
              *(input_ptr - 1) != '-'  &&
              total_n < 3
            ) ||
            //Special conditional for symbolic n processing
            (
              *(input_ptr    ) != '\0' &&
              *(input_ptr - 1) == 'n'  &&
              *(input_ptr    ) == 'n'  &&
              total_n < 3
            )
          )
    {
      //Update the n count
      if (*(input_ptr - 1) == 'n')
        total_n++;
      else
        total_n = 0;


      if (*(input_ptr - 1) == 'n' &&
              *(input_ptr) != 'a'  &&
              *(input_ptr) != 'i'  &&
              *(input_ptr) != 'u'  &&
              *(input_ptr) != 'e'  &&
              *(input_ptr) != 'o'  &&
              *(input_ptr) != 'y'
         )
        break;

      //Increment
      input_ptr++;
    }

    return input_ptr;
}


char* gw_util_roma_to_hira (char *input, char *output)
{
    //Set up the input pointer
    char *input_ptr;
    input_ptr = input;

    //Make sure the output pointer is initialized
    output[0] = '\0';
    
    //Set up the buffer variables
    char buffer[] = "           ";
    char *buffer_ptr = buffer;

    //Copy the next hiragana char written in romaji to the buffer
    while ( 
            (
              *input_ptr != '\0' &&
              *input_ptr != 'a'  &&
              *input_ptr != 'i'  &&
              *input_ptr != 'u'  &&
              *input_ptr != 'e'  &&
              *input_ptr != 'o'  &&
              *input_ptr != '-'  && 
              *input_ptr != *(input_ptr + 1)  && 
              *(buffer_ptr + 1) == ' '                
            ) 
          )
    {
      *buffer_ptr = *input_ptr;
      input_ptr++;
      buffer_ptr++;
    }
    *buffer_ptr = *input_ptr;
    buffer_ptr++;
    *buffer_ptr = '\0';

    //HACK!!!!!!!!!!!!!!!!
    if (buffer[0] == 'n' &&
              buffer[1] != 'a'  &&
              buffer[1] != 'i'  &&
              buffer[1] != 'u'  &&
              buffer[1] != 'e'  &&
              buffer[1] != 'o'  &&
              buffer[1] != 'y'
       )
    {
       buffer[1] = '\0';
    }
    else if ( buffer[0] != 'a'  &&
              buffer[0] != 'i'  &&
              buffer[0] != 'u'  &&
              buffer[0] != 'e'  &&
              buffer[0] != 'o'  &&
              buffer[0] != 'n'  &&
              strlen(input) == 1
            )
    {
       return NULL;
    }
    //HACK!!!!!!!!!!!!!!!!


    //Reset the buffer pointer
    buffer_ptr = buffer;


    //
    //Start main lookup for conversion
    //

    if (strcmp(buffer_ptr, "n") == 0)
       strcpy(output, "ん");


    else if (strlen(buffer_ptr) == 1 &&
             buffer_ptr[0] != 'a'    &&
             buffer_ptr[0] != 'i'    &&
             buffer_ptr[0] != 'u'    &&
             buffer_ptr[0] != 'e'    &&
             buffer_ptr[0] != 'o'    &&
             buffer_ptr[0] != '-'    &&
             buffer_ptr[0] != 'y'    &&
             input_ptr[1] != '\0'      )
       strcpy(output, "っ");

    else if (strcmp(buffer_ptr, "a") == 0)
       strcpy(output, "あ");
    else if (strcmp(buffer_ptr, "i") == 0)
       strcpy(output, "い");
    else if (strcmp(buffer_ptr, "u") == 0)
       strcpy(output, "う");
    else if (strcmp(buffer_ptr, "e") == 0)
       strcpy(output, "え");
    else if (strcmp(buffer_ptr, "o") == 0)
       strcpy(output, "お");


    else if (strcmp(buffer_ptr, "ka") == 0 || strcmp(buffer_ptr, "ca") == 0)
       strcpy(output, "か");
    else if (strcmp(buffer_ptr, "ki") == 0 || strcmp(buffer_ptr, "ci") == 0)
       strcpy(output, "き");
    else if (strcmp(buffer_ptr, "ku") == 0 || strcmp(buffer_ptr, "cu") == 0)
       strcpy(output, "く");
    else if (strcmp(buffer_ptr, "ke") == 0 || strcmp(buffer_ptr, "ce") == 0)
       strcpy(output, "け");
    else if (strcmp(buffer_ptr, "ko") == 0 || strcmp(buffer_ptr, "co") == 0)
       strcpy(output, "こ");

    else if (strcmp(buffer_ptr, "kya") == 0 || strcmp(buffer_ptr, "cya") == 0)
       strcpy(output, "きゃ");
    else if (strcmp(buffer_ptr, "kyu") == 0 || strcmp(buffer_ptr, "cyu") == 0)
       strcpy(output, "きゅ");
    else if (strcmp(buffer_ptr, "kyo") == 0 || strcmp(buffer_ptr, "cyo") == 0)
       strcpy(output, "きょ");

    else if (strcmp(buffer_ptr, "ga") == 0)
       strcpy(output, "が");
    else if (strcmp(buffer_ptr, "gi") == 0)
       strcpy(output, "ぎ");
    else if (strcmp(buffer_ptr, "gu") == 0)
       strcpy(output, "ぐ");
    else if (strcmp(buffer_ptr, "ge") == 0)
       strcpy(output, "げ");
    else if (strcmp(buffer_ptr, "go") == 0)
       strcpy(output, "ご");

    else if (strcmp(buffer_ptr, "gya") == 0)
       strcpy(output, "ぎゃ");
    else if (strcmp(buffer_ptr, "gyu") == 0)
       strcpy(output, "ぎゅ");
    else if (strcmp(buffer_ptr, "gyo") == 0)
       strcpy(output, "ぎょ");


    else if (strcmp(buffer_ptr, "sa") == 0)
       strcpy(output, "さ");
    else if (strcmp(buffer_ptr, "si") == 0 || strcmp(buffer_ptr, "shi") == 0)
       strcpy(output, "し");
    else if (strcmp(buffer_ptr, "su") == 0)
       strcpy(output, "す");
    else if (strcmp(buffer_ptr, "se") == 0)
       strcpy(output, "せ");
    else if (strcmp(buffer_ptr, "so") == 0)
       strcpy(output, "そ");

    else if (strcmp(buffer_ptr, "sya") == 0 || strcmp(buffer_ptr, "sha") == 0)
       strcpy(output, "しゃ");
    else if (strcmp(buffer_ptr, "syu") == 0 || strcmp(buffer_ptr, "shu") == 0)
       strcpy(output, "しゅ");
    else if (strcmp(buffer_ptr, "syo") == 0 || strcmp(buffer_ptr, "sho") == 0)
       strcpy(output, "しょ");

    else if (strcmp(buffer_ptr, "za") == 0)
       strcpy(output, "ざ");
    else if (strcmp(buffer_ptr, "zi") == 0 || strcmp(buffer_ptr, "ji") == 0)
       strcpy(output, "じ");
    else if (strcmp(buffer_ptr, "zu") == 0)
       strcpy(output, "ず");
    else if (strcmp(buffer_ptr, "ze") == 0)
       strcpy(output, "ぜ");
    else if (strcmp(buffer_ptr, "zo") == 0)
       strcpy(output, "ぞ");

    else if (strcmp(buffer_ptr, "zya") == 0 || strcmp(buffer_ptr, "jya") == 0
                                            || strcmp(buffer_ptr, "ja" ) == 0 )
       strcpy(output, "じゃ");
    else if (strcmp(buffer_ptr, "zyu") == 0 || strcmp(buffer_ptr, "jyu") == 0
                                            || strcmp(buffer_ptr, "ju" ) == 0 )
       strcpy(output, "じゅ");
    else if (strcmp(buffer_ptr, "zyo") == 0 || strcmp(buffer_ptr, "jyo") == 0
                                            || strcmp(buffer_ptr, "jo" ) == 0 )
       strcpy(output, "じょ");


    else if (strcmp(buffer_ptr, "ta") == 0)
       strcpy(output, "た");
    else if (strcmp(buffer_ptr, "ti") == 0 || strcmp(buffer_ptr, "chi") == 0)
       strcpy(output, "ち");
    else if (strcmp(buffer_ptr, "tu") == 0 || strcmp(buffer_ptr, "tsu") == 0)
       strcpy(output, "つ");
    else if (strcmp(buffer_ptr, "te") == 0)
       strcpy(output, "て");
    else if (strcmp(buffer_ptr, "to") == 0)
       strcpy(output, "と");

    else if (strcmp(buffer_ptr, "tya") == 0 || strcmp(buffer_ptr, "cha") == 0)
       strcpy(output, "ちゃ");
    else if (strcmp(buffer_ptr, "tyu") == 0 || strcmp(buffer_ptr, "chu") == 0)
       strcpy(output, "ちゅ");
    else if (strcmp(buffer_ptr, "tyo") == 0 || strcmp(buffer_ptr, "cho") == 0)
       strcpy(output, "ちょ");

    else if (strcmp(buffer_ptr, "da") == 0)
       strcpy(output, "だ");
    else if (strcmp(buffer_ptr, "di") == 0)
       strcpy(output, "ぢ");
    else if (strcmp(buffer_ptr, "du") == 0 || strcmp(buffer_ptr, "dsu") == 0)
       strcpy(output, "づ");
    else if (strcmp(buffer_ptr, "de") == 0)
       strcpy(output, "で");
    else if (strcmp(buffer_ptr, "do") == 0)
       strcpy(output, "ど");

    else if (strcmp(buffer_ptr, "dya") == 0)
       strcpy(output, "ぢゃ");
    else if (strcmp(buffer_ptr, "dyu") == 0)
       strcpy(output, "ぢゅ");
    else if (strcmp(buffer_ptr, "dyo") == 0)
       strcpy(output, "ぢょ");


    else if (strcmp(buffer_ptr, "na") == 0)
       strcpy(output, "な");
    else if (strcmp(buffer_ptr, "ni") == 0)
       strcpy(output, "に");
    else if (strcmp(buffer_ptr, "nu") == 0)
       strcpy(output, "ぬ");
    else if (strcmp(buffer_ptr, "ne") == 0)
       strcpy(output, "ね");
    else if (strcmp(buffer_ptr, "no") == 0)
       strcpy(output, "の");

    else if (strcmp(buffer_ptr, "nya") == 0)
       strcpy(output, "にゃ");
    else if (strcmp(buffer_ptr, "nyu") == 0)
       strcpy(output, "にゅ");
    else if (strcmp(buffer_ptr, "nyo") == 0)
       strcpy(output, "にょ");


    else if (strcmp(buffer_ptr, "ha") == 0)
       strcpy(output, "は");
    else if (strcmp(buffer_ptr, "hi") == 0)
       strcpy(output, "ひ");
    else if (strcmp(buffer_ptr, "hu") == 0 || strcmp(buffer_ptr, "fu") == 0)
       strcpy(output, "ふ");
    else if (strcmp(buffer_ptr, "he") == 0)
       strcpy(output, "へ");
    else if (strcmp(buffer_ptr, "ho") == 0)
       strcpy(output, "ほ");

    else if (strcmp(buffer_ptr, "hya") == 0)
       strcpy(output, "ひゃ");
    else if (strcmp(buffer_ptr, "hyu") == 0)
       strcpy(output, "ひゅ");
    else if (strcmp(buffer_ptr, "hyo") == 0)
       strcpy(output, "ひょ");
   
    else if (strcmp(buffer_ptr, "ba") == 0)
       strcpy(output, "ば");
    else if (strcmp(buffer_ptr, "bi") == 0)
       strcpy(output, "び");
    else if (strcmp(buffer_ptr, "bu") == 0)
       strcpy(output, "ぶ");
    else if (strcmp(buffer_ptr, "be") == 0)
       strcpy(output, "べ");
    else if (strcmp(buffer_ptr, "bo") == 0)
       strcpy(output, "ぼ");

    else if (strcmp(buffer_ptr, "bya") == 0)
       strcpy(output, "びゃ");
    else if (strcmp(buffer_ptr, "byu") == 0)
       strcpy(output, "びゅ");
    else if (strcmp(buffer_ptr, "byo") == 0)
       strcpy(output, "びょ");

    else if (strcmp(buffer_ptr, "pa") == 0)
       strcpy(output, "ぱ");
    else if (strcmp(buffer_ptr, "pi") == 0)
       strcpy(output, "ぴ");
    else if (strcmp(buffer_ptr, "pu") == 0)
       strcpy(output, "ぷ");
    else if (strcmp(buffer_ptr, "pe") == 0)
       strcpy(output, "ぺ");
    else if (strcmp(buffer_ptr, "po") == 0)
       strcpy(output, "ぽ");

    else if (strcmp(buffer_ptr, "pya") == 0)
       strcpy(output, "ぴゃ");
    else if (strcmp(buffer_ptr, "pyu") == 0)
       strcpy(output, "ぴゅ");
    else if (strcmp(buffer_ptr, "pyo") == 0)
       strcpy(output, "ぴょ");


    else if (strcmp(buffer_ptr, "ma") == 0)
       strcpy(output, "ま");
    else if (strcmp(buffer_ptr, "mi") == 0)
       strcpy(output, "み");
    else if (strcmp(buffer_ptr, "mu") == 0)
       strcpy(output, "む");
    else if (strcmp(buffer_ptr, "me") == 0)
       strcpy(output, "め");
    else if (strcmp(buffer_ptr, "mo") == 0)
       strcpy(output, "も");

    else if (strcmp(buffer_ptr, "mya") == 0)
       strcpy(output, "みゃ");
    else if (strcmp(buffer_ptr, "myu") == 0)
       strcpy(output, "みゅ");
    else if (strcmp(buffer_ptr, "myo") == 0)
       strcpy(output, "みょ");


    else if (strcmp(buffer_ptr, "ya") == 0)
       strcpy(output, "や");
    else if (strcmp(buffer_ptr, "yu") == 0)
       strcpy(output, "ゆ");
    else if (strcmp(buffer_ptr, "yo") == 0)
       strcpy(output, "よ");


    else if (strcmp(buffer_ptr, "ra") == 0 || strcmp(buffer_ptr, "la") == 0)
       strcpy(output, "ら");
    else if (strcmp(buffer_ptr, "ri") == 0 || strcmp(buffer_ptr, "li") == 0)
       strcpy(output, "り");
    else if (strcmp(buffer_ptr, "ru") == 0 || strcmp(buffer_ptr, "lu") == 0)
       strcpy(output, "る");
    else if (strcmp(buffer_ptr, "re") == 0 || strcmp(buffer_ptr, "le") == 0)
       strcpy(output, "れ");
    else if (strcmp(buffer_ptr, "ro") == 0 || strcmp(buffer_ptr, "lo") == 0)
       strcpy(output, "ろ");

    else if (strcmp(buffer_ptr, "rya") == 0 || strcmp(buffer_ptr, "lya") == 0)
       strcpy(output, "りゃ");
    else if (strcmp(buffer_ptr, "ryu") == 0 || strcmp(buffer_ptr, "lyu") == 0)
       strcpy(output, "りゅ");
    else if (strcmp(buffer_ptr, "ryo") == 0 || strcmp(buffer_ptr, "lyo") == 0)
       strcpy(output, "りょ");


    else if (strcmp(buffer_ptr, "wa") == 0)
       strcpy(output, "わ");
    else if (strcmp(buffer_ptr, "wi") == 0)
       strcpy(output, "うぃ");
    else if (strcmp(buffer_ptr, "we") == 0)
       strcpy(output, "うぇ");
    else if (strcmp(buffer_ptr, "wo") == 0)
       strcpy(output, "を");

    else if (strcmp(buffer_ptr, "va") == 0)
       strcpy(output, "う゛ぁ");
    else if (strcmp(buffer_ptr, "vi") == 0)
       strcpy(output, "う゛ぃ");
    else if (strcmp(buffer_ptr, "ve") == 0)
       strcpy(output, "う゛ぇ");
    else if (strcmp(buffer_ptr, "vo") == 0)
       strcpy(output, "う゛ぉ");


    else if (strcmp(buffer_ptr, "xa") == 0)
       strcpy(output, "ぁ");
    else if (strcmp(buffer_ptr, "xi") == 0)
       strcpy(output, "ぃ");
    else if (strcmp(buffer_ptr, "xu") == 0)
       strcpy(output, "ぅ");
    else if (strcmp(buffer_ptr, "xe") == 0)
       strcpy(output, "ぇ");
    else if (strcmp(buffer_ptr, "xo") == 0)
       strcpy(output, "ぉ");


    else if (strcmp(buffer_ptr, "fa") == 0)
       strcpy(output, "ふぁ");
    else if (strcmp(buffer_ptr, "fi") == 0)
       strcpy(output, "ふぃ");
    else if (strcmp(buffer_ptr, "fe") == 0)
       strcpy(output, "ふぇ");
    else if (strcmp(buffer_ptr, "fo") == 0)
       strcpy(output, "ふぉ");
   

    else if (strcmp(buffer_ptr, "-") == 0)
       strcpy(output, "ー");

    else return NULL;

    return output;
}


gboolean gw_util_is_japanese_ctype ()
{
    return (setlocale(LC_CTYPE, NULL) != NULL &&
            (
              strcmp(setlocale(LC_CTYPE, NULL), "ja_JP.UTF8")  == 0 ||
              strcmp(setlocale(LC_CTYPE, NULL), "ja_JP.UTF-8") == 0 ||
              strcmp(setlocale(LC_CTYPE, NULL), "ja_JP.utf8")  == 0 ||
              strcmp(setlocale(LC_CTYPE, NULL), "ja_JP.utf-8") == 0 ||
              strcmp(setlocale(LC_CTYPE, NULL), "ja_JP")       == 0 ||
              strcmp(setlocale(LC_CTYPE, NULL), "ja")          == 0 ||
              strcmp(setlocale(LC_CTYPE, NULL), "japanese")    == 0
            )
           );
}


gboolean gw_util_is_japanese_locale()
{
    return ( setlocale(LC_MESSAGES, NULL) != NULL &&
             (
               strcmp(setlocale(LC_MESSAGES, NULL), "ja_JP.UTF8")  == 0 ||
               strcmp(setlocale(LC_MESSAGES, NULL), "ja_JP.UTF-8") == 0 ||
               strcmp(setlocale(LC_MESSAGES, NULL), "ja_JP.utf8")  == 0 ||
               strcmp(setlocale(LC_MESSAGES, NULL), "ja_JP.utf-8") == 0 ||
               strcmp(setlocale(LC_MESSAGES, NULL), "ja_JP")       == 0 ||
               strcmp(setlocale(LC_MESSAGES, NULL), "ja")          == 0 ||
               strcmp(setlocale(LC_MESSAGES, NULL), "japanese")    == 0
             )
           );
}


void gw_util_strncpy_fallback_from_key (char *value, char *key, int n)
{
  if (strcmp (GCKEY_GW_ENGLISH_SOURCE, key) == 0)
    strncpy (value, GW_ENGLISH_URI_FALLBACK, n);
  else if (strcmp (GCKEY_GW_KANJI_SOURCE, key) == 0)
    strncpy (value, GW_KANJI_URI_FALLBACK, n);
  else if (strcmp (GCKEY_GW_NAMES_SOURCE, key) == 0)
    strncpy (value, GW_NAMES_URI_FALLBACK, n);
  else if (strcmp (GCKEY_GW_RADICALS_SOURCE, key) == 0)
    strncpy (value, GW_RADICALS_URI_FALLBACK, n);

  else if (strcmp (GCKEY_GW_MATCH_FG, key) == 0)
    strncpy (value, GW_MATCH_FG_FALLBACK, n);
  else if (strcmp (GCKEY_GW_MATCH_BG, key) == 0)
    strncpy (value, GW_MATCH_BG_FALLBACK, n);
  else if (strcmp (GCKEY_GW_HEADER_FG, key) == 0)
    strncpy (value, GW_HEADER_FG_FALLBACK, n);
  else if (strcmp (GCKEY_GW_HEADER_BG, key) == 0)
    strncpy (value, GW_HEADER_BG_FALLBACK, n);
  else if (strcmp (GCKEY_GW_COMMENT_FG, key) == 0)
    strncpy (value, GW_COMMENT_FG_FALLBACK, n);

  else
    strncpy (value, "", n);
}



