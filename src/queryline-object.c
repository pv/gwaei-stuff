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
//! @file src/queryline-object.c
//!
//! @brief Currently unused preliminary query object
//!
//! The GwQueryLine object will be used for parsing
//! the query into token so comparisons can be more
//! intelligently handled.
//!

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/queryline-object.h>

#define EFLAGS_EXIST    (REG_EXTENDED | REG_ICASE | REG_NOSUB)
#define EFLAGS_LOCATE   (REG_EXTENDED | REG_ICASE)

//Create the needed regex for searching and locating


GwQueryLine* gw_queryline_new ()
{
    GwQueryLine* temp;

    if ((temp = (GwQueryLine*) malloc(sizeof(struct GwQueryLine))) == NULL) return NULL;

    //A place for a copy of the raw string
    temp->string[0]      = '\0';
    temp->furi_atom[0]   = NULL;
    temp->kanji_atom[0]  = NULL;
    temp->roma_atom[0]   = NULL;
    temp->mix_atom[0]    = NULL;
    return temp;
}

void gw_queryline_free (GwQueryLine *item)
{
    free (item);
}



int gw_queryline_parse_string (GwQueryLine *ql, const char* string)
{
   strncpy(ql->string, string, MAX_QUERY); 

   //Create atoms
   int i = 0;
   char *generic_atoms[MAX_ATOMS];
   generic_atoms[i] = ql->string;
   while ((generic_atoms[i + 1] = g_utf8_strchr (generic_atoms[i], -1, L'&')) != NULL && i < MAX_ATOMS)
   {
     i++;
     *generic_atoms[i] = '\0';
     *generic_atoms[i]++;
   }
   generic_atoms[i + 1] = NULL;

    //Organize atoms
   i = 0;
   int kanji_pos = 0;
   int furi_pos  = 0;
   int mix_pos   = 0;
   int roma_pos  = 0;

   while (generic_atoms[i] != NULL && i < MAX_ATOMS)
   {
     if (gw_util_is_kanji_ish_str (generic_atoms[i]))
     {
       ql->kanji_atom[kanji_pos] = generic_atoms[i];
       if (regcomp (&(ql->kanji_regex                         [GW_QUERYLINE_EXIST] [kanji_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (regcomp (&(ql->kanji_regex                         [GW_QUERYLINE_LOCATE][kanji_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0) return FALSE;
       if (gw_regex_create_kanji_high_regex (&(ql->kanji_regex[GW_QUERYLINE_HIGH]  [kanji_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (gw_regex_create_kanji_med_regex (&(ql->kanji_regex [GW_QUERYLINE_MED]   [kanji_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       kanji_pos++;
     }
     else if (gw_util_is_furigana_str (generic_atoms[i]))
     {
       ql->furi_atom[furi_pos] = generic_atoms[i];
       if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_EXIST] [furi_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_LOCATE][furi_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0)  return FALSE;
       if (gw_regex_create_furi_high_regex (&(ql->furi_regex[GW_QUERYLINE_HIGH]  [furi_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (gw_regex_create_furi_med_regex  (&(ql->furi_regex[GW_QUERYLINE_MED]   [furi_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       furi_pos++;
     }
     else if (gw_util_is_romaji_str (generic_atoms[i]))
     {
       ql->roma_atom[roma_pos] = generic_atoms[i];
       if (regcomp (&(ql->roma_regex                        [GW_QUERYLINE_EXIST] [roma_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (regcomp (&(ql->roma_regex                        [GW_QUERYLINE_LOCATE][roma_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0)  return FALSE;
       if (gw_regex_create_roma_high_regex (&(ql->roma_regex[GW_QUERYLINE_HIGH]  [roma_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (gw_regex_create_roma_med_regex  (&(ql->roma_regex[GW_QUERYLINE_MED]   [roma_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       roma_pos++;
     }
     else
     {
       ql->mix_atom[mix_pos] = generic_atoms[i];
       if (regcomp (&(ql->mix_regex                       [GW_QUERYLINE_EXIST] [mix_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (regcomp (&(ql->mix_regex                       [GW_QUERYLINE_LOCATE][mix_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0) return FALSE;
       if (gw_regex_create_mix_high_regex (&(ql->mix_regex[GW_QUERYLINE_HIGH]  [mix_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (gw_regex_create_mix_med_regex  (&(ql->mix_regex[GW_QUERYLINE_MED]   [mix_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       mix_pos++;
     }
     i++;
   }
   ql->kanji_atom[kanji_pos] = NULL;
   ql->furi_atom[furi_pos]   = NULL;
   ql->roma_atom[roma_pos]   = NULL;
   ql->mix_atom[mix_pos]     = NULL;

   gw_queryline_parse_kanji_string (ql, string);

   return TRUE;
}


int gw_queryline_parse_kanji_string (GwQueryLine *ql, const char* string)
{
    char *ptr = ql->string;
    char *next = NULL;
    gunichar character;

    //Get stroke
    ql->strokes[0] = '\0';
    if (gw_regex_locate_boundary_byte_pointers(ptr, "\\bS[0-4]{1,2}\\b", &ptr, &next))
    {
      strncpy (ql->strokes, ptr + 1, next - ptr);
      ql->strokes[next - ptr] = '\0';
    }
    printf("Strokes: %s\n", ql->strokes);

    //Get Frequency
    ql->frequency[0] = '\0';
    if (gw_regex_locate_boundary_byte_pointers(ptr, "\\bF[0-4]{1,4}\\b", &ptr, &next))
    {
      strncpy (ql->frequency, ptr + 1, next - ptr);
      ql->frequency[next - ptr] = '\0';
    }
    printf("Frequency: %s\n", ql->frequency);

    //Get Grade
    ql->grade[0] = '\0';
    if (gw_regex_locate_boundary_byte_pointers(ptr, "\\bG[0-4]{1,1}\\b", &ptr, &next))
    {
      strncpy (ql->grade, ptr + 1, next - ptr);
      ql->grade[next - ptr] = '\0';
    }
    printf("Grade: %s\n", ql->grade);

    //Get JLPT 
    ql->jlpt[0] = '\0';
    if (gw_regex_locate_boundary_byte_pointers(ptr, "\\bJ[0-4]{1,1}\\b", &ptr, &next))
    {
      strncpy (ql->jlpt, ptr + 1, next - ptr);
      ql->jlpt[next - ptr] = '\0';
    }
    printf("JLPT: %s\n", ql->jlpt);

    //Kanji
    ptr = ql->string;
    int i = 0;
    printf("kanji: ");
    while ((character = g_utf8_get_char(ptr)) != '\0' && i < MAX_ATOMS)
    {
      next = g_utf8_next_char (ptr);
      if (character >= L'ア')
      {
        strncpy (ql->kanji[i], ptr, next - ptr);
        ql->kanji[i][next - ptr] = '\0';
        printf("%s", ql->kanji[i]);
        i++;
      }
      ptr = next;
    }
    printf("\n");
    ql->kanji[i][0] = '\0';

    //copy the hirakana/kanakana search atom
    ql->readings[0] = '\0';
    char exp[1000];
    strcpy(exp, "[(");
    strcat(exp, HIRAGANA);
    strcat(exp, "|");
    strcat(exp, KATAKANA);
    strcat(exp, ")]+");
    ptr = ql->string;
    if (gw_regex_locate_boundary_byte_pointers(ptr, exp, &ptr, &next) && (next - ptr) % 3 == 0)
    {
      strncpy (ql->readings, ptr, next - ptr);
      ql->readings[next - ptr] = '\0';
    }
    printf("readings: %s\n", ql->readings);

    //English
    ql->meanings[0] = '\0';
    ptr = ql->string;
    if (gw_regex_locate_boundary_byte_pointers(ptr, "[A-Za-z][a-z ]{1,20}", &ptr, &next))
    {
      strncpy (ql->meanings, ptr, next - ptr);
      ql->meanings[next - ptr] = '\0';
    }
    printf("english: %s\n", ql->meanings);
}


