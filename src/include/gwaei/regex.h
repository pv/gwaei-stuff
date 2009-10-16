regex_t re_english;
regex_t re_radical;
regex_t re_kanji;
regex_t re_places;
regex_t re_names;
regex_t re_mix;
regex_t re_gz;
regex_t re_hexcolor;


regex_t re_i_adj_pastform;
regex_t re_na_adj_pastform;
regex_t re_verb_presentform;
regex_t re_verb_pastform_negative;
regex_t re_verb_pastform;
regex_t re_verb_negative;
regex_t re_verb_politepast;
regex_t re_i_adj_negative;
regex_t re_na_adj_negative;
regex_t re_verb_te_form;
regex_t re_i_adj_te_form;
regex_t re_na_adj_te_form;
regex_t re_verb_potention;
regex_t re_verb_causative;
regex_t re_i_adj_causative;
regex_t re_na_adj_causative;
regex_t re_conditional;
regex_t re_i_adj_conditional;
regex_t re_na_adj_conditional;
regex_t re_negative_conditional;
regex_t re_verb_imperative;
regex_t re_verb_passive;
regex_t re_verb_volitional;




void gw_regex_initialize_constant_regular_expressions(void);
char* gw_regex_locate_offset (char*, char*, regex_t*, gint*, gint*);
