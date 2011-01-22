#ifndef GW_BASE_INCLUDED
#define GW_BASE_INCLUDED

#include <gwaei/definitions.h>   
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>

#include <gwaei/dictinfo-object.h>
#include <gwaei/dictlist-object.h>

#include <gwaei/gsettings-preferences.h>

#include <gwaei/resultline-object.h>
#include <gwaei/queryline-object.h>
#include <gwaei/searchitem-object.h>
#include <gwaei/historylist-object.h>

#include <gwaei/engine.h>

void gw_backend_initialize (int, char**);
void gw_backend_free(void);


#endif
