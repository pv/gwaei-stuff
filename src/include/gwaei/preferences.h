#ifndef GW_PREFERENCES_INCLUDED
#define GW_PREFERENCES_INCLUDED

#include <gio/gio.h>

void gw_prefs_initialize_preferences(void);
void do_dictionary_source_pref_key_changed_action (GSettings*, gchar*, gpointer);

#endif
