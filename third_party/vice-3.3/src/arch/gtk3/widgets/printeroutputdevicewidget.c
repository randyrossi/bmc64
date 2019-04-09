/** \file   printeroutputdevicewidget.c
 * \brief   Widget to control printer output device settings
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES Printer4TextDevice  -vsid
 * $VICERES Printer5TextDevice  -vsid
 * $VICERES Printer6TextDevice  -vsid
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "basewidgets.h"
#include "resourcehelpers.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "printer.h"

#include "printeroutputdevicewidget.h"


/** \brief  List of text output devices
 */
static const vice_gtk3_radiogroup_entry_t device_list[] = {
    { "#1 (file dump)", 0 },
    { "#2 (exec)", 1 },
    { "#3 (exec)", 2 },
    { NULL, -1 }
};


/** \brief  Create widget to control the "Printer[4-6]TextDevice resource
 *
 * \param[in]   device number (4-6)
 *
 * \return  GtkGrid
 */
GtkWidget *printer_output_device_widget_create(int device)
{
    GtkWidget *grid;
    GtkWidget *radio_group;

    grid = uihelpers_create_grid_with_label("Output device", 1);
    radio_group = vice_gtk3_resource_radiogroup_new_sprintf(
            "Printer%dTextDevice", device_list, GTK_ORIENTATION_VERTICAL,
            device);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 0, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
