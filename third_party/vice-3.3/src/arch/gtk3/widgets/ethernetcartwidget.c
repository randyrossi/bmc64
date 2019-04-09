/** \file   ethernetcartwidget.c
 * \brief   Widget to control ethernet cartridge settings
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES ETHERNETCART_ACTIVE     x64 x64s xscpu64 x128 xvic
 * $VICERES ETHERNETCARTMode        x64 x64s xscpu64 x128
 * $VICERES ETHERNETCARTBase        x64 x64s xscpu64 x128 xvic
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
 * TODO: fix up the layout a bit
 */

#include "vice.h"
#include <gtk/gtk.h>
#include <stdlib.h>

#include "machine.h"
#include "resources.h"
#include "debug_gtk3.h"
#include "basewidgets.h"
#include "widgethelpers.h"
#include "basedialogs.h"
#include "cartridge.h"

#include "ethernetcartwidget.h"


/** \brief  List of Ethernet Cartridge emulation modes
 */
static const vice_gtk3_radiogroup_entry_t modes[] = {
    { "ETFE", 0 },
    { "RRNet", 1 },
    { NULL, -1 }
};




/** \brief  Handler for the "changed" event of the I/O base combo box
 *
 * \param[in]   widget      combo box
 * \param[in]   user_data   extra event data (unused)
 */
static void on_base_changed(GtkWidget *widget, gpointer user_data)
{
    int base;
    char *endptr;
    const gchar *id;

    id = gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget));
    base = (int)strtol(id, &endptr, 10);
    if (*endptr == '\0') {
        debug_gtk3("setting ETHERNETCARTBase to $%04X.", base);
        resources_set_int("ETHERNETCARTBase", base);
    }
}


/** \brief  Create widget to select the Ethernet Cartridge emulation mode
 *
 * \return  GtkGrid
 */
static GtkWidget *create_cartridge_mode_widget(void)
{
    GtkWidget *group;

    group = vice_gtk3_resource_radiogroup_new("ETHERNETCARTMode", modes,
            GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_set_column_spacing(GTK_GRID(group), 16);
    return group;
}


/** \brief  Create widget to create Ethernet Cartridge I/O base
 *
 * \return  GtkGrid
 */
static GtkWidget *create_cartridge_base_widget(void)
{
    GtkWidget *combo;
    unsigned int base;
    char text[256];
    char id[80];
    int current;
    int index;

    resources_get_int("ETHERNETCARTBase", &current);

    combo = gtk_combo_box_text_new();
    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fallthrough */
        case VICE_MACHINE_C64SC:    /* fallthrough */
        case VICE_MACHINE_C128:     /* fallthrough */
        case VICE_MACHINE_SCPU64:

            index = 0;
            for (base = 0xde00; base < 0xe000; base += 0x10) {
                g_snprintf(text, 256, "$%04X", base);
                g_snprintf(id, 80, "%u", base);
                gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), id, text);
                if (current == base) {
                    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), index);
                }
                index++;
            }
            break;

        case VICE_MACHINE_VIC20:

            index = 0;
            /* add range $9800-$98f0 */
            for (base = 0x9800; base < 0x9900; base += 0x10) {
                g_snprintf(text, 256, "$%04X", base);
                g_snprintf(id, 80, "%u", base);
                gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), id, text);
                if (current == base) {
                    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), index);
                }
                index++;
            }
            /* add range $9c00-$9cf0 */
            for (base = 0x9c00; base < 0x9d00; base += 0x10) {
                g_snprintf(text, 256, "$%04X", base);
                g_snprintf(id, 80, "%u", base);
                gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), id, text);
                if (current == base) {
                    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), index);
                }
                index++;
            }

            break;
        default:
            break;
    }
    g_signal_connect(combo, "changed", G_CALLBACK(on_base_changed), NULL);

    return combo;
}


/** \brief  Create widget to control generic Ethernet cartridget settings
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *ethernet_cart_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *enable_widget;
    GtkWidget *mode_widget;
    GtkWidget *mode_label;
    GtkWidget *base_widget;
    GtkWidget *base_label;
    int row;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    enable_widget = vice_gtk3_resource_check_button_new(
            "ETHERNETCART_ACTIVE", "Enable ethernet cartridge");
    gtk_grid_attach(GTK_GRID(grid), enable_widget, 0, 0, 1, 1);

    row = 1;    /* next row in grid */

    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fallthrough */
        case VICE_MACHINE_C64SC:    /* fallthrough */
        case VICE_MACHINE_C128:     /* fallthrough */
        case VICE_MACHINE_SCPU64:
            mode_widget = create_cartridge_mode_widget();
            mode_label = gtk_label_new("Ethernet Cartridge mode");
            g_object_set(mode_label, "margin-left", 16, NULL);
            gtk_widget_set_halign(mode_label, GTK_ALIGN_START);
            gtk_grid_attach(GTK_GRID(grid), mode_label, 0, row, 1, 1);
            gtk_grid_attach(GTK_GRID(grid), mode_widget, 1, row, 1, 1);
            row++;
            break;

        case VICE_MACHINE_VIC20:
            break;
        default:
            break;
    }

    base_label = gtk_label_new("Cartridge I/O base");
    gtk_widget_set_halign(base_label, GTK_ALIGN_START);
    g_object_set(base_label, "margin-left", 16, NULL);

    gtk_grid_attach(GTK_GRID(grid), base_label, 0, row, 1,1);
    base_widget = create_cartridge_base_widget();
    gtk_grid_attach(GTK_GRID(grid), base_widget, 1, row, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
