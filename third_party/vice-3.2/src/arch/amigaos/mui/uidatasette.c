/*
 * uidatasette.c
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

#include "uidatasette.h"
#include "intl.h"
#include "translate.h"

static int ui_datasette_reset_with_cpu_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_datasette_reset_with_cpu[countof(ui_datasette_reset_with_cpu_translate)];

static const int ui_datasette_reset_with_cpu_values[] = {
    0,
    1,
    -1
};

static char *ui_datasette_additional_delay[] = {
    "0 cycles",
    "1 cycle",
    "2 cycles",
    "3 cycles",
    "4 cycles",
    "5 cycles",
    "6 cycles",
    "7 cycles",
    NULL
};

static const int ui_datasette_additional_delay_values[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    6,
    7,
    -1
};

static char *ui_datasette_delay_at_zero[] = {
    "1000 cycles",
    "2000 cycles",
    "5000 cycles",
    "10000 cycles",
    "20000 cycles",
    "50000 cycles",
    "100000 cycles",
    NULL
};

static const int ui_datasette_delay_at_zero_values[] = {
    1000,
    2000,
    5000,
    10000,
    20000,
    50000,
    100000,
    -1
};

static ui_wobble_range[] = {
    0,
    0xfffffff
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "Datasette", ui_datasette_reset_with_cpu, ui_datasette_reset_with_cpu_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DatasetteResetWithCPU", ui_datasette_reset_with_cpu, ui_datasette_reset_with_cpu_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DatasetteSpeedTuning", ui_datasette_additional_delay, ui_datasette_additional_delay_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DatasetteZeroGapDelay", ui_datasette_delay_at_zero, ui_datasette_delay_at_zero_values, NULL },
    { NULL, MUI_TYPE_INTEGER, "DatasetteTapeWobble", NULL, ui_wobble_range, NULL },
    UI_END /* mandatory */
};

static APTR build_gui(void)
{
    return GroupObject,
             CYCLE(ui_to_from[0].object, translate_text(IDS_DATASETTE), ui_datasette_reset_with_cpu)
             CYCLE(ui_to_from[1].object, translate_text(IDS_RESET_DATASETTE_WITH_CPU), ui_datasette_reset_with_cpu)
             CYCLE(ui_to_from[2].object, translate_text(IDS_ADDITIONAL_DELAY), ui_datasette_additional_delay)
             CYCLE(ui_to_from[3].object, translate_text(IDS_DELAY_AT_ZERO_VALUES), ui_datasette_delay_at_zero)
             NSTRING(ui_to_from[4].object, translate_text(IDS_RANDOM_TAPE_WOBBLE), "0123456789", 5+1)
           End;
}

void ui_datasette_settings_dialog(void)
{
    intl_convert_mui_table(ui_datasette_reset_with_cpu_translate, ui_datasette_reset_with_cpu);
    mui_show_dialog(build_gui(), translate_text(IDS_DATASETTE_SETTINGS), ui_to_from);
}
