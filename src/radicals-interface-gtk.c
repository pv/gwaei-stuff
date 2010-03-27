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

enum radical_fields
{
  STROKES,
  REPRESENTATIVE_RADICAL,
  ACTUAL_RADICAL,
  RADICAL_NAME,
  TOTAL_FIELDS
};

static char *radical_array[][5] =
{
  //{Strokes, Representative_radical, Actual_radical, Name, NULL}}
  {"1", "｜", "｜", "", NULL },
  {"1", "丶", "丶", "", NULL },
  {"1", "ノ", "ノ", "", NULL },
  {"1", "乙", "乙", "", NULL },
  {"1", "亅", "亅", "", NULL },

  {"2", "二", "二", "", NULL },
  {"2", "亠", "亠", "", NULL },
  {"2", "人", "人", "", NULL },
  {"2", "化", "化", "", NULL },
  {"2", "个", "个", "", NULL },
  {"2", "儿", "儿", "", NULL },
  {"2", "入", "入", "", NULL },
  {"2", "ハ", "ハ", "", NULL },
  {"2", "冂", "冂", "", NULL },
  {"2", "冖", "冖", "", NULL },
  {"2", "冫", "冫", "", NULL },
  {"2", "几", "几", "", NULL },
  {"2", "凵", "凵", "", NULL },
  {"2", "刀", "刀", "", NULL },
  {"2", "刈", "刈", "", NULL },
  {"2", "力", "力", "", NULL },
  {"2", "勹", "勹", "", NULL },
  {"2", "匕", "匕", "", NULL },
  {"2", "匚", "匚", "", NULL },
  {"2", "十", "十", "", NULL },
  {"2", "卜", "卜", "", NULL },
  {"2", "卩", "卩", "", NULL },
  {"2", "厂", "厂", "", NULL },
  {"2", "厶", "厶", "", NULL },
  {"2", "又", "又", "", NULL },
  {"2", "込", "込", "", NULL },
  {"2", "九", "九", "", NULL },
  {"2", "マ", "マ", "", NULL },
  {"2", "ユ", "ユ", "", NULL },
  {"2", "乃", "乃", "", NULL },

  {"3", "口", "口", "", NULL },
  {"3", "囗", "囗", "", NULL },
  {"3", "土", "土", "", NULL },
  {"3", "士", "士", "", NULL },
  {"3", "夂", "夂", "", NULL },
  {"3", "夕", "夕", "", NULL },
  {"3", "大", "大", "", NULL },
  {"3", "女", "女", "", NULL },
  {"3", "子", "子", "", NULL },
  {"3", "宀", "宀", "", NULL },
  {"3", "寸", "寸", "", NULL },
  {"3", "小", "小", "", NULL },
  {"3", "尚", "尚", "", NULL },
  {"3", "尢", "尢", "", NULL },
  {"3", "尸", "尸", "", NULL },
  {"3", "屮", "屮", "", NULL },
  {"3", "山", "山", "", NULL },
  {"3", "川", "川", "", NULL },
  {"3", "巛", "巛", "", NULL },
  {"3", "工", "工", "", NULL },
  {"3", "已", "已", "", NULL },
  {"3", "巾", "巾", "", NULL },
  {"3", "干", "干", "", NULL },
  {"3", "幺", "幺", "", NULL },
  {"3", "广", "广", "", NULL },
  {"3", "廴", "廴", "", NULL },
  {"3", "廾", "廾", "", NULL },
  {"3", "弋", "弋", "", NULL },
  {"3", "弓", "弓", "", NULL },
  {"3", "彑", "彑", "", NULL },
  {"3", "ヨ", "ヨ", "", NULL },
  {"3", "彡", "彡", "", NULL },
  {"3", "彳", "彳", "", NULL },
  {"3", "忙", "忙", "", NULL },
  {"3", "扎", "扎", "", NULL },
  {"3", "汁", "汁", "", NULL },
  {"3", "犯", "犯", "", NULL },
  {"3", "艾", "艾", "", NULL },
  {"3", "込", "込", "", NULL },
  {"3", "邦", "邦", "", NULL },
  {"3", "阡", "阡", "", NULL },
  {"3", "并", "并", "", NULL },
  {"3", "也", "也", "", NULL },
  {"3", "亡", "亡", "", NULL },
  {"3", "及", "及", "", NULL },
  {"3", "久", "久", "", NULL },

  {"4", "心", "心", "", NULL },
  {"4", "戈", "戈", "", NULL },
  {"4", "戸", "戸", "", NULL },
  {"4", "手", "手", "", NULL },
  {"4", "支", "支", "", NULL },
  {"4", "攵", "攵", "", NULL },
  {"4", "文", "文", "", NULL },
  {"4", "斗", "斗", "", NULL },
  {"4", "斤", "斤", "", NULL },
  {"4", "方", "方", "", NULL },
  {"4", "无", "无", "", NULL },
  {"4", "日", "日", "", NULL },
  {"4", "曰", "曰", "", NULL },
  {"4", "月", "月", "", NULL },
  {"4", "木", "木", "", NULL },
  {"4", "欠", "欠", "", NULL },
  {"4", "止", "止", "", NULL },
  {"4", "歹", "歹", "", NULL },
  {"4", "殳", "殳", "", NULL },
  {"4", "母", "母", "", NULL },
  {"4", "毋", "毋", "", NULL },
  {"4", "比", "比", "", NULL },
  {"4", "毛", "毛", "", NULL },
  {"4", "氏", "氏", "", NULL },
  {"4", "气", "气", "", NULL },
  {"4", "水", "水", "", NULL },
  {"4", "火", "火", "", NULL },
  {"4", "杰", "杰", "", NULL },
  {"4", "爪", "爪", "", NULL },
  {"4", "父", "父", "", NULL },
  {"4", "爻", "爻", "", NULL },
  {"4", "爿", "爿", "", NULL },
  {"4", "片", "片", "", NULL },
  {"4", "牙", "牙", "", NULL },
  {"4", "牛", "牛", "", NULL },
  {"4", "犬", "犬", "", NULL },
  {"4", "王", "王", "", NULL },
  {"4", "礼", "礼", "", NULL },
  {"4", "老", "老", "", NULL },
  {"4", "肋", "肋", "", NULL },
  {"4", "勿", "勿", "", NULL },
  {"4", "井", "井", "", NULL },
  {"4", "尤", "尤", "", NULL },
  {"4", "五", "五", "", NULL },
  {"4", "巴", "巴", "", NULL },
  {"4", "屯", "屯", "", NULL },
  {"4", "元", "元", "", NULL },

  {"5", "玄", "玄", "", NULL },
  {"5", "玉", "玉", "", NULL },
  {"5", "瓜", "瓜", "", NULL },
  {"5", "瓦", "瓦", "", NULL },
  {"5", "甘", "甘", "", NULL },
  {"5", "生", "生", "", NULL },
  {"5", "用", "用", "", NULL },
  {"5", "田", "田", "", NULL },
  {"5", "疋", "疋", "", NULL },
  {"5", "疔", "疔", "", NULL },
  {"5", "癶", "癶", "", NULL },
  {"5", "白", "白", "", NULL },
  {"5", "皮", "皮", "", NULL },
  {"5", "皿", "皿", "", NULL },
  {"5", "目", "目", "", NULL },
  {"5", "矛", "矛", "", NULL },
  {"5", "矢", "矢", "", NULL },
  {"5", "石", "石", "", NULL },
  {"5", "示", "示", "", NULL },
  {"5", "禹", "禹", "", NULL },
  {"5", "禾", "禾", "", NULL },
  {"5", "穴", "穴", "", NULL },
  {"5", "立", "立", "", NULL },
  {"5", "買", "買", "", NULL },
  {"5", "初", "初", "", NULL },
  {"5", "巨", "巨", "", NULL },
  {"5", "世", "世", "", NULL },
  {"5", "冊", "冊", "", NULL },

  {"6", "竹", "竹", "", NULL },
  {"6", "米", "米", "", NULL },
  {"6", "糸", "糸", "", NULL },
  {"6", "缶", "缶", "", NULL },
  {"6", "羊", "羊", "", NULL },
  {"6", "羽", "羽", "", NULL },
  {"6", "老", "老", "", NULL },
  {"6", "而", "而", "", NULL },
  {"6", "耒", "耒", "", NULL },
  {"6", "耳", "耳", "", NULL },
  {"6", "聿", "聿", "", NULL },
  {"6", "肉", "肉", "", NULL },
  {"6", "自", "自", "", NULL },
  {"6", "至", "至", "", NULL },
  {"6", "臼", "臼", "", NULL },
  {"6", "舌", "舌", "", NULL },
  {"6", "舛", "舛", "", NULL },
  {"6", "舟", "舟", "", NULL },
  {"6", "艮", "艮", "", NULL },
  {"6", "色", "色", "", NULL },
  {"6", "虍", "虍", "", NULL },
  {"6", "虫", "虫", "", NULL },
  {"6", "血", "血", "", NULL },
  {"6", "行", "行", "", NULL },
  {"6", "衣", "衣", "", NULL },
  {"6", "西", "西", "", NULL },

  {"7", "臣", "臣", "", NULL },
  {"7", "見", "見", "", NULL },
  {"7", "角", "角", "", NULL },
  {"7", "言", "言", "", NULL },
  {"7", "谷", "谷", "", NULL },
  {"7", "豆", "豆", "", NULL },
  {"7", "豕", "豕", "", NULL },
  {"7", "豸", "豸", "", NULL },
  {"7", "貝", "貝", "", NULL },
  {"7", "赤", "赤", "", NULL },
  {"7", "走", "走", "", NULL },
  {"7", "足", "足", "", NULL },
  {"7", "身", "身", "", NULL },
  {"7", "車", "車", "", NULL },
  {"7", "辛", "辛", "", NULL },
  {"7", "辰", "辰", "", NULL },
  {"7", "酉", "酉", "", NULL },
  {"7", "釆", "釆", "", NULL },
  {"7", "里", "里", "", NULL },
  {"7", "麦", "麦", "", NULL },
  {"8", "金", "金", "", NULL },
  {"8", "長", "長", "", NULL },
  {"8", "門", "門", "", NULL },
  {"8", "隶", "隶", "", NULL },
  {"8", "隹", "隹", "", NULL },
  {"8", "雨", "雨", "", NULL },
  {"8", "青", "青", "", NULL },
  {"8", "非", "非", "", NULL },
  {"8", "斉", "斉", "", NULL },
  {"8", "岡", "岡", "", NULL },
  {"8", "奄", "奄", "", NULL },
  {"8", "免", "免", "", NULL },

  {"9", "面", "面", "", NULL },
  {"9", "革", "革", "", NULL },
  {"9", "韋", "韋", "", NULL },
  {"9", "音", "音", "", NULL },
  {"9", "頁", "頁", "", NULL },
  {"9", "風", "風", "", NULL },
  {"9", "飛", "飛", "", NULL },
  {"9", "食", "食", "", NULL },
  {"9", "首", "首", "", NULL },
  {"9", "香", "香", "", NULL },
  {"9", "品", "品", "", NULL },

  {"10", "馬", "馬", "", NULL },
  {"10", "骨", "骨", "", NULL },
  {"10", "高", "高", "", NULL },
  {"10", "髟", "髟", "", NULL },
  {"10", "鬥", "鬥", "", NULL },
  {"10", "鬯", "鬯", "", NULL },
  {"10", "鬲", "鬲", "", NULL },
  {"10", "鬼", "鬼", "", NULL },
  {"10", "竜", "竜", "", NULL },

  {"11", "魚", "魚", "", NULL },
  {"11", "鳥", "鳥", "", NULL },
  {"11", "鹵", "鹵", "", NULL },
  {"11", "鹿", "鹿", "", NULL },
  {"11", "麻", "麻", "", NULL },
  {"11", "亀", "亀", "", NULL },

  {"12", "黄", "黄", "", NULL },
  {"12", "黍", "黍", "", NULL },
  {"12", "黒", "黒", "", NULL },
  {"12", "黹", "黹", "", NULL },
  {"12", "無", "無", "", NULL },

  {"13", "黽", "黽", "", NULL },
  {"13", "鼓", "鼓", "", NULL },
  {"13", "鼠", "鼠", "", NULL },

  {"14", "鼻", "鼻", "", NULL },
  {"14", "齊", "齊", "", NULL },

  {"17", "龠", "龠", "", NULL },

  {NULL}
};

