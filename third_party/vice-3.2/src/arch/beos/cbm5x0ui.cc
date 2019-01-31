/*
 * cbm5x0ui.cc - CBM5x0-specific user interface.
 *
 * Written by
 *  Marcus Sutton <loggedoubt@gmail.com>
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

#include <Message.h>
#include <Window.h>
#include <stdio.h>

#include "ui_file.h"
#include "vicemenu.h"

extern "C" {
#include "cartio.h"
#include "cartridge.h"
#include "cbm2model.h"
#include "cbm2ui.h"
#include "constants.h"
#include "gfxoutput.h"
#include "joyport.h"
#include "ui.h"
#include "ui_cbm5x0.h"
#include "ui_cia.h"
#include "ui_drive.h"
#include "ui_joystick.h"
#include "ui_printer.h"
#include "ui_sid.h"
#include "ui_vicii.h"
#include "ui_video.h"
#include "video.h"
}

static ui_drive_type_t cbm5x0_drive_types[] = {
    { "2031", DRIVE_TYPE_2031 },
    { "2040", DRIVE_TYPE_2040 },
    { "3040", DRIVE_TYPE_3040 },
    { "4040", DRIVE_TYPE_4040 },
    { "1001", DRIVE_TYPE_1001 },
    { "8050", DRIVE_TYPE_8050 },
    { "8250", DRIVE_TYPE_8250 },
    { "None", DRIVE_TYPE_NONE },
    { NULL, 0 }
};

ui_menu_toggle  cbm5x0_ui_menu_toggles[] = {
    { "VICIIDoubleSize", MENU_TOGGLE_DOUBLESIZE },
    { "VICIIDoubleScan", MENU_TOGGLE_DOUBLESCAN },
    { "VICIIVideoCache", MENU_TOGGLE_VIDEOCACHE },
    { "CartridgeReset", MENU_CART_CBM2_RESET_ON_CHANGE },
    { "Mouse", MENU_TOGGLE_MOUSE },
    { "SmartMouseRTCSave", MENU_TOGGLE_SMART_MOUSE_RTC_SAVE },
    { NULL, 0 }
};

ui_res_possible_values cbm5x0AciaDevice[] = {
    { 1, MENU_ACIA_RS323_DEVICE_1 },
    { 2, MENU_ACIA_RS323_DEVICE_2 },
    { 3, MENU_ACIA_RS323_DEVICE_3 },
    { 4, MENU_ACIA_RS323_DEVICE_4 },
    { -1, 0 }
};

ui_res_possible_values cbm5x0_JoyPort1Device[JOYPORT_MAX_DEVICES + 1];
ui_res_possible_values cbm5x0_JoyPort2Device[JOYPORT_MAX_DEVICES + 1];

ui_res_possible_values cbm5x0RenderFilters[] = {
    { VIDEO_FILTER_NONE, MENU_RENDER_FILTER_NONE },
    { VIDEO_FILTER_CRT, MENU_RENDER_FILTER_CRT_EMULATION },
    { VIDEO_FILTER_SCALE2X, MENU_RENDER_FILTER_SCALE2X },
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

static ui_res_possible_values IOCollisions[] = {
    { IO_COLLISION_METHOD_DETACH_ALL, MENU_IO_COLLISION_DETACH_ALL },
    { IO_COLLISION_METHOD_DETACH_LAST, MENU_IO_COLLISION_DETACH_LAST },
    { IO_COLLISION_METHOD_AND_WIRES, MENU_IO_COLLISION_AND_WIRES },
    { -1, 0 }
};

ui_res_value_list cbm5x0_ui_res_values[] = {
    { "Acia1Dev", cbm5x0AciaDevice },
    { "VICIIFilter", cbm5x0RenderFilters },
    { "JoyPort1Device", cbm5x0_JoyPort1Device },
    { "JoyPort2Device", cbm5x0_JoyPort2Device },
    { "DoodleMultiColorHandling", DoodleMultiColor },
    { "IOCollisionHandling", IOCollisions },
    { NULL, NULL }
};

static ui_cartridge_t cbm2_ui_cartridges[]={
    { MENU_CART_CBM2_LOAD_1000, CARTRIDGE_CBM2_8KB_1000, "Load new Cart $1000" },
    { MENU_CART_CBM2_LOAD_2000, CARTRIDGE_CBM2_8KB_2000, "Load new Cart $2000-$3000" },
    { MENU_CART_CBM2_LOAD_4000, CARTRIDGE_CBM2_16KB_4000, "Load new Cart $4000-$5000" },
    { MENU_CART_CBM2_LOAD_6000, CARTRIDGE_CBM2_16KB_6000, "Load new Cart $6000-$7000" },
    { 0, 0, NULL }
};

static void cbm2_ui_attach_cartridge(int menu)
{
    int i = 0;

    while (menu != cbm2_ui_cartridges[i].menu_item && cbm2_ui_cartridges[i].menu_item) {
        i++;
    }

    if (!cbm2_ui_cartridges[i].menu_item) {
        ui_error("Bad cartridge config in UI");
        return;
    }

    ui_select_file(B_OPEN_PANEL, CBM2_CARTRIDGE_FILE, &cbm2_ui_cartridges[i]);
}

void cbm5x0_ui_specific(void *msg, void *window)
{
    switch (((BMessage*)msg)->what) {
        case MENU_CART_CBM2_LOAD_1000:
        case MENU_CART_CBM2_LOAD_2000:
        case MENU_CART_CBM2_LOAD_4000:
        case MENU_CART_CBM2_LOAD_6000:
            cbm2_ui_attach_cartridge(((BMessage*)msg)->what);
            break;
        case MENU_CART_CBM2_UNLOAD_1000:
            cartridge_detach_image(CARTRIDGE_CBM2_8KB_1000);
            break;
        case MENU_CART_CBM2_UNLOAD_2000:
            cartridge_detach_image(CARTRIDGE_CBM2_8KB_2000);
            break;
        case MENU_CART_CBM2_UNLOAD_4000:
            cartridge_detach_image(CARTRIDGE_CBM2_16KB_4000);
            break;
        case MENU_CART_CBM2_UNLOAD_6000:
            cartridge_detach_image(CARTRIDGE_CBM2_16KB_6000);
            break;
        case MENU_CBM5X0_MODEL_510_PAL:
            cbm2model_set(CBM2MODEL_510_PAL);
            break;
        case MENU_CBM5X0_MODEL_510_NTSC:
            cbm2model_set(CBM2MODEL_510_NTSC);
            break;
        case MENU_CBM5X0_SETTINGS:
            ui_cbm5x0();
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
        case MENU_CIA_SETTINGS:
            ui_cia(1);
            break;
        case MENU_JOYSTICK_SETTINGS:
            ui_joystick(1, 2);
            break;
        case MENU_DRIVE_SETTINGS:
            ui_drive(cbm5x0_drive_types, HAS_NO_CAPS);
            break;
        case MENU_PRINTER_SETTINGS:
            ui_printer(HAS_USERPORT_PRINTER);
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
        case MENU_DRIVE_2031_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, DRIVE_2031_ROM_FILE, (void*)0);
            break;
        case MENU_DRIVE_2040_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, DRIVE_2040_ROM_FILE, (void*)0);
            break;
        case MENU_DRIVE_3040_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, DRIVE_3040_ROM_FILE, (void*)0);
            break;
        case MENU_DRIVE_4040_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, DRIVE_4040_ROM_FILE, (void*)0);
            break;
        case MENU_DRIVE_1001_ROM_FILE:
            ui_select_file(B_SAVE_PANEL, DRIVE_1001_ROM_FILE, (void*)0);
            break;
        default: ;
    }
}

int cbm5x0ui_init_early(void)
{
    vicemenu_set_joyport_func(joyport_get_valid_devices, joyport_get_port_name, 1, 1, 0, 0, 0);
    return 0;
}

static void build_joyport_values(void)
{
    int i;

    for (i = 0; i < JOYPORT_MAX_DEVICES; ++i) {
        cbm5x0_JoyPort1Device[i].value = i;
        cbm5x0_JoyPort1Device[i].item_id = MENU_JOYPORT1 + i;
        cbm5x0_JoyPort2Device[i].value = i;
        cbm5x0_JoyPort2Device[i].item_id = MENU_JOYPORT2 + i;
    }
    cbm5x0_JoyPort1Device[i].value = -1;
    cbm5x0_JoyPort1Device[i].item_id = 0;
    cbm5x0_JoyPort2Device[i].value = -1;
    cbm5x0_JoyPort2Device[i].item_id = 0;
}

int cbm5x0ui_init(void)
{
    build_joyport_values();
    ui_register_machine_specific(cbm5x0_ui_specific);
    ui_register_menu_toggles(cbm5x0_ui_menu_toggles);
    ui_register_res_values(cbm5x0_ui_res_values);
    ui_update_menus();
    return 0;
}

void cbm5x0ui_shutdown(void)
{
}
