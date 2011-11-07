#ifndef GW_CONSOLE_CALLBACKS_INCLUDED
#define GW_CONSOLE_CALLBACKS_INCLUDED

void w_console_append_edict_result_cb (LwSearchItem*);
void w_console_append_kanjidict_result_cb (LwSearchItem*);
void w_console_append_examplesdict_result_cb (LwSearchItem*);
void w_console_append_unknowndict_result_cb (LwSearchItem*);
void w_console_update_progress_feedback_cb (LwSearchItem*);
void w_console_prepare_search_cb (LwSearchItem*);
void w_console_cleanup_search_cb (LwSearchItem*);

void w_console_append_less_relevant_header_cb (LwSearchItem*);
void w_console_append_more_relevant_header_cb (LwSearchItem*);

int w_console_install_progress_cb (double, gpointer);
int w_console_uninstall_progress_cb (double, gpointer);

#endif
