#ifndef GW_GTK_MAIN_INTERFACE_TABS_INCLUDED
#define GW_GTK_MAIN_INTERFACE_TABS_INCLUDED

#include <libwaei/searchitem.h> // needed for the LwSearchItem below

void gw_tabs_initialize (void);
void gw_tabs_free (void);

GList *gw_tabs_get_searchitem_list (void);

int gw_tabs_new (void);
void gw_tabs_guarantee_first (void);

void gw_tabs_new_cb (GtkWidget*, gpointer);
void gw_tabs_new_with_search_cb (GtkWidget*, gpointer);
void gw_tabs_no_results_search_for_dictionary_cb (GtkWidget*, gpointer);
void gw_tabs_destroy_tab_menuitem_searchitem_data_cb (GObject*, gpointer);
void gw_tabs_remove_cb (GtkWidget*, gpointer);

void gw_tabs_set_current_tab_text (const char*);
void gw_tabs_set_searchitem (LwSearchItem *item);
LwSearchItem* gw_tabs_get_searchitem (void);
void gw_tabs_update_on_deck_historylist_by_searchitem (LwSearchItem*);

gboolean gw_tabs_cancel_search_by_tab_number (const int);
gboolean gw_tabs_cancel_search_for_current_tab (void);
void gw_tabs_cancel_all_searches (void);
gboolean gw_tabs_cancel_search_by_content (gpointer);



#endif
