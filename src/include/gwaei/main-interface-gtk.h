#ifndef GW_MAIN_INTERFACE_GTK_INCLUDED
#define GW_MAIN_INTERFACE_GTK_INCLUDED

void gw_ui_append_edict_results_to_buffer (GwSearchItem*, gboolean);
void gw_ui_append_kanjidict_results_to_buffer (GwSearchItem*, gboolean);
void gw_ui_append_examplesdict_results_to_buffer (GwSearchItem*, gboolean);
void gw_ui_append_unknowndict_results_to_buffer (GwSearchItem*, gboolean);
void gw_ui_update_progress_feedback (GwSearchItem*);
void gw_ui_no_result(GwSearchItem*);
void gw_ui_append_less_relevant_header_to_output (GwSearchItem*);
void gw_ui_append_more_relevant_header_to_output (GwSearchItem*);
void gw_ui_after_search_cleanup (GwSearchItem*);


#endif
