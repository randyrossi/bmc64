/** \file   userportprinterwidget.c
 * \brief   Widget to control userport printer
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/* FIXME:   Not sure the following is correct:
 *
 * $VICERES PrinterUserPort             -vsid
 * $VICERES PrinterUserportDriver       -vsid
 * $VICERES PrinterUserportOutput       -vsid
 * $VICERES PrinterUserportTextDevice   -vsid
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

#include "resourcecheckbutton.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "printer.h"

#include "userportprinterwidget.h"


/** \brief  List of text output devices
 */
static const vice_gtk3_radiogroup_entry_t text_devices[] = {
    { "#1 (file dump", 0 },
    { "#2 (exec)", 1 },
    { "#3 (exec)", 2 },
    { NULL, -1 }
};


/** \brief  Handler for the "toggled" event of the driver radio buttons
 *
 * \param[in]   radio       radio button
 * \param[in]   user_data   new value for the resource (`const char *`)
 */
static void on_driver_toggled(GtkRadioButton *radio, gpointer user_data)
{

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
        const char *driver = (const char *)user_data;

        debug_gtk3("setting 'PrinterUserportDriver' to '%s'.", driver);
        resources_set_string("PrinterUserportDriver", driver);
    }
}


/** \brief  Handler for the "toggled" event of the output mode widget
 *
 * \param[in]   radio       radio button
 * \param[in]   user_data   new value for the resource (`const char *`)
 */
static void on_output_mode_toggled(GtkRadioButton *radio, gpointer user_data)
{

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
        const char *mode = (const char *)user_data;

        debug_gtk3("setting 'PrinterUserportOutput' to '%s'.", mode);
        resources_set_string("PrinterUserportOutput", mode);
    }
}


/** \brief  Handler for the "toggled" event of the text device widget
 *
 * \param[in]   radio       radio button
 * \param[in]   user_data   new value for the resource (`int`)
 */
static void on_text_device_toggled(GtkWidget *radio, gpointer user_data)
{

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
        int device = GPOINTER_TO_INT(user_data);

        debug_gtk3("setting 'PrinterUserportTextDevice' to %d.", device);
        resources_set_int("PrinterUserportTextDevice", device);
    }
}


/** \brief  Create checkbox to control the PrinterUserport resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_userport_emulation_widget(void)
{
    GtkWidget *check;

    check = vice_gtk3_resource_check_button_new("PrinterUserPort",
            "Enable userport printer emulation");
    g_object_set(check, "margin-left", 16, NULL);
    return check;
}


/** \brief  Create printer driver selection widget
 *
 * Creates a group of radio buttons to select the driver of the Userport
 * printer.
 *
 * \return  GtkGrid
 */
static GtkWidget *create_driver_widget(void)
{
    GtkWidget *grid;
    GtkWidget *radio_ascii = NULL;
    GtkWidget *radio_nl10 = NULL;
    GtkWidget *radio_raw = NULL;
    GSList *group = NULL;
    const char *driver;

    /* build grid */
    grid = uihelpers_create_grid_with_label("Driver", 1);
    /* set DeviceNumber property to allow the update function to work */

    /* ASCII */
    radio_ascii = gtk_radio_button_new_with_label(group, "ASCII");
    g_object_set(radio_ascii, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_ascii, 0, 1, 1, 1);

    /* NL10 */
    radio_nl10 = gtk_radio_button_new_with_label(group, "NL10");
    gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio_nl10),
            GTK_RADIO_BUTTON(radio_ascii));
    g_object_set(radio_nl10, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_nl10, 0, 3, 1, 1);

    /* RAW */
    radio_raw = gtk_radio_button_new_with_label(group, "RAW");
    gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio_raw),
            GTK_RADIO_BUTTON(radio_nl10));
    g_object_set(radio_raw, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_raw, 0, 4, 1, 1);

    /* set current driver from resource */
    resources_get_string("PrinterUserPortDriver", &driver);
    /* printer_driver_widget_update(grid, driver); */

    /* connect signal handlers */
    g_signal_connect(radio_raw, "toggled", G_CALLBACK(on_driver_toggled),
            (gpointer)"raw");
    g_signal_connect(radio_ascii, "toggled", G_CALLBACK(on_driver_toggled),
            (gpointer)"ascii");
    g_signal_connect(radio_nl10, "toggled", G_CALLBACK(on_driver_toggled),
            (gpointer)"nl10");

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create userport printer output mode widget
 *
 * \return  GtkGrid
 */
static GtkWidget *create_output_mode_widget(void)
{
    GtkWidget *grid;
    GtkWidget *radio_text;
    GtkWidget *radio_gfx;
    GSList *group = NULL;

    grid = uihelpers_create_grid_with_label("Output mode", 1);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    radio_text = gtk_radio_button_new_with_label(group, "Text");
    g_object_set(radio_text, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_text, 0, 1, 1, 1);

    radio_gfx = gtk_radio_button_new_with_label(group, "Graphics");
    g_object_set(radio_gfx, "margin-left", 16, NULL);
    gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio_gfx),
            GTK_RADIO_BUTTON(radio_text));
    gtk_grid_attach(GTK_GRID(grid), radio_gfx, 0, 2, 1, 1);


    g_signal_connect(radio_text, "toggled", G_CALLBACK(on_output_mode_toggled),
            (gpointer)"text");
    g_signal_connect(radio_gfx, "toggled", G_CALLBACK(on_output_mode_toggled),
            (gpointer)"graphics");

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create text output device selection widget for Userport printer
 *
 * \return  GtkGrid
 */
static GtkWidget *create_text_device_widget(void)
{
    GtkWidget *grid;
    int current;

    resources_get_int("PrinterUserportTextDevice", &current);

    grid = uihelpers_radiogroup_create("Text output device",
            text_devices, on_text_device_toggled, current);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to control Userport printer settings
 *
 * \return  GtkGrid
 */
GtkWidget *userport_printer_widget_create(void)
{
    GtkWidget *grid;

    grid = uihelpers_create_grid_with_label("Userport printer settings", 3);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    gtk_grid_attach(GTK_GRID(grid), create_userport_emulation_widget(),
            0, 1, 3, 1);

    gtk_grid_attach(GTK_GRID(grid), create_driver_widget(),
            0, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), create_output_mode_widget(),
            1, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), create_text_device_widget(),
            2, 2,1,1);

    gtk_widget_show_all(grid);
    return grid;
}


void userport_printer_widget_update(void)
{
}
