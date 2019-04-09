/** \file   petiosizewidget.c
 * \brief   Widget to set the PET I/O area size (PET 8296 only)
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES IOSize  xpet
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

#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "basewidgets.h"
#include "resources.h"

#include "petiosizewidget.h"


/** \brief  Available I/O sizes
 */
static const vice_gtk3_radiogroup_entry_t io_sizes[] = {
    { "256 bytes", 256 },
    { "2048 bytes", 2048 },
    { NULL, -1 }
};


static void (*user_callback)(int) = NULL;



static void on_io_size_changed(GtkWidget *widget, int id)
{
    if (user_callback != NULL) {
        user_callback(id);
    }
}


/** \brief  Create PET I/O area size widget
 *
 * \return  GtkGrid
 */
GtkWidget *pet_io_size_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *group;

    user_callback = NULL;

    grid = uihelpers_create_grid_with_label("I/O area size", 1);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    group = vice_gtk3_resource_radiogroup_new("IOSize", io_sizes,
            GTK_ORIENTATION_VERTICAL);
    vice_gtk3_resource_radiogroup_add_callback(group, on_io_size_changed);
    g_object_set(group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


void pet_io_size_widget_set_callback(GtkWidget *widget,
                                     void (*func)(int))
{
    user_callback = func;
}



void pet_io_size_widget_sync(GtkWidget *widget)
{
    int size;

    resources_get_int("IOSize", &size);
    GtkWidget *group = gtk_grid_get_child_at(GTK_GRID(widget), 0, 1);
    vice_gtk3_resource_radiogroup_set(group, size);
}
