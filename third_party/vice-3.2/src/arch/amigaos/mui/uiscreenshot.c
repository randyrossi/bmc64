/*
 * uiscreenshot.c
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

#include "uiscreenshot.h"
#include "gfxoutput.h"
#include "intl.h"
#include "machine.h"
#include "translate.h"

static int ui_screenshot_oversize_translate[] = {
    IDS_SCALE,
    IDS_CROP_LEFT_TOP,
    IDS_CROP_MIDDLE_TOP,
    IDS_CROP_RIGHT_TOP,
    IDS_CROP_LEFT_CENTER,
    IDS_CROP_MIDDLE_CENTER,
    IDS_CROP_RIGHT_CENTER,
    IDS_CROP_LEFT_BOTTOM,
    IDS_CROP_MIDDLE_BOTTOM,
    IDS_CROP_RIGHT_BOTTOM,
    0
};

static char *ui_screenshot_oversize[countof(ui_screenshot_oversize_translate)];

static const int ui_screenshot_oversize_values[] = {
    NATIVE_SS_OVERSIZE_SCALE,
    NATIVE_SS_OVERSIZE_CROP_LEFT_TOP,
    NATIVE_SS_OVERSIZE_CROP_CENTER_TOP,
    NATIVE_SS_OVERSIZE_CROP_RIGHT_TOP,
    NATIVE_SS_OVERSIZE_CROP_LEFT_CENTER,
    NATIVE_SS_OVERSIZE_CROP_CENTER,
    NATIVE_SS_OVERSIZE_CROP_RIGHT_CENTER,
    NATIVE_SS_OVERSIZE_CROP_LEFT_BOTTOM,
    NATIVE_SS_OVERSIZE_CROP_CENTER_BOTTOM,
    NATIVE_SS_OVERSIZE_CROP_RIGHT_BOTTOM,
    -1
};

static int ui_screenshot_undersize_translate[] = {
    IDS_SCALE,
    IDS_BORDERIZE,
    0
};

static char *ui_screenshot_undersize[countof(ui_screenshot_undersize_translate)];

static const int ui_screenshot_undersize_values[] = {
    NATIVE_SS_UNDERSIZE_SCALE,
    NATIVE_SS_UNDERSIZE_BORDERIZE,
    -1
};

static int ui_screenshot_multicolor_translate[] = {
    IDS_BLACK_AND_WHITE,
    IDS_2_COLORS,
    IDS_4_COLORS,
    IDS_GRAY_SCALE,
    IDS_DITHER,
    0
};

static char *ui_screenshot_multicolor[countof(ui_screenshot_multicolor_translate)];

static const int ui_screenshot_multicolor_values[] = {
    NATIVE_SS_MC2HR_BLACK_WHITE,
    NATIVE_SS_MC2HR_2_COLORS,
    NATIVE_SS_MC2HR_4_COLORS,
    NATIVE_SS_MC2HR_GRAY,
    NATIVE_SS_MC2HR_DITHER,
    -1
};

static int ui_screenshot_tedlum_translate[] = {
    IDS_IGNORE,
    IDS_DITHER,
    0
};

static char *ui_screenshot_tedlum[countof(ui_screenshot_tedlum_translate)];

static const int ui_screenshot_tedlum_values[] = {
    NATIVE_SS_TED_LUM_IGNORE,
    NATIVE_SS_TED_LUM_DITHER,
    -1
};

static int ui_screenshot_crtc_colors_translate[] = {
    IDS_WHITE,
    IDS_AMBER,
    IDS_GREEN,
    0
};

static char *ui_screenshot_crtc_colors[countof(ui_screenshot_crtc_colors_translate)];

static const int ui_screenshot_crtc_colors_values[] = {
    NATIVE_SS_CRTC_WHITE,
    NATIVE_SS_CRTC_AMBER,
    NATIVE_SS_CRTC_GREEN,
    -1
};

static ui_to_from_t ui_to_from_vic_vicii_vdc[] = {
    { NULL, MUI_TYPE_CYCLE, "DoodleOversizeHandling", ui_screenshot_oversize, ui_screenshot_oversize_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DoodleUndersizeHandling", ui_screenshot_undersize, ui_screenshot_undersize_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DoodleMultiColorHandling", ui_screenshot_multicolor, ui_screenshot_multicolor_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "KoalaOversizeHandling", ui_screenshot_oversize, ui_screenshot_oversize_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "KoalaUndersizeHandling", ui_screenshot_undersize, ui_screenshot_undersize_values, NULL },
    UI_END /* mandatory */
};

