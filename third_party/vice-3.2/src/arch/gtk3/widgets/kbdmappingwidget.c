/** \file   kbdmappingwidget.c
 * \brief   GTK3 keyboard mapping widget for the settings dialog
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES KeymapIndex     -vsid
 * $VICERES KeymapPosFile   -vsid
 * $VICERES KeymapSymFile   -vsid
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
 */


#include "vice.h"

#include <gtk/gtk.h>

#include "lib.h"
#include "ui.h"
#include "resources.h"
#include "vsync.h"
#include "widgethelpers.h"
#include "openfiledialog.h"

#include "debug_gtk3.h"

#include "kbdmappingwidget.h"


/** \brief  GtkGrid layout for the widget
 */
static GtkWidget *layout = NULL;


/** \brief  Keyboard mapping types
 */
static const vice_gtk3_radiogroup_entry_t mappings[] = {
    { "Symbolic mapping", 0 },
    { "Positional mapping", 1 },
    { "Symbolic mapping (User)", 2 },
    { "Positional mapping (User)", 3 },
    { NULL, -1 }
};


/** \brief  Load user symbolic keymap file
 *
 * \param[in]   widget      widget triggering the event (unused)
 * \param[in]   user_data   filename returned by 'open file' dialog
 */
static void open_sym_file_callback(GtkWidget *widget, gpointer user_data)
{
    gchar *filename;
    const char *filters[] = { "*.vkm", NULL };

    filename = vice_gtk3_open_file_dialog("Open symbolic keymap file",
            "Keymaps", filters, NULL);

    debug_gtk3("got file \"%s\"\n", filename);
    if (filename != NULL) {
        resources_set_string("KeymapSymFile", filename);
        g_free(filename);
        resources_set_int("KeymapIndex", 2);
        /* set proper radio button */
        vice_gtk3_radiogroup_set_index(layout, 2);

    }
}


/** \brief  Load user positional keymap file
 *
 * \param[in]   widget      widget triggering the event (unused)
 * \param[in]   user_data   filename returned by 'open file' dialog
 */
static void open_pos_file_callback(GtkWidget *widget, gpointer user_data)
{
    gchar *filename;
    const char *filters[] = { "*.vkm", NULL };

    filename = vice_gtk3_open_file_dialog("Open positional keymap file",
            "Keymaps", filters, NULL);

    debug_gtk3("got file \"%s\"\n", filename);
    if (filename != NULL) {
        resources_set_string("KeymapPosFile", filename);
        g_free(filename);
        resources_set_int("KeymapIndex", 3);
        /* set proper radio button */
        vice_gtk3_radiogroup_set_index(layout, 3);
    }
}


/** \brief  Event handler for changing keymapping
 *
 * \param[in]   widget      widget triggering the event (unused)
 * \param[in]   user_data   value for 'KeymapIndex' resource
 */
static void on_mapping_changed(GtkWidget *widget, gpointer user_data)
{
    int index = GPOINTER_TO_INT(user_data);

    debug_gtk3("setting mapping to %d\n", index);
    resources_set_int("KeymapIndex", index);
}


/** \brief  Create a keyboard mapping selection widget
 *
 * \return  GtkWidget
 *
 * \fixme   I'm not really satisfied with the 'select file' buttons, perhaps
 *          they should be placed next to the radio buttons?
 */
GtkWidget *kbdmapping_widget_create(GtkWidget *widget)
{
    GtkWidget *btn_sym;
    GtkWidget *btn_pos;
    int index = 0;

    resources_get_int("KeymapIndex", &index);

    /* create grid with label and four radio buttons */
    layout = uihelpers_radiogroup_create("Keyboard mapping", mappings,
            on_mapping_changed, index);

    btn_sym = gtk_button_new_with_label("Select file ...");
    g_signal_connect(btn_sym, "clicked",
            G_CALLBACK(open_sym_file_callback), (gpointer)(widget));
    g_object_set(btn_sym, "margin-left", 32, NULL);

    gtk_grid_insert_row(GTK_GRID(layout), 4);
    gtk_grid_attach(GTK_GRID(layout), btn_sym, 0, 4, 1, 1);


    btn_pos = gtk_button_new_with_label("Select file ...");
    g_signal_connect(btn_pos, "clicked",
            G_CALLBACK(open_pos_file_callback), (gpointer)(widget));

    g_object_set(btn_pos, "margin-left", 32, NULL);
    gtk_grid_attach(GTK_GRID(layout), btn_pos, 0, 6, 1, 1);


    gtk_widget_show_all(layout);
    return layout;
}
