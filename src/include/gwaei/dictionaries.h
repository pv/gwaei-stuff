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
//! @file src/include/gwaei/dictionaries.h
//!
//! @brief To be written.
//!
//! To be written.
//!

#define INSTALLING    0
#define INSTALLED     1
#define NOT_INSTALLED 2
#define UPDATING      3
#define UPDATED       4
#define CANCELING     5
#define CANCELED      6
#define ERRORED       7
#define REBUILDING    8

#define OTHER    0
#define ENGLISH  1
#define KANJI    2
#define RADICALS 3
#define NAMES    4
#define PLACES   5
#define MIX      6


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
};
typedef struct GwDictInfo GwDictInfo;


//!
//! @brief Primitive for storing lists of dictionaries
//!
struct GwDictList
{
    GList *list;      //!< GList of the installed dictionaries
    GList *selected;  //!< Pointer to the currently selected dictionary in the GList
    int id_increment; //!< Unique id increment for the dictionary list object
};
typedef struct GwDictList GwDictList;


GList* gw_dictlist_get_list (void);
GwDictInfo* gw_dictlist_get_dictionary_by_id (int);
int gw_dictlist_get_total (void);
GList* gw_dictlist_get_selected(void);


GwDictInfo* gw_dictlist_get_dictionary_by_name (const char*);
/*
 *  Returns a dictionary in the GwDictList by name.  Unlike
 *  gw_dictlist_get_dictionary_by_alias, it will alway return what you ask
 *  for.  When the dictionary doesn't exist, it returns null.
 */


GwDictInfo* gw_dictlist_get_dictionary_by_alias(const char*);
/*
 *  Searches for a dictionary by a name in the GwDictList.  When approprate
 *  it will swap the requsted dictionary for another one.  This usally comes out
 *  to swapping the Kanji dictionary for the Mixed one when it is present.  When
 *  the dictionary is not found, it returns null.
 */
