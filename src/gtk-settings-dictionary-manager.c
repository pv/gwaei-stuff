#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>
#include <gwaei/preferences.h>

#include <gwaei/gtk.h>
#include <gwaei/gtk-settings-callbacks.h>
#include <gwaei/gtk-main-interface.h>
#include <gwaei/gtk-settings-interface.h>

static void _get_parsed_dictionary_order_array (char*, int, char**, int);
void gw_settings_dictionary_manager_update_items (void);


GtkListStore *model = NULL;
GtkTreeView *view = NULL;
enum { IMAGE, POSITION, NAME, LONG_NAME, PROGRESS, STATUS, TOTAL_FIELDS };
GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
GtkTreeIter iter;
GtkTreeViewColumn *status_column;

void gw_dictionary_manager_init ()
{
    //Setup the model and view
    model = gtk_list_store_new (TOTAL_FIELDS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING);
    view = GTK_TREE_VIEW (gtk_builder_get_object (builder, "manage_dictionaries_treeview"));
    gtk_tree_view_set_model (GTK_TREE_VIEW (view), GTK_TREE_MODEL (model));

    //Create the columns and renderer for each column
    renderer = gtk_cell_renderer_pixbuf_new();
    column = gtk_tree_view_column_new_with_attributes (" ", renderer, "icon-name", IMAGE, NULL);
    gtk_tree_view_append_column (view, column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes ("#", renderer, "text", POSITION, NULL);
    gtk_tree_view_append_column (view, column);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes ("Dictionary Name", renderer, "text", NAME, NULL);
    gtk_tree_view_append_column (view, column);

    renderer = gtk_cell_renderer_progress_new();
    column = gtk_tree_view_column_new_with_attributes ("Status", renderer, "value", PROGRESS, "text", STATUS, NULL);
    gtk_tree_view_append_column (view, column);
    status_column = column;

    gw_settings_dictionary_manager_update_items ();
}

void gw_settings_dictionary_manager_update_items ()
{
    gtk_list_store_clear (GTK_LIST_STORE (model));

    //Start filling in the new items
    GwDictInfo *di = NULL, *di_alias = NULL, *di_name = NULL;
    GtkTreeIter iter;
    int j = 0, i = 0;
    char *dictionary_name = NULL;
    char *long_name = NULL;
    char *icon_name = NULL;
    char *shortcut_name = NULL;
    char *order_number = NULL;
    char *favorite_icon = "emblem-favorite";

    //Get an array of the dictionaries
    char order[5000];
    char *names[50];
    _get_parsed_dictionary_order_array (order, 5000, names, 50);

    while (names[i] != NULL)
    {
      di_alias = gw_dictlist_get_dictinfo_by_alias (names[i]);
      di_name = gw_dictlist_get_dictinfo_by_name (names[i]);
      di = di_alias;
      if (strcmp(di_alias->name, di_name->name) == 0 && di_alias->status == GW_DICT_STATUS_INSTALLED)
      {
        printf("%d %s\n", j, di_alias->long_name);

        dictionary_name = di_alias->short_name;
        long_name = di_alias->long_name;
        if (j == 0)
          icon_name = favorite_icon;
        else
          icon_name = NULL;
        if (j < 10)
          shortcut_name = g_strdup_printf ("Alt-%d", j + 1);

        order_number = g_strdup_printf ("%d", j + 1);

        //Refill the combobox
        gtk_list_store_append (GTK_LIST_STORE (model), &iter);
        gtk_list_store_set (model, &iter,
                            IMAGE, icon_name,
                            POSITION, order_number,
                            NAME, dictionary_name,
                            LONG_NAME, long_name,
                            PROGRESS, 100,
                            STATUS, "complete",
                                                   -1);

        //Free allocated momory
        if (shortcut_name != NULL)
        {
          g_free (shortcut_name);
          shortcut_name = NULL;
        }
        if (order_number != NULL)
        {
          g_free (order_number);
          order_number = NULL;
        }

        di->load_position = j;
        j++;
      }

      i++;
    }
}



static void _get_parsed_dictionary_order_array (char *new_order, int n, char **names, int m)
{
    //Parse the string
    char order[n];
    GwDictInfo* di = NULL;
    gw_pref_get_string (order, GW_SCHEMA_DICTIONARY, GW_KEY_LOAD_ORDER, n);

    char *mix_name = NULL, *kanji_name = NULL, *radicals_name = NULL;
    names[0] = order;
    int i = 0;
    while ((names[i + 1] = g_utf8_strchr (names[i], -1, L',')) && i < m)
    {
      i++;
      *names[i] = '\0';
      names[i]++;
    }
    names[i + 1] = NULL;

    //Add any missing dictionaries
    int j = 0;
    GList *list = gw_dictlist_get_list ();
    while (list != NULL)
    {
      di = list->data;
      j = 0;
      while (names[j] != NULL && strcmp(di->name, names[j]) != 0)
        j++;

      if (names[j] == NULL && j > 0 && di->status == GW_DICT_STATUS_INSTALLED)
      {
        names[j] = names[j - 1];
        while (*names[j] != '\0') names[j]++;
        names[j]++;
        strcpy(names[j], di->name);
        names[j + 1] = NULL;
      }
      list = g_list_next (list);
    }

    //Remove not installed dictionaries from the list
    i = 0;
    while (names[i] != NULL)
    {
      di = gw_dictlist_get_dictinfo_by_name (names[i]);
      if (di == NULL)
        *names[i] = '\0';
      i++;
    }

    //Collapse the holes
    j = 0;
    i = 0;
    new_order[0] = '\0';
    while (names[i] != NULL && names[j] != NULL)
    {
      if (*names[j] == '\0')
      {
        j++;
      }
      else if (*names[j] != '\0')
      {
        names[i] = names[j];
        strcat(new_order, names[j]);
        strcat(new_order, ",");
        i++;
        j++;
      }
    }
    new_order[strlen(new_order) - 1] = '\0';
    names[i] = NULL;
    gw_pref_set_string (GW_SCHEMA_DICTIONARY, GW_KEY_LOAD_ORDER, new_order);
}











/*
    GtkListStore *liststore;
    liststore = GTK_LIST_STORE (gtk_builder_get_object (builder, "list_store_dictionaries"));
    GtkWidget *treeview;
    treeview = GTK_WIDGET (gtk_builder_get_object (builder, "organize_dictionaries_treeview"));
    GtkTreeModel *model;
    model = gtk_tree_view_get_model (GTK_TREE_VIEW (treeview));
    GtkTreeSelection * selection;
    selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeview));
   
    GtkTreeIter selection_iter;
    if (gtk_tree_selection_get_selected (selection, &model, &selection_iter))
    {
      GtkTreePath *selection_path;
      selection_path = gtk_tree_model_get_path (model, &selection_iter);

      GtkTreeIter comparison_iter;
      gtk_tree_model_get_iter_first (model, &comparison_iter);
      GtkTreePath *comparison_path;
      comparison_path = gtk_tree_model_get_path (model, &comparison_iter);

      gtk_widget_set_sensitive (move_dictionary_up, gtk_tree_path_compare (selection_path, comparison_path) != 0);

      gtk_tree_path_free (comparison_path);
      comparison_path = NULL;
      GtkTreeIter previous_iter;
      while (gtk_tree_model_iter_next (model, &comparison_iter)) previous_iter = comparison_iter;
      comparison_iter = previous_iter;
      comparison_path = gtk_tree_model_get_path (model, &comparison_iter);

      gtk_widget_set_sensitive (move_dictionary_down, gtk_tree_path_compare (selection_path, comparison_path) != 0);

      gtk_tree_path_free (comparison_path);
      comparison_path = NULL;
      gtk_tree_path_free (selection_path);
      selection_path = NULL;
    }
    else
    {
      gtk_widget_set_sensitive (move_dictionary_up, FALSE);
      gtk_widget_set_sensitive (move_dictionary_down, FALSE);
    }
*/