static ui_to_from_t ui_to_from_ted[] = {
    { NULL, MUI_TYPE_CYCLE, "DoodleOversizeHandling", ui_screenshot_oversize, ui_screenshot_oversize_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DoodleUndersizeHandling", ui_screenshot_undersize, ui_screenshot_undersize_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DoodleMultiColorHandling", ui_screenshot_multicolor, ui_screenshot_multicolor_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DoodleTEDLumHandling", ui_screenshot_tedlum, ui_screenshot_tedlum_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "KoalaOversizeHandling", ui_screenshot_oversize, ui_screenshot_oversize_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "KoalaUndersizeHandling", ui_screenshot_undersize, ui_screenshot_undersize_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "KoalaTEDLumHandling", ui_screenshot_tedlum, ui_screenshot_tedlum_values, NULL },
    UI_END /* mandatory */
};

static ui_to_from_t ui_to_from_crtc[] = {
    { NULL, MUI_TYPE_CYCLE, "DoodleOversizeHandling", ui_screenshot_oversize, ui_screenshot_oversize_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DoodleUndersizeHandling", ui_screenshot_undersize, ui_screenshot_undersize_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "DoodleCRTCTextColor", ui_screenshot_crtc_colors, ui_screenshot_crtc_colors_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "KoalaOversizeHandling", ui_screenshot_oversize, ui_screenshot_oversize_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "KoalaUndersizeHandling", ui_screenshot_undersize, ui_screenshot_undersize_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "KoalaCRTCTextColor", ui_screenshot_crtc_colors, ui_screenshot_crtc_colors_values, NULL },
    UI_END /* mandatory */
};

static APTR build_gui_vic_vicii_vdc(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from_vic_vicii_vdc[0].object, translate_text(IDS_DOODLE_OVERSIZE_HANDLING), ui_screenshot_oversize)
           CYCLE(ui_to_from_vic_vicii_vdc[1].object, translate_text(IDS_DOODLE_UNDERSIZE_HANDLING), ui_screenshot_undersize)
           CYCLE(ui_to_from_vic_vicii_vdc[2].object, translate_text(IDS_DOODLE_MULTICOLOR_HANDLING), ui_screenshot_multicolor)
           CYCLE(ui_to_from_vic_vicii_vdc[3].object, translate_text(IDS_KOALA_OVERSIZE_HANDLING), ui_screenshot_oversize)
           CYCLE(ui_to_from_vic_vicii_vdc[4].object, translate_text(IDS_KOALA_UNDERSIZE_HANDLING), ui_screenshot_undersize)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);
    }

    return ui;
}

static APTR build_gui_ted(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from_ted[0].object, translate_text(IDS_DOODLE_OVERSIZE_HANDLING), ui_screenshot_oversize)
           CYCLE(ui_to_from_ted[1].object, translate_text(IDS_DOODLE_UNDERSIZE_HANDLING), ui_screenshot_undersize)
           CYCLE(ui_to_from_ted[2].object, translate_text(IDS_DOODLE_MULTICOLOR_HANDLING), ui_screenshot_multicolor)
           CYCLE(ui_to_from_ted[3].object, translate_text(IDS_DOODLE_TED_LUMINOSITY_HANDLING), ui_screenshot_tedlum)
           CYCLE(ui_to_from_ted[4].object, translate_text(IDS_KOALA_OVERSIZE_HANDLING), ui_screenshot_oversize)
           CYCLE(ui_to_from_ted[5].object, translate_text(IDS_KOALA_UNDERSIZE_HANDLING), ui_screenshot_undersize)
           CYCLE(ui_to_from_ted[6].object, translate_text(IDS_KOALA_TED_LUMINOSITY_HANDLING), ui_screenshot_tedlum)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);
    }

    return ui;
}

static APTR build_gui_crtc(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from_crtc[0].object, translate_text(IDS_DOODLE_OVERSIZE_HANDLING), ui_screenshot_oversize)
           CYCLE(ui_to_from_crtc[1].object, translate_text(IDS_DOODLE_UNDERSIZE_HANDLING), ui_screenshot_undersize)
           CYCLE(ui_to_from_crtc[2].object, translate_text(IDS_DOODLE_CRTC_TEXT_COLOR), ui_screenshot_crtc_colors)
           CYCLE(ui_to_from_crtc[3].object, translate_text(IDS_KOALA_OVERSIZE_HANDLING), ui_screenshot_oversize)
           CYCLE(ui_to_from_crtc[4].object, translate_text(IDS_KOALA_UNDERSIZE_HANDLING), ui_screenshot_undersize)
           CYCLE(ui_to_from_crtc[5].object, translate_text(IDS_KOALA_CRTC_TEXT_COLOR), ui_screenshot_crtc_colors)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);
    }

    return ui;
}

void ui_screenshot_settings_dialog(void)
{
    APTR window;
    ui_to_from_t *ui_to_from = NULL;
    APTR (*build_gui)(void) = NULL;

    intl_convert_mui_table(ui_screenshot_oversize_translate, ui_screenshot_oversize);
    intl_convert_mui_table(ui_screenshot_undersize_translate, ui_screenshot_undersize);
    intl_convert_mui_table(ui_screenshot_multicolor_translate, ui_screenshot_multicolor);
    intl_convert_mui_table(ui_screenshot_tedlum_translate, ui_screenshot_tedlum);
    intl_convert_mui_table(ui_screenshot_crtc_colors_translate, ui_screenshot_crtc_colors);

    switch (machine_class) {
        default:
            ui_to_from = ui_to_from_vic_vicii_vdc;
            build_gui = build_gui_vic_vicii_vdc;
            break;
        case VICE_MACHINE_PET:
        case VICE_MACHINE_CBM6x0:
            ui_to_from = ui_to_from_crtc;
            build_gui = build_gui_crtc;
            break;
        case VICE_MACHINE_PLUS4:
            ui_to_from = ui_to_from_ted;
            build_gui = build_gui_ted;
            break;
    }

    window = mui_make_simple_window(build_gui(), translate_text(IDS_SCREENSHOT_SETTINGS));

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            ui_get_from(ui_to_from);
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}
