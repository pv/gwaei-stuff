#ifndef GW_GTK_SETTINGS_INTERFACE_INSTALL_LINE_INCLUDED
#define GW_GTK_SETTINGS_INTERFACE_INSTALL_LINE_INCLUDED

GwUiDictInstallLine *gw_ui_new_dict_install_line (GwDictInfo*);
void gw_ui_dict_install_set_action_button (GwUiDictInstallLine*, const gchar*, gboolean);
void gw_ui_dict_install_set_message (GwUiDictInstallLine*, const gchar*, const char*);

#endif
