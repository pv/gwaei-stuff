#ifndef GW_IO_INCLUDED
#define GW_IO_INCLUDED

#include <glib.h>


#define GW_IO_MAX_FGETS_LINE 5000
#define GW_IO_ERROR "gwaei generic error"

typedef int (*GwIoProgressCallback) (double percent, gpointer data);

struct _GwIoProgressCallbackWithData {
  GwIoProgressCallback cb;
  gpointer data;
};
typedef struct _GwIoProgressCallbackWithData GwIoProgressCallbackWithData;

typedef enum  {
  GW_IO_DECOMPRESSION_ERROR,
  GW_IO_COPY_ERROR,
  GW_IO_DOWNLOAD_ERROR,
  GW_IO_ENCODING_CONVERSION_ERROR
} GwIoErrorTypes;

extern char save_path[500];

void gw_io_write_file (const char*, const char*, gchar*, GwIoProgressCallback, gpointer, GError**);

gboolean gw_io_create_mix_dictionary (const char*, const char*, const char*, GwIoProgressCallback, gpointer, GError**);
gboolean gw_io_split_places_from_names_dictionary (const char*, const char*, const char*, GwIoProgressCallback, gpointer, GError**);
gboolean gw_io_copy_with_encoding (const char*, const char*, const char*, const char*, GwIoProgressCallback, gpointer, GError**);
gboolean gw_io_copy_file (char*, char*, GwIoProgressCallback, gpointer, GError**);
char** gw_io_get_dictionary_file_list (void);
gboolean gw_io_download_file (char*, char*, GwIoProgressCallback, gpointer, GError**);
gboolean gw_io_gunzip_file (char*, GwIoProgressCallback, gpointer, GError**);
gboolean gw_io_unzip_file (char*, GwIoProgressCallback, gpointer, GError**);
void gw_io_set_savepath (const gchar *);
const gchar* gw_io_get_savepath (void);


int gw_io_get_total_lines_for_path (char*);

#endif
