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
#include <stdlib.h>
#include <stdio.h>
#include <libintl.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <gwaei/backend.h>

typedef void _InterfaceUpdateCallback(GwDictInst*, const char*);


//!
//! @brief Updates the GwDictInst source uri when the pref changes
//! @param settting A GSetting object
//! @param key The key of the pref
//! @param data User data passed to the preference listener
//!
static void _update_dictinst_source_uri_cb (GSettings *settings, char* key, gpointer data)
{
    GwDictInst *di = (GwDictInst*) data;
    char source_uri[200];

    gw_pref_get_string_by_schema (source_uri, di->schema, di->key, 200);
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
                                            const char* schema,
                                            const char* key,
                                            const GwEngine ENGINE,
                                            const GwCompression COMPRESSION,
                                            const GwEncoding ENCODING,
                                            gboolean split, gboolean merge, gboolean builtin)
{
    char source_uri[200];
    gw_pref_get_string_by_schema (source_uri, schema, key, 200);
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

    di->schema = g_strdup (schema);
    di->key = g_strdup (key);
    if (di->listenerid_is_set == TRUE)
      gw_pref_remove_change_listener_by_schema (schema, di->listenerid);
    di->listenerid = gw_pref_add_change_listener_by_schema (schema, key, _update_dictinst_source_uri_cb, di);

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
                             const GwEngine ENGINE,
                             const GwCompression COMPRESSION,
                             const GwEncoding ENCODING,
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
    temp->schema = NULL;
    temp->key = NULL;
    temp->progress = 0;
    temp->selected = FALSE;
    temp->status_message = NULL;
    temp->listenerid = 0;
    temp->listenerid_is_set = FALSE;
    temp->compression = COMPRESSION;    //!< Path to the gziped dictionary file
    temp->encoding = ENCODING;          //!< Path to the raw unziped dictionary file
    temp->engine = ENGINE;
    temp->builtin = builtin;
    temp->split = split;
    temp->merge = merge;
    temp->mutex = g_mutex_new ();

    //Set the values
    temp->filename = NULL;
    temp->shortname = g_strdup (shortname);
    temp->longname = g_strdup (longname);
    temp->description = g_strdup (description);
    
    gw_dictinst_set_filename (temp, filename);
    gw_dictinst_set_download_source (temp, source_uri);


    return temp;
}


//!
//! @brief Frees a GwDictInst object
//! 
void gw_dictinst_free (GwDictInst* di)
{
    if (di->listenerid_is_set == TRUE)
      gw_pref_remove_change_listener_by_schema (di->schema, di->listenerid);

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
    g_free (di->schema);
    g_free (di->key);
    di->compression = 0;    //!< Path to the gziped dictionary file
    di->encoding = 0;          //!< Path to the raw unziped dictionary file
    di->engine = 0;
    di->split = FALSE;
    di->merge = FALSE;
    g_mutex_free (di->mutex);
    free (di);
}


//!
//! @brief Updates the filename save targets of the GwDictInst.
//! @param di The GwDictInst object to set the filename to
//! @param filename to copy to the GwDictInst object and use to generate uris
//!
void gw_dictinst_set_filename (GwDictInst *di, const char *filename)
{
    g_free (di->filename);
    di->filename = g_strdup (filename);

    gw_dictinst_regenerate_save_target_uris (di);
}


//!
//! @brief Updates the engine of the GwDictInst
//! @param The GwDictInst object to set the ENGINE to
//! @param ENGINE the engine that you want to set
//!
void gw_dictinst_set_engine (GwDictInst *di, const GwEngine ENGINE)
{
    di->engine = ENGINE;

    gw_dictinst_regenerate_save_target_uris (di);
}


//!
//! @brief Updates the encoding of the GwDictInst
//! @param di The GwDictInfo object to set the ENCODING to
//! @param ENCODING Tells the GwDictInfo object what the initial character encoding of the downloaded file will be
//!
void gw_dictinst_set_encoding (GwDictInst *di, const GwEncoding ENCODING)
{
    di->encoding = ENCODING;

    gw_dictinst_regenerate_save_target_uris (di);
}


