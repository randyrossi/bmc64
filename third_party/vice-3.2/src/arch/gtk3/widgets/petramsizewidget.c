/** \file   petramsizewidget.c
 * \brief   Widget to set the PET RAM size
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES RamSize     xpet
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

#include "petramsizewidget.h"


/** \brief  Data for the radio buttons group
 */
static const vice_gtk3_radiogroup_entry_t ram_sizes[] = {
    { "4KB", 4 },
    { "8KB", 8 },
    { "16KB", 16 },
    { "32KB", 32 },
    { "96KB", 96 },
    { "128KB", 128 },
    { NULL, -1 }
};


/** \brief  Create PET RAM size widget
 *
 * Creates a widget to control the PET's RAM size
 *
 * \return  GtkGrid
 */
GtkWidget *pet_ram_size_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *group;

    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT,
            "Memory size", 1);
    group = vice_gtk3_resource_radiogroup_new("RamSize", ram_sizes,
            GTK_ORIENTATION_VERTICAL);
    g_object_set(group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
