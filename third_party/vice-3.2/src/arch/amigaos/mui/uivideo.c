/*
 * uivideo.c
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

#include "uireu.h"
#include "intl.h"
#include "translate.h"

static video_canvas_t *video_canvas = NULL;
static char *video_palette_filename_text = NULL;

static int ui_video_enable_translate[] = {
    IDMS_DISABLED,
    IDS_ENABLED,
    0
};

static char *ui_video_enable[countof(ui_video_enable_translate)];

static const int ui_video_enable_values[] = {
    0,
    1,
    -1
};

static int ui_render_filter_translate[] = {
    IDS_NONE,
    IDS_CRT_EMULATION,
    IDS_SCALE2X,
    0
};

static char *ui_render_filter[countof(ui_render_filter_translate)];

static const int ui_render_filter_values[] = {
    0,
    1,
    2,
    -1
};

static int ui_range_0_4[] = {
    0,
    4000
};

static int ui_range_0_2[] = {
    0,
    2000
};

static int ui_range_0_1[] = {
    0,
    1000
};

#define PAL_SET(name, palname, fname) \
static ULONG name( struct Hook *hook, Object *obj, APTR arg ) \
{                                                             \
    return resources_set_string(palname, fname);              \
}

#ifdef AMIGA_MORPHOS
#define PAL_HOOK(name, pal_set) \
static const struct Hook name = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)pal_set, NULL }
#else
#define PAL_HOOK(name, pal_set) \
static const struct Hook name = { { NULL, NULL }, (VOID *)pal_set, NULL, NULL }
#endif

#define PAL_METHOD(button, hook) \
DoMethod(button, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_CallHook, &hook)

static ui_to_from_t ui_to_from_palette[] = {
    { NULL, MUI_TYPE_CYCLE, NULL, ui_video_enable, ui_video_enable_values, NULL },
    { NULL, MUI_TYPE_FILENAME, NULL, NULL, NULL, NULL },
    UI_END /* mandatory */
};

static ui_to_from_t ui_to_from_colors[] = {
    { NULL, MUI_TYPE_FLOAT, NULL, NULL, ui_range_0_4, NULL },
    { NULL, MUI_TYPE_FLOAT, NULL, NULL, ui_range_0_2, NULL },
    { NULL, MUI_TYPE_FLOAT, NULL, NULL, ui_range_0_2, NULL },
    { NULL, MUI_TYPE_FLOAT, NULL, NULL, ui_range_0_2, NULL },
    { NULL, MUI_TYPE_FLOAT, NULL, NULL, ui_range_0_2, NULL },
    UI_END /* mandatory */
};

static ui_to_from_t ui_to_from_render_filter[] = {
    { NULL, MUI_TYPE_CYCLE, NULL, ui_render_filter, ui_render_filter_values, NULL },
    UI_END /* mandatory */
};

static ui_to_from_t ui_to_from_crt[] = {
    { NULL, MUI_TYPE_FLOAT, NULL, NULL, ui_range_0_1, NULL },
    { NULL, MUI_TYPE_FLOAT, NULL, NULL, ui_range_0_1, NULL },
    { NULL, MUI_TYPE_FLOAT, NULL, NULL, ui_range_0_2, NULL },
    { NULL, MUI_TYPE_FLOAT, NULL, NULL, ui_range_0_2, NULL },
    UI_END /* mandatory */
};

static ULONG Browse_palette(struct Hook *hook, Object *obj, APTR arg)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_EXTERNAL_PALETTE_SELECT), "#?.vpl", video_canvas);

    if (fname != NULL) {
        set(ui_to_from_palette[1].object, MUIA_String_Contents, fname);
    }

    return 0;
}

