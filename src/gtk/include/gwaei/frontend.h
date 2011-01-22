#ifndef GW_FRONTEND_INCLUDED
#define GW_FRONTED_INCLUDED

#include <gtk/gtk.h>

#include <gwaei/common.h>

#include <gwaei/kanjipad.h>
#include <gwaei/kanjipad-drawingarea.h>
#include <gwaei/kanjipad-candidatearea.h>
#include <gwaei/kanjipad-callbacks.h>

#include <gwaei/main.h>
#include <gwaei/main-callbacks.h>
#ifdef WITH_LIBSEXY
#include <gwaei/main-libsexy.h>
#endif
#include <gwaei/main-tabs.h>
#ifdef WITH_LIBUNIQUE
#include <gwaei/main-libunique.h>
#endif

#include <gwaei/preferences.h>
#include <gwaei/printing.h>

#include <gwaei/radicals.h>
#include <gwaei/radicals-callbacks.h>

#include <gwaei/settings.h>
#include <gwaei/settings-dictionary-install-dialog.h>
#include <gwaei/settings-callbacks.h>
#include <gwaei/settings-dictionary-manager.h>

void gw_frontend_initialize (int, char**);
void gw_frontend_free (void);
void gw_frontend_start_gtk (int, char**);

#endif
