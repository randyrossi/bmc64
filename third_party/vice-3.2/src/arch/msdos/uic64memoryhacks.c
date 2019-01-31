/*
 * uic64memoryhacks.c - C64 256k/+60K/+256K EXPANSION UI interface for MS-DOS.
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

#include "c64-memory-hacks.h"
#include "lib.h"
#include "resources.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "uic64memoryhacks.h"
#include "util.h"

TUI_MENU_DEFINE_RADIO(MemoryHack)
TUI_MENU_DEFINE_RADIO(C64_256Kbase)
TUI_MENU_DEFINE_FILENAME(C64_256Kfilename, "256K")
TUI_MENU_DEFINE_RADIO(PLUS60Kbase)
TUI_MENU_DEFINE_FILENAME(PLUS60Kfilename, "PLUS60K")
TUI_MENU_DEFINE_FILENAME(PLUS256Kfilename, "PLUS256K")

static TUI_MENU_CALLBACK(memory_hack_device_submenu_callback)
{
    char *s;
    int value;

    resources_get_int("MemoryHack", &value);
    switch (value) {
        default:
        case MEMORY_HACK_NONE:
            s = "None";
            break;
        case MEMORY_HACK_C64_256K:
            s = "C64 256K";
            break;
        case MEMORY_HACK_PLUS60K:
            s = "+60K";
            break;
        case MEMORY_HACK_PLUS256K:
            s = "+256K";
            break;
    }
    return s;
}

static TUI_MENU_CALLBACK(c64_256k_base_submenu_callback)
{
    int value;
    static char s[100];

    resources_get_int("C64_256Kbase", &value);
    sprintf(s, "%X-%X", value, value + 0x7f);
    return s;
}

static TUI_MENU_CALLBACK(plus60k_base_submenu_callback)
{
    int value;
    static char s[100];

    resources_get_int("PLUS60Kbase", &value);
    sprintf(s, "$%X",value);
    return s;
}

static tui_menu_item_def_t memory_hack_device_submenu[] = {
    { "None", NULL, radio_MemoryHack_callback,
      (void *)MEMORY_HACK_NONE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "C64 256K", NULL, radio_MemoryHack_callback,
      (void *)MEMORY_HACK_C64_256K, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "+60K", NULL, radio_MemoryHack_callback,
      (void *)MEMORY_HACK_PLUS60K, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "+256K", NULL, radio_MemoryHack_callback,
      (void *)MEMORY_HACK_PLUS256K, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t c64_256k_base_submenu[] = {
    { "$_DE00-$DE7F", NULL, radio_C64_256Kbase_callback,
      (void *)0xde00, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$D_E80-$DEFF", NULL, radio_C64_256Kbase_callback,
      (void *)0xde80, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF_00-$DF7F", NULL, radio_C64_256Kbase_callback,
      (void *)0xdf00, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$D_F80-$DFFF", NULL, radio_C64_256Kbase_callback,
      (void *)0xdf80, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t plus60k_base_submenu[] = {
    { "$_D040", NULL, radio_PLUS60Kbase_callback,
      (void *)0xd040, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$D_100", NULL, radio_PLUS60Kbase_callback,
      (void *)0xd100, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t memory_hacks_menu_items[] = {
    { "Memory Expansion Hack Device:", "Select the device to use",
      memory_hack_device_submenu_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, memory_hack_device_submenu, "device" },
    { "256K base:", "Select the base of the 256K RAM Expansion",
      c64_256k_base_submenu_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, c64_256k_base_submenu, "256K base" },
    { "256K image file:", "Select the 256K image file",
      filename_C64_256Kfilename_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "PLUS60K base:", "Select the base address of the PLUS60K RAM Expansion",
      plus60k_base_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, plus60k_base_submenu,
      "PLUS60K base" },
    { "PLUS60K image file:", "Select the PLUS60K image file",
      filename_PLUS60Kfilename_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "PLUS256K image file:", "Select the PLUS256K image file",
      filename_PLUS256Kfilename_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uic64_memory_hacks_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_memory_hacks_submenu;

    ui_memory_hacks_submenu = tui_menu_create("Memory Expansion Hacks settings", 1);

    tui_menu_add(ui_memory_hacks_submenu, memory_hacks_menu_items);

    tui_menu_add_submenu(parent_submenu, "Memory Expansion Hacks settings...",
                         "Memory Expansion Hacks settings",
                         ui_memory_hacks_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
