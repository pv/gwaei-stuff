#ifndef GW_WINDOW_PRIVATE_INCLUDED
#define GW_WINDOW_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _GwWindowPrivate {
  GtkBuilder *builder;
  GwApplication *application;
};

#define GW_WINDOW_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_WINDOW, GwWindowPrivate))

void gw_window_private_init (GwWindow*);
void gw_window_private_finalize (GwWindow*);

G_END_DECLS

#endif

