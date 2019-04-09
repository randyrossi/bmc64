/** \file   uitapecreate.c
 * \brief   Gtk3 dialog to create and attach a new tape image
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
#include <string.h>

#include "basewidgets.h"
#include "basedialogs.h"
#include "debug_gtk3.h"
#include "widgethelpers.h"
#include "filechooserhelpers.h"
#include "util.h"
#include "lib.h"
#include "attach.h"
#include "diskimage.h"  /* for DISK_IMAGE_TYPE_TAP*/
#include "cbmimage.h"
#include "resources.h"
#include "tape.h"
#include "ui.h"

#include "uitapecreate.h"


/* forward declarations of functions */
static gboolean create_tape_image(const char *filename);


/** \brief  Reference to the 'auto-attach' check button
 */
static GtkWidget *auto_attach = NULL;


/** \brief  Handler for 'response' event of the dialog
 *
 * This handler is called when the user clicks a button in the dialog.
 *
 * \param[in]   widget      the dialog
 * \param[in]   response_id response ID
 * \param[in]   data        extra data (unused)
 */
static void on_response(GtkWidget *widget, gint response_id, gpointer data)
{
    gchar *filename;
    int status = TRUE;

    switch (response_id) {

        case GTK_RESPONSE_ACCEPT:
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
            if (filename != NULL) {
                /* create tape */
                status = create_tape_image(filename);
            }
            g_free(filename);
            if (status) {
                /* image creation and attaching was succesful, exit dialog */
                gtk_widget_destroy(widget);
            }
            break;

        case GTK_RESPONSE_REJECT:
            gtk_widget_destroy(widget);
            break;
        default:
            debug_gtk3("warning: unhandled response ID %d\n", response_id);
            break;
    }
}


/** \brief  Actually create the tape image and attach it
 *
 * \param[in]   filename    filename of the new image
 *
 * \return  bool
 */
static gboolean create_tape_image(const char *filename)
{
    gboolean status = TRUE;
    char *fname_copy;

    /* fix extension */
    fname_copy = util_add_extension_const(filename, "tap");

    /* try to create the image */
    if (cbmimage_create_image(fname_copy, DISK_IMAGE_TYPE_TAP) < 0) {
        vice_gtk3_message_error("VICE error",
                "Failed to create tape image '%s'", fname_copy);
        status = FALSE;
    } else if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(auto_attach))) {
        /* try to attach the image */
        if (tape_image_attach(1, fname_copy) < 0) {
            vice_gtk3_message_error("VICE error",
                    "Failed to attach tape image '%s'", fname_copy);
            status = FALSE;
        }
    }

    lib_free(fname_copy);
    return status;
}


/** \brief  Create the 'extra' widget for the dialog
 *
 * \return  GtkGrid
 */
static GtkWidget *create_extra_widget(void)
{
    GtkWidget *grid;

    /* create a grid with some spacing and margins */
    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);
    g_object_set(grid, "margin-left", 16, "margin-right", 16, NULL);

    auto_attach = gtk_check_button_new_with_label("Auto-attach tape image");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(auto_attach), TRUE);
    gtk_grid_attach(GTK_GRID(grid), auto_attach, 0, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create and show 'attach new tape image' dialog
 *
 * \param[in]   parent  parent widget (ignored)
 * \param[in]   data    extra data (ignored)
 *
 */
void uitapecreate_dialog_show(GtkWidget *parent, gpointer data)
{
    GtkWidget *dialog;
    GtkFileFilter *filter;

    dialog = gtk_file_chooser_dialog_new(
            "Create and attach a new tape image",
            ui_get_active_window(),
            GTK_FILE_CHOOSER_ACTION_SAVE,
            /* buttons */
            "Save", GTK_RESPONSE_ACCEPT,
            "Close", GTK_RESPONSE_REJECT,
            NULL, NULL);

    gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog),
            TRUE);

    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog),
            create_extra_widget());

    filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, "Tape images (*.tap)");
    gtk_file_filter_add_pattern(filter, "*.tap");
    gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

    g_signal_connect(dialog, "response", G_CALLBACK(on_response), NULL);

    gtk_widget_show(dialog);
}
