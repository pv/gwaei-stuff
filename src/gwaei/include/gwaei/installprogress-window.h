#ifndef GW_INSTALL_PROGRESS_INCLUDED
#define GW_INSTALL_PROGRESS_INCLUDED

struct _GwInstallProgressWindow {
  EXTENDS_GW_WINDOW

  double install_fraction;
};
typedef struct _GwInstallProgressWindow GwInstallProgressWindow;

GwInstallProgressWindow* gw_installprogresswindow_new (void);
void gw_installprogress_destroy (GwInstallProgressWindow*);
void gw_installprogress_start_cb (GtkWidget*, gpointer);

#endif
