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

#include <glib.h>

#include <gwaei/backend.h>

//Private function declarations
static GRegex*** _allocate_regex_pointers (int);
static void _free_regex_pointers (GRegex ***);


GwQueryLine* gw_queryline_new ()
{
    GwQueryLine* temp;

    if ((temp = (GwQueryLine*) malloc(sizeof(GwQueryLine))) == NULL) return NULL;

    //A place for a copy of the raw string
    temp->string = NULL;
    temp->re_kanji = NULL;
    temp->re_furi = NULL;
    temp->re_roma = NULL;
    temp->re_mix = NULL;
    temp->re_strokes = NULL;
    temp->re_frequency = NULL;
    temp->re_grade = NULL;
    temp->re_jlpt = NULL;

    return temp;
}

void gw_queryline_free (GwQueryLine *ql)
{
    g_free (ql->string);
    ql->string = NULL;

    _free_regex_pointers (ql->re_kanji);
    _free_regex_pointers (ql->re_furi);
    _free_regex_pointers (ql->re_roma);
    _free_regex_pointers (ql->re_mix);
    _free_regex_pointers (ql->re_strokes);
    _free_regex_pointers (ql->re_frequency);
    _free_regex_pointers (ql->re_grade);
    _free_regex_pointers (ql->re_jlpt);

    free (ql);
}

static void _free_regex_pointers (GRegex ***re)
{
    //Sanity check
    if (re == NULL) return;

    //Declarations
    int i;
    int j;

    //Free pointers intelligently
    for (i = 0; re[i] != NULL; i++)
      for (j = 0; j < GW_RELEVANCE_TOTAL; j++)
        if (re[i][j] != NULL) g_regex_unref(re[i][j]);

    for (i = 0; re[i] != NULL; i++)
      free (re[i]);

    free (re);
}

static GRegex*** _allocate_regex_pointers (int length)
{
    //Declarations
    int i;
    int j;
    GRegex ***re;

    //Allocate the memory
    re = (GRegex***) malloc((length + 1) * sizeof(GRegex**));

    //Initialize it to NULL
    for (i = 0; i < length; i++)
    {
      re[i] = (GRegex**) malloc(GW_RELEVANCE_TOTAL * sizeof (GRegex*));
      for (j = 0; j < GW_RELEVANCE_TOTAL; j++)
        re[i][j] = NULL;
    }
    re[i] = NULL;

    //Return the address
    return re;
}


