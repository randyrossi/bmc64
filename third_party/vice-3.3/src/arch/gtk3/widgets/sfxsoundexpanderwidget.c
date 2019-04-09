/** \file   sfxsoundexpanderwidget.c
 * \brief   SFX Sound Expander widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES SFXSoundExpander            x64 x64sc xscpu64 x128 xvic
 * $VICERES SFXSoundExpanderChip        x64 x64sc xscpu64 x128 xvic
 * $VICERES SFXSoundExpanderIOSwap      xvic
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
#include <stdlib.h>

#include "machine.h"
#include "resources.h"
#include "debug_gtk3.h"
#include "basewidgets.h"
#include "widgethelpers.h"
#include "basedialogs.h"
#include "cartio.h"
#include "cartridge.h"

#include "sfxsoundexpanderwidget.h"


/** \brief  List of YM chip models
 */
static const vice_gtk3_radiogroup_entry_t chip_models[] = {
    { "3526", 3526 },
    { "3812", 3812 },
    { NULL, -1 }
};


/*
 * Widget references to be able to enable/disable them depending on the SFX
 * Sound Expander enabled toggle button
 */
static GtkWidget *chip_group = NULL;
static GtkWidget *io_swap = NULL;


/** \brief  Handler for the "toggled" event of the SFXSE check button
 *
 * \param[in]       widget      check button
 * \param[in,out]   user_data   unused
 */
static void on_sfx_sound_expander_toggled(GtkWidget *widget, gpointer user_data)
{
    int state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    gtk_widget_set_sensitive(chip_group, state);
    if (machine_class == VICE_MACHINE_VIC20) {
        gtk_widget_set_sensitive(io_swap, state);
    }
}


/** \brief  Create YM chip selection widget
 *
 * \return  GtkGrid
 */
static GtkWidget *create_sfx_chip_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    label = gtk_label_new("YM chip model");
    g_object_set(label, "margin-left", 16, NULL);
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    chip_group = vice_gtk3_resource_radiogroup_new("SFXSoundExpanderChip",
            chip_models, GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_attach(GTK_GRID(grid), chip_group, 1, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create SFX Sound Expander widget
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *sfx_sound_expander_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *sfx_enable;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    sfx_enable = vice_gtk3_resource_check_button_new("SFXSoundExpander",
            "Enable SFX Sound Expander");
    gtk_grid_attach(GTK_GRID(grid), sfx_enable, 0, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), create_sfx_chip_widget(), 0, 1, 1, 1);

    if (machine_class == VICE_MACHINE_VIC20) {
        io_swap = vice_gtk3_resource_check_button_new(
                "SFXSoundExpanderIOSwap", "Enable MasC=uerade I/O swap");
        g_object_set(io_swap, "margin-left", 16, NULL);
        gtk_grid_attach(GTK_GRID(grid), io_swap, 0, 2, 1, 1);
    }

    g_signal_connect(sfx_enable, "toggled",
            G_CALLBACK(on_sfx_sound_expander_toggled), (gpointer)chip_group);
    /* set proper state */
    on_sfx_sound_expander_toggled(sfx_enable, (gpointer)chip_group);
    gtk_widget_show_all(grid);
    return grid;
}
