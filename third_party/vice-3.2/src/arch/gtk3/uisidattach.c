/** \file   uisidattach.c
 * \brief   Gtk3 SID-attach dialog
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
#include <stdbool.h>

#include "debug_gtk3.h"
#include "basedialogs.h"
#include "filechooserhelpers.h"
#include "machine.h"
#include "lib.h"
#include "psid.h"
#include "ui.h"
#include "uiapi.h"
#include "vsync.h"

#include "uisidattach.h"


/* forward declarations */
/* static bool load_psid_handler(const char *filename); */


/** \brief  File type filters for the dialog
 */
static ui_file_filter_t filters[] = {
    { "SID/PSID files", file_chooser_pattern_sid },
    { "All files", file_chooser_pattern_all },
    { NULL, NULL }
};


/* function pointers to work around the linking differences between VSID and
 * other emulators
 */
static void (*psid_init_func)(void) = NULL;
static void (*psid_play_func)(int) = NULL;


/** \brief  Last used directory in dialog
 */
static gchar *last_dir = NULL;


/** \brief  Update the last directory reference
 *
 * \param[in]   widget  dialog
 */
static void update_last_dir(GtkWidget *widget)
{
    gchar *new_dir;

    new_dir = gtk_file_chooser_get_current_folder(GTK_FILE_CHOOSER(widget));
    debug_gtk3("new dir = '%s'\n", new_dir);
    if (new_dir != NULL) {
        /* clean up previous value */
        if (last_dir != NULL) {
            g_free(last_dir);
        }
        last_dir = new_dir;
    }
}


/*
 * Once I've merged my hvsclib, the preview widget can be used to show info
 * on a SID -- compyx
 */
#if 0
static GtkWidget *preview_widget = NULL;
#endif

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
            debug_gtk3("called with '%s'\n", path);
            /* TODO: show SID info */
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
static void on_response(GtkWidget *widget, gint response_id, gpointer user_data)
{
    gchar *filename;
    char *text;
    int index;

    index = GPOINTER_TO_INT(user_data);

    debug_gtk3("got response ID %d, index %d\n", response_id, index);

    switch (response_id) {

        /* 'Open' button, double-click on file */
        case GTK_RESPONSE_ACCEPT:
            update_last_dir(widget);
            filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(widget));
            text = lib_msprintf("Opening '%s'", filename);
            ui_display_statustext(text, TRUE);
            debug_gtk3("Loading SID file '%s'\n", filename);
            load_psid_handler(filename);
            g_free(filename);
            lib_free(text);
            gtk_widget_destroy(widget);
            break;

        /* 'Close'/'X' button */
        case GTK_RESPONSE_REJECT:
            gtk_widget_destroy(widget);
            break;
        default:
            break;
    }
}


/** \brief  Load and play PSID/SID file \a filename
 *
 * \param[in]   filename    file to play
 */
bool load_psid_handler(const char *filename)
{
    vsync_suspend_speed_eval();

    if (machine_autodetect_psid(filename) < 0) {
        debug_gtk3("'%s' is not a valid PSID file", filename);
        ui_error("'%s' is not a valid PSID file", filename);
        return false;
    }

    if (psid_init_func != NULL && psid_play_func != NULL) {
        psid_init_func();
        psid_play_func(0);
        machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
    }
    return true;
}


/** \brief  Create the 'extra' widget
 *
 * \return  GtkGrid
 */
static GtkWidget *create_extra_widget(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *hidden_check;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    hidden_check = gtk_check_button_new_with_label("Show hidden files");
    g_signal_connect(hidden_check, "toggled", G_CALLBACK(on_hidden_toggled),
            (gpointer)(parent));
    gtk_grid_attach(GTK_GRID(grid), hidden_check, 0, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create the SID attach dialog
 *
 * \param[in]   parent  parent widget, used to get the top level window
 *
 * \return  GtkFileChooserDialog
 */
static GtkWidget *create_sid_attach_dialog(GtkWidget *parent)
{
    GtkWidget *dialog;
    size_t i;

    /* create new dialog */
    dialog = gtk_file_chooser_dialog_new(
            "Load a SID/PSID file",
            ui_get_active_window(),
            GTK_FILE_CHOOSER_ACTION_OPEN,
            /* buttons */
            "Open", GTK_RESPONSE_ACCEPT,
            "Close", GTK_RESPONSE_REJECT,
            NULL, NULL);

    /* add 'extra' widget: 'readonly' and 'show preview' checkboxes */
    gtk_file_chooser_set_extra_widget(GTK_FILE_CHOOSER(dialog),
                                      create_extra_widget(dialog));

/*    preview_widget = content_preview_widget_create(dialog, tapecontents_read,
            on_response);
    gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog),
            preview_widget);
*/
    /* set last used directory, if present */
    if (last_dir != NULL) {
        gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), last_dir);
    }

    /* add filters */
    for (i = 0; filters[i].name != NULL; i++) {
        gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog),
                create_file_chooser_filter(filters[i], TRUE));
    }

    /* connect "reponse" handler: the `user_data` argument gets filled in when
     * the "response" signal is emitted: a response ID */
    g_signal_connect(dialog, "response", G_CALLBACK(on_response), NULL);
    g_signal_connect(dialog, "update-preview", G_CALLBACK(on_update_preview), NULL);

    return dialog;

}


/** \brief  Callback for the "Load SID file" menu item
 *
 * Creates the dialog and runs it.
 *
 * \param[in]   widget  menu item triggering the callback
 * \param[in]   data    ignored
 */
void uisidattach_show_dialog(GtkWidget *widget, gpointer data)
{
    GtkWidget *dialog;

    debug_gtk3("called\n");
    dialog = create_sid_attach_dialog(widget);
    gtk_widget_show(dialog);

}


/** \brief  Set psid init function
 *
 * \param[in]   func    psid_init_driver() function
 */
void uisidattach_set_psid_init_func(void (*func)(void))
{
    psid_init_func = func;
}


/** \brief  Set psid play function
*
* \param[in]   func    machine_play_psid() function
*/
void uisidattach_set_psid_play_func(void (*func)(int))
{
    psid_play_func = func;
}


/** \brief  Clean up the last directory string
 */
void uisidattach_shutdown(void)
{
    if (last_dir != NULL) {
        g_free(last_dir);
    }
}
