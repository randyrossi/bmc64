/** \file   settings_controlport.c
 * \brief   Widget to control settings for control ports
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES JoyPort1Device  x64 x64sc x64dtv xscpu64 x128 xcbm5x0 xplus4 xvic
 * $VICERES JoyPort2Device  x64 x64sc x64dtv xscpu64 x128 xcbm5x0 xplus4
 * $VICERES JoyPort3Device  x64 x64sc x64dtv xscpu64 x128 xcbm2 xvic
 * $VICERES JoyPort4Device  x64 x64sc xscpu64 x128 xcbm2 xpet xvice
 * $VICERES JoyPort5Device  xplus4
 * $VICERES BBRTCSave
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
#include <stdbool.h>
#include <stdlib.h>

#include "vice_gtk3.h"
#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "joyport.h"

#include "settings_controlport.h"


static void joyport_devices_list_shutdown(void);


/** \brief  Lists of valid devices for each joyport
 */
static joyport_desc_t *joyport_devices[JOYPORT_MAX_PORTS];


/** \brief  Handler for the "destroy" event of the main widget
 *
 * \param[in]   widget      the main widget (unused)
 * \param[in]   user_data   extra data (unused)
 */
static void on_destroy(GtkWidget *widget, gpointer user_data)
{
    joyport_devices_list_shutdown();
}


/** \brief  Handler for the "changed" event of a combo box
 *
 * \param[in]   combo       combo box
 * \param[in]   user_data   port number (0-based)
 */
static void on_joyport_changed(GtkComboBoxText *combo, gpointer user_data)
{
    int port = GPOINTER_TO_INT(user_data);
    char *endptr;
    const char *id_str;
    int id;

    id_str = gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo));
    id = (int)strtol(id_str, &endptr, 10);

    debug_gtk3("changing JoyPort%dDevice to %d\n", port + 1, id);
    resources_set_int_sprintf("JoyPort%dDevice", id, port + 1);
}


/** \brief  Create combo box for joyport \a port
 *
 * \param[in]   port    Joyport number (0-4, 0 == JoyPort1Device)
 * \param[in]   title   widget title
 *
 * \return  GtkGrid
 */
static GtkWidget *create_joyport_widget(int port, const char *title)
{
    GtkWidget *grid;
    GtkWidget *combo;
    joyport_desc_t *dev = joyport_devices[port];
    int current;

    resources_get_int_sprintf("JoyPort%dDevice", &current, port + 1);

    grid = uihelpers_create_grid_with_label(title, 1);
    if (dev == NULL) {
        fprintf(stderr, "error: no devices list\n");
        return grid;
    }

    combo = gtk_combo_box_text_new();
    g_object_set(combo, "margin-left", 16, NULL);
    gtk_widget_set_hexpand(combo, TRUE);

    while (dev->name != NULL) {
        char id[32];

        g_snprintf(id, 32, "%d", dev->id);
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), id, dev->name);
        if (current == dev->id) {
            gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo), id);
        }
        dev++;
    }
    gtk_grid_attach(GTK_GRID(grid), combo, 0, 1, 1, 1);

    g_signal_connect(combo, "changed", G_CALLBACK(on_joyport_changed),
            GINT_TO_POINTER(port));

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Create checkbox for the battery-backed RTC save option
 *
 * \return  GtkCheckButton
 */
static void *create_bbrtc_widget(void)
{
    GtkWidget *check;

    check = vice_gtk3_resource_check_button_new("BBRTCSave",
            "Save battery-backed real time clock data when changed");
    g_object_set(check, "margin-left", 16, NULL);
    return check;
}


/** \brief  Retrieve valid devices for each joyport
 *
 * joyport_get_valid_devices() returns an empty list for unsupported devices,
 * so no need to check for machine type.
 */
static void joyport_devices_list_init(void)
{
    int i;

    for (i = 0; i < JOYPORT_MAX_PORTS; i++) {
        joyport_devices[i] = joyport_get_valid_devices(i);
    }
}


/** \brief  Clean up memory used by the valid devices list
 */
static void joyport_devices_list_shutdown(void)
{
    int i;

    debug_gtk3("called: free memory used by joyport devices list\n");

    for (i = 0; i < JOYPORT_MAX_PORTS; i++) {
        if (joyport_devices[i] != NULL) {
            lib_free(joyport_devices[i]);
            joyport_devices[i] = NULL;
        }
    }
}


/** \brief  Create layout for x64, x64sc, xscpu64 and x128
 *
 * Two control ports and two userport adapter ports.
 *
 * \param[in,out]   grid    main widget grid
 *
 * \return  number of rows used in the \a grid
 */
