/** \file   settings_io.c
 * \brief   I/O extension settings that don't go/fit into subwidgets
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES IOCollisionHandling -vsid
 * $VICERES CartridgeReset      -vsid
 * $VICERES HummerADC           x64dtv
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
#include "cartio.h"
#include "cartridge.h"

#include "settings_io.h"


/** \brief  Methods of handling I/O collisions
 */
static const vice_gtk3_radiogroup_entry_t io_collision_methods[] = {
    { "Detach all", IO_COLLISION_METHOD_DETACH_ALL },
    { "Detach last", IO_COLLISION_METHOD_DETACH_LAST },
    { "AND values", IO_COLLISION_METHOD_AND_WIRES },
    { NULL, -1 }
};


/** \brief  Create widget to specify I/O collision handling method
 *
 * The \a desc is added as a label under the "I/O collision handling" label,
 * in italics and surrounded with parenthesis.
 *
 * \param[in]   desc    description of the I/O areas involved
 *
 * \return  GtkGrid
 */
static GtkWidget *create_collision_widget(const char *desc)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *group;
    char buffer[256];

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 0);

    label = gtk_label_new("I/O collision handling");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    group = vice_gtk3_resource_radiogroup_new("IOCollisionHandling",
            io_collision_methods, GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_set_column_spacing(GTK_GRID(group), 16);
    gtk_grid_attach(GTK_GRID(grid), group, 1, 0, 1, 1);

    label = gtk_label_new(NULL);
    g_snprintf(buffer, 256, "<i>(%s)</i>", desc);
    gtk_label_set_markup(GTK_LABEL(label), buffer);

    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);


    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create check button for "reset-on-cart-change"
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_cart_reset_widget(void)
{
    GtkWidget *check;

    check = vice_gtk3_resource_check_button_new("CartridgeReset",
            "Reset machine on cartridge change");
    g_object_set(check, "margin-left", 16, NULL);
    return check;
}


/** \brief  Create layout for x64/x64sc
 *
 * \param[in,out]   grid    grid to add widgets to
 */
static void create_c64_layout(GtkWidget *grid)
{
    GtkWidget *collision_widget;

    collision_widget = create_collision_widget("$D000-$DFFF");
    gtk_grid_attach(GTK_GRID(grid), collision_widget, 0, 1, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), create_cart_reset_widget(), 0, 2, 3, 1);
}


/** \brief  Create layout for xscpu64
 *
 * \param[in,out]   grid    grid to add widgets to
 */
static void create_scpu64_layout(GtkWidget *grid)
{
    GtkWidget *collision_widget;

    collision_widget = create_collision_widget("$D000-$DFFF");
    gtk_grid_attach(GTK_GRID(grid), collision_widget, 0, 1, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), create_cart_reset_widget(), 0, 2, 3, 1);
}


/** \brief  Create layout for x128
 *
 * \param[in,out]   grid    grid to add widgets to
 */
static void create_c128_layout(GtkWidget *grid)
{
    GtkWidget *collision_widget;

    collision_widget = create_collision_widget("$D000-$DFFF");
    gtk_grid_attach(GTK_GRID(grid), collision_widget, 0, 1, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), create_cart_reset_widget(), 0, 2, 3, 1);
}



/** \brief  Create layout for x64dtv
 *
 * \param[in,out]   grid    grid to add widgets to
 */
static void create_c64dtv_layout(GtkWidget *grid)
{
    GtkWidget *hummer;

    hummer = vice_gtk3_resource_check_button_new("HummerADC",
            "Enable Hummer ADC");
    g_object_set(hummer, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), hummer, 0, 1, 3, 1);
}


/** \brief  Create layout for xvic
 *
 * \param[in,out]   grid    grid to add widgets to
 */
static void create_vic20_layout(GtkWidget *grid)
{
    GtkWidget *collision_widget;

    collision_widget = create_collision_widget("$9000-$93FF, $9800-$9FFF");
    gtk_grid_attach(GTK_GRID(grid), collision_widget, 0, 1, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), create_cart_reset_widget(), 0, 2, 3, 1);
}


/** \brief  Create layout for xplus4
 *
 * \param[in,out]   grid    grid to add widgets to
 */
static void create_plus4_layout(GtkWidget *grid)
{
    GtkWidget *collision_widget;

    collision_widget = create_collision_widget("$FD00-$FEFF");
    gtk_grid_attach(GTK_GRID(grid), collision_widget, 0, 1, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), create_cart_reset_widget(), 0, 2, 3, 1);
}


/** \brief  Create layout for xpet
 *
 * \param[in,out]   grid    grid to add widgets to
 */
static void create_pet_layout(GtkWidget *grid)
{
    GtkWidget *collision_widget;

    collision_widget = create_collision_widget("$8800-$8FFF, $E900-$EEFF");
    gtk_grid_attach(GTK_GRID(grid), collision_widget, 0, 1, 3, 1);
}


/** \brief  Create layout for xcbm5x0/xcbm2
 *
 * \param[in,out]   grid    grid to add widgets to
 */
static void create_cbm5x0_layout(GtkWidget *grid)
{
    GtkWidget *collision_widget;

    collision_widget = create_collision_widget("$D800-$DFFF");
    gtk_grid_attach(GTK_GRID(grid), collision_widget, 0, 1, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), create_cart_reset_widget(), 0, 2, 3, 1);
}



/** \brief  Create widget for generic I/O extension settings
 *
 * Creates a widget for generic I/O extension settings and settings that don't
 * make sense to create a separate widget for (ie stuff that has a single
 * bool resource or a single "open file" dialog").
 *
 * \param[in]   parent  parent widget
 *
 * \return GtkGrid
 */
GtkWidget *settings_io_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;

    grid = uihelpers_create_grid_with_label(
            "Generic I/O extension settings", 3);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 16);

    switch (machine_class) {

        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:
            create_c64_layout(grid);
            break;

        case VICE_MACHINE_SCPU64:
            create_scpu64_layout(grid);
            break;

        case VICE_MACHINE_C128:
            create_c128_layout(grid);
            break;

        case VICE_MACHINE_C64DTV:
            create_c64dtv_layout(grid);
            break;

        case VICE_MACHINE_VIC20:
            create_vic20_layout(grid);
            break;

        case VICE_MACHINE_PLUS4:
            create_plus4_layout(grid);
            break;

        case VICE_MACHINE_PET:
            create_pet_layout(grid);
            break;

        case VICE_MACHINE_CBM5x0:       /* fall through */
        case VICE_MACHINE_CBM6x0:
            create_cbm5x0_layout(grid);
            break;

        default:
            /* NOP */
            break;
    }

    gtk_widget_show_all(grid);
    return grid;
}
