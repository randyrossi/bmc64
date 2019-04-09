/** \file   sounddriverwidget.c
 * \brief   GTK3 sound driver widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES SoundDeviceName     all
 * $VICERES SoundDeviceArg      all
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

#include "basewidgets.h"
#include "lib.h"
#include "ui.h"
#include "resources.h"
#include "vsync.h"
#include "sound.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"

#include "sounddriverwidget.h"


#define LABEL_BUFFER_SIZE   256


/*
 * Event handlers
 */


/** \brief  Handler for the "changed" event of the device combobox
 *
 * \param[in]   widget      widget triggering the event
 * \param[in]   user_data   data for the event (unused)
 */
static void on_device_changed(GtkWidget *widget, gpointer user_data)
{
    const char *id;

    id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget));
    debug_gtk3("device ID = '%s'.", id);
    resources_set_string("SoundDeviceName", id);
}


/*
 * Helper functions
 */


/** \brief  Create combobox with sound devices
 *
 * \return  combobox
 */
static GtkWidget *create_device_combobox(void)
{
    GtkWidget *combo;
    int i;
    int count = sound_device_num();
    const char *current_device = NULL;

    debug_gtk3("%d sound devices.", count);

    resources_get_string("SoundDeviceName", &current_device);
    debug_gtk3("current device: '%s'.", current_device);

    combo = gtk_combo_box_text_new();
    for (i = 0; i < count; i++) {
        const char *device = sound_device_name(i);
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo),
                device, device);
    }

    /* set active device */
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo), current_device);

    /* now connect event handler */
    g_signal_connect(combo, "changed", G_CALLBACK(on_device_changed), NULL);

    return combo;
}


/** \brief  Create the 'device driver argument' text entry box
 *
 * \return  entry
 */
static GtkWidget *create_argument_entry(void)
{
    return vice_gtk3_resource_entry_full_new("SoundDeviceArg");
}


/** \brief  Create the sound driver/device widget
 *
 * \return  driver widget (GtkGrid)
 */
GtkWidget *sound_driver_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *device;
    GtkWidget *args;

    debug_gtk3("called.");

    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT, "Driver", 2);

    gtk_grid_attach(GTK_GRID(grid),
            vice_gtk3_create_indented_label("Device name"), 0, 1, 1, 1);
    device = create_device_combobox();
    gtk_widget_set_hexpand(device, TRUE);
    gtk_grid_attach(GTK_GRID(grid), device, 1, 1, 1, 1);

    args = create_argument_entry();
    gtk_widget_set_hexpand(args, TRUE);
    gtk_grid_attach(GTK_GRID(grid),
            vice_gtk3_create_indented_label("Driver argument"), 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), args, 1, 2, 1, 1);

    g_object_set(grid, "margin", 8, NULL);

    gtk_widget_show_all(grid);
    return grid;
}
