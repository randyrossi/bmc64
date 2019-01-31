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
#include <string.h>

#include "lib.h"
#include "resources.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "uiuserport.h"
#include "util.h"

TUI_MENU_DEFINE_TOGGLE(Datasette)
TUI_MENU_DEFINE_TOGGLE(TapeLog)
TUI_MENU_DEFINE_RADIO(TapeLogDestination)
TUI_MENU_DEFINE_FILENAME(TapeLogfilename, "Tape log")
TUI_MENU_DEFINE_TOGGLE(CPClockF83)
TUI_MENU_DEFINE_TOGGLE(CPClockF83Save)
TUI_MENU_DEFINE_TOGGLE(TapeSenseDongle)
TUI_MENU_DEFINE_TOGGLE(DTLBasicDongle)

static TUI_MENU_CALLBACK(tapelog_destination_submenu_callback)
{
    int value;
    static char s[100];

    resources_get_int("TapeLogDestination", &value);
    sprintf(s, "%s", value ? "Emulator logfile" : "User specified file");
    return s;
}

static tui_menu_item_def_t tapelog_destination_submenu[] = {
    { "Default emulator log", NULL, radio_TapeLogDestination_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "User specified file", NULL, radio_TapeLogDestination_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t tapelog_menu_items[] = {
    { "Enable tape log device:", "Enable tape log device",
      toggle_TapeLog_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Tape log messages destination:", "Tape log messages destination",
      tapelog_destination_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, tapelog_destination_submenu,
      "Tape log messages destination" },
    { "Tape log file:", "Select the tape log file",
      filename_TapeLogfilename_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t cp_clock_f83_menu_items[] = {
    { "Enable CP Clock F83:", "Enable CP Clock F83",
      toggle_CPClockF83_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Save RTC data when changed:", "Save RTC data when changed",
      toggle_CPClockF83Save_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t tapeport_menu_items[] = {
    { "Enable datasette:", "Enable datasette",
      toggle_Datasette_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable tape sense dongle:", "Enable tape sense dongle",
      toggle_TapeSenseDongle_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable DTL Basic dongle:", "Enable DTL Basic dongle",
      toggle_DTLBasicDongle_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Tape log device settings:", "Tape log device settings",
      NULL, NULL, 11,
      TUI_MENU_BEH_CONTINUE, tapelog_menu_items,
      "Tape log device settings" },
    { "CP CLock F83 settings:", "CP Clock F83 settings",
      NULL, NULL, 11,
      TUI_MENU_BEH_CONTINUE, cp_clock_f83_menu_items,
      "CP Clock F83 settings" },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uitapeport_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_tapeport_submenu;

    ui_tapeport_submenu = tui_menu_create("Tape port devices", 1);

    tui_menu_add(ui_tapeport_submenu, tapeport_menu_items);

    tui_menu_add_submenu(parent_submenu, "Tape port devices...",
                         "Tape port devices",
                         ui_tapeport_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
