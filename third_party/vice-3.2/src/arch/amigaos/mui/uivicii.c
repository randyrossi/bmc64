/*
 * uivicii.c
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

#include "vicii.h"
#include "uivicii.h"
#include "intl.h"
#include "translate.h"

static int ui_vicii_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_vicii_enable[countof(ui_vicii_enable_translate)];

static const int ui_vicii_enable_values[] = {
    0,
    1,
    -1
};

static int ui_vicii_borders_translate[] = {
    IDMS_NORMAL,
    IDS_FULL,
    IDS_DEBUG,
    IDS_NONE,
    0
};

static char *ui_vicii_borders[countof(ui_vicii_borders_translate)];

static const int ui_vicii_borders_values[] = {
    VICII_NORMAL_BORDERS,
    VICII_FULL_BORDERS,
    VICII_DEBUG_BORDERS,
    VICII_NO_BORDERS,
    -1
};

static ui_to_from_t ui_to_from_64[] = {
    { NULL, MUI_TYPE_CYCLE, "VICIIBorderMode", ui_vicii_borders, ui_vicii_borders_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "VICIICheckSsColl", ui_vicii_enable, ui_vicii_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "VICIICheckSbColl", ui_vicii_enable, ui_vicii_enable_values, NULL },
    UI_END /* mandatory */
};

static ui_to_from_t ui_to_from_dtv[] = {
    { NULL, MUI_TYPE_CYCLE, "VICIIBorderMode", ui_vicii_borders, ui_vicii_borders_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "VICIICheckSsColl", ui_vicii_enable, ui_vicii_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "VICIICheckSbColl", ui_vicii_enable, ui_vicii_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "VICIINewLuminances", ui_vicii_enable, ui_vicii_enable_values, NULL },
    UI_END /* mandatory */
};

static ui_to_from_t ui_to_from_sc[] = {
    { NULL, MUI_TYPE_CYCLE, "VICIIBorderMode", ui_vicii_borders, ui_vicii_borders_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "VICIICheckSsColl", ui_vicii_enable, ui_vicii_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "VICIICheckSbColl", ui_vicii_enable, ui_vicii_enable_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "VICIIVSPBug", ui_vicii_enable, ui_vicii_enable_values, NULL },
    UI_END /* mandatory */
};

static APTR build_gui_64(void)
{
    return GroupObject,
             CYCLE(ui_to_from_64[0].object, translate_text(IDS_BORDER_MODE), ui_vicii_borders)
             CYCLE(ui_to_from_64[1].object, translate_text(IDS_SPRITE_SPRITE_COL), ui_vicii_enable)
             CYCLE(ui_to_from_64[2].object, translate_text(IDS_SPRITE_BACKGROUND_COL), ui_vicii_enable)
           End;
}

static APTR build_gui_dtv(void)
{
    return GroupObject,
             CYCLE(ui_to_from_dtv[0].object, translate_text(IDS_BORDER_MODE), ui_vicii_borders)
             CYCLE(ui_to_from_dtv[1].object, translate_text(IDS_SPRITE_SPRITE_COL), ui_vicii_enable)
             CYCLE(ui_to_from_dtv[2].object, translate_text(IDS_SPRITE_BACKGROUND_COL), ui_vicii_enable)
             CYCLE(ui_to_from_dtv[3].object, translate_text(IDS_NEW_LUMINANCES), ui_vicii_enable)
           End;
}

static APTR build_gui_sc(void)
{
    return GroupObject,
             CYCLE(ui_to_from_sc[0].object, translate_text(IDS_BORDER_MODE), ui_vicii_borders)
             CYCLE(ui_to_from_sc[1].object, translate_text(IDS_SPRITE_SPRITE_COL), ui_vicii_enable)
             CYCLE(ui_to_from_sc[2].object, translate_text(IDS_SPRITE_BACKGROUND_COL), ui_vicii_enable)
             CYCLE(ui_to_from_sc[3].object, translate_text(IDS_VSPBUG), ui_vicii_enable)
           End;
}

void ui_vicii_settings_dialog(void)
{
    intl_convert_mui_table(ui_vicii_enable_translate, ui_vicii_enable);
    intl_convert_mui_table(ui_vicii_borders_translate, ui_vicii_borders);
    mui_show_dialog(build_gui_64(), translate_text(IDS_VICII_SETTINGS), ui_to_from_64);
}

void ui_viciidtv_settings_dialog(void)
{
    intl_convert_mui_table(ui_vicii_enable_translate, ui_vicii_enable);
    intl_convert_mui_table(ui_vicii_borders_translate, ui_vicii_borders);
    mui_show_dialog(build_gui_dtv(), translate_text(IDS_VICII_SETTINGS), ui_to_from_dtv);
}

void ui_viciisc_settings_dialog(void)
{
    intl_convert_mui_table(ui_vicii_enable_translate, ui_vicii_enable);
    intl_convert_mui_table(ui_vicii_borders_translate, ui_vicii_borders);
    mui_show_dialog(build_gui_sc(), translate_text(IDS_VICII_SETTINGS), ui_to_from_sc);
}
