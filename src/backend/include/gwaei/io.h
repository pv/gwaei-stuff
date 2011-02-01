#ifndef GW_IO_INCLUDED
#define GW_IO_INCLUDED

#include <glib.h>


#define GW_IO_MAX_FGETS_LINE 5000
#define GW_IO_ERROR "gwaei generic error"

typedef enum  {
  GW_IO_DECOMPRESSION_ERROR,
  GW_IO_COPY_ERROR,
  GW_IO_DOWNLOAD_ERROR,
  GW_IO_ENCODING_CONVERSION_ERROR
} GwIoErrorTypes;


struct _GwDictInfo; //Forward declaration

extern char save_path[500];

void gw_io_write_file(const char*, gchar*);

gboolean gw_io_create_mix_dictionary(char*, char*, char*);
gboolean gw_io_split_places_from_names_dictionary(char*, char*, char*);
gboolean gw_io_copy_with_encoding( char *source_path,
                                      char *target_path,
                                      char *source_encoding,
                                      char *target_encoding,
                                      GError **error);
char** gw_io_get_dictionary_file_list (void);

gboolean gw_io_check_for_rsync(void);
int gw_io_get_total_lines_for_path (char*);

void gw_io_uninstall_dictinfo (struct _GwDictInfo*, int (char*, int, gpointer), gpointer, gboolean);
void gw_io_install_dictinfo (struct _GwDictInfo*, int (char*, int, gpointer), gpointer, gboolean, GError**);

#endif
