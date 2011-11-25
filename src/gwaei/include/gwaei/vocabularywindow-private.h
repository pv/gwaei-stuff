#ifndef GW_VOCABULARYWINDOW_PRIVATE_INCLUDED
#define GW_VOCABULARYWINDOW_PRIVATE_INCLUDED

G_BEGIN_DECLS

typedef enum {
//  GW_VOCABULARYWINDOW_TIMEOUTID_PROGRESS,
  TOTAL_GW_VOCABULARYWINDOW_TIMEOUTIDS
} GwVocabularyWindowTimeoutId;

typedef enum {
//  GW_VOCABULARYWINDOW_SIGNALID_SPELLCHECK,
  TOTAL_GW_VOCABULARYWINDOW_SIGNALIDS
} GwVocabularyWindowSignalId;

struct _GwVocabularyWindowPrivate {
  GtkToolbar   *toolbar;

  GtkTreeView  *list_treeview;
  GtkToolbar   *list_toolbar;
  GtkListStore *list_model;

  GtkTreeView  *item_treeview;
  GtkListStore *item_model;
  GtkToolbar   *item_toolbar;

  //Main variables
  guint timeoutid[TOTAL_GW_VOCABULARYWINDOW_TIMEOUTIDS];
  guint signalid[TOTAL_GW_VOCABULARYWINDOW_SIGNALIDS];
};

#define GW_VOCABULARYWINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_VOCABULARYWINDOW, GwVocabularyWindowPrivate))

G_END_DECLS

#endif
