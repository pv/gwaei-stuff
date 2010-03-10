#define GW_PREFERENCES_INCLUDED

void gw_pref_set_int (char*, int);
int gw_pref_get_int (char*, int);
int gw_pref_get_default_int (char*, int);

void gw_pref_set_boolean (char*, gboolean);
gboolean gw_pref_get_boolean (char*, gboolean);
gboolean gw_pref_get_default_boolean (char*, gboolean);

void gw_pref_set_string (char*, const char*);
char* gw_pref_get_string (char*, char*, char*, int);
const char* gw_pref_get_default_string (char*, char*);

void gw_prefs_initialize_preferences(void);
void do_dictionary_source_gconf_key_changed_action (gpointer, guint, gpointer, gpointer);

