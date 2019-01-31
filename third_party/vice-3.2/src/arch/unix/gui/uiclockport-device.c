/*
 * uiclockport-device.c - Dynamic submenu generation of clockport device settings.
 *
 * Written by
 *   Bas Wassink <b.wassink@ziggo.nl>
 *
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

#include <stdio.h>

#include "lib.h"
#include "clockport.h"
#include "uilib.h"
#include "uimenu.h"

#include "uiclockport-device.h"

/** \brief  Generate a submenu containing clockport devices
 *
 * \param[in]   callback    callback that sets the clockport device id
 *
 * \return  new submenu
 */
ui_menu_entry_t *uiclockport_device_menu_create(ui_callback_t callback)
{
    ui_menu_entry_t *menu = NULL;
    unsigned int i = 0;
    unsigned int num;

    /* determine number of entries required */
    while (clockport_supported_devices[i].id >= 0
            && clockport_supported_devices[i].name != NULL) {
        i++;
    }
    num = i;

    menu = lib_calloc((size_t)(num + 1), sizeof *menu);

    for (i = 0; i < num; i++) {
        int id = clockport_supported_devices[i].id;
        menu[i].string = (ui_callback_data_t)clockport_supported_devices[i].name;
        menu[i].type = UI_MENU_TYPE_TICK;
        menu[i].callback = callback;
        menu[i].callback_data = (ui_callback_data_t)(int_to_void_ptr(id));
    }

    return menu;
}

/** \brief  Clean up memory used by \a menu
 */
void uiclockport_device_menu_shutdown(ui_menu_entry_t *menu)
{
    lib_free(menu);
}

