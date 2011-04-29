#ifndef GW_DICTINST_HEADER_INCLUDED
#define GW_DICTINST_HEADER_INCLUDED

#define GW_DICTINST_ERROR "gWaei Dictionary Installer Error"

typedef enum {
  GW_DICTINST_ERROR_SOURCE_PATH,
  GW_DICTINST_ERROR_TARGET_PATH,
  GW_DICTINST_ERROR_FILE_MOVE
} GwDictInstError;

typedef enum {
  GW_DICTINST_NEEDS_DOWNLOADING,
  GW_DICTINST_NEEDS_DECOMPRESSION,
  GW_DICTINST_NEEDS_TEXT_ENCODING,
  GW_DICTINST_NEEDS_POSTPROCESSING,
  GW_DICTINST_NEEDS_FINALIZATION,
  GW_DICTINST_NEEDS_NOTHING,
  GW_DICTINST_TOTAL_URIS,
} GwDictInstUri;

typedef enum {
  GW_DICTINST_STATUS_UNSET,
  GW_DICTINST_STATUS_DOWNLOADING,
  GW_DICTINST_STATUS_COPYING,
  GW_DICTINST_STATUS_ENCODING,
  GW_DICTINST_STATUS_DECOMPRESSING,
  GW_DICTINST_STATUS_POSTPROCESSING,
  GW_DICTINST_STATUS_FINALIZING,
  GW_DICTINST_STATUS_INSTALLED,
  GW_DICTINST_STATUS_NOT_INSTALLED,
  GW_DICTINST_STATUS_REMOVING,
  GW_DICTINST_TOTAL_STATUSES
} GwDictInstStatus;


struct _GwDictInst {
  char *filename;
  char *shortname;
  char *longname;
  char *description;
  char *uri[GW_DICTINST_TOTAL_URIS];
  double progress;
  gboolean selected;
  char *schema;
  char *key;
  gboolean builtin;
  gulong listenerid;            //!< An id to hold the g_signal_connect value when the source copy uri pref is set
  gboolean listenerid_is_set;   //!< Allows disconnecting the signal on destruction of the GwDictInst
  GwCompression compression;    //!< Path to the gziped dictionary file
  GwEncoding encoding;          //!< Path to the raw unziped dictionary file
  GwEngine engine;
  GwDictInstStatus status;
  gboolean split;
  gboolean merge;
  GMutex *mutex;
};
typedef struct _GwDictInst GwDictInst;

GwDictInst* gw_dictinst_new_using_pref_uri (const char*, 
                                            const char*,
                                            const char*,
                                            const char*,
                                            const char*,
                                            const char*,
                                            const GwEngine,
                                            const GwCompression,
                                            const GwEncoding,
                                            gboolean, gboolean, gboolean);

GwDictInst* gw_dictinst_new (const char*,
                             const char*,
                             const char*,
                             const char*,
                             const char*,
                             const GwEngine,
                             const GwCompression,
                             const GwEncoding,
                             gboolean, gboolean, gboolean);

void gw_dictinst_free (GwDictInst*);

void gw_dictinst_set_filename (GwDictInst*, const char*);
void gw_dictinst_set_engine (GwDictInst*, const GwEngine);
void gw_dictinst_set_encoding (GwDictInst*, const GwEncoding);
void gw_dictinst_set_compression (GwDictInst*, const GwCompression);
void gw_dictinst_set_download_source (GwDictInst*, const char*);
void gw_dictinst_set_split (GwDictInst *di, const gboolean);
void gw_dictinst_set_merge (GwDictInst *di, const gboolean);
void gw_dictinst_set_status (GwDictInst *di, const GwDictInstStatus);
gchar* gw_dictinst_get_status_string (GwDictInst*, gboolean);

void gw_dictinst_regenerate_save_target_uris (GwDictInst*);
gboolean gw_dictinst_data_is_valid (GwDictInst*);

gboolean gw_dictinst_install (GwDictInst*, GwIoProgressCallback, GError**);

#endif
