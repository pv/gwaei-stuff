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
//! @file src/common.c
//!
//! @brief Common functions used between many files
//!


#include <stdlib.h>
#include <string.h>

#include <gwaei/gwaei.h>


//!
//! @brief Optionally displays an error dialog and frees memory from an error
//!
//! @param error A pointer to an error pointer
//! @param show_dialog Whether to show a dialog or not.
//!
void gw_common_handle_error (GError **error, GtkWindow *parent, gboolean show_dialog)
{
    //Sanity checks
    if (error == NULL || *error == NULL) return;

    //Declarations
    GtkWidget *dialog;
    gint response;

    //Handle the error
    if (show_dialog)
    {
      dialog = gtk_message_dialog_new_with_markup (parent,
                                                   GTK_DIALOG_MODAL,
                                                   GTK_MESSAGE_ERROR,
                                                   GTK_BUTTONS_CLOSE,
                                                   "<b>%s</b>\n\n%s",
                                                   "An Error Occured",
                                                   (*error)->message
                                                  );
      g_signal_connect_swapped (dialog, "response", G_CALLBACK (gtk_widget_destroy), dialog);
      gtk_widget_show_all (GTK_WIDGET (dialog));
      response = gtk_dialog_run (GTK_DIALOG (dialog));
    }
    else
    {
      fprintf(stderr, "ERROR: %s\n", (*error)->message);
    }

    //Cleanup
    g_error_free (*error);
    *error = NULL;
}


