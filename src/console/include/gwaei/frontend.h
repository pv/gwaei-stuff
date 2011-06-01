#ifndef GW_FRONTEND_INCLUDED
#define GW_FRONTEND_INCLUDED

#include <gwaei/console.h>
#include <gwaei/ncurses.h>

void gw_frontend_initialize (int*, char**);
void gw_frontend_free (void);
int gw_frontend_start_console (int, char**);
int gw_frontend_start_ncurses (int, char**);
gboolean gw_frontend_get_color_switch (void);

#endif
