
/*
 * uivideo.c - video settings UI interface for MS-DOS.
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
#include "machine.h"
#include "palette.h"
#include "resources.h"
#include "ted.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "uiapi.h"
#include "uivideo.h"
#include "vic.h"
#include "vicii.h"

typedef struct ui_video_item_s {
    int video_id;
    char *id;
    char *cache_res;
    char *double_size_res;
    char *double_scan_res;
    char *render_res;
    char *audioleak_res;
    char *ext_pal_res;
    char *ext_pal_file_res;
    char *settings_name;
    char *settings_title;
    tui_menu_item_def_t *chip_menu;
    int has_video_standard;
    char *scanlineshade_res;
    char *blur_res;
    char *oddlinephase_res;
    char *oddlineoffset_res;
    char *gamma_res;
    char *tint_res;
    char *saturation_res;
    char *contrast_res;
    char *brightness_res;
    char *verticalstretch_res;
} ui_video_item_t;

TUI_MENU_DEFINE_RADIO(TEDBorderMode)

static TUI_MENU_CALLBACK(ted_border_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("TEDBorderMode", &value);
    switch (value) {
        default:
        case TED_NORMAL_BORDERS:
            s = "Normal";
            break;
        case TED_FULL_BORDERS:
            s = "Full";
            break;
        case TED_DEBUG_BORDERS:
            s = "Debug";
            break;
        case TED_NO_BORDERS:
            s = "None";
            break;
    }
    return s;
}

static tui_menu_item_def_t ted_border_submenu[] = {
    { "_Normal", NULL, radio_TEDBorderMode_callback,
      (void *)TED_NORMAL_BORDERS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Full", NULL, radio_TEDBorderMode_callback,
      (void *)TED_FULL_BORDERS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Debug", NULL, radio_TEDBorderMode_callback,
      (void *)TED_DEBUG_BORDERS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_None", NULL, radio_TEDBorderMode_callback,
      (void *)TED_NO_BORDERS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ted_menu_items[] = {
    { "Border mode:", "Select the border mode",
      ted_border_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, ted_border_submenu,
      "Border mode" },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_RADIO(VICBorderMode)

static TUI_MENU_CALLBACK(vic_border_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("VICBorderMode", &value);
    switch (value) {
        default:
        case VIC_NORMAL_BORDERS:
            s = "Normal";
            break;
        case VIC_FULL_BORDERS:
            s = "Full";
            break;
        case VIC_DEBUG_BORDERS:
            s = "Debug";
            break;
        case VIC_NO_BORDERS:
            s = "None";
            break;
    }
    return s;
}

static tui_menu_item_def_t vic_border_submenu[] = {
    { "_Normal", NULL, radio_VICBorderMode_callback,
      (void *)VIC_NORMAL_BORDERS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Full", NULL, radio_VICBorderMode_callback,
      (void *)VIC_FULL_BORDERS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Debug", NULL, radio_VICBorderMode_callback,
      (void *)VIC_DEBUG_BORDERS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_None", NULL, radio_VICBorderMode_callback,
      (void *)VIC_NO_BORDERS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t vic_menu_items[] = {
    { "Border mode:", "Select the border mode",
      vic_border_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, vic_border_submenu,
      "Border mode" },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(VICIICheckSsColl)
TUI_MENU_DEFINE_TOGGLE(VICIICheckSbColl)
TUI_MENU_DEFINE_RADIO(VICIIBorderMode)

static TUI_MENU_CALLBACK(vicii_border_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("VICIIBorderMode", &value);
    switch (value) {
        default:
        case VICII_NORMAL_BORDERS:
            s = "Normal";
            break;
        case VICII_FULL_BORDERS:
            s = "Full";
            break;
        case VICII_DEBUG_BORDERS:
            s = "Debug";
            break;
        case VICII_NO_BORDERS:
            s = "None";
            break;
    }
    return s;
}

static tui_menu_item_def_t vicii_border_submenu[] = {
    { "_Normal", NULL, radio_VICIIBorderMode_callback,
      (void *)VICII_NORMAL_BORDERS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Full", NULL, radio_VICIIBorderMode_callback,
      (void *)VICII_FULL_BORDERS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Debug", NULL, radio_VICIIBorderMode_callback,
      (void *)VICII_DEBUG_BORDERS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_None", NULL, radio_VICIIBorderMode_callback,
      (void *)VICII_NO_BORDERS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t vicii_menu_items[] = {
    { "Border mode:", "Select the border mode",
      vicii_border_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, vicii_border_submenu,
      "Border mode" },
    { "Sprite-_Background Collisions:",
      "Emulate sprite-background collision register",
      toggle_VICIICheckSbColl_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Sprite-_Sprite Collisions:",
      "Emulate sprite-sprite collision register",
      toggle_VICIICheckSsColl_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(VICIIVSPBug)

static tui_menu_item_def_t viciisc_menu_items[] = {
    { "Border mode:", "Select the border mode",
      vicii_border_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, vicii_border_submenu,
      "Border mode" },
    { "Sprite-_Background Collisions:",
      "Emulate sprite-background collision register",
      toggle_VICIICheckSbColl_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Sprite-_Sprite Collisions:",
      "Emulate sprite-sprite collision register",
      toggle_VICIICheckSsColl_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "VSP bug:",
      "Emulate the VSP bug",
      toggle_VICIIVSPBug_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(VDC64KB)
TUI_MENU_DEFINE_RADIO(VDCRevision)

static TUI_MENU_CALLBACK(vdc_revision_submenu_callback)
{
    int value;
    static char s[100];

    resources_get_int("VDCRevision", &value);
    sprintf(s, "Rev %d",value);
    return s;
}

static tui_menu_item_def_t vdc_revision_submenu[] = {
    { "Rev _0", NULL, radio_VDCRevision_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Rev _1", NULL, radio_VDCRevision_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Rev _2", NULL, radio_VDCRevision_callback,
      (void *)2, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t vdc_menu_items[] = {
    { "VDC _64KB video memory",
      "Emulate a VDC with 64KB video RAM",
      toggle_VDC64KB_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "VDC _revision:", "Select the revision of the VDC",
      vdc_revision_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, vdc_revision_submenu,
      "VDC revision" },
    TUI_MENU_ITEM_DEF_LIST_END
};

static ui_video_item_t video_item[] = {
    { VID_NONE, NULL, NULL,
      NULL, NULL,
      NULL, NULL,
      NULL, NULL,
      NULL, NULL,
      NULL, 0,
      NULL, NULL,
      NULL, NULL,
      NULL, NULL,
      NULL, NULL,
      NULL, NULL },
    { VID_VIC, "VIC", "VICVideoCache",
      "VICDoubleSize", "VICDoubleScan",
      "VICFilter", "VICAudioLeak",
      "VICExternalPalette", "VICPaletteFile",
      "VIC settings...", "VIC settings",
      vic_menu_items, 1,
      "VICPALScanLineShade", "VICPALBlur",
      "VICPALOddLinePhase", "VICPALOddLineOffset",
      "VICColorGamma", "VICColorTint",
      "VICColorSaturation", "VICColorContrast",
      "VICColorBrightness", NULL },
    { VID_VICII, "VICII", "VICIIVideoCache",
      "VICIIDoubleSize", "VICIIDoubleScan",
      "VICIIFilter", "VICIIAudioLeak",
      "VICIIExternalPalette", "VICIIPaletteFile",
      "VICII settings...", "VICII settings",
      vicii_menu_items, 1,
      "VICIIPALScanLineShade", "VICIIPALBlur",
      "VICIIPALOddLinePhase", "VICIIPALOddLineOffset",
      "VICIIColorGamma", "VICIIColorTint",
      "VICIIColorSaturation", "VICIIColorContrast",
      "VICIIColorBrightness", NULL },
    { VID_VICIISC, "VICII", "VICIIVideoCache",
      "VICIIDoubleSize", "VICIIDoubleScan",
      "VICIIFilter", "VICIIAudioLeak",
      "VICIIExternalPalette", "VICIIPaletteFile",
      "VICII settings...", "VICII settings",
      viciisc_menu_items, 1,
      "VICIIPALScanLineShade", "VICIIPALBlur",
      "VICIIPALOddLinePhase", "VICIIPALOddLineOffset",
      "VICIIColorGamma", "VICIIColorTint",
      "VICIIColorSaturation", "VICIIColorContrast",
      "VICIIColorBrightness", NULL },
    { VID_TED, "TED", "TEDVideoCache",
      "TEDDoubleSize", "TEDDoubleScan",
      "TEDFilter", "TEDAudioLeak",
      "TEDExternalPalette", "TEDPaletteFile",
      "TED settings...", "TED settings",
      ted_menu_items, 1,
      "TEDPALScanLineShade", "TEDPALBlur",
      "TEDPALOddLinePhase", "TEDPALOddLineOffset",
      "TEDColorGamma", "TEDColorTint",
      "TEDColorSaturation", "TEDColorContrast",
      "TEDColorBrightness", NULL },
    { VID_VDC, "VDC", NULL,
      "VDCDoubleSize", "VDCDoubleScan",
      "VDCFilter", "VDCAudioLeak",
      "VDCExternalPalette", "VDCPaletteFile",
      "VDC settings...", "VDC settings",
      vdc_menu_items, 0,
      "VDCPALScanLineShade", "VDCPALBlur",
      "VDCPALOddLinePhase", "VDCPALOddLineOffset",
      "VDCColorGamma", "VDCColorTint",
      "VDCColorSaturation", "VDCColorContrast",
      "VDCColorBrightness", "VDCStretchVertical" },
    { VID_CRTC, "Crtc", "CrtcVideoCache",
      "CrtcDoubleSize", "CrtcDoubleScan",
      "CrtcFilter", "CrtcAudioLeak",
      "CrtcExternalPalette", "CrtcPaletteFile",
      "CRTC settings...", "CRTC settings",
      NULL, 0,
      "CrtcPALScanLineShade", "CrtcPALBlur",
      "CrtcPALOddLinePhase", "CrtcPALOddLineOffset",
      "CrtcColorGamma", "CrtcColorTint",
      "CrtcColorSaturation", "CrtcColorContrast",
      "CrtcColorBrightness", "CrtcStretchVertical" }
};

TUI_MENU_DEFINE_RADIO(MachineVideoStandard)

static TUI_MENU_CALLBACK(video_standard_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("MachineVideoStandard", &value);
    switch (value) {
        default:
        case MACHINE_SYNC_PAL:
            s = "PAL";
            break;
        case MACHINE_SYNC_NTSC:
            s = "NTSC";
            break;
    }
    return s;
}

static tui_menu_item_def_t video_standard_submenu[] = {
    { "_PAL", NULL, radio_MachineVideoStandard_callback,
      (void *)MACHINE_SYNC_PAL, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_NTSC", NULL, radio_MachineVideoStandard_callback,
      (void *)MACHINE_SYNC_NTSC, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t video_standard_menu_items[] = {
    { "V_ideo Standard:", "Select machine clock ratio",
      video_standard_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, video_standard_submenu,
      "Video standard" },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(toggle_cache_callback)
{
    int i = (int)param;

    return _tui_menu_toggle_helper(been_activated, video_item[i].cache_res);
}

static TUI_MENU_CALLBACK(toggle_doublesize_callback)
{
    int i = (int)param;

    return _tui_menu_toggle_helper(been_activated, video_item[i].double_size_res);
}

static TUI_MENU_CALLBACK(toggle_doublescan_callback)
{
    int i = (int)param;

    return _tui_menu_toggle_helper(been_activated, video_item[i].double_scan_res);
}

static TUI_MENU_CALLBACK(toggle_audioleak_callback)
{
    int i = (int)param;

    return _tui_menu_toggle_helper(been_activated, video_item[i].audioleak_res);
}

static TUI_MENU_CALLBACK(toggle_verticalstretch_callback)
{
    int i = (int)param;

    return _tui_menu_toggle_helper(been_activated, video_item[i].verticalstretch_res);
}

static TUI_MENU_CALLBACK(radio_renderfilter_callback)
{
    int i;
    int video_index;
    int render_index;
    int render_filter;

    i = (int)param;
    video_index = i >> 4;
    render_index = i & 0xf;

    if (been_activated) {
        resources_set_int(video_item[video_index].render_res, render_index);
        *become_default = 1;
    } else {
        resources_get_int(video_item[video_index].render_res, &render_filter);
        if (render_index == render_filter) {
            *become_default = 1;
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(toggle_external_palette_callback)
{
    char *resource = (char *)param;
    char *s;
    int val;

    if (been_activated) {
        resources_toggle(resource, NULL);
    }
    resources_get_int(resource, &val);

    return (val) ? "On" : "Off";
}

static TUI_MENU_CALLBACK(custom_palette_callback)
{
    char *resource = (char *)param;

    if (been_activated) {
        char *name;

        name = tui_file_selector("Load custom palette", NULL, "*.vpl", NULL, NULL, NULL, NULL);

        if (name != NULL) {
            if (resources_set_string(resource, name) < 0) {
                tui_error("Invalid palette file");
            }
            ui_update_menus();
            lib_free(name);
        }
    }
    return NULL;
}

/* being lazy ;) */
#define float_inputs(callback, name, range, start, end)                                         \
    static TUI_MENU_CALLBACK(callback)                                                          \
    {                                                                                           \
        int value;                                                                              \
        int val;                                                                                \
        float f;                                                                                \
        char buf[44];                                                                           \
        char *resource = (char *)param;                                                         \
                                                                                                \
        if (been_activated) {                                                                   \
            resources_get_int(resource, &value);                                                \
                                                                                                \
            sprintf(buf, "%.3f", (float)(value / 1000.0));                                      \
                                                                                                \
            if (tui_input_string(name, "Enter the value for "name " "range":", buf, 40) == 0) { \
                f = atof(buf);                                                                  \
                value = (int)(f * 1000.0);                                                      \
                if (value < start) {                                                            \
                    value = start;                                                              \
                }                                                                               \
                if (value > end) {                                                              \
                    value = end;                                                                \
                }                                                                               \
                resources_set_int(resource, value);                                             \
                val = value / 1000;                                                             \
                tui_message(name" set to : %d.%03d", val, value - (val * 1000));                \
            } else {                                                                            \
                return NULL;                                                                    \
            }                                                                                   \
        }                                                                                       \
        return NULL;                                                                            \
    }

