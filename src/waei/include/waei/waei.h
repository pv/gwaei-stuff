#ifndef GW_WAEI_INCLUDED
#define GW_WAEI_INCLUDED

#include <libwaei/libwaei.h>

struct _WApplication {
    LwDictInfoList *dictinfolist;
    LwDictInstList *dictinstlist;
    LwPrefManager *prefmanager;
    LwEngine *engine;

    gboolean quiet_switch;
    gboolean exact_switch;
    gboolean list_switch;
    gboolean version_switch;
#ifdef WITH_NCURSES
    gboolean ncurses_switch;
#endif
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
void w_app_run (WApplication*);
void w_app_parse_args (WApplication*, int*, char***);

int w_app_start_console (WApplication*);
int w_app_start_ncurses (WApplication*);


extern WApplication *app;


#include <waei/console.h>
#ifdef WITH_NCURSES
  #include <waei/ncurses.h>
#endif

#endif
