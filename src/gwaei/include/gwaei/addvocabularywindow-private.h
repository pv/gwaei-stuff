#ifndef GW_ADDVOCABULARYWINDOW_PRIVATE_INCLUDED
#define GW_ADDVOCABULARYWINDOW_PRIVATE_INCLUDED

G_BEGIN_DECLS

typedef enum {
  TOTAL_GW_ADDVOCABULARYWINDOW_TIMEOUTIDS
} GwAddVocabularyWindowTimeoutId;

typedef enum {
  TOTAL_GW_ADDVOCABULARYWINDOW_SIGNALIDS
} GwAddVocabularyWindowSignalId;

struct _GwAddVocabularyWindowPrivate {
  GtkEntry *kanji_entry;
  GtkEntry *furigana_entry;
  GtkTextView *definitions_textview;

  GtkComboBox *vocabulary_list_combobox;

  GtkButton *add_button;
  GtkButton *cancel_button;

  //Main variables
  guint timeoutid[TOTAL_GW_ADDVOCABULARYWINDOW_TIMEOUTIDS];
  guint signalid[TOTAL_GW_ADDVOCABULARYWINDOW_SIGNALIDS];

  gchar *kanji_text;
  gchar *furigana_text;
  gchar *definitions_text;
};

#define GW_ADDVOCABULARYWINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_ADDVOCABULARYWINDOW, GwAddVocabularyWindowPrivate))

G_END_DECLS

#endif
