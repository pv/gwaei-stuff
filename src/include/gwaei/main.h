#ifndef GW_MAIN_INCLUDED
#define GW_MAIN_INCLUDED

#include <gwaei/searchitem-object.h>

void (*gw_output_generic_append_edict_results)(struct GwSearchItem*);
void (*gw_output_generic_append_kanjidict_results)(struct GwSearchItem*);
void (*gw_output_generic_append_examplesdict_results)(struct GwSearchItem*);
void (*gw_output_generic_append_unknowndict_results)(struct GwSearchItem*);

void (*gw_output_generic_update_progress_feedback)(struct GwSearchItem*);
void (*gw_output_generic_append_less_relevant_header_to_output)(struct GwSearchItem*);
void (*gw_output_generic_append_more_relevant_header_to_output)(struct GwSearchItem*);
void (*gw_output_generic_pre_search_prep)(struct GwSearchItem*);
void (*gw_output_generic_after_search_cleanup)(struct GwSearchItem*);

gboolean gw_main_verify_output_generic_functions (void);

#endif
