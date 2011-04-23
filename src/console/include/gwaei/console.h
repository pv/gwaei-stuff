#ifndef GW_CONSOLE_INCLUDED
#define GW_CONSOLE_INCLUDED


void gw_console_append_edict_results_to_buffer (GwSearchItem*);
void gw_console_append_kanjidict_results_to_buffer (GwSearchItem*);
void gw_console_append_examplesdict_results_to_buffer (GwSearchItem*);
void gw_console_append_unknowndict_results_to_buffer (GwSearchItem*);
void gw_console_update_progress_feedback (GwSearchItem*);
void gw_console_no_result(GwSearchItem*);
void gw_console_append_less_relevant_header_to_output (GwSearchItem*);
void gw_console_append_more_relevant_header_to_output (GwSearchItem*);
void gw_console_pre_search_prep (GwSearchItem*);
void gw_console_after_search_cleanup (GwSearchItem*);

void gw_console_list (void);
void gw_console_about (void);
void gw_console_start_banner (char *, char *);

#endif
