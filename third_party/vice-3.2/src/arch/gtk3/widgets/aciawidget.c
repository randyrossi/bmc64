/** \file   aciawidget.c
 * \brief   Widget to control various ACIA related resources
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * Controls the following resource(s):
 *
 * $VICERES Acia1Dev    -x64dtv -vsid
 * $VICERES Acia1Base   x64 x64sc xscpu64 xvic x128
 * $VICERES RsDevice1   all
 * $VICERES RsDevice2   all
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
#include "basewidgets.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "openfiledialog.h"

#include "aciawidget.h"


/** \brief  Reference to baud rates list
 */
static int *acia_baud_rates;


/** \brief  List of baud rates
 *
 * Set in acia_widget_create()
 */
static vice_gtk3_combo_entry_int_t *baud_rate_list = NULL;


/** \brief  List of ACIA devices
 */
static const vice_gtk3_radiogroup_entry_t acia_device_list[] = {
    { "Serial 1", 0 },
    { "Serial 2", 1 },
    { "Dump to file", 2 },
    { "Exec process", 3 },
    { NULL, -1 }
};


/** \brief  Generate heap-allocated list to use in a resourcecombobox
 *
 * Creates a list of ui_combo_box_int_t entries from the `acia_baud_rates` list
 */
static void generate_baud_rate_list(void)
{
    unsigned int i;

    /* count number of baud rates */
    for (i = 0; acia_baud_rates[i] > 0; i++) {
        /* NOP */
    }

    baud_rate_list = lib_malloc((i + 1) * sizeof *baud_rate_list);
    for (i = 0; acia_baud_rates[i] > 0; i++) {
        baud_rate_list[i].name = lib_msprintf("%d", acia_baud_rates[i]);
        baud_rate_list[i].id = acia_baud_rates[i];
    }
    /* terminate list */
    baud_rate_list[i].name = NULL;
    baud_rate_list[i].id = -1;
}


/** \brief  Free memory used by `baud_rate_list`
 */
static void free_baud_rate_list(void)
{
    int i;

    for (i = 0; baud_rate_list[i].name != NULL; i++) {
        lib_free(baud_rate_list[i].name);
    }
    lib_free(baud_rate_list);
    baud_rate_list = NULL;
}


/** \brief  Handler for the "destroy" event of the main widget
 *
 * Frees memory used by the baud rate list
 *
 * \param[in]   widget      main widget (unused)
 * \param[in]   user_data   extra event data (unused)
 */
static void on_destroy(GtkWidget *widget, gpointer user_data)
{
    free_baud_rate_list();
}


/** \brief  Handler for the "changed" event of a serial device text box
 *
 * \param[in]   widget      text box triggering the event
 * \param[in]   user_data   serial device number (`int`)
 */
static void on_serial_device_changed(GtkWidget *widget, gpointer user_data)
{
    int device = GPOINTER_TO_INT(user_data);
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(widget));

    /* debug_gtk3("got RsDevice %d\n", device); */
    debug_gtk3("setting RsDevice%d to '%s'\n", device, text);
    resources_set_string_sprintf("RsDevice%d", text, device);
}


/** \brief  Handler for the "clicked" event of the "browse" buttons
 *
 * \param[in]   widget      button triggering the event
 * \param[in]   user_data   device number (`int`)
 */
static void on_browse_clicked(GtkWidget *widget, gpointer user_data)
{
    int device;
    const char *fdesc = "Serial ports";
    const char *flist[] = { "ttyS*", NULL };
    gchar *filename;
    gchar title[256];

    device = GPOINTER_TO_INT(user_data);
    g_snprintf(title, 256, "Select serial device #%d", device);

    filename = vice_gtk3_open_file_dialog(title, fdesc, flist, "/dev");
    if (filename != NULL) {

        GtkWidget *grid;
        GtkWidget *entry;

        debug_gtk3("setting RsDevice%d to '%s'\n", device, filename);
        /* resources_set_string_sprintf("RsDevice%d", filename, device); */

        /* update text entry box, forces an update of the resource */
        grid = gtk_widget_get_parent(widget);
        entry = gtk_grid_get_child_at(GTK_GRID(grid), 0, 1);
        gtk_entry_set_text(GTK_ENTRY(entry), filename);

        g_free(filename);
    }
}


