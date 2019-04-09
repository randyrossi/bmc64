/** \file   digimaxwidget.c
 * \brief   DigiMAX widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES DIGIMAX         x64 x64sc xscpu64 x128 xvic
 * $VICERES DIGIMAXbase     x64 x64sc xscpu64 x128 xvic
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

#include "digimaxwidget.h"


/** \brief  Handler for the "toggled" event of the DIGIMAX check button
 *
 * \param[in]       widget      check button
 * \param[in,out]   user_data   DIGIMAXbase combo box
 */
static void on_digimax_toggled(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *combo = GTK_WIDGET(user_data);

    gtk_widget_set_sensitive(combo,
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
}


/** \brief  Handler for the "changed" event of the DIGImax combo box
 *
 * \param[in]   widget      combo box
 * \param[in]   unused
 */
static void on_combo_changed(GtkWidget *widget, gpointer user_data)
{
    int value;
    char *endptr;
    const char *id_str;

    id_str = gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget));
    value = (int)strtol(id_str, &endptr, 10);
    if (*endptr == '\0') {
        debug_gtk3("setting DIGIMAXbase to $%04X\n", value);
        resources_set_int("DIGIMAXbase", value);
    }
}


/** \brief  Create DIGIMAX widget
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *digimax_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *digimax;
    GtkWidget *combo;
    unsigned int base;
    int current;
    int index;
    char text[256];
    char id_str[80];


    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    digimax = vice_gtk3_resource_check_button_new("DIGIMAX", "Enable DIGIMAX");
    gtk_grid_attach(GTK_GRID(grid), digimax, 0, 0, 1, 1);

    resources_get_int("DIGIMAXbase", &current);
    index = 0;
    combo = gtk_combo_box_text_new();

    if (machine_class != VICE_MACHINE_VIC20) {
        for (base = 0xde00; base < 0xe000; base += 0x20) {
            g_snprintf(text, 256, "$%04X", base);
            g_snprintf(id_str, 80, "%u", base);
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), id_str, text);
            if (current == base) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(combo), index);
            }
            index++;
        }
    } else {
        for (base = 0x9800; base < 0x9900; base += 0x20) {
            g_snprintf(text, 256, "$%04X", base);
            g_snprintf(id_str, 80, "%u", base);
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), id_str, text);
            if (current == base) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(combo), index);
            }
            index++;
        }
        for (base = 0x9c00; base < 0x9d00; base += 0x20) {
            g_snprintf(text, 256, "$%04X", base);
            g_snprintf(id_str, 80, "%u", base);
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), id_str, text);
            if (current == base) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(combo), index);
            }
            index++;
        }
    }

    g_signal_connect(combo, "changed", G_CALLBACK(on_combo_changed), NULL);
    g_signal_connect(digimax, "toggled", G_CALLBACK(on_digimax_toggled),
            (gpointer)combo);

    label = gtk_label_new("DigiMAX base");
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo, 1, 1, 1, 1);

    on_digimax_toggled(digimax, combo);

    gtk_widget_show_all(grid);
    return grid;
}
