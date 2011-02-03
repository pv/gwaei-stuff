#ifndef GW_DICTINSTLIST_HEADER_INCLUDED
#define GW_DICTINSTLIST_HEADER_INCLUDED

void gw_dictinstlist_initialize (void);
void gw_dictinstlist_free (void);

GList* gw_dictinstlist_get_list (void);
gboolean gw_dictinstlist_data_is_valid (void);

#endif
