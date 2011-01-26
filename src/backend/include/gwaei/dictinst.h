

typedef enum {
//  GW_DICTINST_COMPRESSION_ZIP, //Unsupported since you can't tell what the file will be named
  GW_DICTINST_COMPRESSION_GZIP,
  GW_DICTINST_COMPRESSION_NONE,
  GW_DICTINST_COMPRESSION_TOTAL
} GwDictInstallerCompression;

typedef enum {
  GW_DICTINST_ENCODING_UTF8,
  GW_DICTINST_ENCODING_EUC_JP,
  GW_DICTINST_ENCODING_SHIFT_JS,
  GW_DICTINST_ENCODING_TOTAL
} GwDictInstallerEncoding;

typedef enum {
  GW_DICTINST_DOWNLOAD_SOURCE,
  GW_DICTINST_COMPRESSED_FILE,
  GW_DICTINST_TEXT_ENCODING,
  GW_DICTINST_FINAL_TARGET,
  GW_DICTINST_TOTAL_URI,
}


struct _GwDictInstaller {
  char *uri[GW_DICTINST_TOTAL_URI];
  int progress;
  char *status_message;
  GwDictInstallerCompression compression;    //!< Path to the gziped dictionary file
  GwDictInstallerEncoding encoding;          //!< Path to the raw unziped dictionary file
  GwDictEngine engine;
  gboolean split_dictionary;
  gboolean merge_dictionary;
  GMutex *mutex;
};
typedef struct _GwDictInstaller GwDicInstaller;

GwDictInstaller* gw_dictinstaller_new_using_pref_uri (const char*, 
                                                      const char*,
                                                      const char*,
                                                      const GwDictEngine,
                                                      const GwDictInstallerCompression,
                                                      const GwDictInstallerEncoding,
                                                      gboolean, gboolean);

GwDictInstaller* gw_dictinstaller_new (const char*,
                                       const char*,
                                       const GwDictEngine,
                                       const GwDictInstallerCompression,
                                       const GwDictInstallerEncoding,
                                       gboolean, gboolean);
gw_dictinstaller_free ();
