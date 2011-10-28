#ifndef LW_ENGINE_INCLUDED
#define LW_ENGINE_INCLUDED

//Starts a search.  Make sure to set target_text_buffer and target_text view

#define LW_MAX_HIGH_RELEVENT_RESULTS 200
#define LW_MAX_MEDIUM_IRRELEVENT_RESULTS 100
#define LW_MAX_LOW_IRRELEVENT_RESULTS    50

void lw_searchitem_start_search (LwSearchItem*, gboolean, gboolean);


#endif
