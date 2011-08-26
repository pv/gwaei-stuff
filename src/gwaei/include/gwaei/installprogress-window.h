#ifndef GW_INSTALL_PROGRESS_INCLUDED
#define GW_INSTALL_PROGRESS_INCLUDED

struct _GwInstallProgressWindow {
  EXTENDS_GW_WINDOW

  LwDictInst *di;

  GtkLabel *label;
  GtkLabel *sublabel;
  GtkProgressBar* progressbar;

  double install_fraction;
};
typedef struct _GwInstallProgressWindow GwInstallProgressWindow;

#define GW_INSTALLPROGRESSWINDOW(object) (GwInstallProgressWindow*)object

GwInstallProgressWindow* gw_installprogresswindow_new (GwSettingsWindow*);
void gw_installprogresswindow_destroy (GwInstallProgressWindow*);
void gw_installprogresswindow_start_cb (GtkWidget*, gpointer);
void gw_installprogresswindow_init (GwInstallProgressWindow*, GwSettingsWindow*);
void gw_installprogresswindow_deinit (GwInstallProgressWindow*);

#include <gwaei/installprogress-callbacks.h>

#endif
