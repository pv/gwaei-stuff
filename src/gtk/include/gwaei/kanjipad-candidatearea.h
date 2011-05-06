#ifndef GW_KANJIPAD_CANDIDATEAREA_INCLUDED
#define GW_KANJIPAD_CANDIDATEAREA_INCLUDED

gboolean candidatearea_configure_event (GtkWidget*, GdkEventConfigure*);
gboolean candidatearea_expose_event (GtkWidget*, GdkEventExpose*);
gboolean candidatearea_button_press_event (GtkWidget*, GdkEventButton*);
void candidatearea_draw (GtkWidget*);
void gw_kanjipad_candidatearea_initialize (GwKanjipad*);

#endif

