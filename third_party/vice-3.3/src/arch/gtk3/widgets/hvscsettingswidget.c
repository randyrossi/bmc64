/** \file   hvscsettingswidget.c
 * \brief   High Voltage SID Collection settings widget for VSID
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
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

#include "resources.h"
#include "vice_gtk3.h"
#include "debug_gtk3.h"
#include "resourceentry.h"
#include "hvsc.h"

#include "hvscsettingswidget.h"


/** \brief  Reference to the entry box for the HVSC root dir
 *
 * This is a "full" resource entry widget, which means the resource is only
 * updated when the user pushes Enter or the widget looses focus.
 */
static GtkWidget *hvsc_root_entry;


/** \brief  Handler for the 'clicked' event of the HVSC root "browse" button
 *
 * \param[in]   widget  browse button (ignored)
 * \param[in]   data    extra event data (ignored)
 */
static void on_browse_clicked(GtkWidget *widget, gpointer data)
{
    gchar *path;
    const char *current = NULL;

    /* try to get the current HVSC root dir */
    if (resources_get_string("HSVCRoot", &current) != 0) {
        current = NULL;
    }

    /* pop up dialog */
    path = vice_gtk3_select_directory_dialog(
            "Select HVSC root directory",
            current,
            FALSE,
            NULL);

    /* handle input */
    if (path != NULL) {
        debug_gtk3("Got path '%s'.", path);
        vice_gtk3_resource_entry_full_set(hvsc_root_entry, path);
        g_free(path);
    }
}


/** \brief  Create HVSC settings widget
 *
 * \param[in]   parent  parent widget (ignored)
 *
 * \return  GtkGrid
 */
GtkWidget *hvsc_settings_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *browse;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);

    label = gtk_label_new("HSVC root directory");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    hvsc_root_entry = vice_gtk3_resource_entry_full_new("HVSCRoot");
    gtk_widget_set_hexpand(hvsc_root_entry, TRUE);
    browse = gtk_button_new_with_label("Browse ...");

    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), hvsc_root_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 0, 1, 1);

    g_signal_connect(browse, "clicked", G_CALLBACK(on_browse_clicked), NULL);

    gtk_widget_show_all(grid);
    return grid;
}
