#ifndef GW_WINDOW_INCLUDED
#define GW_WINDOW_INCLUDED

#include <gtk/gtk.h>

#define GW_WINDOW(object) (GwWindow*) object
//Include this line in the beginning of any struct that extends a GwWindow
#define EXTENDS_GW_WINDOW GtkBuilder *builder; GtkWindow *toplevel; GwWindowType type;

typedef enum {
  GW_WINDOW_SEARCH,
  GW_WINDOW_SETTINGS,
  GW_WINDOW_RADICALS,
  GW_WINDOW_KANJIPAD,
  GW_WINDOW_INSTALLPROGRESS,
  GW_WINDOW_DICTIONARYINSTALL
} GwWindowType;

struct _GwWindow {
  GtkBuilder *builder;
  GtkWindow *toplevel;
  GwWindowType type;
};
typedef struct _GwWindow GwWindow;

GwWindow* gw_window_new (const GwWindowType);
void gw_window_destroy (GwWindow*);
gboolean gw_window_load_ui_xml (GwWindow*, const char*);

#endif
