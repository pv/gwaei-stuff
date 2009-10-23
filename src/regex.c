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
//!  @file src/regex.c
//!
//!  @brief Compiled often used regex expressions
//!
//!  Functions that deal with regex operations. This is also where constant
//!  regrexs are stored to improve search speed.
//!


#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>


regex_t re_english;
regex_t re_radical;
regex_t re_kanji;
regex_t re_places;
regex_t re_names;
regex_t re_mix;
regex_t re_gz;
regex_t re_hexcolor;


regex_t re_i_adj_pastform;
regex_t re_i_adj_negative;
regex_t re_i_adj_te_form;
regex_t re_i_adj_causative;
regex_t re_i_adj_conditional;
regex_t re_na_adj_pastform;
regex_t re_na_adj_negative;
regex_t re_na_adj_te_form;
regex_t re_na_adj_causative;
regex_t re_na_adj_conditional;


//!
//! @brief Initializes rebuild often used prebuilt regex expressions
//!
//! These are mostly dictionary names where we want to ignore things like case.
//!
void gw_regex_initialize_constant_regular_expressions ()
{
    if (regcomp (&re_english, "English", GW_REGEX_EFLAGS_EXIST) != 0)
      printf ("A problem occured while setting the regular expression for English\n");
    if (regcomp (&re_radical, "Radical", GW_REGEX_EFLAGS_EXIST) != 0)
      printf ("A problem occured while setting the regular expression for Radical\n");
    if (regcomp (&re_kanji, "Kanji", GW_REGEX_EFLAGS_EXIST) != 0)
      printf ("A problem occured while setting the regular expression for Kanji\n");
    if (regcomp (&re_names, "Names", GW_REGEX_EFLAGS_EXIST) != 0)
      printf ("A problem occured while setting the regular expression for Names\n");
    if (regcomp (&re_places, "Places", GW_REGEX_EFLAGS_EXIST) != 0)
      printf ("A problem occured while setting the regular expression for Places\n");
    if (regcomp (&re_mix, "Mix", GW_REGEX_EFLAGS_EXIST) != 0)
      printf ("A problem occured while setting the regular expression for Mix\n");
    if (regcomp (&re_gz, "\\.gz", GW_REGEX_EFLAGS_EXIST) != 0)
      printf ("A problem occured while setting the regular expression for .gz\n");
    if (regcomp (&re_hexcolor, "^#[0-9A-Fa-f]{6,6}$", GW_REGEX_EFLAGS_EXIST) != 0)
      printf ("A problem occured while setting the regular expression for hexcolor\n");
    if (regcomp (&re_hexcolor, "^#[0-9A-Fa-f]{6,6}$", GW_REGEX_EFLAGS_EXIST) != 0)
      printf ("A problem occured while setting the regular expression for hexcolor\n");

    //Adjective forms
    regcomp (&re_i_adj_past,         "\\B((かった))$",              GW_REGEX_EFLAGS_LOCATE);
    regcomp (&re_i_adj_negative,     "\\B((くない))$",              GW_REGEX_EFLAGS_LOCATE);
    regcomp (&re_i_adj_te,           "\\B((くて))$",                GW_REGEX_EFLAGS_LOCATE);
    regcomp (&re_i_adj_causative,    "\\B((くさせる))$",            GW_REGEX_EFLAGS_LOCATE);
    regcomp (&re_i_adj_conditional,  "\\B((ければ))$",              GW_REGEX_EFLAGS_LOCATE);

    regcomp (&re_na_adj_past,        "\\B((だった))$",              GW_REGEX_EFLAGS_LOCATE);
    regcomp (&re_na_adj_negative,    "\\B((ではない)|(じゃない))$", GW_REGEX_EFLAGS_LOCATE);
    regcomp (&re_na_adj_te,          "\\B((で))$",                  GW_REGEX_EFLAGS_LOCATE);
    regcomp (&re_na_adj_causative,   "\\B((にさせる))$",            GW_REGEX_EFLAGS_LOCATE);
    regcomp (&re_na_adj_conditional, "\\B((であれば))$",            GW_REGEX_EFLAGS_LOCATE);


    //Verb forms
/*
    regcomp (&re_verb_presentform, "\\B((ます))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (&re_verb_politepast, "\\B((ました))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (&re_verb_pastform_negative, "\\B((なかった))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (&re_verb_pastform, "\\B((った)|(いた)|(いだ)|(した)|(んだ)|(えた))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (&re_verb_negative, "\\B((わない)|(かない)|(がない)|(さない)|(たない)|(なない)|(まない)|(いない))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (&re_verb_te_form, "\\B((って)|(いて)|(いで)|(して)|(んで))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (&re_verb_potention, "\\B((える)|(ける)|(げる)|(せる)|(てる)|(ねる)|(べる)|(める)|(れる)|(いられる)|(えられる)|(いれる))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (&re_verb_causative, "\\B((させる)|(わせる)|(かせる)|(がせる)|(なせる)|(たせる)|(ばせる)|ませる(らせる)|(いさせる)|())$", GW_REGEX_EFLAGS_EXIST);
    regcomp (&re_conditional, "\\B((すれば)|(くれば)|(であれば)|(えば)|(けば)|(げば)|(せば)|(てば)|(ねば)|(べば)|(めば)|(れば)|(いれば)|(れば))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (&re_negative_conditional, "\\B((なければ))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (&re_verb_imperative, "\\B((しろ)|(せよ)|(こい)|(くれ)|(ませ)|(であれ)|(え)|(け)|(せ)|(て)|(ね)|(べ)|(め)|(れ)|(いろ)|(えろ))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (&re_verb_passive, "\\B((される)|(こられる)|(われる)|(かれる)|(がれる)|(される)|(たれる)|(なれる)|(ばれる)|(まれる)|(られる)|(いられる)|(えられる))$", GW_REGEX_EFLAGS_EXIST);
    regcomp (&re_verb_volitional, "\\B((しよう)|(せよう)|(こよう)|(だろう)|(ましょう)|(おう)|(こう)|(ごう)|(そう)|(とう)|(のう)|(ぼう)|(もう)|(ろう)|(いよう)|(よう))$", GW_REGEX_EFLAGS_EXIST);
*/
}


