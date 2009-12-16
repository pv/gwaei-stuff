#define MAX_ATOMS 20
#define MAX_ATOM_LENGTH 100

#define GW_QUERYLINE_EXIST 0
#define GW_QUERYLINE_LOCATE 1
#define GW_QUERYLINE_LOW 2
#define GW_QUERYLINE_MED 3
#define GW_QUERYLINE_HIGH 4

typedef struct GwQueryLine {
    char string[MAX_QUERY];

    regex_t kanji_regex[5][MAX_ATOMS];
    int kanji_total;
    regex_t furi_regex[5][MAX_ATOMS];
    int furi_total;
    regex_t roma_regex[5][MAX_ATOMS];
    int roma_total;
    regex_t mix_regex[5][MAX_ATOMS];
    int mix_total;

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
