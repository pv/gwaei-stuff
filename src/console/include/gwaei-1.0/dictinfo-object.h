#ifndef GW_DICTINFO_OBJECT_INCLUDED
#define GW_DICTINFO_OBJECT_INCLUDED

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
//! @file src/include/gwaei/dictinfo-object.h
//!
//! @brief To be written.
//!
//! To be written.
//!

#include <gwaei/resultline-object.h>

//!
//! @brief Enumeration of dictionary statuses
//!
//! Statuses used for clueing thet states of dictionaries added to the
//! dictionary list.  This helps to set the GUI up correctly and generally
//! keep havoc from breaking out.
//!
typedef enum {
  GW_DICT_STATUS_INSTALLING,
  GW_DICT_STATUS_INSTALLED,
  GW_DICT_STATUS_NOT_INSTALLED,
  GW_DICT_STATUS_UPDATING,
  GW_DICT_STATUS_UPDATED,
  GW_DICT_STATUS_CANCELING,
  GW_DICT_STATUS_CANCELED,
  GW_DICT_STATUS_ERRORED,
  GW_DICT_STATUS_REBUILDING
} GwDictStatus;

//!
//! @brief Dictionary type assigned by the program.  It determines the parsing algorithm
//!
typedef enum {  
  GW_DICT_ENGINE_EDICT,         //!< Standard edict format dictionary
  GW_DICT_ENGINE_KANJI,         //!< Kanjidic format dictionary
  GW_DICT_ENGINE_EXAMPLES,      //!< Examples format dictionary
  GW_DICT_ENGINE_UNKNOWN,          //!< Unkown format which should use safe parsing
  GW_DICT_ENGINE_TOTAL
} GwDictEngine;

typedef enum {
  GW_DICT_COMPRESSION_ZIP,
  GW_DICT_COMPRESSION_GZIP,
  GW_DICT_COMPRESSION_NONE,
  GW_DICT_COMPRESSION_TOTAL
} GwDictCompression;

typedef enum {
  GW_DICT_ENCODING_UTF8,
  GW_DICT_ENCODING_EUC_JP,
  GW_DICT_ENCODING_SHIFT_JS,
  GW_DICT_ENCODING_TOTAL
} GwDictEncoding;


//!
//! @brief Primitive for storing dictionary information
//!
struct _GwDictInfo
{
    char *name;                       //!< name of the file in the .waei folder
    char *long_name;                  //!< long name of the file (usually localized)
    char *short_name;                 //!< short name of the file (usually localized)
    GwDictStatus status;              //!< install status of the dictionary
    GwDictEngine engine;              //!< Path to the dictionary file
    GwDictCompression compression;    //!< Path to the gziped dictionary file
    GwDictEncoding encoding;          //!< Path to the raw unziped dictionary file
    char *gskey;                      //!< gckey for the download source
    char *source_uri;                 //!< gckey for the download source
    int load_position;                //!< load position in the GUI
    long total_lines;                 //!< total lines in the file
    GwResultLine *cached_resultlines;
    GwResultLine *current_resultline;
    GMutex *mutex;
};
typedef struct _GwDictInfo GwDictInfo;


GwDictInfo* gw_dictinfo_new (GwDictEngine, char*);
void gw_dictinfo_free(GwDictInfo*);


#endif