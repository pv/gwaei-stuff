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
//! @file src/radicals-interface-gtk.c
//!
//! @brief Abstraction layer for gtk objects 
//!
//!  Used as a go between for functions interacting with GUI interface objects.
//!  This is the gtk version.
//!


#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/definitions.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>
#include <gwaei/interface.h>
#include <gwaei/callbacks.h>
#include <gwaei/gtk.h>


static char radical_cache[300 * 3];


GList *radical_list = NULL;
/*
char *radical_pointer = malloc(sizeof({"1", "°ì", "¡¼", "¤¤¤Á", NULL}));
g_list_append (radical_list, { "1", "°ì", "¡¼", "¤¤¤Á", NULL });
*/


/*
static char** create_radical_info (char** radical_info)
{
  char **temp;
  temp = malloc (sizeof(radical_info));
  return temp;
}
g_list_append (radical_list, create_radical_info (1, "°ì", "¡¼", "¤¤¤Á", NULL));
*/

char *radical_array[][5] =
{
  //{Strokes, Representative_radical, Actual_radical, Name, NULL}}
  {"1", "¡Ã", "¡Ã", "", NULL },
  {"1", "Ð¦", "Ð¦", "", NULL },
  {"1", "¥Î", "¥Î", "", NULL },
  {"1", "²µ", "²µ", "", NULL },
  {"1", "Ð­", "Ð­", "", NULL },

  {"2", "Æó", "Æó", "", NULL },
  {"2", "Ðµ", "Ðµ", "", NULL },
  {"2", "¿Í", "¿Í", "", NULL },
  {"2", "²½", "²½", "", NULL },
  {"2", "Ð¤", "Ð¤", "", NULL },
  {"2", "Ñ¹", "Ñ¹", "", NULL },
  {"2", "Æþ", "Æþ", "", NULL },
  {"2", "¥Ï", "¥Ï", "", NULL },
  {"2", "ÑÄ", "ÑÄ", "", NULL },
  {"2", "ÑÌ", "ÑÌ", "", NULL },
  {"2", "ÑÒ", "ÑÒ", "", NULL },
  {"2", "ÑÜ", "ÑÜ", "", NULL },
  {"2", "Ñá", "Ñá", "", NULL },
  {"2", "Åá", "Åá", "", NULL },
  {"2", "´¢", "´¢", "", NULL },
  {"2", "ÎÏ", "ÎÏ", "", NULL },
  {"2", "Ò±", "Ò±", "", NULL },
  {"2", "Ò¸", "Ò¸", "", NULL },
  {"2", "Ò¹", "Ò¹", "", NULL },
  {"2", "½½", "½½", "", NULL },
  {"2", "ËÎ", "ËÎ", "", NULL },
  {"2", "ÒÇ", "ÒÇ", "", NULL },
  {"2", "ÒÌ", "ÒÌ", "", NULL },
  {"2", "ÒÓ", "ÒÓ", "", NULL },
  {"2", "Ëô", "Ëô", "", NULL },
  {"2", "¹þ", "¹þ", "", NULL },
  {"2", "¶å", "¶å", "", NULL },
  {"2", "¥Þ", "¥Þ", "", NULL },
  {"2", "¥æ", "¥æ", "", NULL },
  {"2", "Çµ", "Çµ", "", NULL },

  {"3", "¸ý", "¸ý", "", NULL },
  {"3", "Óø", "Óø", "", NULL },
  {"3", "ÅÚ", "ÅÚ", "", NULL },
  {"3", "»Î", "»Î", "", NULL },
  {"3", "Ôé", "Ôé", "", NULL },
  {"3", "Í¼", "Í¼", "", NULL },
  {"3", "Âç", "Âç", "", NULL },
  {"3", "½÷", "½÷", "", NULL },
  {"3", "»Ò", "»Ò", "", NULL },
  {"3", "Õß", "Õß", "", NULL },
  {"3", "À£", "À£", "", NULL },
  {"3", "¾®", "¾®", "", NULL },
  {"3", "¾°", "¾°", "", NULL },
  {"3", "Õ÷", "Õ÷", "", NULL },
  {"3", "Õù", "Õù", "", NULL },
  {"3", "Ö¥", "Ö¥", "", NULL },
  {"3", "»³", "»³", "", NULL },
  {"3", "Àî", "Àî", "", NULL },
  {"3", "Öß", "Öß", "", NULL },
  {"3", "¹©", "¹©", "", NULL },
  {"3", "Öá", "Öá", "", NULL },
  {"3", "¶Ò", "¶Ò", "", NULL },
  {"3", "´³", "´³", "", NULL },
  {"3", "Öö", "Öö", "", NULL },
  {"3", "Öø", "Öø", "", NULL },
  {"3", "×®", "×®", "", NULL },
  {"3", "×°", "×°", "", NULL },
  {"3", "×µ", "×µ", "", NULL },
  {"3", "µÝ", "µÝ", "", NULL },
  {"3", "×À", "×À", "", NULL },
  {"3", "¥è", "¥è", "", NULL },
  {"3", "×Ä", "×Ä", "", NULL },
  {"3", "×Æ", "×Æ", "", NULL },
  {"3", "Ë»", "Ë»", "", NULL },
  {"3", "Ù©", "Ù©", "", NULL },
  {"3", "½Á", "½Á", "", NULL },
  {"3", "ÈÈ", "ÈÈ", "", NULL },
  {"3", "çè", "çè", "", NULL },
  {"3", "¹þ", "¹þ", "", NULL },
  {"3", "Ë®", "Ë®", "", NULL },
  {"3", "ïô", "ïô", "", NULL },
  {"3", "Öõ", "Öõ", "", NULL },
  {"3", "Ìé", "Ìé", "", NULL },
  {"3", "Ë´", "Ë´", "", NULL },
  {"3", "µÚ", "µÚ", "", NULL },
  {"3", "µ×", "µ×", "", NULL },

  {"4", "¿´", "¿´", "", NULL },
  {"4", "Øù", "Øù", "", NULL },
  {"4", "¸Í", "¸Í", "", NULL },
  {"4", "¼ê", "¼ê", "", NULL },
  {"4", "»Ù", "»Ù", "", NULL },
  {"4", "Ú¾", "Ú¾", "", NULL },
  {"4", "Ê¸", "Ê¸", "", NULL },
  {"4", "ÅÍ", "ÅÍ", "", NULL },
  {"4", "¶Ô", "¶Ô", "", NULL },
  {"4", "Êý", "Êý", "", NULL },
  {"4", "ÚÛ", "ÚÛ", "", NULL },
  {"4", "Æü", "Æü", "", NULL },
  {"4", "Û©", "Û©", "", NULL },
  {"4", "·î", "·î", "", NULL },
  {"4", "ÌÚ", "ÌÚ", "", NULL },
  {"4", "·ç", "·ç", "", NULL },
  {"4", "»ß", "»ß", "", NULL },
  {"4", "ÝÆ", "ÝÆ", "", NULL },
  {"4", "ÝÕ", "ÝÕ", "", NULL },
  {"4", "Êì", "Êì", "", NULL },
  {"4", "ÝÙ", "ÝÙ", "", NULL },
  {"4", "Èæ", "Èæ", "", NULL },
  {"4", "ÌÓ", "ÌÓ", "", NULL },
  {"4", "»á", "»á", "", NULL },
  {"4", "Ýã", "Ýã", "", NULL },
  {"4", "¿å", "¿å", "", NULL },
  {"4", "²Ð", "²Ð", "", NULL },
  {"4", "Û¿", "Û¿", "", NULL },
  {"4", "ÄÞ", "ÄÞ", "", NULL },
  {"4", "Éã", "Éã", "", NULL },
  {"4", "à«", "à«", "", NULL },
  {"4", "à­", "à­", "", NULL },
  {"4", "ÊÒ", "ÊÒ", "", NULL },
  {"4", "²ç", "²ç", "", NULL },
  {"4", "µí", "µí", "", NULL },
  {"4", "¸¤", "¸¤", "", NULL },
  {"4", "²¦", "²¦", "", NULL },
  {"4", "Îé", "Îé", "", NULL },
  {"4", "Ï·", "Ï·", "", NULL },
  {"4", "Ï¾", "Ï¾", "", NULL },
  {"4", "ÌÞ", "ÌÞ", "", NULL },
  {"4", "°æ", "°æ", "", NULL },
  {"4", "Ìà", "Ìà", "", NULL },
  {"4", "¸Þ", "¸Þ", "", NULL },
  {"4", "ÇÃ", "ÇÃ", "", NULL },
  {"4", "ÆÖ", "ÆÖ", "", NULL },
  {"4", "¸µ", "¸µ", "", NULL },

  {"5", "¸¼", "¸¼", "", NULL },
  {"5", "¶Ì", "¶Ì", "", NULL },
  {"5", "±»", "±»", "", NULL },
  {"5", "´¤", "´¤", "", NULL },
  {"5", "´Å", "´Å", "", NULL },
  {"5", "À¸", "À¸", "", NULL },
  {"5", "ÍÑ", "ÍÑ", "", NULL },
  {"5", "ÅÄ", "ÅÄ", "", NULL },
  {"5", "É¥", "É¥", "", NULL },
  {"5", "áË", "áË", "", NULL },
  {"5", "â¢", "â¢", "", NULL },
  {"5", "Çò", "Çò", "", NULL },
  {"5", "Èé", "Èé", "", NULL },
  {"5", "»®", "»®", "", NULL },
  {"5", "ÌÜ", "ÌÜ", "", NULL },
  {"5", "Ì·", "Ì·", "", NULL },
  {"5", "Ìð", "Ìð", "", NULL },
  {"5", "ÀÐ", "ÀÐ", "", NULL },
  {"5", "¼¨", "¼¨", "", NULL },
  {"5", "ã»", "ã»", "", NULL },
  {"5", "²Ó", "²Ó", "", NULL },
  {"5", "·ê", "·ê", "", NULL },
  {"5", "Î©", "Î©", "", NULL },
  {"5", "Çã", "Çã", "", NULL },
  {"5", "½é", "½é", "", NULL },
  {"5", "µð", "µð", "", NULL },
  {"5", "À¤", "À¤", "", NULL },
  {"5", "ºý", "ºý", "", NULL },

  {"6", "ÃÝ", "ÃÝ", "", NULL },
  {"6", "ÊÆ", "ÊÆ", "", NULL },
  {"6", "»å", "»å", "", NULL },
  {"6", "´Ì", "´Ì", "", NULL },
  {"6", "ÍÓ", "ÍÓ", "", NULL },
  {"6", "±©", "±©", "", NULL },
  {"6", "Ï·", "Ï·", "", NULL },
  {"6", "¼©", "¼©", "", NULL },
  {"6", "æÐ", "æÐ", "", NULL },
  {"6", "¼ª", "¼ª", "", NULL },
  {"6", "ææ", "ææ", "", NULL },
  {"6", "Æù", "Æù", "", NULL },
  {"6", "¼«", "¼«", "", NULL },
  {"6", "»ê", "»ê", "", NULL },
  {"6", "±±", "±±", "", NULL },
  {"6", "Àå", "Àå", "", NULL },
  {"6", "Á¤", "Á¤", "", NULL },
  {"6", "½®", "½®", "", NULL },
  {"6", "º±", "º±", "", NULL },
  {"6", "¿§", "¿§", "", NULL },
  {"6", "éÈ", "éÈ", "", NULL },
  {"6", "Ãî", "Ãî", "", NULL },
  {"6", "·ì", "·ì", "", NULL },
  {"6", "¹Ô", "¹Ô", "", NULL },
  {"6", "°á", "°á", "", NULL },
  {"6", "À¾", "À¾", "", NULL },

  {"7", "¿Ã", "¿Ã", "", NULL },
  {"7", "¸«", "¸«", "", NULL },
  {"7", "³Ñ", "³Ñ", "", NULL },
  {"7", "¸À", "¸À", "", NULL },
  {"7", "Ã«", "Ã«", "", NULL },
  {"7", "Æ¦", "Æ¦", "", NULL },
  {"7", "ìµ", "ìµ", "", NULL },
  {"7", "ì¸", "ì¸", "", NULL },
  {"7", "³­", "³­", "", NULL },
  {"7", "ÀÖ", "ÀÖ", "", NULL },
  {"7", "Áö", "Áö", "", NULL },
  {"7", "Â­", "Â­", "", NULL },
  {"7", "¿È", "¿È", "", NULL },
  {"7", "¼Ö", "¼Ö", "", NULL },
  {"7", "¿É", "¿É", "", NULL },
  {"7", "Ã¤", "Ã¤", "", NULL },
  {"7", "ÆÓ", "ÆÓ", "", NULL },
  {"7", "ÈÐ", "ÈÐ", "", NULL },
  {"7", "Î¤", "Î¤", "", NULL },
  {"7", "Çþ", "Çþ", "", NULL },
  {"8", "¶â", "¶â", "", NULL },
  {"8", "Ä¹", "Ä¹", "", NULL },
  {"8", "Ìç", "Ìç", "", NULL },
  {"8", "ð°", "ð°", "", NULL },
  {"8", "ð²", "ð²", "", NULL },
  {"8", "±«", "±«", "", NULL },
  {"8", "ÀÄ", "ÀÄ", "", NULL },
  {"8", "Èó", "Èó", "", NULL },
  {"8", "ÀÆ", "ÀÆ", "", NULL },
  {"8", "²¬", "²¬", "", NULL },
  {"8", "±â", "±â", "", NULL },
  {"8", "ÌÈ", "ÌÈ", "", NULL },

  {"9", "ÌÌ", "ÌÌ", "", NULL },
  {"9", "³×", "³×", "", NULL },
  {"9", "ðê", "ðê", "", NULL },
  {"9", "²»", "²»", "", NULL },
  {"9", "ÊÇ", "ÊÇ", "", NULL },
  {"9", "É÷", "É÷", "", NULL },
  {"9", "Èô", "Èô", "", NULL },
  {"9", "¿©", "¿©", "", NULL },
  {"9", "¼ó", "¼ó", "", NULL },
  {"9", "¹á", "¹á", "", NULL },
  {"9", "ÉÊ", "ÉÊ", "", NULL },

  {"10", "ÇÏ", "ÇÏ", "", NULL },
  {"10", "¹ü", "¹ü", "", NULL },
  {"10", "¹â", "¹â", "", NULL },
  {"10", "ñõ", "ñõ", "", NULL },
  {"10", "ò¨", "ò¨", "", NULL },
  {"10", "ò®", "ò®", "", NULL },
  {"10", "ò¯", "ò¯", "", NULL },
  {"10", "µ´", "µ´", "", NULL },
  {"10", "Îµ", "Îµ", "", NULL },

  {"11", "µû", "µû", "", NULL },
  {"11", "Ä»", "Ä»", "", NULL },
  {"11", "óÃ", "óÃ", "", NULL },
  {"11", "¼¯", "¼¯", "", NULL },
  {"11", "Ëã", "Ëã", "", NULL },
  {"11", "µµ", "µµ", "", NULL },

  {"12", "²«", "²«", "", NULL },
  {"12", "µÐ", "µÐ", "", NULL },
  {"12", "¹õ", "¹õ", "", NULL },
  {"12", "óã", "óã", "", NULL },
  {"12", "Ìµ", "Ìµ", "", NULL },

  {"13", "óæ", "óæ", "", NULL },
  {"13", "¸Ý", "¸Ý", "", NULL },
  {"13", "ÁÍ", "ÁÍ", "", NULL },

  {"14", "É¡", "É¡", "", NULL },
  {"14", "óî", "óî", "", NULL },

  {"17", "óþ", "óþ", "", NULL },

  NULL
};

