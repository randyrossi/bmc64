/** \file   petkeyboardtypewidget.c
 * \brief   PET keyboard type widget
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES KeyboardType
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

#include "widgethelpers.h"
#include "debug_gtk3.h"
#include "resources.h"
#include "machine.h"
#include "pet.h"

#include "petkeyboardtypewidget.h"


static int (*get_keyboard_num)(void) = NULL;
static kbdtype_info_t *(*get_keyboard_list)(void) = NULL;

static void (*user_callback)(int) = NULL;


/** \brief  Handler for the "toggled" event of the radio buttons
 *
 * \param[in]   widget      radio button triggering the event
 * \param[in]   user_data   keyboard ID (`int`)
 */
static void on_keyboard_type_toggled(GtkWidget *widget, gpointer user_data)
{
    int old_type;
    int new_type = GPOINTER_TO_INT(user_data);

    resources_get_int("KeyboardType", &old_type);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))
            && old_type != new_type) {
        /* update resource */
        debug_gtk3("setting KeyboardType to %d.", new_type);
        resources_set_int("KeyboardType", new_type);

        if (user_callback != NULL) {
            debug_gtk3("calling user callback with %d.", new_type);
            user_callback(new_type);
        }
    }
}


/** \brief  Set the getter function for the number of keyboard types
 *
 * This doesn't seem strictly required for PET, since the keyboard info lisr
 * has a terminator
 *
 * \param[in]   f   function pointer
 */
void pet_keyboard_type_widget_set_keyboard_num_get(int (*f)(void))
{
    get_keyboard_num = f;
}


/** \brief  Set the getter function for the keyboard types list
 *
 * \param[in]   f   function pointer
 */
void pet_keyboard_type_widget_set_keyboard_list_get(kbdtype_info_t *(*f)(void))
{
    get_keyboard_list = f;
}


/** \brief  Create PET keyboard type widget
 *
 * \return  GtkGrid
 */
GtkWidget * pet_keyboard_type_widget_create(void)
{
    GtkWidget *grid;
    int num;
    kbdtype_info_t *list;
    GSList *group = NULL;
    GtkRadioButton *last = NULL;

    user_callback = NULL;

    grid = uihelpers_create_grid_with_label("Keyboard type", 1);

    num = get_keyboard_num();
    debug_gtk3("number of keyboards = %d.", num);
    if (num > 0) {
        GtkWidget *radio;
        int active, i;

        list = get_keyboard_list();
        resources_get_int("KeyboardType", &active);

        for (i = 0; i < num; i++) {
            radio = gtk_radio_button_new_with_label(group, list[i].name);
            g_object_set(radio, "margin-left", 16, NULL);
            gtk_radio_button_join_group(GTK_RADIO_BUTTON(radio), last);
            if (active == i) {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
            }
            g_signal_connect(radio, "toggled",
                    G_CALLBACK(on_keyboard_type_toggled),
                    GINT_TO_POINTER(list[i].type));

            gtk_grid_attach(GTK_GRID(grid), radio, 0, i + 1, 1, 1);
            last = GTK_RADIO_BUTTON(radio);
        }
    }

    gtk_widget_show_all(grid);
    return grid;
}


/** \brief  Set user-defined callback to be triggered when the widget changes
 *
 * \param[in]   widget  PET keyboard type widget
 * \param[in]   func    user-defined callback
 */
void pet_keyboard_type_widget_set_callback(GtkWidget *widget,
                                           void(*func)(int))
{
    user_callback = func;
}


/** \brief  Synchronize \a widget with its current resource value
 *
 * \param[in,out]   widget  PET keyboard type widget
 */
void pet_keyboard_type_widget_sync(GtkWidget *widget)
{
    int type;
    GtkWidget *radio;

    if (resources_get_int("KeyboardType", &type) < 0) {
        return;
    }
    radio = gtk_grid_get_child_at(GTK_GRID(widget), 0, type + 1);
    if (GTK_IS_RADIO_BUTTON(radio)) {
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(radio), TRUE);
    }
}
