#ifndef GW_DICTINST_HEADER_INCLUDED
#define GW_DICTINST_HEADER_INCLUDED

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
  GwCompression compression;    //!< Path to the gziped dictionary file
  GwEncoding encoding;          //!< Path to the raw unziped dictionary file
  GwEngine engine;
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

#endif
