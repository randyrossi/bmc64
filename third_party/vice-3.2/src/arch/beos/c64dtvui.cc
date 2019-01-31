/*
 * c64dtvui.cc - C64DTV-specific user interface.
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

#include <FilePanel.h>
#include <Menu.h>
#include <MenuItem.h>
#include <Window.h>
#include <stdio.h>
#include <stdlib.h>

#if defined(__BEOS__) && defined(WORDS_BIGENDIAN)
#include <string.h>
#endif

#include "ui_file.h"
#include "vicemenu.h"

extern "C" {
#include "archdep.h"
#include "c64ui.h"
#include "c64dtv-resources.h"
#include "c64dtvmodel.h"
#include "cartridge.h"
#include "constants.h"
#include "gfxoutput.h"
#include "joyport.h"
#include "keyboard.h"
#include "resources.h"
#include "types.h"
#include "ui.h"
#include "ui_drive.h"
#include "ui_joystick.h"
#include "ui_printer.h"
#include "ui_sid.h"
#include "ui_vicii.h"
#include "ui_video.h"
#include "util.h"
#include "video.h"
}

static ui_drive_type_t c64dtv_drive_types[] = {
    { "1540", DRIVE_TYPE_1540 },
    { "1541", DRIVE_TYPE_1541 },
    { "1541-II", DRIVE_TYPE_1541II },
    { "1570", DRIVE_TYPE_1570 },
    { "1571", DRIVE_TYPE_1571 },
    { "1581", DRIVE_TYPE_1581 },
    { "2000", DRIVE_TYPE_2000 },
    { "4000", DRIVE_TYPE_4000 },
    { "None", DRIVE_TYPE_NONE },
    { NULL, 0 }
};

ui_menu_toggle  c64dtv_ui_menu_toggles[] = {
    { "VICIIDoubleSize", MENU_TOGGLE_DOUBLESIZE },
    { "VICIIDoubleScan", MENU_TOGGLE_DOUBLESCAN },
    { "VICIIVideoCache", MENU_TOGGLE_VIDEOCACHE },
    { "c64dtvromrw", MENU_TOGGLE_C64DTV_WRITE_ENABLE },
    { "Mouse", MENU_TOGGLE_MOUSE },
    { "ps2mouse", MENU_TOGGLE_PS2MOUSE },
    { "HummerADC", MENU_HUMMER_USERPORT_ADC },
    { NULL, 0 }
};

ui_res_possible_values c64dtv_JoyPort1Device[JOYPORT_MAX_DEVICES + 1];
ui_res_possible_values c64dtv_JoyPort2Device[JOYPORT_MAX_DEVICES + 1];
ui_res_possible_values c64dtv_JoyPort3Device[JOYPORT_MAX_DEVICES + 1];

ui_res_possible_values C64DTVRenderFilters[] = {
    { VIDEO_FILTER_NONE, MENU_RENDER_FILTER_NONE },
    { VIDEO_FILTER_CRT, MENU_RENDER_FILTER_CRT_EMULATION },
    { VIDEO_FILTER_SCALE2X, MENU_RENDER_FILTER_SCALE2X },
    { -1, 0 }
};

ui_res_possible_values C64DTVRevision[] = {
    { 2, MENU_C64DTV_REVISION_2 },
    { 3, MENU_C64DTV_REVISION_3 },
    { -1, 0 }
};

static ui_res_possible_values DoodleMultiColor[] = {
    { NATIVE_SS_MC2HR_BLACK_WHITE, MENU_SCREENSHOT_DOODLE_MULTICOLOR_BLACK_WHITE },
    { NATIVE_SS_MC2HR_2_COLORS, MENU_SCREENSHOT_DOODLE_MULTICOLOR_2_COLORS },
    { NATIVE_SS_MC2HR_4_COLORS, MENU_SCREENSHOT_DOODLE_MULTICOLOR_4_COLORS },
    { NATIVE_SS_MC2HR_GRAY, MENU_SCREENSHOT_DOODLE_MULTICOLOR_GRAY_SCALE },
    { NATIVE_SS_MC2HR_DITHER, MENU_SCREENSHOT_DOODLE_MULTICOLOR_DITHER },
    { -1, 0 }
};

ui_res_value_list c64dtv_ui_res_values[] = {
    { "VICIIFilter", C64DTVRenderFilters },
    { "DtvRevision", C64DTVRevision },
    { "JoyPort1Device", c64dtv_JoyPort1Device },
    { "JoyPort2Device", c64dtv_JoyPort2Device },
    { "JoyPort3Device", c64dtv_JoyPort3Device },
    { "DoodleMultiColorHandling", DoodleMultiColor },
    { NULL, NULL }
};

void c64dtv_ui_specific(void *msg, void *window)
{
    switch (((BMessage*)msg)->what) {
        case MENU_C64DTV_MODEL_V2_PAL:
            dtvmodel_set(DTVMODEL_V2_PAL);
            break;
        case MENU_C64DTV_MODEL_V2_NTSC:
            dtvmodel_set(DTVMODEL_V2_NTSC);
            break;
        case MENU_C64DTV_MODEL_V3_PAL:
            dtvmodel_set(DTVMODEL_V3_PAL);
            break;
        case MENU_C64DTV_MODEL_V3_NTSC:
            dtvmodel_set(DTVMODEL_V3_NTSC);
            break;
        case MENU_C64DTV_MODEL_HUMMER_NTSC:
            dtvmodel_set(DTVMODEL_HUMMER_NTSC);
            break;
        case MENU_VIDEO_SETTINGS:
            ui_video(UI_VIDEO_CHIP_VICII);
            break;
        case MENU_VICII_SETTINGS:
            ui_vicii();
            break;
        case MENU_SID_SETTINGS:
            ui_sid(NULL);
            break;
        case MENU_JOYSTICK_SETTINGS:
            ui_joystick(1, 2);
            break;
        case MENU_USERPORT_JOY_SETTINGS:
            ui_joystick(3, 0);
            break;
        case MENU_DRIVE_SETTINGS:
            ui_drive(c64dtv_drive_types, HAS_NO_CAPS);
            break;
        case MENU_PRINTER_SETTINGS:
            ui_printer(HAS_IEC_BUS);
            break;
        case MENU_C64DTV_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, C64DTV_ROM_FILE, (void*)0);
            break;
        case MENU_COMPUTER_KERNAL_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, COMPUTER_KERNAL_ROM_FILE, (void*)0);
            break;
        case MENU_COMPUTER_BASIC_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, COMPUTER_BASIC_ROM_FILE, (void*)0);
            break;
        case MENU_COMPUTER_CHARGEN_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, COMPUTER_CHARGEN_ROM_FILE, (void*)0);
            break;
        case MENU_DRIVE_1540_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, DRIVE_1540_ROM_FILE, (void*)0);
            break;
        case MENU_DRIVE_1541_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, DRIVE_1541_ROM_FILE, (void*)0);
            break;
        case MENU_DRIVE_1541II_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, DRIVE_1541II_ROM_FILE, (void*)0);
            break;
        case MENU_DRIVE_1570_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, DRIVE_1570_ROM_FILE, (void*)0);
            break;
        case MENU_DRIVE_1571_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, DRIVE_1571_ROM_FILE, (void*)0);
            break;
        case MENU_DRIVE_1581_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, DRIVE_1581_ROM_FILE, (void*)0);
            break;
        case MENU_DRIVE_2000_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, DRIVE_2000_ROM_FILE, (void*)0);
            break;
        case MENU_DRIVE_4000_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, DRIVE_4000_ROM_FILE, (void*)0);
            break;
        default:
            break;
    }
}

int c64dtvui_init_early(void) {
    vicemenu_set_joyport_func(joyport_get_valid_devices, joyport_get_port_name, 1, 1, 1, 0, 0);
    return 0;
}

static void build_joyport_values(void)
{
    int i;

    for (i = 0; i < JOYPORT_MAX_DEVICES; ++i) {
        c64dtv_JoyPort1Device[i].value = i;
        c64dtv_JoyPort1Device[i].item_id = MENU_JOYPORT1 + i;
        c64dtv_JoyPort2Device[i].value = i;
        c64dtv_JoyPort2Device[i].item_id = MENU_JOYPORT2 + i;
        c64dtv_JoyPort3Device[i].value = i;
        c64dtv_JoyPort3Device[i].item_id = MENU_JOYPORT3 + i;
    }
    c64dtv_JoyPort1Device[i].value = -1;
    c64dtv_JoyPort1Device[i].item_id = 0;
    c64dtv_JoyPort2Device[i].value = -1;
    c64dtv_JoyPort2Device[i].item_id = 0;
    c64dtv_JoyPort3Device[i].value = -1;
    c64dtv_JoyPort3Device[i].item_id = 0;
}

int c64dtvui_init(void)
{
    build_joyport_values();
    ui_register_machine_specific(c64dtv_ui_specific);
    ui_register_menu_toggles(c64dtv_ui_menu_toggles);
    ui_register_res_values(c64dtv_ui_res_values);
    ui_update_menus();
    return 0;
}

void c64dtvui_shutdown(void)
{
}
