regex_t re_english;
regex_t re_radical;
regex_t re_kanji;
regex_t re_places;
regex_t re_names;
regex_t re_mix;
regex_t re_gz;
regex_t re_hexcolor;


//Adjectives
regex_t re_i_adj_past;
regex_t re_i_adj_negative;
regex_t re_i_adj_te;
regex_t re_i_adj_causative;
regex_t re_i_adj_conditional;

regex_t re_na_adj_past;
regex_t re_na_adj_negative;
regex_t re_na_adj_te;
regex_t re_na_adj_causative;
regex_t re_na_adj_conditional;




void gw_regex_initialize_constant_regular_expressions(void);
char* gw_regex_locate_offset (char*, char*, regex_t*, gint*, gint*);
