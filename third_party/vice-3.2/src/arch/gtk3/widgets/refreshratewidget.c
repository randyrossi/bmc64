/** \file   refreshratewidget.c
 * \brief   GTK3 refresh rate widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES RefreshRate     all
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
#include "resources.h"
#include "vsync.h"
#include "debug_gtk3.h"
#include "widgethelpers.h"

#include "refreshratewidget.h"


/** \brief  List of text/id pairs for the refresh rates
 */
static const vice_gtk3_radiogroup_entry_t refresh_rates[] = {
    { "Automatic", 0 },
    { "1/1", 1 },
    { "1/2", 2 },
    { "1/3", 3 },
    { "1/4", 4 },
    { "1/5", 5 },
    { "1/6", 6 },
    { "1/7", 7 },
    { "1/8", 8 },
    { "1/9", 9 },
    { "1/10", 10 },
    { NULL, -1 }
};


/** \brief  Create 'fresh rate' widget
 *
 * \todo    Get current refresh rate from resources and set proper radio button
 *
 * \return  GtkGrid
 */
GtkWidget *refreshrate_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *group;

    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT, "Refresh rate", 1);
    group = vice_gtk3_resource_radiogroup_new("RefreshRate", refresh_rates,
            GTK_ORIENTATION_VERTICAL);
    g_object_set(group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
