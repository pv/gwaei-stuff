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
//! @file src/main.c
//!
//! @brief Main entrance into the backend for initializing it and freeing it
//!

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>
#include <stdio.h>
#include <locale.h>
#include <libintl.h>

#include <curl/curl.h>
#include <glib.h>
#include <glib-object.h>

#include <gwaei/backend.h>


void gw_backend_initialize (int argc, char* argv[])
{
    //Setup for localized messages
    setlocale(LC_MESSAGES, "");
    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (PACKAGE, "UTF-8");
    textdomain(PACKAGE);

    //Set the default CTYPE local
    setlocale(LC_CTYPE, "");
    setlocale(LC_COLLATE, "");
    setlocale(LC_MESSAGES, "");
    //Check if it's the correct locale
    if (gw_util_is_japanese_ctype() == FALSE)
      gw_util_force_japanese_locale();

    if (!g_thread_supported ())
      g_thread_init (NULL);

    g_type_init ();
    curl_global_init (CURL_GLOBAL_ALL);

    gw_regex_initialize ();
    gw_dictlist_initialize ();
    gw_historylist_initialize ();

    g_thread_init(NULL);
}


void gw_backend_free ()
{
    gw_dictlist_free ();
    gw_regex_free ();
}