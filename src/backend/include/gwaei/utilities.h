#ifndef GW_UTILITIES_INCLUDED
#define GW_UTILITIES_INCLUDED

#define HIRAGANA  "(あ)|(い)|(う)|(え)|(お)|(か)(き)|(く)|(け)|(こ)|(が)|(ぎ)|(ぐ)|(げ)|(ご)|(さ)|(し)|(す)|(せ)|(そ)|(ざ)|(じ)|(ず)|(ぜ)|(ぞ)|(た)|(ち)(つ)|(て)|(と)|(だ)|(ぢ)|(づ)|(で)|(ど)|(な)|(に)|(ぬ)|(ね)|(の)|(は)(ひ)|(ふ)|(へ)|(ほ)|(ば)|(び)(ぶ)|(べ)|(ぼ)|(ぱ)|(ぴ)|(ぷ)|(ぺ)|(ぽ)(ま)|(み)|(む)|(め)|(も)|(や)|(ゆ)|(よ)|(ら)|(り)|(る)(れ)|(ろ)|(わ)|(を)|(ん)(ぁ)|(ぃ)|(ぇ)|(ぉ)"
#define KATAKANA "(ア)|(イ)|(ウ)|(エ)|(オ)|(カ)|(キ)|(ク)|(ケ)|(コ)|(ガ)|(ギ)|(グ)|(ゲ)|(ゴ)|(サ)|(シ)|(ス)|(セ)|(ソ)|(ザ)|(ジ)|(ズ)|(ゼ)|(ゾ)|(タ)|(チ)|(ツ)|(テ)|(ト)|(ダ)|(ジ)|(ヅ)|(デ)|(ド)|(ナ)|(ニ)|(ヌ)|(ネ)|(ノ)|(ハ)|(ヒ)|(フ)|(ヘ)|(ホ)|(バ)|(ビ)|(ブ)|(ベ)|(ボ)|(パ)|(ピ)|(プ)|(ペ)|(ポ)|(マ)|(ミ)|(ム)|(メ)|(モ)|(ヤ)|(ユ)|(ヨ)|(ラ)|(リ)|(ル)|(レ)|(ロ)|(ワ)|(ヲ)|(ン)|(ァ)|(ィ)|(ェ)|(ォ)"

typedef enum {
  GW_PATH_BASE, 
  GW_PATH_DICTIONARY,
  GW_PATH_DICTIONARY_EDICT,
  GW_PATH_DICTIONARY_KANJI,
  GW_PATH_DICTIONARY_EXAMPLES,
  GW_PATH_DICTIONARY_UNKNOWN,
  GW_PATH_PLUGIN,
  GW_PATH_CACHE,
  GW_PATH_TOTAL
} GwFolderPath;

//!
//! @brief Dictionary type assigned by the program.  It determines the parsing algorithm
//!
typedef enum {  
  GW_ENGINE_EDICT,         //!< Standard edict format dictionary
  GW_ENGINE_KANJI,         //!< Kanjidic format dictionary
  GW_ENGINE_EXAMPLES,      //!< Examples format dictionary
  GW_ENGINE_UNKNOWN,          //!< Unkown format which should use safe parsing
  GW_ENGINE_TOTAL
} GwEngine;

typedef enum {
//  GW_COMPRESSION_ZIP, //Unsupported since you can't tell what the file will be named
  GW_COMPRESSION_GZIP,
  GW_COMPRESSION_NONE,
  GW_COMPRESSION_TOTAL
} GwCompression;

typedef enum {
  GW_ENCODING_UTF8,
  GW_ENCODING_EUC_JP,
  GW_ENCODING_SHIFT_JS,
  GW_ENCODING_TOTAL
} GwEncoding;


const char* gw_util_get_directory (const GwFolderPath);
const char* gw_util_get_directory_for_engine (const GwEngine);
const char* gw_util_get_engine_name (const GwEngine ENGINE);
GwEngine gw_util_get_engine_from_enginename (const char*);
const char* gw_util_get_compression_name (const GwCompression);
const char* gw_util_get_encoding_name (const GwEncoding);


char* gw_util_next_hira_char_from_roma (char*);
char* gw_util_roma_to_hira (char*, char*);
gboolean gw_util_str_roma_to_hira (char*, char*, int);
gboolean gw_util_is_japanese_locale (void);
gboolean gw_util_is_japanese_ctype (void);


gboolean gw_util_is_hiragana_str (char*);
gboolean gw_util_is_util_kanji_str (char*);
gboolean gw_util_is_katakana_str (char*);
gboolean gw_util_is_romaji_str (char*);
gboolean gw_util_is_kanji_ish_str (char*);
gboolean gw_util_is_kanji_str (char*);
gboolean gw_util_is_furigana_str (char*);

void gw_util_str_shift_kata_to_hira (char*);
void gw_util_str_shift_hira_to_kata (char*);


gboolean gw_util_all_chars_are_in_range (char*, int, int);

gboolean gw_util_force_japanese_locale (void);

gchar* gw_util_prepare_query(char*, gboolean);
gchar* gw_util_sanitize_input(char*, gboolean);
gboolean gw_util_contains_halfwidth_japanese(gchar*);
gchar* gw_util_enlarge_halfwidth_japanese(gchar*);

#endif
