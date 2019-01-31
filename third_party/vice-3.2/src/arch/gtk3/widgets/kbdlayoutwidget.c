/** \file   kbdlayoutwidget.c
 * \brief   GTK3 keyboard layout widget for the settings dialog
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES KeyboardMapping     -vsid
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
#include "ui.h"
#include "resources.h"
#include "vsync.h"
#include "widgethelpers.h"

#include "debug_gtk3.h"

#include "kbdlayoutwidget.h"



/** \brief  Keyboard layout types
 */
static const vice_gtk3_radiogroup_entry_t kbd_layouts[] = {
    { "American", 0 },
    { "British", 1 },
    { "German", 2 },
    { "Danish", 3 },
    { "Norwegian", 4 },
    { "Finnish", 5 },
    { "Italian", 6 },
    { NULL, -1 }
};


/** \brief  Create a keyboard layout selection widget
 *
 * \return  GtkGrid
 *
 * \fixme   I'm not really satisfied with the 'select file' buttons, perhaps
 *          they should be placed next to the radio buttons?
 */
GtkWidget *kbdlayout_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *group;

    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT, "Keyboard layout", 1);
    group = vice_gtk3_resource_radiogroup_new(
            "KeyboardMapping", kbd_layouts, GTK_ORIENTATION_VERTICAL);
    g_object_set(group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}
