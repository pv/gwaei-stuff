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
  GW_IO_READ_ERROR,
  GW_IO_WRITE_ERROR,
  GW_IO_DECOMPRESSION_ERROR,
  GW_IO_COPY_ERROR,
  GW_IO_DOWNLOAD_ERROR,
  GW_IO_ENCODING_CONVERSION_ERROR
} GwIoErrorTypes;

void gw_io_write_file (const char*, const char*, gchar*, GwIoProgressCallback, gpointer, GError**);
char** gw_io_get_dictionary_file_list (void);
size_t gw_io_get_filesize (const char*);

gboolean gw_io_create_mix_dictionary (const char*, const char*, const char*, GwIoProgressCallback, gpointer, GError**);
gboolean gw_io_split_places_from_names_dictionary (const char*, const char*, const char*, GwIoProgressCallback, gpointer, GError**);
gboolean gw_io_copy_with_encoding (const char*, const char*, const char*, const char*, GwIoProgressCallback, gpointer, GError**);
gboolean gw_io_copy (const char*, const char*, GwIoProgressCallback, gpointer, GError**);
gboolean gw_io_remove (const char*, GError**);
gboolean gw_io_download (char*, char*, GwIoProgressCallback, gpointer, GError**);
gboolean gw_io_gunzip_file (const char*, const char*, GwIoProgressCallback, gpointer, GError **);
gboolean gw_io_unzip_file (char*, GwIoProgressCallback, gpointer, GError**);

void gw_io_set_savepath (const gchar *);
const gchar* gw_io_get_savepath (void);

gboolean gw_io_pipe_data (char**, const char *source_path, const char *target_path, GwIoProgressCallback cb, gpointer data, GError **error);
void gw_io_set_cancel_operations (gboolean);
int gw_io_get_total_lines_for_file (const char*);

#endif
