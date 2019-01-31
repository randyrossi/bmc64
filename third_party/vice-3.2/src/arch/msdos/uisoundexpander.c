/*
 * uisoundexpander.c - SFX Sound Expander cartridge UI interface for MS-DOS.
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
#include "uisoundexpander.h"

TUI_MENU_DEFINE_TOGGLE(SFXSoundExpander)
TUI_MENU_DEFINE_TOGGLE(SFXSoundExpanderIOSwap)
TUI_MENU_DEFINE_RADIO(SFXSoundExpanderChip)

static TUI_MENU_CALLBACK(soundexpander_chip_submenu_callback)
{
    int value;

    resources_get_int("SFXSoundExpanderChip", &value);
    if (value == 3526) {
      return "3526";
    }
    return "3812";
}

static tui_menu_item_def_t soundexpander_chip_submenu[] = {
    { "3526", NULL, radio_SFXSoundExpanderChip_callback,
      (void *)3526, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "3812", NULL, radio_SFXSoundExpanderChip_callback,
      (void *)3812, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t soundexpander_c64_menu_items[] = {
    { "_Enable SFX Sound Expander:", "Emulate SFX Sound Expander Cartridge",
      toggle_SFXSoundExpander_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "SFX Sound Expander _chip:", "Select the YM chip model used in the SFX Sound Expander Cartridge",
      soundexpander_chip_submenu_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, soundexpander_chip_submenu,
      "SFX Sound Expander chip" },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t soundexpander_vic20_menu_items[] = {
    { "_Enable SFX Sound Expander:", "Emulate SFX Sound Expander Cartridge",
      toggle_SFXSoundExpander_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable MasC=uerade I/O swap:", "Map SFX SE I/O to vic20 I/O-3",
      toggle_SFXSoundExpanderIOSwap_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "SFX Sound Expander _chip:", "Select the YM chip model used in the SFX Sound Expander Cartridge",
      soundexpander_chip_submenu_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, soundexpander_chip_submenu,
      "SFX Sound Expander chip" },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uisoundexpander_c64_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_soundexpander_submenu;

    ui_soundexpander_submenu = tui_menu_create("SFX Sound Expander settings", 1);

    tui_menu_add(ui_soundexpander_submenu, soundexpander_c64_menu_items);

    tui_menu_add_submenu(parent_submenu, "SFX Sound _Expander settings...",
                         "SFX Sound Expander settings",
                         ui_soundexpander_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}

void uisoundexpander_vic20_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_soundexpander_submenu;

    ui_soundexpander_submenu = tui_menu_create("SFX Sound Expander settings (MasC=uerade)", 1);

    tui_menu_add(ui_soundexpander_submenu, soundexpander_vic20_menu_items);

    tui_menu_add_submenu(parent_submenu, "SFX Sound _Expander settings (MasC=uerade)...",
                         "SFX Sound Expander settings",
                         ui_soundexpander_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
