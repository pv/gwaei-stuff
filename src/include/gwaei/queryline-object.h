#define MAX_ATOMS 20
#define MAX_ATOM_LENGTH 100

#define GW_QUERYLINE_EXIST 0
#define GW_QUERYLINE_LOCATE 1
#define GW_QUERYLINE_LOW 2
#define GW_QUERYLINE_MED 3
#define GW_QUERYLINE_HIGH 4

typedef struct GwQueryLine {
    char string[MAX_QUERY];

    char   *kanji_atom[MAX_ATOMS];
    regex_t kanji_regex[5][MAX_ATOMS];
    char   *hira_atom[MAX_ATOMS];
    regex_t hira_regex[5][MAX_ATOMS];
    char   *kata_atom[MAX_ATOMS];
    regex_t kata_regex[5][MAX_ATOMS];
    char   *roma_atom[MAX_ATOMS];
    regex_t roma_regex[5][MAX_ATOMS];
    char   *mix_atom[MAX_ATOMS];
    regex_t mix_regex[5][MAX_ATOMS];
} GwQueryLine;


GwQueryLine* gw_queryline_new (void );
void gw_queryline_free (GwQueryLine*);
