#ifndef GW_GTK_RADICALS_INTERFACE_INCLUDED
#define GW_GTK_RADICALS_INTERFACE_INCLUDED


typedef enum {
  GW_RADARRAY_STROKES,
  GW_RADARRAY_REPRESENTATIVE,
  GW_RADARRAY_ACTUAL,
  GW_RADARRAY_NAME,
  GW_RADARRAY_TOTAL
} GwRadicalArrayField;


char* gw_ui_strdup_all_selected_radicals (void);
char* gw_ui_strdup_prefered_stroke_count (void);
void gw_ui_deselect_all_radicals (void);


#endif
