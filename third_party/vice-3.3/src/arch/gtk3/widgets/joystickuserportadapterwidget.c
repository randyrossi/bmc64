/** \file   joystickuserportadapterwidget.c
 * \brief   Widget to select a userport joystick adapter
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES UserportJoyType     -xcbm5x0 -vsid
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
#include "userport_joystick.h"

#include "joystickuserportadapterwidget.h"


/** \brief  Struct containing adapter name and id
 */
typedef struct adapter_info_s {
    const char *name;   /**< device name */
    int         id;     /**< device ID (\see joy.h) */
} adapter_info_t;


/** \brief  List of available adapters for x64/x64sc/xscpu64/x128
 * */
static adapter_info_t adapter_list_c64[] = {
    { "Classical Games/Protovision", USERPORT_JOYSTICK_CGA },
    { "PET", USERPORT_JOYSTICK_PET },
    { "Hummer", USERPORT_JOYSTICK_HUMMER },
    { "OEM", USERPORT_JOYSTICK_OEM },
    { "Digital Excess/Hitmen", USERPORT_JOYSTICK_HIT },
    { "Kingsoft", USERPORT_JOYSTICK_KINGSOFT },
    { "Starbyte", USERPORT_JOYSTICK_STARBYTE },
    { NULL, -1 }
};


/** \brief  List of available adapters for xvic/xpet/xplus4/xcbm
 * */
static adapter_info_t adapter_list_other[] = {
    { "Classical Games/Protovision", USERPORT_JOYSTICK_CGA },
    { "PET", USERPORT_JOYSTICK_PET },
    { "Hummer", USERPORT_JOYSTICK_HUMMER },
    { "OEM", USERPORT_JOYSTICK_OEM },
    { NULL, -1 }
};


/** \brief  Empty list, used as a safety measure
 *
 * If an empty combo box shows up in the main joystick settings widget, there's
 * something wrong with the code in uijoystick.c.
 */
static adapter_info_t adapter_list_empty[] = {
    { NULL, -1 }
};


/** \brief  Handler for the "changed" event of the combo box
 *
 * \param[in]   combo       combo box
 * \param[in]   user_data   device number (0-4) (int`)
 */
static void on_adapter_changed(GtkComboBoxText *combo, gpointer user_data)
{
    const char *id_str;
    int id_val;
    char *endptr;

    id_str = gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo));
    id_val = (int)strtol(id_str, &endptr, 10);

    if (*endptr == '\0') {
        debug_gtk3("setting UserportJoyType to %d.", id_val);
        resources_set_int("UserportJoyType", id_val);
    }
}


/** \brief  Create joystick device selection widget
 *
 * \param[in]   device  device number (0-4)
 * \param[in]   title   widget title
 *
 * \return  GtkGrid
 */
GtkWidget *joystick_userport_adapter_widget_create(void)
{
    GtkWidget *grid;
    GtkWidget *combo;
    adapter_info_t *list;
    int current;
    int i;

    /* determine the list of available adapters based on machine class */
    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_C128:
            list = adapter_list_c64;
            break;
        case VICE_MACHINE_CBM6x0:   /* fall through */
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_PLUS4:    /* fall through */
        case VICE_MACHINE_VIC20:
            list = adapter_list_other;
            break;
        case VICE_MACHINE_C64DTV:   /* fall through */
        case VICE_MACHINE_CBM5x0:   /* fall through */
        case VICE_MACHINE_VSID:
            list = adapter_list_empty;
            break;
        default:
            list = adapter_list_empty;
    }


    /* get current value for resource */
    resources_get_int("UserportJoyType", &current);

    grid = uihelpers_create_grid_with_label("Userport adapter", 1);

    combo = gtk_combo_box_text_new();
    g_object_set(combo, "margin-left", 16, NULL);
    for (i = 0; list[i].name != NULL; i++) {
        char id[32];

        g_snprintf(id, 32, "%d", list[i].id);

        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo),
                id, list[i].name);
        if (list[i].id == current) {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo), i);
        }
    }
    g_signal_connect(combo, "changed", G_CALLBACK(on_adapter_changed), NULL);

    gtk_grid_attach(GTK_GRID(grid), combo, 0, 1, 1, 1);
    gtk_widget_show_all(grid);
    return grid;
}
