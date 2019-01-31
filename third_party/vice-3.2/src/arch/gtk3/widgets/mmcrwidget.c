/** \file   mmcrwidget.c
 * \brief   Widget to control MMC Replay resources
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES MMCRCardImage   x64 x64sc xscpu64 x128
 * $VICERES MMCREEPROMImage x64 x64sc xscpu64 x128
 * $VICERES MMCREEPROMRW    x64 x64sc xscpu64 x128
 * $VICERES MMCRRescueMode  x64 x64sc xscpu64 x128
 * $VICERES MMCRImageWrite  x64 x64sc xscpu64 x128
 * $VICERES MMCRCardRW      x64 x64sc xscpu64 x128
 * $VICERES MMCRSDType      x64 x64sc xscpu64 x128
 * $VICERES MMCRClockPort   x64 x64sc xscpu64 x128
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
#include "cartridge.h"
#include "carthelpers.h"

#include "mmcrwidget.h"


/** \brief  List of memory card types
 */
static const vice_gtk3_radiogroup_entry_t card_types[] = {
    { "Auto", 0 },
    { "MMC", 1 },
    { "SD", 2 },
    { "SDHC", 3 },
    { NULL, -1 }
};


/** \brief  Handler for the "clicked" event of the memory card browse button
 *
 * \param[in]   button      browse button
 * \param[in]   user_data   unused
 */
static void on_card_browse_clicked(GtkWidget *button, gpointer user_data)
{
    char *filename;

    filename = vice_gtk3_open_file_dialog("Open memory card file",
            NULL, NULL, NULL);
    if (filename != NULL) {
        GtkWidget *parent;
        GtkWidget *entry;

        parent = gtk_widget_get_parent(button);
        entry= gtk_grid_get_child_at(GTK_GRID(parent), 1, 1);
        /* trigger resource update */
        vice_gtk3_resource_entry_full_set(entry, filename);
        g_free(filename);
    }
}



/** \brief  Create check button for the MMCRRescueMode resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_rescue_mode_widget(void)
{
    return vice_gtk3_resource_check_button_new(
            "MMCRRescueMode", "Enable rescue mode");
}


/** \brief  Create widget for the MMCRClockPort resource
 *
 * \return  GtkComboBoxText
 */
static GtkWidget *create_clockport_widget(void)
{
    return clockport_device_widget_create("MMCRClockPort");
}


/** \brief  Create widget for the MMCREEPROMRW resource
 *
 * \return  GtkComboBoxText
 */
static GtkWidget *create_eeprom_rw_widget(void)
{
    return vice_gtk3_resource_check_button_new("MMCREEPROMRW",
            "Enable writes to EEPROM image");
}


/** \brief  Create widget to control EEPROM resources
 *
 * \param[in]   parent  parent widget (used for dialogs)
 *
 * \return  GtkGrid
 */
static GtkWidget *create_eeprom_image_widget(GtkWidget *parent)
{
    GtkWidget *widget;
    GtkWidget *readwrite;

    widget = cart_image_widget_create(parent, "MMC Replay EEPROM image",
            "MMCREEPROMImage", "MMCRImageWrite",
            carthelpers_save_func, carthelpers_flush_func,
            CARTRIDGE_NAME_MMC_REPLAY, CARTRIDGE_MMC_REPLAY);

    /* add RW widget */
    readwrite = create_eeprom_rw_widget();
    g_object_set(readwrite, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(widget), readwrite, 0, 3, 2, 1);

    gtk_widget_show_all(widget);
    return widget;

}




/** \brief  Create widget to control memory card image
 *
 * \param[in]   widget  parent widget (used for dialogs
 *
 * \return  GtkGrid
 */
static GtkWidget *create_card_image_widget(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *browse;
    GtkWidget *card_writes;

    grid = uihelpers_create_grid_with_label("MMC Replay SD/MMC Card image", 3);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);

    label = gtk_label_new("file name");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);

    entry = vice_gtk3_resource_entry_full_new("MMCRCardImage");
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 1, 1, 1);

    browse = gtk_button_new_with_label("Browse ...");
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);

    card_writes = vice_gtk3_resource_check_button_new("MMCRCardRW",
            "Enable SD/MMC card writes");
    g_object_set(card_writes, "margin-left", 16, "margin-top", 8, NULL);
    gtk_grid_attach(GTK_GRID(grid), card_writes, 0, 2, 3, 1);

    g_signal_connect(browse, "clicked", G_CALLBACK(on_card_browse_clicked),
            NULL);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to control memory card type
 *
 * \return  GtkGrid
 */
static GtkWidget *create_card_type_widget(void)
{
    GtkWidget *grid;
    GtkWidget *radio_group;
    GtkWidget *label;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    label = gtk_label_new("Card type");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    radio_group = vice_gtk3_resource_radiogroup_new("MMCRSDType", card_types,
            GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_set_column_spacing(GTK_GRID(radio_group), 16);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 1, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}



/** \brief  Create widget to control MMC Replay resources
 *
 * \param[in]   parent  parent widget (used for dialogs)
 *
 * \return  GtkGrid
 */
GtkWidget *mmcr_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *label;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    gtk_grid_attach(GTK_GRID(grid), create_rescue_mode_widget(), 0, 0, 1, 1);
    label = gtk_label_new("ClockPort device");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), create_clockport_widget(), 2, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), create_eeprom_image_widget(parent),
            0, 1, 3, 1);

    gtk_grid_attach(GTK_GRID(grid), create_card_image_widget(parent),
            0, 2, 3, 1);

    gtk_grid_attach(GTK_GRID(grid), create_card_type_widget(), 0, 3, 3, 1);

    gtk_widget_show_all(grid);
    return grid;
}