PAL_SET(C64HQ, "VICIIPaletteFile", "c64hq.vpl")
PAL_SET(C64S, "VICIIPaletteFile", "c64s.vpl")
PAL_SET(CCS64, "VICIIPaletteFile", "ccs64.vpl")
PAL_SET(COMMUNITY_COLORS, "VICIIPaletteFile", "community-colors.vpl")
PAL_SET(DEEKAY, "VICIIPaletteFile", "deekay.vpl")
PAL_SET(FRODO, "VICIIPaletteFile", "frodo.vpl")
PAL_SET(GODOT, "VICIIPaletteFile", "godot.vpl")
PAL_SET(PC64, "VICIIPaletteFile", "pc64.vpl")
PAL_SET(PEPTO_NTSC, "VICIIPaletteFile", "pepto-ntsc.vpl")
PAL_SET(PEPTO_NTSC_SONY, "VICIIPaletteFile", "pepto-ntsc-sony.vpl")
PAL_SET(PEPTO_PAL, "VICIIPaletteFile", "pepto-pal.vpl")
PAL_SET(PEPTO_PALOLD, "VICIIPaletteFile", "pepto-palold.vpl")
PAL_SET(PTOING, "VICIIPaletteFile", "ptoing.vpl")
PAL_SET(RGB, "VICIIPaletteFile", "rgb.vpl")
PAL_SET(VICE_VICII, "VICIIPaletteFile", "vice.vpl")

static APTR build_gui_palette_vicii(void)
{
    APTR app, ui, ok, browse_button, cancel;
    APTR c64hq_button, c64s_button, ccs64_button, community_colors_button, deekay_button;
    APTR frodo_button, godot_button, pc64_button, pepto_ntsc_button, pepto_ntsc_sony_button;
    APTR pepto_pal_button, pepto_palold_button, ptoing_button, rgb_button, vice_button;

    PAL_HOOK(C64HQHook, C64HQ);
    PAL_HOOK(C64SHook, C64S);
    PAL_HOOK(CCS64Hook, CCS64);
    PAL_HOOK(COMMUNITY_COLORSHook, COMMUNITY_COLORS);
    PAL_HOOK(DEEKAYHook, DEEKAY);
    PAL_HOOK(FRODOHook, FRODO);
    PAL_HOOK(GODOTHook, GODOT);
    PAL_HOOK(PC64Hook, PC64);
    PAL_HOOK(PEPTO_NTSCHook, PEPTO_NTSC);
    PAL_HOOK(PEPTO_NTSC_SONYHook, PEPTO_NTSC_SONY);
    PAL_HOOK(PEPTO_PALHook, PEPTO_PAL);
    PAL_HOOK(PEPTO_PALOLDHook, PEPTO_PALOLD);
    PAL_HOOK(PTOINGHook, PTOING);
    PAL_HOOK(RGBHook, RGB);
    PAL_HOOK(VICE_VICIIHook, VICE_VICII);

#ifdef AMIGA_MORPHOS
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse_palette, NULL };
#else
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)Browse_palette, NULL, NULL };
#endif

    app = mui_get_app();

    ui = GroupObject,
           BUTTON(c64hq_button, "c64hq")
           BUTTON(c64s_button, "c64s")
           BUTTON(ccs64_button, "ccs64")
           BUTTON(community_colors_button, "community-colors")
           BUTTON(deekay_button, "deekay")
           BUTTON(frodo_button, "frodo")
           BUTTON(godot_button, "godot")
           BUTTON(pc64_button, "pc64")
           BUTTON(pepto_ntsc_button, "pepto-ntsc")
           BUTTON(pepto_ntsc_sony_button, "pepto-ntsc-sony")
           BUTTON(pepto_pal_button, "pepto-pal")
           BUTTON(pepto_palold_button, "pepto-palold")
           BUTTON(ptoing_button, "ptoing")
           BUTTON(rgb_button, "rgb")
           BUTTON(vice_button, "vice")
           CYCLE(ui_to_from_palette[0].object, ui_to_from_palette[0].resource, ui_video_enable)
           FILENAME(ui_to_from_palette[1].object, video_palette_filename_text, browse_button)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(browse_button, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseFileHook);

        PAL_METHOD(c64hq_button, C64HQHook);
        PAL_METHOD(c64s_button, C64SHook);
        PAL_METHOD(ccs64_button, CCS64Hook);
        PAL_METHOD(community_colors_button, COMMUNITY_COLORSHook);
        PAL_METHOD(deekay_button, DEEKAYHook);
        PAL_METHOD(frodo_button, FRODOHook);
        PAL_METHOD(godot_button, GODOTHook);
        PAL_METHOD(pc64_button, PC64Hook);
        PAL_METHOD(pepto_ntsc_button, PEPTO_NTSCHook);
        PAL_METHOD(pepto_ntsc_sony_button, PEPTO_NTSC_SONYHook);
        PAL_METHOD(pepto_pal_button, PEPTO_PALHook);
        PAL_METHOD(pepto_palold_button, PEPTO_PALOLDHook);
        PAL_METHOD(ptoing_button, PTOINGHook);
        PAL_METHOD(rgb_button, RGBHook);
        PAL_METHOD(vice_button, VICE_VICIIHook);
    }

    return ui;
}

