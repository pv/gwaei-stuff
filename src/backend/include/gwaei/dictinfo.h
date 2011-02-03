#ifndef GW_DICTINFO_HEADER_INCLUDED
#define GW_DICTINFO_HEADER_INCLUDED

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
//! @file src/include/gwaei/dictinfo.h
//!
//! @brief To be written.
//!
//! To be written.
//!

#include <gwaei/resultline.h>


//!
//! @brief Primitive for storing dictionary information
//!
struct _GwDictInfo
{
    char *filename;                   //!< name of the file in the gwaei user data folder
    char *longname;                   //!< long name of the file (usually localized)
    char *shortname;                  //!< short name of the file (usually localized)
    GwEngine engine;                  //!< Path to the dictionary file
    int load_position;                //!< load position in the GUI
    long total_lines;                 //!< total lines in the file
    GwResultLine *cached_resultlines; //!< Allocated resultline swapped with current_resultline when needed
    GwResultLine *current_resultline; //!< Allocated resultline where the current parsed result data resides
};
typedef struct _GwDictInfo GwDictInfo;


GwDictInfo* gw_dictinfo_new (const GwEngine, const char*);
void gw_dictinfo_free(GwDictInfo*);


#endif
