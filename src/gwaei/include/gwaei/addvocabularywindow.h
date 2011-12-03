#ifndef GW_ADDVOCABULARYWINDOW_INCLUDED
#define GW_ADDVOCABULARYWINDOW_INCLUDED

#include <libwaei/searchitem.h>

G_BEGIN_DECLS

//Boilerplate
typedef struct _GwAddVocabularyWindow GwAddVocabularyWindow;
typedef struct _GwAddVocabularyWindowClass GwAddVocabularyWindowClass;
typedef struct _GwAddVocabularyWindowPrivate GwAddVocabularyWindowPrivate;

#define GW_TYPE_ADDVOCABULARYWINDOW              (gw_addvocabularywindow_get_type())
#define GW_ADDVOCABULARYWINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_ADDVOCABULARYWINDOW, GwAddVocabularyWindow))
#define GW_ADDVOCABULARYWINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_ADDVOCABULARYWINDOW, GwAddVocabularyWindowClass))
#define GW_IS_ADDVOCABULARYWINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_ADDVOCABULARYWINDOW))
#define GW_IS_ADDVOCABULARYWINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_ADDVOCABULARYWINDOW))
#define GW_ADDVOCABULARYWINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_ADDVOCABULARYWINDOW, GwAddVocabularyWindowClass))

struct _GwAddVocabularyWindow {
  GwWindow window;
  GwAddVocabularyWindowPrivate *priv;
};

struct _GwAddVocabularyWindowClass {
  GwWindowClass parent_class;
};

GtkWindow* gw_addvocabularywindow_new (GtkApplication *application);
GType gw_addvocabularywindow_get_type (void) G_GNUC_CONST;

const gchar* gw_addvocabularywindow_get_kanji (GwAddVocabularyWindow *);
const gchar* gw_addvocabularywindow_get_furigana (GwAddVocabularyWindow *);
const gchar* gw_addvocabularywindow_get_definitions (GwAddVocabularyWindow*);
GtkListStore* gw_addvocabularywindow_get_wordstore (GwAddVocabularyWindow*);

#include "addvocabularywindow-callbacks.h"

G_END_DECLS

#endif

