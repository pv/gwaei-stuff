#ifndef GW_PREFERENCES_INCLUDED
#define GW_PREFERENCES_INCLUDED

#ifndef ENABLE_WIN32
#include <gconf/gconf-client.h>
#endif

void gw_pref_set_int (char*, int);
int gw_pref_get_int (char*, int);
int gw_pref_get_default_int (char*, int);

void gw_pref_set_boolean (char*, gboolean);
gboolean gw_pref_get_boolean (char*, gboolean);
gboolean gw_pref_get_default_boolean (char*, gboolean);

void gw_pref_set_string (char*, const char*);
char* gw_pref_get_string (char*, char*, char*, int);
char* gw_pref_get_default_string (char*, char*, char*, int);

void gw_prefs_initialize_preferences(void);

#ifdef WITH_GCONF
void do_dictionary_source_gconf_key_changed_action (GConfClient*, guint, GConfEntry*, gpointer);
void gw_prefs_add_change_listener (const char*, void (GConfClient*, guint, GConfEntry*, gpointer), gpointer);
#else
void do_dictionary_source_gconf_key_changed_action (void*, guint, void*, gpointer);
void gw_prefs_add_change_listener (const char*, void (void*, guint, void*, gpointer), gpointer);
#endif


#endif
