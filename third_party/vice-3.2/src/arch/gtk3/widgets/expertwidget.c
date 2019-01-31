/** \file   expertwidget.c
 * \brief   Widget to control Expert Cartridge resources
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Controls the following resource(s):
 *  ExpertCartridgeEnabled  (x64/x64sc/xscpu64/x128)
 *  ExpertCartridgeMode     (x64/x64sc/xscpu64/x128)
 *  Expertfilename          (x64/x64sc/xscpu64/x128)
 *  ExpertImageWrite        (x64/x64sc/xscpu64/x128)
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
#include "cartimagewidget.h"
#include "openfiledialog.h"
#include "savefiledialog.h"
#include "cartridge.h"
#include "carthelpers.h"

#include "expertwidget.h"


/** \brief  List of 'modes' for the Expert Cartridge
 */
static const vice_gtk3_radiogroup_entry_t mode_list[] = {
    { "Off", 0 },
    { "Programmable", 1 },
    { "On", 2 },
    { NULL, -1 }
};


/** \brief  Create Expert Cartridge mode widget
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_expert_mode_widget(void)
{
    GtkWidget *grid;
    GtkWidget *radio_group;

    grid = uihelpers_create_grid_with_label("Cartridge mode", 3);

    radio_group = vice_gtk3_resource_radiogroup_new("ExpertCartridgeMode",
            mode_list, GTK_ORIENTATION_HORIZONTAL);
    g_object_set(radio_group, "margin-left", 16, NULL);
    gtk_grid_set_column_spacing(GTK_GRID(radio_group), 16);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to load/save Expert Cartridge image file
 *
 * \return  GtkGrid
 */
static GtkWidget *create_expert_image_widget(GtkWidget *parent)
{
    return cart_image_widget_create(parent, "Expert Cartridge image",
            "Expertfilename", "ExpertImageWrite",
            carthelpers_save_func, carthelpers_flush_func,
            CARTRIDGE_NAME_EXPERT, CARTRIDGE_EXPERT);
}


/** \brief  Create widget to control Expert Cartridge resources
 *
 * \param[in]   parent  parent widget, used for dialogs
 *
 * \return  GtkGrid
 */
GtkWidget *expert_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *expert_enable_widget; /* expert_enable is defined by expert.c */
    GtkWidget *expert_image;
    GtkWidget *expert_mode;


    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    expert_enable_widget = carthelpers_create_enable_check_button(
            CARTRIDGE_NAME_EXPERT, CARTRIDGE_EXPERT);
    gtk_grid_attach(GTK_GRID(grid), expert_enable_widget, 0, 0, 1, 1);

    expert_mode = create_expert_mode_widget();
    gtk_grid_attach(GTK_GRID(grid), expert_mode, 0, 1, 1, 1);

    expert_image = create_expert_image_widget(parent);
    gtk_grid_attach(GTK_GRID(grid), expert_image, 0, 2, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
