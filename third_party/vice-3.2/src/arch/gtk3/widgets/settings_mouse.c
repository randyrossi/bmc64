/** \file   settings_mouse.c
 * \brief   Mouse settings widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES ps2mouse            x64dtv
 * $VICERES SmartMouseRTCSave   x64 x64sc xscpu64 x128 xvic xplus4 xcbm5x0
 * $VICERES MouseSensitivity
 *          (appears to be Windows-specific)
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

#include "resources.h"
#include "machine.h"
#include "vice_gtk3.h"

#include "settings_mouse.h"


#ifdef WIN32_COMPILE
/** \brief  Create mouse sensitivity slider
 *
 * Appears to be Win32 only.
 *
 * \return  GtkGrid
 */
static GtkWidget *create_sensitivity_widget(void)
{
    GtkWidget * grid;
    GtkWidget * scale;
    GtkWidget * label;
    int         current;

    if (resources_get_int("MouseSensitivity", &current) < 0) {
        current = 20;
    }

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    g_object_set(grid, "margin", 16, NULL);

    label = gtk_label_new("Mouse sensitivity");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 40.0, 1.0);
    gtk_range_set_value(GTK_RANGE(scale), (gdouble)current);
    gtk_widget_set_hexpand(scale, TRUE);
    gtk_grid_attach(GTK_GRID(grid), scale, 1, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
#endif


/** \brief  Create mouse settings widget
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *settings_mouse_widget_create(GtkWidget *parent)
{
    GtkWidget *layout;
    GtkWidget *ps2_enable;
    GtkWidget *mouse_save = NULL;
    int row = 0;

    layout = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);
    g_object_set(layout, "margin", 16, NULL);

    if (machine_class == VICE_MACHINE_C64DTV) {
        ps2_enable = vice_gtk3_resource_check_button_new("ps2mouse",
                "Enable PS/2 mouse on Userport");
        gtk_grid_attach(GTK_GRID(layout), ps2_enable, 0, row, 1, 1);
        row++;
    }

    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_C128:     /* fall through */
        case VICE_MACHINE_VIC20:    /* fall through */
        case VICE_MACHINE_PLUS4:    /* fall through */
        case VICE_MACHINE_CBM5x0:
            mouse_save = vice_gtk3_resource_check_button_new(
                    "SmartMouseRTCSave", "Enable SmartMouse RTC Saving");
            gtk_grid_attach(GTK_GRID(layout), mouse_save, 0, row, 1, 1);
            row++;
            break;
        default:
            /* No SmartMouse support */
            break;
    }

#ifdef WIN32_COMPILE
    gtk_grid_attach(GTK_GRID(layout), create_sensitivity_widget(), 0, row, 1, 1);
#endif

    gtk_widget_show_all(layout);
    return layout;
}
