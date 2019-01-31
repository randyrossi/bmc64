/** \file   printerdriverwidget.c
 * \brief   Widget to control printer drivers
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Allows selecting drives for printers #4, #5 and #6.
 */

/*
 * $VICERES Printer4Driver  -vsid
 * $VICERES Printer5Driver  -vsid
 * $VICERES Printer6Driver  -vsid
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "widgethelpers.h"
#include "resourcehelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "printer.h"

#include "printerdriverwidget.h"


/** \brief  Handler for the "toggled" event of the radio buttons
 *
 * \param[in]   radio       radio button
 * \param[in]   user_data   new value for the resource (`const char *`)
 */
static void on_radio_toggled(GtkWidget *radio, gpointer user_data)
{

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
        int device;
        const char *type;

        /* get device number from the "DeviceNumber" property of the radio
         * button */
        device = resource_widget_get_int(radio, "DeviceNumber");
        type = (const char *)user_data;
        debug_gtk3("setting Printer%dDriver to '%s'\n", device, type);
        resources_set_string_sprintf("Printer%dDriver", type, device);
    }
}


/** \brief  Create printer driver selection widget
 *
 * Creates a group of radio buttons to select the driver of printer # \a device.
 * Uses a custom property "DeviceNumber" for the radio buttons and the widget
 * itself to pass the device number to the event handler and to allow
 * printer_driver_widget_update() to select the proper radio button index.
 *
 * Printer 4/5: [ascii, mps803, nl10, raw]
 * Printer 6  : [1520, raw]
 *
 * \param[in]   device  device number (4-6)
 *
 * \return  GtkGrid
 */
GtkWidget *printer_driver_widget_create(int device)
{
    GtkWidget *grid;
    GtkWidget *radio_ascii = NULL;
    GtkWidget *radio_mps803 = NULL;
    GtkWidget *radio_nl10 = NULL;
    GtkWidget *radio_raw = NULL;
    GtkWidget *radio_1520 = NULL;
    GSList *group = NULL;
    const char *driver;

    /* build grid */
    grid = uihelpers_create_grid_with_label("Driver", 1);
    /* set DeviceNumber property to allow the update function to work */
    resource_widget_set_int(grid, "DeviceNumber", device);

    if (device == 4 || device == 5) {
        /* 'normal' printers */

        /* ASCII */
        radio_ascii = gtk_radio_button_new_with_label(group, "ASCII");
        g_object_set_data(G_OBJECT(radio_ascii), "DeviceNumber",
                GINT_TO_POINTER(device));
        g_object_set(radio_ascii, "margin-left", 16, NULL);
        gtk_grid_attach(GTK_GRID(grid), radio_ascii, 0, 1, 1, 1);

        /* MPS803 */
        radio_mps803 = gtk_radio_button_new_with_label(group, "MPS-803");
        gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio_mps803),
                GTK_RADIO_BUTTON(radio_ascii));
        g_object_set_data(G_OBJECT(radio_mps803), "DeviceNumber",
                GINT_TO_POINTER(device));
        g_object_set(radio_mps803, "margin-left", 16, NULL);
        gtk_grid_attach(GTK_GRID(grid), radio_mps803, 0, 2, 1, 1);

        /* NL10 */
        radio_nl10 = gtk_radio_button_new_with_label(group, "NL10");
        gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio_nl10),
                GTK_RADIO_BUTTON(radio_mps803));
        g_object_set_data(G_OBJECT(radio_nl10), "DeviceNumber",
                GINT_TO_POINTER(device));
        g_object_set(radio_nl10, "margin-left", 16, NULL);
        gtk_grid_attach(GTK_GRID(grid), radio_nl10, 0, 3, 1, 1);

        /* RAW */
        radio_raw = gtk_radio_button_new_with_label(group, "RAW");
        gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio_raw),
                GTK_RADIO_BUTTON(radio_nl10));
        g_object_set_data(G_OBJECT(radio_raw), "DeviceNumber",
                GINT_TO_POINTER(device));
        g_object_set(radio_raw, "margin-left", 16, NULL);
        gtk_grid_attach(GTK_GRID(grid), radio_raw, 0, 4, 1, 1);
    } else if (device == 6) {
        /* plotter */

        /* 1520 */
        radio_1520 = gtk_radio_button_new_with_label(group, "1520");
        g_object_set_data(G_OBJECT(radio_1520), "DeviceNumber",
                GINT_TO_POINTER(device));
        g_object_set(radio_1520, "margin-left", 16, NULL);
        gtk_grid_attach(GTK_GRID(grid), radio_1520, 0, 1, 1, 1);

        /* RAW */
        radio_raw = gtk_radio_button_new_with_label(group, "RAW");
        gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio_raw),
                GTK_RADIO_BUTTON(radio_1520));
        g_object_set_data(G_OBJECT(radio_raw), "DeviceNumber",
                GINT_TO_POINTER(device));
        g_object_set(radio_raw, "margin-left", 16, NULL);
        gtk_grid_attach(GTK_GRID(grid), radio_raw, 0, 2, 1, 1);
    } else {
        fprintf(stderr, "%s:%d:%s(): invalid device #%d\n",
                __FILE__, __LINE__, __func__, device);
        exit(1);
    }


    /* set current driver from resource */
    resources_get_string_sprintf("Printer%dDriver", &driver, device);
    printer_driver_widget_update(grid, driver);

    /* connect signal handlers */
    g_signal_connect(radio_raw, "toggled", G_CALLBACK(on_radio_toggled),
            (gpointer)"raw");

    if (device == 4 || device == 5) {
        g_signal_connect(radio_ascii, "toggled", G_CALLBACK(on_radio_toggled),
                (gpointer)"ascii");
        g_signal_connect(radio_mps803, "toggled", G_CALLBACK(on_radio_toggled),
                (gpointer)"mps803");
        g_signal_connect(radio_nl10, "toggled", G_CALLBACK(on_radio_toggled),
                (gpointer)"nl10");
    } else if (device == 6) {
        g_signal_connect(radio_1520, "toggled", G_CALLBACK(on_radio_toggled),
                (gpointer)"1520");
    }

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Update the printer driver widget
 *
 * \param[in]   widget  printer driver widget
 * \param[in]   driver  driver name
 */
void printer_driver_widget_update(GtkWidget *widget, const char *driver)
{
    GtkWidget *radio;
    int index = 4;  /* RAW for 4/5 */
    int device;

    /* get device number from custom GObject property */
    device = GPOINTER_TO_INT(
            g_object_get_data(G_OBJECT(widget), "DeviceNumber"));

    /* this is a little silly, using string constants, but it works */
    if (device == 4 || device == 5) {
        if (strcmp(driver, "ascii") == 0) {
            index = 1;
        } else if (strcmp(driver, "mps803") == 0) {
            index = 2;
        } else if (strcmp(driver, "nl10") == 0) {
            index = 3;
        }
    } else if (device == 6) {
        if (strcmp(driver, "1520") == 0) {
            index = 1;
        } else {
            index = 2;  /* RAW */
        }
    } else {
        fprintf(stderr, "%s:%d:%s(): invalid printer device #%d\n",
                __FILE__, __LINE__, __func__, device);
        exit(1);
    }

    /* now select the proper radio button */
    radio = gtk_grid_get_child_at(GTK_GRID(widget), 0, index);
    if (radio != NULL && GTK_IS_RADIO_BUTTON(radio)) {
        /* set toggle button to active, this also sets the resource */
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
    }
}
