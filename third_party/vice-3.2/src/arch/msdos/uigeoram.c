/*
 * uigeoram.c - GEO-RAM UI interface for MS-DOS.
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
#include "uigeoram.h"
#include "util.h"

TUI_MENU_DEFINE_TOGGLE(GEORAM)
TUI_MENU_DEFINE_TOGGLE(GEORAMImageWrite)
TUI_MENU_DEFINE_TOGGLE(GEORAMIOSwap)
TUI_MENU_DEFINE_RADIO(GEORAMsize)
TUI_MENU_DEFINE_FILENAME(GEORAMfilename, "GEO-RAM")

static TUI_MENU_CALLBACK(georam_size_submenu_callback)
{
    int value;
    static char s[100];

    resources_get_int("GEORAMsize", &value);
    sprintf(s, "%dKB",value);
    return s;
}

static tui_menu_item_def_t georam_size_submenu[] = {
    { "_64KB", NULL, radio_GEORAMsize_callback,
      (void *)64, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_128KB", NULL, radio_GEORAMsize_callback,
      (void *)128, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_256KB", NULL, radio_GEORAMsize_callback,
      (void *)256, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_512KB", NULL, radio_GEORAMsize_callback,
      (void *)512, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "102_4KB", NULL, radio_GEORAMsize_callback,
      (void *)1024, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "2_048KB", NULL, radio_GEORAMsize_callback,
      (void *)2048, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "40_96KB", NULL, radio_GEORAMsize_callback,
      (void *)4096, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t georam_c64_menu_items[] = {
    { "_Enable GEO-RAM:", "Emulate GEO-RAM Expansion Unit",
      toggle_GEORAM_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "GEO-RAM _size:", "Select the size of the GEO-RAM",
      georam_size_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, georam_size_submenu,
      "GEO-RAM size" },
    { "_Write to GEO-RAM image when changed:", "Write to GEO-RAM image when the data has been changed",
      toggle_GEORAMImageWrite_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "GEO-RAM _image file:", "Select the GEO-RAM image file",
      filename_GEORAMfilename_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t georam_vic20_menu_items[] = {
    { "_Enable GEO-RAM:", "Emulate GEO-RAM Expansion Unit",
      toggle_GEORAM_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable MasC=uerade I/O swap:", "Map GEO-RAM I/O-1 to vic20 I/O-3 and GEO-RAM I/O-2 to vic20 I/O-2",
      toggle_GEORAMIOSwap_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "GEO-RAM _size:", "Select the size of the GEO-RAM",
      georam_size_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, georam_size_submenu,
      "GEO-RAM size" },
    { "_Write to GEO-RAM image when changed:", "Write to GEO-RAM image when the data has been changed",
      toggle_GEORAMImageWrite_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "GEO-RAM _image file:", "Select the GEO-RAM image file",
      filename_GEORAMfilename_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uigeoram_c64_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_georam_submenu;

    ui_georam_submenu = tui_menu_create("GEO-RAM settings", 1);

    tui_menu_add(ui_georam_submenu, georam_c64_menu_items);

    tui_menu_add_submenu(parent_submenu, "_GEO-RAM settings...",
                         "GEO-RAM settings",
                         ui_georam_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}

void uigeoram_vic20_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_georam_submenu;

    ui_georam_submenu = tui_menu_create("GEO-RAM settings (MasC=uerade)", 1);

    tui_menu_add(ui_georam_submenu, georam_vic20_menu_items);

    tui_menu_add_submenu(parent_submenu, "_GEO-RAM settings (MasC=uerade)...",
                         "GEO-RAM settings",
                         ui_georam_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