//
//! @brief Copies all the lables of the depressed buttons in the radicals window
//!
//! @param output The string to copy to
//! @param The max characters to copy
//!
void gw_ui_strcpy_all_selected_radicals(char *output, int *MAX)
{
    char id[50];
    
    GtkWidget *table;
    strcpy(id, "radicals_table");
    table = GTK_WIDGET (gtk_builder_get_object(builder, id));

    GList     *list;
    list  = gtk_container_get_children (GTK_CONTAINER (table));

    int leftover = *MAX;
    const char *label_text = NULL;
    radical_cache[0] = '\0';
    gboolean a_button_was_in_pressed_state = FALSE;

    //Probe all of the active toggle buttons in the table
    while (list != NULL)
    {
      if (G_OBJECT_TYPE(list->data) == g_type_from_name("GtkToggleButton"))
      {
         label_text = gtk_button_get_label( GTK_BUTTON(list->data));
         if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(list->data)))
         {
           a_button_was_in_pressed_state = TRUE;
           strncat(output, label_text, leftover);
           leftover -= strlen(label_text);
         }
         strcat (radical_cache, label_text);
         gtk_widget_set_sensitive (GTK_WIDGET (list->data), FALSE);
      }
      list = list->next;
    }

    if (!a_button_was_in_pressed_state)
      gw_ui_deselect_all_radicals ();

    *MAX = leftover;
}


