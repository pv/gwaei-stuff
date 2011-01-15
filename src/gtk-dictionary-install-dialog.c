#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/io.h>
#include <gwaei/dictionary-objects.h>
#include <gwaei/search-objects.h>
#include <gwaei/preferences.h>

#include <gwaei/gtk.h>
#include <gwaei/gtk-settings-callbacks.h>
#include <gwaei/gtk-main-interface.h>
#include <gwaei/gtk-settings-interface.h>


//!
//! @brief opens the dictionary install dialog
//!
//! @param widget Unused GtkWidget pointer
//! @param data Unused gpointer
//!
G_MODULE_EXPORT void do_open_dictionary_install_dialog (GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog = GTK_WIDGET( gtk_builder_get_object (builder, "dictionary_install_dialog" ));
  GtkWidget *settings_window = GTK_WIDGET(gtk_builder_get_object (builder, "main_window" ));
  gtk_window_set_transient_for (GTK_WINDOW (dialog), GTK_WINDOW (settings_window));
  gtk_window_set_position (GTK_WINDOW (dialog), GTK_WIN_POS_CENTER_ON_PARENT);
  gtk_widget_show (GTK_WIDGET (dialog));
}


G_MODULE_EXPORT void do_start_dictionary_install (GtkWidget *widget, gpointer data)
{
  printf("start dictionary install\n");
}

G_MODULE_EXPORT void do_toggle_other_dictionary_show (GtkWidget *widget, gpointer data)
{
  gw_ui_load_gtk_builder_xml ("install.ui");

  GtkWidget *checkbox = GTK_WIDGET (data);
  GtkWidget *table = GTK_WIDGET (widget);
  GtkWidget *dialog = GTK_WIDGET (gtk_builder_get_object (builder, "dictionary_install_dialog"));

  gboolean active = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (checkbox));
  if (active) gtk_widget_show (table);
  else gtk_widget_hide (table);
}


