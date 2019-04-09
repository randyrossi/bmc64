/** \file   src/arch/gtk3/widgets/jamactionwidget.c
 * \brief   GTK3 default JAM action setting dialog
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES JAMAction   all
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
#include "debug_gtk3.h"
#include "basewidgets.h"
#include "widgethelpers.h"

#include "jamactionwidget.h"


/** \brief  List of possible actions on a CPU JAM
 */
static const vice_gtk3_radiogroup_entry_t actions[] = {
    { "Show dialog", 0 },
    { "Continue emulation", 1 },
    { "Start monitor", 2 },
    { "Soft RESET", 3 },
    { "Hard RESET", 4 },
    { "Quit emulator", 5 },
    { NULL, -1 }
};


/** \brief  Create widget to control the "JAMAction" resource
 *
 * \return  GtkGrid
 */
GtkWidget *jam_action_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *selection;

    grid = uihelpers_create_grid_with_label("Default action on CPU JAM", 1);

    selection = vice_gtk3_resource_radiogroup_new("JAMAction", actions,
            GTK_ORIENTATION_VERTICAL);
    g_object_set(selection, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), selection, 0, 1, 1,1);

    gtk_widget_show_all(grid);
    return grid;
}
