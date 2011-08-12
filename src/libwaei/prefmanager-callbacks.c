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
//! @file src/preferences.c
//!
//! @brief Abstraction layer for gsettings preferences
//!
//! Allows access to gsettings with the ability to specify backup preferences upon
//! failure to get the preference value.
//!


#include <string.h>
#include <locale.h>
#include <libintl.h>
#include <stdlib.h>

#include <gio/gio.h>

#include <libwaei/libwaei.h>


void lw_prefmanager_boolean_changed_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    GList *iter;
    LwPrefCallbackData* pcd;
    LwPrefManager *pm;
    gboolean state;

    pm = (LwPrefManager*) data;
    state = lw_prefmanager_get_boolean (settings, KEY);

    for (iter = pm->callbacklist; iter != NULL; iter = iter->next)
    {
      pcd = (LwPrefCallbackData*) iter->data;
      if (pcd != NULL)
      {
        (pcd->func) (pcd->data, &state, FALSE);
      }
    }
}


void lw_prefmanager_integer_changed_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    GList *iter;
    LwPrefCallbackData* pcd;
    LwPrefManager *pm;
    gint number;

    pm = (LwPrefManager*) data;
    number = lw_prefmanager_get_int (settings, KEY);

    for (iter = pm->callbacklist; iter != NULL; iter = iter->next)
    {
      pcd = (LwPrefCallbackData*) iter->data;
      if (pcd != NULL)
      {
        (pcd->func) (pcd->data, &number, FALSE);
      }
    }
}


void lw_prefmanager_string_changed_cb (GSettings *settings, gchar *KEY, gpointer data)
{
    GList *iter;
    LwPrefCallbackData* pcd;
    LwPrefManager *pm;
    char text[256];

    pm = (LwPrefManager*) data;
    lw_prefmanager_get_string (text, settings, KEY, 256);

    for (iter = pm->callbacklist; iter != NULL; iter = iter->next)
    {
      pcd = (LwPrefCallbackData*) iter->data;
      if (pcd != NULL)
      {
        (pcd->func) (pcd->data, text, FALSE);
      }
    }
}

