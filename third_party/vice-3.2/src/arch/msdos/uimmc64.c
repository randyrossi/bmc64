/*
 * uimc64.c - MMC64 UI interface for MS-DOS.
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
#include "uimmc64.h"
#include "util.h"

TUI_MENU_DEFINE_TOGGLE(MMC64)
TUI_MENU_DEFINE_RADIO(MMC64_revision)
TUI_MENU_DEFINE_TOGGLE(MMC64_flashjumper)
TUI_MENU_DEFINE_TOGGLE(MMC64_bios_write)
TUI_MENU_DEFINE_TOGGLE(MMC64_RO)
TUI_MENU_DEFINE_RADIO(MMC64_sd_type)
TUI_MENU_DEFINE_RADIO(MMC64ClockPort)
TUI_MENU_DEFINE_FILENAME(MMC64BIOSfilename, "MMC64 BIOS")
TUI_MENU_DEFINE_FILENAME(MMC64imagefilename, "MMC64 MMC/SD")

static TUI_MENU_CALLBACK(mmc64_revision_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("MMC64_revision", &value);
    s = (value) ? "Rev B" : "Rev A";
    return s;
}

static tui_menu_item_def_t mmc64_revision_submenu[] = {
    { "Rev _A", NULL, radio_MMC64_revision_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Rev _B", NULL, radio_MMC64_revision_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(mmc64_sd_type_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("MMC64_sd_type", &value);
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

static tui_menu_item_def_t mmc64_sd_type_submenu[] = {
    { "_Auto", NULL, radio_MMC64_sd_type_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "MM_C", NULL, radio_MMC64_sd_type_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_SD", NULL, radio_MMC64_sd_type_callback,
      (void *)2, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "SD_HC", NULL, radio_MMC64_sd_type_callback,
      (void *)3, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(mmc64_clockport_submenu_callback)
{
    int value;
    char *s = NULL;
    int i;

    resources_get_int("MMC64ClockPort", &value);
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

static tui_menu_item_def_t mmc64_clockport_submenu[CLOCKPORT_MAX_ENTRIES + 1];

static tui_menu_item_def_t mmc64_menu_items[] = {
    { "_Enable MMC64:", "Emulate MMC64",
      toggle_MMC64_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_MMC64 Revision:", "Select the MMC64 Revision",
      mmc64_revision_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, mmc64_revision_submenu,
      "MMC64 Revision" },
    { "Enable MMC64 _flashjumper:", "Enable MMC64 flashjumper",
      toggle_MMC64_flashjumper_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Save MMC64 _BIOS when changed:", "Enable MMC64 BIOS save when changed",
      toggle_MMC64_bios_write_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "MMC64 B_IOS file:", "Select the MMC64 BIOS file",
      filename_MMC64BIOSfilename_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "MMC64 image _read-only:", "Enable MMC64 MMC/SD image read-only",
      toggle_MMC64_RO_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "MMC64 _cart type:", "Select the cart type",
      mmc64_sd_type_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, mmc64_sd_type_submenu,
      "MMC64 card type" },
    { "MMC64 i_mage file:", "Select the MMC64 MMC/SD image file",
      filename_MMC64imagefilename_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "MMC64 _clockport device:", "Select the clockport device",
      mmc64_clockport_submenu_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, mmc64_clockport_submenu,
      "MMC64 clockport device" },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uimmc64_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_mmc64_submenu;
    int i;

    for (i = 0; clockport_supported_devices[i].name; ++i) {
        mmc64_clockport_submenu[i].label = clockport_supported_devices[i].name;
        mmc64_clockport_submenu[i].help_string = NULL;
        mmc64_clockport_submenu[i].callback = radio_MMC64ClockPort_callback;
        mmc64_clockport_submenu[i].callback_param = (void *)clockport_supported_devices[i].id;
        mmc64_clockport_submenu[i].par_string_max_len = 20;
        mmc64_clockport_submenu[i].behavior = TUI_MENU_BEH_CLOSE;
        mmc64_clockport_submenu[i].submenu = NULL;
        mmc64_clockport_submenu[i].submenu_title = NULL;
    }

    mmc64_clockport_submenu[i].label = NULL;
    mmc64_clockport_submenu[i].help_string = NULL;
    mmc64_clockport_submenu[i].callback = NULL;
    mmc64_clockport_submenu[i].callback_param = NULL;
    mmc64_clockport_submenu[i].par_string_max_len = 0;
    mmc64_clockport_submenu[i].behavior = TUI_MENU_BEH_CLOSE;
    mmc64_clockport_submenu[i].submenu = NULL;
    mmc64_clockport_submenu[i].submenu_title = NULL;

    ui_mmc64_submenu = tui_menu_create("MMC64 settings", 1);

    tui_menu_add(ui_mmc64_submenu, mmc64_menu_items);

    tui_menu_add_submenu(parent_submenu, "_MMC64 settings...",
                         "MMC64 settings",
                         ui_mmc64_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
