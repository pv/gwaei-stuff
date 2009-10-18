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

#define EFLAGS_EXIST    REG_EXTENDED | REG_ICASE | REG_NOSUB
#define EFLAGS_RELEVANT REG_EXTENDED | REG_ICASE | REG_NOSUB
#define EFLAGS_LOCATE   REG_EXTENDED | REG_ICASE

//Create the needed regex for searching and locating


GwQueryLine* gw_queryline_new ()
{
    GwQueryLine* temp;

    if ((temp = (GwQueryLine*) malloc(sizeof(struct GwQueryLine))) == NULL) return NULL;

    //A place for a copy of the raw string
    temp->string[0] = '\0';
    temp->kata_atom[0]   = NULL;
    temp->hira_atom[0]   = NULL;
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

   //Parse the string

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
   int kata_pos  = 0;
   int hira_pos  = 0;
   int mix_pos   = 0;
   int roma_pos  = 0;

   while (generic_atoms[i] != NULL && i < MAX_ATOMS)
   {
     printf("%s\n", generic_atoms[i]);
     if (gw_util_is_kanji_str (generic_atoms[i]))
     {
       line->kanji_atom[kanji_pos] = generic_atoms[i];
       if (regcomp(&((line->kanji_regex_exist)[kanji_pos]), generic_atoms[i], EFLAGS_EXIST) != 0)  return FALSE;
       if (regcomp(&((line->kanji_regex_locate)[kanji_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0)return FALSE;
       if (!create_kanji_high_regex(&((line->kanji_regex_high)[kanji_pos]), generic_atoms[i], EFLAGS_LOCATE))return FALSE;
       if (!create_kanji_med_regex(&((line->kanji_regex_med)[kanji_pos]), generic_atoms[i], EFLAGS_LOCATE))return FALSE;
       printf("kanji atom: %s\n", line->kanji_atom[kanji_pos]);
       kanji_pos++;
     }
     else if (gw_util_is_hiragana_str (generic_atoms[i]))
     {
       line->hira_atom[hira_pos] = generic_atoms[i];
       if (regcomp(&((line->hira_regex_exist)[hira_pos]), generic_atoms[i], EFLAGS_EXIST) != 0)  return FALSE;
       if (regcomp(&((line->hira_regex_locate)[hira_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0)return FALSE;
       if (!create_katahira_high_regex(&((line->hira_regex_high)[hira_pos]), generic_atoms[i], EFLAGS_LOCATE))return FALSE;
       if (!create_katahira_med_regex(&((line->hira_regex_med)[hira_pos]), generic_atoms[i], EFLAGS_LOCATE))return FALSE;
       printf("hiragana atom: %s\n", line->hira_atom[hira_pos]);
       hira_pos++;
     }
     else if (gw_util_is_katakana_str (generic_atoms[i]))
     {
       line->kata_atom[kata_pos] = generic_atoms[i];
       if (regcomp(&((line->kata_regex_exist)[kata_pos]), generic_atoms[i], EFLAGS_EXIST) != 0)  return FALSE;
       if (regcomp(&((line->kata_regex_locate)[kata_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0)return FALSE;
       if (!create_katahira_high_regex(&((line->kata_regex_high)[kata_pos]), generic_atoms[i], EFLAGS_LOCATE))return FALSE;
       if (!create_katahira_med_regex(&((line->kata_regex_med)[kata_pos]), generic_atoms[i], EFLAGS_LOCATE))return FALSE;
       printf("katakanaatom: %s\n", line->kata_atom[kata_pos]);
       kata_pos++;
     }
     else if (gw_util_is_romaji_str (generic_atoms[i]))
     {
       line->roma_atom[roma_pos] = generic_atoms[i];
       if (regcomp(&((line->roma_regex_exist)[roma_pos]), generic_atoms[i], EFLAGS_EXIST) != 0)  return FALSE;
       if (regcomp(&((line->roma_regex_locate)[roma_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0)return FALSE;
       if (!create_roma_high_regex(&((line->roma_regex_high)[roma_pos]), generic_atoms[i], EFLAGS_LOCATE))return FALSE;
       if (!create_roma_med_regex(&((line->roma_regex_med)[roma_pos]), generic_atoms[i], EFLAGS_LOCATE))return FALSE;
       printf("romaji atom: %s\n", line->roma_atom[roma_pos]);
       roma_pos++;
     }
     else
     {
       line->mix_atom[mix_pos] = generic_atoms[i];
       if (regcomp(&((line->mix_regex_exist)[mix_pos]), generic_atoms[i], EFLAGS_EXIST) != 0)  return FALSE;
       if (regcomp(&((line->mix_regex_locate)[mix_pos]), generic_atoms[i], EFLAGS_LOCATE) != 0)return FALSE;
//       if (!create_mix_high_regex(&((line->mix_regex_high)[mix_pos]), generic_atoms[i], EFLAGS_LOCATE))return FALSE;
//       if (!create_mix_med_regex(&((line->mix_regex_med)[mix_pos]), generic_atoms[i], EFLAGS_LOCATE))return FALSE;
       printf("mix: %s\n", line->mix_atom[mix_pos]);
       mix_pos++;
     }
     i++;
   }
   line->kanji_atom[kanji_pos] = NULL;
   line->hira_atom[hira_pos]   = NULL;
   line->kata_atom[kata_pos]   = NULL;
   line->roma_atom[roma_pos] = NULL;
   line->mix_atom[mix_pos]     = NULL;
}



gboolean create_kanji_high_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy(expression, "((^)|(\\[)|(\\()|(\\{)|( )|(お)|(を)|(に)|(で)|(は)|(と))(");
    strcat(expression, string);
    strcat(expression, ")((で)|(が)|(の)|(を)|(に)|(で)|(は)|(と)|(\\])|(\\))|(\\})|( ))");
    return (regcomp(regex, expression, flags) != 0);
}

gboolean create_katahira_high_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy(expression, "((^)|(\\[)|(\\()|(\\{)|( )|(^お))(");
    strcat(expression, string);
    strcat(expression, ")((\\])|(\\))|(\\})|( ))");
    return (regcomp(regex, expression, flags) != 0);
}



gboolean create_roma_high_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy(expression, "(\\b(");
    strcat(expression, string);
    strcat(expression, ")\\b|^(");
    strcat(expression, string);
    strcat(expression, "))");
    return (regcomp(regex, expression, flags) != 0);
}

gboolean create_kanji_med_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy(expression, "((^無)|(^不)|(^非)|(^)|(^お)|(^御))(");
    strcat(expression, string);
    strcat(expression, ")((\\])|(\\))|(\\})|( ))");
    return (regcomp(regex, expression, flags) != 0);
}


gboolean create_katahira_med_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy(expression, "((^)|(\\[)|(\\()|(\\{)|( )|(^お))(");
    strcat(expression, string);
    strcat(expression, ")((\\])|(\\))|(\\})|( ))");
    return (regcomp(regex, expression, flags) != 0);
}


gboolean create_roma_med_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy(expression, "\\{(");
    strcat(expression, string);
    strcat(expression, ")\\}|(\\) |/)((to )|(to be )|())(");
    strcat(expression, string);
    strcat(expression, ")(( \\([^/]+\\)/)|(/))|(\\[)(");
    strcat(expression, string);
    strcat(expression, ")(\\])|^(");
    strcat(expression, string);
    strcat(expression, ")\\b");
    return (regcomp(regex, expression, flags) != 0);
}


//!
//! @brief Comparison function that should be moved to the GwSearchItem file when it matures
//!
//! @param item A GwSearchItem to get search information from
//!
gboolean gw_searchitem_generic_comparison (GwSearchItem *item;)
{
    int i;
    int j;
    GwResultLine *rl, *ql;
    rl = item->resultline;
    ql = item->queryline;

    //Compare kanji atoms
    i = 0;
    while (ql->kanji_atom[i])
    {
      if (regexec(&(ql->kanji_regex_high[i]), rl->kanji_start, 1, NULL, 0) == 0)
        return TRUE;
      i++;  
    }
    //Compare furigana atoms
    i = 0;
    while (ql->hira_atom[i])
    {
      if (regexec(&(ql->hira_regex_high[i]), rl->furigana_start, 1, NULL, 0) == 0)
        return TRUE;
      else if (regexec(&(ql->kata_regex_high[i]), rl->furigana_start, 1, NULL, 0) == 0)
        return TRUE;
      i++;  
    }
    //Compare romaji atoms
    i = 0;
    while (ql->roma_atom[i])
    {
      j = 0;
      while (rl->def_start[j])
      {
        if (regexec(&(ql->roma_regex_high[i]), rl->def_start[j], 1, NULL, 0) == 0)
          return TRUE;
        j++;
      }
      i++;  
    }
    //Compare mix atoms
    i = 0;
    while (ql->mix[i])
    {
      if (regexec(&(ql->mix_regex_high[i]), rl->string, 1, NULL, 0) == 0)
        return TRUE;
      i++;  
    }

    return FALSE;
}

