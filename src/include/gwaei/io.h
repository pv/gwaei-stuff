#ifndef GW_IO_INCLUDED
#define GW_IO_INCLUDED

char save_path[FILENAME_MAX];

void gw_io_write_file(const char*, gchar*);

gboolean gw_io_create_mix_dictionary(char*, char*, char*);
gboolean gw_io_split_places_from_names_dictionary(char*, char*, char*);
gboolean gw_io_copy_with_encoding( char *source_path,
                                      char *target_path,
                                      char *source_encoding,
                                      char *target_encoding,
                                      GError **error);

gboolean gw_io_check_for_rsync(void);

#endif
