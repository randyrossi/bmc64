/*
 * uivic.c
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

#include "vic.h"
#include "uivic.h"
#include "intl.h"
#include "translate.h"

static int ui_vic_borders_translate[] = {
    IDMS_NORMAL,
    IDS_FULL,
    IDS_DEBUG,
    IDS_NONE,
    0
};

static char *ui_vic_borders[countof(ui_vic_borders_translate)];

static const int ui_vic_borders_values[] = {
    VIC_NORMAL_BORDERS,
    VIC_FULL_BORDERS,
    VIC_DEBUG_BORDERS,
    VIC_NO_BORDERS,
    -1
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "VICBorderMode", ui_vic_borders, ui_vic_borders_values, NULL },
    UI_END /* mandatory */
};

static APTR build_gui(void)
{
    return GroupObject,
             CYCLE(ui_to_from[0].object, translate_text(IDS_BORDER_MODE), ui_vic_borders)
           End;
}

void ui_vic_settings_dialog(void)
{
    intl_convert_mui_table(ui_vic_borders_translate, ui_vic_borders);
    mui_show_dialog(build_gui(), translate_text(IDS_VIC_SETTINGS), ui_to_from);
}
