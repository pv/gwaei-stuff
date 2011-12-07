#ifndef GW_VOCABULARYWINDOW_PRIVATE_INCLUDED
#define GW_VOCABULARYWINDOW_PRIVATE_INCLUDED

G_BEGIN_DECLS


typedef enum {
  GW_VOCABULARYWINDOW_TIMEOUTID_UNUSED,
  TOTAL_GW_VOCABULARYWINDOW_TIMEOUTIDS
} GwVocabularyWindowTimeoutId;

typedef enum {
  GW_VOCABULARYWINDOW_SIGNALID_UNUSED,
  GW_VOCABULARYWINDOW_SIGNALID_CHANGED,
  TOTAL_GW_VOCABULARYWINDOW_SIGNALIDS
} GwVocabularyWindowSignalId;

struct _GwVocabularyWindowPrivate {
  GtkTreeView  *list_treeview;
  GtkToolbar   *list_toolbar;

  GtkTreeView  *word_treeview;
  GtkToolbar   *word_toolbar;
  GtkToolbar   *study_toolbar;
  GtkToggleToolButton *edit_toolbutton;

  gboolean has_changes;

  //Main variables
  guint timeoutid[TOTAL_GW_VOCABULARYWINDOW_TIMEOUTIDS];
  guint signalid[TOTAL_GW_VOCABULARYWINDOW_SIGNALIDS];

  GtkCellRenderer *renderer[TOTAL_GW_VOCABULARYWORDSTORE_COLUMNS];
};

#define GW_VOCABULARYWINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_VOCABULARYWINDOW, GwVocabularyWindowPrivate))

G_END_DECLS

#endif