//!
//! @brief Parses a query using the edict style
//!
//! The program will (to be written)
//!
//! @param ql Pointer to a GwQueryLine object ot parse a query string into.
//! @param string constant string that is the raw query.
//!
gboolean gw_queryline_parse_edict_string (GwQueryLine *ql, const char* string)
{
   //Sanity check
   if (ql->string != NULL && strcmp(ql->string, string) == 0) return TRUE;

   //Free previously used memory
   g_free (ql->string);

   _free_regex_pointers (ql->re_kanji);
   _free_regex_pointers (ql->re_furi);
   _free_regex_pointers (ql->re_roma);
   _free_regex_pointers (ql->re_mix);
   _free_regex_pointers (ql->re_strokes);
   _free_regex_pointers (ql->re_frequency);
   _free_regex_pointers (ql->re_grade);
   _free_regex_pointers (ql->re_jlpt);

   ql->string = NULL;
   ql->re_kanji = NULL;
   ql->re_furi = NULL;
   ql->re_roma = NULL;
   ql->re_mix = NULL;
   ql->re_strokes = NULL;
   ql->re_frequency = NULL;
   ql->re_grade = NULL;
   ql->re_jlpt = NULL;

   //Declarations
   char **atoms;
   char **iter;
   char *atom;
   char *temp;
   char *expression;
   char *half;
   char buffer[300];
   int rk_conv_pref;
   gboolean want_rk_conv;
   gboolean want_hk_conv;
   gboolean want_kh_conv;
   gboolean want_conv;
   gboolean all_regex_built;
   int length;
   GRegex ***re;
   int i;

   //Memory initializations
   ql->string = gw_util_prepare_query (string, FALSE);
   all_regex_built = TRUE;

   rk_conv_pref = gw_pref_get_int_by_schema (GW_SCHEMA_BASE, GW_KEY_ROMAN_KANA);
   want_rk_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !gw_util_is_japanese_locale()));
   want_hk_conv = gw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_HIRA_KATA);
   want_kh_conv = gw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_KATA_HIRA);

   atoms = g_strsplit (ql->string, "&", GW_QUERYLINE_MAX_ATOMS);
   length = g_strv_length (atoms);
   
   ql->re_kanji     = _allocate_regex_pointers (length);
   ql->re_furi      = _allocate_regex_pointers (length);
   ql->re_roma      = _allocate_regex_pointers (length);
   ql->re_mix       = _allocate_regex_pointers (length);
   ql->re_strokes   = _allocate_regex_pointers (length);
   ql->re_frequency = _allocate_regex_pointers (length);
   ql->re_grade     = _allocate_regex_pointers (length);
   ql->re_jlpt      = _allocate_regex_pointers (length);


   //Setup the expression to be used in the base of the regex for kanji-ish strings
   re = ql->re_kanji;
   for (iter = atoms; *iter != NULL && re < (ql->re_kanji + length); iter++)
   {
     atom = *iter;
     if (gw_util_is_kanji_ish_str (atom) || gw_util_is_kanji_str (atom)) //Figures out if the string may contain hiragana
     {
       expression = g_strdup_printf ("(%s)", atom);

       if (gw_util_is_yojijukugo_str (atom))  //Check for yojijukugo
       {
          //First half of the yojijukugo
          half = g_strndup (atom, g_utf8_next_char(g_utf8_next_char(atom))- atom);
          temp = g_strdup_printf ("%s|(%s)", expression, half);
          g_free (expression);
          expression = temp;
          g_free (half);

          //Second half of the yojijukugo
          half = g_strdup (g_utf8_next_char(g_utf8_next_char(atom)));
          temp = g_strdup_printf ("%s|(%s)", expression, half);
          g_free (expression);
          expression = temp;
          g_free (half);
       }

       //Compile the regexes
       for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
         if (((*re)[i] = gw_regex_kanji_new (expression, i)) == NULL) all_regex_built = FALSE;

       g_free (expression);
       re++;
     }
   }


   //Setup the expression to be used in the base of the regex for furigana strings
   re = ql->re_furi;
   for (iter = atoms; *iter != NULL && re < (ql->re_furi + length); iter++)
   {
     atom = *iter;
     if (gw_util_is_furigana_str (atom))
     {
       expression = g_strdup_printf ("(%s)", atom);

       if (want_hk_conv && gw_util_is_hiragana_str (atom))
       {
         temp = g_strdup (atom);
         gw_util_str_shift_hira_to_kata (temp);
         g_free (expression);
         expression = g_strdup_printf("(%s)|(%s)", atom, temp);
         g_free (temp);
       }
       else if (want_kh_conv && gw_util_is_katakana_str (atom))
       {
         temp = g_strdup (atom);
         gw_util_str_shift_kata_to_hira (temp);
         g_free (expression);
         expression = g_strdup_printf("(%s)|(%s)", atom, temp);
         g_free (temp);
       }

       //Compile the regexes
       for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
         if (((*re)[i] = gw_regex_furi_new (expression, i)) == NULL) all_regex_built = FALSE;

       g_free (expression);
       re++;
     }

     else if (gw_util_is_romaji_str (atom) && gw_util_str_roma_to_hira (atom, buffer, 300) && want_rk_conv)
     {
       expression = g_strdup_printf("(%s)", buffer);

       if (want_hk_conv)
       {
         temp = g_strdup (buffer);
         gw_util_str_shift_hira_to_kata (temp);
         g_free (expression);
         expression = g_strdup_printf("(%s)|(%s)", buffer, temp);
         g_free (temp);
       }

       //Compile the regexes
       for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
         if (((*re)[i] = gw_regex_furi_new (expression, i)) == NULL) all_regex_built = FALSE;

       g_free (expression);
       re++;
     }
   }


   //Setup the expression to be used in the base of the regex
   re = ql->re_roma;
   for (iter = atoms; *iter != NULL && re < (ql->re_roma + length); iter++)
   {
     atom = *iter;
     if (gw_util_is_romaji_str (atom))
     {
       expression = g_strdup (atom);

       //Compile the regexes
       for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
         if (((*re)[i] = gw_regex_romaji_new (expression, i)) == NULL) all_regex_built = FALSE;

       g_free (expression);
       re++;
     }
   }  


   //Setup the expression to be used in the base of the regex
   re = ql->re_mix;
   for (iter = atoms; iter != NULL; iter++)
   {
     atom = *iter;
     if (!gw_util_is_kanji_ish_str (atom) &&
         !gw_util_is_kanji_str (atom)     && 
         !gw_util_is_furigana_str (atom)  &&
         !gw_util_is_romaji_str (atom)      )
     {
       expression = g_strdup (atom);

       //Compile the regexes
       for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
         if (((*re)[i] = gw_regex_romaji_new (expression, i)) == NULL) all_regex_built = FALSE;

       g_free (expression);
       re++;
     }
   }

   return all_regex_built;
}


