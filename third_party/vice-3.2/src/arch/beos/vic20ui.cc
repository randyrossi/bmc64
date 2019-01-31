/*
 * vic20ui.cc - VIC20-specific user interface.
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
#include "cartio.h"
#include "cartridge.h"
#include "constants.h"
#include "gfxoutput.h"
#include "joyport.h"
#include "resources.h"
#include "types.h"
#include "ui.h"
#include "ui_drive.h"
#include "ui_joystick.h"
#include "ui_printer.h"
#include "ui_sidcart.h"
#include "ui_vic20.h"
#include "ui_vic.h"
#include "ui_video.h"
#include "util.h"
#include "vic20model.h"
#include "vic20ui.h"
#include "video.h"
}

static ui_drive_type_t vic20_drive_types[] = {
    { "1540", DRIVE_TYPE_1540 },
    { "1541", DRIVE_TYPE_1541 },
    { "1541-II", DRIVE_TYPE_1541II },
    { "1570", DRIVE_TYPE_1570 },
    { "1571", DRIVE_TYPE_1571 },
    { "1581", DRIVE_TYPE_1581 },
    { "2000", DRIVE_TYPE_2000 },
    { "4000", DRIVE_TYPE_4000 },
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

ui_menu_toggle  vic20_ui_menu_toggles[] = {
    { "VICDoubleSize", MENU_TOGGLE_DOUBLESIZE },
    { "VICDoubleScan", MENU_TOGGLE_DOUBLESCAN },
    { "VICVideoCache", MENU_TOGGLE_VIDEOCACHE },
#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    { "Acia1Enable", MENU_TOGGLE_ACIA },
#endif
    { "GEORAM", MENU_TOGGLE_GEORAM },
    { "GEORAMImageWrite", MENU_TOGGLE_GEORAM_SWC },
    { "GEORAMIOSwap", MENU_TOGGLE_GEORAM_IO_SWAP },
    { "DIGIMAX", MENU_TOGGLE_DIGIMAX },
    { "SFXSoundExpander", MENU_TOGGLE_SFX_SE },
    { "SFXSoundExpanderIOSwap", MENU_TOGGLE_SFX_SE_IO_SWAP },
    { "SFXSoundSampler", MENU_TOGGLE_SFX_SS },
    { "SFXSoundSamplerIOSwap", MENU_TOGGLE_SFX_SS_IO_SWAP },
    { "IEEE488", MENU_TOGGLE_IEEE488 },
    { "Mouse", MENU_TOGGLE_MOUSE },
    { "FinalExpansionWriteBack", MENU_TOGGLE_FE_WRITE_BACK },
    { "MegaCartNvRAMWriteBack", MENU_TOGGLE_MC_NVRAM_WRITE_BACK },
    { "VicFlashPluginWriteBack", MENU_TOGGLE_VFP_WRITE_BACK },
    { "UltiMemWriteBack", MENU_TOGGLE_UM_WRITE_BACK },
    { "DS12C887RTC", MENU_TOGGLE_DS12C887_RTC },
    { "DS12C887RTCRunMode", MENU_TOGGLE_DS12C887_RTC_RUNNING_MODE },
    { "DS12C887RTCSave", MENU_TOGGLE_DS12C887_RTC_SAVE },
    { "IO2RAM", MENU_TOGGLE_IO2_RAM },
    { "IO3RAM", MENU_TOGGLE_IO3_RAM },
    { "VFLImod", MENU_TOGGLE_VFLI },
    { "UserportDAC", MENU_TOGGLE_USERPORT_DAC },
    { "UserportRTC58321a", MENU_TOGGLE_USERPORT_58321A },
    { "UserportRTC58321aSave", MENU_TOGGLE_USERPORT_58321A_SAVE },
    { "UserportRTCDS1307", MENU_TOGGLE_USERPORT_DS1307 },
    { "UserportRTCDS1307Save", MENU_TOGGLE_USERPORT_DS1307_SAVE },
    { NULL, 0 }
};

#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
ui_res_possible_values vic20AciaDevice[] = {
    { 1, MENU_ACIA_RS323_DEVICE_1 },
    { 2, MENU_ACIA_RS323_DEVICE_2 },
    { 3, MENU_ACIA_RS323_DEVICE_3 },
    { 4, MENU_ACIA_RS323_DEVICE_4 },
    { -1, 0 }
};

ui_res_possible_values vic20AciaBase[] = {
    { 0x9800, MENU_ACIA_BASE_9800 },
    { 0x9c00, MENU_ACIA_BASE_9C00 },
    { -1, 0 }
};

ui_res_possible_values vic20AciaInt[] = {
    { 0, MENU_ACIA_INT_NONE },
    { 1, MENU_ACIA_INT_IRQ },
    { 2, MENU_ACIA_INT_NMI },
    { -1, 0 }
};

ui_res_possible_values vic20AciaMode[] = {
    { 0, MENU_ACIA_MODE_NORMAL },
    { 1, MENU_ACIA_MODE_SWIFTLINK },
    { 2, MENU_ACIA_MODE_TURBO232 },
    { -1, 0 }
};
#endif

ui_res_possible_values vic20_JoyPort1Device[JOYPORT_MAX_DEVICES + 1];
ui_res_possible_values vic20_JoyPort3Device[JOYPORT_MAX_DEVICES + 1];
ui_res_possible_values vic20_JoyPort4Device[JOYPORT_MAX_DEVICES + 1];

ui_res_possible_values vic20GeoRAMSize[] = {
    { 64, MENU_GEORAM_SIZE_64 },
    { 128, MENU_GEORAM_SIZE_128 },
    { 256, MENU_GEORAM_SIZE_256 },
    { 512, MENU_GEORAM_SIZE_512 },
    { 1024, MENU_GEORAM_SIZE_1024 },
    { 2048, MENU_GEORAM_SIZE_2048 },
    { 4096, MENU_GEORAM_SIZE_4096 },
    { -1, 0 }
};

ui_res_possible_values vic20DigimaxBase[] = {
    { 0x9800, MENU_DIGIMAX_BASE_9800 },
    { 0x9820, MENU_DIGIMAX_BASE_9820 },
    { 0x9840, MENU_DIGIMAX_BASE_9840 },
    { 0x9860, MENU_DIGIMAX_BASE_9860 },
    { 0x9880, MENU_DIGIMAX_BASE_9880 },
    { 0x98a0, MENU_DIGIMAX_BASE_98A0 },
    { 0x98c0, MENU_DIGIMAX_BASE_98C0 },
    { 0x98e0, MENU_DIGIMAX_BASE_98E0 },
    { 0x9c00, MENU_DIGIMAX_BASE_9C00 },
    { 0x9c20, MENU_DIGIMAX_BASE_DF20 },
    { 0x9c40, MENU_DIGIMAX_BASE_DF40 },
    { 0x9c60, MENU_DIGIMAX_BASE_DF60 },
    { 0x9c80, MENU_DIGIMAX_BASE_DF80 },
    { 0x9ca0, MENU_DIGIMAX_BASE_DFA0 },
    { 0x9cc0, MENU_DIGIMAX_BASE_DFC0 },
    { 0x9ce0, MENU_DIGIMAX_BASE_DFE0 },
    { -1, 0 }
};

ui_res_possible_values vic20SFXSoundExpanderChip[] = {
    { 3526, MENU_SFX_SE_3526 },
    { 3812, MENU_SFX_SE_3812 },
    { -1, 0 }
};

ui_res_possible_values vic20_RenderFilters[] = {
    { VIDEO_FILTER_NONE, MENU_RENDER_FILTER_NONE },
    { VIDEO_FILTER_CRT, MENU_RENDER_FILTER_CRT_EMULATION },
    { VIDEO_FILTER_SCALE2X, MENU_RENDER_FILTER_SCALE2X },
    { -1, 0 }
};

ui_res_possible_values vic20_DS12C887RTC_base[] = {
    { 0x9800, MENU_DS12C887_RTC_BASE_9800 },
    { 0x9c00, MENU_DS12C887_RTC_BASE_9C00 },
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

ui_res_value_list vic20_ui_res_values[] = {
#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    { "Acia1Dev", vic20AciaDevice },
    { "Acia1Base", vic20AciaBase },
    { "Acia1Irq", vic20AciaInt },
    { "Acia1Mode", vic20AciaMode },
#endif
    { "GEORAMsize", vic20GeoRAMSize },
    { "DIGIMAXbase", vic20DigimaxBase },
    { "SFXSoundExpanderChip", vic20SFXSoundExpanderChip },
    { "VICFilter", vic20_RenderFilters },
    { "DS12C887RTCbase", vic20_DS12C887RTC_base },
    { "JoyPort1Device", vic20_JoyPort1Device },
    { "JoyPort3Device", vic20_JoyPort3Device },
    { "JoyPort4Device", vic20_JoyPort4Device },
    { "DoodleMultiColorHandling", DoodleMultiColor },
    { "IOCollisionHandling", IOCollisions },
    { NULL, NULL }
};

static ui_cartridge_t vic20_ui_cartridges[] = {
    { MENU_CART_VIC20_16KB_2000, CARTRIDGE_VIC20_16KB_2000, "4/8/16KB at $2000" },
    { MENU_CART_VIC20_16KB_4000, CARTRIDGE_VIC20_16KB_4000, "4/8/16KB at $4000" },
    { MENU_CART_VIC20_16KB_6000, CARTRIDGE_VIC20_16KB_6000, "4/8/16KB at $6000" },
    { MENU_CART_VIC20_8KB_A000, CARTRIDGE_VIC20_8KB_A000, "8KB at $A000" },
    { MENU_CART_VIC20_4KB_B000, CARTRIDGE_VIC20_4KB_B000, "4KB at $B000" },
    { 0, 0, NULL }
};


static void vic20_ui_attach_cartridge(int menu)
{
    int i = 0;

    while (menu != vic20_ui_cartridges[i].menu_item && vic20_ui_cartridges[i].menu_item) {
        i++;
    }

    if (!vic20_ui_cartridges[i].menu_item) {
        ui_error("Bad cartridge config in UI");
        return;
    }

    ui_select_file(B_OPEN_PANEL, VIC20_CARTRIDGE_FILE, &vic20_ui_cartridges[i]);
}       

static const char *vic20_sidcart_address_text_pair[] = { "$9800", "$9C00" };
static const char *vic20_sidcart_clock_pair[] = { "C64", "VIC20" };
static const int vic20_sidcart_address_int_pair[] = { 0x9800, 0x9C00 };

void vic20_ui_specific(void *msg, void *window)
{
    switch (((BMessage*)msg)->what) {
        case MENU_VIC20_MODEL_VIC20_PAL:
            vic20model_set(VIC20MODEL_VIC20_PAL);
            break;
        case MENU_VIC20_MODEL_VIC20_NTSC:
            vic20model_set(VIC20MODEL_VIC20_NTSC);
            break;
        case MENU_VIC20_MODEL_VIC21:
            vic20model_set(VIC20MODEL_VIC21);
            break;
        case MENU_CART_VIC20_16KB_2000:
        case MENU_CART_VIC20_16KB_4000:
        case MENU_CART_VIC20_16KB_6000:
        case MENU_CART_VIC20_8KB_A000:
        case MENU_CART_VIC20_4KB_B000:
            vic20_ui_attach_cartridge(((BMessage*)msg)->what);
            break;
        case MENU_CART_VIC20_GENERIC:
            ui_select_file(B_SAVE_PANEL, VIC20_GENERIC_CART_FILE, (void*)0);
            break;
        case MENU_CART_VIC20_UM:
            ui_select_file(B_SAVE_PANEL, VIC20_UM_FILE, (void*)0);
            break;
        case MENU_CART_VIC20_FP:
            ui_select_file(B_SAVE_PANEL, VIC20_FP_FILE, (void*)0);
            break;
        case MENU_CART_VIC20_BEHR_BONZ:
            ui_select_file(B_SAVE_PANEL, VIC20_BEHR_BONZ_FILE, (void*)0);
            break;
        case MENU_CART_VIC20_MEGACART:
            ui_select_file(B_SAVE_PANEL, VIC20_MEGACART_FILE, (void*)0);
            break;
        case MENU_CART_VIC20_FINAL_EXPANSION:
            ui_select_file(B_SAVE_PANEL, VIC20_FINAL_EXPANSION_FILE, (void*)0);
            break;
        case MENU_CART_VIC20_SMART_ATTACH:
            ui_select_file(B_SAVE_PANEL, VIC20_SMART_CART_ATTACH_FILE, (void*)0);
            break;
        case MENU_MC_NVRAM_FILE:
            ui_select_file(B_SAVE_PANEL, VIC20_MEGACART_NVRAM_FILE, (void*)0);
            break;
        case MENU_CART_SET_DEFAULT:
            cartridge_set_default();
            break;
        case MENU_CART_DETACH:
            cartridge_detach_image(-1);
            break;
        case ATTACH_VIC20_CART:
            {
                const char *filename;
                int32 type;

                ((BMessage*)msg)->FindInt32("type", &type);
                ((BMessage*)msg)->FindString("filename", &filename);
                if (cartridge_attach_image(type, filename) < 0) {
                    ui_error("Invalid cartridge image");
                }
                break;
            }
        case MENU_VIC20_SETTINGS:
            ui_vic20();
            break;
        case MENU_VIDEO_SETTINGS:
            ui_video(UI_VIDEO_CHIP_VIC);
            break;
        case MENU_VIC_SETTINGS:
            ui_vic();
            break;
        case MENU_JOYSTICK_SETTINGS:
            ui_joystick(1, 0);
            break;
        case MENU_USERPORT_JOY_SETTINGS:
            ui_joystick(3, 4);
            break;
        case MENU_DRIVE_SETTINGS:
            ui_drive(vic20_drive_types, HAS_NO_CAPS);
            break;
        case MENU_PRINTER_SETTINGS:
            ui_printer(HAS_USERPORT_PRINTER | HAS_IEC_BUS);
            break;
        case MENU_SIDCART_SETTINGS:
            ui_sidcart(vic20_sidcart_address_text_pair, vic20_sidcart_clock_pair, vic20_sidcart_address_int_pair);
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
        default:
            break;
    }
}

int vic20ui_init_early(void)
{
    vicemenu_set_joyport_func(joyport_get_valid_devices, joyport_get_port_name, 1, 0, 1, 1, 0);
    return 0;
}

static void build_joyport_values(void)
{
    int i;

    for (i = 0; i < JOYPORT_MAX_DEVICES; ++i) {
        vic20_JoyPort1Device[i].value = i;
        vic20_JoyPort1Device[i].item_id = MENU_JOYPORT1 + i;
        vic20_JoyPort3Device[i].value = i;
        vic20_JoyPort3Device[i].item_id = MENU_JOYPORT3 + i;
        vic20_JoyPort4Device[i].value = i;
        vic20_JoyPort4Device[i].item_id = MENU_JOYPORT4 + i;
    }
    vic20_JoyPort1Device[i].value = -1;
    vic20_JoyPort1Device[i].item_id = 0;
    vic20_JoyPort3Device[i].value = -1;
    vic20_JoyPort3Device[i].item_id = 0;
    vic20_JoyPort4Device[i].value = -1;
    vic20_JoyPort4Device[i].item_id = 0;
}

int vic20ui_init(void)
{
    build_joyport_values();
    ui_register_machine_specific(vic20_ui_specific);
    ui_register_menu_toggles(vic20_ui_menu_toggles);
    ui_register_res_values(vic20_ui_res_values);
    ui_update_menus();
    return 0;
}

void vic20ui_shutdown(void)
{
}

int vic20_cartridge_attach_image(int type, const char *filename)
{
    return cartridge_attach_image(type, filename);
}
