/** \file   joystickmenupopup.c
 *  \brief  Create a menu to swap joysticks
 *
 * This module contains a popup menu that can be triggered via the statusbar's
 * joysticks widget. With it, the user can swap joystick ports.
 *
 *  \author Bas Wassink <b.wassink@ziggo.nl>
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
#include "debug_gtk3.h"
#include "machine.h"
#include "resources.h"
#include "uicommands.h"
#include "uisettings.h"

#include "joystickmenupopup.h"


/** \brief  Determine if joystick swapping is possible
 *
 * \return  bool
 */
static gboolean joystick_swap_possible(void)
{
    switch (machine_class) {

        /* always TRUE */
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_C128:     /* fall through */
        /* x64dtv emulates the second joystick hack by degault */
        case VICE_MACHINE_C64DTV:   /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_PLUS4:    /* fall through */
        case VICE_MACHINE_CBM5x0:
            return TRUE;

        /* always FALSE */
        case VICE_MACHINE_VIC20:    /* fall through */
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_CBM6x0:
            return FALSE;

        /* should never get here */
        default:
            return FALSE;
    }
}


/** \brief  Determine if userport joystick swapping is possible
 *
 * This function only checks if userport joystick swapping is possible,
 * ignoring if an adapter is active to actually allow userport joysticks (and
 * thus swapping)
 *
 * \return  bool
 */
static gboolean userport_joystick_swap_possible(void)
{
    switch (machine_class) {
        /* these all support userport joystick adapters with two ports */
        case VICE_MACHINE_C64:      /* fall through */
        case VICE_MACHINE_C64SC:    /* fall through */
        case VICE_MACHINE_C128:     /* fall through */
        case VICE_MACHINE_SCPU64:   /* fall through */
        case VICE_MACHINE_PLUS4:    /* fall through */
        case VICE_MACHINE_VIC20:    /* fall through */
        case VICE_MACHINE_PET:      /* fall through */
        case VICE_MACHINE_CBM6x0:
            return TRUE;

        case VICE_MACHINE_C64DTV:
            /* DTV only supports a hack for a single userport joystick */
            return FALSE;

        case VICE_MACHINE_CBM5x0:
            /* CBM-II 5x0 models don't support userport joystick adapters */
            return FALSE;

        default:
            /* shouldn't get here */
            return FALSE;
    }
}


/** \brief  Determine if a userport joystick adapter is enabled
 *
 * \return  bool
 */
static gboolean userport_joystick_adapter_enabled(void)
{
    int enabled;

    if (resources_get_int("UserportJoy", &enabled) < 0) {
        debug_gtk3("failed to get value for resource 'UserportJoy',"
                " assuming FALSE");
        enabled = 0;
    }
    return (gboolean)enabled;
}


static void on_configure_activate(GtkWidget *widget, gpointer user_data)
{
    ui_settings_dialog_create_and_activate_node("input/joystick");
}


/** \brief  Create joystick menu popup for the statusbar
 *
 * \return  GtkMenu
 */
GtkWidget *joystick_menu_popup_create(void)
{
    GtkWidget *menu;
    GtkWidget *item;

    menu = gtk_menu_new();

    if (joystick_swap_possible()) {
        item = gtk_menu_item_new_with_label("Swap joysticks");
        gtk_container_add(GTK_CONTAINER(menu), item);
        g_signal_connect(item, "activate",
                G_CALLBACK(ui_swap_joysticks_callback), NULL);
    }

    if (userport_joystick_swap_possible()) {
        item = gtk_menu_item_new_with_label("Swap userport joysticks");
        gtk_container_add(GTK_CONTAINER(menu), item);
        g_signal_connect(item, "activate",
                G_CALLBACK(ui_swap_userport_joysticks_callback), NULL);
        gtk_widget_set_sensitive(GTK_WIDGET(item),
                userport_joystick_adapter_enabled());

    }

    item = gtk_separator_menu_item_new();
    gtk_container_add(GTK_CONTAINER(menu), item);

    item = gtk_menu_item_new_with_label("Configure joysticks ...");
    gtk_container_add(GTK_CONTAINER(menu), item);
    g_signal_connect(item, "activate", G_CALLBACK(on_configure_activate),
            NULL);

    gtk_widget_show_all(menu);
    return menu;
}