//!
//! @brief Builds a regex for a pattern and returns pointers to matches
//!
//! Function takes searches for the pattern in a string and then returns
//! pointers to the beginning and end of matches. As is expected, this kind of search
//! should be avoided when possible because it is slow.
//!
//! @param string a constant string to be searched
//! @param pattern a pattern string to search for
//! @param start a character pointer array for match starts points
//! @param end a character pointer array for match end points
//! @return Returns truE when pattern is found.
//!
gboolean gw_regex_locate_boundary_byte_pointers (const char *string, char  *pattern,
                                                 char      **start,  char **end     )
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
//! @brief A function used to quickly locate the position of a pattern in a haystack
//!
//! Mostly this function is used for located where matches are a string so the approprate
//! highlighting can be applied to them.
//!
//! @param string the current position in the line
//! @param line_start the start of the line to calculate the offset against
//! @param re_locate A compiled regex to use
//! @param start the start offset
//! @param start the end offset
//! @return Returns a new position in the line after the match
//!
char* gw_regex_locate_offset (char *string, char *line_start, regex_t *re_locate,
                              gint *start,  gint    *end                         )

{
    if (string == NULL) return NULL;

    //Force regex to stop searching at line breaks
    char *string_ptr = string;
    char temp;
    while(*string_ptr != '\n' && *string_ptr != '\0')
      string_ptr++;
    temp = *string_ptr;
    *string_ptr = '\0';

    size_t nmatch = 1;
    regmatch_t pmatch[nmatch];

    int status;
    if ((status = regexec(re_locate, string, 1, pmatch, 0)) == 0)
    {
      *start = g_utf8_pointer_to_offset (line_start, string + pmatch[0].rm_so);
      *end = g_utf8_pointer_to_offset (line_start, string + pmatch[0].rm_eo);
      *string_ptr = temp;
      return (string + pmatch[0].rm_eo);
    }
    else
    {
      *string_ptr = temp;
      return NULL;
    }
}


