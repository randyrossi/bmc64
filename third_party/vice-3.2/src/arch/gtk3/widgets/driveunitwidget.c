/** \file   driveunitwidget.c
 * \brief   GTK3 drive unit selection widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
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

#include "widgethelpers.h"
#include "debug_gtk3.h"

#include "driveunitwidget.h"


/** \brief  Destination of the unit number when changed
 *
 * Set when the unit number in the widget changes, pass `NULL` as the `target`
 * argument to disable this.
 */
static int *unit_target;


/** \brief  Callback triggered on unit number change
 *
 * This is an optional callback to allow updating other widgets depending on
 * the selected unit number. Pass `NULL` as the `callback` argument to
 * create_drive_unit_widget() to disable this.
 */
static void (*unit_callback)(int) = NULL;


/** \brief  Handler for the "toggled" events of the radio buttons
 *
 * \param[in]   widget      radio button triggering the event
 * \param[in]   user_data   unit number (int)
 */
static void on_radio_toggled(GtkWidget *widget, gpointer user_data)
{
    int unit = GPOINTER_TO_INT(user_data);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        debug_gtk3("setting unit to %d\n", unit);
        if (unit_target != NULL) {
            *unit_target = unit;
        }
        if (unit_callback != NULL) {
            unit_callback(unit);
        }
    }
}


/** \brief  Create a GtkGrid with radio buttons in a group for unit numbers
 *
 * Sets up event handler to change the `unit_target` variable passed in
 * create_drive_unit_widget().
 *
 * \param[in]   unit    default unit
 *
 * \return GtkGrid
 */
static GtkWidget *create_radio_group(int unit)
{
    GtkWidget *grid;
    GtkRadioButton *last;
    GSList *group = NULL;
    int i;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    last = NULL;
    for (i = 8; i < 12; i++) {
        gchar buffer[16];
        GtkWidget *radio;

        g_snprintf(buffer, 16, "%d", i);

        radio = gtk_radio_button_new_with_label(group, buffer);
        gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio), last);
        gtk_grid_attach(GTK_GRID(grid), radio, i - 8, 0, 1, 1);

        if (unit == i) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
        }

        /* don't connect signal handlers here, since that can trigger the
         * handlers through the previous statement */

        last = GTK_RADIO_BUTTON(radio);
    }
    gtk_widget_show_all(grid);
    return grid;
}



/** \brief  Create a unit#-selection widget
 *
 * \param[in]   unit    currently selected unit number (8-11)
 * \param[out]  target  destination of the unit number on radio button clicks
 *
 * \return  GtkGrid
 */
GtkWidget *drive_unit_widget_create(int unit, int *target, void (*callback)(int))
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *group;
    int i;

    unit_target = target;
    unit_callback = callback;

    if (target != NULL) {
        *target = unit; /* make sure we don't end up with unintialized data */
    }

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    label = gtk_label_new("Unit #:");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    group = create_radio_group(unit);
    gtk_grid_attach(GTK_GRID(grid), group, 1, 0, 1, 1);

    /* connect signal handlers */
    for (i = 0; i < 4; i++) {
        GtkWidget *radio = gtk_grid_get_child_at(GTK_GRID(group), i, 0);
        g_signal_connect(radio, "toggled", G_CALLBACK(on_radio_toggled),
            GINT_TO_POINTER(i + 8));
    }

    gtk_widget_show_all(grid);
    return grid;
}
