#ifndef LW_PREFMANAGER_INCLUDED
#define LW_PREFMANAGER_INCLUDED

#include <gio/gio.h>
#include <libwaei/prefmanager-callbacks.h>

//GSettings
#define LW_SCHEMA_GNOME_INTERFACE   "org.gnome.desktop.interface"
#define LW_KEY_TOOLBAR_STYLE        "toolbar-style"
#define LW_KEY_DOCUMENT_FONT_NAME   "font-name"
#define LW_KEY_PROGRAM_VERSION      "version"

/////////////////////////
#define LW_SCHEMA_BASE             "org.gnome.gwaei"
#define LW_KEY_TOOLBAR_SHOW        "toolbar-show"
#define LW_KEY_STATUSBAR_SHOW      "statusbar-show"
#define LW_KEY_LESS_RELEVANT_SHOW  "less-relevant-results-show"
#define LW_KEY_HIRA_KATA           "query-hiragana-to-katakana"
#define LW_KEY_KATA_HIRA           "query-katakana-to-hiragana"
#define LW_KEY_ROMAN_KANA          "query-romanji-to-kana"
#define LW_KEY_SPELLCHECK          "query-spellcheck"
#define LW_KEY_SEARCH_AS_YOU_TYPE  "search-as-you-type"
#define LW_KEY_WINDOW_POSITIONS    "window-positions"

//////////////////////////
#define LW_SCHEMA_FONT               "org.gnome.gwaei.fonts"
#define LW_KEY_FONT_USE_GLOBAL_FONT  "use-global-document-font"
#define LW_KEY_FONT_CUSTOM_FONT      "custom-document-font"
#define LW_KEY_FONT_MAGNIFICATION    "magnification"

////////////////////////////
#define LW_SCHEMA_HIGHLIGHT     "org.gnome.gwaei.highlighting"
#define LW_KEY_MATCH_FG         "match-foreground"
#define LW_KEY_MATCH_BG         "match-background"
#define LW_KEY_HEADER_FG        "header-foreground"
#define LW_KEY_HEADER_BG        "header-background"
#define LW_KEY_COMMENT_FG       "comment-foreground"

#define LW_MATCH_FG_DEFAULT       "#000000"
#define LW_MATCH_BG_DEFAULT       "#CCEECC"
#define LW_HEADER_FG_DEFAULT      "#EE1111"
#define LW_HEADER_BG_DEFAULT      "#FFDEDE"
#define LW_COMMENT_FG_DEFAULT     "#2222DD"

////////////////////////////
#define LW_SCHEMA_DICTIONARY       "org.gnome.gwaei.dictionary"
#define LW_KEY_ENGLISH_SOURCE      "english-source"
#define LW_KEY_KANJI_SOURCE        "kanji-source"
#define LW_KEY_NAMES_PLACES_SOURCE "names-places-source"
#define LW_KEY_EXAMPLES_SOURCE     "examples-source"
#define LW_KEY_LOAD_ORDER          "load-order"

#define LW_PREFMANAGER(object) (LwPrefManager*) object

//! PrefCallback form for being stored in a list
typedef void (*LwPrefCallback)(gpointer, gpointer, gboolean);

//! The data for uniquely identifying a callback
struct _LwPrefCallbackData {
  const char *schema;
  const char *key;
  LwPrefCallback func;
  gpointer data;
};
typedef struct _LwPrefCallbackData LwPrefCallbackData;

LwPrefCallbackData* lw_prefcallbackdata_new (const char*, const char*, LwPrefCallback, gpointer);
void lw_prefcallbackdata_free (LwPrefCallbackData*);



struct _LwPrefManager {
  GList *settingslist;
  GList *callbacklist;
  GMutex *mutex;

  gboolean toolbar_show; 
  gboolean statusbar_show;
  gboolean query_katakana_to_hiragana;
  gboolean query_hiragana_to_katakana;
  gint query_romaji_to_kana;
  gboolean query_spellcheck;
  gboolean search_as_you_type;
  gchar* window_positions;

  gchar *dictionary_load_order;
  gchar *dictionary_english_source;
  gchar *dictionary_kanji_source;
  gchar *dictionary_names_places_source;
  gchar *dictionary_examples_source;

  gboolean use_global_document_font;
  gchar *custom_document_font;
  gint magnification;
  
  gchar *comment_foreground;
  gchar *comment_background;
  gchar *match_foreground;
  gchar *match_background;
  gchar *header_foreground;
  gchar *header_background;
};
typedef struct _LwPrefManager LwPrefManager;

LwPrefManager* lw_prefmanager_new (void);
void lw_prefmanager_free (LwPrefManager*);

void lw_prefmanager_add_callback (LwPrefManager*, const char*, const char*, LwPrefCallback, gpointer);
void lw_prefmanager_destroy_callbacks_for_matching_data (LwPrefManager*, gpointer);

GSettings* lw_prefmanager_get_settings_object (LwPrefManager*, const char*);

void lw_prefmanager_reset_value (GSettings*, const char*);
void lw_prefmanager_reset_value_by_schema (LwPrefManager*, const char*, const char*);

int lw_prefmanager_get_int (GSettings*, const char *);
int lw_prefmanager_get_int_by_schema (LwPrefManager*, const char*, const char *);

void lw_prefmanager_set_int (GSettings*, const char*, const int);
void lw_prefmanager_set_int_by_schema (LwPrefManager*, const char*, const char*, const int);

gboolean lw_prefmanager_get_boolean (GSettings*, const char *);
gboolean lw_prefmanager_get_boolean_by_schema (LwPrefManager*, const char*, const char*);

void lw_prefmanager_set_boolean (GSettings*, const char*, const gboolean);
void lw_prefmanager_set_boolean_by_schema (LwPrefManager*, const char*, const char*, const gboolean);

void lw_prefmanager_get_string (char*, GSettings*, const char*, const int);
void lw_prefmanager_get_string_by_schema (LwPrefManager*, char*, const char*, const char*, const int);

void lw_prefmanager_set_string (GSettings*, const char*, const char*);
void lw_prefmanager_set_string_by_schema (LwPrefManager*, const char*, const char*, const char*);

gulong lw_prefmanager_add_change_listener (GSettings*, const char*, void (*callback_function) (GSettings*, gchar*, gpointer), gpointer);
gulong lw_prefmanager_add_change_listener_by_schema (LwPrefManager*, const char*, const char*, void (*callback_function) (GSettings*, gchar*, gpointer), gpointer);

void lw_prefmanager_remove_change_listener (GSettings*, gulong);
void lw_prefmanager_remove_change_listener_by_schema (LwPrefManager*, const char*, gulong);


#endif





