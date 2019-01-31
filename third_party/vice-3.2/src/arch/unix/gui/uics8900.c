/*
 * uics8900.c - CS8900 submenu
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
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
#include "util.h"
#include "cartridge.h"
#include "uiapi.h"
#include "uicartridge.h"
#include "uilib.h"
#include "uimenu.h"

#include "uics8900.h"


/** \brief  Callback for setting the ethernet interface
 */
static UI_CALLBACK(uics8900_set_interface_name)
{
    char *name = util_concat(_("Name"), ":", NULL);

    uilib_select_string((char *)UI_MENU_CB_PARAM, _("Ethernet interface"), name);
    lib_free(name);
}


ui_menu_entry_t uics8900_submenu[] = {
    { N_("Interface"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)uics8900_set_interface_name, (ui_callback_data_t)"ETHERNET_INTERFACE", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

