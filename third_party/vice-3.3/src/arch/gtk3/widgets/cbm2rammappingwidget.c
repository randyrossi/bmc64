/** \file   cbm2rammappingwidget.c
 * \brief   Widget to map RAM into bank 15 for CBM-II
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES Ram08   xcbm5x0 xcbm2
 * $VICERES Ram1    xcbm5x0 xcbm2
 * $VICERES Ram2    xcbm5x0/xcbm2
 * $VICERES Ram4    xcbm5x0 xcbm2
 * $VICERES Ram6    xcbm5x0 xcbm2
 * $VICERES RamC    xcbm5x0 xcbm2
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

#include "cbm2rammappingwidget.h"

/** \brief  Struct with ram mapping resource names and labels
 */
typedef struct ram_mapping_s {
    const char *text;
    char *resource;
} ram_mapping_t;


/** \brief  List of RAM mappings
 */
static const ram_mapping_t mappings[] = {
    { "$0800-$0FFF", "Ram08" },
    { "$1000-$1FFF", "Ram1" },
    { "$2000-$3FFF", "Ram2" },
    { "$4000-$5FFF", "Ram4" },
    { "$6000-$7FFF", "Ram6" },
    { "$C000-$CFFF", "RamC" },
    { NULL, NULL }
};


/** \brief  Create widget with RAM to expansion ROM mappings
 *
 * \return  GtkGrid
 */
GtkWidget *cbm2_ram_mapping_widget_create(void)
{
    GtkWidget *grid;
    int i;

    grid = uihelpers_create_grid_with_label("Map RAM into bank 15", 1);
    for (i = 0; mappings[i].text != NULL; i++) {
        GtkWidget *check;

        check = vice_gtk3_resource_check_button_new(mappings[i].resource,
                mappings[i].text);
        g_object_set(check, "margin-left", 16, NULL);
        gtk_grid_attach(GTK_GRID(grid), check, 0, i + 1, 1, 1);
    }

    gtk_widget_show_all(grid);
    return grid;
}
