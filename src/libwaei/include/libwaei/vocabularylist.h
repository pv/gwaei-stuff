#ifndef LW_VOCABULARYLIST_INCLUDED
#define LW_VOCABULARYLIST_INCLUDED

struct _LwVocabularyList {
  gchar *name;
  GList *items;
  gboolean changed;
  gdouble progress;
};

typedef struct _LwVocabularyList LwVocabularyList;

#define LW_VOCABULARYLIST(obj) (LwVocabularyList*)obj

gchar** lw_vocabularylist_get_lists ();
LwVocabularyList* lw_vocabularylist_new (const gchar*);
void lw_vocabularylist_free (LwVocabularyList*);

void lw_vocabularylist_save (LwVocabularyList*, LwIoProgressCallback);
void lw_vocabularylist_load (LwVocabularyList*, LwIoProgressCallback);

#endif