PAL_SET(SPIFF, "VICIIPaletteFile", "spiff.vpl")

static APTR build_gui_palette_viciidtv(void)
{
    APTR app, ui, ok, browse_button, cancel;
    APTR spiff_button;

    PAL_HOOK(SPIFFHook, SPIFF);

#ifdef AMIGA_MORPHOS
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse_palette, NULL };
#else
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)Browse_palette, NULL, NULL };
#endif

    app = mui_get_app();

    ui = GroupObject,
           BUTTON(spiff_button, "spiff")
           CYCLE(ui_to_from_palette[0].object, ui_to_from_palette[0].resource, ui_video_enable)
           FILENAME(ui_to_from_palette[1].object, video_palette_filename_text, browse_button)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(browse_button, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseFileHook);

        PAL_METHOD(spiff_button, SPIFFHook);
    }

    return ui;
}

PAL_SET(VDC_COMP, "VDCPaletteFile", "vdc_comp.vpl")
PAL_SET(VDC_DEFT, "VDCPaletteFile", "vdc_deft.vpl")

static APTR build_gui_palette_vdc(void)
{
    APTR app, ui, ok, browse_button, cancel;
    APTR vdc_comp_button, vdc_deft_button;

    PAL_HOOK(VDC_COMPHook, VDC_COMP);
    PAL_HOOK(VDC_DEFTHook, VDC_DEFT);

#ifdef AMIGA_MORPHOS
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse_palette, NULL };
#else
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)Browse_palette, NULL, NULL };
#endif

    app = mui_get_app();

    ui = GroupObject,
           BUTTON(vdc_comp_button, "vdc-comp")
           BUTTON(vdc_deft_button, "vdc-deft")
           CYCLE(ui_to_from_palette[0].object, ui_to_from_palette[0].resource, ui_video_enable)
           FILENAME(ui_to_from_palette[1].object, video_palette_filename_text, browse_button)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(browse_button, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseFileHook);

        PAL_METHOD(vdc_comp_button, VDC_COMPHook);
        PAL_METHOD(vdc_deft_button, VDC_DEFTHook);
    }

    return ui;
}

PAL_SET(AMBER, "CrtcPaletteFile", "amber.vpl")
PAL_SET(GREEN, "CrtcPaletteFile", "green.vpl")
PAL_SET(WHITE, "CrtcPaletteFile", "white.vpl")

static APTR build_gui_palette_crtc(void)
{
    APTR app, ui, ok, browse_button, cancel;
    APTR amber_button, green_button, white_button;

    PAL_HOOK(AMBERHook, AMBER);
    PAL_HOOK(GREENHook, GREEN);
    PAL_HOOK(WHITEHook, WHITE);

#ifdef AMIGA_MORPHOS
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse_palette, NULL };
#else
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)Browse_palette, NULL, NULL };
#endif

    app = mui_get_app();

    ui = GroupObject,
           BUTTON(amber_button, "amber")
           BUTTON(green_button, "green")
           BUTTON(white_button, "white")
           CYCLE(ui_to_from_palette[0].object, ui_to_from_palette[0].resource, ui_video_enable)
           FILENAME(ui_to_from_palette[1].object, video_palette_filename_text, browse_button)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(browse_button, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseFileHook);

        PAL_METHOD(amber_button, AMBERHook);
        PAL_METHOD(green_button, GREENHook);
        PAL_METHOD(white_button, WHITEHook);
    }

    return ui;
}

