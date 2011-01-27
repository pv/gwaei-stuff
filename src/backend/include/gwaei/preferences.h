#ifndef GW_GSETTINGS_PREFERENCES_INCLUDED
#define GW_GSETTINGS_PREFERENCES_INCLUDED

#include <gio/gio.h>

//GSettings
#define GW_SCHEMA_GNOME_INTERFACE   "org.gnome.interface"
#define GW_KEY_TOOLBAR_STYLE        "toolbar-style"
#define GW_KEY_DOCUMENT_FONT_NAME   "document-font-name"

/////////////////////////
#define GW_SCHEMA_BASE             "org.gnome.gwaei"
#define GW_KEY_TOOLBAR_SHOW        "toolbar-show"
#define GW_KEY_LESS_RELEVANT_SHOW  "less-relevant-results-show"
#define GW_KEY_HIRA_KATA           "query-hiragana-to-katakana"
#define GW_KEY_KATA_HIRA           "query-katakana-to-hiragana"
#define GW_KEY_ROMAN_KANA          "query-romanji-to-kana"
#define GW_KEY_SPELLCHECK          "query-spellcheck"
#define GW_KEY_WINDOW_POSITIONS    "window-positions"

//////////////////////////
#define GW_SCHEMA_FONT               "org.gnome.gwaei.fonts"
#define GW_KEY_FONT_USE_GLOBAL_FONT  "use-global-document-font"
#define GW_KEY_FONT_CUSTOM_FONT      "custom-document-font"
#define GW_KEY_FONT_MAGNIFICATION    "magnification"

////////////////////////////
#define GW_SCHEMA_HIGHLIGHT     "org.gnome.gwaei.highlighting"
#define GW_KEY_MATCH_FG         "match-foreground"
#define GW_KEY_MATCH_BG         "match-background"
#define GW_KEY_HEADER_FG        "header-foreground"
#define GW_KEY_HEADER_BG        "header-background"
#define GW_KEY_COMMENT_FG       "comment-foreground"

#define GW_MATCH_FG_DEFAULT       "#000000"
#define GW_MATCH_BG_DEFAULT       "#CCEECC"
#define GW_HEADER_FG_DEFAULT      "#EE1111"
#define GW_HEADER_BG_DEFAULT      "#FFDEDE"
#define GW_COMMENT_FG_DEFAULT     "#2222DD"

////////////////////////////
#define GW_SCHEMA_DICTIONARY   "org.gnome.gwaei.dictionary"
#define GW_KEY_ENGLISH_SOURCE  "english-source"
#define GW_KEY_KANJI_SOURCE    "kanji-source"
#define GW_KEY_NAMES_SOURCE    "names-source"
#define GW_KEY_PLACES_SOURCE   "places-source"
#define GW_KEY_RADICALS_SOURCE "radicals-source"
#define GW_KEY_EXAMPLES_SOURCE "examples-source"
#define GW_KEY_LOAD_ORDER      "load-order"
#define GW_KEY_FRENCH_SOURCE   "french-source"
#define GW_KEY_GERMAN_SOURCE   "german-source"
#define GW_KEY_SPANISH_SOURCE  "spanish-source"

#define GW_ENGLISH_URI_DEFAULT      "ftp://ftp.monash.edu.au/pub/nihongo/edict.gz"
#define GW_KANJI_URI_DEFAULT        "ftp://ftp.monash.edu.au/pub/nihongo/kanjidic.gz,ftp://ftp.monash.edu.au/pub/nihongo/kradfile.gz"
#define GW_NAMES_PLACES_URI_DEFAULT "ftp://ftp.monash.edu.au/pub/nihongo/enamdict.gz"
#define GW_EXAMPLES_URI_DEFAULT     "http://www.csse.monash.edu.au/~jwb/examples.gz"
#define GW_LOAD_ORDER_DEFAULT       "edict/English;kanji/Kanji;edict/Names;edict/Places;examples/Examples"

void gw_pref_set_int (const char*, const char*, const int);
int gw_pref_get_int (const char*, const char*);

void gw_pref_set_boolean (const char*, const char*, const gboolean);
gboolean gw_pref_get_boolean (const char*, const char*);

void gw_pref_set_string (const char*, const char*, const char*);
void gw_pref_get_string (char*, const char*, const char*, const int);

void gw_prefs_add_change_listener (const char*, const char*, void (GSettings*, gchar*, gpointer), gpointer);

void gw_pref_reset_value (const char*, const char*);

#endif

