#ifndef GW_MAIN_INTERFACE_NCURSES_INCLUDED
#define GW_MAIN_INTERFACE_NCURSES_INCLUDED

void gw_ncurses_append_edict_results (GwSearchItem*, gboolean);
void gw_ncurses_append_kanjidict_results (GwSearchItem*, gboolean);
void gw_ncurses_append_examplesdict_results (GwSearchItem*, gboolean);
void gw_ncurses_append_unknowndict_results (GwSearchItem*, gboolean);

#endif
