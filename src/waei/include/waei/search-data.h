#ifndef W_SEARCHDATA_INCLUDED
#define W_SEARCHDATA_INCLUDED

#define W_SEARCHDATA(object) (WSearchData*)object

struct _WSearchData {
  GMainLoop *loop;
  WApplication *application;
};
typedef struct _WSearchData WSearchData;

WSearchData* w_searchdata_new (GMainLoop*, WApplication*);
void w_searchdata_free (WSearchData*);

#endif