static gboolean _compile_number_search_regex (GRegex ***re, const char* subject, const GwRegexDataIndex INDEX)
{
    //Declarations
    GRegex ***iter;
    GMatchInfo *match_info;
    gboolean all_regex_built;
    char *match_text;
    char *expression;
    char *ptr;
    int i;

    //Initializations
    all_regex_built = TRUE;

    //Compile the regexes
    g_regex_match (gw_re[INDEX], subject, 0, &match_info);
    for (iter = re; all_regex_built && iter - re < GW_QUERYLINE_MAX_ATOMS && g_match_info_matches (match_info); iter++)
    {
      match_text = g_match_info_fetch (match_info, 0);
      expression = g_strdup_printf("\\b%s\\b", match_text);

      for (i = 0; all_regex_built && i < GW_RELEVANCE_TOTAL; i++)
        if (((*iter)[i] = gw_regex_new (expression, i)) == NULL) all_regex_built = FALSE;

      g_free (expression);
      g_free (match_text);
    }
    g_match_info_free (match_info);

    //Cleanup if there was an error
    for (iter = re; !all_regex_built && iter - re < GW_QUERYLINE_MAX_ATOMS; iter++)
    {
      for (i = 0; i < GW_RELEVANCE_TOTAL; i++)
      {
        if ((*iter)[i] != NULL) g_regex_unref ((*re)[i]);
        (*iter)[i] = NULL;
      }
    }

    //Finish
    return all_regex_built;
}



