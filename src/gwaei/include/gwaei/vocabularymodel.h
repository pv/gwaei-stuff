#ifndef GW_VOCABULARYMODEL_INCLUDED
#define GW_VOCABULARYMODEL_INCLUDED

G_BEGIN_DECLS


typedef enum { 
  GW_VOCABULARYMODEL_COLUMN_KANJI,
  GW_VOCABULARYMODEL_COLUMN_FURIGANA,
  GW_VOCABULARYMODEL_COLUMN_DEFINITIONS,
  TOTAL_GW_VOCABULARYMODEL_COLUMNS
} GwVocabularyModelColumn;

//Boilerplate
typedef struct _GwVocabularyModel GwVocabularyModel;
typedef struct _GwVocabularyModelClass GwVocabularyModelClass;
typedef struct _GwVocabularyModelPrivate GwVocabularyModelPrivate;

#define GW_TYPE_VOCABULARYMODEL              (gw_vocabularymodel_get_type())
#define GW_VOCABULARYMODEL(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_VOCABULARYMODEL, GwVocabularyModel))
#define GW_VOCABULARYMODEL_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_VOCABULARYMODEL, GwVocabularyModelClass))
#define GW_IS_VOCABULARYMODEL(obj)                    (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_VOCABULARYMODEL))
#define GW_IS_VOCABULARYMODEL_CLASS(klass)            (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_VOCABULARYMODEL))
#define GW_VOCABULARYMODEL_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_VOCABULARYMODEL, GwVocabularyModelClass))

struct _GwVocabularyModel {
  GtkListStore model;
  GwVocabularyModelPrivate *priv;
};

struct _GwVocabularyModelClass {
  GtkListStoreClass parent_class;
};

//Methods
GtkListStore* gw_vocabularymodel_new (const gchar*);
GType gw_vocabularymodel_get_type (void) G_GNUC_CONST;

void gw_vocabularymodel_save (GwVocabularyModel*);
void gw_vocabularymodel_load (GwVocabularyModel*);
gboolean gw_vocabularymodel_loaded (GwVocabularyModel*);
const gchar* gw_vocabularymodel_get_name (GwVocabularyModel*);

G_END_DECLS

#endif
