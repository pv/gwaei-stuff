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
//! @brief Main entrance into the program.
//!
//! Main entrance into the program.
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

#include <gwaei/definitions.h>
#include <gwaei/regex.h>
#include <gwaei/utilities.h>
#include <gwaei/dictionaries.h>
#include <gwaei/history.h>
#include <gwaei/interface.h>



int main( int argc, char *argv[] )
{    
    gw_util_initialize_runmode(argv[0]);

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
    if (is_japanese_ctype() == FALSE)
    {
      //Try forcing a correct or better setting
      //This will have nice effects like antialiased text
      if ( setlocale(LC_CTYPE, "ja_JP.UTF8")  == NULL &&
           setlocale(LC_CTYPE, "ja_JP.utf8")  == NULL &&
           setlocale(LC_CTYPE, "ja_JP.UTF-8") == NULL &&
           setlocale(LC_CTYPE, "ja_JP.utf-8") == NULL &&
           setlocale(LC_CTYPE, "ja_JP")       == NULL &&
           setlocale(LC_CTYPE, "ja")          == NULL &&
           setlocale(LC_CTYPE, "japanese")    == NULL    )
        //All failed, go for the default
        setlocale(LC_CTYPE, "");

      //printf("Set CTYPE locale to %s\n", setlocale(LC_CTYPE, NULL));

      if ( setlocale(LC_COLLATE, "ja_JP.UTF8")  == NULL &&
           setlocale(LC_COLLATE, "ja_JP.utf8")  == NULL &&
           setlocale(LC_COLLATE, "ja_JP.UTF-8") == NULL &&
           setlocale(LC_COLLATE, "ja_JP.utf-8") == NULL &&
           setlocale(LC_COLLATE, "ja_JP")       == NULL &&
           setlocale(LC_COLLATE, "ja")          == NULL &&
           setlocale(LC_COLLATE, "japanese")    == NULL    )
        //All failed, go for the default
        setlocale(LC_COLLATE, "");

      //printf("Set COLLATE locale to %s\n", setlocale(LC_COLLATE, NULL));
    }

    if (!g_thread_supported ())
      g_thread_init (NULL);

    //g_type_init();
    curl_global_init(CURL_GLOBAL_ALL);

    gw_regex_initialize_constant_regular_expressions();
    gw_dictionaries_initialize_dictionary_list();
    gw_history_initialize_history();
    gw_io_check_for_rsync();

    if  (gw_util_get_runmode() == GW_CONSOLE_RUNMODE)
      initialize_console_interface(argc, argv);
    else
      initialize_gui_interface(&argc, &argv);

    exit(EXIT_SUCCESS);
}

