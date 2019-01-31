/** \file   cartimagewidget.c
 * \brief   Widget to control load/save/flush for cart images
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
 *
 */

#include "vice.h"
#include <gtk/gtk.h>

#include "machine.h"
#include "resources.h"
#include "debug_gtk3.h"
#include "basewidgets.h"
#include "widgethelpers.h"
#include "basedialogs.h"
#include "openfiledialog.h"
#include "savefiledialog.h"
#include "cartridge.h"

#include "cartimagewidget.h"


static int (*save_func)(int, const char *) = NULL;
static int (*flush_func)(int) = NULL;
static const char *crt_name;   /**< cartridge name used in messages */
static int crt_id;  /**< cartridge ID in cartridge_*() calls */
static const char *res_fname;
static const char *res_write;


/** \brief  Handler for the "clicked" event of the "browse" button
 *
 * Select an image file for the extension.
 *
 * \param[in]   button      browse button
 * \param[in]   user_data   unused
 */
static void on_browse_clicked(GtkWidget *button, gpointer user_data)
{
    gchar *filename;
    char buffer[256];

    g_snprintf(buffer, 256, "Open or create %s image file", crt_name);

    filename = vice_gtk3_open_create_file_dialog(buffer, NULL, FALSE, NULL);
    if (filename != NULL) {
        GtkWidget *grid = gtk_widget_get_parent(button);
        GtkWidget *entry = gtk_grid_get_child_at(GTK_GRID(grid), 1, 1);

        vice_gtk3_resource_entry_full_set(entry, filename);
        g_free(filename);
    }
}


/** \brief  Handler for the "clicked" event of the "save" button
 *
 * Save cart image file. Uses dirname()/basename() on the GEORAMfilename
 * resource to act as a "Save" button, but also allows changing filename/dir
 * to act as a "Save As" button.
 *
 * \param[in]   button      save button
 * \param[in]   user_data   unused
 */
static void on_save_clicked(GtkWidget *button, gpointer user_data)
{
    gchar *new_filename;
    gchar *fname = NULL;
    gchar *dname = NULL;
    char buffer[256];

    /* the BIOS/EEPROM filename isn't always/never is the cartridge image name */
#if 0
    resources_get_string(res_fname, &current_filename);
    if (current_filename != NULL && *current_filename != '\0') {
        /* provide the current filename and path */
        fname = g_path_get_basename(current_filename);
        dname = g_path_get_dirname(current_filename);
        debug_gtk3("got dir '%s', file '%s'\n", dname, fname);
    }
#endif

    g_snprintf(buffer, 256, "Save %s image file", crt_name);
    new_filename = vice_gtk3_save_file_dialog(buffer, fname, TRUE, dname);
    if (new_filename != NULL) {
        debug_gtk3("writing %s file image as '%s'\n", crt_name, new_filename);
        /* write file */
        if (save_func != NULL) {
            if (save_func(crt_id, new_filename) < 0) {
                /* oops */
                vice_gtk3_message_error("I/O error",
                        "Failed to save '%s'", new_filename);
            }
        } else {
            vice_gtk3_message_error("Core error",
                    "%s save handler not specified", crt_name);
        }
        g_free(new_filename);
    }

    if (fname != NULL) {
        g_free(fname);
    }
    if (dname != NULL) {
        g_free(dname);
    }
}


/** \brief  Handler for the "clicked" event of the "Flush image" button
 *
 * \param[in]   widget      button triggering the event
 * \param[in]   user_data   unused
 */
static void on_flush_clicked(GtkWidget *widget, gpointer user_data)
{
    if (flush_func != NULL) {
        if (flush_func(crt_id) < 0) {
            vice_gtk3_message_error("I/O error", "Failed to flush image");
        }
    } else {
        vice_gtk3_message_error("Core error",
                "%s flush handler not specified", crt_name);
    }
}


/** \brief  Create widget to load/save/flush cart image file
 *
 * \param[in]   parent          parent widget (used for dialogs)
 * \param[in]   title           widget title
 * \param[in[   resource_fname  resource for the image file name
 * \param[in]   resource_fwrite resource controlling flush-on-exit/detach
 * \param[in]   func_save       function to save the image via dialog
 * \param[in]   func_flush      function to flush current image to host
 * \param[in]   cart_name       cartridge name to use in dialogs
 * \param[in]   cart_id         cartridge ID to use in save/flush callbacks
 *
 * \note    \a cartname and \a cart_id should be taken from cartridge.h
 *
 * \return  GtkGrid
 */
GtkWidget *cart_image_widget_create(
        GtkWidget *parent, const char *title,
        const char *resource_fname, const char *resource_write,
        int (*func_save)(int, const char *),
        int (*func_flush)(int),
        const char *cart_name, int cart_id)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *browse;
    GtkWidget *auto_save;
    GtkWidget *save_button;
    GtkWidget *flush_button;

    res_fname = resource_fname;
    res_write = resource_write;
    save_func = func_save;
    flush_func = func_flush;
    crt_name = cart_name;
    crt_id = cart_id;

    grid = uihelpers_create_grid_with_label(title, 3);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    label = gtk_label_new("file name");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    entry = vice_gtk3_resource_entry_full_new(resource_fname);
    gtk_widget_set_hexpand(entry, TRUE);
    /* gtk_widget_set_sensitive(entry, FALSE); */
    browse = gtk_button_new_with_label("Browse ...");

    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);

    auto_save = vice_gtk3_resource_check_button_new(resource_write,
            "Write image on image detach/emulator quit");
    g_object_set(auto_save, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), auto_save, 0, 2, 2, 1);



    save_button = gtk_button_new_with_label("Save as ...");
    gtk_grid_attach(GTK_GRID(grid), save_button, 2, 2, 1, 1);

    flush_button = gtk_button_new_with_label("Flush image");
    gtk_grid_attach(GTK_GRID(grid), flush_button, 2, 3, 1, 1);

    g_signal_connect(browse, "clicked", G_CALLBACK(on_browse_clicked), NULL);
    g_signal_connect(save_button, "clicked", G_CALLBACK(on_save_clicked), NULL);
    g_signal_connect(flush_button, "clicked", G_CALLBACK(on_flush_clicked),
            NULL);

    gtk_widget_show_all(grid);
    return grid;
}
