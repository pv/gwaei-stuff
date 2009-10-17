#define MAX_ATOMS 20
#define MAX_ATOM_LENGTH 100

typedef struct GwQueryLine {
    char string[MAX_QUERY];

    char   *kanji_atom[MAX_ATOMS];
    regex_t kanji_regex_exist[MAX_ATOMS];
    regex_t kanji_regex_locate[MAX_ATOMS];
    regex_t kanji_regex_low[MAX_ATOMS];
    regex_t kanji_regex_med[MAX_ATOMS];
    regex_t kanji_regex_high[MAX_ATOMS];

    char   *hira_atom[MAX_ATOMS];
    regex_t hira_regex_exist[MAX_ATOMS];
    regex_t hira_regex_locate[MAX_ATOMS];
    regex_t hira_regex_low[MAX_ATOMS];
    regex_t hira_regex_med[MAX_ATOMS];
    regex_t hira_regex_high[MAX_ATOMS];

    char   *kata_atom[MAX_ATOMS];
    regex_t kata_regex_exist[MAX_ATOMS];
    regex_t kata_regex_locate[MAX_ATOMS];
    regex_t kata_regex_low[MAX_ATOMS];
    regex_t kata_regex_med[MAX_ATOMS];
    regex_t kata_regex_high[MAX_ATOMS];

    char   *roma_atom[MAX_ATOMS];
    regex_t roma_regex_exist[MAX_ATOMS];
    regex_t roma_regex_locate[MAX_ATOMS];
    regex_t roma_regex_low[MAX_ATOMS];
    regex_t roma_regex_med[MAX_ATOMS];
    regex_t roma_regex_high[MAX_ATOMS];

    char   *mix_atom[MAX_ATOMS];
    regex_t mix_regex_exist[MAX_ATOMS];
    regex_t mix_regex_locate[MAX_ATOMS];
    regex_t mix_regex_low[MAX_ATOMS];
    regex_t mix_regex_med[MAX_ATOMS];
    regex_t mix_regex_high[MAX_ATOMS];
} GwQueryLine;


GwQueryLine* gw_queryline_new (void );
void gw_queryline_free (GwQueryLine*);
