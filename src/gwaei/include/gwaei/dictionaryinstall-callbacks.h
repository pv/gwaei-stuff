#ifndef GW_DICTIONARY_INSTALL_CALLBACKS_INCLUDED
#define GW_DICTIONARY_INSTALL_CALLBACKS_INCLUDED

void gw_dictionaryinstall_source_changed_cb (GSettings*, char*, gpointer);
void gw_dictionaryinstall_cursor_changed_cb (GtkTreeView*, gpointer);
void gw_dictionaryinstall_select_file_cb (GtkWidget*, gpointer);
void gw_dictionaryinstall_listitem_toggled_cb (GtkCellRendererToggle*, gchar*, gpointer);
void gw_dictionaryinstall_reset_default_uri_cb (GtkWidget*, gpointer);
void gw_dictionaryinstall_filename_entry_changed_cb (GtkWidget*, gpointer);
void gw_dictionaryinstall_source_entry_changed_cb (GtkWidget*, gpointer);
void gw_dictionaryinstall_engine_combobox_changed_cb (GtkWidget*, gpointer);
void gw_dictionaryinstall_encoding_combobox_changed_cb (GtkWidget*, gpointer);
void gw_dictionaryinstall_compression_combobox_changed_cb (GtkWidget*, gpointer);
void gw_dictionaryinstall_split_checkbox_toggled_cb (GtkWidget*, gpointer);
void gw_dictionaryinstall_merge_checkbox_toggled_cb (GtkWidget*, gpointer);

#endif
