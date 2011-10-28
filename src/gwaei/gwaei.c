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
//! @file gwaei.c
//!
//! @brief To be written
//!

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <gio/gio.h>

#include <gtk/gtk.h>

#include <gwaei/gwaei.h>


int main (int argc, char *argv[])
{    
    GApplication *application;
    int resolution;

    setlocale(LC_MESSAGES, "");
    setlocale(LC_CTYPE, "");
    setlocale(LC_COLLATE, "");

    bindtextdomain(PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (PACKAGE, "UTF-8");
    textdomain(PACKAGE);

    g_type_init ();

    g_thread_init (NULL);
    gdk_threads_init ();

    lw_regex_initialize ();

    application = gw_application_new ("gtk.org.gWaei", G_APPLICATION_HANDLES_COMMAND_LINE);

gdk_threads_enter ();
    resolution = g_application_run (application, argc, argv);
gdk_threads_leave ();

    g_object_unref (G_OBJECT (application));

    lw_regex_free ();

    return resolution;
}


