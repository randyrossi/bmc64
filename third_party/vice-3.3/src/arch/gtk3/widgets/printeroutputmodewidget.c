/** \file   printeroutputmodewidget.c
 * \brief   Widget to control printer output mode settings
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES Printer4Output  -vsid
 * $VICERES Printer5Output  -vsid
 * $VICERES Printer6Output  -vsid
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

#include "basewidgets.h"
#include "resourcehelpers.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "printer.h"

#include "printeroutputmodewidget.h"


/** \brief  Handler for the "destroy" event of \a widget
 *
 * Frees memory used by the copy of the resource name.
 *
 * \param[in]   widget      widget
 * \param[in]   user_data   extra event data (unused)
 */
static void on_widget_destroy(GtkWidget *widget, gpointer user_data)
{
    resource_widget_free_resource_name(widget);
}


/** \brief  Handler for the "toggled" event of the radio buttons
 *
 * \param[in]   radio       radio button
 * \param[in]   user_data   new value for resource (`string`)
 */
static void on_radio_toggled(GtkWidget *radio, gpointer user_data)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio))) {
        GtkWidget *parent;
        const char *new_val;
        const char *old_val;
        const char *resource;

        parent = gtk_widget_get_parent(radio);
        resource = resource_widget_get_resource_name(parent);
        resources_get_string(resource, &old_val);
        new_val = (const char *)user_data;

        if (strcmp(new_val, old_val) == 0) {
            debug_gtk3("setting %s to '%s'.", resource, new_val);
            resources_set_string(resource, new_val);
        }
    }
}


/** \brief  Create widget to control Printer[device]TextDevice resource
 *
 * \param[in]   device  device number
 *
 * \return  GtkGrid
 */
GtkWidget *printer_output_mode_widget_create(int device)
{
    GtkWidget *grid;
    GtkWidget *radio_text;
    GtkWidget *radio_gfx;
    GSList *group = NULL;
    char resource[256];
    const char *value;

    /* can't use the resource base widgets here, since for some reason this
     * resource is a string with two possible values: "text" and "graphics"
     */

    grid = uihelpers_create_grid_with_label("Output mode", 1);

    g_snprintf(resource, 256, "Printer%dOutput", device);
    resource_widget_set_resource_name(grid, resource);

    radio_text = gtk_radio_button_new_with_label(group, "Text");
    g_object_set(radio_text, "margin-left", 16, NULL);
    radio_gfx = gtk_radio_button_new_with_label(group, "Graphics");
    g_object_set(radio_gfx, "margin-left", 16, NULL);
    gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio_gfx),
            GTK_RADIO_BUTTON(radio_text));

    resources_get_string(resource, &value);
    if (strcmp(value, "text") == 0) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_text), TRUE);
    } else {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio_gfx), TRUE);
    }

    gtk_grid_attach(GTK_GRID(grid), radio_text, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio_gfx, 0, 2, 1, 1);

    g_signal_connect(radio_text, "toggled", G_CALLBACK(on_radio_toggled),
            (gpointer)"text");
    g_signal_connect(radio_gfx, "toggled", G_CALLBACK(on_radio_toggled),
            (gpointer)"graphics");

    g_signal_connect(grid, "destroy", G_CALLBACK(on_widget_destroy), NULL);

    gtk_widget_show_all(grid);
    return grid;
}
