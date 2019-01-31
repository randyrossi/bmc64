/** \file   settings_monitor.c
 * \brief   GTK3 monitor setting dialog
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES KeepMonitorOpen         all
 * $VICERES MonitorServer           all
 * $VICERES MonitorServerAddress    all
 * $VICERES NativeMonitor           all
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

#include "vice_gtk3.h"

#include "settings_monitor.h"


/** \brief  Create widget to control monitor resources
 *
 * \param[in]   parent  parent widget (unused)
 *
 * \return  GtkGrid
 */
GtkWidget *settings_monitor_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *native;
    GtkWidget *keep_open;
    GtkWidget *server_enable;
    GtkWidget *server_address;
    GtkWidget *label;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    native = vice_gtk3_resource_check_button_new("NativeMonitor",
            "Use native monitor interface");
    keep_open = vice_gtk3_resource_check_button_new("KeepMonitorOpen",
            "Keep monitor open");
    server_enable = vice_gtk3_resource_check_button_new("MonitorServer",
            "Enable remote monitor");
    label = gtk_label_new("Server address");
    /* align with the rest, more or less */
    g_object_set(label, "margin-left", 8, NULL);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    server_address = vice_gtk3_resource_entry_full_new(
            "MonitorServerAddress");
    gtk_widget_set_hexpand(server_address, TRUE);
    gtk_grid_attach(GTK_GRID(grid), native, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), keep_open, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), server_enable, 0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), server_address, 1, 3, 1, 1);


    gtk_widget_show_all(grid);
    return grid;
}
