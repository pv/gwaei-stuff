#ifndef GW_APPLICATION_INCLUDED
#define GW_APPLICATION_INCLUDED

#include <gtk/gtk.h>

G_BEGIN_DECLS

//Boilerplate
typedef struct _GwApplication GwApplication;
typedef struct _GwApplicationClass GwApplicationClass;
typedef struct _GwApplicationPrivate GwApplicationPrivate;

#define GW_TYPE_APPLICATION              (gw_application_get_type())
#define GW_APPLICATION(obj)              (G_TYPE_CHECK_INSTANCE_CAST((obj), GW_TYPE_APPLICATION, GwApplication))
#define GW_APPLICATION_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST((klass), GW_TYPE_APPLICATION, GwApplicationClass))
#define GW_IS_APPLICATION(obj)           (G_TYPE_CHECK_INSTANCE_TYPE((obj), GW_TYPE_APPLICATION))
#define GW_IS_APPLICATION_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), GW_TYPE_APPLICATION))
#define GW_APPLICATION_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS((obj), GW_TYPE_APPLICATION, GwApplicationClass))

#define GW_APPLICATION_MAX_FONT_MAGNIFICATION  6
#define GW_APPLICATION_MIN_FONT_MAGNIFICATION -6
#define GW_APPLICATION_DEFAULT_FONT_MAGNIFICATION 0
#define GW_APPLICATION_FONT_ZOOM_STEP 2
#define GW_APPLICATION_MAX_FONT_SIZE 100
#define GW_APPLICATION_MIN_FONT_SIZE 6
#define GW_APPLICATION_DEFAULT_FONT_SIZE 12
#define GW_APPLICATION_DEFAULT_FONT "Sans 12"

typedef enum {
  GW_APP_RESOLUTION_NO_ERRORS,
  GW_APP_RESOLUTION_OUT_OF_MEMORY
} GwApplicationResolution;


struct _GwApplication {
  GtkApplication application;
  GwApplicationPrivate *priv;
};

struct _GwApplicationClass {
  GtkApplicationClass parent_class;
};

//Methods
GtkApplication *gw_application_new (int*, char***);
void gw_application_free (GwApplication *app);

GwApplicationResolution gw_application_run (GwApplication*);
void gw_application_parse_args (GwApplication*, int*, char***);
void gw_application_quit (GwApplication*);

GwWindow* gw_application_get_window_by_type (GwApplication*, GType);
GwWindow* gw_application_get_window_by_widget (GwApplication*, GtkWidget*);

const char* gw_application_get_program_name (GwApplication*);
void gw_application_cancel_all_searches (GwApplication*);

void gw_application_sync_tag_cb (GSettings*, gchar*, gpointer);

void gw_application_block_searches (GwApplication*);
void gw_application_unblock_searches (GwApplication*);
gboolean gw_application_can_start_search (GwApplication*);

void gw_application_handle_error (GwApplication*, GtkWindow*, gboolean, GError**);

/*
void gw_application_set_last_focused_searchwindow (GwApplication*, GwSearchWindow*);
GwSearchWindow* gw_application_get_last_focused_searchwindow (GwApplication*);
*/
LwPreferences* gw_application_get_preferences (GwApplication*);
GwDictInfoList* gw_application_get_dictinfolist (GwApplication*);

G_END_DECLS

#endif
