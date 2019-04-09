/** \file   clockportdevicewidget.c
 * \brief   Widget to select ClockPort device
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
#include <stdlib.h>

#include "machine.h"
#include "resources.h"
#include "debug_gtk3.h"
#include "basewidgets.h"
#include "widgethelpers.h"
#include "resourcehelpers.h"

#include "clockportdevicewidget.h"


/** Struct to re-cast to `clockport_supported_devices_t` without actually
 *  using clockport.h"
 */
typedef struct clockport_dev_s {
    int id;         /**< clockport device ID */
    char *name;     /**< clockport device name */
} clockport_dev_t;


/** \brief  Pointer to `clockport_supported_devices` in clockport.c
 *
 * VSID doesn't link against clockport.o, so this hack is required
 */
static clockport_dev_t *clockport_devices;


static void on_destroy(GtkWidget *widget, gpointer user_data)
{
    resource_widget_free_resource_name(widget);
}


/** \brief  Handler for the "changed" event of the combo box
 *
 * \param[in]   widget      combo box
 * \param[in]   user_data   unused
 */
static void on_device_changed(GtkWidget *widget, gpointer user_data)
{
    int value;
    char *endptr;
    const char *resource;
    const char *id;

    resource = resource_widget_get_resource_name(widget);
    id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget));

    value = (int)strtol(id, &endptr, 10);
    if (*endptr == '\0') {
        debug_gtk3("setting %s to %d.", resource, value);
        resources_set_int(resource, value);
    }
}


/** \brief  Create widget to select ClockPort device
 *
 * \param[in]   resource    resource name
 *
 * \return  GtkGrid
 */
GtkWidget *clockport_device_widget_create(const char *resource)
{
    GtkWidget *combo;
    int current;
    int i;

    if (resources_get_int(resource, &current) < 0) {
        current = 0;
    }

    combo = gtk_combo_box_text_new();
    /* make a copy of the resource name */
    resource_widget_set_resource_name(combo, resource);

    for (i = 0; clockport_devices[i].id >= 0; i++) {
        char id_str[80];
        int id = clockport_devices[i].id;
        char *name = clockport_devices[i].name;

        /* combo boxes have a string ID */
        g_snprintf(id_str, 80, "%d", id);

        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), id_str, name);

        /* set currently selected device */
        if (current == id) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
        }
    }

    g_signal_connect(combo, "changed", G_CALLBACK(on_device_changed), NULL);
    g_signal_connect(combo, "destroy", G_CALLBACK(on_destroy), NULL);

    gtk_widget_show_all(combo);
    return combo;
}


/** \brief  Set pointer to list of supported clockport devices
 *
 * Unfortunately this is required since VSID doesn't like against clockport.o,
 * so I need to pass pointers around to access a list that is known at compile
 * time.
 *
 * \param[in]   devices pointer to clockport_supported_devices
 */
void clockport_device_widget_set_devices(void *devices)
{
    clockport_devices = (clockport_dev_t *)devices;
}

