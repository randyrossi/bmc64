/** \file   c128machinetypewidget.c
 * \brief   C128 machine type widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES MachineType x128
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
#include "c128.h"

#include "c128machinetypewidget.h"

/** \brief  List of C128 machine types
 */
static const vice_gtk3_radiogroup_entry_t machine_types[] = {
    { "International",  C128_MACHINE_INT },
    /* any of these cause a KERNAL corrupted message from c128mem */
    { "Finish",         C128_MACHINE_FINNISH },
    { "French",         C128_MACHINE_FRENCH },
    { "German",         C128_MACHINE_GERMAN },
    { "Italian",        C128_MACHINE_ITALIAN },
    { "Norwegian",      C128_MACHINE_NORWEGIAN },
    { "Swedish",        C128_MACHINE_SWEDISH },
    { "Swiss",          C128_MACHINE_SWISS },
    { NULL, -1 }
};


/** \brief  Create C128 machine type widget
 *
 * \return  GtkGrid
 */
GtkWidget * c128_machine_type_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *radio_group;

    grid = uihelpers_create_grid_with_label("Machine type", 1);
    radio_group = vice_gtk3_resource_radiogroup_new(
            "MachineType", machine_types, GTK_ORIENTATION_VERTICAL);
    g_object_set(radio_group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}
