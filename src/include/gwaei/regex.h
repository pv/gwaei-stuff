regex_t re_english;
regex_t re_radical;
regex_t re_kanji;
regex_t re_places;
regex_t re_names;
regex_t re_mix;
regex_t re_gz;
regex_t re_hexcolor;

void gw_regex_initialize_constant_regular_expressions(void);
char* gw_regex_locate_offset (char*, char*, regex_t*, gint*, gint*);
