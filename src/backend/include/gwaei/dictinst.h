#ifndef GW_DICTINST_HEADER_INCLUDED
#define GW_DICTINST_HEADER_INCLUDED

typedef enum {
//  GW_DICTINST_COMPRESSION_ZIP, //Unsupported since you can't tell what the file will be named
  GW_DICTINST_COMPRESSION_GZIP,
  GW_DICTINST_COMPRESSION_NONE,
  GW_DICTINST_COMPRESSION_TOTAL
} GwDictInstCompression;

typedef enum {
  GW_DICTINST_ENCODING_UTF8,
  GW_DICTINST_ENCODING_EUC_JP,
  GW_DICTINST_ENCODING_SHIFT_JS,
  GW_DICTINST_ENCODING_TOTAL
} GwDictInstEncoding;

typedef enum {
  GW_DICTINST_DOWNLOAD_SOURCE,
  GW_DICTINST_COMPRESSED_FILE,
  GW_DICTINST_TEXT_ENCODING,
  GW_DICTINST_FINAL_TARGET,
  GW_DICTINST_TOTAL_URI,
} GwDictInstUri;


struct _GwDictInst {
  char *filename;
  char *shortname;
  char *longname;
  char *description;
  char *uri[GW_DICTINST_TOTAL_URI];
  int progress;
  char *status_message;
  char *schemaid;
  char *key;
  gboolean builtin;
  gulong listenerid;
  gboolean listenerid_is_set;
  GwDictInstCompression compression;    //!< Path to the gziped dictionary file
  GwDictInstEncoding encoding;          //!< Path to the raw unziped dictionary file
  GwDictEngine engine;
  gboolean split_dictionary;
  gboolean merge_dictionary;
  GMutex *mutex;
};
typedef struct _GwDictInst GwDictInst;

GwDictInst* gw_dictinst_new_using_pref_uri (const char*, 
                                            const char*,
                                            const char*,
                                            const char*,
                                            const char*,
                                            const char*,
                                            const GwDictEngine,
                                            const GwDictInstCompression,
                                            const GwDictInstEncoding,
                                            gboolean, gboolean, gboolean);

GwDictInst* gw_dictinst_new (const char*,
                             const char*,
                             const char*,
                             const char*,
                             const char*,
                             const GwDictEngine,
                             const GwDictInstCompression,
                             const GwDictInstEncoding,
                             gboolean, gboolean, gboolean);

void gw_dictinst_free (GwDictInst*);

#endif
