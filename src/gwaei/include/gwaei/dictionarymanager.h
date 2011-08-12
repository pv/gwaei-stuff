#ifndef GW_DICTIONARYMANAGER_INCLUDED
#define GW_DICTIONARYMANAGER_INCLUDED


typedef enum {
  GW_DICTIONARYMANAGER_SIGNALID_ROW_CHANGED,
  TOTAL_GW_DICTIONARYMANAGER_SIGNALIDS
} GwDictionaryManagerSignalId;


typedef enum { 
  GW_DICTIONARYMANAGER_COLUMN_IMAGE,
  GW_DICTIONARYMANAGER_COLUMN_POSITION,
  GW_DICTIONARYMANAGER_COLUMN_NAME, 
  GW_DICTIONARYMANAGER_COLUMN_LONG_NAME, 
  GW_DICTIONARYMANAGER_COLUMN_ENGINE,
  GW_DICTIONARYMANAGER_COLUMN_SHORTCUT,
  GW_DICTIONARYMANAGER_COLUMN_DICT_POINTER,
  TOTAL_GW_DICTIONARYMANAGER_COLUMNS
} GwDictionaryManagerListColumns;




struct _GwDictionaryManager {
  LW_EXTENDS_DICTINFOLIST
  GtkListStore *model;
  gulong list_update_handler_id;
  LwDictInfo *selected;
//  GwCallbackData callbacklist;
  guint signalids[TOTAL_GW_DICTIONARYMANAGER_SIGNALIDS];
};
typedef struct _GwDictionaryManager GwDictionaryManager;

GwDictionaryManager* gw_dictionarymanager_new (void);
void gw_dictionarymanager_free (GwDictionaryManager*);

void gw_dictionarymanager_reload (GwDictionaryManager*);
LwDictInfo* gw_dictionarymanager_get_selected_dictinfo (GwDictionaryManager *dm);
LwDictInfo* gw_dictionarymanager_set_selected_by_load_position (GwDictionaryManager*, int);



#endif