float_inputs(scanlineshade_callback, "Scan line shade", "(0..1)", 0, 1000)
float_inputs(blur_callback, "Blur", "(0..1)", 0, 1000)
float_inputs(oddlinephase_callback, "Oddline phase", "(0..2)", 0, 2000)
float_inputs(oddlineoffset_callback, "Oddline offset", "(0..2)", 0, 2000)

float_inputs(gamma_callback, "Gamma", "(0..4)", 0, 4000)
float_inputs(tint_callback, "Tint", "(0..2)", 0, 2000)
float_inputs(saturation_callback, "Saturation", "(0..2)", 0, 2000)
float_inputs(contrast_callback, "Contrast", "(0..2)", 0, 2000)
float_inputs(brightness_callback, "Brightness", "(0..2)", 0, 2000)

int video_chip_index1 = 0;
int video_chip_index2 = 0;

static TUI_MENU_CALLBACK(available_palette1_callback)
{
    char *name = (char *)param;
    char *res_name = video_item[video_chip_index1].ext_pal_file_res;
    const char *name_set = NULL;

    if (been_activated) {
        resources_set_string(res_name, name);
        *become_default = 1;
    } else {
        resources_get_string(res_name, &name_set);
        if (!strcmp(name_set, name)) {
            *become_default = 1;
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(available_palette2_callback)
{
    char *name = (char *)param;
    char *res_name = video_item[video_chip_index2].ext_pal_file_res;
    const char *name_set = NULL;

    if (been_activated) {
        resources_set_string(res_name, name);
        *become_default = 1;
    } else {
        resources_get_string(res_name, &name_set);
        if (!strcmp(name_set, name)) {
            *become_default = 1;
        }
    }
    return NULL;
}

void uivideo_init(struct tui_menu *parent_submenu, int vid1, int vid2)
{
    tui_menu_t video_submenu1;
    tui_menu_t video_submenu2;
    tui_menu_t render_submenu1;
    tui_menu_t render_submenu2;
    tui_menu_t crt_emulation_submenu1;
    tui_menu_t crt_emulation_submenu2;
    tui_menu_t colors_submenu1;
    tui_menu_t colors_submenu2;
    tui_menu_t palette_smenu1;
    tui_menu_t palette_smenu2;
    palette_info_t *palettelist = palette_get_info_list();

    video_chip_index1 = vid1;
    video_chip_index2 = vid2;

    if (vid2 != VID_NONE) {
        video_submenu1 = tui_menu_create(video_item[vid1].settings_name, 1);
        video_submenu2 = tui_menu_create(video_item[vid2].settings_name, 1);
    } else {
        video_submenu1 = parent_submenu;
    }

    tui_menu_add_item(video_submenu1, "Video cache",
                      "Enable video cache",
                      toggle_cache_callback,
                      (void *)vid1, 3,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_item(video_submenu1, "Double size",
                      "Enable double size",
                      toggle_doublesize_callback,
                      (void *)vid1, 3,
                      TUI_MENU_BEH_CONTINUE);

    if (vid2 != VID_NONE) {
        tui_menu_add_item(video_submenu2, "Double size",
                          "Enable double size",
                          toggle_doublesize_callback,
                          (void *)vid2, 3,
                          TUI_MENU_BEH_CONTINUE);
    }

    tui_menu_add_item(video_submenu1, "Double scan",
                      "Enable double scan",
                      toggle_doublescan_callback,
                      (void *)vid1, 3,
                      TUI_MENU_BEH_CONTINUE);

    if (vid2 != VID_NONE) {
        tui_menu_add_item(video_submenu2, "Double scan",
                          "Enable double scan",
                          toggle_doublesize_callback,
                          (void *)vid2, 3,
                          TUI_MENU_BEH_CONTINUE);
    }

    if (video_item[vid1].verticalstretch_res) {
        tui_menu_add_item(video_submenu1, "Vertical stretch",
                          "Enable vertical stretch",
                          toggle_verticalstretch_callback,
                          (void *)vid1, 3,
                          TUI_MENU_BEH_CONTINUE);
    }

    if (video_item[vid2].verticalstretch_res) {
        tui_menu_add_item(video_submenu2, "Vertical stretch",
                          "Enable vertical stretch",
                          toggle_verticalstretch_callback,
                          (void *)vid2, 3,
                          TUI_MENU_BEH_CONTINUE);
    }

    tui_menu_add_item(video_submenu1, "Audio leak",
                      "Enable audio leak",
                      toggle_audioleak_callback,
                      (void *)vid1, 3,
                      TUI_MENU_BEH_CONTINUE);

    if (vid2 != VID_NONE) {
        tui_menu_add_item(video_submenu2, "Audio leak",
                          "Enable audio leak",
                          toggle_audioleak_callback,
                          (void *)vid2, 3,
                          TUI_MENU_BEH_CONTINUE);
    }

    tui_menu_add_separator(video_submenu1);

    if (vid2 != VID_NONE) {
        tui_menu_add_separator(video_submenu2);
    }

    if (video_item[vid1].chip_menu != NULL) {
        tui_menu_add(video_submenu1, video_item[vid1].chip_menu);
        if (machine_class != VICE_MACHINE_C64SC && video_item[vid1].has_video_standard == 1) {
            tui_menu_add(video_submenu1, video_standard_menu_items);
        }
        tui_menu_add_separator(video_submenu1);
    }

    if (vid2 != VID_NONE) {
        if (video_item[vid2].chip_menu != NULL) {
            tui_menu_add(video_submenu2, video_item[vid2].chip_menu);
            tui_menu_add_separator(video_submenu2);
        }
    }

    tui_menu_add_item(video_submenu1, "External palette",
                      "Use external palette file",
                      toggle_external_palette_callback,
                      (void *)video_item[vid1].ext_pal_res, 3,
                      TUI_MENU_BEH_CONTINUE);

    palette_smenu1 = tui_menu_create("Available palettes", 1);

    while (palettelist->name) {
        if (palettelist->chip && !strcmp(palettelist->chip, video_item[vid1].id)) {
            tui_menu_add_item(palette_smenu1, palettelist->name,
                              NULL,
                              available_palette1_callback,
                              (void *)palettelist->file, 20,
                              TUI_MENU_BEH_CONTINUE);

        }
        ++palettelist;
    }

    tui_menu_add_submenu(video_submenu1, "Available palettes",
                         "Available palettes",
                         palette_smenu1,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    tui_menu_add_item(video_submenu1, "Choose external palette",
                      "Load a custom palette",
                      custom_palette_callback,
                      (void *)video_item[vid1].ext_pal_file_res, 0,
                      TUI_MENU_BEH_RESUME);

    if (vid2 != VID_NONE) {
        tui_menu_add_item(video_submenu2, "External palette",
                          "Use external palette file",
                          toggle_external_palette_callback,
                          (void *)video_item[vid2].ext_pal_res, 3,
                          TUI_MENU_BEH_CONTINUE);

        palette_smenu2 = tui_menu_create("Available palettes", 1);

        palettelist = palette_get_info_list();

        while (palettelist->name) {
            if (palettelist->chip && !strcmp(palettelist->chip, video_item[vid1].id)) {
                tui_menu_add_item(palette_smenu2, palettelist->name,
                                  NULL,
                                  available_palette2_callback,
                                  (void *)palettelist->file, 20,
                                  TUI_MENU_BEH_CONTINUE);

            }
            ++palettelist;
        }


        tui_menu_add_submenu(video_submenu2, "Available palettes",
                             "Available palettes",
                             palette_smenu2,
                             NULL, 0,
                             TUI_MENU_BEH_CONTINUE);

        tui_menu_add_item(video_submenu2, "Choose external palette",
                          "Load a custom palette",
                          custom_palette_callback,
                          (void *)video_item[vid2].ext_pal_file_res, 0,
                          TUI_MENU_BEH_RESUME);
    }

    if (vid2 != VID_NONE) {
        tui_menu_add_submenu(parent_submenu, video_item[vid1].settings_name,
                         video_item[vid1].settings_title,
                         video_submenu1,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

        tui_menu_add_submenu(parent_submenu, video_item[vid2].settings_name,
                         video_item[vid2].settings_title,
                         video_submenu2,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
    }

    render_submenu1 = tui_menu_create("Render filter", 1);

    tui_menu_add_item(render_submenu1, "None",
                      "No render filter",
                      radio_renderfilter_callback,
                      (void *)((vid1 << 4) | 0), 3,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_item(render_submenu1, "CRT emulation",
                      "CRT emulation",
                      radio_renderfilter_callback,
                      (void *)((vid1 << 4) | 1), 3,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_item(render_submenu1, "Scale2x",
                      "CRT emulation",
                      radio_renderfilter_callback,
                      (void *)((vid1 << 4) | 2), 3,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_submenu(video_submenu1, "Render filter",
                         "Render filter",
                         render_submenu1,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    if (vid2 != VID_NONE) {
        render_submenu2 = tui_menu_create("Render filter", 1);

        tui_menu_add_item(render_submenu2, "None",
                          "No render filter",
                          radio_renderfilter_callback,
                          (void *)((vid2 << 4) | 0), 3,
                          TUI_MENU_BEH_CONTINUE);

        tui_menu_add_item(render_submenu2, "CRT emulation",
                          "CRT emulation",
                          radio_renderfilter_callback,
                          (void *)((vid2 << 4) | 1), 3,
                          TUI_MENU_BEH_CONTINUE);

        tui_menu_add_item(render_submenu2, "Scale2x",
                          "CRT emulation",
                          radio_renderfilter_callback,
                          (void *)((vid2 << 4) | 2), 3,
                          TUI_MENU_BEH_CONTINUE);

        tui_menu_add_submenu(video_submenu2, "Render filter",
                             "Render filter",
                             render_submenu2,
                             NULL, 0,
                             TUI_MENU_BEH_CONTINUE);
    }

    crt_emulation_submenu1 = tui_menu_create("CRT emulation", 1);

    tui_menu_add_item(crt_emulation_submenu1, "Scan line shade",
                      "Adjust the scan line shade",
                      scanlineshade_callback,
                      (void *)video_item[vid1].scanlineshade_res, 0,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_item(crt_emulation_submenu1, "Blur",
                      "Adjust the blur",
                      blur_callback,
                      (void *)video_item[vid1].blur_res, 0,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_item(crt_emulation_submenu1, "Oddline phase",
                      "Adjust the oddline phase",
                      oddlinephase_callback,
                      (void *)video_item[vid1].oddlinephase_res, 0,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_item(crt_emulation_submenu1, "Oddline offset",
                      "Adjust the oddline offset",
                      oddlineoffset_callback,
                      (void *)video_item[vid1].oddlineoffset_res, 0,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_submenu(video_submenu1, "CRT emulation",
                         "CRT emulation",
                         crt_emulation_submenu1,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    colors_submenu1 = tui_menu_create("Colors", 1);

    tui_menu_add_item(colors_submenu1, "Gamma",
                      "Adjust the gamma",
                      gamma_callback,
                      (void *)video_item[vid1].gamma_res, 0,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_item(colors_submenu1, "Tint",
                      "Adjust the tint",
                      tint_callback,
                      (void *)video_item[vid1].tint_res, 0,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_item(colors_submenu1, "Saturation",
                      "Adjust the saturation",
                      saturation_callback,
                      (void *)video_item[vid1].saturation_res, 0,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_item(colors_submenu1, "Contrast",
                      "Adjust the contrast",
                      contrast_callback,
                      (void *)video_item[vid1].contrast_res, 0,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_item(colors_submenu1, "Brightness",
                      "Adjust the brightness",
                      brightness_callback,
                      (void *)video_item[vid1].brightness_res, 0,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_submenu(video_submenu1, "Colors",
                         "Colors",
                         colors_submenu1,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    if (vid2 != VID_NONE) {
        crt_emulation_submenu2 = tui_menu_create("CRT emulation", 1);

        tui_menu_add_item(crt_emulation_submenu2, "Scan line shade",
                          "Adjust the scan line shade",
                          scanlineshade_callback,
                          (void *)video_item[vid2].scanlineshade_res, 0,
                          TUI_MENU_BEH_CONTINUE);

        tui_menu_add_item(crt_emulation_submenu2, "Blur",
                          "Adjust the blur",
                          blur_callback,
                          (void *)video_item[vid2].blur_res, 0,
                          TUI_MENU_BEH_CONTINUE);

        tui_menu_add_item(crt_emulation_submenu2, "Oddline phase",
                          "Adjust the oddline phase",
                          oddlinephase_callback,
                          (void *)video_item[vid2].oddlinephase_res, 0,
                          TUI_MENU_BEH_CONTINUE);

        tui_menu_add_item(crt_emulation_submenu2, "Oddline offset",
                          "Adjust the oddline offset",
                          oddlineoffset_callback,
                          (void *)video_item[vid2].oddlineoffset_res, 0,
                          TUI_MENU_BEH_CONTINUE);

        tui_menu_add_submenu(video_submenu2, "CRT emulation",
                             "CRT emulation",
                             crt_emulation_submenu2,
                             NULL, 0,
                             TUI_MENU_BEH_CONTINUE);

        colors_submenu2 = tui_menu_create("Colors", 1);

        tui_menu_add_item(colors_submenu2, "Gamma",
                          "Adjust the gamma",
                          gamma_callback,
                          (void *)video_item[vid2].gamma_res, 0,
                          TUI_MENU_BEH_CONTINUE);

        tui_menu_add_item(colors_submenu2, "Tint",
                          "Adjust the tint",
                          tint_callback,
                          (void *)video_item[vid2].tint_res, 0,
                          TUI_MENU_BEH_CONTINUE);

        tui_menu_add_item(colors_submenu2, "Saturation",
                          "Adjust the saturation",
                          saturation_callback,
                          (void *)video_item[vid2].saturation_res, 0,
                          TUI_MENU_BEH_CONTINUE);

        tui_menu_add_item(colors_submenu2, "Contrast",
                          "Adjust the contrast",
                          contrast_callback,
                          (void *)video_item[vid2].contrast_res, 0,
                          TUI_MENU_BEH_CONTINUE);

        tui_menu_add_item(colors_submenu2, "Brightness",
                          "Adjust the brightness",
                          brightness_callback,
                          (void *)video_item[vid2].brightness_res, 0,
                          TUI_MENU_BEH_CONTINUE);

        tui_menu_add_submenu(video_submenu2, "Colors",
                             "Colors",
                             colors_submenu2,
                             NULL, 0,
                             TUI_MENU_BEH_CONTINUE);
    }
}
