/*
 * uicbm5x0settings.c
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

#include "uicbm5x0settings.h"
#include "intl.h"
#include "translate.h"

static char *ui_cbm5x0_model[] = {
    "5x0 (60 Hz)",
    "5x0 (50 Hz)",
    NULL
};

static const int ui_cbm5x0_model_values[] = {
    1,
    2,
    -1
};

static char *ui_cbm5x0_memsize[] = {
    "64K",
    "128K",
    "256K",
    "512K",
    "1024K",
    NULL
};

static const int ui_cbm5x0_memsize_values[] = {
    64,
    128,
    256,
    512,
    1024,
    -1
};

static int ui_cbm5x0mem_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_cbm5x0mem_enable[countof(ui_cbm5x0mem_enable_translate)];

static const int ui_cbm5x0mem_enable_values[] = {
    0,
    1,
    -1
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "ModelLine", ui_cbm5x0_model, ui_cbm5x0_model_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "RamSize", ui_cbm5x0_memsize, ui_cbm5x0_memsize_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "Ram08", ui_cbm5x0mem_enable, ui_cbm5x0mem_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "Ram1", ui_cbm5x0mem_enable, ui_cbm5x0mem_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "Ram2", ui_cbm5x0mem_enable, ui_cbm5x0mem_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "Ram4", ui_cbm5x0mem_enable, ui_cbm5x0mem_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "Ram6", ui_cbm5x0mem_enable, ui_cbm5x0mem_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "RamC", ui_cbm5x0mem_enable, ui_cbm5x0mem_enable_values, NULL },
    UI_END /* mandatory */
};

static APTR build_gui(void)
{
    return GroupObject,
             CYCLE(ui_to_from[0].object, translate_text(IDS_MODEL_LINE), ui_cbm5x0_model)
             CYCLE(ui_to_from[1].object, translate_text(IDS_RAM_SIZE), ui_cbm5x0_memsize)
             CYCLE(ui_to_from[2].object, translate_text(IDS_RAM_BLOCK_0800_0FFF), ui_cbm5x0mem_enable)
             CYCLE(ui_to_from[3].object, translate_text(IDS_RAM_BLOCK_1000_1FFF), ui_cbm5x0mem_enable)
             CYCLE(ui_to_from[4].object, translate_text(IDS_RAM_BLOCK_2000_3FFF), ui_cbm5x0mem_enable)
             CYCLE(ui_to_from[5].object, translate_text(IDS_RAM_BLOCK_4000_5FFF), ui_cbm5x0mem_enable)
             CYCLE(ui_to_from[6].object, translate_text(IDS_RAM_BLOCK_6000_7FFF), ui_cbm5x0mem_enable)
             CYCLE(ui_to_from[7].object, translate_text(IDS_RAM_BLOCK_C000_CFFF), ui_cbm5x0mem_enable)
           End;
}

void ui_cbm5x0_settings_dialog(void)
{
    intl_convert_mui_table(ui_cbm5x0mem_enable_translate, ui_cbm5x0mem_enable);
    mui_show_dialog(build_gui(), translate_text(IDS_CBM5X0_SETTINGS), ui_to_from);
}