//!
//! @brief Finds the radical button with the string label and sets it sensitive
//!
//! @param string The label to search for
//!
void gw_ui_set_button_sensitive_when_label_is (const char *string)
{
    GtkWidget *table;
    table = GTK_WIDGET (gtk_builder_get_object(builder, "radicals_table"));

    GList     *list, *it;
    const char *label_text = NULL;

    const char *jump = string;
    char radical[4];
    if (jump[0] != '\0' && jump[1] != '\0' && jump[2] != '\0')
    {
      radical[0] = jump[0];
      radical[1] = jump[1];
      radical[2] = jump[2];
      radical[3] = '\0';

      it = list  = gtk_container_get_children (GTK_CONTAINER (table));
      while (it != NULL)
      {
        if (G_OBJECT_TYPE(it->data) == g_type_from_name("GtkToggleButton"))
        {
           label_text = gtk_button_get_label (GTK_BUTTON(it->data));
           if (strcmp(label_text, radical) == 0)
            gtk_widget_set_sensitive (GTK_WIDGET (it->data), TRUE);
        }
        it = it->next;
      }
      g_list_free(list);
    }
    while ((jump = g_utf8_strchr (jump, -1, L' ')))
    {
      jump++;
      if (jump[0] != '\0' && jump[1] != '\0' && jump[2] != '\0')
      {
        radical[0] = jump[0];
        radical[1] = jump[1];
        radical[2] = jump[2];
        radical[3] = '\0';

        it = list  = gtk_container_get_children (GTK_CONTAINER (table));
        while (it != NULL)
        {
          if (G_OBJECT_TYPE(it->data) == g_type_from_name("GtkToggleButton"))
          {
             label_text = gtk_button_get_label (GTK_BUTTON(it->data));
             if (strcmp(label_text, radical) == 0)
              gtk_widget_set_sensitive (GTK_WIDGET (it->data), TRUE);
          }
          it = it->next;
        }
        g_list_free(list);
      }
    }
}

