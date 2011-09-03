#ifndef LW_NCURSES_CALLBACKS_INCLUDED
#define LW_NCURSES_CALLBACKS_INCLUDED

void nw_output_append_edict_result_cb (LwSearchItem*);
void nw_output_append_kanjidict_result_cb (LwSearchItem*);
void nw_output_append_examplesdict_result_cb (LwSearchItem*);
void nw_output_append_unknowndict_result_cb (LwSearchItem*);

void nw_output_append_less_relevant_header_cb (LwSearchItem*);
void nw_output_append_more_relevant_header_cb (LwSearchItem*);

void nw_output_prepare_search_cb (LwSearchItem*);
void nw_output_cleanup_search_cb (LwSearchItem*);

void nw_update_progress_feedback_cb (LwSearchItem*);

#endif
