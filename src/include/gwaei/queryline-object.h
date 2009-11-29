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
    char   *furi_atom[MAX_ATOMS];
    regex_t furi_regex[5][MAX_ATOMS];
    char   *roma_atom[MAX_ATOMS];
    regex_t roma_regex[5][MAX_ATOMS];
    char   *mix_atom[MAX_ATOMS];
    regex_t mix_regex[5][MAX_ATOMS];

    //Kanji things
    char strokes[10];
    char frequency[10];
    char readings[100];
    char meanings[100];
    char grade[10];
    char jlpt[10];
    char kanji[MAX_ATOMS][5];
} GwQueryLine;


GwQueryLine* gw_queryline_new (void );
void gw_queryline_free (GwQueryLine*);
