/** \file   sidcartwidget.c
 * \brief   SID Cartidge widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES SidCart     xvic xplus4 xpet
 * $VICERES SidAddress  xvic xplus4 xpet
 * $VICERES SidClock    xvic xplus4 xpet
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

#include "archdep.h"
#include "basewidgets.h"
#include "debug_gtk3.h"
#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "sidmodelwidget.h"
#include "widgethelpers.h"

#include "sidcartwidget.h"


/* References to widgets to enable/disable depending on the "SidCart" resource
 */
static GtkWidget *sidcart_enable = NULL;
static GtkWidget *sid_model = NULL;
static GtkWidget *sid_address = NULL;
static GtkWidget *sid_clock = NULL;



/** \brief  SID cart I/O base addresses for VIC-20
 */
static const vice_gtk3_radiogroup_entry_t sid_base_vic20[] = {
    { "$9800", 0x9800 },
    { "$9C00", 0x9c00 },
    { NULL, -1 }
};


/** \brief  SID cart I/O base addresses for Plus4
 */
static const vice_gtk3_radiogroup_entry_t sid_base_plus4[] = {
    { "$FD40", 0xfd40 },
    { "$FE80", 0xfe80 },
    { NULL, -1 }
};


/** \brief  SID cart I/O base addresses for PET
 */
static const vice_gtk3_radiogroup_entry_t sid_base_pet[] = {
    { "$8F00", 0x8f00 },
    { "$E900", 0xe900 },
    { NULL, -1 }
};


/** \brief  SID cart clock for VIC-20
 */
static const vice_gtk3_radiogroup_entry_t sid_clock_vic20[] = {
    { "C64", 0 },
    { "VIC-20", 1 },
    { NULL, -1 }
};


/** \brief  SID cart clock for Plus4
 */
static const vice_gtk3_radiogroup_entry_t sid_clock_plus4[] = {
    { "C64", 0 },
    { "Plus4", 1 },
    { NULL, -1 }
};


/** \brief  SID cart clock for PET
 */
static const vice_gtk3_radiogroup_entry_t sid_clock_pet[] = {
    { "C64", 0 },
    { "PET", 1 },
    { NULL, -1 }
};


/** \brief  Handler for the "toggled" event of the SidCart enable widget
 *
 * Enables/disables the model, address and clock widgets depending on the
 * SidCart enabled state.
 *
 * \param[in]   widget      SidCart toggle button
 * \param[in]   user_data   unused
 */
static void on_sidcart_enable_toggled(GtkWidget *widget, gpointer user_data)
{
    int state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    gtk_widget_set_sensitive(sid_model, state);
    gtk_widget_set_sensitive(sid_address, state);
    gtk_widget_set_sensitive(sid_clock, state);
}


/** \brief  Create toggle button to switch the "SidCart" resource
 *
 * \return  GtkGrid
 */
static GtkWidget *create_sidcart_enable_widget(void)
{
    return vice_gtk3_resource_check_button_new(
            "SidCart", "Enable SID cartridge");
}


/** \brief  Create widget to set SID I/O base address
 *
 * \return  GtkGrid
 */
static GtkWidget *create_sidcart_address_widget(void)
{
    GtkWidget *grid;
    GtkWidget *group;
    const vice_gtk3_radiogroup_entry_t *list;

    switch (machine_class) {
        case VICE_MACHINE_VIC20:
            list = sid_base_vic20;
            break;
        case VICE_MACHINE_PLUS4:
            list = sid_base_plus4;
            break;
        case VICE_MACHINE_PET:
            list = sid_base_pet;
            break;
        default:
            list = NULL;
            archdep_vice_exit(1);
            break;
    }

    grid = uihelpers_create_grid_with_label("SID address", 1);

    group = vice_gtk3_resource_radiogroup_new("SidAddress", list,
            GTK_ORIENTATION_VERTICAL);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to set SID clock
 *
 * \return  GtkGrid
 */
static GtkWidget *create_sidcart_clock_widget(void)
{
    GtkWidget *grid;
    GtkWidget *group;
    const vice_gtk3_radiogroup_entry_t *list;

    switch (machine_class) {
        case VICE_MACHINE_VIC20:
            list = sid_clock_vic20;
            break;
        case VICE_MACHINE_PLUS4:
            list = sid_clock_plus4;
            break;
        case VICE_MACHINE_PET:
            list = sid_clock_pet;
            break;
        default:
            list = NULL;
            archdep_vice_exit(1);
            break;
    }

    grid = uihelpers_create_grid_with_label("SID clock", 1);

    group = vice_gtk3_resource_radiogroup_new("SidClock", list,
            GTK_ORIENTATION_VERTICAL);
    gtk_grid_attach(GTK_GRID(grid), group, 0, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}



/** \brief  Create widget to conrol SID cartridge settings
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *sidcart_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    sidcart_enable = create_sidcart_enable_widget();
    gtk_grid_attach(GTK_GRID(grid), sidcart_enable, 0, 0, 3, 1);

    sid_model = sid_model_widget_create(NULL);
    gtk_grid_attach(GTK_GRID(grid), sid_model, 0, 1, 1, 1);

    sid_address = create_sidcart_address_widget();
    gtk_grid_attach(GTK_GRID(grid), sid_address, 1, 1, 1, 1);

    sid_clock = create_sidcart_clock_widget();
    gtk_grid_attach(GTK_GRID(grid), sid_clock, 2, 1, 1, 1);

    g_signal_connect(sidcart_enable, "toggled",
            G_CALLBACK(on_sidcart_enable_toggled), NULL);

    /* initialize senstitive state of widgets */
    on_sidcart_enable_toggled(sidcart_enable, NULL);

    gtk_widget_show_all(grid);
    return grid;
}
