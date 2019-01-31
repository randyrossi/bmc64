/** \file   settings_snapshot.c
 * \brief   Snapshot/recording settings widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES EventSnapshotDir    all
 * $VICERES EventStartMode      all
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
#include "vice-event.h"
#include "vice_gtk3.h"

#include "settings_snapshot.h"


/** \brief  List of Event start modes
 */
static const vice_gtk3_radiogroup_entry_t recstart_modes[] = {
    { "Save new snapshot",          EVENT_START_MODE_FILE_SAVE },
    { "Load existing snapshot",     EVENT_START_MODE_FILE_LOAD },
    { "Start with reset",           EVENT_START_MODE_RESET },
    { "Overwrite running playback", EVENT_START_MODE_PLAYBACK },
    { NULL, -1 }
};


/** \brief  Reference to the 'history directory' entry box
 */
static GtkWidget *histdir_entry;


/** \brief  Handler for the "clicked" event of the "browse" button
 *
 * \param[in]   widget      widget triggering the event
 * \param[in]   user_data   extra event data (unused)
 */
static void on_histdir_browse_clicked(GtkWidget *widget, gpointer user_data)
{
    char *filename;
    const char *current;

    if (resources_get_string("EventSnapshotDir", &current) < 0) {
        debug_gtk3("failed to get current history directory, using NULL\n");
        current = NULL;
    }

    filename = vice_gtk3_select_directory_dialog(
            "Select history directory", NULL, TRUE, current);
    if (filename != NULL) {
        debug_gtk3("Setting EventSnapshotDir to '%s'\n", filename);
        vice_gtk3_resource_entry_full_set(histdir_entry, filename);
        g_free(filename);
    }
}


/** \brief  Create settings widget for snapshot/event recording
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *settings_snapshot_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;

    GtkWidget *label;
    GtkWidget *histdir_browse;
    GtkWidget *recmode_widget;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    label = gtk_label_new("History directory");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);

    histdir_entry = vice_gtk3_resource_entry_full_new("EventSnapshotDir");
    gtk_widget_set_hexpand(histdir_entry, TRUE);

    histdir_browse = gtk_button_new_with_label("Browse ...");
    g_signal_connect(histdir_browse, "clicked",
            G_CALLBACK(on_histdir_browse_clicked), NULL);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), histdir_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), histdir_browse, 2, 0, 1, 1);

    label = gtk_label_new("Recording start mode");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_widget_set_valign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);

    recmode_widget = vice_gtk3_resource_radiogroup_new("EventStartMode",
            recstart_modes, GTK_ORIENTATION_VERTICAL);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), recmode_widget, 1, 1, 2, 1);

    gtk_widget_show_all(grid);
    return grid;
}
