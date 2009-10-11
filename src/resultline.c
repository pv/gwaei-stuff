#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <glib.h>

#include <gwaei/definitions.h>
#include <gwaei/resultline.h>



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


void gw_resultline_parse_result_string (GwResultLine *item, char *string)
{
    gw_resultline_clear_variables (item);
    strncpy(item->string, string, MAX_LINE);

    char *ptr = item->string;
    char *next = NULL;
    char *nextnext = NULL;
    char *nextnextnext = NULL;
    char *temp = NULL;

    //Remove the final line break
    if ((temp = g_utf8_strrchr (item->string, -1, L'\n')) != NULL)
    {
        *temp = '\0';
    }

    //Set the kanji pointers
    item->kanji_start = ptr;
    ptr = g_utf8_strchr (ptr, -1, L' ');
    *ptr = '\0';

    //Set the furigana pointer
    ptr++;
    if (g_utf8_get_char(ptr) == L'[')
    {
      ptr = g_utf8_next_char(ptr);
      item->furigana_start = ptr;
      ptr = g_utf8_strchr (ptr, -1, L']');
      *ptr = '\0';
    }
    else
    {
      item->furigana_start = NULL;
      ptr--;
    }


    //Find if there is a type description classification
    temp = ptr;
    temp++;
    temp = g_utf8_strchr (temp, -1, L'/');
    if (g_utf8_get_char(temp + 1) == '(')
    {
      item->classification_start = temp + 2;
      temp = g_utf8_strchr (temp, -1, L')');
      *temp = '\0';
      ptr = temp;
    }

    //Set the definition pointers
    ptr++;
    ptr = g_utf8_next_char(ptr);
    item->def_start[0] = ptr;
    item->number[0] = item->first;
    int i = 1;

    temp = ptr;
    while ((temp = g_utf8_strchr(temp, -1, L'(')) != NULL)
    {
      next = g_utf8_next_char (temp);
      nextnext = g_utf8_next_char (next);
      nextnextnext = g_utf8_next_char (nextnext);
      if (*next != '\0' && *nextnext != '\0' &&
          *next == L'1' && *nextnext == L')')
      {
         item->def_start[0] = item->def_start[0] + 4;
      }
      else if (*next != '\0' && *nextnext != '\0' && *nextnextnext != '\0' &&
               *next >= L'1' && *next <= L'9' && (*nextnext == L')' || *nextnextnext == L')'))
      {
         *(temp - 1) = '\0';
         item->number[i] = temp;
         *(temp + 3) = '\0';
         item->def_start[i] = temp + 4;
         i++;
      }
      temp = nextnextnext + 1;
    }
    item->def_total = i;
    i--;

    //Get the importance
    //temp = item->def_start[i] + strlen(item->def_start[i]) - 4;
    if ((temp = g_utf8_strrchr (item->def_start[i], -1, L'(')) != NULL)
    {
      item->important = (*temp == '(' && *(temp + 1) == 'P' && *(temp + 2) == ')');
      if (item->important) *(temp) = '\0';
    }
}


void gw_resultline_parse_kanji_result_string (GwResultLine *item, char *string)
{
    gw_resultline_clear_variables (item);
    strncpy(item->string, string, MAX_LINE);

    //First generate the grade, stroke, frequency, and jplt fields
    char *start[4], *end[4];
    gboolean found[4];
    char *ptr = item->string;

    if ((found[0] = gw_regex_locate_boundary_byte_pointers(ptr, " G[0-9]{1,2} ", &start[0], &end[0])))
      item->grade = start[0] + 2;
    else
      item->grade = NULL;

    if ((found[1] =gw_regex_locate_boundary_byte_pointers(ptr, " S[0-9]{1,2} ", &start[1], &end[1])))
      item->strokes = start[1] + 2;
    else
      item->strokes = NULL;

    if ((found[2] =gw_regex_locate_boundary_byte_pointers(ptr, " F[0-9]{1,5} ", &start[2], &end[2])))
      item->frequency = start[2] + 2;
    else
      item->frequency = NULL;

    if ((found[3] = gw_regex_locate_boundary_byte_pointers(ptr, " J[0-9]{1,1} ", &start[3], &end[3])))
      item->jlpt = start[3] + 2;
    else
      item->jlpt = NULL;

    //Get the kanji character
    item->kanji = ptr;
    while (g_utf8_get_char(ptr) != L' ') {
      ptr = g_utf8_next_char(ptr);
    }
    *ptr = '\0';
    ptr++;

    //Test if the radicals information is present
    if(g_utf8_get_char(ptr) > 3040)
    {
      item->radicals = ptr;
      while(g_utf8_get_char(ptr) > 3040 || g_utf8_get_char(ptr) == L' ')
      {
        ptr = g_utf8_next_char(ptr);
      }
      *(ptr - 1) = '\0';
    }

    //Go to the readings section
    while (g_utf8_get_char(ptr) < 3041 && *ptr != '\0')
      ptr = g_utf8_next_char (ptr);
    item->readings[0] = ptr;

    //Copy the rest of the data
    char *next = ptr;
    while (*ptr != '\0' && (next = g_utf8_next_char(ptr)) != NULL && g_utf8_get_char(next) != '{')
    {
      //The strange T1 character between kana readings
      if (g_utf8_get_char (ptr) == L'T' && g_utf8_get_char(next) == L'1') {
        *(ptr - 1) = '\0';
        item->readings[1] = next + 2;
      }
      ptr = next;
    }
    *ptr = '\0';
    item->meanings = next;

    ptr++;
    if ((ptr = g_utf8_strrchr (ptr, -1, '\n')))
      *ptr = '\0';

    if (found[0]) *end[0] = '\0';
    if (found[1]) *end[1] = '\0';
    if (found[2]) *end[2] = '\0';
    if (found[3]) *end[3] = '\0';
}


void gw_resultline_parse_radical_result_string (GwResultLine *item, char *string)
{
    gw_resultline_clear_variables (item);
    strncpy(item->string, string, MAX_LINE);

    //First generate the grade, stroke, frequency, and jplt fields
    item->kanji = item->string;

    char *temp = NULL;

    if (temp = g_utf8_strchr (item->string, -1, L'\n'))
    {
      *temp = '\0';
    }

    if (temp = g_utf8_strchr (item->string, -1, L':'))
    {
       *temp = '\0';
       item->radicals = temp + 1;
    }
}

void gw_resultline_parse_unknown_result_string (GwResultLine *item, char *string)
{
    gw_resultline_clear_variables (item);
    strncpy(item->string, string, MAX_LINE);
    char *temp = NULL;
    if (temp = g_utf8_strchr (item->string, -1, L'\n'))
    {
      *temp = '\0';
    }
}
