/** \file   ide64widget.c
 * \brief   Widget to control IDE64 resources
 *
 * IDE64 settings widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES IDE64AutodetectSize1    x64 x64sc xscpu64 x128
 * $VICERES IDE64AutodetectSize2    x64 x64sc xscpu64 x128
 * $VICERES IDE64AutodetectSize3    x64 x64sc xscpu64 x128
 * $VICERES IDE64AutodetectSize4    x64 x64sc xscpu64 x128
 * $VICERES IDE64version            x64 x64sc xscpu64 x128
 * $VICERES IDE64Image1             x64 x64sc xscpu64 x128
 * $VICERES IDE64Image2             x64 x64sc xscpu64 x128
 * $VICERES IDE64Image3             x64 x64sc xscpu64 x128
 * $VICERES IDE64Image4             x64 x64sc xscpu64 x128
 * $VICERES IDE64ClockPort          x64 x64sc xscpu64 x128
 * $VICERES IDE64Cylinders1         x64 x64sc xscpu64 x128
 * $VICERES IDE64Cylinders2         x64 x64sc xscpu64 x128
 * $VICERES IDE64Cylinders3         x64 x64sc xscpu64 x128
 * $VICERES IDE64Cylinders4         x64 x64sc xscpu64 x128
 * $VICERES IDE64Heads1             x64 x64sc xscpu64 x128
 * $VICERES IDE64Heads2             x64 x64sc xscpu64 x128
 * $VICERES IDE64Heads3             x64 x64sc xscpu64 x128
 * $VICERES IDE64Heads4             x64 x64sc xscpu64 x128
 * $VICERES IDE64Sectors1           x64 x64sc xscpu64 x128
 * $VICERES IDE64Sectors2           x64 x64sc xscpu64 x128
 * $VICERES IDE64Sectors3           x64 x64sc xscpu64 x128
 * $VICERES IDE64Sectors4           x64 x64sc xscpu64 x128
 * $VICERES IDE64USBServer          x64 x64sc xscpu64 x128
 * $VICERES IDE64USBServerAddress   x64 x64sc xscpu64 x128
 * $VICERES IDE64RTCSave            x64 x64sc xscpu64 x128
 * $VICERES SBDIGIMAX               x64 x64sc xscpu64 x128
 * $VICERES SBDIGIMAXbase           x64 x64sc xscpu64 x128
 * $ViCERES SBETFE                  x64 x64sc xscpu64 x128
 * $VICERES SBETFEbase              x64 x64sc xscpu64 x128
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

#include "vice_gtk3.h"
#include "machine.h"
#include "resources.h"
#include "ide64.h"

#include "ide64widget.h"


/** \brief  List of IDE64 revisions
 */
static const vice_gtk3_radiogroup_entry_t revisions[] = {
    { "Version 3",      IDE64_VERSION_3 },
    { "Version 4.1",    IDE64_VERSION_4_1 },
    { "Version 4.2",    IDE64_VERSION_4_2 },
    { NULL, -1 }
};


/** \brief  List of ShortBus DIGIMAX I/O bases
 */
static const vice_gtk3_combo_entry_int_t digimax_addresses[] = {
    { "$DE40", 0xde40 },
    { "$DE48", 0xde48 },
    { NULL, -1 }
};

#ifdef HAVE_RAWNET
/** \brief  List of ShortBus ETFE I/O bases
 */
static const vice_gtk3_combo_entry_int_t etfe_addresses[] = {
    { "$DE00", 0xde00 },
    { "$DE10", 0xde10 },
    { "$DF00", 0xdf00 },
    { NULL, -1 }
};
#endif




/** \brief  Handler for the "toggled" event of the USB Server check button
 *
 * \param[in]       widget      check button triggering the event
 * \param[in,out]   user_data   reference to the USB server address widget
 */
static void on_usb_enable_toggled(GtkWidget *widget, gpointer user_data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(user_data),
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
}


/** \brief  Handler for the "clicked" event of the HD image "browse" buttons
 *
 * \param[in]       widget      button
 * \param[in,out]   user_data   entry box to store the HD image file
 */
static void on_browse_clicked(GtkWidget *widget, gpointer user_data)
{
    gchar *filename;
    const char *filter_list[] = {
        "*.hdd", "*.iso", "*.fdd", "*.cfa", NULL
    };

    filename = vice_gtk3_open_file_dialog("Select disk image file",
            "HD image files", filter_list, NULL);
    if (filename != NULL) {
        vice_gtk3_resource_entry_full_set(GTK_WIDGET(user_data), filename);
        g_free(filename);
    }
}


/** \brief  Handler for the "toggled" event of the "Autodetect Size" widgets
 *
 * \param[in]       widget      check button triggering the event
 * \param[in,out]   user_data   reference to the geometry widget
 */
static void on_autosize_toggled(GtkWidget *widget, gpointer user_data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(user_data),
            !(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))));
}


