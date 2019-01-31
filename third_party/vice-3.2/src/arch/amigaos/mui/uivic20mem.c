/*
 * uivic20mem.c
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

#include "uivic20mem.h"
#include "intl.h"
#include "translate.h"

static int ui_vic20mem_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_vic20mem_enable[countof(ui_vic20mem_enable_translate)];

static const int ui_vic20mem_enable_values[] = {
    0,
    1,
    -1
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "RAMBlock0", ui_vic20mem_enable, ui_vic20mem_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "RAMBlock1", ui_vic20mem_enable, ui_vic20mem_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "RAMBlock2", ui_vic20mem_enable, ui_vic20mem_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "RAMBlock3", ui_vic20mem_enable, ui_vic20mem_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "RAMBlock5", ui_vic20mem_enable, ui_vic20mem_enable_values, NULL },
    UI_END /* mandatory */
};

static APTR build_gui(void)
{
    return GroupObject,
             CYCLE(ui_to_from[0].object, translate_text(IDS_RAM_BLOCK_0400_0FFF), ui_vic20mem_enable)
             CYCLE(ui_to_from[1].object, translate_text(IDS_RAM_BLOCK_2000_3FFF), ui_vic20mem_enable)
             CYCLE(ui_to_from[2].object, translate_text(IDS_RAM_BLOCK_4000_5FFF), ui_vic20mem_enable)
             CYCLE(ui_to_from[3].object, translate_text(IDS_RAM_BLOCK_6000_7FFF), ui_vic20mem_enable)
             CYCLE(ui_to_from[4].object, translate_text(IDS_RAM_BLOCK_A000_BFFF), ui_vic20mem_enable)
           End;
}

void ui_vic20_settings_dialog(void)
{
    intl_convert_mui_table(ui_vic20mem_enable_translate, ui_vic20mem_enable);
    mui_show_dialog(build_gui(), translate_text(IDS_VIC20_SETTINGS), ui_to_from);
}
