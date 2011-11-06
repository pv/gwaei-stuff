#ifndef GW_WAEI_INCLUDED
#define GW_WAEI_INCLUDED

#include "../../../../config.h"
#include "gettext.h"

#include <libwaei/libwaei.h>


typedef enum {
  W_APP_RESOLUTION_SUCCESS,
  W_APP_RESOLUTION_ERROR,
  W_APP_RESOLUTION_OUT_OF_MEMORY,
  W_APP_RESOLUTION_INVALID_QUERY,
  W_APP_RESOLUTION_INVALID_DICTIONARY,
  W_APP_RESOLUTION_DICTIONARY_INSTALL_ERROR,
  W_APP_RESOLUTION_DICTIONARY_UNINSTALL_ERROR
} WAppResolution;


struct _WApplication {
    int* argc;
    char*** argv;

    LwDictInfoList *dictinfolist;
    LwDictInstList *dictinstlist;
    LwPreferences *preferences;
    LwEngine *engine;

    gboolean quiet_switch;
    gboolean exact_switch;
    gboolean list_switch;
    gboolean version_switch;
    gboolean color_switch;

    char* dictionary_switch_data;
    char* install_switch_data;
    char* uninstall_switch_data;
    char* query_text_data;
    GOptionContext *context;
};
typedef struct _WApplication WApplication;


WApplication* w_app_new (int*, char***);
void w_app_free (WApplication*);
void w_app_init (WApplication*, int*, char***, GError **);
void w_app_deinit (WApplication*);

void w_app_parse_args (WApplication*, int*, char***, GError**);

WAppResolution w_app_start_console (WApplication*);


extern WApplication *app;

#include <waei/console.h>

#endif
