#ifndef GW_FRONTEND_INCLUDED
#define GW_FRONTEND_INCLUDED

#include <gtk/gtk.h>

#include <libwaei/libwaei.h>

#include <gwaei/common.h>
#include <gwaei/window.h>

#include <gwaei/spellcheck.h>

#include <gwaei/dictinfolist.h>

#include <gwaei/search-window.h>
#include <gwaei/radicals-window.h>
#include <gwaei/settings-window.h>
#include <gwaei/kanjipad-window.h>
#include <gwaei/dictionaryinstall-window.h>
#include <gwaei/installprogress-window.h>

#ifdef WITH_LIBUNIQUE
#include <gwaei/libunique.h>
#endif

#include <gwaei/printing.h>

#include <gwaei/output-callbacks.h>

typedef enum {
  GW_APP_RESOLUTION_NO_ERRORS,
  GW_APP_RESOLUTION_OUT_OF_MEMORY
} GwApplicationResolution;

#define GW_APPLICATION(object) (GwApplication*)object

struct _GwApplication {
  int* argc;
  char*** argv;

  GList *windowlist;

  LwDictInstList *dictinstlist;
  LwPrefManager *prefmanager;
  GwDictInfoList *dictinfolist;
  LwEngine *engine;
  GtkTextTagTable *tagtable;

  gchar   *arg_dictionary;
  gchar   *arg_query;
#ifdef WITH_LIBUNIQUE
  gboolean arg_new_instance;
#endif
  gboolean arg_version_switch;
  GOptionContext *context;

  int block_new_searches;
};
typedef struct _GwApplication GwApplication;


GwApplication *gw_app_new (int*, char***);
void gw_app_free (GwApplication *app);

GwApplicationResolution gw_app_run (GwApplication*);
void gw_app_parse_args (GwApplication*, int*, char***);
void gw_app_quit (GwApplication*);

GwWindow* gw_app_show_window (GwApplication*, const GwWindowType, GwWindow*, gboolean);
void gw_app_destroy_window (GwApplication*, GwWindow*);
GwWindow* gw_app_get_window_by_type (GwApplication*, const GwWindowType);
GwWindow* gw_app_get_window_by_widget (GwApplication*, GtkWidget*);

const char* gw_app_get_program_name (GwApplication*);
void gw_app_cancel_all_searches (GwApplication*);

extern GwApplication *app;

void gw_app_sync_tag_cb (GSettings*, gchar*, gpointer);
GtkTextTagTable* gw_texttagtable_new ();

void gw_app_block_searches (GwApplication*);
void gw_app_unblock_searches (GwApplication*);
gboolean gw_app_can_start_search (GwApplication*);

#endif