void gw_ui_initialize_radicals_table ()
{
  int total_columns = 19;
  GtkTable *table = GTK_TABLE (gtk_builder_get_object (builder, "radical_selection_table"));
  if (table->ncols < total_columns) gtk_table_resize (table, table->nrows, total_columns);
  int rows = table->nrows - 1;
  int i = 0;
  int cols = 0;
  char *stroke = NULL;
  GtkWidget *button = NULL;
  GtkWidget *label = NULL;

  while (radical_array[i][0] != NULL)
  {
    //Add a new stroke label
    if (stroke == NULL || strcmp(stroke, radical_array[i][STROKES]) != 0)
    {
      stroke = radical_array[i][STROKES];
      label = gtk_label_new (stroke);
      char *markup = g_markup_printf_escaped ("<span color=\"red\"><b>%s</b></span>", radical_array[i][STROKES]);
      if (markup != NULL)
      {
        gtk_label_set_markup (GTK_LABEL (label), markup);
        g_free (markup);
      }
      gtk_table_attach (table, label, cols, cols + 1, rows, rows + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
    }
    //Add a radical button
    else
    {
      if (cols == total_columns && rows != 0) gtk_table_resize (table, rows, table->ncols);
      button = gtk_toggle_button_new_with_label (radical_array[i][ACTUAL_RADICAL]);
      g_signal_connect(button, "toggled", G_CALLBACK (do_radical_search), NULL);
      gtk_table_attach (table, button, cols, cols + 1, rows, rows + 1, GTK_EXPAND | GTK_FILL, GTK_EXPAND | GTK_FILL, 0, 0);
      i++;
    }

    cols++;
    if (cols == total_columns + 1)
    {
      rows++;
      cols = 0;
    }
  }
  gtk_widget_show_all (GTK_WIDGET (table));

}

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
    strcpy(id, "radical_selection_table");
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
    table = GTK_WIDGET (gtk_builder_get_object(builder, "radical_selection_table"));

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
    strcpy(id, "radical_selection_table");
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

