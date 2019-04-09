/** \file   uitapeattach.c
 * \brief   GTK3 tape-attach dialog
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 * \author  Michael C. Martin <mcmartin@gmail.com>
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

#include "attach.h"
#include "autostart.h"
#include "tape.h"
#include "debug_gtk3.h"
#include "basedialogs.h"
#include "contentpreviewwidget.h"
#include "filechooserhelpers.h"
#include "imagecontents.h"
#include "lastdir.h"
#include "tapecontents.h"
#include "ui.h"
#include "uimachinewindow.h"

#include "uitapeattach.h"


/** \brief  File type filters for the dialog
 */
static ui_file_filter_t filters[] = {
    { "Tape images", file_chooser_pattern_tape },
    { "All files", file_chooser_pattern_all },
    { NULL, NULL }
};

static GtkWidget *preview_widget = NULL;


/** \brief  Last directory used
 *
 * When a taoe is attached, this is set to the directory of that file. Since
 * it's heap-allocated by Gtk3, it must be freed with a call to
 * ui_tape_attach_shutdown() on emulator shutdown.
 */
static gchar *last_dir = NULL;


/** \brief  Handler for the "update-preview" event
 *
 * \param[in]   chooser file chooser dialog
 * \param[in]   data    extra event data (unused)
 */
static void on_update_preview(GtkFileChooser *chooser, gpointer data)
{
    GFile *file;
    gchar *path;

    file = gtk_file_chooser_get_preview_file(chooser);
    if (file != NULL) {
        path = g_file_get_path(file);
        if (path != NULL) {
            debug_gtk3("called with '%s'.", path);

            content_preview_widget_set_image(preview_widget, path);
           g_free(path);
        }
        g_object_unref(file);
    }
}


/** \brief  Handler for the 'toggled' event of the 'show hidden files' checkbox
 *
 * \param[in]   widget      checkbox triggering the event
 * \param[in]   user_data   data for the event (the dialog)
 */
static void on_hidden_toggled(GtkWidget *widget, gpointer user_data)
{
    int state;

    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    debug_gtk3("show hidden files: %s.", state ? "enabled" : "disabled");

    gtk_file_chooser_set_show_hidden(GTK_FILE_CHOOSER(user_data), state);
}


/** \brief  Handler for 'response' event of the dialog
 *
 * This handler is called when the user clicks a button in the dialog.
 *
 * \param[in]   widget      the dialog
 * \param[in]   response_id response ID
 * \param[in]   user_data   extra data (unused)
 *
 * TODO:    proper (error) messages, which requires implementing ui_error() and
 *          ui_message() and moving them into gtk3/widgets to avoid circular
 *          references
 */
static void on_response(GtkWidget *widget, gint response_id,
                        gpointer user_data)
{
    gchar *filename;
    int index;

    index = GPOINTER_TO_INT(user_data);

    debug_gtk3("got response ID %d, index %d.", response_id, index);

    switch (response_id) {

        /* 'Open' button, double-click on file */
        case GTK_RESPONSE_ACCEPT:
            lastdir_update(widget, &last_dir);
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
            /* ui_message("Opening file '%s' ...", filename); */
            debug_gtk3("Attaching file '%s' to tape unit.", filename);

            /* copied from Gtk2: I fail to see how brute-forcing your way
             * through file types is 'smart', but hell, it works */
            if (tape_image_attach(1, filename) < 0) {
                /* failed */
                debug_gtk3("tape attach failed.");
            }
            g_free(filename);
            gtk_widget_destroy(widget);
            break;

        /* 'Autostart' button clicked */
        case VICE_RESPONSE_AUTOSTART:
            lastdir_update(widget, &last_dir);
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
            debug_gtk3("Autostarting file '%s'.", filename);
            if (autostart_tape(
                        filename,
                        NULL,   /* program name */
                        index,
                        AUTOSTART_MODE_RUN) < 0) {
                /* oeps */
                debug_gtk3("autostart tape attach failed.");
            }
            g_free(filename);
            gtk_widget_destroy(widget);
            break;

        /* 'Close'/'X' button */
        case GTK_RESPONSE_REJECT:
            gtk_widget_destroy(widget);
            break;
        default:
            break;
    }

    ui_set_ignore_mouse_hide(FALSE);
}


