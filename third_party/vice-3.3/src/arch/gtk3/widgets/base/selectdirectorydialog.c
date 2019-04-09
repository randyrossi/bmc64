/** \file   selectdirectorydialog.c
 * \brief   GtkFileChooser wrapper to select/create a directory
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
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
 */

#include "vice.h"

#include <gtk/gtk.h>

#include "debug_gtk3.h"
#include "filechooserhelpers.h"
#include "ui.h"

#include "selectdirectorydialog.h"


/** \brief  Create a 'select directory' dialog
 *
 * \param[in]   title           dialog title
 * \param[in]   proposed        proposed directory name (optional)
 * \param[in]   allow_create    allow creating a new directory
 * \param[in]   path            set starting directory (optional)
 *
 * \return  dircetory name or `NULL` on cancel
 *
 * \note    the directory name returned is allocated by GLib and needs to be
 *          freed after use with g_free()
 */
gchar *vice_gtk3_select_directory_dialog(
        const char *title,
        const char *proposed,
        gboolean allow_create,
        const char *path)
{
    GtkWidget *dialog;
    gint result;
    gchar *filename;
    GtkFileFilter *filter;

    dialog = gtk_file_chooser_dialog_new(
            title,
            ui_get_active_window(),
            allow_create
                ? GTK_FILE_CHOOSER_ACTION_CREATE_FOLDER
                : GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
            "Select", GTK_RESPONSE_ACCEPT,
            "Cancel", GTK_RESPONSE_REJECT,
            NULL);

    /* set proposed file name, if any */
    if (proposed != NULL && *proposed != '\0') {
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(dialog), proposed);
    }

    /* change directory if specified */
    if (path != NULL && *path != '\0') {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), path);
    }

    /* create a custom filter for directories. without it all files will still
       be displayed, which can be irritating */
    filter = gtk_file_filter_new ();
    gtk_file_filter_add_mime_type (filter, "inode/directory");
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

    result = gtk_dialog_run(GTK_DIALOG(dialog));
    if (result == GTK_RESPONSE_ACCEPT) {
        filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    } else {
        filename = NULL;
    }
    gtk_widget_destroy(dialog);
    return filename;
}
