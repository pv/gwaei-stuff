#ifndef LW_ENGINE_INCLUDED
#define LW_ENGINE_INCLUDED

//Starts a search.  Make sure to set target_text_buffer and target_text view

#define LW_ENGINE(object) (LwEngine*) object
#define LW_MAX_HIGH_RELEVENT_RESULTS 200
#define LW_MAX_MEDIUM_IRRELEVENT_RESULTS 100
#define LW_MAX_LOW_IRRELEVENT_RESULTS    50


struct _LwEngine {
  //Function pointers that should be set on program startup
  void (*append_edict_result_cb)(LwSearchItem*);
  void (*append_kanjidict_result_cb)(LwSearchItem*);
  void (*append_examplesdict_result_cb)(LwSearchItem*);
  void (*append_unknowndict_result_cb)(LwSearchItem*);

  void (*append_less_relevant_header_cb)(LwSearchItem*);
  void (*append_more_relevant_header_cb)(LwSearchItem*);

  void (*prepare_search_cb)(LwSearchItem*);
  void (*cleanup_search_cb)(LwSearchItem*);
};
typedef struct _LwEngine LwEngine;

typedef void(*LwOutputFunc)(LwSearchItem*);

LwEngine* lw_engine_new (
                           LwOutputFunc,
                           LwOutputFunc,
                           LwOutputFunc,
                           LwOutputFunc,
                           LwOutputFunc,
                           LwOutputFunc,
                           LwOutputFunc,
                           LwOutputFunc
                          );
void lw_engine_free (LwEngine*);

void lw_engine_get_results (LwEngine*, LwSearchItem*, gboolean, gboolean);

void lw_engine_append_result (LwEngine*, LwSearchItem*);
void lw_engine_append_less_relevant_header (LwEngine*, LwSearchItem*);
void lw_engine_append_more_relevant_header (LwEngine*, LwSearchItem*);
gpointer lw_engine_prepare_search (LwEngine*, LwSearchItem*, gboolean);
void lw_engine_cleanup_search (gpointer);


#endif
