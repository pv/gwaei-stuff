#define GW_CONSOLE_RUNMODE 0
#define GW_GTK_RUNMODE     1
#define GW_QT_RUNMODE      2

gboolean gw_itoa(int, char *, const int);
char* get_waei_directory( char* );
void initialize_gconf_schemas(void);
gboolean gw_itohexstr(char*, guint);
char* gw_next_hiragana_char_from_romaji(char*);
char* gw_romaji_to_hiragana(char*, char*);
gboolean is_japanese_locale(void);
gboolean is_japanese_ctype(void);

int gw_util_get_runmode(void);

gboolean gw_util_is_hiragana_str(char*);
gboolean gw_util_is_util_kanji_str(char*);
gboolean gw_util_is_katakana_str(char*);
gboolean gw_util_is_romaji_str(char*);
#define IS_HEXCOLOR(color) (regexec(&re_hexcolor, (color), 1, NULL, 0) == 0)
