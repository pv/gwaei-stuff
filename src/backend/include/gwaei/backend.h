#ifndef GW_BASE_INCLUDED
#define GW_BASE_INCLUDED

#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/dictinfo.h>
#include <gwaei/dictinfolist.h>
#include <gwaei/dictinst.h>
#include <gwaei/dictinstlist.h>
#include <gwaei/preferences.h>
#include <gwaei/resultline.h>
#include <gwaei/queryline.h>
#include <gwaei/searchitem.h>
#include <gwaei/historylist.h>
#include <gwaei/engine.h>

void gw_backend_initialize (int*, char**);
void gw_backend_free(void);


#endif
