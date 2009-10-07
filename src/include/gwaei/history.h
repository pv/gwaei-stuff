#define GWAEI_HISTORYLIST_RESULTS 0
#define GWAEI_HISTORYLIST_KANJI   1

#define GWAEI_SEARCH_IDLE 0
#define GWAEI_SEARCH_SEARCHING 1
#define GWAEI_SEARCH_CANCELING 2

/*Searchitem primitives*/

struct GwSearchItem {
  char query[MAX_QUERY];
  GwDictInfo* dictionary;

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
typedef struct GwSearchItem GwSearchItem;


//searchitem methods
GwSearchItem* gw_searchitem_new    (char*, GwDictInfo*, int);
void        gw_searchitem_remove (struct GwSearchItem*);


/*Historylist primitives*/

struct GwHistoryList
{
    GList *back;
    GList *forward;
    GwSearchItem *current;
};
typedef struct GwHistoryList GwHistoryList;

/*Historylist methods*/
GwHistoryList* historylist_new_item(GwHistoryList*, char*, char*);
GwHistoryList* historylist_add_item(GwHistoryList*, GwSearchItem*);
GwHistoryList* historylist_unlink_item(GwHistoryList*);
GwHistoryList* historylist_remove_last_item(GwHistoryList*);
void   historylist_clear(GwHistoryList*, GList**);
void   historylist_shift_item(GwHistoryList*, GList**);


/*Functions*/
gboolean gw_searchitem_do_pre_search_prep (GwSearchItem*);
GwHistoryList* historylist_get_list(const int);
GwSearchItem* historylist_get_current (const int);
GList* historylist_get_combined_history_list (const int);
GList* historylist_get_back_history (const int);
GList* historylist_get_forward_history (const int);
