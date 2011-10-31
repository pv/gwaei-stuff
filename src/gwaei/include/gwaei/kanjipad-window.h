#ifndef GW_KANJIPAD_WINDOW_INCLUDED
#define GW_KANJIPAD_WINDOW_INCLUDED

G_BEGIN_DECLS

//Boilerplate
typedef struct _GwKanjipadWindow GwKanjipadWindow;
typedef struct _GwKanjipadWindowClass GwKanjipadWindowClass;
typedef struct _GwKanjipadWindowPrivate GwKanjipadWindowPrivate;

#define GW_TYPE_KANJIPADWINDOW              (gw_kanjipadwindow_get_type())
#define GW_KANJIPADWINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_KANJIPADWINDOW, GwKanjipadWindow))
#define GW_KANJIPADWINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_KANJIPADWINDOW, GwKanjipadWindowClass))
#define GW_IS_KANJIPADWINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_KANJIPADWINDOW))
#define GW_IS_KANJIPADWINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_KANJIPADWINDOW))
#define GW_KANJIPADWINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_KANJIPADWINDOW, GwKanjipadWindowClass))

#define GW_KANJIPADWINDOW_MAX_GUESSES 10

struct _GwKanjipadWindow {
  GwWindow window;
  GwKanjipadWindowPrivate *priv;
};

struct _GwKanjeipadWindowClass {
  GwWindowClass parent_class;
};

GtkWindow* gw_kanjipadwindow_new (GtkApplication *application);
GType gw_kanjipadwindow_get_type (void) G_GNUC_CONST;

void gw_kanjipadwindow_init (GwKanjipadWindow*, GwSearchWindow*);
void gw_kanjipadwindow_deinit (GwKanjipadWindow*);


#include "kanjipad-callbacks.h"
#include "kanjipad-candidatearea.h"
#include "kanjipad-drawingarea.h"

#endif
