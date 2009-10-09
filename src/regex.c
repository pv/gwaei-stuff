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

#include <gwaei/regex.h>


regex_t re_english;
regex_t re_radical;
regex_t re_kanji;
regex_t re_places;
regex_t re_names;
regex_t re_mix;
regex_t re_gz;
regex_t re_hexcolor;


//!
//! @brief Initializes rebuild often used prebuilt regex expressions
//!
//! These are mostly dictionary names where we want to ignore things like case.
//!
void gw_regex_initialize_constant_regular_expressions ()
{
    int eflags_exist = REG_EXTENDED | REG_ICASE | REG_NOSUB;
    if (regcomp (&re_english, "English", eflags_exist) != 0)
      printf ("A problem occured while setting the regular expression for English\n");
    if (regcomp (&re_radical, "Radical", eflags_exist) != 0)
      printf ("A problem occured while setting the regular expression for Radical\n");
    if (regcomp (&re_kanji, "Kanji", eflags_exist) != 0)
      printf ("A problem occured while setting the regular expression for Kanji\n");
    if (regcomp (&re_names, "Names", eflags_exist) != 0)
      printf ("A problem occured while setting the regular expression for Names\n");
    if (regcomp (&re_places, "Places", eflags_exist) != 0)
      printf ("A problem occured while setting the regular expression for Places\n");
    if (regcomp (&re_mix, "Mix", eflags_exist) != 0)
      printf ("A problem occured while setting the regular expression for Mix\n");
    if (regcomp (&re_gz, "\\.gz", eflags_exist) != 0)
      printf ("A problem occured while setting the regular expression for .gz\n");
    if (regcomp (&re_hexcolor, "^#[0-9A-Fa-f]{6,6}$", eflags_exist) != 0)
      printf ("A problem occured while setting the regular expression for hexcolor\n");
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



