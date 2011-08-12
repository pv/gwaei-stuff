#ifndef GW_MAIN_INCLUDED
#define GW_MAIN_INCLUDED

#include <gtk/gtk.h>

#include <libwaei/searchitem.h>

#define GW_SEARCHWINDOW_KEEP_SEARCHING_MAX_DELAY 3

typedef enum {
  GW_SEARCHWINDOW_TIMEOUT_PROGRESS,
  GW_SEARCHWINDOW_TIMEOUT_SELECTION_ICONS,
  GW_SEARCHWINDOW_TIMEOUT_KEEP_SEARCHING,
  TOTAL_GW_SEARCHWINDOW_TIMEOUTS
} GwSearchWindowTimeoutId;


typedef enum {
  GW_SEARCHWINDOW_SIGNALID_COPY,
  GW_SEARCHWINDOW_SIGNALID_CUT,
  GW_SEARCHWINDOW_SIGNALID_PASTE,
  GW_SEARCHWINDOW_SIGNALID_SELECT_ALL,
  TOTAL_GW_SEARCHWINDOW_SIGNALIDS
} GwSearchWindowSignalId;


struct _GwSearchWindowProgressFeedbackData {
  LwSearchItem *item;
  long line;
  LwSearchStatus status;
};
typedef struct _GwSearchWindowProgressFeedbackData GwSearchWindowProgressFeedbackData;

struct _GwSearchWindowMouseData {
  LwSearchItem *item;
  gint button_press_x;
  gint button_press_y;
  gunichar button_character;
  char* hovered_word; 
};
typedef struct _GwSearchWindowMouseData GwSearchWindowMouseData;

struct _GwSearchWindowKeepSearchingData {
  int delay;
  char *query;
};
typedef struct _GwSearchWindowKeepSearchingData GwSearchWindowKeepSearchingData;


struct _GwSearchWindowSelectionIconData {
  gboolean selected;
};
typedef struct _GwSearchWindowSelectionIconData GwSearchWindowSelectionIconData;


struct _GwSearchWindow {
  EXTENDS_GW_WINDOW

  GtkEntry *entry;
  GtkNotebook *notebook;
  GtkToolbar *toolbar;
  GtkWidget *statusbar;
  GtkComboBox *combobox;

  //Tabs
  GList *tablist; //!< Stores the current search item set to each tab

  //History
  LwHistoryList *history;

  //Main variables
  guint timeoutids[TOTAL_GW_SEARCHWINDOW_TIMEOUTS];
  guint signalids[TOTAL_GW_SEARCHWINDOW_SIGNALIDS];

  int previous_tip;

  gboolean new_tab; 

  GwSearchWindowSelectionIconData selectionicondata;
  GwSearchWindowProgressFeedbackData feedbackdata;
  GwSearchWindowMouseData mousedata;
  GwSearchWindowKeepSearchingData keepsearchingdata;
};
typedef struct _GwSearchWindow GwSearchWindow;


GwSearchWindow* gw_searchwindow_new (void);
void gw_searchwindow_destroy (GwSearchWindow*);

gboolean gw_searchwindow_update_progress_feedback_timeout (GwSearchWindow*);
gboolean gw_searchwindow_update_icons_for_selection_timeout (GwSearchWindow*);
gboolean gw_searchwindow_keep_searching_timeout (GwSearchWindow*);

void gw_searchwindow_update_history_popups (GwSearchWindow*);
void gw_searchwindow_update_toolbar_buttons (GwSearchWindow*);
void gw_searchwindow_update_total_results_label (GwSearchWindow*, LwSearchItem*);

char* gw_searchwindow_get_text_slice_from_buffer (GwSearchWindow*, int, int, int);
char* gw_searchwindow_get_text_by_target (GwSearchWindow*, LwOutputTarget);

gboolean gw_searchwindow_cancel_search (GwSearchWindow*, gpointer);
gboolean gw_searchwindow_cancel_search_by_searchitem (GwSearchWindow*, LwSearchItem*);
gboolean gw_searchwindow_cancel_search_by_target (GwSearchWindow*, LwOutputTarget);
gboolean gw_searchwindow_cancel_search_by_tab_number (GwSearchWindow*, const int);
gboolean gw_searchwindow_cancel_search_for_current_tab (GwSearchWindow*);
gboolean gw_searchwindow_cancel_search_by_content (GwSearchWindow*, gpointer);
void gw_searchwindow_cancel_all_searches (GwSearchWindow*);

