#ifndef GW_DEFINITIONS_INCLUDED
#define GW_DEFINITIONS_INCLUDED

#define MAX_QUERY        250
#define MAX_DICTIONARIES 20
#define MAX_DICTIONARY   100
#define MAX_RESULTS      1000000
#define MAX_LINE         5000 
#define MAX_NMATCH       5
#define MAX_CHUNK        300
#define MAX_HIGH_RELIVENT_RESULTS 200
#define MAX_MEDIUM_IRRELIVENT_RESULTS 100
#define MAX_LOW_IRRELIVENT_RESULTS    50

#define GW_HISTORY_TIME_TO_RELEVANCE 20

#define GW_MAX_FONT_MAGNIFICATION  6
#define GW_MIN_FONT_MAGNIFICATION -6
#define GW_DEFAULT_FONT_MAGNIFICATION 0

#define GW_FONT_ZOOM_STEP 2

#define GW_MAX_FONT_SIZE 100
#define GW_MIN_FONT_SIZE 6
#define GW_DEFAULT_FONT_SIZE 12

#define GW_DEFAULT_FONT "Sans 12"

//GConfig
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

#define GW_ENGLISH_URI_DEFAULT  "ftp://ftp.monash.edu.au/pub/nihongo/edict.gz"
#define GW_KANJI_URI_DEFAULT    "ftp://ftp.monash.edu.au/pub/nihongo/kanjidic.gz,ftp://ftp.monash.edu.au/pub/nihongo/kradfile.gz"
#define GW_NAMES_URI_DEFAULT    "@NAMES_DEFAULT_URI@"
#define GW_RADICALS_URI_DEFAULT "@RADICALS_DEFAULT_URI@"
#define GW_EXAMPLES_URI_DEFAULT "http://www.csse.monash.edu.au/~jwb/examples.gz"
#define GW_FRENCH_URI_DEFAULT   "@FRENCH_DEFAULT_URI@"
#define GW_GERMAN_URI_DEFAULT   "@GERMAN_DEFAULT_URI@"
#define GW_SPANISH_URI_DEFAULT  "@SPANISH_DEFAULT_URI@"
#define GW_LOAD_ORDER_DEFAULT   "edict/English;kanji/Kanji;edict/Names;edict/Places;examples/Examples"


#define HIRAGANA  "(あ)|(い)|(う)|(え)|(お)|(か)(き)|(く)|(け)|(こ)|(が)|(ぎ)|(ぐ)|(げ)|(ご)|(さ)|(し)|(す)|(せ)|(そ)|(ざ)|(じ)|(ず)|(ぜ)|(ぞ)|(た)|(ち)(つ)|(て)|(と)|(だ)|(ぢ)|(づ)|(で)|(ど)|(な)|(に)|(ぬ)|(ね)|(の)|(は)(ひ)|(ふ)|(へ)|(ほ)|(ば)|(び)(ぶ)|(べ)|(ぼ)|(ぱ)|(ぴ)|(ぷ)|(ぺ)|(ぽ)(ま)|(み)|(む)|(め)|(も)|(や)|(ゆ)|(よ)|(ら)|(り)|(る)(れ)|(ろ)|(わ)|(を)|(ん)(ぁ)|(ぃ)|(ぇ)|(ぉ)"
#define KATAKANA "(ア)|(イ)|(ウ)|(エ)|(オ)|(カ)|(キ)|(ク)|(ケ)|(コ)|(ガ)|(ギ)|(グ)|(ゲ)|(ゴ)|(サ)|(シ)|(ス)|(セ)|(ソ)|(ザ)|(ジ)|(ズ)|(ゼ)|(ゾ)|(タ)|(チ)|(ツ)|(テ)|(ト)|(ダ)|(ジ)|(ヅ)|(デ)|(ド)|(ナ)|(ニ)|(ヌ)|(ネ)|(ノ)|(ハ)|(ヒ)|(フ)|(ヘ)|(ホ)|(バ)|(ビ)|(ブ)|(ベ)|(ボ)|(パ)|(ピ)|(プ)|(ペ)|(ポ)|(マ)|(ミ)|(ム)|(メ)|(モ)|(ヤ)|(ユ)|(ヨ)|(ラ)|(リ)|(ル)|(レ)|(ロ)|(ワ)|(ヲ)|(ン)|(ァ)|(ィ)|(ェ)|(ォ)"

#define DELIMITOR_STR     ";"
#define DELIMITOR_CHR     ';'

#define GW_GENERIC_ERROR "gwaei generic error"

#define GW_FILE_ERROR 1


#endif