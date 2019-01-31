/** \file   printeremulationtypewidget.c
 * \brief   Widget to control printer emulation type
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 * \see     src/printer.h for the constants.
 */

/*
 * $VICERES Printer4    -vsid
 * $VICERES Printer5    -vsid
 * $VICERES Printer6    -vsid
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

#include "basewidgets.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "printer.h"

#include "printeremulationtypewidget.h"


/** \brief  List of printer emulation types
 */
static const vice_gtk3_radiogroup_entry_t emu_types[] = {
    { "None", PRINTER_DEVICE_NONE },
    { "File system access", PRINTER_DEVICE_FS },
    { "Real device access", PRINTER_DEVICE_REAL },
    { NULL, -1 }
};


/** \brief  Create printer emulation type widget
 *
 * Creates a group of radio buttons to select the emulation type of printer
 * # \a device. Uses a custom property "DeviceNumber" for the radio buttons to
 * pass the device number to the event handler.
 *
 * \param[in]   device  device number (4-6)
 *
 * \return  GtkGrid
 */
GtkWidget *printer_emulation_type_widget_create(int device)
{
    GtkWidget *grid;
    GtkWidget *radio_group;

    /* build grid */
    grid = uihelpers_create_grid_with_label("Emulation type", 1);
    radio_group = vice_gtk3_resource_radiogroup_new_sprintf(
            "Printer%d", emu_types, GTK_ORIENTATION_VERTICAL, device);
    g_object_set(radio_group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 0, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Update the printer emulation type widget
 *
 * \param[in]   widget  printer emulation type widget
 * \param[in]   type    emulation type (\see src/printer.h)
 */
void printer_emulation_type_widget_update(GtkWidget *widget, int type)
{
    GtkWidget *radio_group;

    radio_group = gtk_grid_get_child_at(GTK_GRID(widget), 0, 1);
    vice_gtk3_resource_radiogroup_set(radio_group, type);
}