/** \brief  Handler for the "toggled" event of the "Enable DigiMAX" widget
 *
 * \param[in]       widget      check button triggering the event
 * \param[in,out]   user_data   reference to the DigiMax address widget
 */
static void on_digimax_toggled(GtkWidget *widget, gpointer user_data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(user_data),
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
}


#ifdef HAVE_RAWNET
/** \brief  Handler for the "toggled" event of the "Enable ETFE" widget
 *
 * \param[in]       widget      check button triggering the event
 * \param[in,out]   user_data   reference to the ETFE address widget
 */
static void on_etfe_toggled(GtkWidget *widget, gpointer user_data)
{
    gtk_widget_set_sensitive(GTK_WIDGET(user_data),
            gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)));
}
#endif


/** \brief  Create widget to set the IDE64 revision
 *
 * \return  GtkGrid
 */
static GtkWidget *create_ide64_revision_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *group;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);

    label = gtk_label_new("IDE64 revision");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    group = vice_gtk3_resource_radiogroup_new("IDE64version", revisions,
            GTK_ORIENTATION_HORIZONTAL);
    gtk_grid_set_column_spacing(GTK_GRID(group), 16);
    gtk_grid_attach(GTK_GRID(grid), group, 1, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to control the USB server resources
 *
 * \return  GtkGrid
 */
static GtkWidget *create_ide64_usb_widget(void)
{
    GtkWidget *grid;
    GtkWidget *enable;
    GtkWidget *address;
    GtkWidget *label;
    int state;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);

    enable = vice_gtk3_resource_check_button_new("IDE64USBServer",
            "Enable USB server");
    label = gtk_label_new("USB server address");
    address = vice_gtk3_resource_entry_full_new("IDE64USBServerAddress");
    gtk_widget_set_hexpand(address, TRUE);

    gtk_grid_attach(GTK_GRID(grid), enable, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), address, 2, 0, 1, 1);

    resources_get_int("IDE64USBServer", &state);
    gtk_widget_set_sensitive(address, state);

    g_signal_connect(enable, "toggled", G_CALLBACK(on_usb_enable_toggled),
            (gpointer)address);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to select the IDE64 clockport device
 *
 * \return  GtkGrid
 */
static GtkWidget *create_ide64_clockport_widget(void)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *clockport;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);

    label = gtk_label_new("ClockPort device");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    clockport = clockport_device_widget_create("IDE64ClockPort");

    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), clockport, 1, 0, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to control the RTC-save resource
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_ide64_rtc_widget(void)
{
    return vice_gtk3_resource_check_button_new("IDE64RTCSave",
            "Enable RTC saving");
}


/** \brief  Create widget to select HD image and set geometry
 *
 * \param[in]   device  device number (1-4)
 *
 * \return  GtkGrid
 */
static GtkWidget *create_ide64_device_widget(int device)
{
    GtkWidget *grid;
    GtkWidget *label;
    GtkWidget *entry;
    GtkWidget *browse;
    GtkWidget *autosize;
    GtkWidget *geometry;
    GtkWidget *cylinders;
    GtkWidget *heads;
    GtkWidget *sectors;

    char buffer[256];
    char resource[256];

    g_snprintf(buffer, 256, "Device %d settings", device);

    grid = uihelpers_create_grid_with_label(buffer, 3);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    label = gtk_label_new("Image file");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);

    g_snprintf(resource, 256, "IDE64image%d", device);
    entry = vice_gtk3_resource_entry_full_new(resource);
    gtk_widget_set_hexpand(entry, TRUE);

    browse = gtk_button_new_with_label("Browse ...");
    g_signal_connect(browse, "clicked", G_CALLBACK(on_browse_clicked),
            (gpointer)entry);

    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), browse, 2, 1, 1, 1);

    autosize = vice_gtk3_resource_check_button_new_sprintf(
            "IDE64AutodetectSize%d", "Autodetect image size", device);
    g_object_set(autosize, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(grid), autosize, 0, 2, 3, 1);

    /* create grid for the geomerty spin buttons */

    geometry = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(geometry), 8);


    label = gtk_label_new("Cylinders");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(geometry), label, 0, 0, 1, 1);

    cylinders = vice_gtk3_resource_spin_int_new_sprintf("IDE64cylinders%d",
            0, 65536, 256, device);
    gtk_widget_set_hexpand(cylinders, FALSE);
    gtk_grid_attach(GTK_GRID(geometry), cylinders, 1, 0, 1, 1);

    label = gtk_label_new("Heads");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(geometry), label, 0, 1, 1, 1);

    heads = vice_gtk3_resource_spin_int_new_sprintf(
            "IDE64heads%d", 0, 16, 1, device);
    gtk_widget_set_hexpand(heads, FALSE);
    gtk_grid_attach(GTK_GRID(geometry), heads, 1, 1, 1, 1);

    label = gtk_label_new("Sectors");
    gtk_widget_set_halign(label, GTK_ALIGN_START);
    g_object_set(label, "margin-left", 16, NULL);
    gtk_grid_attach(GTK_GRID(geometry), label, 0, 2, 1, 1);

    sectors = vice_gtk3_resource_spin_int_new_sprintf(
            "IDE64sectors%d", 0, 63, 1, device);
    gtk_widget_set_hexpand(heads, FALSE);
    gtk_grid_attach(GTK_GRID(geometry), sectors, 1, 2, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), geometry, 0, 3, 3, 1);

    g_signal_connect(autosize, "toggled", G_CALLBACK(on_autosize_toggled),
            (gpointer)geometry);

    /* enable/disable geometry widgets depending on 'autosize' state */
    on_autosize_toggled(autosize, (gpointer)geometry);

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create ShortBus settings widget
 *
 * Handles the SBDIGIMAX/SBDIGIMAXbase and SBETFE/SBETFEbase resources
 *
 * \return  GtkGrid
 */
