#ifndef GW_DICTIONARY_INSTALL_INCLUDED
#define GW_DICTIONARY_INSTALL_INCLUDED

struct _GwDictInstWindow {
  EXTENDS_GW_WINDOW

  GtkListStore *encoding_store;
  GtkListStore *compression_store;
  GtkListStore *engine_store;
  GtkListStore *dictionary_store;
  LwDictInst *di;
};
typedef struct _GwDictInstWindow GwDictInstWindow;

void gw_dictinstwindow_new (void);
void gw_dictinstwindow_destroy (void);

#endif
