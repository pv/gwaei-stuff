#include <libintl.h>
#include <gwaei/gwaei.h>
#include <gwaei/search-private.h>

static void _searchwindow_attach_signals (GwSearchWindow*);
static void _searchwindow_remove_signals (GwSearchWindow*);

void gw_searchwindow_private_init (GwSearchWindow *window)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    int i;

    //Initializations
    priv = GW_SEARCHWINDOW_GET_PRIVATE (window);

    priv->entry = GTK_ENTRY (gw_window_get_object (GW_WINDOW (window), "search_entry"));
    priv->notebook = GTK_NOTEBOOK (gw_window_get_object (GW_WINDOW (window), "notebook"));
    priv->toolbar = GTK_TOOLBAR (gw_window_get_object (GW_WINDOW (window), "toolbar"));
    priv->statusbar = GTK_WIDGET (gw_window_get_object (GW_WINDOW (window), "statusbar"));
    priv->combobox = GTK_COMBO_BOX (gw_window_get_object (GW_WINDOW (window), "dictionary_combobox"));
    priv->accel_group = gtk_accel_group_new ();

    gw_searchwindow_initialize_dictionary_combobox (window);
    gw_searchwindow_initialize_dictionary_menu (window);

    priv->dictinfo = NULL;
    priv->tablist = NULL;
    priv->font_size = 0;

    priv->feedback_item = NULL;
    priv->feedback_line = 0;
    priv->feedback_status = LW_SEARCHSTATUS_IDLE;

    for (i = 0; i < TOTAL_GW_SEARCHWINDOW_TIMEOUTIDS; i++)
      priv->timeoutid[i] = 0;

    priv->previous_tip = 0;

    //Mouse initialize
    priv->mouse_item = NULL;
    priv->mouse_button_press_x = 0;
    priv->mouse_button_press_y = 0;
    priv->mouse_button_character = 0;
    priv->mouse_hovered_word = NULL; 

    //Init keep searching
    priv->keep_searching_delay = 0;
    priv->keep_searching_query = NULL;
    priv->keep_searching_enabled = FALSE;

    priv->text_selected = FALSE;

    priv->new_tab = FALSE; 

    priv->history = lw_history_new (20);
    priv->spellcheck = NULL;

    _searchwindow_attach_signals (window);

    gtk_widget_grab_focus (GTK_WIDGET (priv->entry));
    gw_searchwindow_set_dictionary (window, 0);
    gw_searchwindow_guarantee_first_tab (window);

    //We are going to lazily update the sensitivity of the spellcheck buttons only when the window is created
    GtkToolButton *toolbutton;
    gboolean enchant_exists;

    toolbutton = GTK_TOOL_BUTTON (gw_window_get_object (GW_WINDOW (window), "spellcheck_toolbutton")); 
    enchant_exists = g_file_test (ENCHANT, G_FILE_TEST_IS_REGULAR);

    gtk_widget_set_sensitive (GTK_WIDGET (priv->entry), enchant_exists);
    gtk_widget_set_sensitive (GTK_WIDGET (toolbutton), enchant_exists);
}


void gw_searchwindow_private_finalize (GwSearchWindow *window)
{
    //Declarations
    GwSearchWindowPrivate *priv;
    GSource *source;
    int i;

    priv = GW_SEARCHWINDOW_GET_PRIVATE (window);

    for (i = 0; i < TOTAL_GW_SEARCHWINDOW_TIMEOUTIDS; i++)
    {
      if (g_main_current_source () != NULL &&
          !g_source_is_destroyed (g_main_current_source ()) &&
          priv->timeoutid[i] > 0
         )
      {
        source = g_main_context_find_source_by_id (NULL, priv->timeoutid[i]);
        if (source != NULL)
        {
          g_source_destroy (source);
        }
      }
      priv->timeoutid[i] = 0;
    }

    _searchwindow_remove_signals (window);

    gw_searchwindow_cancel_all_searches (window);

    if (priv->spellcheck != NULL) gw_spellcheck_free (priv->spellcheck);
    if (priv->history != NULL) lw_history_free (priv->history);
    if (priv->tablist != NULL) g_list_free (priv->tablist);

    //Mouse finalize
    if (priv->mouse_hovered_word != NULL)
    {
      g_free (priv->mouse_hovered_word);
      priv->mouse_hovered_word = NULL;
    }

    //Keep searching finalize
    if (priv->keep_searching_query != NULL)
    {
      g_free (priv->keep_searching_query);
      priv->keep_searching_query = NULL;
    } 
}


