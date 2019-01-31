/** \file   petdwwwidget.c
 * \brief   PET RAM expansion module widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES PETDWW          xpet
 * $VICERES PETDWWfilename  xpet
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

#include "petdwwwidget.h"


/* references to widgets to be able to toggle sensitive state depending on the
 * DWW Enable check button
 */
static GtkWidget *entry = NULL;
static GtkWidget *browse = NULL;


/** \brief  Handler for the "toggled" event of the DWW Enable check button
 *
 * Toggles sensitive state of other widgets
 *
 * \param[in]   widget      DWW check button
 * \param[in]   user_data   unused
 */
static void on_dww_toggled(GtkWidget *widget, gpointer user_data)
{
    int state = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    int io_size;

    if (resources_get_int("IOSize", &io_size) < 0) {
        io_size = 0;
    }

    /* only enable when I/O size is 2048 bytes */
    if (state && (io_size < 2048)) {
        vice_gtk3_message_error("Cannot enable DWW",
                "To be able to use DWW, the I/O size of the machine "
                " needs to be 2048 bytes."
                " The current I/O size is %d bytes.\n\n"
                "Use the model settings dialog to set I/O size",
                io_size);
        state = 0;
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FALSE);
    } else {
        resources_set_int("PETDWW", state);
    }

    gtk_widget_set_sensitive(entry, state);
    gtk_widget_set_sensitive(browse, state);
}


/** \brief  Create PETDWW Enable check button
 *
 * Checks if the I/O size is 2048 bytes before setting the resource to TRUE and
 * pops up an error message if the I/O size less than 2048 bytes.
 * Also toggles the sensitivity of the text entry and browse button.
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_dww_check_button(void)
{
    GtkWidget *check;
    int enabled;
    int io_size;

    check = gtk_check_button_new_with_label("Enable DWW hi-res graphics");
    if (resources_get_int("PETDWW", &enabled) < 0) {
        enabled = 0;
    }
    if (resources_get_int("IOSize", &io_size) < 0) {
        io_size = 0;
    }
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check),
            (enabled && io_size >= 2048));

    g_signal_connect(check, "toggled", G_CALLBACK(on_dww_toggled), NULL);
    return check;
}


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

    filename = vice_gtk3_open_file_dialog("Open DWW image file", NULL,
            NULL, NULL);
    if (filename != NULL) {
        debug_gtk3("setting PETDWWfilename to '%s'\n", filename);
        vice_gtk3_resource_entry_full_set(GTK_WIDGET(user_data), filename);
        g_free(filename);
    }
}


/** \brief  Create widget to control PET DWW settings
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *pet_dww_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *enable;
    GtkWidget *label;
    int state;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    /* DWW enable */
    enable = create_dww_check_button();
    gtk_grid_attach(GTK_GRID(grid), enable, 0, 0, 3, 1);


    /* DWW filename */
    label = gtk_label_new("DWW image file");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    entry = vice_gtk3_resource_entry_full_new("PETDWWfilename");
    gtk_widget_set_hexpand(entry, TRUE);
    browse = gtk_button_new_with_label("Browse ...");
    g_signal_connect(browse, "clicked", G_CALLBACK(on_browse_clicked),
            (gpointer)entry);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);

    /* set initial sensitive state of widgets */
    if (resources_get_int("PETDWW", &state) < 0) {
        state = 0;
    }
    gtk_widget_set_sensitive(entry, state);
    gtk_widget_set_sensitive(browse, state);

    gtk_widget_show_all(grid);
    return grid;
}
