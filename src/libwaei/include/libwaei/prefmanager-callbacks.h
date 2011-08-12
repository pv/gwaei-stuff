#ifndef LW_PREFMANAGER_CALLBACKS_INCLUDED
#define LW_PREFMANAGER_CALLBACKS_INCLUDED

void lw_prefmanager_boolean_changed_cb (GSettings*, gchar*, gpointer);
void lw_prefmanager_integer_changed_cb (GSettings*, gchar*, gpointer);
void lw_prefmanager_string_changed_cb (GSettings*, gchar*, gpointer);

#endif

