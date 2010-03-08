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

