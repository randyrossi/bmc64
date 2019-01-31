/*
 * uipetreu.c
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#include "uilib.h"
#include "uimenu.h"
#include "uipetreu.h"

UI_MENU_DEFINE_TOGGLE(PETREU)
UI_MENU_DEFINE_RADIO(PETREUsize)

static UI_CALLBACK(set_petreu_image_name)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM, _("PET REU file"), UILIB_FILTER_ALL);
}

static ui_menu_entry_t petreu_size_submenu[] = {
    { "128kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_PETREUsize, (ui_callback_data_t)128, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "512kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_PETREUsize, (ui_callback_data_t)512, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1024kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_PETREUsize, (ui_callback_data_t)1024, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "2048kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_PETREUsize, (ui_callback_data_t)2048, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t petreu_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_PETREU, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Size"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, petreu_size_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Image name"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_petreu_image_name, (ui_callback_data_t)"PETREUfilename", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
