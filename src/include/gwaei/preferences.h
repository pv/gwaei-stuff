#ifndef GW_PREFERENCES_INCLUDED
#define GW_PREFERENCES_INCLUDED

#include <gio/gio.h>

void gw_pref_set_int (char*, char*, int);
int gw_pref_get_int (char*, char*, int);
int gw_pref_get_default_int (char*, char*, int);

void gw_pref_set_boolean (char*, char*, gboolean);
gboolean gw_pref_get_boolean (char*, char*, gboolean);
gboolean gw_pref_get_default_boolean (char*, char*, gboolean);

void gw_pref_set_string (char*, char*, const char*);
char* gw_pref_get_string (char*, char*, char*, char*, int);
char* gw_pref_get_default_string (char*, char*, char*, char*, int);

void gw_prefs_initialize_preferences(void);

void do_dictionary_source_pref_key_changed_action (GSettings*, gchar*, gpointer);
void gw_prefs_add_change_listener (const char*, const char*, void (GSettings*, gchar*, gpointer), gpointer);

void do_kata_hira_conv_pref_changed_action (GSettings*, gchar*, gpointer);

#endif
