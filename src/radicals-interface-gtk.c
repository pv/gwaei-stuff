/*****************************************************************************

  FILE:
  src/radicals-interface-gtk.c

  DESCRIPTION:
  Used as a go between for functions interacting with GUI interface objects.
  This is the gtk version.

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

#include <string.h>
#include <regex.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gtk.h>
#include <gwaei/definitions.h>
#include <gwaei/dictionaries.h>
#include <gwaei/history.h>
#include <gwaei/interface.h>
#include <gwaei/callbacks.h>


void gwaei_ui_strcpy_all_selected_radicals(char *output, int *MAX)
{
    char id[50];
    
    GtkWidget *table;
    strcpy(id, "radicals_table");
    table = GTK_WIDGET (gtk_builder_get_object(builder, id));

    GList     *list;
    list  = gtk_container_get_children (GTK_CONTAINER (table));

    int leftover = *MAX;
    const char *label_text = NULL;

    //Probe all of the active toggle buttons in the table
    while (list != NULL)
    {
      if (G_OBJECT_TYPE(list->data) == g_type_from_name("GtkToggleButton"))
      {
         if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(list->data)))
         {
           label_text = gtk_button_get_label( GTK_BUTTON(list->data));
           strncat(output, label_text, leftover);
           leftover -= strlen(label_text);
         }
      }
      list = list->next;
    }

    *MAX = leftover;
}


void gwaei_ui_strcpy_prefered_stroke_count(char *output, int *MAX)
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
      gwaei_itoa(strokes, S, 3);

      strncat(output, "S", leftover);
      leftover -= 1;

      strncat(output, S, leftover);
      leftover -= strlen(S);
    }

    *MAX = leftover;
}


void gwaei_ui_update_strokes_checkbox_state()
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


void gwaei_ui_deselect_all_radicals()
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

      list = list->next;
    }
}


void gwaei_ui_set_strokes_checkbox_state (gboolean state)
{
    char id[50];

    GtkWidget *checkbox;
    strcpy(id, "strokes_checkbox");
    checkbox = GTK_WIDGET (gtk_builder_get_object(builder, id));

    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON (checkbox), state);
}

