/*
 * uisid.h - SID UI interface for MS-DOS.
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

#ifndef UISID_H
#define UISID_H

#define SID_D4XX_MENU(menu, callback)   \
static tui_menu_item_def_t menu[] = {   \
    { "$D420", "$D420",                 \
      callback, (void *)0xd420, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D440", "$D440",                 \
      callback, (void *)0xd440, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D460", "$D460",                 \
      callback, (void *)0xd460, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D480", "$D480",                 \
      callback, (void *)0xd480, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D4A0", "$D4A0",                 \
      callback, (void *)0xd4a0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D4C0", "$D4C0",                 \
      callback, (void *)0xd4c0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D4E0", "$D4E0",                 \
      callback, (void *)0xd4e0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    TUI_MENU_ITEM_DEF_LIST_END          \
};

#define SID_D5XX_MENU(menu, callback)   \
static tui_menu_item_def_t menu[] = {   \
    { "$D500", "$D500",                 \
      callback, (void *)0xd500, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D520", "$D520",                 \
      callback, (void *)0xd520, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D540", "$D540",                 \
      callback, (void *)0xd540, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D560", "$D560",                 \
      callback, (void *)0xd560, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D580", "$D580",                 \
      callback, (void *)0xd580, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D5A0", "$D5A0",                 \
      callback, (void *)0xd5a0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D5C0", "$D5C0",                 \
      callback, (void *)0xd5c0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D5E0", "$D5E0",                 \
      callback, (void *)0xd5e0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    TUI_MENU_ITEM_DEF_LIST_END          \
};

#define SID_D6XX_MENU(menu, callback)   \
static tui_menu_item_def_t menu[] = {   \
    { "$D600", "$D600",                 \
      callback, (void *)0xd600, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D620", "$D620",                 \
      callback, (void *)0xd620, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D640", "$D640",                 \
      callback, (void *)0xd640, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D660", "$D660",                 \
      callback, (void *)0xd660, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D680", "$D680",                 \
      callback, (void *)0xd680, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D6A0", "$D6A0",                 \
      callback, (void *)0xd6a0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D6C0", "$D6C0",                 \
      callback, (void *)0xd6c0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D6E0", "$D6E0",                 \
      callback, (void *)0xd6e0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    TUI_MENU_ITEM_DEF_LIST_END          \
};

#define SID_D7XX_MENU(menu, callback)   \
static tui_menu_item_def_t menu[] = {   \
    { "$D700", "$D700",                 \
      callback, (void *)0xd700, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D720", "$D720",                 \
      callback, (void *)0xd720, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D740", "$D740",                 \
      callback, (void *)0xd740, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D760", "$D760",                 \
      callback, (void *)0xd760, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D780", "$D780",                 \
      callback, (void *)0xd780, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D7A0", "$D7A0",                 \
      callback, (void *)0xd7a0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D7C0", "$D7C0",                 \
      callback, (void *)0xd7c0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$D7E0", "$D7E0",                 \
      callback, (void *)0xd7e0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    TUI_MENU_ITEM_DEF_LIST_END          \
};

#define SID_DEXX_MENU(menu, callback)   \
static tui_menu_item_def_t menu[] = {   \
    { "$DE00", "$DE00",                 \
      callback, (void *)0xde00, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$DE20", "$DE20",                 \
      callback, (void *)0xde20, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$DE40", "$DE40",                 \
      callback, (void *)0xde40, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$DE60", "$DE60",                 \
      callback, (void *)0xde60, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$DE80", "$DE80",                 \
      callback, (void *)0xde80, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$DEA0", "$DEA0",                 \
      callback, (void *)0xdea0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$DEC0", "$DEC0",                 \
      callback, (void *)0xdec0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$DEE0", "$DEE0",                 \
      callback, (void *)0xdee0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    TUI_MENU_ITEM_DEF_LIST_END          \
};

#define SID_DFXX_MENU(menu, callback)   \
static tui_menu_item_def_t menu[] = {   \
    { "$DF00", "$DF00",                 \
      callback, (void *)0xdf00, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$DF20", "$DF20",                 \
      callback, (void *)0xdf20, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$DF40", "$DF40",                 \
      callback, (void *)0xdf40, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$DF60", "$DF60",                 \
      callback, (void *)0xdf60, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$DF80", "$DF80",                 \
      callback, (void *)0xdf80, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$DFA0", "$DFA0",                 \
      callback, (void *)0xdfa0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$DFC0", "$DFC0",                 \
      callback, (void *)0xdfc0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    { "$DFE0", "$DFE0",                 \
      callback, (void *)0xdfe0, 0,      \
      TUI_MENU_BEH_CLOSE, NULL, NULL }, \
    TUI_MENU_ITEM_DEF_LIST_END          \
};

#endif
