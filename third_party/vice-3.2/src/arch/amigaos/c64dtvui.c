/*
 * c64dtvui.c
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

#define UI_C64DTV
#define UI_MENU_NAME c64dtv_ui_translation_menu
#define UI_TRANSLATED_MENU_NAME c64dtv_ui_menu

#include "private.h"
#include "c64ui.h"
#include "c64dtvmodel.h"
#include "c64dtvuires.h"
#include "translate.h"

#include "mui/uic64dtv.h"
#include "mui/uidrivec64dtv.h"
#include "mui/uijoyport.h"
#include "mui/uijoystick.h"
#include "mui/uijoystickll.h"
#include "mui/uikeymap.h"
#include "mui/uiprinter.h"
#include "mui/uiromc64settings.h"
#include "mui/uisampler.h"
#include "mui/uisiddtv.h"
#include "mui/uivicii.h"
#include "mui/uivideo.h"

static const ui_menu_toggle_t c64dtv_ui_menu_toggles[] = {
    { "VICIIDoubleSize", IDM_TOGGLE_DOUBLESIZE },
    { "VICIIDoubleScan", IDM_TOGGLE_DOUBLESCAN },
    { "VICIIVideoCache", IDM_TOGGLE_VIDEOCACHE },
    { "VICIIAudioLeak", IDM_TOGGLE_AUDIO_LEAK },
    { "Mouse", IDM_MOUSE },
    { "ps2mouse", IDM_PS2_MOUSE },
    { NULL, 0 }
};

static int c64dtv_ui_specific(video_canvas_t *canvas, int idm)
{
    switch (idm) {
        case IDM_PALETTE_SETTINGS:
            ui_video_palette_settings_dialog(canvas, "VICIIExternalPalette", "VICIIPaletteFile", translate_text(IDS_VICII_PALETTE_FILENAME));
            break;
        case IDM_COLOR_SETTINGS:
            ui_video_color_settings_dialog(canvas, "VICIIColorGamma", "VICIIColorTint", "VICIIColorSaturation", "VICIIColorContrast", "VICIIColorBrightness");
            break;
        case IDM_RENDER_FILTER:
            ui_video_render_filter_settings_dialog(canvas, "VICIIFilter");
            break;
        case IDM_CRT_EMULATION_SETTINGS:
            ui_video_crt_settings_dialog(canvas, "VICIIPALScanLineShade", "VICIIPALBlur", "VICIIPALOddLinePhase", "VICIIPALOddLineOffset");
            break;
        case IDM_C64DTV_MODEL_V2_PAL:
            dtvmodel_set(DTVMODEL_V2_PAL);
            break;
        case IDM_C64DTV_MODEL_V2_NTSC:
            dtvmodel_set(DTVMODEL_V2_NTSC);
            break;
        case IDM_C64DTV_MODEL_V3_PAL:
            dtvmodel_set(DTVMODEL_V3_PAL);
            break;
        case IDM_C64DTV_MODEL_V3_NTSC:
            dtvmodel_set(DTVMODEL_V3_NTSC);
            break;
        case IDM_C64DTV_MODEL_HUMMER_NTSC:
            dtvmodel_set(DTVMODEL_HUMMER_NTSC);
            break;
        case IDM_C64DTV_SETTINGS:
            ui_c64dtv_settings_dialog(canvas);
            break;
        case IDM_VICII_SETTINGS:
            ui_viciidtv_settings_dialog();
            break;
        case IDM_SID_SETTINGS:
            ui_siddtv_settings_dialog();
            break;
        case IDM_COMPUTER_ROM_SETTINGS:
            ui_c64_computer_rom_settings_dialog(canvas);
            break;
        case IDM_DRIVE_ROM_SETTINGS:
            ui_c64_drive_rom_settings_dialog(canvas);
            break;
        case IDM_DRIVE_SETTINGS:
            uidrivec64dtv_settings_dialog();
            break;
        case IDM_PRINTER_SETTINGS:
            ui_printer_settings_dialog(canvas, 0, 0);
            break;
        case IDM_JOYPORT_SETTINGS:
            ui_joyport_settings_dialog(1, 1, 1, 0, 0);
            break;
#ifdef AMIGA_OS4
        case IDM_JOY_SETTINGS:
            ui_joystick_settings_c64dtv_dialog();
            break;
#else
        case IDM_JOY_DEVICE_SELECTION:
            ui_joystick_device_c64dtv_dialog();
            break;
        case IDM_JOY_FIRE_SELECTION:
            ui_joystick_fire_c64_dialog();
            break;
#endif
        case IDM_SAMPLER_SETTINGS:
            ui_sampler_settings_dialog(canvas);
            break;
        case IDM_KEYBOARD_SETTINGS:
            ui_keymap_settings_dialog(canvas);
            break;
    }

    return 0;
}

int c64dtvui_init(void)
{
    ui_register_menu_translation_layout(c64dtv_ui_translation_menu);
    ui_register_menu_layout(c64dtv_ui_menu);
    ui_register_machine_specific(c64dtv_ui_specific);
    ui_register_menu_toggles(c64dtv_ui_menu_toggles);

    return 0;
}

void c64dtvui_shutdown(void)
{
}