//!
//! @brief Updates the compression of the GwDictInst
//! @param di The GwDictInfo objcet to set the COMPRESSION variable on
//! @param COMPRESSION Tells the GwDictInfo object what kind of compression the downloaded dictionary file will have.
//!
void gw_dictinst_set_compression (GwDictInst *di, const GwCompression COMPRESSION)
{
    di->compression = COMPRESSION;

    gw_dictinst_regenerate_save_target_uris (di);
}




//!
//! @brief Updates the download source of the GwDictInst object
//!
void gw_dictinst_set_download_source (GwDictInst *di, const char *SOURCE)
{
    g_free (di->uri[GW_DICTINST_DOWNLOAD_SOURCE]);
    di->uri[GW_DICTINST_DOWNLOAD_SOURCE] = g_strdup (SOURCE);
}


//!
//! @brief Updates the merge state of the GwDictInst
//!
void gw_dictinst_set_merge (GwDictInst *di, const gboolean MERGE)
{
    di->merge = MERGE;

    gw_dictinst_regenerate_save_target_uris (di);
}


//!
//! @brief Updates the split state of the GwDictInst
//!
void gw_dictinst_set_split (GwDictInst *di, const gboolean SPLIT)
{
    di->split = SPLIT;

    gw_dictinst_regenerate_save_target_uris (di);
}


//!
//! @brief This method should be called after the filename, engine, compression,
//!        or encoding members of the GwDictInst is changed to sync the new paths
//!
void gw_dictinst_regenerate_save_target_uris (GwDictInst *di)
{
    //Sanity check
    g_assert (di != NULL && di->filename != NULL);

    //Declarations
    char *cache_filename;
    char *engine_filename;
    const char *compression_ext;
    const char *encoding_ext;

    //Remove the previous contents
    g_free (di->uri[GW_DICTINST_COMPRESSED_FILE]);
    g_free (di->uri[GW_DICTINST_TEXT_ENCODING]);
    g_free (di->uri[GW_DICTINST_FINAL_TARGET]);

    //Initializations
    cache_filename = g_build_filename (gw_util_get_directory (GW_PATH_CACHE), di->filename, NULL);
    engine_filename = g_build_filename (gw_util_get_directory_for_engine (di->engine), di->filename, NULL);
    compression_ext = gw_util_get_compression_name (di->compression);
    encoding_ext = gw_util_get_encoding_name (di->encoding);

    di->uri[GW_DICTINST_COMPRESSED_FILE] =  g_strjoin (".", cache_filename, compression_ext, NULL);
    di->uri[GW_DICTINST_TEXT_ENCODING] =   g_strjoin (".", cache_filename, encoding_ext, NULL);
    di->uri[GW_DICTINST_FINAL_TARGET] =  g_strdup (engine_filename);

    //Cleanup
    g_free (cache_filename);
    g_free (engine_filename);
}


//!
//! @brief Tells the installer mechanism if it is going to fail if it tries installing because of missing info
//!
gboolean gw_dictinst_data_is_valid (GwDictInst *di)
{
    if (!di->selected) return TRUE;

    //Declarations
    char *ptr;
    char **temp_string_array;
    int total_download_arguments;

    ptr = di->filename;
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    ptr = di->uri[GW_DICTINST_DOWNLOAD_SOURCE];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    //Make sure the correct number of download arguments are available
    temp_string_array = g_strsplit (ptr, ";", -1);
    total_download_arguments = g_strv_length (temp_string_array);
    g_strfreev (temp_string_array);

    if (di->merge && total_download_arguments != 2) return FALSE;
    if (!di->merge && total_download_arguments != 1) return FALSE;

    ptr = di->uri[GW_DICTINST_COMPRESSED_FILE];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    ptr = di->uri[GW_DICTINST_TEXT_ENCODING];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    ptr = di->uri[GW_DICTINST_FINAL_TARGET];
    if (ptr == NULL || strlen (ptr) == 0) return FALSE;

    if (di->engine < 0 || di->engine >= GW_ENGINE_TOTAL) return FALSE;
    if (di->compression < 0 || di->compression >= GW_COMPRESSION_TOTAL) return FALSE;
    if (di->encoding < 0 || di->encoding >= GW_ENCODING_TOTAL) return FALSE;

    return TRUE;
}


