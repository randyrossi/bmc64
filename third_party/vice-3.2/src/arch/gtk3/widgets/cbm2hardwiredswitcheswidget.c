/** \file   cbm2hardwiredswitcheswidget.c
 * \brief   Widget to select ModelLine
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES ModelLine   xcbm5x0 xcbm2
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
#include <glib/gstdio.h>

#include "lib.h"
#include "basewidgets.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "machine.h"

#include "cbm2hardwiredswitcheswidget.h"


/** \brief  Hardwired switches for 5x0 models
 */
static const vice_gtk3_radiogroup_entry_t models_cbm5x0[] = {
    { "50Hz 5x0", 2 },
    { "60Hz 5x0", 1 },
    { NULL, -1 }
};


/** \brief  Hardwired switches for 6x0 models
 */
static const vice_gtk3_radiogroup_entry_t models_cbm6x0[] = {
    { "50Hz 6x0", 2 },
    { "60Hz 6x0", 1 },
    { "50Hz 7x0", 0 },
    { NULL, -1 }
};


/** \brief  Create widget to control the 'hardwired switches', whatever those may be
 *
 * \return  GtkGrid
 */
GtkWidget *cbm2_hardwired_switches_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *radio_group;

    grid = uihelpers_create_grid_with_label("Hardwired switches", 1);
    radio_group = vice_gtk3_resource_radiogroup_new("ModelLine",
            machine_class == VICE_MACHINE_CBM5x0 ? models_cbm5x0 : models_cbm6x0,
            GTK_ORIENTATION_VERTICAL);
    g_object_set(radio_group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Set user-defined callback
 *
 * The \a callback is called with the ModelLine as its argument
 *
 * \param[in]   callback    user-defined callback
 */
void cbm2_hardwired_switches_widget_set_callback(
        GtkWidget *widget,
        void (*callback)(GtkWidget *, int))
{
    GtkWidget *group;

    group = gtk_grid_get_child_at(GTK_GRID(widget), 0, 1);
    if (group != NULL) {
        vice_gtk3_resource_radiogroup_add_callback(group, callback);
    }
}

