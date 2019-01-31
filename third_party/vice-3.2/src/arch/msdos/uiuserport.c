/*
 * uiuserport.c - Userport devices UI interface for MS-DOS.
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

#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uiuserport.h"

TUI_MENU_DEFINE_TOGGLE(UserportDAC)
TUI_MENU_DEFINE_TOGGLE(UserportDIGIMAX)
TUI_MENU_DEFINE_TOGGLE(UserportRTCDS1307)
TUI_MENU_DEFINE_TOGGLE(UserportRTCDS1307Save)
TUI_MENU_DEFINE_TOGGLE(UserportRTC58321a)
TUI_MENU_DEFINE_TOGGLE(UserportRTC58321aSave)
TUI_MENU_DEFINE_TOGGLE(Userport4bitSampler)
TUI_MENU_DEFINE_TOGGLE(Userport8BSS)

static tui_menu_item_def_t userport_c64_cbm2_menu_items[] = {
    { "Enable 8 bit DAC:", "Emulate 8 bit DAC",
      toggle_UserportDAC_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable DigiMAX:", "Emulate DigiMAX",
      toggle_UserportDIGIMAX_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable 4 bit sampler:", "Emulate 4 bit sampler",
      toggle_Userport4bitSampler_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable 8 bit stereo sampler:", "Emulate 8 bit stereo sampler",
      toggle_Userport8BSS_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable RTC (58321a):", "Emulate 58321a RTC",
      toggle_UserportRTC58321a_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable saving of RTC (58321a)data when changed:", "Save RTC data when changed",
      toggle_UserportRTC58321aSave_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable RTC (DS1307):", "Emulate DS1307 RTC",
      toggle_UserportRTCDS1307_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable saving of RTC (DS1307)data when changed:", "Save RTC data when changed",
      toggle_UserportRTCDS1307Save_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t userport_pet_vic20_menu_items[] = {
    { "Enable 8 bit DAC:", "Emulate 8 bit DAC",
      toggle_UserportDAC_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable RTC (58321a):", "Emulate 58321a RTC",
      toggle_UserportRTC58321a_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable saving of RTC (58321a)data when changed:", "Save RTC data when changed",
      toggle_UserportRTC58321aSave_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable RTC (DS1307):", "Emulate DS1307 RTC",
      toggle_UserportRTCDS1307_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable saving of RTC (DS1307)data when changed:", "Save RTC data when changed",
      toggle_UserportRTCDS1307Save_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t userport_plus4_menu_items[] = {
    { "Enable 8 bit DAC:", "Emulate 8 bit DAC",
      toggle_UserportDAC_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiuserport_c64_cbm2_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_userport_submenu;

    ui_userport_submenu = tui_menu_create("Userport devices", 1);

    tui_menu_add(ui_userport_submenu, userport_c64_cbm2_menu_items);

    tui_menu_add_submenu(parent_submenu, "Userport devices...",
                         "Userport devices",
                         ui_userport_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}

void uiuserport_pet_vic20_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_userport_submenu;

    ui_userport_submenu = tui_menu_create("Userport devices", 1);

    tui_menu_add(ui_userport_submenu, userport_pet_vic20_menu_items);

    tui_menu_add_submenu(parent_submenu, "Userport devices...",
                         "Userport devices",
                         ui_userport_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}

void uiuserport_plus4_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_userport_submenu;

    ui_userport_submenu = tui_menu_create("Userport devices", 1);

    tui_menu_add(ui_userport_submenu, userport_pet_vic20_menu_items);

    tui_menu_add_submenu(parent_submenu, "Userport devices...",
                         "Userport devices",
                         ui_userport_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
