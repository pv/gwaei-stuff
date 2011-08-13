#ifndef LW_NCURSES_CALLBACKS_INCLUDED
#define LW_NCURSES_CALLBACKS_INCLUDED

void nw_append_edict_results_cb (LwSearchItem*);
void nw_append_kanjidict_results_cb (LwSearchItem*);
void nw_append_examplesdict_results_cb (LwSearchItem*);
void nw_append_unknowndict_results_cb (LwSearchItem*);

void nw_update_progress_feedback_cb (LwSearchItem*);

void nw_append_less_relevant_header_cb (LwSearchItem*);
void nw_append_more_relevant_header_cb (LwSearchItem*);

void nw_prepare_search_cb (LwSearchItem*);
void nw_cleanup_search_cb (LwSearchItem*);

#endif