//!
//! @brief Regex for determining highly relevent kanji atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a kanji one.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_kanji_high_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "((^無)|(^不)|(^非)|(^)|(^お)|(^御))(");
    strcat (expression, string);
    strcat (expression, ")(($))");
    return regcomp (regex, expression, flags);
}


//!
//! @brief Regex for determining mediumly relevent kanji atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a kanji one.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_kanji_med_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "((^)|(^お)|(を)|(に)|(で)|(は)|(と))(");
    strcat (expression, string);
    strcat (expression, ")((で)|(が)|(の)|(を)|(に)|(で)|(は)|(と)|($))");
    return regcomp (regex, expression, flags);
}


//!
//! @brief Regex for determining highly relevent furigana atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a furigana one.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_furi_high_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "^((お)|())(");
    strcat (expression, string);
    strcat (expression, ")$");
    return regcomp (regex, expression, flags);
}


//!
//! @brief Regex for determining mediumly relevent furigana atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a furigana one.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_furi_med_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "((^)|(^お)|(を)|(に)|(で)|(は)|(と))(");
    strcat (expression, string);
    strcat (expression, ")((で)|(が)|(の)|(を)|(に)|(で)|(は)|(と)|($))");
    return regcomp (regex, expression, flags);
}


//!
//! @brief Regex for determining highly relevent romaji atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a romaji one.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_roma_high_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "(^|\\)|(/)|(to ))\\b(");
    strcat (expression, string);
    strcat (expression, ")\\b(\\(|/|$)");
    printf("%s\n", expression);
    return regcomp (regex, expression, flags);
}


//!
//! @brief Regex for determining mediumly relevent romaji atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a romaji one.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_roma_med_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "\\{(");
    strcat (expression, string);
    strcat (expression, ")\\}|(\\) |/)((to )|(to be )|())(");
    strcat (expression, string);
    strcat (expression, ")(( \\([^/]+\\)/)|(/))|(\\[)(");
    strcat (expression, string);
    strcat (expression, ")(\\])|^(");
    strcat (expression, string);
    strcat (expression, ")\\b");
    return regcomp (regex, expression, flags);
}


//!
//! @brief Regex for determining highly relevent mix atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a mix one. This means
//! there is kanji/hiragana/romaji mingled throughout.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_mix_high_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "(\\b(");
    strcat (expression, string);
    strcat (expression, ")\\b|^(");
    strcat (expression, string);
    strcat (expression, "))");
    return regcomp (regex, expression, flags);
}


//!
//! @brief Regex for determining mediumly relevent mix atoms
//!
//! Builds the regex used in engine.c for determining the relevance
//! of a returned result when the query is a mix one. This means
//! there is kanji/hiragana/romaji mingled throughout.
//!
//! @param regex A passed regex_t to assign an allocated regext to
//! @param strang The regex pattern to use
//! @param flags The regex flags to use
//! 
gboolean gw_regex_create_mix_med_regex (regex_t *regex, char *string, int flags)
{
    char expression[MAX_LINE * 2];
    strcpy (expression, "\\{(");
    strcat (expression, string);
    strcat (expression, ")\\}|(\\) |/)((to )|(to be )|())(");
    strcat (expression, string);
    strcat (expression, ")(( \\([^/]+\\)/)|(/))|(\\[)(");
    strcat (expression, string);
    strcat (expression, ")(\\])|^(");
    strcat (expression, string);
    strcat (expression, ")\\b");
    return regcomp (regex, expression, flags);
}

