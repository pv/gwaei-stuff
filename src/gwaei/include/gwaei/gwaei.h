#ifndef GW_FRONTEND_INCLUDED
#define GW_FRONTEND_INCLUDED

#include <gtk/gtk.h>

#include <libwaei/libwaei.h>

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


#define GW_MAX_FONT_MAGNIFICATION  6
#define GW_MIN_FONT_MAGNIFICATION -6
#define GW_DEFAULT_FONT_MAGNIFICATION 0
#define GW_FONT_ZOOM_STEP 2
#define GW_MAX_FONT_SIZE 100
#define GW_MIN_FONT_SIZE 6
#define GW_DEFAULT_FONT_SIZE 12
#define GW_DEFAULT_FONT "Sans 12"

typedef enum {
  GW_APP_RESOLUTION_NO_ERRORS,
  GW_APP_RESOLUTION_OUT_OF_MEMORY
} GwApplicationResolution;

#define GW_APPLICATION(object) (GwApplication*)object

struct _GwApplication {
  int* argc;
  char*** argv;

  GList *windowlist;

  LwPrefManager *prefmanager;
  GwDictInfoList *dictinfolist;
  LwEngine *engine;
  GtkTextTagTable *tagtable;
  GwSearchWindow *last_focused;

  gchar   *arg_dictionary;
  gchar   *arg_query;
#ifdef WITH_LIBUNIQUE
  gboolean arg_new_window;
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

void gw_app_handle_error (GwApplication*, GwWindow*, gboolean, GError**);

void gw_app_set_last_focused_searchwindow (GwApplication*, GwSearchWindow*);
GwSearchWindow* gw_app_get_last_focused_searchwindow (GwApplication*);

#endif
