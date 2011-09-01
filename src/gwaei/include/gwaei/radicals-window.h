#ifndef GW_RADICALS_WINDOW_INCLUDED
#define GW_RADICALS_WINDOW_INCLUDED

struct _GwRadicalsWindow {
  EXTENDS_GW_WINDOW

  
  GtkToggleButton *strokes_checkbutton;
  GtkTable *radicals_table;
  GtkSpinButton *strokes_spinbutton;
  char cache[300 * 4];
};
typedef struct _GwRadicalsWindow GwRadicalsWindow;

#define GW_RADICALSWINDOW(object) (GwRadicalsWindow*)object

typedef enum {
  GW_RADARRAY_STROKES,
  GW_RADARRAY_REPRESENTATIVE,
  GW_RADARRAY_ACTUAL,
  GW_RADARRAY_NAME,
  GW_RADARRAY_TOTAL
} GwRadicalArrayField;

GwRadicalsWindow* gw_radicalswindow_new (GwWindow*, GList*);
void gw_radicalswindow_init (GwRadicalsWindow*, GwWindow*);
void gw_radicalswindow_deinit (GwRadicalsWindow*);
void gw_radicalswindow_destroy (GwRadicalsWindow*);

char* gw_radicalswindow_strdup_all_selected (GwRadicalsWindow*);
char* gw_radicalswindow_strdup_prefered_stroke_count (GwRadicalsWindow*);
void gw_radicalswindow_deselect_all_radicals (GwRadicalsWindow*);
void gw_radicalswindow_set_strokes_checkbox_state (GwRadicalsWindow*, gboolean);
void gw_radicalswindow_set_button_sensitive_when_label_is (GwRadicalsWindow*, const char*);
void gw_radicalswindow_update_strokes_checkbox_state (GwRadicalsWindow*);

#include <gwaei/radicals-callbacks.h>

#endif
