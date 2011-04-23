#ifndef GW_FRONTEND_INCLUDED
#define GW_FRONTEND_INCLUDED

#include <gwaei/console.h>

void gw_frontend_initialize (int*, char**);
void gw_frontend_free (void);
void gw_frontend_start_console (int, char**);

#endif
