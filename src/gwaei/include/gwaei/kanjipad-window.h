#ifndef GW_KANJIPAD_WINDOW_INCLUDED
#define GW_KANJIPAD_WINDOW_INCLUDED

#define MAX_GUESSES 10


struct _GwKanjipadWindow {
  EXTENDS_GW_WINDOW;

  GtkDrawingArea *drawingarea;
  GtkDrawingArea *candidates;

  gint annotate;
  GList *strokes;
  cairo_surface_t *surface;
  cairo_surface_t *ksurface;
  GList *curstroke;
  int instroke;
  char kselected[2];
  char kanji_candidates[MAX_GUESSES][2];
  int total_candidates;
  int engine_pid;
  GIOChannel *from_engine;
  GIOChannel *to_engine;
  char *data_file;
};
typedef struct _GwKanjipadWindow GwKanjipadWindow;

#define GW_KANJIPADWINDOW(object) (GwKanjipadWindow*)object


//Methods
/*
GwKanjipad* padarea_create (GtkWidget*);
void padarea_clear (GwKanjipad*);
void padarea_set_annotate (GwKanjipad*, gint);
void padarea_changed_callback (GwKanjipad*);
void padarea_init_engine (GwKanjipad*);
*/

GwKanjipadWindow* gw_kanjipadwindow_new (void);
void gw_kanjipad_destroy (GwKanjipadWindow*);
void gw_kanjipadwindow_init (GwKanjipadWindow*);
void gw_kanjipadwindow_deinit (GwKanjipadWindow*);


#include <gwaei/kanjipad-window.h>
#include <gwaei/kanjipad-callbacks.h>
#include <gwaei/kanjipad-candidatearea.h>
#include <gwaei/kanjipad-drawingarea.h>

#endif
