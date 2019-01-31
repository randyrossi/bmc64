/*
 * uisoundsampler.c - SFX Sound Sampler cartridge UI interface for MS-DOS.
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
#include "uisoundsampler.h"

TUI_MENU_DEFINE_TOGGLE(SFXSoundSampler)
TUI_MENU_DEFINE_TOGGLE(SFXSoundSamplerIOSwap)

static tui_menu_item_def_t soundsampler_menu_items[] = {
    { "_Enable SFX Sound Sampler:", "Emulate SFX Sound Sampler Cartridge",
      toggle_SFXSoundSampler_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable MasC=uerade I/O swap:", "Map SFX SS I/O to vic20 I/O-3",
      toggle_SFXSoundSamplerIOSwap_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uisoundsampler_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_soundsampler_submenu;

    ui_soundsampler_submenu = tui_menu_create("SFX Sound Sampler settings (MasC=uerade)", 1);

    tui_menu_add(ui_soundsampler_submenu, soundsampler_menu_items);

    tui_menu_add_submenu(parent_submenu, "SFX Sound _Sampler settings (MasC=uerade)...",
                         "SFX Sound Sampler settings",
                         ui_soundsampler_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