PAL_SET(YAPE_NTSC, "TEDPaletteFile", "yape-ntsc.vpl")
PAL_SET(YAPE_PAL, "TEDPaletteFile", "yape-pal.vpl")

static APTR build_gui_palette_ted(void)
{
    APTR app, ui, ok, browse_button, cancel;
    APTR yape_ntsc_button, yape_pal_button;

    PAL_HOOK(YAPE_NTSCHook, YAPE_NTSC);
    PAL_HOOK(YAPE_PALHook, YAPE_PAL);

#ifdef AMIGA_MORPHOS
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse_palette, NULL };
#else
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)Browse_palette, NULL, NULL };
#endif

    app = mui_get_app();

    ui = GroupObject,
           BUTTON(yape_ntsc_button, "yape-ntsc")
           BUTTON(yape_pal_button, "yape-pal")
           CYCLE(ui_to_from_palette[0].object, ui_to_from_palette[0].resource, ui_video_enable)
           FILENAME(ui_to_from_palette[1].object, video_palette_filename_text, browse_button)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(browse_button, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseFileHook);

        PAL_METHOD(yape_ntsc_button, YAPE_NTSCHook);
        PAL_METHOD(yape_pal_button, YAPE_PALHook);
    }

    return ui;
}

PAL_SET(MIKE_NTSC, "VICPaletteFile", "mike-ntsc.vpl")
PAL_SET(MIKE_PAL, "VICPaletteFile", "mike-pal.vpl")
PAL_SET(VICE_VIC, "TEDPaletteFile", "vice.vpl")

static APTR build_gui_palette_vic(void)
{
    APTR app, ui, ok, browse_button, cancel;
    APTR mike_ntsc_button, mike_pal_button, vice_button;

    PAL_HOOK(MIKE_NTSCHook, MIKE_NTSC);
    PAL_HOOK(MIKE_PALHook, MIKE_PAL);
    PAL_HOOK(VICE_VICHook, VICE_VIC);

#ifdef AMIGA_MORPHOS
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse_palette, NULL };
#else
    static const struct Hook BrowseFileHook = { { NULL, NULL }, (VOID *)Browse_palette, NULL, NULL };
#endif

    app = mui_get_app();

    ui = GroupObject,
           BUTTON(mike_ntsc_button, "mike-ntsc")
           BUTTON(mike_pal_button, "mike-pal")
           BUTTON(vice_button, "vice")
           CYCLE(ui_to_from_palette[0].object, ui_to_from_palette[0].resource, ui_video_enable)
           FILENAME(ui_to_from_palette[1].object, video_palette_filename_text, browse_button)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(browse_button, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseFileHook);

        PAL_METHOD(mike_ntsc_button, MIKE_NTSCHook);
        PAL_METHOD(mike_pal_button, MIKE_PALHook);
        PAL_METHOD(vice_button, VICE_VICHook);
    }

    return ui;
}

