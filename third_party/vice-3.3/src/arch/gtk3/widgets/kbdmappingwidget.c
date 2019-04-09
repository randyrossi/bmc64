/** \file   kbdmappingwidget.c
 * \brief   GTK3 keyboard mapping widget for the settings dialog
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES KeymapIndex         -vsid
 * $VICERES KeymapUserPosFile   -vsid
 * $VICERES KeymapUserSymFile   -vsid
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
#include "basewidgets.h"
#include "widgethelpers.h"
#include "openfiledialog.h"

#include "debug_gtk3.h"

#include "kbdmappingwidget.h"


/** \brief  Keymap file glob
 *
 * Looks like shit, but this is how Gtk/GLib works
 */
static const char *keymap_patterns[] = { "*.[vV][kK][mM]", NULL };



/** \brief  resource radiogroup widget controlling the "KeymapIndex" resource
 */
static GtkWidget *radio_group = NULL;


/** \brief  Keyboard mapping types
 */
static const vice_gtk3_radiogroup_entry_t mappings[] = {
    { "Symbolic", 0 },
    { "Positional", 1 },
    { "Symbolic (user)", 2 },
    { "Positional (user)", 3 },
    { NULL, -1 }
};


/** \brief  Custom callback for the symbolic user keymap browser
 *
 * \param[in]   widget      widget triggering the event (unused)
 * \param[in]   user_data   filename returned by the widget (unused)
 */
static void open_sym_file_callback(GtkWidget *widget, gpointer user_data)
{
    vice_gtk3_resource_radiogroup_set(radio_group, 2);   /* sym-user */
}


/** \brief  Custom callback for the positional user keymap browser
 *
 * \param[in]   widget      widget triggering the event (unused)
 * \param[in]   user_data   filename returned by the widget (unused)
 */
static void open_pos_file_callback(GtkWidget *widget, gpointer user_data)
{
    vice_gtk3_resource_radiogroup_set(radio_group, 3);  /* pos-user */
}


/** \brief  Create resource browser widget for the user-defined symbolic keymap
 *
 * \return  resource browser widget
 */
static GtkWidget *create_symbolic_keymap_browser(void)
{
    GtkWidget *browser = vice_gtk3_resource_browser_new(
            "KeymapUserSymFile",
            keymap_patterns,
            "VICE keymap files",
            "Select user-defined symbolic keymap",
            NULL,
            open_sym_file_callback);
    return browser;
}


/** \brief  Create resource browser widget for the user-defined positional keymap
 *
 * \return  resource browser widget
 */
static GtkWidget *create_positional_keymap_browser(void)
{
    GtkWidget *browser = vice_gtk3_resource_browser_new(
            "KeymapUserPosFile",
            keymap_patterns,
            "VICE keymap files",
            "Select user-defined positional keymap",
            NULL,
            open_pos_file_callback);
    return browser;
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
    GtkWidget *grid;
    GtkWidget *browser_sym;
    GtkWidget *browser_pos;
    GtkWidget *label;

    grid = vice_gtk3_grid_new_spaced(VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT);
    label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<b>Keyboard mapping</b>");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    radio_group = vice_gtk3_resource_radiogroup_new(
            "KeymapIndex",
            mappings,
            GTK_ORIENTATION_VERTICAL);
    gtk_grid_set_row_homogeneous(GTK_GRID(radio_group), TRUE);
    g_object_set(G_OBJECT(radio_group), "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 0, 1, 1, 1);

    browser_sym = create_symbolic_keymap_browser();
    gtk_grid_attach(GTK_GRID(radio_group), browser_sym, 1, 2, 1, 1);

    browser_pos = create_positional_keymap_browser();
    gtk_grid_attach(GTK_GRID(radio_group), browser_pos, 1, 3, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
