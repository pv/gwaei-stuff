#include <gwaei/gwaei.h>
#include <gwaei/searchwindowprivate.h>

void gw_searchwindow_private_init (GwSearchWindow *window)
{
    //Declarations
    GwSearchWindowPrivate *private;
    int i;

    //Initializations
    priv = GW_WINDOW_GET_PRIVATE (window);

    priv->entry = GTK_ENTRY (gw_window_get_object (window, "search_entry"));
    priv->notebook = GTK_NOTEBOOK (gw_window_get_object (window, "notebook"));
    priv->toolbar = GTK_TOOLBAR (gw_window_get_object (window, "toolbar"));
    priv->statusbar = GTK_WIDGET (gw_window_get_object (window, "statusbar"));
    priv->combobox = GTK_COMBO_BOX (gw_window_get_object (window, "dictionary_combobox"));

    gw_searchwindow_initialize_dictionary_combobox (window);
    gw_searchwindow_initialize_dictionary_menu (window);

    priv->dictinfo = NULL;
    priv->tablist = NULL;
    priv->font_size = 0;

    priv->feedbackdata.item = NULL;
    priv->feedbackdata.line = 0;
    priv->feedbackdata.status = LW_SEARCHSTATUS_IDLE;

    for (i = 0; i < TOTAL_GW_SEARCHWINDOW_TIMEOUTIDS; i++)
      priv->timeoutid[i] = 0;

    priv->previous_tip = 0;

    _searchwindow_mousedata_init (&(priv->mousedata));
    _searchwindow_keepsearchingdata_init (&(priv->keepsearchingdata));

    priv->selectionicondata.selected = FALSE;

    priv->new_tab = FALSE; 

    priv->history = lw_history_new (20);
    priv->tagtable = gtk_text_tag_table_new ();
    priv->spellcheck = NULL;

    _searchwindow_attach_signals (window);

    gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
    gw_searchwindow_set_dictionary (window, 0);
    gw_searchwindow_guarantee_first_tab (window);

    //We are going to lazily update the sensitivity of the spellcheck buttons only when the window is created
    GtkToolButton *toolbutton;
    gboolean enchant_exists;

    toolbutton = GTK_TOOL_BUTTON (gw_window_get_object (window, "spellcheck_toolbutton")); 
    enchant_exists = g_file_test (ENCHANT, G_FILE_TEST_IS_REGULAR);

    gtk_widget_set_sensitive (GTK_WIDGET (priv->entry), enchant_exists);
    gtk_widget_set_sensitive (GTK_WIDGET (toolbutton), enchant_exists);

}


void gw_searchwindow_private_finialize (GwSearchWindow *window)
{
    //Declarations
    GSource *source;
    int i;

    for (i = 0; i < TOTAL_GW_SEARCHWINDOW_TIMEOUTIDS; i++)
    {
      if (g_main_current_source () != NULL &&
          !g_source_is_destroyed (g_main_current_source ()) &&
          window->timeoutid[i] > 0
         )
      {
        source = g_main_context_find_source_by_id (NULL, window->timeoutid[i]);
        if (source != NULL)
        {
          g_source_destroy (source);
        }
      }
      window->timeoutid[i] = 0;
    }

    _searchwindow_remove_signals (window);

    gw_searchwindow_cancel_all_searches (window);

    if (window->spellcheck != NULL) gw_spellcheck_free (window->spellcheck);
    if (window->history != NULL) lw_history_free (window->history);
    if (window->tablist != NULL) g_list_free (window->tablist);

    _searchwindow_mousedata_deinit (&(window->mousedata));
    _searchwindow_keepsearchingdata_deinit (&(window->keepsearchingdata));

    g_object_unref (window->tagtable);
}


