/** \file   v364speechwidget.c
 * \brief   V364 Speech widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES SpeechEnabled   xplus4
 * $VICERES SpeechImage     xplus4
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

#include "lib.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "basewidgets.h"
#include "openfiledialog.h"
#include "machine.h"

#include "v364speechwidget.h"


static GtkWidget *entry;
static GtkWidget *browse;


/** \brief  Handler for the "toggled" event of the Enable check button
 *
 * \param[in]   widget      check button
 * \param[in]   user_data   unused
 */
static void on_enable_toggled(GtkWidget *widget, gpointer user_data)
{
    int state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    gtk_widget_set_sensitive(entry, state);
    gtk_widget_set_sensitive(browse, state);
}


/** \brief  Handler for the "clicked" event of the browse button
 *
 * \param[in]   widget      browse button
 * \param[in]   user_data   text entry to update
 */
static void on_browse_clicked(GtkWidget *widget, gpointer user_data)
{
    gchar *filename = vice_gtk3_open_file_dialog("Open V364 ROM file",
            NULL, NULL, NULL);
    if (filename != NULL) {
        vice_gtk3_resource_entry_full_set(GTK_WIDGET(user_data), filename);
        g_free(filename);
    }
}


/** \brief  Create V364 Speech widget
 *
 * \return  GtkGrid
 */
GtkWidget *v364_speech_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *enable;
    GtkWidget *label;


    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    enable = vice_gtk3_resource_check_button_new("SpeechEnabled",
            "Enable V364 Speech");

    gtk_grid_attach(GTK_GRID(grid), enable, 0, 0, 3, 1);

    label = gtk_label_new("ROM image");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    entry = vice_gtk3_resource_entry_full_new("SpeechImage");
    gtk_widget_set_hexpand(entry, TRUE);
    browse = gtk_button_new_with_label("Browse ...");

    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);

    g_signal_connect(browse, "clicked", G_CALLBACK(on_browse_clicked),
            (gpointer)entry);

    g_signal_connect(enable, "toggled", G_CALLBACK(on_enable_toggled), NULL);

    on_enable_toggled(enable, NULL);

    gtk_widget_show_all(grid);
    return grid;
}