static GtkWidget *create_ide64_shortbus_widget(void)
{
    GtkWidget *grid;
    GtkWidget *digimax_enable;
    GtkWidget *digimax_label;
    GtkWidget *digimax_address;
#ifdef HAVE_RAWNET
    GtkWidget *etfe_enable;
    GtkWidget *etfe_label;
    GtkWidget *etfe_address;
#endif

    grid = uihelpers_create_grid_with_label("ShortBus settings", 3);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 16);

    digimax_enable = vice_gtk3_resource_check_button_new("SBDIGIMAX",
            "Enable DigiMAX");
    g_object_set(digimax_enable, "margin-left", 16, NULL);
    digimax_label = gtk_label_new("DigMAX base address");
    gtk_widget_set_halign(digimax_label, GTK_ALIGN_END);
    digimax_address = vice_gtk3_resource_combo_box_int_new("SBDIGIMAXbase",
            digimax_addresses);

    gtk_grid_attach(GTK_GRID(grid), digimax_enable, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), digimax_label, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), digimax_address, 2, 1, 1, 1);

    g_signal_connect(digimax_enable, "toggled", G_CALLBACK(on_digimax_toggled),
            (gpointer)digimax_address);

    on_digimax_toggled(digimax_enable, (gpointer)digimax_address);

#ifdef HAVE_RAWNET
    etfe_enable = vice_gtk3_resource_check_button_new("SBETFE",
            "Enable ETFE");
    g_object_set(etfe_enable, "margin-left", 16, NULL);
    etfe_label = gtk_label_new("ETFE base address");
    gtk_widget_set_halign(etfe_label, GTK_ALIGN_END);
    etfe_address = vice_gtk3_resource_combo_box_int_new("SBETFEbase",
            etfe_addresses);

    gtk_grid_attach(GTK_GRID(grid), etfe_enable, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), etfe_label, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), etfe_address, 2, 2, 1, 1);

    g_signal_connect(etfe_enable, "toggled", G_CALLBACK(on_etfe_toggled),
            (gpointer)etfe_address);

    on_etfe_toggled(etfe_enable, (gpointer)etfe_address);

#endif

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create widget to control IDE64 resources
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *ide64_widget_create(GtkWidget *parent)
{
    GtkWidget *grid;
    GtkWidget *wrapper;
    GtkWidget *stack;
    GtkWidget *stack_switcher;
    int d;

    grid = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(grid), 8);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);

    gtk_grid_attach(GTK_GRID(grid), create_ide64_revision_widget(),
            0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), create_ide64_usb_widget(), 0, 1, 1, 1);

    wrapper = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(wrapper), 16);
    gtk_grid_attach(GTK_GRID(wrapper), create_ide64_rtc_widget(), 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(wrapper), create_ide64_clockport_widget(),
            1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), wrapper, 0, 2, 1, 1);

    /* create stack and stack switcher (tab-like interface) for the HD image
     * settings widgets
     */
    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack),
            GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_stack_set_transition_duration(GTK_STACK(stack), 1000);

    for (d = 1; d <= 4; d++) {
        char buffer[256];

        g_snprintf(buffer, 256, "Device %d", d);
        gtk_stack_add_titled(GTK_STACK(stack), create_ide64_device_widget(d),
                buffer, buffer);
    }

    stack_switcher = gtk_stack_switcher_new();
    gtk_stack_switcher_set_stack(GTK_STACK_SWITCHER(stack_switcher),
            GTK_STACK(stack));
    gtk_widget_set_halign(stack_switcher, GTK_ALIGN_CENTER);

    gtk_widget_show_all(stack);
    gtk_widget_show_all(stack_switcher);

    gtk_grid_attach(GTK_GRID(grid), stack_switcher, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), stack, 0, 4, 1, 1);

    gtk_grid_attach(GTK_GRID(grid), create_ide64_shortbus_widget(),
            0, 5, 1, 1);

    gtk_widget_show_all(grid);
    return grid;
}
