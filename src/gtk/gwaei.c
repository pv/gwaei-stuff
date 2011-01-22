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
//! @file src/gwaei.c
//!
//! @brief Main entrance into the program.
//!
//! Main entrance into the program.
//!

#include <stdlib.h>

#include <gwaei/backend.h>
#include <gwaei/frontend.h>

int main (int argc, char *argv[])
{    
    gw_backend_initialize (argc, argv);
    gw_frontend_initialize (argc, argv);

    gw_frontend_start_gtk (argc, argv);

    gw_frontend_free ();
    gw_backend_free();

    return EXIT_SUCCESS;
}