static APTR build_gui_colors(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           NSTRING(ui_to_from_colors[0].object, translate_text(IDS_GAMMA_0_4), ".0123456789", 5+1)
           NSTRING(ui_to_from_colors[1].object, translate_text(IDS_TINT_0_2), ".0123456789", 5+1)
           NSTRING(ui_to_from_colors[2].object, translate_text(IDS_SATURATION_0_2), ".0123456789", 5+1)
           NSTRING(ui_to_from_colors[3].object, translate_text(IDS_CONTRAST_0_2), ".0123456789", 5+1)
           NSTRING(ui_to_from_colors[4].object, translate_text(IDS_BRIGHTNESS_0_2), ".0123456789", 5+1)
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

static APTR build_gui_render_filter(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(ui_to_from_render_filter[0].object, ui_to_from_render_filter[0].resource, ui_render_filter)
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

static APTR build_gui_crt(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           NSTRING(ui_to_from_crt[0].object, translate_text(IDS_SCANLINE_SHADE_0_1), ".0123456789", 5+1)
           NSTRING(ui_to_from_crt[1].object, translate_text(IDS_BLUR_0_1), ".0123456789", 5+1)
           NSTRING(ui_to_from_crt[2].object, translate_text(IDS_ODDLINE_PHASE_0_2), ".0123456789", 5+1)
           NSTRING(ui_to_from_crt[3].object, translate_text(IDS_ODDLINE_OFFSET_0_2), ".0123456789", 5+1)
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

void ui_video_palette_settings_dialog(video_canvas_t *canvas, char *palette_enable_res, char *palette_filename_res, char *palette_filename_text)
{
    APTR window;

    video_canvas = canvas;
    ui_to_from_palette[0].resource = palette_enable_res;
    ui_to_from_palette[1].resource = palette_filename_res;
    video_palette_filename_text = palette_filename_text;
    intl_convert_mui_table(ui_video_enable_translate, ui_video_enable);

    switch (machine_class) {
        case VICE_MACHINE_C64:
        case VICE_MACHINE_CBM5x0:
        case VICE_MACHINE_C64SC:
        case VICE_MACHINE_SCPU64:
            window = mui_make_simple_window(build_gui_palette_vicii(), translate_text(IDS_PALETTE_SETTINGS));
            break;
        case VICE_MACHINE_C128:
            if (!strcmp(palette_filename_res, "VICIIPaletteFile")) {
                window = mui_make_simple_window(build_gui_palette_vicii(), translate_text(IDS_PALETTE_SETTINGS));
            } else {
                window = mui_make_simple_window(build_gui_palette_vdc(), translate_text(IDS_PALETTE_SETTINGS));
            }
            break;
        case VICE_MACHINE_C64DTV:
            window = mui_make_simple_window(build_gui_palette_viciidtv(), translate_text(IDS_PALETTE_SETTINGS));
            break;
        case VICE_MACHINE_VIC20:
            window = mui_make_simple_window(build_gui_palette_vic(), translate_text(IDS_PALETTE_SETTINGS));
            break;
        case VICE_MACHINE_PET:
        case VICE_MACHINE_CBM6x0:
            window = mui_make_simple_window(build_gui_palette_crtc(), translate_text(IDS_PALETTE_SETTINGS));
            break;
        case VICE_MACHINE_PLUS4:
            window = mui_make_simple_window(build_gui_palette_ted(), translate_text(IDS_PALETTE_SETTINGS));
            break;
    }

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from_palette);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            ui_get_from(ui_to_from_palette);
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}

void ui_video_color_settings_dialog(video_canvas_t *canvas, char *gamma_res, char *tint_res, char *saturation_res, char *contrast_res, char *brightness_res)
{
    APTR window;

    video_canvas = canvas;
    ui_to_from_colors[0].resource = gamma_res;
    ui_to_from_colors[1].resource = tint_res;
    ui_to_from_colors[2].resource = saturation_res;
    ui_to_from_colors[3].resource = contrast_res;
    ui_to_from_colors[4].resource = brightness_res;

    window = mui_make_simple_window(build_gui_colors(), translate_text(IDS_COLOR_SETTINGS));

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from_colors);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            ui_get_from(ui_to_from_colors);
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}

void ui_video_render_filter_settings_dialog(video_canvas_t *canvas, char *render_filter_res)
{
    APTR window;

    video_canvas = canvas;
    ui_to_from_render_filter[0].resource = render_filter_res;

    intl_convert_mui_table(ui_render_filter_translate, ui_render_filter);

    window = mui_make_simple_window(build_gui_render_filter(), translate_text(IDS_RENDER_FILTER_SETTINGS));

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from_render_filter);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            ui_get_from(ui_to_from_render_filter);
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}

void ui_video_crt_settings_dialog(video_canvas_t *canvas, char *scanline_shade_res, char *blur_res, char *oddline_phase_res, char *oddline_offset_res)
{
    APTR window;

    video_canvas = canvas;
    ui_to_from_crt[0].resource = scanline_shade_res;
    ui_to_from_crt[1].resource = blur_res;
    ui_to_from_crt[2].resource = oddline_phase_res;
    ui_to_from_crt[3].resource = oddline_offset_res;

    window = mui_make_simple_window(build_gui_crt(), translate_text(IDS_CRT_SETTINGS));

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from_crt);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            ui_get_from(ui_to_from_crt);
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}
