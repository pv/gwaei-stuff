#ifndef GW_SEARCHWINDOW_PRIVATE_INCLUDED
#define GW_SEARCHWINDOW_PRIVATE_INCLUDED

G_BEGIN_DECLS

typedef enum {
  GW_SEARCHWINDOW_TIMEOUTID_PROGRESS,
  GW_SEARCHWINDOW_TIMEOUTID_KEEP_SEARCHING,
  TOTAL_GW_SEARCHWINDOW_TIMEOUTIDS
} GwSearchWindowTimeoutId;


typedef enum {
  GW_SEARCHWINDOW_SIGNALID_SPELLCHECK,
  GW_SEARCHWINDOW_SIGNALID_KEEP_SEARCHING,
  GW_SEARCHWINDOW_SIGNALID_TOOLBAR_SHOW,
  GW_SEARCHWINDOW_SIGNALID_STATUSBAR_SHOW,
  GW_SEARCHWINDOW_SIGNALID_USE_GLOBAL_FONT,
  GW_SEARCHWINDOW_SIGNALID_CUSTOM_FONT,
  GW_SEARCHWINDOW_SIGNALID_FONT_MAGNIFICATION,
  GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_ADDED,
  GW_SEARCHWINDOW_SIGNALID_DICTIONARIES_DELETED,
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
  gboolean enabled;
};
typedef struct _GwSearchWindowKeepSearchingData GwSearchWindowKeepSearchingData;


struct _GwSearchWindowSelectionIconData {
  gboolean selected;
};
typedef struct _GwSearchWindowSelectionIconData GwSearchWindowSelectionIconData;


struct _GwSearchWindowPrivate {
  GtkEntry *entry;
  GtkNotebook *notebook;
  GtkToolbar *toolbar;
  GtkWidget *statusbar;
  GtkComboBox *combobox;
  LwDictInfo *dictinfo;

  //Tabs
  GList *tablist; //!< Stores the current search item set to each tab

  //History
  LwHistory *history;

  //Main variables
  guint timeoutid[TOTAL_GW_SEARCHWINDOW_TIMEOUTIDS];
  guint signalid[TOTAL_GW_SEARCHWINDOW_SIGNALIDS];

  int previous_tip;
  int font_size;

  gboolean new_tab; 

  GwSearchWindowSelectionIconData selectionicondata;
  GwSearchWindowProgressFeedbackData feedbackdata;
  GwSearchWindowMouseData mousedata;
  GwSearchWindowKeepSearchingData keepsearchingdata;
  GwSpellcheck *spellcheck;
};

#endif
