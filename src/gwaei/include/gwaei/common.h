#ifndef GW_COMMON_INCLUDED
#define GW_COMMON_INCLUDED


#define GW_MAX_FONT_MAGNIFICATION  6
#define GW_MIN_FONT_MAGNIFICATION -6
#define GW_DEFAULT_FONT_MAGNIFICATION 0
#define GW_FONT_ZOOM_STEP 2
#define GW_MAX_FONT_SIZE 100
#define GW_MIN_FONT_SIZE 6
#define GW_DEFAULT_FONT_SIZE 12
#define GW_DEFAULT_FONT "Sans 12"


void gw_common_handle_error (GError**, GtkWindow*, gboolean);

#endif
