/*
 * petui.c
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

#define UI_PET
#define UI_MENU_NAME pet_ui_translation_menu
#define UI_TRANSLATED_MENU_NAME pet_ui_menu

#include "private.h"
#include "petmodel.h"
#include "petui.h"
#include "petuires.h"
#include "translate.h"

#include "mui/uiacia.h"
#include "mui/uicpclockf83.h"
#include "mui/uidatasette.h"
#include "mui/uidrivepetcbm2.h"
#include "mui/uiiocollisions.h"
#include "mui/uijoyport.h"
#include "mui/uijoystick.h"
#include "mui/uijoystickll.h"
#include "mui/uikeymap.h"
#include "mui/uimouse.h"
#include "mui/uipetcolour.h"
#include "mui/uipetdww.h"
#include "mui/uipetreu.h"
#include "mui/uipetsettings.h"
#include "mui/uiprinter.h"
#include "mui/uirompetsettings.h"
#include "mui/uisampler.h"
#include "mui/uisidcart.h"
#include "mui/uitapelog.h"
#include "mui/uiuserportds1307rtc.h"
#include "mui/uiuserportrtc58321a.h"
#include "mui/uivideo.h"

static const ui_menu_toggle_t pet_ui_menu_toggles[] = {
    { "CrtcDoubleSize", IDM_TOGGLE_DOUBLESIZE },
    { "CrtcDoubleScan", IDM_TOGGLE_DOUBLESCAN },
    { "CrtcVideoCache", IDM_TOGGLE_VIDEOCACHE },
    { "CrtcAudioLeak", IDM_TOGGLE_AUDIO_LEAK },
    { "CrtcStretchVertical", IDM_TOGGLE_VERTICAL_STRETCH },
    { "PETHRE", IDM_ENABLE_PETHRE },
    { "UserportDAC", IDM_TOGGLE_USERPORT_DAC },
    { "Mouse", IDM_MOUSE },
    { "TapeSenseDongle", IDM_TOGGLE_TAPE_SENSE_DONGLE },
    { "DTLBasicDongle", IDM_TOGGLE_DTL_BASIC_DONGLE },
    { NULL, 0 }
};

static int pet_ui_specific(video_canvas_t *canvas, int idm)
{
    switch (idm) {
        case IDM_PET_MODEL_2001:
            petmodel_set(PETMODEL_2001);
            break;
        case IDM_PET_MODEL_3008:
            petmodel_set(PETMODEL_3008);
            break;
        case IDM_PET_MODEL_3016:
            petmodel_set(PETMODEL_3016);
            break;
        case IDM_PET_MODEL_3032:
            petmodel_set(PETMODEL_3032);
            break;
        case IDM_PET_MODEL_3032B:
            petmodel_set(PETMODEL_3032B);
            break;
        case IDM_PET_MODEL_4016:
            petmodel_set(PETMODEL_4016);
            break;
        case IDM_PET_MODEL_4032:
            petmodel_set(PETMODEL_4032);
            break;
        case IDM_PET_MODEL_4032B:
            petmodel_set(PETMODEL_4032B);
            break;
        case IDM_PET_MODEL_8032:
            petmodel_set(PETMODEL_8032);
            break;
        case IDM_PET_MODEL_8096:
            petmodel_set(PETMODEL_8096);
            break;
        case IDM_PET_MODEL_8296:
            petmodel_set(PETMODEL_8296);
            break;
        case IDM_PET_MODEL_SUPERPET:
            petmodel_set(PETMODEL_SUPERPET);
            break;
        case IDM_PALETTE_SETTINGS:
            ui_video_palette_settings_dialog(canvas, "CrtcExternalPalette", "CrtcPaletteFile", translate_text(IDS_CRTC_PALETTE_FILENAME));
            break;
        case IDM_COLOR_SETTINGS:
            ui_video_color_settings_dialog(canvas, "CrtcColorGamma", "CrtcColorTint", "CrtcColorSaturation", "CrtcColorContrast", "CrtcColorBrightness");
            break;
        case IDM_RENDER_FILTER:
            ui_video_render_filter_settings_dialog(canvas, "CrtcFilter");
            break;
        case IDM_CRT_EMULATION_SETTINGS:
            ui_video_crt_settings_dialog(canvas, "CrtcPALScanLineShade", "CrtcPALBlur", "CrtcPALOddLinePhase", "CrtcPALOddLineOffset");
            break;
        case IDM_PETREU_SETTINGS:
            ui_petreu_settings_dialog(canvas);
            break;
        case IDM_PET_SETTINGS:
            ui_pet_settings_dialog();
            break;
        case IDM_COMPUTER_ROM_SETTINGS:
            ui_pet_computer_rom_settings_dialog(canvas);
            break;
        case IDM_DRIVE_ROM_SETTINGS:
            ui_pet_drive_rom_settings_dialog(canvas);
            break;
        case IDM_DRIVE_SETTINGS:
            uidrivepetcbm2_settings_dialog();
            break;
        case IDM_PRINTER_SETTINGS:
            ui_printer_settings_dialog(canvas, 1, 1);
            break;
        case IDM_USERPORT_RTC58321A_SETTINGS:
            ui_userport_rtc58321a_settings_dialog();
            break;
        case IDM_USERPORT_DS1307_RTC_SETTINGS:
            ui_userport_ds1307_rtc_settings_dialog();
            break;
        case IDM_ACIA_SETTINGS:
            ui_acia_settings_dialog();
            break;
        case IDM_SIDCART_SETTINGS:
            ui_sidcart_settings_dialog("$8F00", "$E900", "PET", 0x8f00, 0xe900);
            break;
        case IDM_PETDWW_SETTINGS:
            ui_petdww_settings_dialog(canvas);
            break;
        case IDM_PETCOLOUR_SETTINGS:
            ui_petcolour_settings_dialog();
            break;
        case IDM_JOYPORT_SETTINGS:
            ui_joyport_settings_dialog(0, 0, 1, 1, 0);
            break;
#ifdef AMIGA_OS4
        case IDM_JOY_SETTINGS:
            ui_joystick_settings_pet_dialog();
            break;
#else
        case IDM_JOY_DEVICE_SELECTION:
            ui_joystick_device_pet_dialog();
            break;
        case IDM_JOY_FIRE_SELECTION:
            ui_joystick_fire_pet_dialog();
            break;
#endif
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

int petui_init(void)
{
    ui_register_menu_translation_layout(pet_ui_translation_menu);
    ui_register_menu_layout(pet_ui_menu);
    ui_register_machine_specific(pet_ui_specific);
    ui_register_menu_toggles(pet_ui_menu_toggles);

    return 0;
}

void petui_shutdown(void)
{
}
