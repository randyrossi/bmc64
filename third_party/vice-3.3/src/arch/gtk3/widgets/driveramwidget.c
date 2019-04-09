/** \file   driveramwidget.c
 * \brief   Drive RAM expansions widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/* TODO: check drives against emus to turn this into a proper table:
 *  Drive[8-11]RAM2000 (1540, 1541 and 1541-II)
 *  Drive[8-11]RAM4000 (1540, 1541, 1541-II, 1570, 1571 and 1751CR)
 *  Drive[8-11]RAM6000 (1540, 1541, 1541-II, 1570, 1571 and 1751CR)
 *  Drive[8-11]RAM8000 (1540, 1541 and 1541-II)
 *  Drive[8-11]RAMA000 (1540, 1541 and 1541-II)
 *
 * This probably not quite correct:
 *
 * $VICERES Drive8RAM2000       -vsid
 * $VICERES Drive9RAM2000       -vsid
 * $VICERES Drive10RAM2000      -vsid
 * $VICERES Drive11RAM2000      -vsid
 * $VICERES Drive8RAM4000       -vsid
 * $VICERES Drive9RAM4000       -vsid
 * $VICERES Drive10RAM4000      -vsid
 * $VICERES Drive11RAM4000      -vsid
 * $VICERES Drive8RAM6000       -vsid
 * $VICERES Drive9RAM6000       -vsid
 * $VICERES Drive10RAM6000      -vsid
 * $VICERES Drive11RAM6000      -vsid
 * $VICERES Drive8RAM8000       -vsid
 * $VICERES Drive9RAM8000       -vsid
 * $VICERES Drive10RAM8000      -vsid
 * $VICERES Drive11RAM8000      -vsid
 * $VICERES Drive8RAMA000       -vsid
 * $VICERES Drive9RAMA000       -vsid
 * $VICERES Drive10RAMA000      -vsid
 * $VICERES Drive11RAMA000      -vsid
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
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "drive.h"
#include "drive-check.h"
#include "machine.h"

#include "driveramwidget.h"

enum {
    RAM2000_INDEX = 1,
    RAM4000_INDEX,
    RAM6000_INDEX,
    RAM8000_INDEX,
    RAMA000_INDEX
};



/** \brief  Create drive RAM expansion check button
 *
 * \param[in]   unit    unit number (8-11)
 * \param[in]   base    RAM base address (word)
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_ram_check_button(int unit, unsigned int base)
{
    GtkWidget *check;
    char label[256];

    g_snprintf(label, 256, "$%04X-$%04X RAM", base, base + 0x1fff);
    check = vice_gtk3_resource_check_button_new_sprintf("Drive%dRAM%04X", label,
            unit, base);
    g_object_set(check, "margin-left", 16, NULL);
    return check;
}


GtkWidget *drive_ram_widget_create(int unit)
{
    GtkWidget *grid;
    unsigned int base;
    int row;

    grid = uihelpers_create_grid_with_label("RAM expansions", 1);
    g_object_set_data(G_OBJECT(grid), "UnitNumber", GINT_TO_POINTER(unit));

    row = 1;
    for (base = 0x2000; base <= 0xa000; base += 0x2000) {
        GtkWidget *check = create_ram_check_button(unit, base);
        g_object_set(check, "margin-left", 16, NULL);
        gtk_grid_attach(GTK_GRID(grid), check, 0, row, 1, 1);
        row++;
    }

    gtk_widget_show_all(grid);
    return grid;
}