/** \brief  Create an ACIA device widget
 *
 * Creates a widget to select an ACIA device.
 *
 * \return  GtkGrid
 */
static GtkWidget *create_acia_device_widget(void)
{
    GtkWidget *grid;
    GtkWidget *radio_group;

    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT, "Acia device", 1);
    radio_group = vice_gtk3_resource_radiogroup_new(
            "Acia1Dev", acia_device_list, GTK_ORIENTATION_VERTICAL);
    gtk_grid_attach(GTK_GRID(grid), radio_group, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create a widget to set an ACIA serial device (path + baud rate)
 *
 * \param[in]   num     serial device number
 *
 * \return  GtkGrid
 */
static GtkWidget *create_acia_serial_device_widget(int num)
{
    GtkWidget *grid;
    GtkWidget *entry;
    GtkWidget *browse;
    GtkWidget *label;
    GtkWidget *combo;
    char *title;
    const char *path;
    char buffer[256];

    title = lib_msprintf("Serial %d device", num);
    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT, title, 2);
    g_object_set_data(G_OBJECT(grid), "SerialDevice", GINT_TO_POINTER(num));
    lib_free(title);

    /* add "RsDevice" property to widget to allow the event handlers to set
     * the proper resources
     */
    g_object_set_data(G_OBJECT(grid), "RsDevice", GINT_TO_POINTER(num));

    entry = gtk_entry_new();
    gtk_widget_set_hexpand(entry, TRUE);
    g_object_set(entry, "margin-left", 16, NULL);
    browse = gtk_button_new_with_label("Browse ...");
    g_signal_connect(browse, "clicked", G_CALLBACK(on_browse_clicked),
            GINT_TO_POINTER(num));

    gtk_grid_attach(GTK_GRID(grid), entry, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 1, 1, 1, 1);

    label = gtk_label_new("Baud rate");
    g_object_set(label, "margin-left", 16, NULL);
    gtk_widget_set_halign(label, GTK_ALIGN_START);

    g_snprintf(buffer, 256, "RsDevice%dBaud", num);
    combo = vice_gtk3_resource_combo_box_int_new(buffer, baud_rate_list);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo, 1, 2, 1, 1);


    /* set resources*/

    resources_get_string_sprintf("RsDevice%d", &path, num);
    if (path != NULL && *path != '\0') {
        gtk_entry_set_text(GTK_ENTRY(entry), path);
    }

    /* connect handlers */
    g_signal_connect(entry, "changed", G_CALLBACK(on_serial_device_changed),
            GINT_TO_POINTER(num));

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create ACIA settings widget
 *
 * XXX: currently designed for PET, might need updating when used in other UI's
 *
 * \param[in]   baud    list of baud rates (list of `int`'s, terminated by -1)
 *
 * \return  GtkGrid
 */
GtkWidget *acia_widget_create(int *baud)
{
    GtkWidget *grid;
    GtkWidget *device_widget;
    GtkWidget *serial1_widget;
    GtkWidget *serial2_widget;

    acia_baud_rates = baud;
    generate_baud_rate_list();

    grid = vice_gtk3_grid_new_spaced_with_label(
            VICE_GTK3_DEFAULT, VICE_GTK3_DEFAULT, "ACIA settings", 3);

    device_widget = create_acia_device_widget();
    g_object_set(device_widget, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), device_widget, 0, 1, 1, 1);

    serial1_widget = create_acia_serial_device_widget(1);
    gtk_grid_attach(GTK_GRID(grid), serial1_widget, 1, 1, 1, 1);

    serial2_widget = create_acia_serial_device_widget(2);
    gtk_grid_attach(GTK_GRID(grid), serial2_widget, 2, 1, 1, 1);

    g_signal_connect(grid, "destroy", G_CALLBACK(on_destroy), NULL);

    gtk_widget_show_all(grid);
    return grid;
}
