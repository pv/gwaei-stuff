#define INSTALLING    0
#define INSTALLED     1
#define NOT_INSTALLED 2
#define UPDATING      3
#define UPDATED       4
#define CANCELING     5
#define CANCELED      6
#define ERRORED       7
#define REBUILDING    8

#define OTHER    0
#define ENGLISH  1
#define KANJI    2
#define RADICALS 3
#define NAMES    4
#define PLACES   5
#define MIX      6


struct DictionaryInfo
{
    int id;
    int type;
    int status;
    char name[100];
    char path[FILENAME_MAX];
    char gz_path[FILENAME_MAX];
    char sync_path[FILENAME_MAX];
    char rsync[FILENAME_MAX];
    char gckey[100];
};
typedef struct DictionaryInfo DictionaryInfo;


struct DictionaryList
{
    GList *list;
    GList *selected;
};
typedef struct DictionaryList DictionaryList;


GList* dictionarylist_get_list (void);
DictionaryInfo* dictionarylist_get_dictionary_by_id (int);
DictionaryInfo* dictionarylist_get_dictionary_by_name (const char*);
int dictionarylist_get_total (void);
