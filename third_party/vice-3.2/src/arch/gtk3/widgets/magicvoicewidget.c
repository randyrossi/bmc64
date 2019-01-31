/** \file   magicvoicewidget.c
 * \brief   Magic Voice widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES MagicVoiceCartridgeEnabled  x64 x64sc xscpu64 x128
 * $VICERES MagicVoiceImage             x64 x64sc xscpu64 x128
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
#include "cartridge.h"

#include "magicvoicewidget.h"


static GtkWidget *entry = NULL;
static GtkWidget *browse = NULL;


/** \brief  Handler for the "clicked" event of the browse button
 *
 * Activates a file-open dialog and stores the file name in the GtkEntry passed
 * in \a user_data if valid, triggering a resource update.
 *
 * \param[in]   widget      button
 * \param[in]   user_data   entry to store filename in
 */
static void on_browse_clicked(GtkWidget *widget, gpointer user_data)
{
    gchar *filename;

    filename = vice_gtk3_open_file_dialog("Open Magic Voice image", NULL,
            NULL, NULL);
    if (filename != NULL) {
        GtkWidget *entry = GTK_WIDGET(user_data);
        debug_gtk3("setting MagicVoiceImage to '%s'\n", filename);
        vice_gtk3_resource_entry_full_set(entry, filename);
        g_free(filename);
    }
}


/** \brief  Create widget to control MagicVoice cartridge
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *magic_voice_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *enable;
    GtkWidget *label;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    /*    enable = create_enable_check_button(); */
    enable = carthelpers_create_enable_check_button(CARTRIDGE_NAME_MAGIC_VOICE,
            CARTRIDGE_MAGIC_VOICE);
    gtk_grid_attach(GTK_GRID(grid), enable, 0, 0, 3, 1);

    label = gtk_label_new("Magic Voice ROM");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    entry = vice_gtk3_resource_entry_full_new("MagicVoiceImage");
    gtk_widget_set_hexpand(entry, TRUE);
    browse = gtk_button_new_with_label("Browse ...");
    g_signal_connect(browse, "clicked", G_CALLBACK(on_browse_clicked),
            (gpointer)entry);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
