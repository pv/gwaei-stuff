#ifndef GW_DICTINSTLIST_HEADER_INCLUDED
#define GW_DICTINSTLIST_HEADER_INCLUDED

void gw_dictinstlist_initialize (void);
void gw_dictinstlist_free (void);

GList* gw_dictinstlist_get_list (void);
gboolean gw_dictinstlist_data_is_valid (void);
GwDictInst* gw_dictinstlist_get_dictinst_fuzzy (const char*);
GwDictInst* gw_dictinstlist_get_dictinst_by_idstring (const char*);
GwDictInst* gw_dictinstlist_get_dictinst_by_filename (const char*);

#endif
