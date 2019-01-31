/** \file   cwdwidget.c
 * \brief   Widget to set the current working directory
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
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
#include <glib/gstdio.h>

#include "lib.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "selectdirectorydialog.h"

#include "cwdwidget.h"


/** \brief  Reference to the text entry box
 */
static GtkWidget *entry;


/** \brief  Handler for the "changed" event of the text entry box
 *
 * \param[in]   widget      widget triggering the event
 * \param[in]   user_data   data for the event (unused)
 */
static void on_entry_changed(GtkWidget *widget, gpointer user_data)
{
    const char *cwd = gtk_entry_get_text(GTK_ENTRY(widget));

    debug_gtk3("setting cwd to '%s'\n", cwd);
    /* TODO: make the entry background 'red' or so when chdir() fails */
    g_chdir(cwd);
}


/** \brief  Handler for the "clicked" event of the browse button
 *
 * \param[in]   widget      widget triggering the event
 * \param[in]   user_data   data for the event (unused)
 */
static void on_browse_clicked(GtkWidget *widget, gpointer user_data)
{
    gchar *filename;

    filename = vice_gtk3_select_directory_dialog("Select directory",
            NULL, TRUE, NULL);
    if (filename != NULL) {
        gtk_entry_set_text(GTK_ENTRY(entry), filename);
    }
}



/** \brief  Create widget to change the current working directory
 *
 * \return  GtkGrid
 */
GtkWidget *cwd_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *wrapper;
    GtkWidget *browse;

    grid = uihelpers_create_grid_with_label("Current working directory", 1);

    wrapper = gtk_grid_new();
    g_object_set(wrapper, "margin", 8, NULL);
    gtk_grid_set_column_spacing(GTK_GRID(wrapper), 8);

    entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), g_get_current_dir());
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_grid_attach(GTK_GRID(wrapper), entry, 0, 0, 1, 1);

    browse = gtk_button_new_with_label("Browse ...");
    gtk_grid_attach(GTK_GRID(wrapper), browse, 1, 0, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), wrapper, 0, 1, 1, 1);

    g_signal_connect(entry, "changed", G_CALLBACK(on_entry_changed), NULL);
    g_signal_connect(browse, "clicked", G_CALLBACK(on_browse_clicked), NULL);

    gtk_widget_show_all(grid);
    return grid;
}
