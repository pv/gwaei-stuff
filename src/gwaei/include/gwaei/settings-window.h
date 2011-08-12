#ifndef GW_GTK_SETTINGS_INTERFACE_INCLUDED
#define GW_GTK_SETTINGS_INTERFACE_INCLUDED

#include <gwaei/settings-callbacks.h>
#include <gwaei/settings-listeners.h>

struct _GwSettingsWindow {
  EXTENDS_GW_WINDOW
};
typedef struct _GwSettingsWindow GwSettingsWindow;

GwSettingsWindow* gw_settingswindow_new (int);
void gw_settingswindow_destroy (GwSettingsWindow*);

void gw_settingswindow_update_interface (GwSettingsWindow*);

void gw_settingswindow_initialize_dictionary_order_list (GwSettingsWindow*);
void gw_settings_set_dictionary_source (GtkWidget*, const char*);
void gw_settingswindow_update_global_font_label (GwSettingsWindow*, const char*);
void gw_settingswindow_update_custom_font_button (GwSettingsWindow*, const char*);
void gw_settingswindow_update_dictionary_order_list (GwSettingsWindow*);
void gw_settingswindow_increment_order_list_processes (GwSettingsWindow*);
void gw_settingswindow_decrement_order_list_processes (GwSettingsWindow*);
void gw_settingswindow_set_use_global_document_font_checkbox (GwSettingsWindow*, gboolean);
void gw_settingswindow_set_search_as_you_type (GwSettingsWindow*, gboolean);
gboolean gw_settingswindow_get_search_as_you_type (GwSettingsWindow*);

#endif
