/** \file   drivemodelwidget.c
 * \brief   Drive model selection widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES Drive8Type      -vsid
 * $VICERES Drive9Type      -vsid
 * $VICERES Drive10Type     -vsid
 * $VICERES Drive11Type     -vsid
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

#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "machine-drive.h"
#include "resources.h"
#include "drive.h"
#include "drive-check.h"
#include "driveparallelcablewidget.h"
#include "drivewidgethelpers.h"
#include "driveoptionswidget.h"

#include "drivemodelwidget.h"


/** \brief  Handler for the "toggled" event of the radio buttons
 *
 * \param[in]   widget      radio button
 * \param[in]   user_data   unit number
 */
static void on_radio_toggled(GtkWidget *widget, gpointer user_data)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
        GtkWidget *parent;
        int unit;
        int new_type;
        int old_type;

        parent = gtk_widget_get_parent(widget);
        unit = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(parent), "UnitNumber"));
        new_type = GPOINTER_TO_INT(user_data);
        old_type = ui_get_drive_type(unit);


        /* prevent drive reset when switching unit number and updating the
         * drive type widget */
        if (new_type != old_type) {
            GtkWidget *parent;
            void (*cb_func)(GtkWidget *, gpointer);
            gpointer cb_data;

            debug_gtk3("setting Drive%dType to %d\n", unit, new_type);
            resources_set_int_sprintf("Drive%dType", new_type, unit);

            /* check for a custom callback */
            parent = gtk_widget_get_parent(widget);
            cb_func = g_object_get_data(G_OBJECT(parent), "CallbackFunc");
            if (cb_func != NULL) {
                /* get callback data */
                cb_data = g_object_get_data(G_OBJECT(parent), "CallbackData");
                /* trigger callback */
                cb_func(widget, cb_data);
            }

        }

        /* this should be handled by signal handlers in uidrivesettings.c now */
#if 0
        /* enable/disable 40-track settings widget */
        if (drive_extend_widget != NULL) {
             tgtk_widget_set_sensitive(drive_extend_widget,
                    drive_check_extend_policy(new_type));
        }
        /* update expansions widget */
        if (drive_expansion_widget != NULL) {
            drive_expansion_widget_update(drive_expansion_widget, unit_number);
        }
        /* update parallel cable widget */
        if (drive_parallel_cable_widget != NULL) {
            drive_parallel_cable_widget_update(drive_parallel_cable_widget,
                    unit_number);
        }
        if (drive_options_widget != NULL) {
            drive_options_widget_update(drive_options_widget, unit_number);
        }
#endif
    }
}


/** \brief  Create a drive unit selection widget
 *
 * Creates a widget with four radio buttons, horizontally aligned, to select
 * a drive unit (8-11)
 *
 * \param[in]   unit    default drive unit
 *
 * \return  GtkGrid
 */
GtkWidget *drive_model_widget_create(int unit)
{
    GtkWidget *grid;
    drive_type_info_t *list;
    GtkRadioButton *last = NULL;
    GSList *group = NULL;
    size_t i;
    int type;
    int num;
    int row;

    resources_get_int_sprintf("Drive%dType", &type, unit);

    grid = uihelpers_create_grid_with_label("Drive type", 2);
    /* store unit number as a property in the widget */
    g_object_set_data(G_OBJECT(grid), "UnitNumber", GINT_TO_POINTER(unit));

    list = machine_drive_get_type_info_list();
    for (i = 0; list[i].name != NULL; i++) {
        /* NOP */
    }
    num = i;

    for (i = 0; list[i].name != NULL && i < num / 2; i++) {
        GtkWidget *radio = gtk_radio_button_new_with_label(group, list[i].name);
        gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio), last);
        g_object_set(radio, "margin-left", 16, NULL);
        g_object_set_data(G_OBJECT(radio), "ModelID",
                GINT_TO_POINTER(list[i].id));

        if (list[i].id == type) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
        }

        g_signal_connect(radio, "toggled", G_CALLBACK(on_radio_toggled),
                GINT_TO_POINTER(list[i].id));

        gtk_grid_attach(GTK_GRID(grid), radio, 0, i + 1, 1, 1);
        last = GTK_RADIO_BUTTON(radio);
    }

    row = 1;
    while (list[i].name != NULL) {
        GtkWidget *radio = gtk_radio_button_new_with_label(group, list[i].name);
        gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio), last);
        g_object_set(radio, "margin-left", 16, NULL);
        g_object_set_data(G_OBJECT(radio), "ModelID",
                GINT_TO_POINTER(list[i].id));
        if (list[i].id == type) {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
        }

        g_signal_connect(radio, "toggled", G_CALLBACK(on_radio_toggled),
                GINT_TO_POINTER(list[i].id));

        gtk_grid_attach(GTK_GRID(grid), radio, 1, row, 1, 1);
        row++;
        last = GTK_RADIO_BUTTON(radio);
        i++;
    }

    drive_model_widget_update(grid);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Update the drive type widget
 *
 * This reiterates the drive info list to see if any setting changes made drive
 * types available/unavailable.
 *
 * \param[in,out]   widget  drive type widget
 * \param[in]       unit    new unit number
 */
void drive_model_widget_update(GtkWidget *widget)
{
    drive_type_info_t *list;
    size_t i;
    int unit;
    int type;

    unit = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(widget), "UnitNumber"));
    type = ui_get_drive_type(unit);

    list = machine_drive_get_type_info_list();
    debug_gtk3("updating drive type list\n");
    for (i = 0; list[i].name != NULL; i++) {
        GtkWidget *radio = gtk_grid_get_child_at(GTK_GRID(widget), 0, i + 1);
        if (radio != NULL && GTK_IS_RADIO_BUTTON(radio)) {
            gtk_widget_set_sensitive(radio, drive_check_type(
                        (unsigned int)(list[i].id), (unsigned int)(unit - 8)));
            if (list[i].id == type) {
                /* TODO: temporary block the resource-set callback */
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
            }
        }
    }
}


void drive_model_widget_add_callback(GtkWidget *widget,
                                     void (*cb_func)(GtkWidget *, gpointer),
                                     gpointer cb_data)
{
    g_object_set_data(G_OBJECT(widget), "CallbackFunc", (gpointer)cb_func);
    g_object_set_data(G_OBJECT(widget), "CallbackData", cb_data);
}
