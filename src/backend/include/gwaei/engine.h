#ifndef GW_ENGINE_INCLUDED
#define GW_ENGINE_INCLUDED

//Starts a search.  Make sure to set target_text_buffer and target_text view

typedef void(*GwOutputFunc)(GwSearchItem*);

void gw_engine_initialize (
                           GwOutputFunc,
                           GwOutputFunc,
                           GwOutputFunc,
                           GwOutputFunc,
                           GwOutputFunc,
                           GwOutputFunc,
                           GwOutputFunc,
                           GwOutputFunc
                          );
void gw_engine_free (void);

void gw_engine_get_results (GwSearchItem*, gboolean, gboolean);

GwOutputFunc gw_engine_get_append_edict_results_func (void);
GwOutputFunc gw_engine_get_append_kanjidict_results_func (void);
GwOutputFunc gw_engine_get_append_examplesdict_results_func (void);
GwOutputFunc gw_engine_get_append_unknowndict_results_func (void);
GwOutputFunc gw_engine_get_append_less_relevant_header_func (void);
GwOutputFunc gw_engine_get_append_more_relevant_header_func (void);
GwOutputFunc gw_engine_get_pre_search_prep_func (void);
GwOutputFunc gw_engine_get_after_search_cleanup_func (void);

#endif
