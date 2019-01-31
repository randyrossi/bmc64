/** \file   settings_ramreset.c
 * \brief   Widget to control the RAM reset pattern settings
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES RAMInitStartValue       all
 * $VICERES RamInitValueInvert      all
 * $VICERES RAMInitPatternInvert    all
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
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "vice_gtk3.h"
#include "resources.h"

#include "settings_ramreset.h"


/** \brief  List of powers of two used for the widgets
 *
 * Yes, this looks silly, but allows me to use vice-gtk3 widgets.
 */
static const vice_gtk3_combo_entry_int_t powers_of_two[] = {
    { "0", 0 }, { "1", 1 }, { "2", 2 }, { "4", 4 }, { "8", 8 }, { "16", 16 },
    { "32", 32 }, { "64", 64 }, { "128", 128 }, { "256", 256 }, { "512", 512 },
    { "1024", 1024 }, { NULL, -1 }
};


/** \brief  Create a spin button controlling the "RAMInitStartValue" resource
 *
 * \return  GtkSpinButton
 */
static GtkWidget *create_start_value_widget(void)
{
    return vice_gtk3_resource_spin_int_new("RAMInitStartValue",
            0, 255, 1);
}


/** \brief  Create widget to control RAM init settings
 *
 * \param[in]   parent  parent widget (unused)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_ramreset_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *start_value_widget;
    GtkWidget *value_invert_widget;
    GtkWidget *pattern_invert_widget;

    grid = uihelpers_create_grid_with_label("RAM reset pattern", 2);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    label = gtk_label_new("Value of first byte");
    g_object_set(label, "margin-left", 16, NULL);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    start_value_widget = create_start_value_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), start_value_widget, 1, 1, 1, 1);

    label = gtk_label_new("Length of constant values");
    g_object_set(label, "margin-left", 16, NULL);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    value_invert_widget = vice_gtk3_resource_combo_box_int_new(
            "RAMInitValueInvert", powers_of_two);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_invert_widget, 1, 2, 1, 1);

    label = gtk_label_new("Length of constant pattern");
    g_object_set(label, "margin-left", 16, NULL);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    pattern_invert_widget = vice_gtk3_resource_combo_box_int_new(
            "RAMInitPatternInvert", powers_of_two);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), pattern_invert_widget, 1, 3, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
