char save_path[FILENAME_MAX];

void gw_io_write_file(const char*, gchar*);
char rsync_path[FILENAME_MAX];

gboolean rsync_exists;

gboolean gw_io_create_mix_dictionary(char*, char*, char*);
gboolean gw_io_split_places_from_names_dictionary(char*, char*, char*);
gboolean gw_io_copy_with_encoding( char *source_path,
                                      char *target_path,
                                      char *source_encoding,
                                      char *target_encoding,
                                      GError **error);
