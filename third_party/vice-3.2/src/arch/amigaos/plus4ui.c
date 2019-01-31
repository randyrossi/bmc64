/*
 * plus4ui.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#define UI_PLUS4
#define UI_MENU_NAME plus4_ui_translation_menu
#define UI_TRANSLATED_MENU_NAME plus4_ui_menu

#include "private.h"
#include "plus4cart.h"
#include "plus4model.h"
#include "plus4ui.h"
#include "plus4uires.h"
#include "uicart.h"
#include "lib.h"
#include "cartridge.h"
#include "intl.h"
#include "translate.h"

#include "mui/uiacia.h"
#include "mui/uicpclockf83.h"
#include "mui/uidatasette.h"
#include "mui/uidriveplus4.h"
#include "mui/uiiocollisions.h"
#include "mui/uijoyport.h"
#include "mui/uijoystick.h"
#include "mui/uijoystickll.h"
#include "mui/uikeymap.h"
#include "mui/uimouse.h"
#include "mui/uiplus4settings.h"
#include "mui/uiprinter.h"
#include "mui/uiromplus4settings.h"
#include "mui/uisampler.h"
#include "mui/uisidcart.h"
#include "mui/uitapelog.h"
#include "mui/uited.h"
#include "mui/uiv364speech.h"
#include "mui/uivideo.h"

static const ui_menu_toggle_t plus4_ui_menu_toggles[] = {
    { "TEDDoubleSize", IDM_TOGGLE_DOUBLESIZE },
    { "TEDDoubleScan", IDM_TOGGLE_DOUBLESCAN },
    { "TEDVideoCache", IDM_TOGGLE_VIDEOCACHE },
    { "TEDAudioLeak", IDM_TOGGLE_AUDIO_LEAK },
    { "CartridgeReset", IDM_TOGGLE_CART_RESET },
    { "Mouse", IDM_MOUSE },
    { "UserportDAC", IDM_TOGGLE_USERPORT_DAC },
    { "TapeSenseDongle", IDM_TOGGLE_TAPE_SENSE_DONGLE },
    { "DTLBasicDongle", IDM_TOGGLE_DTL_BASIC_DONGLE },
    { NULL, 0 }
};

static const uicart_params_t plus4_ui_cartridges[] = {
    { IDM_CART_PLUS4_SMART_ATTACH, 0, IDS_SELECT_CARTRIDGE_IMAGE, UILIB_FILTER_ALL },
    { IDM_CART_ATTACH_C1LO, 0, IDS_ATTACH_CART1_LOW, UILIB_FILTER_ALL },
    { IDM_CART_ATTACH_C1HI, 0, IDS_ATTACH_CART1_HIGH, UILIB_FILTER_ALL },
    { IDM_CART_ATTACH_C2LO, 0, IDS_ATTACH_CART2_LOW, UILIB_FILTER_ALL },
    { IDM_CART_ATTACH_C2HI, 0, IDS_ATTACH_CART2_HIGH, UILIB_FILTER_ALL },
    { 0, 0, 0, 0 }
};

static int uiplus4cart_attach_image(int type, char *s)
{
    switch (type) {
        case IDM_CART_PLUS4_SMART_ATTACH:
            return cartridge_attach_image(CARTRIDGE_PLUS4_DETECT, s);
        case IDM_CART_ATTACH_C0LO:
            return cartridge_attach_image(CARTRIDGE_PLUS4_16KB_C0LO, s);
        case IDM_CART_ATTACH_C0HI:
            return cartridge_attach_image(CARTRIDGE_PLUS4_16KB_C0HI, s);
        case IDM_CART_ATTACH_C1LO:
            return cartridge_attach_image(CARTRIDGE_PLUS4_16KB_C1LO, s);
        case IDM_CART_ATTACH_C1HI:
            return cartridge_attach_image(CARTRIDGE_PLUS4_16KB_C1HI, s);
        case IDM_CART_ATTACH_C2LO:
            return cartridge_attach_image(CARTRIDGE_PLUS4_16KB_C2LO, s);
        case IDM_CART_ATTACH_C2HI:
            return cartridge_attach_image(CARTRIDGE_PLUS4_16KB_C2HI, s);
    }
    return -1;
}

static void uiplus4cart_attach(video_canvas_t *canvas, int idm, const uicart_params_t *cartridges)
{
    int i;
    char *name;

    i = 0;

    while ((cartridges[i].idm != idm) && (cartridges[i].idm != 0)) {
        i++;
    }

    if (cartridges[i].idm == 0) {
        ui_error(translate_text(IDMES_BAD_CART_CONFIG_IN_UI));
        return;
    }

    if ((name = uilib_select_file(translate_text(cartridges[i].title),
        cartridges[i].filter, UILIB_SELECTOR_TYPE_FILE_LOAD,
        UILIB_SELECTOR_STYLE_CART)) != NULL) {

        if (uiplus4cart_attach_image(cartridges[i].idm, name) < 0) {
            ui_error(translate_text(IDMES_INVALID_CART_IMAGE));
        }
        lib_free(name);
    }
}

static int plus4_ui_specific(video_canvas_t *canvas, int idm)
{
    switch (idm) {
        case IDM_TED_SETTINGS:
            ui_ted_settings_dialog();
            break;
        case IDM_PALETTE_SETTINGS:
            ui_video_palette_settings_dialog(canvas, "TEDExternalPalette", "TEDPaletteFile", translate_text(IDS_TED_PALETTE_FILENAME));
            break;
        case IDM_COLOR_SETTINGS:
            ui_video_color_settings_dialog(canvas, "TEDColorGamma", "TEDColorTint", "TEDColorSaturation", "TEDColorContrast", "TEDColorBrightness");
            break;
        case IDM_RENDER_FILTER:
            ui_video_render_filter_settings_dialog(canvas, "TEDFilter");
            break;
        case IDM_CRT_EMULATION_SETTINGS:
            ui_video_crt_settings_dialog(canvas, "TEDPALScanLineShade", "TEDPALBlur", "TEDPALOddLinePhase", "TEDPALOddLineOffset");
            break;
        case IDM_PLUS4_MODEL_C16_PAL:
            plus4model_set(PLUS4MODEL_C16_PAL);
            break;
        case IDM_PLUS4_MODEL_C16_NTSC:
            plus4model_set(PLUS4MODEL_C16_NTSC);
            break;
        case IDM_PLUS4_MODEL_PLUS4_PAL:
            plus4model_set(PLUS4MODEL_PLUS4_PAL);
            break;
        case IDM_PLUS4_MODEL_PLUS4_NTSC:
            plus4model_set(PLUS4MODEL_PLUS4_NTSC);
            break;
        case IDM_PLUS4_MODEL_V364_NTSC:
            plus4model_set(PLUS4MODEL_V364_NTSC);
            break;
        case IDM_PLUS4_MODEL_C232_NTSC:
            plus4model_set(PLUS4MODEL_232_NTSC);
            break;
        case IDM_CART_PLUS4_SMART_ATTACH:
        case IDM_CART_ATTACH_C1LO:
        case IDM_CART_ATTACH_C1HI:
        case IDM_CART_ATTACH_C2LO:
        case IDM_CART_ATTACH_C2HI:
            uiplus4cart_attach(canvas, idm, plus4_ui_cartridges);
            break;
        case IDM_CART_DETACH:
            cartridge_detach_image(-1);
            break;
        case IDM_PLUS4_SETTINGS:
            ui_plus4_settings_dialog();
            break;
        case IDM_COMPUTER_ROM_SETTINGS:
            ui_plus4_computer_rom_settings_dialog(canvas);
            break;
        case IDM_DRIVE_ROM_SETTINGS:
            ui_plus4_drive_rom_settings_dialog(canvas);
            break;
        case IDM_DRIVE_SETTINGS:
            uidriveplus4_settings_dialog();
            break;
        case IDM_PRINTER_SETTINGS:
            ui_printer_settings_dialog(canvas, 0, 0);
            break;
        case IDM_ACIA_SETTINGS:
            ui_acia4_settings_dialog();
            break;
        case IDM_SIDCART_SETTINGS:
            ui_sidcart_plus4_settings_dialog("$FD40", "$FE80", "PLUS4", 0xfd40, 0xfe80);
            break;
        case IDM_V364_SPEECH_SETTINGS:
            ui_v364_speech_settings_dialog(canvas);
            break;
        case IDM_JOYPORT_SETTINGS:
            ui_joyport_settings_dialog(1, 1, 1, 1, 1);
            break;
#ifdef AMIGA_OS4
        case IDM_JOY_SETTINGS:
            ui_joystick_settings_plus4_dialog();
            break;
#else
        case IDM_JOY_DEVICE_SELECTION:
            ui_joystick_device_plus4_dialog();
            break;
        case IDM_JOY_FIRE_SELECTION:
            ui_joystick_fire_plus4_dialog();
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

int plus4ui_init(void)
{
    ui_register_menu_translation_layout(plus4_ui_translation_menu);
    ui_register_menu_layout(plus4_ui_menu);
    ui_register_machine_specific(plus4_ui_specific);
    ui_register_menu_toggles(plus4_ui_menu_toggles);

    return 0;
}

void plus4ui_shutdown(void)
{
}