void gw_searchwindow_display_no_results_found_page (GwSearchWindow*, LwSearchItem*);
void gw_searchwindow_initialize_buffer_by_searchitem (GwSearchWindow*, LwSearchItem*);

void gw_searchwindow_set_entry_text_by_searchitem (GwSearchWindow*, LwSearchItem*);
void gw_searchwindow_set_total_results_label_by_searchitem (GwSearchWindow*, LwSearchItem*);
void gw_searchwindow_set_search_progressbar_by_searchitem (GwSearchWindow*, LwSearchItem*);
char* gw_searchwindow_get_title_by_searchitem (GwSearchWindow*, LwSearchItem*);
void gw_searchwindow_set_title_by_searchitem (GwSearchWindow*, LwSearchItem*);

void gw_searchwindow_set_dictionary (GwSearchWindow*, int);
void gw_searchwindow_buffer_initialize_tags (GwSearchWindow*);
void gw_searchwindow_set_font (GwSearchWindow*, char*, int*);
void gw_searchwindow_buffer_initialize_marks (GtkTextBuffer*);


void gw_searchwindow_search_entry_insert (GwSearchWindow*, char*);
void gw_searchwindow_clear_search_entry (GwSearchWindow*);
void gw_searchwindow_select_all_by_target (GwSearchWindow*, LwOutputTarget);


gunichar gw_searchwindow_get_hovered_character (GwSearchWindow*, int*, int*, GtkTextIter*);
void gw_searchwindow_show_window (GwSearchWindow*, char*);
void gw_searchwindow_set_cursor (GwSearchWindow*, GdkCursorType);
guint gw_searchwindow_get_current_target_focus (GwSearchWindow*);

void gw_searchwindow_paste_text (GwSearchWindow*, LwOutputTarget);
void gw_searchwindow_cut_text (GwSearchWindow*, LwOutputTarget);
void gw_searchwindow_copy_text (GwSearchWindow*, LwOutputTarget);

void gw_searchwindow_cycle_dictionaries (GwSearchWindow*, gboolean);

void gw_searchwindow_select_none_by_target (GwSearchWindow*, LwOutputTarget);
void gw_searchwindow_strncpy_text_from_widget_by_target (GwSearchWindow*, char*, LwOutputTarget, int);

gboolean gw_searchwindow_has_selection_by_target (GwSearchWindow*, LwOutputTarget);

void gw_searchwindow_reload_tagtable_tags (GwSearchWindow*);

void gw_searchwindow_set_dictionary_by_searchitem (GwSearchWindow*, LwSearchItem*);

void gw_searchwindow_close_suggestion_box (GwSearchWindow*);
void gw_searchwindow_set_katakana_hiragana_conv (GwSearchWindow*, gboolean);
void gw_searchwindow_set_hiragana_katakana_conv (GwSearchWindow*, gboolean);
void gw_searchwindow_set_romaji_kana_conv (GwSearchWindow*, int);
void gw_searchwindow_set_less_relevant_show (GwSearchWindow*, gboolean);
void gw_searchwindow_set_use_global_document_font_checkbox (GwSearchWindow*, gboolean);
void gw_searchwindow_set_toolbar_show (gpointer, gpointer, gboolean);
void gw_searchwindow_set_toolbar_style (GwSearchWindow*, const char*);
void gw_searchwindow_set_statusbar_show (GwSearchWindow*, gboolean);
void gw_searchwindow_set_color_to_swatch (GwSearchWindow*, const char*, const char*);

void gw_searchwindow_guarantee_first_tab (GwSearchWindow*);
void gw_searchwindow_set_current_tab_text (GwSearchWindow*, const char*);

struct _GwSearchData {
  GtkTextView *view;
  GwSearchWindow *window;
};
typedef struct _GwSearchData GwSearchData;


GwSearchData* gw_searchdata_new (GtkTextView*, GwSearchWindow*);
void gw_searchdata_free (GwSearchData*);

GtkTextView* gw_searchwindow_get_current_textview (GwSearchWindow*);

void gw_searchwindow_set_current_searchitem (GwSearchWindow*, LwSearchItem*);
LwSearchItem* gw_searchwindow_get_current_searchitem (GwSearchWindow*);
void gw_searchwindow_update_tab_appearance (GwSearchWindow*);
void gw_searchwindow_update_tab_appearance_by_searchitem (GwSearchWindow*, LwSearchItem *);
void gw_searchwindow_no_results_search_for_dictionary_cb (GtkWidget*, gpointer);

int gw_searchwindow_new_tab (GwSearchWindow*);

void gw_searchwindow_start_search (GwSearchWindow*, LwSearchItem*);


#endif
