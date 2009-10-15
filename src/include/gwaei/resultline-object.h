typedef struct GwResultLine {
    char string[MAX_LINE];

    //General result things
    char *def_start[50];
    char *number[50];
    int def_total;
    char *kanji_start;
    char *furigana_start;
    char *classification_start;
    char first[5];

    //Kanji things
    char *strokes;
    char *frequency;
    char *readings[2];
    char *meanings;
    char *grade;
    char *jlpt;
    char *kanji;
    char *radicals;

    gboolean important;

} GwResultLine;


GwResultLine* gw_resultline_new (void );
void gw_resultline_parse_normal_result_string (GwResultLine*, char*);
void gw_resultline_parse_kanji_result_string (GwResultLine*, char*);
void gw_resultline_parse_radical_result_string (GwResultLine*, char*);
void gw_resultline_parse_examples_result_string (GwResultLine*, char*);
void gw_resultline_parse_unknown_result_string (GwResultLine*, char*);

