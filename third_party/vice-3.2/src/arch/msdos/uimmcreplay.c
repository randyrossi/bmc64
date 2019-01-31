/*
 * uimmcreplay.c - MMC Replay UI interface for MS-DOS.
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

#include "clockport.h"
#include "lib.h"
#include "resources.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "uimmcreplay.h"
#include "util.h"

TUI_MENU_DEFINE_TOGGLE(MMCRCardRW)
TUI_MENU_DEFINE_TOGGLE(MMCRImageWrite)
TUI_MENU_DEFINE_TOGGLE(MMCREEPROMRW)
TUI_MENU_DEFINE_TOGGLE(MMCRRescueMode)
TUI_MENU_DEFINE_RADIO(MMCRSDType)
TUI_MENU_DEFINE_RADIO(MMCRClockPort)
TUI_MENU_DEFINE_FILENAME(MMCREEPROMImage, "MMC Replay EEPROM")
TUI_MENU_DEFINE_FILENAME(MMCRCardImage, "MMC Replay MMC/SD")

static TUI_MENU_CALLBACK(mmcreplay_sd_type_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("MMCRSDType", &value);
    switch (value) {
        default:
        case 0:
            s = "Auto";
            break;
        case 1:
            s = "MMC";
            break;
        case 2:
            s = "SD";
            break;
        case 3:
            s = "SDHC";
            break;
    }
    return s;
}

static tui_menu_item_def_t mmcreplay_sd_type_submenu[] = {
    { "_Auto", NULL, radio_MMCRSDType_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "MM_C", NULL, radio_MMCRSDType_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_SD", NULL, radio_MMCRSDType_callback,
      (void *)2, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "SD_HC", NULL, radio_MMCRSDType_callback,
      (void *)3, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(mmcr_clockport_submenu_callback)
{
    int value;
    char *s = NULL;
    int i;

    resources_get_int("MMCRClockPort", &value);
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

static tui_menu_item_def_t mmcr_clockport_submenu[CLOCKPORT_MAX_ENTRIES + 1];

static tui_menu_item_def_t mmcreplay_menu_items[] = {
    { "MMC Replay EEPROM read/_write:", "Enable MMC Replay EEPROM read/write",
      toggle_MMCREEPROMRW_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Save MMC Replay _EEPROM when changed:", "Enable MMC Replay EEPROM save when changed",
      toggle_MMCRImageWrite_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "MMC Replay EE_PROM file:", "Select the MMC Replay EEPROM file",
      filename_MMCREEPROMImage_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "MMC Replay rescue mode:", "Enable MMC Replay rescue mode",
      toggle_MMCRRescueMode_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "MMC Replay MMC/SD image _read/write:", "Enable MMC Replay MMC/SD image read/write",
      toggle_MMCRCardRW_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "MMC Replay _cart type:", "Select the cart type",
      mmcreplay_sd_type_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, mmcreplay_sd_type_submenu,
      "MMC Replay card type" },
    { "MMC Replay MMC/SD i_mage file:", "Select the MMC Replay MMC/SD image file",
      filename_MMCRCardImage_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "MMC Replay _clockport device:", "Select the clockport device",
      mmcr_clockport_submenu_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, mmcr_clockport_submenu,
      "MMC Replay clockport device" },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uimmcreplay_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_mmcreplay_submenu;
    int i;

    for (i = 0; clockport_supported_devices[i].name; ++i) {
        mmcr_clockport_submenu[i].label = clockport_supported_devices[i].name;
        mmcr_clockport_submenu[i].help_string = NULL;
        mmcr_clockport_submenu[i].callback = radio_MMCRClockPort_callback;
        mmcr_clockport_submenu[i].callback_param = (void *)clockport_supported_devices[i].id;
        mmcr_clockport_submenu[i].par_string_max_len = 20;
        mmcr_clockport_submenu[i].behavior = TUI_MENU_BEH_CLOSE;
        mmcr_clockport_submenu[i].submenu = NULL;
        mmcr_clockport_submenu[i].submenu_title = NULL;
    }

    mmcr_clockport_submenu[i].label = NULL;
    mmcr_clockport_submenu[i].help_string = NULL;
    mmcr_clockport_submenu[i].callback = NULL;
    mmcr_clockport_submenu[i].callback_param = NULL;
    mmcr_clockport_submenu[i].par_string_max_len = 0;
    mmcr_clockport_submenu[i].behavior = TUI_MENU_BEH_CLOSE;
    mmcr_clockport_submenu[i].submenu = NULL;
    mmcr_clockport_submenu[i].submenu_title = NULL;

    ui_mmcreplay_submenu = tui_menu_create("MMC Replay settings", 1);

    tui_menu_add(ui_mmcreplay_submenu, mmcreplay_menu_items);

    tui_menu_add_submenu(parent_submenu, "MMC _Replay settings...",
                         "MMC Replay settings",
                         ui_mmcreplay_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
