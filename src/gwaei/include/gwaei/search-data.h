#ifndef GW_SEARCHDATA_INCLUDED
#define GW_SEARCHDATA_INCLUDED

#define GW_SEARCHDATA(object) (GwSearchData*)object

struct _GwSearchData {
  GtkTextView *view;
  GwSearchWindow *window;
};
typedef struct _GwSearchData GwSearchData;

GwSearchData* gw_searchdata_new (GtkTextView*, GwSearchWindow*);
void gw_searchdata_free (GwSearchData*);

#endif