static void _searchwindow_attach_signals (GwSearchWindow *window)
{
    //Declarations
    int i;

    for (i = 0; i < TOTAL_GW_SEARCHWINDOW_SIGNALIDS; i++)
      window->signalid[i] = 0;

    window->signalid[GW_SEARCHWINDOW_SIGNALID_TOOLBAR_SHOW] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_BASE,
        LW_KEY_TOOLBAR_SHOW,
        gw_searchwindow_sync_toolbar_show_cb,
        window
    );

    window->signalid[GW_SEARCHWINDOW_SIGNALID_STATUSBAR_SHOW] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_BASE,
        LW_KEY_STATUSBAR_SHOW,
        gw_searchwindow_sync_statusbar_show_cb,
        window
    );

    window->signalid[GW_SEARCHWINDOW_SIGNALID_USE_GLOBAL_FONT] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_USE_GLOBAL_FONT,
        gw_searchwindow_sync_font_cb,
        window
    );
    window->signalid[GW_SEARCHWINDOW_SIGNALID_CUSTOM_FONT] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_CUSTOM_FONT,
        gw_searchwindow_sync_font_cb,
        window
    );

    window->signalid[GW_SEARCHWINDOW_SIGNALID_FONT_MAGNIFICATION] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_MAGNIFICATION,
        gw_searchwindow_sync_font_cb,
        window
    );

    window->signalid[GW_SEARCHWINDOW_SIGNALID_KEEP_SEARCHING] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SEARCH_AS_YOU_TYPE,
        gw_searchwindow_sync_search_as_you_type_cb,
        window
    );

    window->signalid[GW_SEARCHWINDOW_SIGNALID_SPELLCHECK] = lw_preferences_add_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SPELLCHECK,
        gw_searchwindow_sync_spellcheck_cb,
        window
    );

    window->signalid[GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_ADDED] = g_signal_connect (
        G_OBJECT (app->dictinfolist->model),
        "row-inserted",
        G_CALLBACK (gw_searchwindow_dictionaries_added_cb),
        window->toplevel 
    );

    window->signalid[GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_DELETED] = g_signal_connect (
        G_OBJECT (app->dictinfolist->model),
        "row-deleted",
        G_CALLBACK (gw_searchwindow_dictionaries_deleted_cb),
        window->toplevel 
    );

    window->timeoutid[GW_SEARCHWINDOW_TIMEOUTID_KEEP_SEARCHING] = gdk_threads_add_timeout (
          100,
          (GSourceFunc) gw_searchwindow_keep_searching_timeout, 
          window
    );

    window->timeoutid[GW_SEARCHWINDOW_TIMEOUTID_PROGRESS] = g_timeout_add_full (
          G_PRIORITY_LOW, 
          50, 
          (GSourceFunc) gw_searchwindow_update_progress_feedback_timeout, 
          window, 
          NULL
    );
}


static void _searchwindow_remove_signals (GwSearchWindow *window)
{
    //Declarations
    int i;

    lw_preferences_remove_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_BASE,
        window->signalid[GW_SEARCHWINDOW_SIGNALID_TOOLBAR_SHOW]
    );
    lw_preferences_remove_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_BASE,
        window->signalid[GW_SEARCHWINDOW_SIGNALID_STATUSBAR_SHOW]
    );
    lw_preferences_remove_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_FONT,
        window->signalid[GW_SEARCHWINDOW_SIGNALID_USE_GLOBAL_FONT]
    );
    lw_preferences_remove_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_FONT,
        window->signalid[GW_SEARCHWINDOW_SIGNALID_CUSTOM_FONT]
    );
    lw_preferences_remove_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_FONT,
        window->signalid[GW_SEARCHWINDOW_SIGNALID_FONT_MAGNIFICATION]
    );
    lw_preferences_remove_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_BASE,
        window->signalid[GW_SEARCHWINDOW_SIGNALID_KEEP_SEARCHING]
    );
    lw_preferences_remove_change_listener_by_schema (
        app->preferences,
        LW_SCHEMA_BASE,
        window->signalid[GW_SEARCHWINDOW_SIGNALID_SPELLCHECK]
    );

    g_signal_handler_disconnect (
        G_OBJECT (app->dictinfolist->model),
        window->signalid[GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_ADDED]
    );

    g_signal_handler_disconnect (
        G_OBJECT (app->dictinfolist->model),
        window->signalid[GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_DELETED]
    );


    for (i = 0; i < TOTAL_GW_SEARCHWINDOW_SIGNALIDS; i++)
      window->signalid[i] = 0;
}



