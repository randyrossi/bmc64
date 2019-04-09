/** \file   scpu64settingswidget.c
 * \brief   Widget controlling SCPU6-specific settings
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES SIMMSize    xscpu64
 * $VICERES JiffySwitch xscpu64
 * $VICERES SpeedSwitch xscpu64
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

#include <stdio.h>
#include <string.h>
#include <gtk/gtk.h>

#include "vice_gtk3.h"
#include "machine.h"
#include "resources.h"

#include "scpu64settingswidget.h"


/** \brief  List of SCPU64 SIMM sizes
 */
static const vice_gtk3_combo_entry_int_t simm_sizes[] = {
    { "0 MB", 0 },
    { "1 MB", 1 },
    { "4 MB", 4 },
    { "8 MB", 8 },
    { "16 MB", 16 },
    { NULL, -1 }
};


/** \brief  Create left-aligned, 16 px indented label from \a text
 *
 * \param[in]   text    text for the label
 *
 * \return  GtkLabel
 */
static GtkWidget *create_indented_label(const char *text)
{
    GtkWidget *label = gtk_label_new(text);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    return label;
}


/** \brief  Create widgets for SCPU64-specific settings
 *
 * \param[in]   parent  parent widget (ignored)
 *
 * \return  GtkGrid
 */
GtkWidget *scpu64_settings_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *simm_widget;
    GtkWidget *jiffy_widget;
    GtkWidget *speed_widget;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    label = create_indented_label("SIMM size");
    simm_widget = vice_gtk3_resource_combo_box_int_new("SIMMSize",
            simm_sizes);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), simm_widget, 1, 0, 1, 1);

    label = create_indented_label("JiffyDOS switch");
    jiffy_widget = vice_gtk3_resource_switch_new("JiffySwitch");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), jiffy_widget, 1, 1, 1, 1);

    label = create_indented_label("Speed switch");
    speed_widget = vice_gtk3_resource_switch_new("SpeedSwitch");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), speed_widget, 1, 2, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
