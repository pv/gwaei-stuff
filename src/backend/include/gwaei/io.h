#ifndef GW_IO_INCLUDED
#define GW_IO_INCLUDED

#include <glib.h>


#define GW_IO_MAX_FGETS_LINE 5000
#define GW_IO_ERROR "gwaei generic error"

typedef void (*GwIoDownloadCallback) (gdouble fraction);

typedef enum  {
  GW_IO_DECOMPRESSION_ERROR,
  GW_IO_COPY_ERROR,
  GW_IO_DOWNLOAD_ERROR,
  GW_IO_ENCODING_CONVERSION_ERROR
} GwIoErrorTypes;

extern char save_path[500];

void gw_io_write_file (const char*, gchar*);

gboolean gw_io_create_mix_dictionary (const char*, const char*, const char*, GError**);
gboolean gw_io_split_places_from_names_dictionary (const char*, const char*, const char*, GError**);
gboolean gw_io_copy_with_encoding (const char*, const char*, const char*, const char*, GError**);
gboolean gw_io_copy_file (char*, char*, GError**);
char** gw_io_get_dictionary_file_list (void);
gboolean gw_io_download_file (char*, char*, GwIoDownloadCallback, gpointer, GError**);

gboolean gw_io_check_for_rsync(void);
int gw_io_get_total_lines_for_path (char*);

#endif
