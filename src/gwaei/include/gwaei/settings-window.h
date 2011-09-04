#ifndef GW_SETTINGSWINDOW_INCLUDED
#define GW_SETTINGSWINDOW_INCLUDED

#include <gwaei/settings-callbacks.h>

typedef enum {
  GW_SETTINGSWINDOW_SIGNALID_ROMAJI_KANA,
  GW_SETTINGSWINDOW_SIGNALID_HIRA_KATA,
  GW_SETTINGSWINDOW_SIGNALID_KATA_HIRA,
  GW_SETTINGSWINDOW_SIGNALID_USE_GLOBAL_DOCUMENT_FONT,
  GW_SETTINGSWINDOW_SIGNALID_GLOBAL_DOCUMENT_FONT,
  GW_SETTINGSWINDOW_SIGNALID_CUSTOM_FONT,
  GW_SETTINGSWINDOW_SIGNALID_SEARCH_AS_YOU_TYPE,
  GW_SETTINGSWINDOW_SIGNALID_SPELLCHECK,
  GW_SETTINGSWINDOW_SIGNALID_MATCH_FG,
  GW_SETTINGSWINDOW_SIGNALID_MATCH_BG,
  GW_SETTINGSWINDOW_SIGNALID_COMMENT_FG,
  GW_SETTINGSWINDOW_SIGNALID_COMMENT_BG,
  GW_SETTINGSWINDOW_SIGNALID_HEADER_FG,
  GW_SETTINGSWINDOW_SIGNALID_HEADER_BG,
  TOTAL_GW_SETTINGSWINDOW_SIGNALIDS
} GwSettingsWindowSignalIds;

/*
typedef enum {
  TOTAL_GW_SETTINGSWINDOW_TIMEOUTIDS
} GwSettingsWindowTimeoutIds;
*/

struct _GwSettingsWindow {
  EXTENDS_GW_WINDOW

  GtkNotebook *notebook;

  guint signalids[TOTAL_GW_SETTINGSWINDOW_SIGNALIDS];
//  guint timeoutids[TOTAL_GW_SETTINGSWINDOW_TIMEOUTIDS];

  LwDictInstList *dictinstlist;
};
typedef struct _GwSettingsWindow GwSettingsWindow;

#define GW_SETTINGSWINDOW(object) (GwSettingsWindow*)object

GwSettingsWindow* gw_settingswindow_new (GwSearchWindow*, GList*);
void gw_settingswindow_destroy (GwSettingsWindow*);
void gw_settingswindow_init (GwSettingsWindow*, GwWindow*);
void gw_settingswindow_deinit (GwSettingsWindow*);

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
void gw_settingswindow_check_for_dictionaries (GwSettingsWindow*);

#endif
