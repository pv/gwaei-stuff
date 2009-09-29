//Main
void do_back (GtkWidget *widget, gpointer data);
void do_forward (GtkWidget *widget, gpointer data);
void do_save (GtkWidget *widget, gpointer data);
void do_print (GtkWidget *widget, gpointer data);
void do_zoom_in (GtkWidget *widget, gpointer data);
void do_zoom_out (GtkWidget *widget, gpointer data);
void do_zoom_100 (GtkWidget *widget, gpointer data);
void do_toolbar_toggle (GtkWidget *widget, gpointer data);
void do_less_relevant_results_toggle(GtkWidget *widget, gpointer data);
void do_select_all (GtkWidget *widget, gpointer data);
void do_paste (GtkWidget *widget, gpointer data);
void do_cut (GtkWidget *widget, gpointer data);
void do_copy (GtkWidget *widget, gpointer data);
void do_about(GtkWidget *widget, gpointer data);
void do_settings (GtkWidget *widget, gpointer data);
void do_destroy(GtkObject*, gpointer);
void do_search (GtkWidget *widget, gpointer data);
void do_search_from_history (GtkWidget*, gpointer);
void do_clear_search(GtkWidget*, gpointer);
void do_update_button_states_based_on_entry_text (GtkWidget*, gpointer);
void do_go_menuitem_action (GtkWidget*, gpointer);
void do_close_kanji_results(GtkWidget *widget, gpointer data);
gboolean do_switch_dictionaries_on_tab_press (GtkWidget*, GdkEvent*, gpointer*);
void search_drag_data_recieved (GtkWidget*, GdkDragContext*,
                                gint, gint,
                                GtkSelectionData*, guint,
                                guint, gpointer             );
gboolean do_update_clipboard_on_focus_change (GtkWidget*, GtkDirectionType, gpointer);
gboolean do_focus_change_on_key_press (GtkWidget*, GdkEvent*, gpointer*);
gboolean do_history_change_on_key_press ( GtkWidget*, GdkEvent*, gpointer*);

gboolean do_key_press_action (GtkWidget*, GdkEvent*, gpointer*);
gboolean do_close_on_escape (GtkWidget*, GdkEvent*, gpointer*);


//Settings
void do_spellcheck_toggle (GtkWidget *widget, gpointer data);
void do_hiragana_katakana_conv_toggle (GtkWidget *widget, gpointer data);
void do_katakana_hiragana_conv_toggle (GtkWidget *widget, gpointer data);
void do_romanji_kana_conv_change (GtkWidget *widget, gpointer data);
void do_set_color_to_swatch (GtkWidget*, gpointer);
void do_color_reset_for_swatches (GtkWidget*, gpointer);
void do_source_entry_changed_action (GtkWidget*, gpointer);
void do_radical_search (GtkWidget*, gpointer);
