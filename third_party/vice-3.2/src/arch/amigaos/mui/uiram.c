/*
 * uiram.c
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
#ifdef AMIGA_M68K
#define _INLINE_MUIMASTER_H
#endif
#include "mui.h"

#include "uiram.h"
#include "intl.h"
#include "translate.h"

static char *ui_ram_1st_byte[] = {
    "0",
    "255",
    NULL
};

static const int ui_ram_1st_byte_values[] = {
    0,
    255,
    -1
};

static char *ui_ram_constant_val[] = {
    "0",
    "1",
    "2",
    "4",
    "8",
    "16",
    "32",
    "64",
    "128",
    "256",
    "512",
    "1024",
    NULL
};

static const int ui_ram_constant_val_values[] = {
    0,
    1,
    2,
    4,
    8,
    16,
    32,
    64,
    128,
    256,
    512,
    1024,
    -1
};

static char *ui_ram_constant_pat[] = {
    "0",
    "1",
    "2",
    "4",
    "8",
    "16",
    "32",
    "64",
    "128",
    "256",
    "512",
    "1024",
    NULL
};

static const int ui_ram_constant_pat_values[] = {
    0,
    1,
    2,
    4,
    8,
    16,
    32,
    64,
    128,
    256,
    512,
    1024,
    -1
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "RAMInitStartValue", ui_ram_1st_byte, ui_ram_1st_byte_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "RAMInitValueInvert", ui_ram_constant_val, ui_ram_constant_val_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "RAMInitPatternInvert", ui_ram_constant_pat, ui_ram_constant_pat_values, NULL },
    UI_END /* mandatory */
};

static APTR build_gui(void)
{
    return GroupObject,
             CYCLE(ui_to_from[0].object, translate_text(IDS_VALUE_FIRST_BYTE), ui_ram_1st_byte)
             CYCLE(ui_to_from[1].object, translate_text(IDS_LENGTH_CONSTANT_VALUES), ui_ram_constant_val)
             CYCLE(ui_to_from[2].object, translate_text(IDS_LENGTH_CONSTANT_PATTERN), ui_ram_constant_pat)
           End;
}

void ui_ram_settings_dialog(void)
{
    mui_show_dialog(build_gui(), translate_text(IDS_RAM_SETTINGS), ui_to_from);
}
