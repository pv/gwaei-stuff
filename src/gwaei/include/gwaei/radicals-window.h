#ifndef GW_GTK_RADICALS_INTERFACE_INCLUDED
#define GW_GTK_RADICALS_INTERFACE_INCLUDED

struct _GwRadicalsWindow {
  EXTENDS_GW_WINDOW

  GtkTable *table;
  char cache[300 * 4];
};
typedef struct _GwRadicalsWindow GwRadicalsWindow;

typedef enum {
  GW_RADARRAY_STROKES,
  GW_RADARRAY_REPRESENTATIVE,
  GW_RADARRAY_ACTUAL,
  GW_RADARRAY_NAME,
  GW_RADARRAY_TOTAL
} GwRadicalArrayField;

GwRadicalsWindow* gw_radicalswindow_new (void);
void gw_radicalswindow_destroy (GwRadicalsWindow*);

char* gw_radicalswindow_strdup_all_selected (GwRadicalsWindow*);
char* gw_radicalswindow_strdup_prefered_stroke_count (GwRadicalsWindow*);
void gw_radicalswindow_deselect_all_radicals (GwRadicalsWindow*);
void gw_radicalswindow_set_strokes_checkbox_state (GwRadicalsWindow*, gboolean);
void gw_radicalswindow_set_button_sensitive_when_label_is (GwRadicalsWindow*, const char*);
void gw_radicalswindow_update_strokes_checkbox_state (GwRadicalsWindow*);


#endif
