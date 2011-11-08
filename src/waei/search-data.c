#include <waei/waei.h>


WSearchData* w_searchdata_new (GMainLoop *loop, WApplication *application)
{
    //Sanity check
    g_assert (loop != NULL && application != NULL);

    //Declarations
    WSearchData *temp;

    //Initializations
    temp = g_new (WSearchData, 1);

    if (temp != NULL)
    {
      temp->loop = loop;
      temp->application = application;
      temp->less_relevant_header_set = FALSE;
    }

    return temp;
}


void w_searchdata_free (WSearchData *sdata)
{
  g_free (sdata);
}

