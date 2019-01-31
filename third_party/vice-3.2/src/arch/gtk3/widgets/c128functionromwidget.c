/** \file   c128functionromwidget.c
 * \brief   Widget to control C128 function roms
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES InternalFunctionROM         x128
 * $VICERES InternalFunctionName        x128
 * $VICERES InternalFunctionROMRTCSave  x128
 * $VICERES ExternalFunctionROM         x128
 * $VICERES ExternalFunctionName        x128
 * $VICERES ExternalFunctionROMRTCSave  x128
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

#include "debug_gtk3.h"
#include "basewidgets.h"
#include "widgethelpers.h"
#include "openfiledialog.h"
#include "functionrom.h"

#include "c128functionromwidget.h"


/** \brief  List of possible ROM bla things
 *
 * Seems to be the same for ext ROMS
 */
static const vice_gtk3_radiogroup_entry_t rom_types[] = {
    { "None",   INT_FUNCTION_NONE },    /* this one probably requires the
                                           text entry/browse button to be
                                           disabled */
    { "ROM",    INT_FUNCTION_ROM },
    { "RAM",    INT_FUNCTION_RAM },
    { "RTC",    INT_FUNCTION_RTC },
    { NULL, - 1 },
};


/** \brief  Handler for the "clicked" event of a "browse" button
 *
 * \param[in]   widget  browse button
 * \param[in]   data    entry to store filename
 */
static void on_browse_clicked(GtkWidget *widget, gpointer data)
{
    gchar *filename;

    filename = vice_gtk3_open_file_dialog("Open ROM file", NULL, NULL, NULL);
    if (filename != NULL) {
        debug_gtk3("got filename '%s'\n", filename);
        vice_gtk3_resource_entry_full_set(GTK_WIDGET(data), filename);
        g_free(filename);
    }
}


/** \brief  Create ROM type widget
 *
 * \param[in]   prefix  resource prefix
 *
 * \return  GtkGrid
 */
static GtkWidget *create_rom_type_widget(const char *prefix)
{
    GtkWidget *widget;


    widget = vice_gtk3_resource_radiogroup_new_sprintf("%sFunctionROM",
            rom_types, GTK_ORIENTATION_HORIZONTAL, prefix);
    gtk_grid_set_column_spacing(GTK_GRID(widget), 16);
    return widget;
}


/** \brief  Create ROM file selection widget
 *
 * \param[in]   prefix  resource prefix
 *
 * \return  GtkGrid
 */
static GtkWidget *create_rom_file_widget(const char *prefix)
{
    GtkWidget *grid;
    GtkWidget *entry;
    GtkWidget *browse;
    char buffer[256];

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    /* TODO: create vice_gtk3_resource_entry_create_sprintf() */
    g_snprintf(buffer, 256, "%sFunctionName", prefix);

    entry = vice_gtk3_resource_entry_full_new(buffer);
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 1, 1);

    browse = gtk_button_new_with_label("Browse ...");
    gtk_grid_attach(GTK_GRID(grid), browse, 1, 0, 1,1);
    g_signal_connect(browse, "clicked", G_CALLBACK(on_browse_clicked),
            (gpointer)entry);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create External/Internal ROM widget
 *
 * \param[in]   parent  parent widget
 * \param[in]   prefix  resource prefix
 *
 * \return GtkGrid
 */
static GtkWidget *create_rom_widget(GtkWidget *parent, const char *prefix)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *rtc;
    char buffer[256];

    g_snprintf(buffer, 256, "%s Function ROM", prefix);
    grid = uihelpers_create_grid_with_label(buffer, 1);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    gtk_widget_show_all(grid);

    label = gtk_label_new("ROM type");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), create_rom_type_widget(prefix), 1, 1, 1, 1);

    label = gtk_label_new("ROM file");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), create_rom_file_widget(prefix), 1, 2, 1, 1);

    rtc = vice_gtk3_resource_check_button_new_sprintf("%sFunctionROMRTCSave",
            "Save RTC data", prefix);
    g_object_set(rtc, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), rtc, 0, 3, 3, 1);

    return grid;
}


/** \brief  Create widget to select Internal/External function ROMs
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *c128_function_rom_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;

    grid = gtk_grid_new();

    gtk_grid_attach(GTK_GRID(grid), create_rom_widget(parent, "Internal"),
            0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), create_rom_widget(parent, "External"),
            0, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
