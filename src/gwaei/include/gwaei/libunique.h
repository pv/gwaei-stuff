#ifndef GW_MAIN_INTERFACE_UNIQUE_INCLUDED
#define GW_MAIN_INTERFACE_UNIQUE_INCLUDED

void gw_libunique_initialize (gboolean, char*, char*);
void gw_libunique_free (void);

typedef enum {
  GW_MESSAGE_SET_DICTIONARY = 1,
  GW_MESSAGE_SET_QUERY = 2
} GwMessage;

gboolean gw_libunique_is_unique (gboolean);

#endif
