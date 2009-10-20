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



int gw_queryline_parse_string (GwQueryLine *line, const char* string)
{
   strncpy(line->string, string, MAX_QUERY); 

   //Create atoms
   int i = 0;
   char *generic_atoms[MAX_ATOMS];
   generic_atoms[i] = line->string;
   while ((generic_atoms[i + 1] = g_utf8_strchr (generic_atoms[i], -1, L'&')) && i < MAX_ATOMS)
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
       line->kanji_atom[kanji_pos] = generic_atoms[i];
       if (regcomp (&(line->kanji_regex                         [GW_QUERYLINE_EXIST] [kanji_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (regcomp (&(line->kanji_regex                         [GW_QUERYLINE_LOCATE][kanji_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0) return FALSE;
       if (gw_regex_create_kanji_high_regex (&(line->kanji_regex[GW_QUERYLINE_HIGH]  [kanji_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (gw_regex_create_kanji_med_regex (&(line->kanji_regex [GW_QUERYLINE_MED]   [kanji_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       kanji_pos++;
     }
     else if (gw_util_is_furigana_str (generic_atoms[i]))
     {
       line->furi_atom[furi_pos] = generic_atoms[i];
       if (regcomp (&(line->furi_regex                        [GW_QUERYLINE_EXIST] [furi_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (regcomp (&(line->furi_regex                        [GW_QUERYLINE_LOCATE][furi_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0)  return FALSE;
       if (gw_regex_create_furi_high_regex (&(line->furi_regex[GW_QUERYLINE_HIGH]  [furi_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (gw_regex_create_furi_med_regex  (&(line->furi_regex[GW_QUERYLINE_MED]   [furi_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       furi_pos++;
     }
     else if (gw_util_is_romaji_str (generic_atoms[i]))
     {
       line->roma_atom[roma_pos] = generic_atoms[i];
       if (regcomp (&(line->roma_regex                        [GW_QUERYLINE_EXIST] [roma_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (regcomp (&(line->roma_regex                        [GW_QUERYLINE_LOCATE][roma_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0)  return FALSE;
       if (gw_regex_create_roma_high_regex (&(line->roma_regex[GW_QUERYLINE_HIGH]  [roma_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (gw_regex_create_roma_med_regex  (&(line->roma_regex[GW_QUERYLINE_MED]   [roma_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       roma_pos++;
     }
     else
     {
       line->mix_atom[mix_pos] = generic_atoms[i];
       if (regcomp (&(line->mix_regex                       [GW_QUERYLINE_EXIST] [mix_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (regcomp (&(line->mix_regex                       [GW_QUERYLINE_LOCATE][mix_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0) return FALSE;
       if (gw_regex_create_mix_high_regex (&(line->mix_regex[GW_QUERYLINE_HIGH]  [mix_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (gw_regex_create_mix_med_regex  (&(line->mix_regex[GW_QUERYLINE_MED]   [mix_pos]), generic_atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       mix_pos++;
     }
     i++;
   }
   line->kanji_atom[kanji_pos] = NULL;
   line->furi_atom[furi_pos]   = NULL;
   line->roma_atom[roma_pos]   = NULL;
   line->mix_atom[mix_pos]     = NULL;
   return TRUE;
}


