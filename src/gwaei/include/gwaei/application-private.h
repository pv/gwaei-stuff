#ifndef GW_APPLICATION_PRIVATE_INCLUDED
#define GW_APPLICATION_PRIVATE_INCLUDED

G_BEGIN_DECLS

struct _GwApplicationPrivate {
  int* argc;
  char*** argv;

  LwPreferences *preferences;
  GwDictInfoList *dictinfolist;
  LwDictInstList *dictinstlist;
//  LwEngine *engine;
  GtkTextTagTable *tagtable;
  GwSearchWindow *last_focused;

  GOptionContext *context;
  gboolean arg_new_window_switch;
  gchar   *arg_dictionary;
  gchar   *arg_query;
  gboolean arg_version_switch;

  int block_new_searches;
};

#define GW_APPLICATION_GET_PRIVATE(object)(G_TYPE_INSTANCE_GET_PRIVATE ((object), GW_TYPE_APPLICATION, GwApplicationPrivate))

G_END_DECLS

#endif
