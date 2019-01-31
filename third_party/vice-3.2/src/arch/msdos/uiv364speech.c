/*
 * uiv364speech.c - V364 speech UI interface for MS-DOS.
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
#include <string.h>

#include "lib.h"
#include "resources.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "uiv364speech.h"
#include "util.h"

TUI_MENU_DEFINE_TOGGLE(SpeechEnabled)
TUI_MENU_DEFINE_FILENAME(SpeechImage, "V364 speech ROM")

static tui_menu_item_def_t v364speech_menu_items[] = {
    { "_Enable V364 speech:", "Emulate V364 speech",
      toggle_SpeechEnabled_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "V364 speech _ROM file:", "Select the V364 speech ROM file",
      filename_SpeechImage_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiv364speech_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_v364speech_submenu;

    ui_v364speech_submenu = tui_menu_create("V364 speech settings", 1);

    tui_menu_add(ui_v364speech_submenu, v364speech_menu_items);

    tui_menu_add_submenu(parent_submenu, "_V364 speech settings...",
                         "V364 speech settings",
                         ui_v364speech_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
