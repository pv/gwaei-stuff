#include <gtk/gtk.h>

/* Used by external programs for directing kanjipad's output*/
void kanjipad_set_target_text_widget(GtkWidget*);
GtkWidget* kanjipad_get_target_text_widget(void);

void initialize_kanjipad(void);
void show_kanjipad(GtkBuilder*);
