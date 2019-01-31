/*
 * uicbm2cart.c - Implementation of the CBM2 cart-specific part of the UI.
 *
 * Written by
 *  Groepaz <groepaz@gmx.net>
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
#include <stdlib.h>

#include "cartridge.h"
#include "lib.h"
#include "ui.h"
#include "uiapi.h"
#include "uicbm2cart.h"
#include "uicartridge.h"
#include "uilib.h"
#include "uimenu.h"
#include "util.h"
#include "vsync.h"

static char *last_dir = NULL;

static UI_CALLBACK(attach_cartridge)
{
    int type = vice_ptr_to_int(UI_MENU_CB_PARAM);
    char *filename;
    ui_button_t button;
    uilib_file_filter_enum_t filter[] = { UILIB_FILTER_CARTRIDGE, UILIB_FILTER_ALL };

    vsync_suspend_speed_eval();

    filename = ui_select_file(_("Attach cartridge image"), NULL, 0, last_dir, filter, sizeof(filter) / sizeof(*filter), &button, 0, NULL, UI_FC_LOAD);
    switch (button) {
        case UI_BUTTON_OK:
            if (cartridge_attach_image(type, filename) < 0)
            {
                ui_error(_("Cannot attach cartridge"));
            }
            lib_free(last_dir);
            util_fname_split(filename, &last_dir, NULL);
            break;
        default:
            /* Do nothing special. */
            break;
    }
    lib_free(filename);

    ui_update_menus();
}

static UI_CALLBACK(detach_cartridge)
{
    cartridge_detach_image(-1);
    ui_update_menus();
}

/*
static UI_CALLBACK(default_cartridge)
{
    cartridge_set_default();
}
*/
/*
static UI_CALLBACK(freeze_cartridge)
{
    cartridge_trigger_freeze();
}
*/

/*
    FIXME: ideally attaching a binary image should be done
           through an extended file selector, which lets you
           pick the actual cart type from a list.

    FIXME: names should ideally be taken from cartridge.h
*/
static ui_menu_entry_t attach_cartridge_image_submenu[] = {
/*
    { N_("Smart-attach cartridge image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_CBM2_DETECT, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
*/
    { N_("Load new Cart $1***"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_CBM2_8KB_1000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Unload Cart $1***"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)detach_cartridge, (ui_callback_data_t)CARTRIDGE_CBM2_8KB_1000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Load new Cart $2-3***"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_CBM2_8KB_2000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Unload Cart $2-3***"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)detach_cartridge, (ui_callback_data_t)CARTRIDGE_CBM2_8KB_2000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Load new Cart $4-5***"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_CBM2_16KB_4000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Unload Cart $4-5***"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)detach_cartridge, (ui_callback_data_t)CARTRIDGE_CBM2_16KB_4000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Load new Cart $6-7***"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_CBM2_16KB_6000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Unload Cart $6-7***"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)detach_cartridge, (ui_callback_data_t)CARTRIDGE_CBM2_16KB_6000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t ui_cbm2cart_commands_menu[] = {
    { N_("Attach cartridge image"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, attach_cartridge_image_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Detach cartridge image(s)"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)detach_cartridge, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};


/** \brief  For symmetry, doesn't do anything (yet)
 */
void uicbm2cart_menu_create(void)
{
    /* NOP */
}

/** \brief  Clean up memory used by the UI
 */
void uicbm2cart_menu_shutdown(void)
{
    if (last_dir != NULL) {
        lib_free(last_dir);
    }
}

