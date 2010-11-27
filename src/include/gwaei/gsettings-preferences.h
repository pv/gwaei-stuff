#ifndef GW_GSETTINGS_PREFERENCES_INCLUDED
#define GW_GSETTINGS_PREFERENCES_INCLUDED

#include <gio/gio.h>

void gw_pref_set_int (char*, char*, int);
int gw_pref_get_int (char*, char*, int);

void gw_pref_set_boolean (char*, char*, gboolean);
gboolean gw_pref_get_boolean (char*, char*, gboolean);

void gw_pref_set_string (const char*, const char*, const char*);
char* gw_pref_get_string (char*, const char*, const char*, int);

void gw_prefs_add_change_listener (const char*, const char*, void (GSettings*, gchar*, gpointer), gpointer);

void gw_pref_reset_value (const char*, const char*);

#endif

