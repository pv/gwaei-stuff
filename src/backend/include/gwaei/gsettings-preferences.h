#ifndef GW_GSETTINGS_PREFERENCES_INCLUDED
#define GW_GSETTINGS_PREFERENCES_INCLUDED

#include <gio/gio.h>

void gw_pref_set_int (const char*, const char*, const int);
int gw_pref_get_int (const char*, const char*);

void gw_pref_set_boolean (const char*, const char*, const gboolean);
gboolean gw_pref_get_boolean (const char*, const char*);

void gw_pref_set_string (const char*, const char*, const char*);
void gw_pref_get_string (char*, const char*, const char*, const int);

void gw_prefs_add_change_listener (const char*, const char*, void (GSettings*, gchar*, gpointer), gpointer);

void gw_pref_reset_value (const char*, const char*);

#endif

