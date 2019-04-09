/** \file   ieee488widget.c
 * \brief   IEEE-488 adapter widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES IEEE488         x64 x64sc xscpu64 x128 xvic
 * $VICERES IEEE488Image    x64 x64sc xscpu64 x128
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
#include "cartridge.h"

#include "ieee488widget.h"


/** \brief  Handler for the "toggled" event of the 'enable' check button
 *
 * Toggles the 'enabled' state of the IEEE-488 adapter/cart, but only if an
 * EEPROM image has been specified, otherwise when trying to set the check
 * button to 'true', an error message is displayed and the check button is
 * reverted to 'off'.
 *
 * \param[in,out]   widget  check button
 * \param[in]       data    unused
 */
static void on_enable_toggled(GtkWidget *widget, gpointer data)
{
    int state;
    const char *image;

    state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (state) {
        if (resources_get_string("IEEE488Image", &image) < 0) {
            image = NULL;
        }

        if (image == NULL || *image == '\0') {
            /* no image */
            vice_gtk3_message_error("VICE core",
                    "Cannot enable IEEE-488 adapter, no image specified.");
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FALSE);
            state = 0;
        }
    }

    if (state) {
        if (carthelpers_enable_func(CARTRIDGE_IEEE488) < 0) {
            debug_gtk3("failed to enable IEEE488 cartridge.");
        }
    } else {
        if (carthelpers_disable_func(CARTRIDGE_IEEE488) < 0) {
            debug_gtk3("failed to disable IEEE488 cartridge.");
        }
    }
}


/** \brief  Handler for the "clicked" event of the browse button
 *
 * Activates a file-open dialog and stores the file name in the GtkEntry passed
 * in \a user_data if valid, triggering a resource update.
 *
 * \param[in]   widget      button
 * \param[in]   user_data   entry to store filename in
 */
static void on_browse_clicked(GtkWidget *widget, gpointer user_data)
{
    gchar *filename;

    filename = vice_gtk3_open_file_dialog("Open IEEE-488 image", NULL, NULL,
            NULL);
    if (filename != NULL) {
        GtkEntry *entry = GTK_ENTRY(user_data);
        debug_gtk3("setting IEEE488Image to '%s'.", filename);
        gtk_entry_set_text(entry, filename);
        /* required, since setting the text of the entry doesn't trigger an
         * update of the connected resource (it only responds to focus-out and
         * pressing 'Enter' */
        if (resources_set_string("IEEE488Image", filename) < 0) {
            vice_gtk3_message_error("VICE core",
                    "Failed to set '%s' as IEEE-488 EEPROM image.",
                    filename);
        }
        g_free(filename);
    }
}


/** \brief  Create widget to control IEEE-488 adapter
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *ieee488_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *enable_widget;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *browse;

    const char *image;
    int enable_state;

    if (resources_get_string("IEEE488Image", &image) < 0) {
        image = NULL;
    }
    enable_state = carthelpers_is_enabled_func(CARTRIDGE_IEEE488);

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    /* we can't use a `resource_check_button` here, since toggling the resource
     * depends on whether an image file is specified
     */
    enable_widget = gtk_check_button_new_with_label("Enable IEEE-488 interface");
    gtk_grid_attach(GTK_GRID(grid), enable_widget, 0, 0, 3, 1);

    /* only set state to true if both the state is true and an image is given */
    if (enable_state && (image != NULL && *image != '\0')) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(enable_widget), TRUE);
    }

    label = gtk_label_new("IEEE-488 image");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    entry = vice_gtk3_resource_entry_full_new("IEEE488Image");
    gtk_widget_set_hexpand(entry, TRUE);
    browse = gtk_button_new_with_label("Browse ...");
    g_signal_connect(browse, "clicked", G_CALLBACK(on_browse_clicked),
            (gpointer)entry);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);

    g_signal_connect(enable_widget, "toggled", G_CALLBACK(on_enable_toggled), NULL);

    gtk_widget_show_all(grid);
    return grid;
}