//!
//! @brief Downloads or copies the file to the dictionary directory to be worked on
//!        This function should normally only be used in the gw_dictinst_install function.
//! @param path String representing the path of the file to gunzip.
//! @param error Error handling
//! @see gw_dictinst_download
//! @see gw_dictinst_convert_encoding
//! @see gw_dictinst_postprocess
//! @see gw_dictinst_install
//! @see gw_dictinst_uninstall
//!
gboolean gw_dictinst_download (GwDictInst *di, _InterfaceUpdateCallback *cb, GError **error)
{
    //Sanity check
    g_assert (di != NULL);

    //Declarations
    char *source;
    char *target;

    //Initializations
    source = di->uri[GW_DICTINST_DOWNLOAD_SOURCE];
    target = di->uri[GW_DICTINST_COMPRESSED_FILE];

    //Copy the file if it is a local file
    if (*error == NULL)
    {
      //File is located locally
      if (g_file_test (source, G_FILE_TEST_IS_REGULAR))
      {
        gw_io_copy_file (source, target, error);
      }
      //File is online
      else
      {
//        gw_io_download_file (source, target, cb, NULL, error);
      }
    }

    return (*error == NULL);
}


//!
//! @brief Detects the compression scheme of a file and decompresses it using the approprate function.
//!        This function should normally only be used in the gw_dictinst_install function.
//! @param path String representing the path of the file to gunzip.
//! @param error Error handling
//! @see gw_dictinst_download
//! @see gw_dictinst_convert_encoding
//! @see gw_dictinst_postprocess
//! @see gw_dictinst_install
//! @see gw_dictinst_uninstall
//!
gboolean gw_dictinst_decompress (GwDictInst *di, _InterfaceUpdateCallback *cb, GError **error)
{
/*
    //Sanity check
    g_assert (g_file_test (uri, G_FILE_TEST_IS_REGULAR));

    //Preform the correct decompression
    switch (di->compression)
    {
      case GW_COMPRESSION_GZIP:
        return gw_io_gunzip_file (path, error);
      case GW_COMPRESSION_ZIP:
        return gw_io_unzip_file (path, error);
      case GW_COMPRESSION_NONE:
        return  g_rename (di->uri[GW_DOWNLOAD], di->uri[GW_PATH]);
    }

    //Not reachable
    g_signal_unreachable ();
*/
}


//!
//! @brief Converts the encoding to UTF8 for the file
//!        This function should normally only be used in the gw_dictinst_install function.
//! @param path String representing the path of the file to gunzip.
//! @param error Error handling
//! @see gw_dictinst_download
//! @see gw_dictinst_convert_encoding
//! @see gw_dictinst_postprocess
//! @see gw_dictinst_install
//! @see gw_dictinst_uninstall
//!

//!
gboolean gw_dictinst_convert_encoding (GwDictInst *di, _InterfaceUpdateCallback *cb, GError **error)
{
    if (error != NULL) return FALSE;

    //Declarations
    char *source_path;
    char *target_path;
    const char *encoding_name;

    //Initializations
    source_path = di->uri[GW_DICTINST_TEXT_ENCODING];
    target_path = di->uri[GW_DICTINST_FINAL_TARGET];
    encoding_name = gw_util_get_encoding_name (di->encoding);

    if (cb != NULL) cb (di, gettext("Converting encoding..."));

    if (di->encoding == GW_ENCODING_UTF8)
      gw_io_copy_file (source_path, target_path, error);
    else
      gw_io_copy_with_encoding (source_path, target_path, encoding_name,"UTF-8", error);


    return (*error == NULL);
}