static int create_c64_layout(GtkGrid *grid)
{
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_1,
                "Control port 1"),
            0, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_2,
                "Control port 2"),
            1, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_3,
                "Userport joystick adapter port 1"),
            0, 1, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_4,
                "Userport joystick adapter port 2"),
            1, 1, 1, 1);
    return 2;
}


/** \brief  Create layout for x64dtv
 *
 * Two control ports and one userport adapter port.
 *
 * \param[in,out]   grid    main widget grid
 *
 * \return  number of rows used in the \a grid
 */
static int create_c64dtv_layout(GtkGrid *grid)
{
    gtk_grid_attach(grid,
            create_joyport_widget(
                JOYPORT_1,
                "Control port 1"),
            0, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_2, "Control port 2"),
            1, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_3,
                "Userport joystick adapter port 1"),
            0, 1, 1, 1);
    return 2;
}


/** \brief  Create layout for xvic
 *
 * One control port and two userport adapter ports.
 *
 * \param[in,out]   grid    main widget grid
 *
 * \return  number of rows used in the \a grid
 */
static int create_vic20_layout(GtkGrid *grid)
{
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_1, "Control port"),
            0, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_3,
                "Userport joystick adapter port 1"),
            0, 1, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_4,
                "Userport joystick adapter port 2"),
            1, 1, 1, 1);
    return 2;
}


/** \brief  Create layout for x64, x64sc, xscpu64 and x128
 *
 * Two control ports, two userport adapter ports and one SIDCard control port.
 *
 * \param[in,out]   grid    main widget grid
 *
 * \return  number of rows used in the \a grid
 */
static int create_plus4_layout(GtkGrid *grid)
{
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_1, "Control port 1"),
            0, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_2, "Control port 2"),
            1, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_3,
                "Userport joystick adapter port 1"),
            0, 1, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_4,
                "Userport joystick adapter port 2"),
            1, 1, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_5, "SIDCard control port"),
            0, 2, 1, 1);
    return 3;
}


/** \brief  Create layout for xcbm5x0
 *
 * Two control ports.
 *
 * \param[in,out]   grid    main widget grid
 *
 * \return  number of rows used in the \a grid
 */
static int create_cbm5x0_layout(GtkGrid *grid)
{
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_1, "Control port 1"),
            0, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_2, "Control port 2"),
            1, 0, 1, 1);
    return 1;
}


/** \brief  Create layout for xcmb2 and xpet
 *
 * Two userport adapter ports.
 *
 * \param[in,out]   grid    main widget grid
 *
 * \return  number of rows used in the \a grid
 */
static int create_cbm6x0_layout(GtkGrid *grid)
{
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_3,
                "Userport joystick adapter port 1"),
            0, 0, 1, 1);
    gtk_grid_attach(grid,
            create_joyport_widget(JOYPORT_4,
                "Userport joystick adapter port 2"),
            1, 0, 1, 1);
    return 1;
}



/** \brief  Create widget to control control ports
 *
 * Creates a widget to control the settings for the control ports, userport
 * joystick adapter ports and the SIDCard control port, depending on the
 * currently emulated machine.
 *
 * \param[in]   parent  parent widget
 *
 * \return  GtkGrid
 */
GtkWidget *settings_controlport_widget_create(GtkWidget *parent)
{
    GtkWidget *layout;
    int rows = 0;

    joyport_devices_list_init();

    layout = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(layout), 8);
    gtk_grid_set_row_spacing(GTK_GRID(layout), 8);

    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_C128:
            rows = create_c64_layout(GTK_GRID(layout));
            break;
        case VICE_MACHINE_C64DTV:
            rows = create_c64dtv_layout(GTK_GRID(layout));
            break;
        case VICE_MACHINE_VIC20:
            rows = create_vic20_layout(GTK_GRID(layout));
            break;
        case VICE_MACHINE_PLUS4:
            rows = create_plus4_layout(GTK_GRID(layout));
            break;
        case VICE_MACHINE_CBM5x0:
            rows = create_cbm5x0_layout(GTK_GRID(layout));
            break;
        case VICE_MACHINE_PET:          /* fall through */
        case VICE_MACHINE_CBM6x0:
            rows = create_cbm6x0_layout(GTK_GRID(layout));
            break;
        case VICE_MACHINE_VSID:
            break;  /* no control ports or user ports */
        default:
            break;
    }

    /* add BBRTC checkbox */
    if (rows > 0) {
        gtk_grid_attach(GTK_GRID(layout), create_bbrtc_widget(), 0, rows, 2, 1);
    }

    g_signal_connect(layout, "destroy", G_CALLBACK(on_destroy), NULL);
    gtk_widget_show_all(layout);
    return layout;
}
