#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>

void gw_settings_dictionary_manager_update_items (void);


GtkListStore *model = NULL;
GtkTreeView *view = NULL;
enum { IMAGE, POSITION, NAME, LONG_NAME, SHORTCUT, PROGRESS, STATUS, DICT_POINTER, TOTAL_FIELDS };
GtkCellRenderer *renderer;
GtkTreeViewColumn *column;
GtkTreeIter iter;
GtkTreeViewColumn *status_column;


void gw_dictionary_manager_initialize ()
{
    GtkBuilder *builder = gw_common_get_builder ();

    //Setup the model and view
    model = gtk_list_store_new (TOTAL_FIELDS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING, G_TYPE_POINTER);
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

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes ("Shortcut", renderer, "text", SHORTCUT, NULL);
    gtk_tree_view_append_column (view, column);

    renderer = gtk_cell_renderer_progress_new();
    column = gtk_tree_view_column_new_with_attributes ("Status", renderer, "value", PROGRESS, "text", STATUS, NULL);
    gtk_tree_view_append_column (view, column);
    status_column = column;

    GtkWidget *combobox = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_combobox"));
    gtk_combo_box_set_model (GTK_COMBO_BOX (combobox), GTK_TREE_MODEL (model));
    renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combobox), renderer, FALSE);
    gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (combobox), renderer, "text", LONG_NAME);

    gw_settings_dictionary_manager_update_items ();
}


void gw_dictionary_manager_free ()
{
}


void gw_settings_dictionary_manager_update_items ()
{
    GtkBuilder *builder = gw_common_get_builder ();

    //Clear the previous items all over the gui
    gtk_list_store_clear (GTK_LIST_STORE (model));

    GtkMenuShell *shell = GTK_MENU_SHELL (gtk_builder_get_object (builder, "dictionary_popup"));
    if (shell != NULL)
    {
      GList     *children = NULL;
      children = gtk_container_get_children (GTK_CONTAINER (shell));
      while (children != NULL )
      {
        gtk_widget_destroy(children->data);
        children = g_list_delete_link(children, children);
      }
    }



    //Start filling in the new items
    GwDictInfo *di = NULL;
    GtkTreeIter iter;
    char *dictionary_name = NULL;
    char *long_name = NULL;
    char *icon_name = NULL;
    char *shortcut_name = NULL;
    char *order_number = NULL;
    char *favorite_icon = "emblem-favorite";
    GList *dli = NULL;
    GtkAccelGroup* accel_group = GTK_ACCEL_GROUP (gtk_builder_get_object (builder, "main_accelgroup"));
    GSList* group = NULL;
    GtkWidget *item = NULL;

    
    for (dli = gw_dictlist_get_list(); dli != NULL; dli = dli->next)
    {
      //Recreate the liststore
      di = (GwDictInfo*) dli->data;
      if (di->load_position == 0) icon_name = favorite_icon;
      if (di->load_position < 9) shortcut_name = g_strdup_printf ("Alt-%d", (di->load_position + 1));
      order_number = g_strdup_printf ("%d", (di->load_position + 1));
      gtk_list_store_append (GTK_LIST_STORE (model), &iter);
      gtk_list_store_set (model, &iter,
                          IMAGE, icon_name,
                          POSITION, order_number,
                          NAME, di->short_name,
                          LONG_NAME, di->long_name,
                          SHORTCUT, shortcut_name,
                          PROGRESS, 100,
                          STATUS, "complete",
                          DICT_POINTER, di,
                                                 -1);

      //Refill the menu
      item = GTK_WIDGET (gtk_radio_menu_item_new_with_label (group, di->long_name));
      group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (item));
      gtk_menu_shell_append (GTK_MENU_SHELL (shell),  GTK_WIDGET (item));
      if (di->load_position == 0) gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), TRUE);
      g_signal_connect(G_OBJECT (item), "toggled", G_CALLBACK (do_dictionary_changed_action), NULL);
      if (di->load_position < 9) gtk_widget_add_accelerator (GTK_WIDGET (item), "activate", accel_group, (GDK_0 + di->load_position + 1), GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
      gtk_widget_show (item);



      g_free (order_number);
      order_number = NULL;
      icon_name = NULL;
      g_free (shortcut_name);
      shortcut_name = NULL;
    }

    //Fill in the other menu items
    item = GTK_WIDGET (gtk_separator_menu_item_new());
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (item));
    gtk_widget_show (GTK_WIDGET (item));

    item = GTK_WIDGET (gtk_menu_item_new_with_mnemonic(gettext("_Cycle Up")));
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (item));
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (do_cycle_dictionaries_backward), NULL);
    gtk_widget_add_accelerator (GTK_WIDGET (item), "activate", accel_group, GDK_Up, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_show (GTK_WIDGET (item));

    item = GTK_WIDGET (gtk_menu_item_new_with_mnemonic(gettext("Cycle _Down")));
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (item));
    g_signal_connect (G_OBJECT (item), "activate", G_CALLBACK (do_cycle_dictionaries_forward), NULL);
    gtk_widget_add_accelerator (GTK_WIDGET (item), "activate", accel_group, GDK_Down, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_show (GTK_WIDGET (item));

    GtkWidget *combobox = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_combobox"));
    gtk_combo_box_set_active (GTK_COMBO_BOX (combobox), 0);
}



G_MODULE_EXPORT void do_dictionary_cursor_changed_action (GtkTreeView *treeview, 
                                                              gpointer data     )
{
    GtkBuilder *builder = gw_common_get_builder ();

    GtkWidget *button = GTK_WIDGET (gtk_builder_get_object (builder, "remove_dictionary_button"));
    GtkTreeSelection *selection = gtk_tree_view_get_selection (view);
    GtkTreeIter iter;

    GtkTreeModel *tmodel = GTK_TREE_MODEL (model);
    gboolean has_selection = gtk_tree_selection_get_selected (selection, &tmodel, &iter);
    gtk_widget_set_sensitive (GTK_WIDGET (button), has_selection);
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

G_MODULE_EXPORT void do_remove_dictionary_action (GtkWidget *widget, gpointer data)
{
    GtkBuilder *builder = gw_common_get_builder ();
    printf("BREAK1\n");

    GtkWidget *button = GTK_WIDGET (gtk_builder_get_object (builder, "remove_dictionary_button"));
    GtkTreeViewColumn *focus_column = NULL;
    GtkTreePath *path = NULL;
    GtkTreeIter iter;
    GList *list = NULL;

    GtkTreeSelection *selection = gtk_tree_view_get_selection (view);
    GtkTreeModel *tmodel = GTK_TREE_MODEL (model);
    gboolean has_selection = gtk_tree_selection_get_selected (selection, &tmodel, &iter);
    if (!has_selection) return;

    path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), &iter);
    gint* indices = gtk_tree_path_get_indices (path);
    list = gw_dictlist_get_dict_by_load_position (*indices);
    gtk_tree_path_free (path);
    path = NULL;

    if (list != NULL)
    {
      GwDictInfo *di = list->data;
      gw_io_uninstall_dictinfo (di, NULL, NULL, TRUE);
      gw_settings_dictionary_manager_update_items ();
    }

    gtk_widget_set_sensitive (GTK_WIDGET (button), FALSE);
    gw_ui_update_settings_interface ();
}