//!
//! @brief does the required postprocessing on a dictionary
//!        This function should normally only be used in the gw_dictinst_install function.
//! @param path String representing the path of the file to gunzip.
//! @param error Error handling
//! @see gw_dictinst_download
//! @see gw_dictinst_convert_encoding
//! @see gw_dictinst_postprocess
//! @see gw_dictinst_install
//! @see gw_dictinst_uninstall
//!
gboolean gw_dictinst_finalize (GwDictInst *di, _InterfaceUpdateCallback *cb, GError **error)
{
    if (*error != NULL) return FALSE;

    //Declarations
    gchar **source_paths;
    gchar **target_paths;
    char *buffer;
    char *ptr;
    char *message;
    GQuark quark;
    gboolean file_copy_failed;

    //Initializations
    source_paths = NULL;
    target_paths = NULL;
    file_copy_failed = FALSE;
    quark = g_quark_from_string (GW_DICTINST_ERROR);
    source_paths = g_strsplit (di->uri[GW_DICTINST_TEXT_ENCODING], ";", 1);
    target_paths = g_strsplit (di->uri[GW_DICTINST_FINAL_TARGET], ";", 1);

    if (cb != NULL) cb (di, gettext("Postprocessing..."));

    //Rebuild the mix dictionary
    if (di->merge)
    {
      if (g_strv_length (source_paths) != 2)
      {
        message = gettext("The incorrect number of source paths were specified for "
                          "merging the Kanji and Radical dictionaries.");
        *error = g_error_new (quark, GW_DICTINST_ERROR_SOURCE_PATH, message);
      }
      else 
      {
        gw_io_create_mix_dictionary (target_paths[0], source_paths[0], source_paths[1], error);
      }
    }

    //Rebuild the names dictionary
    else if(di->split)
    {
      if (g_strv_length (source_paths) != 1)
      {
        message = gettext("The incorrect number of source paths were specified for "
                          "splitting the places from the Names dictionary.");
        *error = g_error_new (quark, GW_DICTINST_ERROR_SOURCE_PATH, message);
      }
      else
      {
        gw_io_split_places_from_names_dictionary (target_paths[0], target_paths[1], source_paths[0], error);
      }
    }

    //Just copy the file no postprocessing required
    else
    {
      if (g_rename (source_paths[0], target_paths[0]) != 0) file_copy_failed = TRUE;
    }

    //Cleanup
    g_strfreev (source_paths);
    g_strfreev (target_paths);

    //Check if all of the files were sucessfully copied after everything
    if (file_copy_failed)
    {
      message = gettext("Dictionary failed to copy into the dictionaries "
                        "folder. Another program may be using the file.");
      *error = g_error_new (quark, GW_DICTINST_ERROR_FILE_MOVE, message);
    }

    //Finish
    return (*error == NULL);
}


//!
//! @brief removes temporary files created by installation in the dictionary cache folder
//!
void gw_dictinst_clean (GwDictInst *di, _InterfaceUpdateCallback *cb)
{
}



//!
//! @brief Installs a GwDictInst object using the provided gui update callback
//!        This function should normally only be used in the gw_dictinst_install function.
//! @param path String representing the path of the file to gunzip.
//! @param error Error handling
//! @see gw_dictinst_download
//! @see gw_dictinst_convert_encoding
//! @see gw_dictinst_postprocess
//! @see gw_dictinst_install
//! @see gw_dictinst_uninstall
//!
gboolean gw_dictinst_install (GwDictInst *di, _InterfaceUpdateCallback *cb, GError **error)
{
    g_assert (*error != NULL && di != NULL);

    if (*error == NULL) gw_dictinst_download (di, cb, error);
    if (*error == NULL) gw_dictinst_decompress (di, cb, error);
    if (*error == NULL) gw_dictinst_convert_encoding (di, cb, error);
    if (*error == NULL) gw_dictinst_finalize (di, cb, error);
    gw_dictinst_clean (di, cb);

    return (*error == NULL);
}



