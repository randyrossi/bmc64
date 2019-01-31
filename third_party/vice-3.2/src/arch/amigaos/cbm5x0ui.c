/*
 * cbm5x0ui.c
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

#define UI_CBM5X0
#define UI_MENU_NAME cbm5x0_ui_translation_menu
#define UI_TRANSLATED_MENU_NAME cbm5x0_ui_menu

#include "private.h"
#include "cartridge.h"
#include "cbm2model.h"
#include "cbm2ui.h"
#include "cbm5x0uires.h"
#include "machine.h"
#include "translate.h"

#include "mui/mui.h"
#include "mui/uiacia.h"
#include "mui/uicbm5x0settings.h"
#include "mui/uiciamodel.h"
#include "mui/uicpclockf83.h"
#include "mui/uidatasette.h"
#include "mui/uidrivepetcbm2.h"
#include "mui/uiiocollisions.h"
#include "mui/uijoyport.h"
#include "mui/uijoystick.h"
#include "mui/uijoystickll.h"
#include "mui/uikeymap.h"
#include "mui/uimouse.h"
#include "mui/uiprinter.h"
#include "mui/uiromcbm2settings.h"
#include "mui/uisampler.h"
#include "mui/uisid.h"
#include "mui/uitapelog.h"
#include "mui/uivideo.h"

static const ui_menu_toggle_t cbm5x0_ui_menu_toggles[] = {
    { "VICIIDoubleSize", IDM_TOGGLE_DOUBLESIZE },
    { "VICIIDoubleScan", IDM_TOGGLE_DOUBLESCAN },
    { "VICIIVideoCache", IDM_TOGGLE_VIDEOCACHE },
    { "VICIIAudioLeak", IDM_TOGGLE_AUDIO_LEAK },
    { "CartridgeReset", IDM_TOGGLE_CART_RESET },
    { "Mouse", IDM_MOUSE },
    { "TapeSenseDongle", IDM_TOGGLE_TAPE_SENSE_DONGLE },
    { "DTLBasicDongle", IDM_TOGGLE_DTL_BASIC_DONGLE },
    { NULL, 0 }
};

static void cbm2_cart_attach(video_canvas_t *canvas, int cart_type)
{
    char *fname;

    fname = BrowseFile(translate_text(IDS_ATTACH_CART), "#?", canvas);

    if (fname != NULL) {
        if (cartridge_attach_image(cart_type, fname) < 0) {
            ui_error(translate_text(IDMES_INVALID_CART_IMAGE));
        }
    }
}

static int cbm5x0_ui_specific(video_canvas_t *canvas, int idm)
{
    switch (idm) {
        case IDM_CIA_SETTINGS:
            ui_single_cia_model_dialog();
            break;
        case IDM_LOAD_CART_1XXX:
            cbm2_cart_attach(canvas, CARTRIDGE_CBM2_8KB_1000);
            break;
        case IDM_UNLOAD_CART_1XXX:
            cartridge_detach_image(CARTRIDGE_CBM2_8KB_1000);
            break;
        case IDM_LOAD_CART_2_3XXX:
            cbm2_cart_attach(canvas, CARTRIDGE_CBM2_8KB_2000);
            break;
        case IDM_UNLOAD_CART_2_3XXX:
            cartridge_detach_image(CARTRIDGE_CBM2_8KB_2000);
            break;
        case IDM_LOAD_CART_4_5XXX:
            cbm2_cart_attach(canvas, CARTRIDGE_CBM2_16KB_4000);
            break;
        case IDM_UNLOAD_CART_4_5XXX:
            cartridge_detach_image(CARTRIDGE_CBM2_16KB_4000);
            break;
        case IDM_LOAD_CART_6_7XXX:
            cbm2_cart_attach(canvas, CARTRIDGE_CBM2_16KB_6000);
            break;
        case IDM_UNLOAD_CART_6_7XXX:
            cartridge_detach_image(CARTRIDGE_CBM2_16KB_6000);
            break;
        case IDM_CBM2_MODEL_510_PAL:
            cbm2model_set(CBM2MODEL_510_PAL);
            break;
        case IDM_CBM2_MODEL_510_NTSC:
            cbm2model_set(CBM2MODEL_510_NTSC);
            break;
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
        case IDM_CBM5X0_SETTINGS:
            ui_cbm5x0_settings_dialog();
            break;
        case IDM_SID_SETTINGS:
            ui_sid_settings2_dialog();
            break;
        case IDM_COMPUTER_ROM_SETTINGS:
            ui_cbm2_computer_rom_settings_dialog(canvas);
            break;
        case IDM_DRIVE_ROM_SETTINGS:
            ui_cbm2_drive_rom_settings_dialog(canvas);
            break;
        case IDM_DRIVE_SETTINGS:
            uidrivepetcbm2_settings_dialog();
            break;
        case IDM_PRINTER_SETTINGS:
            ui_printer_settings_dialog(canvas, 1, 1);
            break;
        case IDM_ACIA_SETTINGS:
            ui_acia_settings_dialog();
            break;
        case IDM_JOYPORT_SETTINGS:
            ui_joyport_settings_dialog(1, 1, 0, 0, 0);
            break;
#ifdef AMIGA_OS4
        case IDM_JOY_SETTINGS:
            ui_joystick_settings_cbm5x0_dialog();
            break;
#else
        case IDM_JOY_DEVICE_SELECTION:
            ui_joystick_device_cbm5x0_dialog();
            break;
        case IDM_JOY_FIRE_SELECTION:
            ui_joystick_fire_cbm5x0_dialog();
            break;
#endif
        case IDM_MOUSE_SETTINGS:
            ui_mouse_settings_dialog();
            break;
        case IDM_SAMPLER_SETTINGS:
            ui_sampler_settings_dialog(canvas);
            break;
        case IDM_IO_COLLISION_SETTINGS:
            ui_iocollisions_settings_dialog();
            break;
        case IDM_KEYBOARD_SETTINGS:
            ui_keymap_settings_dialog(canvas);
            break;
        case IDM_DATASETTE_SETTINGS:
            ui_datasette_settings_dialog();
            break;
        case IDM_TAPELOG_SETTINGS:
            ui_tapelog_settings_dialog(canvas);
            break;
        case IDM_CPCLOCKF83_SETTINGS:
            ui_cpclockf83_settings_dialog();
            break;
    }

    return 0;
}

int cbm5x0ui_init(void)
{
    ui_register_menu_translation_layout(cbm5x0_ui_translation_menu);
    ui_register_menu_layout(cbm5x0_ui_menu);
    ui_register_machine_specific(cbm5x0_ui_specific);
    ui_register_menu_toggles(cbm5x0_ui_menu_toggles);

    return 0;
}

void cbm5x0ui_shutdown(void)
{
}
