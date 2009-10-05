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
    long total_lines;
    char name[100];
    char long_name[100];
    char path[FILENAME_MAX];
    char gz_path[FILENAME_MAX];
    char sync_path[FILENAME_MAX];
    char rsync[FILENAME_MAX];
    char gckey[100];
    int load_position;
};
typedef struct DictionaryInfo DictionaryInfo;


struct DictionaryList
{
    GList *list;
    GList *selected;
    int id_increment;
};
typedef struct DictionaryList DictionaryList;


GList* dictionarylist_get_list (void);
DictionaryInfo* dictionarylist_get_dictionary_by_id (int);
int dictionarylist_get_total (void);
GList* dictionarylist_get_selected(void);


DictionaryInfo* dictionarylist_get_dictionary_by_name (const char*);
/*
 *  Returns a dictionary in the DictionaryList by name.  Unlike
 *  dictionarylist_get_dictionary_by_alias, it will alway return what you ask
 *  for.  When the dictionary doesn't exist, it returns null.
 */


DictionaryInfo* dictionarylist_get_dictionary_by_alias(const char*);
/*
 *  Searches for a dictionary by a name in the DictionaryList.  When approprate
 *  it will swap the requsted dictionary for another one.  This usally comes out
 *  to swapping the Kanji dictionary for the Mixed one when it is present.  When
 *  the dictionary is not found, it returns null.
 */
