#ifndef GW_VOCABULARYMODEL_PRIVATE_INCLUDED
#define GW_VOCABULARYMODEL_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _GwVocabularyModelPrivate {
  gchar* name;
  gchar* filename;
  LwVocabularyList *vocabulary_list;
  gboolean has_changes;
  gboolean loaded;
};

#define GW_VOCABULARYMODEL_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_VOCABULARYMODEL, GwVocabularyModelPrivate))

G_END_DECLS

#endif

