/*
 * vic20ui.c
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

#define UI_VIC20
#define UI_MENU_NAME vic20_ui_translation_menu
#define UI_TRANSLATED_MENU_NAME vic20_ui_menu

#include "cartridge.h"
#include "private.h"
#include "translate.h"
#include "uicart.h"
#include "uilib.h"
#include "vic20model.h"
#include "vic20ui.h"
#include "vic20uires.h"

#include "mui/mui.h"
#include "mui/uiacia.h"
#include "mui/uicpclockf83.h"
#include "mui/uidatasette.h"
#include "mui/uidigimax.h"
#include "mui/uidrivevic20.h"
#include "mui/uids12c887rtc.h"
#include "mui/uigeoram.h"
#include "mui/uiiocollisions.h"
#include "mui/uijoyport.h"
#include "mui/uijoystick.h"
#include "mui/uijoystickll.h"
#include "mui/uikeymap.h"
#include "mui/uimouse.h"
#include "mui/uiprinter.h"
#include "mui/uiromvic20settings.h"
#include "mui/uirs232user.h"
#include "mui/uisampler.h"
#include "mui/uisidcart.h"
#include "mui/uisoundexpander.h"
#include "mui/uisoundsampler.h"
#include "mui/uitapelog.h"
#include "mui/uiuserportds1307rtc.h"
#include "mui/uiuserportrtc58321a.h"
#include "mui/uivic.h"
#include "mui/uivic20mem.h"
#include "mui/uivideo.h"

static const ui_menu_toggle_t vic20_ui_menu_toggles[] = {
    { "VICDoubleSize", IDM_TOGGLE_DOUBLESIZE },
    { "VICDoubleScan", IDM_TOGGLE_DOUBLESCAN },
    { "VICVideoCache", IDM_TOGGLE_VIDEOCACHE },
    { "VICAudioLeak", IDM_TOGGLE_AUDIO_LEAK },
    { "IEEE488", IDM_IEEE488 },
    { "CartridgeReset", IDM_TOGGLE_CART_RESET },
    { "FinalExpansionWriteBack", IDM_FINAL_EXPANSION_WRITEBACK },
    { "MegaCartNvRAMWriteBack", IDM_MEGACART_WRITEBACK },
    { "VicFlashPluginWriteBack", IDM_FP_WRITEBACK },
    { "UltiMemWriteBack", IDM_UM_WRITEBACK },
    { "Mouse", IDM_MOUSE },
    { "IO2RAM", IDM_IO2_RAM_ENABLE },
    { "IO3RAM", IDM_IO3_RAM_ENABLE },
    { "VFLImod", IDM_VFLI_MOD_ENABLE },
    { "UserportDAC", IDM_TOGGLE_USERPORT_DAC },
    { "TapeSenseDongle", IDM_TOGGLE_TAPE_SENSE_DONGLE },
    { "DTLBasicDongle", IDM_TOGGLE_DTL_BASIC_DONGLE },
    { NULL, 0 }
};

static const uicart_params_t vic20_ui_cartridges[] = {
    { IDM_CART_VIC20_8KB_2000, CARTRIDGE_VIC20_16KB_2000, IDS_ATTACH_4_8_16KB_AT_2000, UILIB_FILTER_ALL },
    { IDM_CART_VIC20_16KB_4000, CARTRIDGE_VIC20_16KB_4000, IDS_ATTACH_4_8_16KB_AT_4000, UILIB_FILTER_ALL },
    { IDM_CART_VIC20_8KB_6000, CARTRIDGE_VIC20_16KB_6000, IDS_ATTACH_4_8_16KB_AT_6000, UILIB_FILTER_ALL },
    { IDM_CART_VIC20_8KB_A000, CARTRIDGE_VIC20_8KB_A000, IDS_ATTACH_4_8KB_AT_A000, UILIB_FILTER_ALL },
    { IDM_CART_VIC20_4KB_B000, CARTRIDGE_VIC20_4KB_B000, IDS_ATTACH_4KB_AT_B000, UILIB_FILTER_ALL },
    { 0, 0, 0, 0 }
};

/* Probably one should simply remove the size numbers from the IDM_* stuff */
static int vic20_ui_specific(video_canvas_t *canvas, int idm)
{
    char *fname = NULL;

    switch (idm) {
        case IDM_VIC_SETTINGS:
            ui_vic_settings_dialog();
            break;
        case IDM_VIC20_MODEL_VIC20_PAL:
            vic20model_set(VIC20MODEL_VIC20_PAL);
            break;
        case IDM_VIC20_MODEL_VIC20_NTSC:
            vic20model_set(VIC20MODEL_VIC20_NTSC);
            break;
        case IDM_VIC20_MODEL_VIC21:
            vic20model_set(VIC20MODEL_VIC21);
            break;
        case IDM_PALETTE_SETTINGS:
            ui_video_palette_settings_dialog(canvas, "VICExternalPalette", "VICPaletteFile", translate_text(IDS_VIC_PALETTE_FILENAME));
            break;
        case IDM_COLOR_SETTINGS:
            ui_video_color_settings_dialog(canvas, "VICColorGamma", "VICColorTint", "VICColorSaturation", "VICColorContrast", "VICColorBrightness");
            break;
        case IDM_RENDER_FILTER:
            ui_video_render_filter_settings_dialog(canvas, "VICFilter");
            break;
        case IDM_CRT_EMULATION_SETTINGS:
            ui_video_crt_settings_dialog(canvas, "VICPALScanLineShade", "VICPALBlur", "VICPALOddLinePhase", "VICPALOddLineOffset");
            break;
        case IDM_CART_VIC20_8KB_2000:
        case IDM_CART_VIC20_16KB_4000:
        case IDM_CART_VIC20_8KB_6000:
        case IDM_CART_VIC20_8KB_A000:
        case IDM_CART_VIC20_4KB_B000:
            uicart_attach(canvas, idm, vic20_ui_cartridges);
            break;
        case IDM_CART_VIC20_GENERIC:
            uicart_attach_special(canvas, translate_text(IDS_SELECT_GENERIC), UILIB_FILTER_ALL, CARTRIDGE_VIC20_GENERIC);
            break;
        case IDM_CART_VIC20_UM:
            uicart_attach_special(canvas, translate_text(IDS_SELECT_UM), UILIB_FILTER_ALL, CARTRIDGE_VIC20_UM);
            break;
        case IDM_CART_VIC20_FP:
            uicart_attach_special(canvas, translate_text(IDS_SELECT_FP), UILIB_FILTER_ALL, CARTRIDGE_VIC20_FP);
            break;
        case IDM_CART_VIC20_BEHR_BONZ:
            uicart_attach_special(canvas, translate_text(IDS_SELECT_BEHR_BONZ), UILIB_FILTER_ALL, CARTRIDGE_VIC20_BEHRBONZ);
            break;
        case IDM_CART_VIC20_MEGACART:
            uicart_attach_special(canvas, translate_text(IDS_SELECT_MEGACART), UILIB_FILTER_ALL, CARTRIDGE_VIC20_MEGACART);
            break;
        case IDM_CART_VIC20_FINAL_EXPANSION:
            uicart_attach_special(canvas, translate_text(IDS_SELECT_FINAL_EXPANSION), UILIB_FILTER_ALL, CARTRIDGE_VIC20_FINAL_EXPANSION);
            break;
        case IDM_CART_VIC20_SMART_ATTACH:
            uicart_attach_special(canvas, translate_text(IDS_SELECT_CARTRIDGE_IMAGE), UILIB_FILTER_ALL, CARTRIDGE_VIC20_DETECT);
            break;
        case IDM_CART_SET_DEFAULT:
            cartridge_set_default();
            break;
        case IDM_CART_DETACH:
            cartridge_detach_image(-1);
            break;
        case IDM_VIC20_SETTINGS:
            ui_vic20_settings_dialog();
            break;
        case IDM_COMPUTER_ROM_SETTINGS:
            ui_vic20_computer_rom_settings_dialog(canvas);
            break;
        case IDM_DRIVE_ROM_SETTINGS:
            ui_vic20_drive_rom_settings_dialog(canvas);
            break;
        case IDM_DRIVE_SETTINGS:
            uidrivevic20_settings_dialog();
            break;
        case IDM_PRINTER_SETTINGS:
            ui_printer_settings_dialog(canvas, 0, 1);
            break;
        case IDM_USERPORT_RTC58321A_SETTINGS:
            ui_userport_rtc58321a_settings_dialog();
            break;
        case IDM_USERPORT_DS1307_RTC_SETTINGS:
            ui_userport_ds1307_rtc_settings_dialog();
            break;
        case IDM_RS232USER_SETTINGS:
            ui_rs232user_settings_dialog();
            break;
        case IDM_SIDCART_SETTINGS:
            ui_sidcart_settings_dialog("$9800", "$9C00", "VIC20", 0x9800, 0x9c00);
            break;
        case IDM_JOYPORT_SETTINGS:
            ui_joyport_settings_dialog(1, 0, 1, 1, 0);
            break;
#ifdef AMIGA_OS4
        case IDM_JOY_SETTINGS:
            ui_joystick_settings_vic20_dialog();
            break;
#else
        case IDM_JOY_DEVICE_SELECTION:
            ui_joystick_device_vic20_dialog();
            break;
        case IDM_JOY_FIRE_SELECTION:
            ui_joystick_fire_vic20_dialog();
            break;
#endif
        case IDM_MEGACART_WRITEBACK_FILE:
            fname = BrowseFile(translate_text(IDMS_MEGACART_WRITEBACK_FILE), "#?", canvas);
            if (fname != NULL) {
                resources_set_string("MegaCartNvRAMfilename", fname);
            }
            break;
        case IDM_ACIA_SETTINGS:
            ui_acia20_settings_dialog();
            break;
        case IDM_DIGIMAX_SETTINGS:
            ui_digimax_vic20_settings_dialog();
            break;
        case IDM_DS12C887RTC_SETTINGS:
            ui_ds12c887rtc_vic20_settings_dialog(canvas);
            break;
        case IDM_GEORAM_SETTINGS:
            ui_georam_vic20_settings_dialog(canvas);
            break;
        case IDM_SFX_SE_SETTINGS:
            ui_soundexpander_vic20_settings_dialog(canvas);
            break;
        case IDM_SFX_SS_SETTINGS:
            ui_soundsampler_settings_dialog(canvas);
            break;
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

int vic20ui_init(void)
{
    ui_register_menu_translation_layout(vic20_ui_translation_menu);
    ui_register_menu_layout(vic20_ui_menu);
    ui_register_machine_specific(vic20_ui_specific);
    ui_register_menu_toggles(vic20_ui_menu_toggles);

    return 0;
}

void vic20ui_shutdown(void)
{
}
