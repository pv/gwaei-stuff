#ifndef GW_GTK_MAIN_INTERFACE_TABS_INCLUDED
#define GW_GTK_MAIN_INTERFACE_TABS_INCLUDED

#include <gwaei/searchitem-object.h> // needed for the GwSearchItem below

void gw_tabs_initialize (void);
void gw_tabs_free (void);

GList *gw_tabs_get_searchitem_list (void);

int gw_tabs_new (void);
void gw_tabs_guarantee_first_tab (void);

void do_new_tab (GtkWidget*, gpointer);
void do_tab_remove (GtkWidget*, gpointer);
void do_prep_and_start_search_in_new_tab (GtkWidget*, gpointer);
void do_no_results_search_for_dictionary (GtkWidget*, gpointer);
void do_destroy_tab_menuitem_searchitem_data (GtkObject*, gpointer);
void gw_tabs_set_searchitem_by_page_num (GwSearchItem*, int);
void gw_tabs_set_current_tab_text (const char*);

#endif