/** \brief  Create the 'extra' widget
 *
 * \return  GtkGrid
 *
 * TODO: 'grey-out'/disable units without a proper drive attached
 */
static GtkWidget *create_extra_widget(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *hidden_check;
    GtkWidget *readonly_check;
#if 0
    GtkWidget *preview_check;
#endif

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    hidden_check = gtk_check_button_new_with_label("Show hidden files");
    g_signal_connect(hidden_check, "toggled", G_CALLBACK(on_hidden_toggled),
            (gpointer)(parent));
    gtk_grid_attach(GTK_GRID(grid), hidden_check, 0, 0, 1, 1);

    readonly_check = gtk_check_button_new_with_label("Attach read-only");
    gtk_grid_attach(GTK_GRID(grid), readonly_check, 1, 0, 1, 1);
#if 0
    preview_check = gtk_check_button_new_with_label("Show image contents");
    g_signal_connect(preview_check, "toggled", G_CALLBACK(on_preview_toggled),
            NULL);
    gtk_grid_attach(GTK_GRID(grid), preview_check, 2, 0, 1, 1);
#endif

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create the tape attach dialog
 *
 * \param[in]   parent  parent widget, used to get the top level window
 *
 * \return  GtkFileChooserDialog
 */
static GtkWidget *create_tape_attach_dialog(GtkWidget *parent)
{
    GtkWidget *dialog;
    size_t i;

    ui_set_ignore_mouse_hide(TRUE);

    /* create new dialog */
    dialog = gtk_file_chooser_dialog_new(
            "Attach a tape image",
            ui_get_active_window(),
            GTK_FILE_CHOOSER_ACTION_OPEN,
            /* buttons */
            "Open", GTK_RESPONSE_ACCEPT,
            "Autostart", VICE_RESPONSE_AUTOSTART,
            "Close", GTK_RESPONSE_REJECT,
            NULL, NULL);

    /* set modal so mouse-grab doesn't get triggered */
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

    /* set last directory */
    lastdir_set(dialog, &last_dir);

    /* add 'extra' widget: 'readonly' and 'show preview' checkboxes */
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog),
                                      create_extra_widget(dialog));

    preview_widget = content_preview_widget_create(dialog, tapecontents_read,
            on_response);
    gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog),
            preview_widget);

    /* add filters */
    for (i = 0; filters[i].name != NULL; i++) {
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),
                create_file_chooser_filter(filters[i], FALSE));
    }

    /* connect "reponse" handler: the `user_data` argument gets filled in when
     * the "response" signal is emitted: a response ID */
    g_signal_connect(dialog, "response", G_CALLBACK(on_response), NULL);
    g_signal_connect(dialog, "update-preview", G_CALLBACK(on_update_preview),
            NULL);

    return dialog;

}


/** \brief  Callback for the "attach tape image" menu items
 *
 * Creates the dialog and runs it.
 *
 * \param[in]   widget      menu item triggering the callback
 * \param[in]   user_data   ignored
 */
void ui_tape_attach_callback(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *dialog;

    debug_gtk3("called.");
    dialog = create_tape_attach_dialog(widget);
    gtk_widget_show(dialog);

}


/** \brief  Callback for "detach tape image" menu items
 *
 * Removes any tape from the specified drive. No additional UI is
 * presented.
 *
 * \param[in]   widget      menu item triggering the callback
 * \param[in]   user_data   ignored
 */
void ui_tape_detach_callback(GtkWidget *widget, gpointer user_data)
{
    tape_image_detach(1);
}


/** \brief  Clean up the last directory string
 */
void ui_tape_attach_shutdown(void)
{
    lastdir_shutdown(&last_dir);
}
