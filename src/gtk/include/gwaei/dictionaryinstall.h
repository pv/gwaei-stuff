#ifndef GW_DICTIONARY_INSTALL_HEADER_INCLUDED
#define GW_DICTIONARY_INSTALL_HEADER_INCLUDED

void gw_dictionaryinstall_initialize (void);
void gw_dictionaryinstall_free (void);

void gw_dictionaryinstall_source_changed_cb (GSettings*, char*, gpointer);
void gw_dictionaryinstall_cursor_changed_cb (GtkTreeView*, gpointer);
void gw_dictionaryinstall_select_file_cb (GtkWidget*, gpointer);
void gw_dictionaryinstall_listitem_toggled_cb (GtkCellRendererToggle*, gchar*, gpointer);
void gw_dictionaryinstall_reset_default_uri_cb (GtkWidget*, gpointer);

#endif
