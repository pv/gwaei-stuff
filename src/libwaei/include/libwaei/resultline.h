#ifndef LW_RESULTLINE_INCLUDED
#define LW_RESULTLINE_INCLUDED

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
//! @file src/include/libwaei/resultline.h
//!
//! @brief To be written.
//!
//! To be written.
//!

#include <libwaei/io.h>

#define LW_RESULTLINE(object) (LwResultLine*) object

typedef enum {
  LW_RESULTLINE_RELEVANCE_UNSET,
  LW_RESULTLINE_RELEVANCE_LOW,
  LW_RESULTLINE_RELEVANCE_MEDIUM,
  LW_RESULTLINE_RELEVANCE_HIGH,
  TOTAL_RESULTLINE_RELEVANCE
} LwResultLineRelevance;

//!
//! @brief Primitive for storing lists of dictionaries
//!

struct _LwResultLine {
    char string[LW_IO_MAX_FGETS_LINE];     //!< Character array holding the result line for the pointers to reference

    //General result things
    LwResultLineRelevance relevance;
    char *def_start[50];        //!< Pointer to the definitions
    int def_total;              //!< Total definitions found for a result
    char *number[50];           //!< Pointer to the numbers of the definitions
    char *kanji_start;          //!< The kanji portion of the definition
    char *furigana_start;       //!< The furigana portion of a definition
    char *classification_start; //!< The classification of the word type of the Japanese word.

    //Kanji things
    char *strokes;      //!< Pointer to the number of strokes of a kanji
    char *frequency;    //!< Pointer to the frequency number of a kanji
    char *readings[3];  //!< Pointer to the readings of a kanji
    char *meanings;     //!< Pointer to the meanings of a kanji
    char *grade;        //!< Pointer to the grade of a kanji
    char *jlpt;         //!< Pointer to the JLPT level of a kanji
    char *kanji;        //!< Pointer to the kanji itself
    char *radicals;     //!< Pointer to a kanji's radicals

    gboolean important; //!< Weather a word/phrase has a high frequency of usage.

};
typedef struct _LwResultLine LwResultLine;


LwResultLine* lw_resultline_new (void);
void lw_resultline_free (LwResultLine*);
void lw_resultline_init (LwResultLine*);
void lw_resultline_deinit (LwResultLine*);

void lw_resultline_parse_edict_result_string (LwResultLine*);
void lw_resultline_parse_kanjidict_result_string (LwResultLine*);
void lw_resultline_parse_radicaldict_result_string (LwResultLine*);
void lw_resultline_parse_examplesdict_result_string (LwResultLine*);
void lw_resultline_parse_unknowndict_result_string (LwResultLine*);

gboolean lw_resultline_is_similar (LwResultLine *rl1, LwResultLine *rl2);

#endif
