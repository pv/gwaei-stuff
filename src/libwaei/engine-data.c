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
//! @file engine-data.c
//!

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <libintl.h>

#include <glib.h>

#include <libwaei/libwaei.h>

#include <libwaei/engine-data.h>


LwEngineData* lw_enginedata_new (LwEngine *engine, LwSearchItem *item)
{
    LwEngineData *temp;

    temp = (LwEngineData*) malloc(sizeof(LwEngineData));

    if (temp != NULL)
    {
      temp->engine = engine;
      temp->item = item;
    }

    return temp;
}


void lw_enginedata_free (LwEngineData *data)
{
    if (data != NULL) free (data);
}