void _searchwindow_mousedata_init (GwSearchWindowMouseData *data)
{
    data->item = NULL;
    data->button_press_x = 0;
    data->button_press_y = 0;
    data->button_character = 0;
    data->hovered_word = NULL; 
}

void _searchwindow_mousedata_deinit (GwSearchWindowMouseData *data)
{
  if (data->hovered_word != NULL)
  {
    g_free (data->hovered_word);
    data->hovered_word = NULL;
  }
}

void _searchwindow_keepsearchingdata_init (GwSearchWindowKeepSearchingData *data)
{
    data->delay = 0;
    data->query = NULL;
    data->enabled = FALSE;
}


void _searchwindow_keepsearchingdata_deinit (GwSearchWindowKeepSearchingData *data)
{
    if (data->query != NULL)
    {
      g_free (data->query);
      data->query = NULL;
    } 
}


void gw_searchwindow_initialize_dictionary_combobox (GwSearchWindow *window)
{
    //Declarations
    GtkCellRenderer *renderer;

    //Initializations
    renderer = gtk_cell_renderer_text_new ();

    gtk_combo_box_set_model (window->combobox, NULL);
    gtk_cell_layout_clear (GTK_CELL_LAYOUT (window->combobox));

    gtk_combo_box_set_model (window->combobox, GTK_TREE_MODEL (app->dictinfolist->model));
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (window->combobox), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (window->combobox), renderer, "text", GW_DICTINFOLIST_COLUMN_LONG_NAME, NULL);
    gtk_combo_box_set_active (window->combobox, 0);
}


void gw_searchwindow_initialize_dictionary_menu (GwSearchWindow *window)
{
    GtkMenuShell *shell;
    GList *list, *iter;
    GtkWidget *widget;

    shell = GTK_MENU_SHELL (gtk_builder_get_object (window->builder, "dictionary_popup"));

    if (shell != NULL)
    {
      list = gtk_container_get_children (GTK_CONTAINER (shell));
      for (iter = list; iter != NULL; iter = iter->next)
      {
        widget = GTK_WIDGET (iter->data);
        if (widget != NULL)
        {
          gtk_widget_destroy(widget);
        }
      }
      g_list_free (list);
    }

    GtkAccelGroup *accel_group;
    GSList *group;
    LwDictInfo *di;

    accel_group = GTK_ACCEL_GROUP (gtk_builder_get_object (window->builder, "main_accelgroup"));
    group = NULL;

    //Refill the menu
    for (iter = app->dictinfolist->list; iter != NULL; iter = iter->next)
    {
      di = LW_DICTINFO (iter->data);
      if (di != NULL)
      {
        widget = GTK_WIDGET (gtk_radio_menu_item_new_with_label (group, di->longname));
        group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (widget));
        gtk_menu_shell_append (GTK_MENU_SHELL (shell),  GTK_WIDGET (widget));
        if (di->load_position == 0)
          gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), TRUE);
        g_signal_connect(G_OBJECT (widget), "toggled", G_CALLBACK (gw_searchwindow_dictionary_radio_changed_cb), window->toplevel);
        if (di->load_position < 9)
          gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", accel_group, (GDK_KEY_0 + di->load_position + 1), GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
        gtk_widget_show (widget);
      }
    }

    //Fill in the other menu items
    widget = GTK_WIDGET (gtk_separator_menu_item_new());
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (widget));
    gtk_widget_show (GTK_WIDGET (widget));

    widget = GTK_WIDGET (gtk_menu_item_new_with_mnemonic(gettext("_Cycle Up")));
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (widget));
    g_signal_connect (G_OBJECT (widget), "activate", G_CALLBACK (gw_searchwindow_cycle_dictionaries_backward_cb), window->toplevel);
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", accel_group, GDK_KEY_Up, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_show (GTK_WIDGET (widget));

    widget = GTK_WIDGET (gtk_menu_item_new_with_mnemonic(gettext("Cycle _Down")));
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (widget));
    g_signal_connect (G_OBJECT (widget), "activate", G_CALLBACK (gw_searchwindow_cycle_dictionaries_forward_cb), window->toplevel);
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", accel_group, GDK_KEY_Down, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_show (GTK_WIDGET (widget));
}