static void _searchwindow_attach_signals (GwSearchWindow *window)
{
    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    GwDictInfoList *dictinfolist;
    LwPreferences *preferences;
    int i;

    application = GW_APPLICATION (gtk_window_get_application (GTK_WINDOW (window)));
    priv = GW_SEARCHWINDOW_GET_PRIVATE (window);
    dictinfolist = gw_application_get_dictinfolist (application);
    preferences = gw_application_get_preferences (application);

    for (i = 0; i < TOTAL_GW_SEARCHWINDOW_SIGNALIDS; i++)
      priv->signalid[i] = 0;

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_TOOLBAR_SHOW] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_TOOLBAR_SHOW,
        gw_searchwindow_sync_toolbar_show_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_STATUSBAR_SHOW] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_STATUSBAR_SHOW,
        gw_searchwindow_sync_statusbar_show_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_USE_GLOBAL_FONT] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_USE_GLOBAL_FONT,
        gw_searchwindow_sync_font_cb,
        window
    );
    priv->signalid[GW_SEARCHWINDOW_SIGNALID_CUSTOM_FONT] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_CUSTOM_FONT,
        gw_searchwindow_sync_font_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_FONT_MAGNIFICATION] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        LW_KEY_FONT_MAGNIFICATION,
        gw_searchwindow_sync_font_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_KEEP_SEARCHING] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SEARCH_AS_YOU_TYPE,
        gw_searchwindow_sync_search_as_you_type_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_SPELLCHECK] = lw_preferences_add_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        LW_KEY_SPELLCHECK,
        gw_searchwindow_sync_spellcheck_cb,
        window
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_ADDED] = g_signal_connect (
        G_OBJECT (dictinfolist->model),
        "row-inserted",
        G_CALLBACK (gw_searchwindow_dictionaries_added_cb),
        window 
    );

    priv->signalid[GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_DELETED] = g_signal_connect (
        G_OBJECT (dictinfolist->model),
        "row-deleted",
        G_CALLBACK (gw_searchwindow_dictionaries_deleted_cb),
        window 
    );

    priv->timeoutid[GW_SEARCHWINDOW_TIMEOUTID_KEEP_SEARCHING] = gdk_threads_add_timeout (
          100,
          (GSourceFunc) gw_searchwindow_keep_searching_timeout, 
          window
    );

    priv->timeoutid[GW_SEARCHWINDOW_TIMEOUTID_PROGRESS] = g_timeout_add_full (
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
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    GwDictInfoList *dictinfolist;
    LwPreferences *preferences;
    int i;

    application = GW_APPLICATION (gtk_window_get_application (GTK_WINDOW (window)));
    priv = GW_SEARCHWINDOW_GET_PRIVATE (window);
    dictinfolist = gw_application_get_dictinfolist (application);
    preferences = gw_application_get_preferences (application);

    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_TOOLBAR_SHOW]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_STATUSBAR_SHOW]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_USE_GLOBAL_FONT]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_CUSTOM_FONT]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_FONT,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_FONT_MAGNIFICATION]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_KEEP_SEARCHING]
    );
    lw_preferences_remove_change_listener_by_schema (
        preferences,
        LW_SCHEMA_BASE,
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_SPELLCHECK]
    );

    g_signal_handler_disconnect (
        G_OBJECT (dictinfolist->model),
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_ADDED]
    );

    g_signal_handler_disconnect (
        G_OBJECT (dictinfolist->model),
        priv->signalid[GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_DELETED]
    );


    for (i = 0; i < TOTAL_GW_SEARCHWINDOW_SIGNALIDS; i++)
      priv->signalid[i] = 0;
}


void gw_searchwindow_initialize_dictionary_combobox (GwSearchWindow *window)
{
    //Declarations
    GwApplication *application;
    GwSearchWindowPrivate *priv;
    GtkCellRenderer *renderer;
    GwDictInfoList *dictinfolist;

    //Initializations
    application = GW_APPLICATION (gtk_window_get_application (GTK_WINDOW (window)));
    priv = GW_SEARCHWINDOW_GET_PRIVATE (window);
    renderer = gtk_cell_renderer_text_new ();
    dictinfolist = gw_application_get_dictinfolist (application);

    gtk_combo_box_set_model (priv->combobox, NULL);
    gtk_cell_layout_clear (GTK_CELL_LAYOUT (priv->combobox));

    gtk_combo_box_set_model (priv->combobox, GTK_TREE_MODEL (dictinfolist->model));
    gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (priv->combobox), renderer, TRUE);
    gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (priv->combobox), renderer, "text", GW_DICTINFOLIST_COLUMN_LONG_NAME, NULL);
    gtk_combo_box_set_active (priv->combobox, 0);
}


void gw_searchwindow_initialize_dictionary_menu (GwSearchWindow *window)
{
    GwApplication *application;
    GtkMenuShell *shell;
    GList *list, *iter;
    GtkWidget *widget;
    GwDictInfoList *dictinfolist;

    application = GW_APPLICATION (gtk_window_get_application (GTK_WINDOW (window)));
    shell = GTK_MENU_SHELL (gw_window_get_object (GW_WINDOW (window), "dictionary_popup"));
    dictinfolist = gw_application_get_dictinfolist (application);

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

    accel_group = GTK_ACCEL_GROUP (gw_window_get_object (GW_WINDOW (window), "main_accelgroup"));
    group = NULL;

    //Refill the menu
    for (iter = dictinfolist->list; iter != NULL; iter = iter->next)
    {
      di = LW_DICTINFO (iter->data);
      if (di != NULL)
      {
        widget = GTK_WIDGET (gtk_radio_menu_item_new_with_label (group, di->longname));
        group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (widget));
        gtk_menu_shell_append (GTK_MENU_SHELL (shell),  GTK_WIDGET (widget));
        if (di->load_position == 0)
          gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (widget), TRUE);
        g_signal_connect(G_OBJECT (widget), "toggled", G_CALLBACK (gw_searchwindow_dictionary_radio_changed_cb), window);
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
    g_signal_connect (G_OBJECT (widget), "activate", G_CALLBACK (gw_searchwindow_cycle_dictionaries_backward_cb), window);
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", accel_group, GDK_KEY_Up, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_show (GTK_WIDGET (widget));

    widget = GTK_WIDGET (gtk_menu_item_new_with_mnemonic(gettext("Cycle _Down")));
    gtk_menu_shell_append (GTK_MENU_SHELL (shell), GTK_WIDGET (widget));
    g_signal_connect (G_OBJECT (widget), "activate", G_CALLBACK (gw_searchwindow_cycle_dictionaries_forward_cb), window);
    gtk_widget_add_accelerator (GTK_WIDGET (widget), "activate", accel_group, GDK_KEY_Down, GDK_MOD1_MASK, GTK_ACCEL_VISIBLE);
    gtk_widget_show (GTK_WIDGET (widget));
}


