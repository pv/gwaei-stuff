#ifndef LW_VOCABULARYITEM_INCLUDED
#define LW_VOCABULARYITEM_INCLUDED

typedef enum {
  LW_VOCABULARYITEM_FIELD_KANJI,
  LW_VOCABULARYITEM_FIELD_FURIGANA,
  LW_VOCABULARYITEM_FIELD_DEFINITIONS,
  TOTAL_LW_VOCABULARYITEM_FIELDS
} LwVocabularyItemField;


struct _LwVocabularyItem {
  gchar *fields[TOTAL_LW_VOCABULARYITEM_FIELDS];
};

typedef struct _LwVocabularyItem LwVocabularyItem;

#define LW_VOCABULARYITEM(obj) (LwVocabularyItem*)obj

LwVocabularyItem* lw_vocabularyitem_new ();
LwVocabularyItem* lw_vocabularyitem_new_from_string (const gchar*);
void lw_vocabularyitem_free (LwVocabularyItem*);

void lw_vocabularyitem_set_kanji (LwVocabularyItem*, const gchar*);
const gchar* lw_vocabularyitem_get_kanji (LwVocabularyItem*);

void lw_vocabularyitem_set_furigana (LwVocabularyItem*, const gchar*);
const gchar* lw_vocabularyitem_get_furigana (LwVocabularyItem*);

void lw_vocabularyitem_set_definitions (LwVocabularyItem*, const gchar*);
const gchar* lw_vocabularyitem_get_definitions (LwVocabularyItem*);

gchar* lw_vocabularyitem_to_string (LwVocabularyItem*);

#endif
