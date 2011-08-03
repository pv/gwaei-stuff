#ifndef NW_NCURSES_INCLUDED
#define NW_NCURSES_INCLUDED


typedef enum {
  NW_NCCOLORS_GREENONBLACK,
  NW_NCCOLORS_BLUEONBLACK,
  NW_NCCOLORS_REDONBLACK,
  NW_NCCOLORS_TOTAL
} NWcursesColorPair;


void initialize_ncurses_interface (int, char**);
void nw_initialize_interface_output_generics (void);
void nw_append_edict_results_to_buffer (LwSearchItem*);
void nw_append_kanjidict_results_to_buffer (LwSearchItem*);
void nw_append_examplesdict_results_to_buffer (LwSearchItem*);
void nw_append_unknowndict_results_to_buffer (LwSearchItem*);

void nw_update_progress_feedback (LwSearchItem*);
void nw_no_result(LwSearchItem*);
void nw_append_less_relevant_header_to_output (LwSearchItem*);
void nw_append_more_relevant_header_to_output (LwSearchItem*);
void nw_pre_search_prep (LwSearchItem*);
void nw_after_search_cleanup (LwSearchItem*);

void nw_start_ncurses ();

#endif
