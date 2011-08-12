#ifndef GW_FRONTEND_INCLUDED
#define GW_FRONTEND_INCLUDED

#include <gtk/gtk.h>

#include <libwaei/libwaei.h>

#include <gwaei/common.h>
#include <gwaei/window.h>

#include <gwaei/search-window.h>
#include <gwaei/search-callbacks.h>

#ifdef WITH_LIBUNIQUE
#include <gwaei/libunique.h>
#endif

#include <gwaei/printing.h>

#include <gwaei/radicals-window.h>
#include <gwaei/radicals-callbacks.h>

#include <gwaei/settings-window.h>
#include <gwaei/settings-callbacks.h>

#include <gwaei/dictionarymanager.h>
#include <gwaei/dictionarymanager-callbacks.h>

#include <gwaei/dictionaryinstall-window.h>
#include <gwaei/dictionaryinstall-callbacks.h>

#include <gwaei/installprogress-window.h>
#include <gwaei/installprogress-callbacks.h>

#include <gwaei/spellcheck.h>

#include <gwaei/kanjipad-window.h>
#include <gwaei/kanjipad-callbacks.h>
#include <gwaei/kanjipad-candidatearea.h>
#include <gwaei/kanjipad-drawingarea.h>

#include <gwaei/output-callbacks.h>


struct _GwApplication {
  int* argc;
  char*** argv;

  GList *windowlist;

  LwDictInstList *dictinstlist;
  LwPrefManager *prefmanager;
  LwHistoryList *history;
  GwDictionaryManager *dictionarymanager;

  gchar   *arg_dictionary;
  gchar   *arg_query;
#ifdef WITH_LIBUNIQUE
  gboolean arg_new_instance;
#endif
  gboolean arg_version_switch;
  GOptionContext *context;
};
typedef struct _GwApplication GwApplication;


GwApplication *gw_app_new (int*, char***);
void gw_app_free (GwApplication *app);

void gw_app_run (GwApplication*);
void gw_app_parse_args (GwApplication*, int*, char***);
void gw_app_quit (GwApplication*);

GwWindow* gw_app_show_window (GwApplication*, const GwWindowType, gboolean);
void gw_app_destroy_window (GwApplication*, const GwWindowType, GtkWidget*);
GwWindow* gw_app_get_window (GwApplication*, const GwWindowType, GtkWidget*);

const char* gw_app_get_program_name (GwApplication*);
void gw_app_cancel_all_searches (GwApplication*);

extern GwApplication *app;



#endif
