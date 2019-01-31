/** \file   driveextendpolicywidget.c
 * \brief   Drive 40-track extend policy widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES Drive8ExtendImagePolicy     -vsid
 * $VICERES Drive9ExtendImagePolicy     -vsid
 * $VICERES Drive10ExtendImagePolicy    -vsid
 * $VICERES Drive11ExtendImagePolicy    -vsid
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
#include "drive.h"
#include "drive-check.h"
#include "drivewidgethelpers.h"

#include "driveextendpolicywidget.h"


/** \brief  List of (name,id) tuples for the radio buttons
 */
static vice_gtk3_combo_entry_int_t policies[] = {
    { "Never extend", 0 },
    { "Ask on extend", 1 },
    { "Extend on access", 2 },
    { NULL, -1 }
};


/** \brief  Create 40-track extend policy widget
 *
 * \param[in]   unit    drive unit number (8-11)
 *
 * \return  GtkGrid
 */
GtkWidget *drive_extend_policy_widget_create(int unit)
{
    GtkWidget *grid;
    GtkWidget *combo;

    grid = uihelpers_create_grid_with_label("40-track policy", 1);
    /* store unit number in "UnitNumber" property for later use */
    g_object_set_data(G_OBJECT(grid), "UnitNumber", GINT_TO_POINTER(unit));

    combo = vice_gtk3_resource_combo_box_int_new_sprintf(
            "Drive%dExtendImagePolicy", policies, unit);
    gtk_widget_set_hexpand(combo, TRUE);
    g_object_set(combo, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), combo, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Update the widget with data from \a unit
 *
 * \param[in,out]   widget  drive 40-track extend policy widget
 */
void drive_extend_policy_widget_update(GtkWidget *widget)
{
    int unit;
    int drive_type;

    unit = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "UnitNumber"));
    drive_type = ui_get_drive_type(unit);

    /* determine if this widget is valid for the current drive type */
    gtk_widget_set_sensitive(widget,
            drive_check_extend_policy(drive_type));
}
