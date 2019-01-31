/*
 * uic64dtv.c - C64DTV UI interface for MS-DOS.
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

#include "c64dtv-resources.h"
#include "lib.h"
#include "resources.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "uic64dtv.h"
#include "util.h"

TUI_MENU_DEFINE_RADIO(DtvRevision)
TUI_MENU_DEFINE_TOGGLE(c64dtvromrw)
TUI_MENU_DEFINE_TOGGLE(HummerADC)
TUI_MENU_DEFINE_FILENAME(c64dtvromfilename, "C64DTV ROM")
TUI_MENU_DEFINE_TOGGLE(VICIINewLuminances)

static TUI_MENU_CALLBACK(DtvRevision_submenu_callback)
{
    int value;
    static char s[10];

    resources_get_int("DtvRevision", &value);
    sprintf(s, "DTV%d", value);
    return s;
}

static tui_menu_item_def_t DtvRevision_submenu[] = {
    { "DTV_2", NULL, radio_DtvRevision_callback,
      (void *)2, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "DTV_3", NULL, radio_DtvRevision_callback,
      (void *)3, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t c64dtv_menu_items[] = {
    { "C64DTV ROM _image file:", "Select the C64DTV ROM image file",
      filename_c64dtvromfilename_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "C64DTV revision:", "Select the revision of the C64DTV",
      DtvRevision_submenu_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, DtvRevision_submenu,
      "C64DTV revision" },
    { "C64DTV ROM _write enabled:", "C64DTV ROM is writable",
      toggle_c64dtvromrw_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Hummer ADC:", "Hummer ADC enable",
      toggle_HummerADC_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Color fix:",
      "Enable color fix",
      toggle_VICIINewLuminances_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uic64dtv_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_c64dtv_submenu;

    ui_c64dtv_submenu = tui_menu_create("C64DTV settings", 1);

    tui_menu_add(ui_c64dtv_submenu, c64dtv_menu_items);

    tui_menu_add_submenu(parent_submenu, "_C64DTV settings...",
                         "C64DTV settings",
                         ui_c64dtv_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
