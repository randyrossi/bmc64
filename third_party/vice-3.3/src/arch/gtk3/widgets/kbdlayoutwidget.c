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
#include "keyboard.h"
#include "lib.h"
#include "widgethelpers.h"

#include "kbdlayoutwidget.h"



/** \brief  Keyboard layout types
 *
 * Dynamically allocated in the "constructor" and freed in the "destructor"
 */
static vice_gtk3_radiogroup_entry_t *kbd_layouts = NULL;


/** \brief  Handler for the 'destroy' event of the widget
 *
 * Frees memory used by radio buttons list
 *
 * \param[in]   widget  widget (unused)
 * \param[in]   data    extra event data (unused)
 */
static void on_destroy(GtkWidget *widget, gpointer data)
{
    lib_free(kbd_layouts);
}


/** \brief  Create a keyboard layout selection widget
 *
 * \return  GtkGrid
 */
GtkWidget *kbdlayout_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *group;
    mapping_info_t *kbdinfo;
    int num;
    int idx;

    /* build the list from the list provided by keyboard.c */

    /* allocate memory for radio buttons list */
    num = keyboard_get_num_mappings();
    kbd_layouts = lib_malloc((num + 1) * sizeof *kbd_layouts);

    /* populate radio buttons list */
    kbdinfo = keyboard_get_info_list();
    idx = 0;
    while (kbdinfo->name != NULL) {
        kbd_layouts[idx].name = kbdinfo->name;
        kbd_layouts[idx].id = kbdinfo->mapping;
        idx++;
        kbdinfo++;
    }
    /* terminate list */
    kbd_layouts[idx].name = NULL;
    kbd_layouts[idx].id = -1;

    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT, "Host keyboard layout", 1);
    group = vice_gtk3_resource_radiogroup_new(
            "KeyboardMapping", kbd_layouts, GTK_ORIENTATION_VERTICAL);
    g_object_set(group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);

    /* connect signal handler to free memory used by the radio buttons list
     * when the widget is destroyed */
    g_signal_connect(grid, "destroy", G_CALLBACK(on_destroy), NULL);

    gtk_widget_show_all(grid);
    return grid;
}