//!
//! @brief Parses a query using the kanjidict style
//!
//! The kanjidict style puts an emphasis on separate
//! atoms, linking them with AND.  Special cases the function
//! will try to parse out are hiragana words, kanji/radicals, english
//! phrases.
//!
//! @param ql Pointer to a GwQueryLine object ot parse a query string into.
//! @param string constant string that is the raw query.
//!
gboolean gw_queryline_parse_kanjidict_string (GwQueryLine *ql, const char* string)
{
  /*
    //Make sure it isn't already created
    if (ql->string != NULL && strcmp(ql->string, string) == 0) return TRUE;

    //Free previously used memory
    g_free (ql->string);

    _free_regex_pointers (ql->re_kanji);
    _free_regex_pointers (ql->re_furi);
    _free_regex_pointers (ql->re_roma);
    _free_regex_pointers (ql->re_mix);
    _free_regex_pointers (ql->re_strokes);
    _free_regex_pointers (ql->re_frequency);
    _free_regex_pointers (ql->re_grade);
    _free_regex_pointers (ql->re_jlpt);

    ql->string = NULL;
    ql->re_kanji = NULL;
    ql->re_furi = NULL;
    ql->re_roma = NULL;
    ql->re_mix = NULL;
    ql->strokes = NULL;
    ql->re_frequency = NULL;
    ql->re_grade = NULL;
    ql->re_jlpt = NULL;

    //Declarations
    GMatchInfo *match_info;
    int rk_conv_pref;
    gboolean want_rk_conv;
    char *atom;
    char *start;
    char *end;
    gunichar character;

    //Initializations
    ql->string = gw_util_prepare_query (string, FALSE);
    rk_conv_pref = gw_pref_get_int_by_schema (GW_SCHEMA_BASE, GW_KEY_ROMAN_KANA);
    want_rk_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !gw_util_is_japanese_locale()));
    all_regex_built = TRUE;
   
    ql->re_kanji     = _allocate_regex_pointers (length);
    ql->re_furi      = _allocate_regex_pointers (length);
    ql->re_roma      = _allocate_regex_pointers (length);
    ql->re_mix       = _allocate_regex_pointers (length);
    ql->re_strokes   = _allocate_regex_pointers (length);
    ql->re_frequency = _allocate_regex_pointers (length);
    ql->re_grade     = _allocate_regex_pointers (length);
    ql->re_jlpt      = _allocate_regex_pointers (length);

    //Get stroke
    if (!_compile_number_search_regex (ql->strokes_regex, string, GW_RE_QUERY_STROKES))
      all_regex_built = FALSE;

    //Get frequency
    if (!_compile_number_search_regex (ql->strokes_frequency, string, GW_RE_QUERY_FREQUENCY))
      all_regex_built = FALSE;

    //Get grade level
    if (!_compile_number_search_regex (ql->strokes_grade, string, GW_RE_QUERY_GRADE))
      all_regex_built = FALSE;

    //Get JLPT level
    if (!_compile_number_search_regex (ql->strokes_jlpt, string, GW_RE_QUERY_JLPT))
      all_regex_built = FALSE;

    //Get Kanji
    atom = NULL;
    re = ql->re_kanji;
    for (ptr = ql->string; *ptr != '\0' && i < GW_QUERYLINE_MAX_ATOMS; ptr = g_utf8_next_char (ptr))
    {
      character = g_utf8_get_char (ptr);
      script = g_unichar_get_script (character);
      if (script == G_UNICODE_SCRIPT_HAN)
      {
        start = iter;
        end = g_utf8_next_char (ptr);
        atom = g_strndup (start, end - start);
        for (i = 0; all_regex_built && i < GW_RELEVANCE_TOTAL; i++)
          (*re)[i] = gw_regex_new (atom, i);
        g_free (atom);
        re++;
      }
    }

    //Get Hiragana and Katakana (aka Furigana)
    atom = NULL;
    re = ql->re_furi;
    for (ptr = ql->string; *ptr != '\0' && i < GW_QUERYLINE_MAX_ATOMS; ptr = g_utf8_next_char (ptr))
    {
      character = g_utf8_get_char (ptr);
      script = g_unichar_get_script (character);
      if (script == G_UNICODE_SCRIPT_KATAKANA || script == G_UNICODE_SCRIPT_HIRAGANA)
      {
        start = ptr;
        while (*ptr != '\0' && (script == G_UNICODE_SCRIPT_KATAKANA || script == G_UNICODE_HIRAGANA))
        {
          ptr = g_utf8_next_char (ptr);
          character = g_utf8_get_char (ptr);
          script = g_unichar_get_script (character);
        }
        end = ptr;

        //Create the regexes
        atom = g_strndup (start, end - start);
        for (i = 0; all_regex_built && i < GW_RELEVANCE_TOTAL; i++)
          (*re)[i] = gw_regex_new (atom, i);
        g_free (atom);
        re++;
      }
    }

    //Get Hiragana and Katakana (aka Furigana)
    atom = NULL;
    re = ql->re_roma;
    for (ptr = ql->string; *ptr != '\0' && i < GW_QUERYLINE_MAX_ATOMS; ptr = g_utf8_next_char (ptr))
    {
      character = g_utf8_get_char (ptr);
      script = g_unichar_get_script (character);
      if (script == G_UNICODE_SCRIPT_LATIN)
      {
        start = ptr;
        while (*ptr != '\0' && script == G_UNICODE_SCRIPT_LATIN)
        {
          ptr = g_utf8_next_char (ptr);
          character = g_utf8_get_char (ptr);
          script = g_unichar_get_script (character);
        }
        end = ptr;

        //Create the regexes
        atom = g_strndup (start, end - start);
        for (i = 0; all_regex_built && i < GW_RELEVANCE_TOTAL; i++)
          if ((*re)[i] = gw_regex_new (atom, i)) == NULL) all_regex_built = FALSE;
        g_free (atom);
        re++;
      }
    }

    return all_regex_built;
    */
  return FALSE;
}


