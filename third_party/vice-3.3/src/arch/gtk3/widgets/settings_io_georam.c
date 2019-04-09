/** \file   settings_io_georam.c
 * \brief   Widget to control GEO-RAM resources
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES GEORAM              x64 x64sc xscpu64 x128
 * $VICERES GEORAMsize          x64 x64sc xscpu64 x128
 * $VICERES GEORAMfilename      x64 x64sc xscpu64 x128
 * $VICERES GEORAMImageWrite    x64 x64sc xscpu64 x128
 * $VICERES GEORAMIOSwap        xvic
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

#include "machine.h"
#include "resources.h"
#include "debug_gtk3.h"
#include "basewidgets.h"
#include "widgethelpers.h"
#include "basedialogs.h"
#include "openfiledialog.h"
#include "savefiledialog.h"
#include "cartimagewidget.h"
#include "carthelpers.h"
#include "cartridge.h"
#include "resourcewidgetmanager.h"
#include "uisettings.h"

#include "settings_io_georam.h"

/** \brief  List of supported RAM sizes
 */
static const vice_gtk3_radiogroup_entry_t ram_sizes[] = {
    { "64KB",   64 },
    { "128KB",  128 },
    { "256KB",  256 },
    { "512KB",  512 },
    { "1MB",    1024 },
    { "2MB",    2048 },
    { "4MB",    4096 },
    { NULL,     -1 }
};


/** \brief  Resource widget manager instance
 */
static resource_widget_manager_t manager;


/** \brief  Clean up resources used by the main widget
 *
 * \param[in]   widget  main widget (unused)
 * \param[in]   data    extra event data (unused
 */
static void on_main_widget_destroy(GtkWidget *widget, gpointer data)
{
    vice_resource_widget_manager_exit(&manager);
}


/** \brief  Create IO-swap check button (seems to be valid for xvic only)
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_georam_ioswap_widget(void)
{
    GtkWidget *check;

    check = vice_gtk3_resource_check_button_new("GEORAMIOSwap",
            "MasC=uarade I/O swap");
    vice_resource_widget_manager_add_widget(&manager, check, NULL,
            NULL, NULL, NULL);
    return check;
}


/** \brief  Create radio button group to determine GEORAM RAM size
 *
 * \return  GtkGrid
 */
static GtkWidget *create_georam_size_widget(void)
{
    GtkWidget *grid;
    GtkWidget *radio_group;

    grid = uihelpers_create_grid_with_label("RAM Size", 1);
    radio_group = vice_gtk3_resource_radiogroup_new("GEORAMsize", ram_sizes,
            GTK_ORIENTATION_VERTICAL);
    vice_resource_widget_manager_add_widget(&manager, radio_group, NULL,
            NULL, NULL, NULL);
    g_object_set(radio_group, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to load/save GEORAM image file
 *
 * \return  GtkGrid
 */
static GtkWidget *create_georam_image_widget(GtkWidget *parent)
{
    /* TODO: add reset()/sync()/factory() methods to whatever is used here */
    return cart_image_widget_create(parent, "GEORAM Image",
            "GEORAMfilename", "GEORAMImageWrite",
            carthelpers_save_func, carthelpers_flush_func,
            CARTRIDGE_NAME_GEORAM, CARTRIDGE_GEORAM);
}


/** \brief  Create widget to control GEORAM resources
 *
 * \param[in]   parent  parent widget, used for dialogs
 *
 * \return  GtkGrid
 */
GtkWidget *settings_io_georam_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *georam_enable;
    GtkWidget *georam_size;
    GtkWidget *georam_ioswap;
    GtkWidget *georam_image;

    vice_resource_widget_manager_init(&manager);
    ui_settings_set_resource_widget_manager(&manager);

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    /*    georam_enable = create_georam_enable_widget(); */
    /* TODO: this also needs the sync()/reset()/factory() methods so it'll
     *       work with the resource manager widget
     */
    georam_enable = carthelpers_create_enable_check_button(
            CARTRIDGE_NAME_GEORAM, CARTRIDGE_GEORAM);
    gtk_grid_attach(GTK_GRID(grid), georam_enable, 0, 0, 1, 1);

    if (machine_class == VICE_MACHINE_VIC20) {
        georam_ioswap = create_georam_ioswap_widget();
        gtk_grid_attach(GTK_GRID(grid), georam_ioswap, 0, 2, 1, 1);
    }

    georam_size = create_georam_size_widget();
    gtk_grid_attach(GTK_GRID(grid), georam_size, 0, 1, 1, 1);

    georam_image = create_georam_image_widget(parent);
    gtk_grid_attach(GTK_GRID(grid), georam_image, 1, 1, 1, 1);


    g_signal_connect(grid, "destroy", G_CALLBACK(on_main_widget_destroy), NULL);

    gtk_widget_show_all(grid);
    return grid;
}
