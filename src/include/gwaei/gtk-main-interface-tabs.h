#ifndef GW_GTK_MAIN_INTERFACE_TABS_INCLUDED
#define GW_GTK_MAIN_INTERFACE_TABS_INCLUDED

GList *gw_tab_get_searchitem_list (void);
void gw_tab_set_searchitem_by_page_num (GwSearchItem*, int);

void do_new_tab (GtkWidget*, gpointer);
void do_tab_remove (GtkWidget*, gpointer);

#endif
