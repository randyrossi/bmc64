/*
 * plus4ui.c - Plus4-specific user interface.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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
#include "cartio.h"
#include "cartridge.h"
#include "constants.h"
#include "gfxoutput.h"
#include "joyport.h"
#include "plus4memhacks.h"
#include "plus4cart.h"
#include "plus4model.h"
#include "plus4ui.h"
#include "resources.h"
#include "ui.h"
#include "ui_drive.h"
#include "ui_joystick.h"
#include "ui_printer.h"
#include "ui_sidcart.h"
#include "ui_ted.h"
#include "ui_video.h"
#include "video.h"
}

static ui_drive_type_t plus4_drive_types[] = {
    { "1540", DRIVE_TYPE_1540 },
    { "1541", DRIVE_TYPE_1541 },
    { "1541-II", DRIVE_TYPE_1541II },
    { "1551", DRIVE_TYPE_1551 },
    { "1570", DRIVE_TYPE_1570 },
    { "1571", DRIVE_TYPE_1571 },
    { "1581", DRIVE_TYPE_1581 },
    { "2000", DRIVE_TYPE_2000 },
    { "4000", DRIVE_TYPE_4000 },
    { "None", DRIVE_TYPE_NONE },
    { NULL, 0 }
};

ui_menu_toggle  plus4_ui_menu_toggles[] = {
    { "TEDDoubleSize", MENU_TOGGLE_DOUBLESIZE },
    { "TEDDoubleScan", MENU_TOGGLE_DOUBLESCAN },
    { "TEDVideoCache", MENU_TOGGLE_VIDEOCACHE },
    { "Acia1Enable", MENU_TOGGLE_ACIA },
    { "SIDCartJoy", MENU_TOGGLE_SIDCART_JOY },
    { "SpeechEnabled", MENU_TOGGLE_V364SPEECH },
    { "CartridgeReset", MENU_CART_PLUS4_RESET_ON_CHANGE },
    { "Mouse", MENU_TOGGLE_MOUSE },
    { "SmartMouseRTCSave", MENU_TOGGLE_SMART_MOUSE_RTC_SAVE },
    { "UserportDAC", MENU_TOGGLE_USERPORT_DAC },
    { NULL, 0 }
};

ui_res_possible_values plus4_JoyPort1Device[JOYPORT_MAX_DEVICES + 1];
ui_res_possible_values plus4_JoyPort2Device[JOYPORT_MAX_DEVICES + 1];
ui_res_possible_values plus4_JoyPort3Device[JOYPORT_MAX_DEVICES + 1];
ui_res_possible_values plus4_JoyPort4Device[JOYPORT_MAX_DEVICES + 1];
ui_res_possible_values plus4_JoyPort5Device[JOYPORT_MAX_DEVICES + 1];

ui_res_possible_values plus4AciaDevice[] = {
    { 1, MENU_ACIA_RS323_DEVICE_1 },
    { 2, MENU_ACIA_RS323_DEVICE_2 },
    { 3, MENU_ACIA_RS323_DEVICE_3 },
    { 4, MENU_ACIA_RS323_DEVICE_4 },
    { -1, 0 }
};

ui_res_possible_values plus4_RenderFilters[] = {
    { VIDEO_FILTER_NONE, MENU_RENDER_FILTER_NONE },
    { VIDEO_FILTER_CRT, MENU_RENDER_FILTER_CRT_EMULATION },
    { VIDEO_FILTER_SCALE2X, MENU_RENDER_FILTER_SCALE2X },
    { -1, 0 }
};

ui_res_possible_values plus4MemoryHacks[] = {
    { MEMORY_HACK_NONE, MENU_PLUS4_MEMORY_HACK_NONE },
    { MEMORY_HACK_C256K, MENU_PLUS4_MEMORY_HACK_C256K },
    { MEMORY_HACK_H256K, MENU_PLUS4_MEMORY_HACK_H256K },
    { MEMORY_HACK_H1024K, MENU_PLUS4_MEMORY_HACK_H1024K },
    { MEMORY_HACK_H4096K, MENU_PLUS4_MEMORY_HACK_H4096K },
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

ui_res_possible_values DoodleTEDLum[] = {
    { NATIVE_SS_TED_LUM_IGNORE, MENU_SCREENSHOT_DOODLE_TED_LUM_IGNORE },
    { NATIVE_SS_TED_LUM_DITHER, MENU_SCREENSHOT_DOODLE_TED_LUM_DITHER },
    { -1, 0 }
};

ui_res_possible_values KoalaTEDLum[] = {
    { NATIVE_SS_TED_LUM_IGNORE, MENU_SCREENSHOT_KOALA_TED_LUM_IGNORE },
    { NATIVE_SS_TED_LUM_DITHER, MENU_SCREENSHOT_KOALA_TED_LUM_DITHER },
    { -1, 0 }
};

static ui_res_possible_values IOCollisions[] = {
    { IO_COLLISION_METHOD_DETACH_ALL, MENU_IO_COLLISION_DETACH_ALL },
    { IO_COLLISION_METHOD_DETACH_LAST, MENU_IO_COLLISION_DETACH_LAST },
    { IO_COLLISION_METHOD_AND_WIRES, MENU_IO_COLLISION_AND_WIRES },
    { -1, 0 }
};

ui_res_value_list plus4_ui_res_values[] = {
    { "Acia1Dev", plus4AciaDevice },
    { "TEDFilter", plus4_RenderFilters },
    { "JoyPort1Device", plus4_JoyPort1Device },
    { "JoyPort2Device", plus4_JoyPort2Device },
    { "JoyPort3Device", plus4_JoyPort3Device },
    { "JoyPort4Device", plus4_JoyPort4Device },
    { "JoyPort5Device", plus4_JoyPort5Device },
    { "MemoryHack", plus4MemoryHacks },
    { "DoodleMultiColorHandling", DoodleMultiColor },
    { "DoodleTEDLumHandling", DoodleTEDLum },
    { "KoalaTEDLumHandling", DoodleTEDLum },
    { "IOCollisionHandling", IOCollisions },
    { NULL, NULL }
};

static const char *plus4_sidcart_address_text_pair[] = { "$FD40", "$FE80" };
static const char *plus4_sidcart_clock_pair[] = { "C64", "PLUS4" };
static const int plus4_sidcart_address_int_pair[] = { 0xfd40, 0xfe80 };

static ui_cartridge_t plus4_ui_cartridges[]={
    { MENU_CART_PLUS4_SMART, CARTRIDGE_PLUS4_DETECT, "Smart attach" },
    { MENU_CART_PLUS4_C0_LOW, CARTRIDGE_PLUS4_16KB_C0LO, "C0 low image" },
    { MENU_CART_PLUS4_C0_HIGH, CARTRIDGE_PLUS4_16KB_C0HI, "C0 high image" },
    { MENU_CART_PLUS4_C1_LOW, CARTRIDGE_PLUS4_16KB_C1LO, "C1 low image" },
    { MENU_CART_PLUS4_C1_HIGH, CARTRIDGE_PLUS4_16KB_C1HI, "C1 high image" },
    { MENU_CART_PLUS4_C2_LOW, CARTRIDGE_PLUS4_16KB_C2LO, "C2 low image" },
    { MENU_CART_PLUS4_C2_HIGH, CARTRIDGE_PLUS4_16KB_C2HI, "C2 high image" },
    { 0, 0, NULL }
};

static void plus4_ui_attach_cartridge(int menu)
{
    int i = 0;

    while (menu != plus4_ui_cartridges[i].menu_item && plus4_ui_cartridges[i].menu_item) {
        i++;
    }

    if (!plus4_ui_cartridges[i].menu_item) {
        ui_error("Bad cartridge config in UI");
        return;
    }

    ui_select_file(B_OPEN_PANEL, PLUS4_CARTRIDGE_FILE, &plus4_ui_cartridges[i]);
}       

static void plus4_ui_specific(void *msg, void *window)
{
    switch (((BMessage*)msg)->what) {
        case MENU_CART_PLUS4_SMART:      
        case MENU_CART_PLUS4_C0_LOW:
        case MENU_CART_PLUS4_C0_HIGH:
        case MENU_CART_PLUS4_C1_LOW:
        case MENU_CART_PLUS4_C1_HIGH:
        case MENU_CART_PLUS4_C2_LOW:
        case MENU_CART_PLUS4_C2_HIGH:
            plus4_ui_attach_cartridge(((BMessage*)msg)->what);
            break;
        case MENU_CART_PLUS4_DETACH:
            cartridge_detach_image(-1);
            break;
        case MENU_VIDEO_SETTINGS:
            ui_video(UI_VIDEO_CHIP_TED);
            break;
        case MENU_TED_SETTINGS:
            ui_ted();
            break;
        case MENU_JOYSTICK_SETTINGS:
            ui_joystick(1, 2);
            break;
        case MENU_USERPORT_JOY_SETTINGS:
            ui_joystick(3, 4);
            break;
        case MENU_SIDCART_JOY_SETTINGS:
            ui_joystick(5, 0);
            break;
        case MENU_DRIVE_SETTINGS:
            ui_drive(plus4_drive_types, HAS_PARA_CABLE);
            break;
        case MENU_PRINTER_SETTINGS:
            ui_printer(HAS_IEC_BUS);
            break;
        case MENU_SIDCART_SETTINGS:
            ui_sidcart(plus4_sidcart_address_text_pair, plus4_sidcart_clock_pair, plus4_sidcart_address_int_pair);
            ui_update_menus();
            break;
        case MENU_V364SPEECH_FILE:
            ui_select_file(B_SAVE_PANEL, V364SPEECH_FILE, (void*)0);
            break;
        case MENU_PLUS4_MODEL_C16_PAL:
            plus4model_set(PLUS4MODEL_C16_PAL);
            break;
        case MENU_PLUS4_MODEL_C16_NTSC:
            plus4model_set(PLUS4MODEL_C16_NTSC);
            break;
        case MENU_PLUS4_MODEL_PLUS4_PAL:
            plus4model_set(PLUS4MODEL_PLUS4_PAL);
            break;
        case MENU_PLUS4_MODEL_PLUS4_NTSC:
            plus4model_set(PLUS4MODEL_PLUS4_NTSC);
            break;
        case MENU_PLUS4_MODEL_V364_NTSC:
            plus4model_set(PLUS4MODEL_V364_NTSC);
            break;
        case MENU_PLUS4_MODEL_C232_NTSC:
            plus4model_set(PLUS4MODEL_232_NTSC);
            break;
        case MENU_COMPUTER_KERNAL_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, COMPUTER_KERNAL_ROM_FILE, (void*)0);
            break;
        case MENU_COMPUTER_BASIC_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, COMPUTER_BASIC_ROM_FILE, (void*)0);
            break;
        case MENU_COMPUTER_FUNCTION_LO_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, COMPUTER_FUNCTION_LO_ROM_FILE, (void*)0);
            break;
        case MENU_COMPUTER_FUNCTION_HI_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, COMPUTER_FUNCTION_HI_ROM_FILE, (void*)0);
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
        case MENU_DRIVE_1551_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, DRIVE_1551_ROM_FILE, (void*)0);
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

int plus4ui_init_early(void)
{
    vicemenu_set_joyport_func(joyport_get_valid_devices, joyport_get_port_name, 1, 1, 1, 1, 1);
    return 0;
}

static void build_joyport_values(void)
{
    int i;

    for (i = 0; i < JOYPORT_MAX_DEVICES; ++i) {
        plus4_JoyPort1Device[i].value = i;
        plus4_JoyPort1Device[i].item_id = MENU_JOYPORT1 + i;
        plus4_JoyPort2Device[i].value = i;
        plus4_JoyPort2Device[i].item_id = MENU_JOYPORT2 + i;
        plus4_JoyPort3Device[i].value = i;
        plus4_JoyPort3Device[i].item_id = MENU_JOYPORT3 + i;
        plus4_JoyPort4Device[i].value = i;
        plus4_JoyPort4Device[i].item_id = MENU_JOYPORT4 + i;
        plus4_JoyPort5Device[i].value = i;
        plus4_JoyPort5Device[i].item_id = MENU_JOYPORT5 + i;
    }
    plus4_JoyPort1Device[i].value = -1;
    plus4_JoyPort1Device[i].item_id = 0;
    plus4_JoyPort2Device[i].value = -1;
    plus4_JoyPort2Device[i].item_id = 0;
    plus4_JoyPort3Device[i].value = -1;
    plus4_JoyPort3Device[i].item_id = 0;
    plus4_JoyPort4Device[i].value = -1;
    plus4_JoyPort4Device[i].item_id = 0;
    plus4_JoyPort5Device[i].value = -1;
    plus4_JoyPort5Device[i].item_id = 0;
}

int plus4ui_init(void)
{
    build_joyport_values();
    ui_register_machine_specific(plus4_ui_specific);
    ui_register_menu_toggles(plus4_ui_menu_toggles);
    ui_register_res_values(plus4_ui_res_values);
    ui_update_menus();
    return 0;
}

void plus4ui_shutdown(void)
{
}
