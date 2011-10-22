#ifndef GW_WINDOW_INCLUDED
#define GW_WINDOW_INCLUDED

#include <gtk/gtk.h>

G_BEGIN_DECLS

//Boilerplate
typedef struct _GwWindow GwWindow;
typedef struct _GwWindowClass GwWindowClass;
typedef struct _GwWindowPrivate GwWindowPrivate;

#define GW_TYPE_WINDOW              (gw_window_get_type())
#define GW_WINDOW(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_WINDOW, GwWindow))
#define GW_WINDOW_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_WINDOW, GwWindowClass))
#define GW_IS_WINDOW(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_WINDOW))
#define GW_IS_WINDOW_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_WINDOW))
#define GW_WINDOW_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_WINDOW, GwWindowClass))

struct _GwWindow {
  GtkWindow window;
  GwWindowPrivate *priv;
};

struct _GwWindowClass {
  GtkWindowClass parent_class;
};

//Methods
GtkWindow* gw_window_new (void);
GType gw_window_get_type (void) G_GNUC_CONST;
GObject* gw_window_get_object (GwWindow*, const gchar*);

void gw_window_set_transient_for (GwWindow*, GwWindow*);
gboolean gw_window_load_ui_xml (GwWindow*, const char*);

G_END_DECLS

#endif
