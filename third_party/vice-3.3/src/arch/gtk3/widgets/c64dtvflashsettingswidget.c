/** \file   c64dtvflashsettingswidget.c
 * \brief   Widget controlling C64DTV Flash settings
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES c64dtvromfilename   x64dtv
 * $VICERES c64dtvromrw         x64dtv
 * $VICERES FSFlashDir          x64dtv
 * $VICERES FlashTrueFS         x64dtv
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
#include "resources.h"

#include "c64dtvflashsettingswidget.h"


/** \brief  Handler for the 'clicked' event of the Flash FS browser button
 *
 * Opens a directory selection/creating dialog and sets the resource
 * 'FSFlashDir' trough the GtkEntry \a data.
 *
 * \param[in]       widget  browse button
 * \param[in,out]   data    reference to the GtkEntry controlling the resource
 */
static void on_flash_dir_browse_clicked(GtkWidget *widget, gpointer data)
{
    GtkWidget *entry = GTK_WIDGET(data);
    gchar *filename;

    filename = vice_gtk3_select_directory_dialog(
            "Select Flash filesystem directory", NULL, TRUE, NULL);
    if (filename != NULL) {
        vice_gtk3_resource_entry_full_set(entry, filename);
        g_free(filename);
    }
}


/** \brief  Create DTV ROM selection widget
 *
 * \return  GtkGrid
 */
static GtkWidget *create_rom_widget(void)
{
    GtkWidget *browser;
    const char *patterns[] = { "*.bin", NULL };

    browser = vice_gtk3_resource_browser_new("c64dtvromfilename", patterns,
            "C64DTV ROMs", "Select C64DTV ROM", NULL, NULL);
    return browser;
}


/** \brief  Create entry and browse button to control 'FSFlashDir'
 *
 * \return  GtkGrid
 */
static GtkWidget *create_flash_dir_widget(void)
{
    GtkWidget *grid;
    GtkWidget *entry;
    GtkWidget *browse;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    entry = vice_gtk3_resource_entry_full_new("FSFlashDir");
    gtk_widget_set_hexpand(entry, TRUE);
    browse = gtk_button_new_with_label("Browse ...");

    gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 1, 0, 1, 1);

    g_signal_connect(browse, "clicked", G_CALLBACK(on_flash_dir_browse_clicked),
            (gpointer)entry);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create C64DTV Flash settings widget
 *
 * \param[in]   parent  parent widget (ignored)
 *
 * \return  GtkGrid
 */
GtkWidget *c64dtv_flash_settings_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *rom_file_widget;
    GtkWidget *rom_write_widget;
    GtkWidget *flash_dir_widget;
    GtkWidget *flash_hw_widget;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    /* DTV ROM browser */
    label = gtk_label_new("C64DTV ROM file");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    rom_file_widget = create_rom_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), rom_file_widget, 1, 0, 1, 1);

    /* DTV ROM R/W widget */
    rom_write_widget = vice_gtk3_resource_check_button_new("c64dtvromrw",
            "Enable writes to C64DTV ROM image");
    gtk_grid_attach(GTK_GRID(grid), rom_write_widget, 1, 1, 1,1);

    /* Flash dir widget */
    label = gtk_label_new("Flash FS directory");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    flash_dir_widget = create_flash_dir_widget();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), flash_dir_widget, 1, 2, 1, 1);

    /* Flash true hardware flash file system widget */
    flash_hw_widget = vice_gtk3_resource_check_button_new("FlashTrueFS",
            "Enable true hardware flash file system");
    gtk_grid_attach(GTK_GRID(grid), flash_hw_widget, 1, 3, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
