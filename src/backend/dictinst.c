#include <gwaei/dictinst-object.h>



static const char *_compression_type_to_string (const GwDictInstallerCompression COMPRESSION)
{
    switch (COMPRESSION)
    {
      case GW_DICT_COMPRESSION_ZIP:
        g_error ("currently unsupported compression type\n");
        return "zip";
      case GW_DICT_COMPRESSION_GZIP:
        return "gz";
      default:
        return "uncompressed";
    }
}

static const char *_encoding_type_to_string (const GwDictInstallerEncoding ENCODING)
{
    switch (ENCODING)
    {
      case GW_DICT_ENCODING_EUC_JP:
        return "euc-jp;
      case GW_DICT_ENCODING_SHIFT_JS:
        return "shift_js";
      case GW_DICT_ENCODING_UTF8:
        return "utf8";
      default:
        g_error ("Unsupported encoding\n");
    }
}


GwDictInstaller* gw_dictinstaller_new_using_pref_uri (const char name,
                                                      const char* schemaid
                                                      const char* key,
                                                      const GwDictEngine ENGINE,
                                                      const GwDictInstallerCompression COMPRESSION,
                                                      const GwDictInstallerEncoding ENCODING,
                                                      gboolean split, gboolean merge)
{
    char source_uri[200];
    gw_pref_get_string (source_uri, schemaid, key, 200);

    return gw_dictinstaller_new (name, source_uri, ENGINE, COMPRESSION, ENCODING, split, merge);
}



//!
//! @brief Creates a GwDictInstaller Object
//! 

GwDictInstaller* gw_dictinstaller_new (const char name,
                                       const char* source_uri,
                                       const GwDictEngine ENGINE,
                                       const GwDictInstallerCompression COMPRESSION,
                                       const GwDictInstallerEncoding ENCODING,
                                       gboolean split, gboolean merge)
{
    //Create the temp object to fill
    GwDictInstaller *temp = NULL;
    temp = (GwDictInstaller*) malloc (sizeof(GwDictInstaller));
    if (temp == NULL) g_error ("Out of memory. Could not create GwDictInstaller\n");

    //Initialize the variables for safety
    int i = 0;
    for (i = 0; i < GW_DICTINST_TOTAL_URI; i++)
      temp->uri[i] = NULL;
    temp->progress = 0;
    temp->status_message = NULL;
    temp->compression = -1;    //!< Path to the gziped dictionary file
    temp->encoding = -1;          //!< Path to the raw unziped dictionary file
    temp->engine = -1;
    temp->split_names_places_dictionary = FALSE;
    temp->merge_kanji_radicals_dictionary = FALSE;
    GMutex *temp->mutex = NULL;

    char *cache_filename = g_build_filename (gw_util_get_directory (GW_PATH_CACHE), name);
    char *engine_filename = g_build_filename (gw_util_get_directory_for_engine (ENGINE), name);
    const char *compression_ext = _compression_type_to_filename_suffix (COMPRESSION);
    const char *encoding_ext = _encoding_type_to_filename_suffix (ENCODING);

    temp->uri[GW_DICTINST_DOWNLOAD_SOURCE] = g_strdup (source_uri);
    temp->uri[GW_DICTINST_COMPRESSED_FILE] =  g_strjoin (".", cache_filename, compression_ext, NULL);
    temp->uri[GW_DICTINST_TEXT_ENCODING] =   g_strjoin (".", cache_filename, encoding_ext, NULL);
    temp->uri[GW_DICTINST_FINAL_TARGET] =  g_strdup (engine_filename);

    g_free (cache_filename);
    cache_filename = NULL;
    g_free (engine_filename);
    engine_filename = NULL;

    return temp;
}


//!
//! @brief Frees a GwDictInstaller object
//! 
void gw_dictinstaller_free (GwDictInstaller* di)
{
    int i = 0;
    for (i = 0; i < GW_DICTINST_TOTAL_URI; i++)
      free(temp->uri[i]);
    temp->progress = 0;
    free(temp->status_message);
    temp->compression = 0;    //!< Path to the gziped dictionary file
    temp->encoding = 0;          //!< Path to the raw unziped dictionary file
    temp->engine = 0;
    temp->split_names_places_dictionary = FALSE;
    temp->merge_kanji_radicals_dictionary = FALSE;
    g_mutex_free (temp->mutex);

    free (di);
}

