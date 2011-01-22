#ifndef GW_PREFERENCES_INCLUDED
#define GW_PREFERENCES_INCLUDED

#include <gio/gio.h>

void gw_preferences_initialize (void);
void gw_preferences_free (void);

void do_dictionary_source_pref_key_changed_action (GSettings*, gchar*, gpointer);

#endif
