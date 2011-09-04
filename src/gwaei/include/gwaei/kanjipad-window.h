#ifndef GW_KANJIPAD_WINDOW_INCLUDED
#define GW_KANJIPAD_WINDOW_INCLUDED

#define GW_KANJIPADWINDOW_MAX_GUESSES 10


struct _GwKanjipadWindow {
  EXTENDS_GW_WINDOW;

  GtkDrawingArea *drawingarea;
  GtkDrawingArea *candidates;

  gint annotate;
  GList *strokes;
  cairo_surface_t *surface;
  cairo_surface_t *ksurface;
  GList *curstroke;
  gboolean instroke;
  char kselected[2];
  char kanji_candidates[GW_KANJIPADWINDOW_MAX_GUESSES][2];
  int total_candidates;
  GPid engine_pid;
  GIOChannel *from_engine;
  GIOChannel *to_engine;
  guint iowatchid;
};
typedef struct _GwKanjipadWindow GwKanjipadWindow;

#define GW_KANJIPADWINDOW(object) (GwKanjipadWindow*)object


GwKanjipadWindow* gw_kanjipadwindow_new (GwSearchWindow*, GList*);
void gw_kanjipadwindow_destroy (GwKanjipadWindow*);
void gw_kanjipadwindow_init (GwKanjipadWindow*, GwSearchWindow*);
void gw_kanjipadwindow_deinit (GwKanjipadWindow*);


#include <gwaei/kanjipad-callbacks.h>
#include <gwaei/kanjipad-candidatearea.h>
#include <gwaei/kanjipad-drawingarea.h>

#endif