//!
//! @brief Parses a query using the example style
//!
//! The program will (to be written)
//!
//! @param ql Pointer to a GwQueryLine object ot parse a query string into.
//! @param string constant string that is the raw query.
//!
gboolean gw_queryline_parse_exampledict_string (GwQueryLine *ql, const char* string)
{
/*
    //Make sure it isn't already created
    if (ql->string != NULL && strcmp(ql->string, string) == 0) return TRUE;

    //Free previously used memory
    g_free (ql->string);

    _free_regex_pointers (ql->re_kanji);
    _free_regex_pointers (ql->re_furi);
    _free_regex_pointers (ql->re_roma);
    _free_regex_pointers (ql->re_mix);
    _free_regex_pointers (ql->re_strokes);
    _free_regex_pointers (ql->re_frequency);
    _free_regex_pointers (ql->re_grade);
    _free_regex_pointers (ql->re_jlpt);

    ql->string = NULL;
    ql->re_kanji = NULL;
    ql->re_furi = NULL;
    ql->re_roma = NULL;
    ql->re_mix = NULL;
    ql->re_strokes = NULL;
    ql->re_frequency = NULL;
    ql->re_grade = NULL;
    ql->re_jlpt = NULL;

    //Declarations
    int rk_conv_pref;
    gboolean want_rk_conv;
    gboolean want_hk_conv;
    gboolean want_kh_conv;

    //Initializations
    ql->string = gw_util_prepare_query (string, FALSE);
    rk_conv_pref = gw_pref_get_int_by_schema (GW_SCHEMA_BASE, GW_KEY_ROMAN_KANA);
    want_rk_conv = (rk_conv_pref == 0 || (rk_conv_pref == 2 && !gw_util_is_japanese_locale()));
    want_hk_conv = gw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_HIRA_KATA);
    want_kh_conv = gw_pref_get_boolean_by_schema (GW_SCHEMA_BASE, GW_KEY_KATA_HIRA);
    all_regex_built = TRUE;
 
    ql->re_kanji     = _allocate_regex_pointers (length);
    ql->re_furi      = _allocate_regex_pointers (length);
    ql->re_roma      = _allocate_regex_pointers (length);
    ql->re_mix       = _allocate_regex_pointers (length);
    ql->re_strokes   = _allocate_regex_pointers (length);
    ql->re_frequency = _allocate_regex_pointers (length);
    ql->re_grade     = _allocate_regex_pointers (length);
    ql->re_jlpt      = _allocate_regex_pointers (length);

    atoms = g_strsplit (ql->string, "&", GW_QUERYLINE_MAX_ATOMS)
    length = g_strv_length (atoms);


    

   while (atoms[i] != NULL && i < MAX_ATOMS)
   {
     if (gw_util_is_kanji_ish_str (atoms[i]) || gw_util_is_kanji_str (atoms[i]))
     {
       strcpy(ql->hira_string, atoms[i]);
       if (regcomp (&(ql->kanji_regex                         [GW_QUERYLINE_EXIST] [kanji_pos]), atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (regcomp (&(ql->kanji_regex                         [GW_QUERYLINE_LOCATE][kanji_pos]), atoms[i], EFLAGS_LOCATE) != 0) return FALSE;
       if (regcomp (&(ql->kanji_regex                         [GW_QUERYLINE_HIGH] [kanji_pos]), atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (gw_regex_create_kanji_med_regex (&(ql->kanji_regex [GW_QUERYLINE_MED]   [kanji_pos]), atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       kanji_pos++;
       ql->kanji_total++;
     }

     else if (gw_util_is_furigana_str (atoms[i]))
     {
       strcpy(ql->hira_string, atoms[i]);
       if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_EXIST] [furi_pos]), atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_LOCATE][furi_pos]), atoms[i], EFLAGS_LOCATE) != 0)  return FALSE;
       if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_HIGH] [furi_pos]), atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (gw_regex_create_furi_med_regex  (&(ql->furi_regex[GW_QUERYLINE_MED]   [furi_pos]), atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       furi_pos++;
       ql->furi_total++;

       if (want_hk_conv && gw_util_is_hiragana_str (atoms[i]))
       {
         strcpy(temp, atoms[i]);
         gw_util_str_shift_hira_to_kata (temp);
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_EXIST] [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_LOCATE][furi_pos]), temp            , EFLAGS_LOCATE) != 0)  return FALSE;
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_HIGH] [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         if (gw_regex_create_furi_med_regex  (&(ql->furi_regex[GW_QUERYLINE_MED]   [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         furi_pos++;
         ql->furi_total++;
       }
       else if (want_kh_conv && gw_util_is_katakana_str (atoms[i]))
       {
         strcpy(temp, atoms[i]);
         gw_util_str_shift_kata_to_hira (temp);
         strcpy(ql->hira_string, atoms[i]);
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_LOCATE][furi_pos]), temp            , EFLAGS_LOCATE) != 0)  return FALSE;
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_EXIST] [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_HIGH][furi_pos]), temp            , EFLAGS_LOCATE) != 0)  return FALSE;
         if (gw_regex_create_furi_med_regex  (&(ql->furi_regex[GW_QUERYLINE_MED]   [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         furi_pos++;
         ql->furi_total++;
       }
     }
     else if (gw_util_is_romaji_str (atoms[i]))
     {
       if (regcomp (&(ql->roma_regex                        [GW_QUERYLINE_EXIST] [roma_pos]), atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (regcomp (&(ql->roma_regex                        [GW_QUERYLINE_LOCATE][roma_pos]), atoms[i], EFLAGS_LOCATE) != 0)  return FALSE;
       if (gw_regex_create_roma_med_regex  (&(ql->roma_regex[GW_QUERYLINE_HIGH]   [roma_pos]), atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       if (gw_regex_create_roma_med_regex  (&(ql->roma_regex[GW_QUERYLINE_MED]   [roma_pos]), atoms[i], EFLAGS_EXIST)  != 0)  return FALSE;
       roma_pos++;
       ql->roma_total++;

       //Add coversions to search on success
       if (gw_util_str_roma_to_hira (atoms[i], temp, 300)  && want_rk_conv)
       {
         //Hiragana
         strcpy(ql->hira_string, temp);
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_EXIST] [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_LOCATE][furi_pos]), temp            , EFLAGS_LOCATE) != 0)  return FALSE;
         if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_HIGH] [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         if (gw_regex_create_furi_med_regex  (&(ql->furi_regex[GW_QUERYLINE_MED]   [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
         furi_pos++;
         ql->furi_total++;

         if (want_hk_conv)
         {
           //Katakana
           gw_util_str_shift_hira_to_kata (temp);
           if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_EXIST] [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
           if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_LOCATE][furi_pos]), temp            , EFLAGS_LOCATE) != 0)  return FALSE;
           if (regcomp (&(ql->furi_regex                        [GW_QUERYLINE_HIGH] [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
           if (gw_regex_create_furi_med_regex  (&(ql->furi_regex[GW_QUERYLINE_MED]   [furi_pos]), temp            , EFLAGS_EXIST)  != 0)  return FALSE;
           furi_pos++;
           ql->furi_total++;
         }
       }
     }
     else
     {
       if (regcomp (&(ql->mix_regex                       [GW_QUERYLINE_EXIST] [mix_pos]), atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (regcomp (&(ql->mix_regex                       [GW_QUERYLINE_LOCATE][mix_pos]), atoms[i], EFLAGS_LOCATE) != 0) return FALSE;
       if (gw_regex_create_mix_high_regex (&(ql->mix_regex[GW_QUERYLINE_HIGH]  [mix_pos]), atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       if (gw_regex_create_mix_med_regex  (&(ql->mix_regex[GW_QUERYLINE_MED]   [mix_pos]), atoms[i], EFLAGS_EXIST)  != 0) return FALSE;
       mix_pos++;
       ql->mix_total++;
     }
     i++;
   }
   return all_regex_built;
*/
   return FALSE;
}


