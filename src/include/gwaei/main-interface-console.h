#ifndef GW_MAIN_INTERFACE_CONSOLE_INCLUDED
#define GW_MAIN_INTERFACE_CONSOLE_INCLUDED

void gw_console_append_edict_results (GwSearchItem*, gboolean);
void gw_console_append_kanjidict_results (GwSearchItem*, gboolean);
void gw_console_append_examplesdict_results (GwSearchItem*, gboolean);
void gw_console_append_unknowndict_results (GwSearchItem*, gboolean);
void gw_console_update_progress_feedback (GwSearchItem*);
void gw_console_no_result(GwSearchItem*);
void gw_console_append_less_relevant_header_to_output (GwSearchItem*);
void gw_console_append_more_relevant_header_to_output (GwSearchItem*);
void gw_console_after_search_cleanup (GwSearchItem*);

#endif
