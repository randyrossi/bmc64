/*
 * uids12c887rtc.c - DS12C887 RTC cartridge UI interface for MS-DOS.
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
#include "uids12c887rtc.h"

TUI_MENU_DEFINE_TOGGLE(DS12C887RTC)
TUI_MENU_DEFINE_RADIO(DS12C887RTCbase)
TUI_MENU_DEFINE_TOGGLE(DS12C887RTCRunMode)
TUI_MENU_DEFINE_TOGGLE(DS12C887RTCSave)

static TUI_MENU_CALLBACK(ds12c887rtc_base_submenu_callback)
{
    int value;
    static char s[100];

    resources_get_int("DS12C887RTCbase", &value);
    sprintf(s, "$%X", value);
    return s;
}

static tui_menu_item_def_t ds12c887rtc_c64_base_submenu[] = {
    { "$D500", NULL, radio_DS12C887RTCbase_callback,
      (void *)0xd500, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$D600", NULL, radio_DS12C887RTCbase_callback,
      (void *)0xd600, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$D700", NULL, radio_DS12C887RTCbase_callback,
      (void *)0xd700, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE00", NULL, radio_DS12C887RTCbase_callback,
      (void *)0xde00, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF00", NULL, radio_DS12C887RTCbase_callback,
      (void *)0xdf00, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ds12c887rtc_c128_base_submenu[] = {
    { "$D700", NULL, radio_DS12C887RTCbase_callback,
      (void *)0xd700, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE00", NULL, radio_DS12C887RTCbase_callback,
      (void *)0xde00, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF00", NULL, radio_DS12C887RTCbase_callback,
      (void *)0xdf00, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ds12c887rtc_vic20_base_submenu[] = {
    { "$9800", NULL, radio_DS12C887RTCbase_callback,
      (void *)0x9800, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C00", NULL, radio_DS12C887RTCbase_callback,
      (void *)0x9c00, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ds12c887rtc_c64_menu_items[] = {
    { "_Enable DS12C887 RTC:", "Emulate DS12C887 RTC Cartridge",
      toggle_DS12C887RTC_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "DS12C887 RTC _base:", "Select the base of the DS12C887 RTC Cartridge",
      ds12c887rtc_base_submenu_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, ds12c887rtc_c64_base_submenu,
      "DS12C887 RTC base" },
    { "DS12C887 RTC running mode enable:", "Sets the initial running mode to 'running'",
      toggle_DS12C887RTCRunMode_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Save DS12C887 RTC data when changed:", "Save DS12C887 RTC data when changed",
      toggle_DS12C887RTCSave_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ds12c887rtc_c128_menu_items[] = {
    { "_Enable DS12C887 RTC:", "Emulate DS12C887 RTC Cartridge",
      toggle_DS12C887RTC_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "DS12C887 RTC _base:", "Select the base of the DS12C887 RTC Cartridge",
      ds12c887rtc_base_submenu_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, ds12c887rtc_c128_base_submenu,
      "DS12C887 RTC base" },
    { "DS12C887 RTC running mode enable:", "Sets the initial running mode to 'running'",
      toggle_DS12C887RTCRunMode_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Save DS12C887 RTC data when changed:", "Save DS12C887 RTC data when changed",
      toggle_DS12C887RTCSave_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ds12c887rtc_vic20_menu_items[] = {
    { "_Enable DS12C887 RTC:", "Emulate DS12C887 RTC Cartridge",
      toggle_DS12C887RTC_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "DS12C887 RTC _base:", "Select the base of the DS12C887 RTC Cartridge",
      ds12c887rtc_base_submenu_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, ds12c887rtc_vic20_base_submenu,
      "DS12C887 RTC base" },
    { "DS12C887 RTC running mode enable:", "Sets the initial running mode to 'running'",
      toggle_DS12C887RTCRunMode_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Save DS12C887 RTC data when changed:", "Save DS12C887 RTC data when changed",
      toggle_DS12C887RTCSave_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uids12c887rtc_c64_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_ds12c887rtc_submenu;

    ui_ds12c887rtc_submenu = tui_menu_create("DS12C887 RTC settings", 1);

    tui_menu_add(ui_ds12c887rtc_submenu, ds12c887rtc_c64_menu_items);

    tui_menu_add_submenu(parent_submenu, "_DS12C887 RTC settings...",
                         "DS12C887 RTC settings",
                         ui_ds12c887rtc_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}

void uids12c887rtc_c128_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_ds12c887rtc_submenu;

    ui_ds12c887rtc_submenu = tui_menu_create("DS12C887 RTC settings", 1);

    tui_menu_add(ui_ds12c887rtc_submenu, ds12c887rtc_c128_menu_items);

    tui_menu_add_submenu(parent_submenu, "_DS12C887 RTC settings...",
                         "DS12C887 RTC settings",
                         ui_ds12c887rtc_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}

void uids12c887rtc_vic20_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_ds12c887rtc_submenu;

    ui_ds12c887rtc_submenu = tui_menu_create("DS12C887 RTC settings (MasC=uerade)", 1);

    tui_menu_add(ui_ds12c887rtc_submenu, ds12c887rtc_vic20_menu_items);

    tui_menu_add_submenu(parent_submenu, "_DS12C887 RTC settings (MasC=uerade)...",
                         "DS12C887 RTC settings",
                         ui_ds12c887rtc_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
