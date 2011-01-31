/******************************************************************************
    AUTHOR:
    File written and Copyrighted by Zachary Dovel. All Rights Reserved.

    LICENSE:
    This file is part of gWaei.

    gWaei is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    gWaei is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with gWaei.  If not, see <http://www.gnu.org/licenses/>.
*******************************************************************************/

//!
//!  @file src/dictinst.c
//!
//!  @brief Basic construct that hold data needed for installing a GwDictInfo item
//!


#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <libintl.h>

#include <glib.h>

#include <gwaei/backend.h>


static const char *_compression_type_to_string (const GwDictInstCompression COMPRESSION)
{
    switch (COMPRESSION)
    {
/*
      case GW_DICTINST_COMPRESSION_ZIP:
        g_error ("currently unsupported compression type\n");
        return "zip";
*/
      case GW_DICTINST_COMPRESSION_GZIP:
        return "gz";
      default:
        return "uncompressed";
    }
}

static const char *_encoding_type_to_string (const GwDictInstEncoding ENCODING)
{
    switch (ENCODING)
    {
      case GW_DICTINST_ENCODING_EUC_JP:
        return "euc-jp";
      case GW_DICTINST_ENCODING_SHIFT_JS:
        return "shift_js";
      case GW_DICTINST_ENCODING_UTF8:
        return "utf8";
      default:
        g_error ("Unsupported encoding\n");
    }
}


//!
//! @brief Updates the GwDictInst source uri when the pref changes
//!
static void _update_dictinst_source_uri_cb (GSettings *settings, char* key, gpointer data)
{
    GwDictInst *di = (GwDictInst*) data;
    char source_uri[200];

    gw_pref_get_string (source_uri, di->schemaid, di->key, 200);
    g_free (di->uri[GW_DICTINST_DOWNLOAD_SOURCE]);
    di->uri[GW_DICTINST_DOWNLOAD_SOURCE] = g_strdup (source_uri);
}


//!
//! @brief Creates a new GwDictInst object.  It also connects a listener to the preference so it updates automatically.
//!
GwDictInst* gw_dictinst_new_using_pref_uri (const char* filename,
                                            const char* shortname,
                                            const char* longname,
                                            const char* description,
                                            const char* schemaid,
                                            const char* key,
                                            const GwDictEngine ENGINE,
                                            const GwDictInstCompression COMPRESSION,
                                            const GwDictInstEncoding ENCODING,
                                            gboolean split, gboolean merge, gboolean builtin)
{
    char source_uri[200];
    gw_pref_get_string (source_uri, schemaid, key, 200);
    GwDictInst *di = NULL;

    di = gw_dictinst_new (
      filename,
      shortname, 
      longname, 
      description, 
      source_uri, 
      ENGINE, 
      COMPRESSION, 
      ENCODING, 
      split, 
      merge,
      builtin
    );

    di->schemaid = g_strdup (schemaid);
    di->key = g_strdup (key);
    if (di->listenerid_is_set == TRUE)
      gw_pref_remove_change_listener (schemaid, di->listenerid);
    di->listenerid = gw_pref_add_change_listener (schemaid, key, _update_dictinst_source_uri_cb, di);

    return di;
}



//!
//! @brief Creates a GwDictInst Object
//! 

GwDictInst* gw_dictinst_new (const char* filename,
                             const char* shortname,
                             const char* longname,
                             const char* description,
                             const char* source_uri,
                             const GwDictEngine ENGINE,
                             const GwDictInstCompression COMPRESSION,
                             const GwDictInstEncoding ENCODING,
                             gboolean split, gboolean merge, gboolean builtin)
{
    //Create the temp object to fill
    GwDictInst *temp = NULL;
    temp = (GwDictInst*) malloc (sizeof(GwDictInst));
    if (temp == NULL) g_error ("Out of memory. Could not create GwDictInst\n");

    //Initialize the variables for safety
    temp->filename = NULL;
    temp->shortname = NULL;
    temp->longname = NULL;
    temp->description = NULL;
    int i = 0;
    for (i = 0; i < GW_DICTINST_TOTAL_URI; i++)
      temp->uri[i] = NULL;
    temp->schemaid = NULL;
    temp->key = NULL;
    temp->progress = 0;
    temp->status_message = NULL;
    temp->listenerid = 0;
    temp->listenerid_is_set = FALSE;
    temp->compression = COMPRESSION;    //!< Path to the gziped dictionary file
    temp->encoding = ENCODING;          //!< Path to the raw unziped dictionary file
    temp->engine = ENGINE;
    temp->builtin = builtin;
    temp->split_dictionary = split;
    temp->merge_dictionary = merge;
    temp->mutex = g_mutex_new ();

    //Set the values
    temp->filename = g_strdup (filename);
    temp->shortname = g_strdup (shortname);
    temp->longname = g_strdup (longname);
    temp->description = g_strdup (description);

    char *cache_filename = g_build_filename (gw_util_get_directory (GW_PATH_CACHE), filename, NULL);
    char *engine_filename = g_build_filename (gw_util_get_directory_for_engine (ENGINE), filename, NULL);
    const char *compression_ext = _compression_type_to_string (COMPRESSION);
    const char *encoding_ext = _encoding_type_to_string (ENCODING);

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
//! @brief Frees a GwDictInst object
//! 
void gw_dictinst_free (GwDictInst* di)
{
    if (di->listenerid_is_set == TRUE)
      gw_pref_remove_change_listener (di->schemaid, di->listenerid);

    g_free(di->filename);
    g_free(di->shortname);
    g_free(di->longname);
    g_free(di->description);

    int i = 0;
    while (i < GW_DICTINST_TOTAL_URI)
    {
      g_free(di->uri[i]);
      i++;
    }
    di->progress = 0;
    g_free(di->status_message);
    g_free (di->schemaid);
    g_free (di->key);
    di->compression = 0;    //!< Path to the gziped dictionary file
    di->encoding = 0;          //!< Path to the raw unziped dictionary file
    di->engine = 0;
    di->split_dictionary = FALSE;
    di->merge_dictionary = FALSE;
    g_mutex_free (di->mutex);
    free (di);
}


