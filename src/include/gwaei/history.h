#define GWAEI_HISTORYLIST_RESULTS 0
#define GWAEI_HISTORYLIST_KANJI   1

#define GWAEI_SEARCH_IDLE 0
#define GWAEI_SEARCH_SEARCHING 1
#define GWAEI_SEARCH_CANCELING 2

/*Searchitem primitives*/

struct SearchItem {
  char query[MAX_QUERY];
  GwaeiDictInfo* dictionary;

  FILE* fd;
  int status;
  char *input;
  char *output;
  int target;
  long current_line;
  char comparison_buffer[LINE_MAX + 2];
  gboolean show_less_relevant_results;

  int total_relevant_results;
  int total_irrelevant_results;
  int total_results;
  gboolean results_found;

  regex_t re_exist[MAX_QUERY];
  regex_t re_locate[MAX_QUERY];

  regex_t re_relevance_medium[MAX_QUERY];
  regex_t re_relevance_high[MAX_QUERY];
  GList *results_medium;
  GList *results_low;

  int total_re;
};
typedef struct SearchItem SearchItem;


//searchitem methods
SearchItem* searchitem_new    (char*, GwaeiDictInfo*, int);
void        searchitem_remove (struct SearchItem*);


/*Historylist primitives*/

struct HistoryList
{
    GList *back;
    GList *forward;
    SearchItem *current;
};
typedef struct HistoryList HistoryList;

/*Historylist methods*/
HistoryList* historylist_new_item(HistoryList*, char*, char*);
HistoryList* historylist_add_item(HistoryList*, SearchItem*);
HistoryList* historylist_unlink_item(HistoryList*);
HistoryList* historylist_remove_last_item(HistoryList*);
void   historylist_clear(HistoryList*, GList**);
void   historylist_shift_item(HistoryList*, GList**);


/*Functions*/
gboolean searchitem_do_pre_search_prep (SearchItem*);
HistoryList* historylist_get_list(const int);
SearchItem* historylist_get_current (const int);
GList* historylist_get_combined_history_list (const int);
GList* historylist_get_back_history (const int);
GList* historylist_get_forward_history (const int);
