#include <string.h>
#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


//!
//! @brief Setup the installprogress.c source code
//!
GwInstallProgressWindow* gw_installprogresswindow_new ()
{
  GwInstallProgressWindow *temp;

  temp = (GwInstallProgressWindow*) malloc(sizeof(GwInstallProgressWindow));

  if (temp != NULL)
  {
    temp->builder = gtk_builder_new ();
    temp->window = GTK_WINDOW (gtk_builder_get_object (temp->builder, "installprogress_dialog"));
    install_fraction = 0.0;
  }

  return temp;
}


//!
//! @brief Free the memory used by the installprogress.c source code
//!
void gw_installprogresswindow_destroy (GwInstallProgressDialog *installprogress)
{
  gtk_widget_destroy (GTK_WIDGET (temp->window));
  free (installprogress);
}

