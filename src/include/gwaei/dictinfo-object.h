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

#ifndef GW_RESULTLINE_OBJECT_INCLUDED
#include <gwaei/resultline-object.h>
#endif

//!
//! @brief Enumeration of dictionary statuses
//!
//! Statuses used for clueing thet states of dictionaries added to the
//! dictionary list.  This helps to set the GUI up correctly and generally
//! keep havoc from breaking out.
//!
enum gw_dictionary_statuses {
  GW_DICT_STATUS_INSTALLING,
  GW_DICT_STATUS_INSTALLED,
  GW_DICT_STATUS_NOT_INSTALLED,
  GW_DICT_STATUS_UPDATING,
  GW_DICT_STATUS_UPDATED,
  GW_DICT_STATUS_CANCELING,
  GW_DICT_STATUS_CANCELED,
  GW_DICT_STATUS_ERRORED,
  GW_DICT_STATUS_REBUILDING
};

//!
//! @brief Enumeration of dictionary types
//!
//! These are used for simple identification of known dictionaries.  Other
//! is not used for known dictionaries, but a unique id number generated
//! dynamically using gw_dictlist_make_dictionary_id().  Other is used for
//! in the dictionary type classificaton which falls into GW_DICT_OTHER,
//! GW_DICT_RADICAL, and GW_DICT_KANJI.
//!
enum gw_dictionary_types {  
  GW_DICT_ENGLISH,
  GW_DICT_KANJI,
  GW_DICT_RADICALS,
  GW_DICT_NAMES,
  GW_DICT_PLACES,
  GW_DICT_MIX,
  GW_DICT_EXAMPLES,
  GW_DICT_FRENCH,
  GW_DICT_GERMAN,
  GW_DICT_SPANISH,
  GW_DICT_OTHER
};

//!
//! @brief Primitive for storing dictionary information
//!
struct GwDictInfo
{
    int id;                        //!< Unique dictionary id number
    int type;                      //!< classification of dictionary
    int status;                    //!< install status of the dictionary
    long total_lines;              //!< total lines in the file
    char name[100];                //!< name of the file in the .waei folder
    char long_name[100];           //!< long name of the file (usually localized)
    char path[FILENAME_MAX];       //!< Path to the dictionary file
    char gz_path[FILENAME_MAX];    //!< Path to the gziped dictionary file
    char sync_path[FILENAME_MAX];  //!< Path to the raw unziped dictionary file
    char rsync[FILENAME_MAX];      //!< rsync command in full with arguments for sync
    char gckey[100];               //!< gckey for the download source
    int load_position;             //!< load position in the GUI
    GwResultLine *cached_resultlines;
    GwResultLine *current_resultline;
};
typedef struct GwDictInfo GwDictInfo;


GwDictInfo* gw_dictinfo_new (char*);