//!
//! @brief Copies the stroke count in the prefered format
//!
//! @param output The string to copy the prefered stroke count to
//! @param MAX The max characters to copy
void gw_ui_strcpy_prefered_stroke_count(char *output, int *MAX)
{
    char id[50];

    GtkWidget *checkbox;
    strcpy(id, "strokes_checkbox");
    checkbox   = GTK_WIDGET (gtk_builder_get_object(builder, id));

    GtkWidget *spinbutton;
    strcpy(id, "strokes_spinbutton");
    spinbutton = GTK_WIDGET (gtk_builder_get_object(builder, id));

    int leftover = *MAX;

    //If the checkbox is checked, get the stroke count from the spinner
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (checkbox)))
    {
      int strokes;
      strokes = gtk_spin_button_get_value (GTK_SPIN_BUTTON (spinbutton));

      char S[3];
      gw_util_itoa(strokes, S, 3);

      strncat(output, "S", leftover);
      leftover -= 1;

      strncat(output, S, leftover);
      leftover -= strlen(S);
    }

    *MAX = leftover;
}


//!
//! @brief Matches the sensativity of the strokes spinbutton to the stokes checkbox
//!
void gw_ui_update_strokes_checkbox_state()
{
    char id[50];

    //Get the needed variables and references
    GtkWidget *checkbutton;
    strcpy(id, "strokes_checkbox");
    checkbutton = GTK_WIDGET (gtk_builder_get_object(builder, id));

    gboolean enable;
    enable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbutton));

    GtkWidget *spinbutton;
    strcpy(id, "strokes_spinbutton");
    spinbutton = GTK_WIDGET (gtk_builder_get_object(builder, id));
    
    GtkWidget *label;
    strcpy(id, "strokes_checkbox");
    label = GTK_WIDGET (gtk_builder_get_object(builder, id));

    gtk_widget_set_sensitive (spinbutton, enable);
}


//!
//! @brief Resets the states of all the radical buttons
//!
void gw_ui_deselect_all_radicals()
{
    char id[50];

    GtkWidget *table;
    strcpy(id, "radicals_table");
    table = GTK_WIDGET (gtk_builder_get_object(builder, id));

    //Reset all of the toggle buttons
    GList* list;
    list = gtk_container_get_children (GTK_CONTAINER (table));

    while (list != NULL)
    {
      g_signal_handlers_block_by_func(list->data, do_radical_search , NULL);
      if (G_OBJECT_TYPE(list->data) == g_type_from_name("GtkToggleButton"))
         gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(list->data), FALSE);
      g_signal_handlers_unblock_by_func(list->data, do_radical_search , NULL);
      gtk_widget_set_sensitive (GTK_WIDGET (list->data), TRUE);

      list = list->next;
    }
}


//!
//! @brief Sets the stroke enable checkbox to a specific state
//!
void gw_ui_set_strokes_checkbox_state (gboolean state)
{
    char id[50];

    GtkWidget *checkbox;
    strcpy(id, "strokes_checkbox");
    checkbox = GTK_WIDGET (gtk_builder_get_object(builder, id));

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (checkbox), state);
}

