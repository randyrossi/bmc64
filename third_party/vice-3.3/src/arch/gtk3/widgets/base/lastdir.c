/** \file   lastdir.c
 * \brief   Last directory used wrapper code
 *
 * Simple code to handle last used directories for file dialogs.
 *
 * Since Gtk3 refuses to remember the last used directory for a file dialog
 * (or even presents a shitty 'last used files' list), we need something to
 * remember the last used directory on a per-dialog basis.
 *
 * This code will handle that, with a very little extra code. Currently this
 * only works with GtkFileChooser-derived widgets, but that's basically all we
 * use in Gtk3-VICE at the moment.
 *
 * The basic API is this:
 *
 * In the code using this API, declare a static `char *` variable to hold the
 * last used directory.
 *
 * For example:
 * \code{.c}
 *  static *last_dir = NULL;
 * \endcode
 *
 * In the dialog building code, add a call to lastdir_set():
 * \code{.c}
 *  lastdir_set(dialog, &last_dir);
 * \endcode
 *
 * Whenever the last directory needs to be updated, for example when the user
 * has selected a file, add a call to lastdir_update():
 * \code{.c}
 *  lastdir_update(dialog, &last_dir);
 * \endcode
 *
 * And finally when shutting down the dialog/UI, don't forget to call
 * \code{.c}
 * lastdir_shutdown(&last_dir);
 * \endcode
 * to clean up the resources used.
 *
 * Maybe just look at src/arch/gtk3/uidiskattach.c to get an idea.
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 */

/*
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <stdio.h>
#include <gtk/gtk.h>

#include "debug_gtk3.h"

#include "lastdir.h"


/** \brief  Set directory of the GtkFileChooser \a widget using *\a last
 *
 * \param[in,out]   widget  GtkFileChooser widget
 * \param[in]       last    pointer to last directory string pointer
 */
void lastdir_set(GtkWidget *widget, char **last)
{
    if (last != NULL && *last != NULL) {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(widget), *last);
    }
}


/** \brief  Update the last used directory
 *
 * Updates *\a last to the directory currently used by \a widget
 *
 * \param[in]   widget  GtkFileChooser widget
 * \param[out]  last    pointer to string pointer containing the last used ir
 */
void lastdir_update(GtkWidget *widget, char **last)
{
    gchar *new_dir;

    new_dir = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(widget));
    debug_gtk3("new dir = '%s'.", new_dir);
    if (new_dir != NULL) {
        /* clean up previous value */
        if (*last != NULL) {
            g_free(*last);
        }
        *last = new_dir;
    }
}


/** \brief  Free memory used by *\a last
 *
 * Clean up memory used by last used directory string, if any.
 *
 * \param[in,out]   last    pointer to string pointer to free
 */
void lastdir_shutdown(char **last)
{
    if (last != NULL && *last != NULL) {
        g_free(*last);
        *last = NULL;
    }
}
