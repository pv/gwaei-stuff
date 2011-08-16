
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


GwSearchData* gw_searchdata_new (GtkTextView *view, GwSearchWindow *window)
{
    GwSearchData *temp;
    temp = (GwSearchData*) malloc(sizeof(GwSearchData));
    if (temp != NULL)
    {
      temp->window = window;
      temp->view = view;
    }
    return temp;
}

void gw_searchdata_free (GwSearchData *data)
{
    g_assert (data != NULL);

    free (data);
}


