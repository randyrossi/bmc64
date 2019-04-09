/**
 * \brief   Widget to control ISEPIC resources
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES IsepicCartridgeEnabled  x64 x64sc xscpu64 x128
 * $VICERES Isepicfilename          x64 x64sc xscpu64 x128
 * $VICERES IsepicSwitch            x64 x64sc xscpu64 x128
 * $VICERES IsepicImageWrite        x64 x64sc xscpu64 x128
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

#include "isepicwidget.h"


/** \brief  Handler for the "state-set" event of the "IsepicSwitch" resource
 *
 * \param[in]   widget      switch widget
 * \param[in]   state       new state of \a widget
 * \param[in]   user_data   unused
 *
 * \return  FALSE
 */
static gboolean on_isepic_switch_state_set(GtkWidget *widget, gboolean state,
        gpointer user_data)
{
    debug_gtk3("setting IsepicSwitch to %s.", state ? "ON" : "OFF");
    resources_set_int("IsepicSwitch", state);
    return FALSE;
}


/** \brief  Create ISEPIC switch button
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_isepic_switch_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *button;
    int state;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    g_object_set(grid, "margin-left", 16, NULL);

    label = gtk_label_new("Isepic switch");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    button = gtk_switch_new();
    resources_get_int("IsepicSwitch", &state);
    gtk_switch_set_active(GTK_SWITCH(button), state);
    gtk_grid_attach(GTK_GRID(grid), button, 1, 0, 1, 1);
    gtk_widget_show_all(grid);

    g_signal_connect(button, "state-set", G_CALLBACK(on_isepic_switch_state_set),
            NULL);
    return grid;
}


/** \brief  Create widget to load/save ISEPIC image file
 *
 * \return  GtkGrid
 */
static GtkWidget *create_isepic_image_widget(GtkWidget *parent)
{
    return cart_image_widget_create(parent, "ISEPIC image",
            "Isepicfilename", "IsepicImageWrite",
            carthelpers_save_func, carthelpers_flush_func,
            CARTRIDGE_NAME_ISEPIC, CARTRIDGE_ISEPIC);
}


/** \brief  Create widget to control ISEPIC resources
 *
 * \param[in]   parent  parent widget, used for dialogs
 *
 * \return  GtkGrid
 */
GtkWidget *isepic_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *isepic_enable_widget;
    GtkWidget *isepic_image;
    GtkWidget *isepic_switch;


    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    isepic_enable_widget = carthelpers_create_enable_check_button(
            CARTRIDGE_NAME_ISEPIC, CARTRIDGE_ISEPIC);
    gtk_grid_attach(GTK_GRID(grid), isepic_enable_widget, 0, 0, 1, 1);

    isepic_switch = create_isepic_switch_widget();
    gtk_grid_attach(GTK_GRID(grid), isepic_switch, 0, 1, 1, 1);

    isepic_image = create_isepic_image_widget(parent);
    gtk_grid_attach(GTK_GRID(grid), isepic_image, 0, 2, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
