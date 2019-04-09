/** \file   joystickdevicewidget.c
 * \brief   Widget to select a joystick device
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES JoyDevice1      -xcbm2 -xpet -vsid
 * $VICERES JoyDevice2      -xcbm2 -xpet -vsid
 * $VICERES JoyDevice3      -xcbm5x0 -vsid
 * $VICERES JoyDevice4      -xcbm5x0 -xplus4 -vsid
 * $VICERES JoyDevice5      xplus4
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
#include <stdlib.h>

#include "lib.h"
#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "machine.h"
#include "resources.h"
#include "joystick.h"
#include "joy.h"

#include "joystickdevicewidget.h"


/** \brief  Struct containing device name and id
 */
typedef struct device_info_s {
    const char *name;   /**< device name */
    int         id;     /**< device ID (\see joy.h) */
} device_info_t;

#define MAX_EXTRA_DEVICES 32

/** \brief  List of detected input devices on the host
 */
static device_info_t device_list[MAX_EXTRA_DEVICES] = {
    { NULL, -1 }
};
/** \brief  List of available input devices on the host
 */
static device_info_t predefined_device_list[] = {
    { "None",               JOYDEV_NONE },
    { "Numpad",             JOYDEV_NUMPAD },
    { "Keyset A",           JOYDEV_KEYSET1 },
    { "Keyset B",           JOYDEV_KEYSET2 },
    { NULL, -1 }
};


/** \brief  Handler for the "changed" event of the combo box
 *
 * \param[in]   combo       combo box
 * \param[in]   user_data   device number (0-4) (int`)
 */
static void on_device_changed(GtkComboBoxText *combo, gpointer user_data)
{
    const char *id_str;
    int id_val;
    char *endptr;
    int device;

    id_str = gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo));
    id_val = (int)strtol(id_str, &endptr, 10);
    device = GPOINTER_TO_INT(user_data) + 1;

    if (*endptr == '\0') {
        debug_gtk3("setting JoyDevice%d to %d.", device, id_val);
        resources_set_int_sprintf("JoyDevice%d", id_val, device);
    }
}


/** \brief  Create joystick device selection widget
 *
 * \param[in]   device  device number (0-4)
 * \param[in]   title   widget title
 *
 * \return  GtkGrid
 */
GtkWidget *joystick_device_widget_create(int device, const char *title)
{
    GtkWidget *grid;
    GtkWidget *combo;
    int id, i1, i2;
    int current;

    resources_get_int_sprintf("JoyDevice%d", &current, device + 1);

    grid = uihelpers_create_grid_with_label(title, 1);

    combo = gtk_combo_box_text_new();
    g_object_set(combo, "margin-left", 16, NULL);
    /* add predefined standard devices */
    for (i1 = 0; predefined_device_list[i1].name != NULL; i1++) {
        char idstr[32];

        g_snprintf(idstr, 32, "%d", predefined_device_list[i1].id);
        debug_gtk3("%d %s.",
                predefined_device_list[i1].id,
                predefined_device_list[i1].name);

        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo),
                idstr, predefined_device_list[i1].name);
        if (predefined_device_list[i1].id == current) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i1);
        }
    }
    /* add more devices (joysticks) */
    joystick_ui_reset_device_list();
    for (i2 = 0; (device_list[i2].name = joystick_ui_get_next_device_name(&id)) != NULL; i2++) {
        char idstr[32];

        if (i2 >= MAX_EXTRA_DEVICES) {
            break;
        }

        device_list[i2].id = id;
        g_snprintf(idstr, 32, "%d", device_list[i2].id);
        debug_gtk3( "%d %s\n", device_list[i2].id, device_list[i2].name);

        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo), idstr, device_list[i2].name);
        if (device_list[i2].id == current) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i1+i2);
        }
    }

    g_signal_connect(combo, "changed", G_CALLBACK(on_device_changed), GINT_TO_POINTER(device));

    gtk_grid_attach(GTK_GRID(grid), combo, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Set joystick device \a widget to \a id
 *
 * \param]in,out]   widget  joystick device widget
 * \param[in]       id      new value for the \a widget
 */
void joystick_device_widget_update(GtkWidget *widget, int id)
{
    char id_str[32];
    GtkWidget *combo;

    /* turn device_id into key for the combo box */
    g_snprintf(id_str, 32, "%d", id);

    /* get combo box widget */
    combo = gtk_grid_get_child_at(GTK_GRID(widget), 0, 1);
    if (combo != NULL && GTK_IS_COMBO_BOX_TEXT(combo)) {
        debug_gtk3("updating widget to %d.", id);
        gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo), id_str);
    }
}
