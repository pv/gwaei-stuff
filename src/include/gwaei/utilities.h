#define GWAEI_CONSOLE_RUNMODE 0
#define GWAEI_GTK_RUNMODE     1
#define GWAEI_QT_RUNMODE      2

gboolean gwaei_itoa(int, char *, const int);
char* get_waei_directory( char* );
void initialize_gconf_schemas(void);
gboolean gwaei_itohexstr(char*, guint);
char* gwaei_next_hiragana_char_from_romanji(char*);
char* gwaei_romanji_to_hiragana(char*, char*);
gboolean is_japanese_locale(void);
gboolean is_japanese_ctype(void);

int gwaei_util_get_runmode(void);

gboolean gwaei_util_is_hiragana_str(char*);
gboolean gwaei_util_is_util_kanji_str(char*);
gboolean gwaei_util_is_katakana_str(char*);
gboolean gwaei_util_is_romanji_str(char*);
#define IS_HEXCOLOR(color) (regexec(&re_hexcolor, (color), 1, NULL, 0) == 0)
