/**\file    cbm2memorysizewidget.c
 * \brief   CBM-II memory size widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES RamSize     xcbm5x0 xcbm2
 */

/*
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *
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
#include "machine.h"

#include "cbm2memorysizewidget.h"


/** \brief  List of RAM sizes for 5x0
 */
static const vice_gtk3_radiogroup_entry_t ram_sizes_cbm5x0[] = {
    { "64KB",   64 },
    { "128KB",  128 },
    { "256KB",  256 },
    { "512KB",  512 },
    { "1024KB", 1024 },
    { NULL, -1 }
};


/** \brief  List of RAM sizes for 6x0/7x0
 */
static const vice_gtk3_radiogroup_entry_t ram_sizes_cbm6x0[] = {
    { "128KB",  128 },
    { "256KB",  256 },
    { "512KB",  512 },
    { "1024KB", 1024 },
    { NULL, -1 }
};



/** \brief  Create CBM-II memory size widget
 *
 * \return  GtkGrid
 */
GtkWidget *cbm2_memory_size_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *radio_group;
    const vice_gtk3_radiogroup_entry_t *ram_sizes;

    if (machine_class == VICE_MACHINE_CBM5x0) {
        ram_sizes = ram_sizes_cbm5x0;
    } else {
        ram_sizes = ram_sizes_cbm6x0;
    }

    grid = uihelpers_create_grid_with_label("RAM size", 1);
    radio_group = vice_gtk3_resource_radiogroup_new("RamSize", ram_sizes,
            GTK_ORIENTATION_VERTICAL);
    g_object_set(radio_group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Set used-defined callback to trigger when the RAM size changes
 *
 * \param[in,out]   widget  cbm2 memory size widget
 * \param[in]       user-defined callback
 */
void cbm2_memory_size_widget_set_callback(
        GtkWidget *widget,
        void (*callback)(GtkWidget *, int))
{
    GtkWidget *group = gtk_grid_get_child_at(GTK_GRID(widget), 0, 1);
    if (group != NULL) {
        vice_gtk3_resource_radiogroup_add_callback(group, callback);
    }
}


/** \brief  Update/sync widget via its resource
 *
 * \param[in]   widget  cbm2 memory size widget
 */
void cbm2_memory_size_widget_update(GtkWidget *widget)
{
    GtkWidget *group = gtk_grid_get_child_at(GTK_GRID(widget), 0, 1);
    if (group != NULL) {
        vice_gtk3_resource_radiogroup_sync(widget);
    }
}

