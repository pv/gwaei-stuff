#ifndef GW_GTK_MAIN_INTERFACE_INCLUDED
#define GW_GTK_MAIN_INTERFACE_INCLUDED


gpointer get_gobject_from_target (const int);
void initialize_text_views (void);
void initialize_window_attributes (char* window_id);
void save_window_attributes (char* window_id);
void rebuild_history_menuitem_popup (void);
int  rebuild_combobox_dictionary_list (void);
void update_toolbar_buttons (void);
void update_history_popup (void);
void gw_ui_update_total_results_label (GwSearchItem*);
gboolean gw_load_gtk_builder_xml (const char );
char* gw_ui_get_text_slice_from_buffer (int, int, int);

const char* gw_ui_get_active_dictionary (void);
char* gw_ui_get_text_from_text_buffer (const int);
void initialize_gui_interface (int, char**);
gboolean gw_ui_cancel_search (gpointer);
gboolean gw_ui_cancel_search_by_searchitem (GwSearchItem*);



void gw_ui_initialize_interface_output_generics (void);
void gw_ui_append_edict_results_to_buffer (GwSearchItem*);
void gw_ui_append_kanjidict_results_to_buffer (GwSearchItem*);
void gw_ui_append_examplesdict_results_to_buffer (GwSearchItem*);
void gw_ui_append_unknowndict_results_to_buffer (GwSearchItem*);
void gw_ui_update_progress_feedback (GwSearchItem*);
void gw_ui_no_result(GwSearchItem*);
void gw_ui_append_less_relevant_header_to_output (GwSearchItem*);
void gw_ui_append_more_relevant_header_to_output (GwSearchItem*);
void gw_ui_pre_search_prep (GwSearchItem*);
void gw_ui_after_search_cleanup (GwSearchItem*);


#endif
