#ifndef GW_UTILITIES_INCLUDED
#define GW_UTILITIES_INCLUDED

enum gw_runmodes {
  GW_CONSOLE_RUNMODE,
  GW_GTK_RUNMODE,
  GW_QT_RUNMODE,
  GW_NCURSES_RUNMODE,
  GW_TOTAL_RUNMODES
};

gboolean gw_util_itoa(int, char *, const int);
char* gw_util_get_waei_directory( char* );
void initialize_gconf_schemas(void);
gboolean gw_util_itohexstr(char*, guint);
char* gw_util_next_hira_char_from_roma(char*);
char* gw_util_roma_to_hira(char*, char*);
gboolean gw_util_str_roma_to_hira (char*, char*, int);
gboolean gw_util_is_japanese_locale(void);
gboolean gw_util_is_japanese_ctype(void);

int gw_util_get_runmode(void);

gboolean gw_util_is_hiragana_str(char*);
gboolean gw_util_is_util_kanji_str(char*);
gboolean gw_util_is_katakana_str(char*);
gboolean gw_util_is_romaji_str(char*);

char *gw_util_strdup_args_to_query (int, char**);

#define IS_HEXCOLOR(color) (regexec(&re_hexcolor, (color), 1, NULL, 0) == 0)

#endif
