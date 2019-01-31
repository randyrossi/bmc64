/*
 * uiretroreplay.c - Retro Replay UI interface for MS-DOS.
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

#include "clockport.h"
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uiretroreplay.h"

TUI_MENU_DEFINE_TOGGLE(RRFlashJumper)
TUI_MENU_DEFINE_TOGGLE(RRBankJumper)
TUI_MENU_DEFINE_RADIO(RRrevision)
TUI_MENU_DEFINE_TOGGLE(RRBiosWrite)
TUI_MENU_DEFINE_RADIO(RRClockPort)

static TUI_MENU_CALLBACK(retroreplay_revision_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("RRrevision", &value);
    switch (value) {
        default:
        case 0:
            s = "Retro Replay";
            break;
        case 1:
            s = "Nordic Replay";
            break;
    }
    return s;
}

static tui_menu_item_def_t retroreplay_revision_submenu[] = {
    { "_Retro Replay", NULL, radio_RRrevision_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Nordic Replay", NULL, radio_RRrevision_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(retroreplay_clockport_submenu_callback)
{
    int value;
    char *s = NULL;
    int i;

    resources_get_int("RRClockPort", &value);
    for (i = 0; clockport_supported_devices[i].name; ++i) {
        if (clockport_supported_devices[i].id == value) {
            s = clockport_supported_devices[i].name;
        }
    }

    if (!s) {
        s = "Unknown";
    }

    return s;
}

static tui_menu_item_def_t retroreplay_clockport_submenu[CLOCKPORT_MAX_ENTRIES + 1];

static tui_menu_item_def_t retroreplay_menu_items[] = {
    { "Retro Replay flash jumper:", "Enable Retro Replay flash jumper",
      toggle_RRFlashJumper_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Retro Replay bank jumper:", "Enable Retro Replay bank jumper",
      toggle_RRBankJumper_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Retro Replay revision:", "Select the revision",
      retroreplay_revision_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, retroreplay_revision_submenu,
      "Retro Replay revision" },
    { "Save Retro Replay BIOS when changed:", "Enable Retro Replay BIOS save when changed",
      toggle_RRBiosWrite_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Retro Replay _clockport device:", "Select the clockport device",
      retroreplay_clockport_submenu_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, retroreplay_clockport_submenu,
      "Retro Replay clockport device" },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiretroreplay_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_retroreplay_submenu;
    int i;

    for (i = 0; clockport_supported_devices[i].name; ++i) {
        retroreplay_clockport_submenu[i].label = clockport_supported_devices[i].name;
        retroreplay_clockport_submenu[i].help_string = NULL;
        retroreplay_clockport_submenu[i].callback = radio_RRClockPort_callback;
        retroreplay_clockport_submenu[i].callback_param = (void *)clockport_supported_devices[i].id;
        retroreplay_clockport_submenu[i].par_string_max_len = 20;
        retroreplay_clockport_submenu[i].behavior = TUI_MENU_BEH_CLOSE;
        retroreplay_clockport_submenu[i].submenu = NULL;
        retroreplay_clockport_submenu[i].submenu_title = NULL;
    }

    retroreplay_clockport_submenu[i].label = NULL;
    retroreplay_clockport_submenu[i].help_string = NULL;
    retroreplay_clockport_submenu[i].callback = NULL;
    retroreplay_clockport_submenu[i].callback_param = NULL;
    retroreplay_clockport_submenu[i].par_string_max_len = 0;
    retroreplay_clockport_submenu[i].behavior = TUI_MENU_BEH_CLOSE;
    retroreplay_clockport_submenu[i].submenu = NULL;
    retroreplay_clockport_submenu[i].submenu_title = NULL;

    ui_retroreplay_submenu = tui_menu_create("Retro Replay settings", 1);

    tui_menu_add(ui_retroreplay_submenu, retroreplay_menu_items);

    tui_menu_add_submenu(parent_submenu, "Retro _Replay settings...",
                         "Retro Replay settings",
                         ui_retroreplay_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
