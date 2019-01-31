/*
 * scpu64ui.cc - SCPU64-specific user interface.
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
#include "c64model.h"
#include "cartio.h"
#include "cartridge.h"
#include "clockport.h"
#include "constants.h"
#include "gfxoutput.h"
#include "joyport.h"
#include "keyboard.h"
#include "mouse.h"
#include "resources.h"
#include "scpu64ui.h"
#include "types.h"
#include "ui.h"
#include "ui_cia.h"
#include "ui_drive.h"
#include "ui_ide64.h"
#include "ui_ide64usb.h"
#include "ui_joystick.h"
#include "ui_printer.h"
#include "ui_sid.h"
#include "ui_vicii.h"
#include "ui_video.h"
#include "util.h"
#include "vicii.h"
#include "video.h"
}

static ui_drive_type_t scpu64_drive_types[] = {
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

ui_menu_toggle scpu64_ui_menu_toggles[] = {
    { "VICIIDoubleSize", MENU_TOGGLE_DOUBLESIZE },
    { "VICIIDoubleScan", MENU_TOGGLE_DOUBLESCAN },
    { "VICIIVideoCache", MENU_TOGGLE_VIDEOCACHE },
#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    { "Acia1Enable", MENU_TOGGLE_ACIA },
#endif
    { "REU", MENU_TOGGLE_REU },
    { "REUImageWrite", MENU_TOGGLE_REU_SWC },
    { "GEORAM", MENU_TOGGLE_GEORAM },
    { "GEORAMImageWrite", MENU_TOGGLE_GEORAM_SWC },
    { "RAMCART", MENU_TOGGLE_RAMCART },
    { "RAMCARTImageWrite", MENU_TOGGLE_RAMCART_SWC },
    { "MagicVoiceCartridgeEnabled", MENU_TOGGLE_MAGICVOICE },
    { "DQBB", MENU_TOGGLE_DQBB },
    { "DQBBImageWrite", MENU_TOGGLE_DQBB_SWC },
    { "IsepicCartridgeEnabled", MENU_TOGGLE_ISEPIC },
    { "IsepicSwitch", MENU_TOGGLE_ISEPIC_SWITCH },
    { "IsepicImageWrite", MENU_TOGGLE_ISEPIC_SWC },
    { "DIGIMAX", MENU_TOGGLE_DIGIMAX },
    { "MMC64", MENU_TOGGLE_MMC64 },
    { "MMC64_flashjumper", MENU_TOGGLE_MMC64_FLASHJUMPER },
    { "MMC64_bios_write", MENU_TOGGLE_MMC64_SAVE },
    { "MMC64_RO", MENU_TOGGLE_MMC64_READ_ONLY },
    { "MMCREEPROMRW", MENU_TOGGLE_MMCR_EEPROM_READ_WRITE },
    { "MMCRImageWrite", MENU_TOGGLE_MMCR_EEPROM_SWC },
    { "MMCRRescueMode", MENU_TOGGLE_MMCR_RESCUE_MODE },
    { "MMCRCardRW", MENU_TOGGLE_MMCR_READ_WRITE },
    { "GMod2FlashWrite", MENU_TOGGLE_GMOD2_FLASH_READ_WRITE },
    { "GMOD2EEPROMRW", MENU_TOGGLE_GMOD2_EEPROM_SWC },
    { "Mouse", MENU_TOGGLE_MOUSE },
    { "SmartMouseRTCSave", MENU_TOGGLE_SMART_MOUSE_RTC_SAVE },
    { "SFXSoundExpander", MENU_TOGGLE_SFX_SE },
    { "SFXSoundSampler", MENU_TOGGLE_SFX_SS },
    { "EasyFlashJumper", MENU_TOGGLE_EASYFLASH_JUMPER },
    { "EasyFlashOptimizeCRT", MENU_TOGGLE_EASYFLASH_OPTIMIZE },
    { "EasyFlashWriteCRT", MENU_TOGGLE_EASYFLASH_AUTOSAVE },
    { "IECReset", MENU_IEC_RESET },
    { "IDE64RTCSave", MENU_TOGGLE_IDE64_RTC_SAVE },
    { "SBDIGIMAX", MENU_TOGGLE_IDE64_SB_DIGIMAX },
#ifdef HAVE_RAWNET
    { "SBETFE", MENU_TOGGLE_IDE64_SB_ETFE },
#endif
    { "ExpertCartridgeEnabled", MENU_TOGGLE_EXPERT },
    { "ExpertImageWrite", MENU_TOGGLE_EXPERT_SWC },
    { "JiffySwitch", MENU_TOGGLE_SCPU64_JIFFY_ENABLE },
    { "SpeedSwitch", MENU_TOGGLE_SCPU64_SPEED_ENABLE },
    { "RRFlashJumper", MENU_TOGGLE_RR_FLASH_JUMPER },
    { "RRBankJumper", MENU_TOGGLE_RR_BANK_JUMPER },
    { "RRBiosWrite",MENU_TOGGLE_SAVE_RR_FLASH },
#ifdef HAVE_RAWNET
    { "RRNETMK3_flashjumper", MENU_TOGGLE_RRNET_MK3_FLASH_JUMPER },
    { "RRNETMK3_bios_write", MENU_TOGGLE_SAVE_RRNET_MK3_FLASH },
#endif
    { "SSRamExpansion", MENU_TOGGLE_SS5_32K },
    { "DS12C887RTC", MENU_TOGGLE_DS12C887_RTC },
    { "DS12C887RTCRunMode", MENU_TOGGLE_DS12C887_RTC_RUNNING_MODE },
    { "DS12C887RTCSave", MENU_TOGGLE_DS12C887_RTC_SAVE },
    { "UserportDAC", MENU_TOGGLE_USERPORT_DAC },
    { "UserportRTC58321a", MENU_TOGGLE_USERPORT_58321A },
    { "UserportRTC58321aSave", MENU_TOGGLE_USERPORT_58321A_SAVE },
    { "UserportRTCDS1307", MENU_TOGGLE_USERPORT_DS1307 },
    { "UserportRTCDS1307Save", MENU_TOGGLE_USERPORT_DS1307_SAVE },
    { "UserportDIGIMAX", MENU_TOGGLE_USERPORT_DIGIMAX },
    { "Userport4bitSampler", MENU_TOGGLE_USERPORT_4BIT_SAMPLER },
    { "Userport8BSS", MENU_TOGGLE_USERPORT_8BSS },
    { NULL, 0 }
};

#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
static ui_res_possible_values scpu64AciaDevice[] = {
    { 1, MENU_ACIA_RS323_DEVICE_1 },
    { 2, MENU_ACIA_RS323_DEVICE_2 },
    { 3, MENU_ACIA_RS323_DEVICE_3 },
    { 4, MENU_ACIA_RS323_DEVICE_4 },
    { -1, 0 }
};

ui_res_possible_values scpu64_JoyPort1Device[JOYPORT_MAX_DEVICES + 1];
ui_res_possible_values scpu64_JoyPort2Device[JOYPORT_MAX_DEVICES + 1];
ui_res_possible_values scpu64_JoyPort3Device[JOYPORT_MAX_DEVICES + 1];
ui_res_possible_values scpu64_JoyPort4Device[JOYPORT_MAX_DEVICES + 1];

static ui_res_possible_values scpu64AciaBase[] = {
    { 0xde00, MENU_ACIA_BASE_DE00 },
    { 0xdf00, MENU_ACIA_BASE_DF00 },
    { -1, 0 }
};

static ui_res_possible_values scpu64AciaInt[] = {
    { 0, MENU_ACIA_INT_NONE },
    { 1, MENU_ACIA_INT_IRQ },
    { 2, MENU_ACIA_INT_NMI },
    { -1, 0 }
};

static ui_res_possible_values scpu64AciaMode[] = {
    { 0, MENU_ACIA_MODE_NORMAL },
    { 1, MENU_ACIA_MODE_SWIFTLINK },
    { 2, MENU_ACIA_MODE_TURBO232 },
    { -1, 0 }
};
#endif

static ui_res_possible_values scpu64ui_SimmSize[] = {
    { 0, MENU_SCPU64_SIMM_SIZE_0 },
    { 1, MENU_SCPU64_SIMM_SIZE_1 },
    { 4, MENU_SCPU64_SIMM_SIZE_4 },
    { 8, MENU_SCPU64_SIMM_SIZE_8 },
    { 16, MENU_SCPU64_SIMM_SIZE_16 },
    { -1, 0 }
};

static ui_res_possible_values scpu64ui_ReuSize[] = {
    { 128, MENU_REU_SIZE_128 },
    { 256, MENU_REU_SIZE_256 },
    { 512, MENU_REU_SIZE_512 },
    { 1024, MENU_REU_SIZE_1024 },
    { 2048, MENU_REU_SIZE_2048 },
    { 4096, MENU_REU_SIZE_4096 },
    { 8192, MENU_REU_SIZE_8192 },
    { 16384, MENU_REU_SIZE_16384 },
    { -1, 0 }
};

static ui_res_possible_values scpu64ui_GeoRAMSize[] = {
    { 64, MENU_GEORAM_SIZE_64 },
    { 128, MENU_GEORAM_SIZE_128 },
    { 256, MENU_GEORAM_SIZE_256 },
    { 512, MENU_GEORAM_SIZE_512 },
    { 1024, MENU_GEORAM_SIZE_1024 },
    { 2048, MENU_GEORAM_SIZE_2048 },
    { 4096, MENU_GEORAM_SIZE_4096 },
    { -1, 0 }
};

static ui_res_possible_values scpu64ui_RamCartSize[] = {
    { 64, MENU_RAMCART_SIZE_64 },
    { 128, MENU_RAMCART_SIZE_128 },
    { -1, 0 }
};

static ui_res_possible_values scpu64ui_DigimaxBase[] = {
    { 0xde00, MENU_DIGIMAX_BASE_DE00 },
    { 0xde20, MENU_DIGIMAX_BASE_DE20 },
    { 0xde40, MENU_DIGIMAX_BASE_DE40 },
    { 0xde60, MENU_DIGIMAX_BASE_DE60 },
    { 0xde80, MENU_DIGIMAX_BASE_DE80 },
    { 0xdea0, MENU_DIGIMAX_BASE_DEA0 },
    { 0xdec0, MENU_DIGIMAX_BASE_DEC0 },
    { 0xdee0, MENU_DIGIMAX_BASE_DEE0 },
    { 0xdf00, MENU_DIGIMAX_BASE_DF00 },
    { 0xdf20, MENU_DIGIMAX_BASE_DF20 },
    { 0xdf40, MENU_DIGIMAX_BASE_DF40 },
    { 0xdf60, MENU_DIGIMAX_BASE_DF60 },
    { 0xdf80, MENU_DIGIMAX_BASE_DF80 },
    { 0xdfa0, MENU_DIGIMAX_BASE_DFA0 },
    { 0xdfc0, MENU_DIGIMAX_BASE_DFC0 },
    { 0xdfe0, MENU_DIGIMAX_BASE_DFE0 },
    { -1, 0 }
};

static ui_res_possible_values scpu64ui_SFXSoundExpanderChip[] = {
    { 3526, MENU_SFX_SE_3526 },
    { 3812, MENU_SFX_SE_3812 },
    { -1, 0 }
};

static ui_res_possible_values scpu64ui_BurstMod[] = {
    { 0, MENU_BURSTMOD_NONE },
    { 1, MENU_BURSTMOD_CIA1 },
    { 2, MENU_BURSTMOD_CIA2 },
    { -1, 0 }
};

static ui_res_possible_values IDE64DigimaxBase[] = {
    { 0xde40, MENU_IDE64_SB_DIGIMAX_BASE_DE40 },
    { 0xde48, MENU_IDE64_SB_DIGIMAX_BASE_DE48 },
    { -1, 0 }
};

#ifdef HAVE_RAWNET
static ui_res_possible_values IDE64ETFEBase[] = {
    { 0xde00, MENU_IDE64_SB_ETFE_BASE_DE00 },
    { 0xde10, MENU_IDE64_SB_ETFE_BASE_DE10 },
    { 0xdf00, MENU_IDE64_SB_ETFE_BASE_DF00 },
    { -1, 0 }
};
#endif

static ui_res_possible_values IDE64ClockPortDevice[] = {
    { CLOCKPORT_DEVICE_NONE, MENU_IDE64_CLOCKPORT_NONE },
#ifdef HAVE_RAWNET
    { CLOCKPORT_DEVICE_RRNET, MENU_IDE64_CLOCKPORT_RRNET },
#endif
#ifdef USE_MPG123
    { CLOCKPORT_DEVICE_MP3_64, MENU_IDE64_CLOCKPORT_MP3AT64 },
#endif
    { -1, 0 }
};

static ui_res_possible_values scpu64ui_MMC64Revision[] = {
    { 0, MENU_MMC64_REVISION_A },
    { 1, MENU_MMC64_REVISION_B },
    { -1, 0 }
};

static ui_res_possible_values scpu64ui_MMC64CardType[] = {
    { 0, MENU_MMC64_CARD_TYPE_AUTO },
    { 1, MENU_MMC64_CARD_TYPE_MMC },
    { 2, MENU_MMC64_CARD_TYPE_SD },
    { 3, MENU_MMC64_CARD_TYPE_SDHC },
    { -1, 0 }
};

static ui_res_possible_values MMC64ClockPortDevice[] = {
    { CLOCKPORT_DEVICE_NONE, MENU_MMC64_CLOCKPORT_NONE },
#ifdef HAVE_RAWNET
    { CLOCKPORT_DEVICE_RRNET, MENU_MMC64_CLOCKPORT_RRNET },
#endif
#ifdef USE_MPG123
    { CLOCKPORT_DEVICE_MP3_64, MENU_MMC64_CLOCKPORT_MP3AT64 },
#endif
    { -1, 0 }
};

static ui_res_possible_values scpu64ui_MMCRCardType[] = {
    { 0, MENU_MMCR_CARD_TYPE_AUTO },
    { 1, MENU_MMCR_CARD_TYPE_MMC },
    { 2, MENU_MMCR_CARD_TYPE_SD },
    { 3, MENU_MMCR_CARD_TYPE_SDHC },
    { -1, 0 }
};

static ui_res_possible_values MMCRClockPortDevice[] = {
    { CLOCKPORT_DEVICE_NONE, MENU_MMCR_CLOCKPORT_NONE },
#ifdef HAVE_RAWNET
    { CLOCKPORT_DEVICE_RRNET, MENU_MMCR_CLOCKPORT_RRNET },
#endif
#ifdef USE_MPG123
    { CLOCKPORT_DEVICE_MP3_64, MENU_MMCR_CLOCKPORT_MP3AT64 },
#endif
    { -1, 0 }
};

static ui_res_possible_values scpu64ui_viciimodels[] = {
    { VICII_MODEL_6569, MENU_VICII_MODEL_6569_PAL },
    { VICII_MODEL_8565, MENU_VICII_MODEL_8565_PAL },
    { VICII_MODEL_6569R1, MENU_VICII_MODEL_6569R1_OLD_PAL },
    { VICII_MODEL_6567, MENU_VICII_MODEL_6567_NTSC },
    { VICII_MODEL_8562, MENU_VICII_MODEL_8562_NTSC },
    { VICII_MODEL_6567R56A, MENU_VICII_MODEL_6567R56A_OLD_NTSC },
    { VICII_MODEL_6572, MENU_VICII_MODEL_6572_PAL_N },
    { -1, 0 }
};

static ui_res_possible_values scpu64ui_gluelogic[] = {
    { 0, MENU_GLUE_LOGIC_DISCRETE },
    { 1, MENU_GLUE_LOGIC_CUSTOM_IC },
    { -1, 0 }
};

static ui_res_possible_values scpu64ui_RenderFilters[] = {
    { VIDEO_FILTER_NONE, MENU_RENDER_FILTER_NONE },
    { VIDEO_FILTER_CRT, MENU_RENDER_FILTER_CRT_EMULATION },
    { VIDEO_FILTER_SCALE2X, MENU_RENDER_FILTER_SCALE2X },
    { -1, 0 }
};

static ui_res_possible_values scpu64ui_ExpertModes[] = {
    { 0, MENU_EXPERT_MODE_OFF },
    { 1, MENU_EXPERT_MODE_PRG },
    { 2, MENU_EXPERT_MODE_ON },
    { -1, 0 }
};

static ui_res_possible_values scpu64RRrevs[] = {
    { 0, MENU_RR_REV_RETRO },
    { 1, MENU_RR_REV_NORDIC },
    { -1, 0 }
};

static ui_res_possible_values scpu64_DS12C887RTC_base[] = {
    { 0xd500, MENU_DS12C887_RTC_BASE_D500 },
    { 0xd600, MENU_DS12C887_RTC_BASE_D600 },
    { 0xd700, MENU_DS12C887_RTC_BASE_D700 },
    { 0xde00, MENU_DS12C887_RTC_BASE_DE00 },
    { 0xdf00, MENU_DS12C887_RTC_BASE_DF00 },
    { -1, 0 }
};

static ui_res_possible_values scpu64ui_IDE64version[] = {
    { 0, MENU_IDE64_VERSION_V3 },
    { 1, MENU_IDE64_VERSION_V4_1 },
    { 2, MENU_IDE64_VERSION_V4_2 },
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

static ui_res_possible_values RRClockPortDevice[] = {
    { CLOCKPORT_DEVICE_NONE, MENU_RR_CLOCKPORT_NONE },
#ifdef HAVE_RAWNET
    { CLOCKPORT_DEVICE_RRNET, MENU_RR_CLOCKPORT_RRNET },
#endif
#ifdef USE_MPG123
    { CLOCKPORT_DEVICE_MP3_64, MENU_RR_CLOCKPORT_MP3AT64 },
#endif
    { -1, 0 }
};

ui_res_value_list scpu64_ui_res_values[] = {
#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    { "Acia1Dev", scpu64AciaDevice },
    { "Acia1Base", scpu64AciaBase },
    { "Acia1Irq", scpu64AciaInt },
    { "Acia1Mode", scpu64AciaMode },
#endif
    { "VICIIModel", scpu64ui_viciimodels },
    { "SIMMSize", scpu64ui_SimmSize },
    { "REUsize", scpu64ui_ReuSize },
    { "GEORAMsize", scpu64ui_GeoRAMSize },
    { "RAMCARTsize", scpu64ui_RamCartSize },
    { "DIGIMAXbase", scpu64ui_DigimaxBase },
    { "SFXSoundExpanderChip", scpu64ui_SFXSoundExpanderChip },
    { "BurstMod", scpu64ui_BurstMod },
    { "MMC64_revision", scpu64ui_MMC64Revision },
    { "MMC64_sd_type", scpu64ui_MMC64CardType },
    { "MMCRSDType", scpu64ui_MMCRCardType },
    { "GlueLogic", scpu64ui_gluelogic },
    { "VICIIFilter", scpu64ui_RenderFilters },
    { "ExpertCartridgeMode", scpu64ui_ExpertModes },
    { "RRrevision", scpu64RRrevs },
    { "DS12C887RTCbase", scpu64_DS12C887RTC_base },
    { "IDE64version", scpu64ui_IDE64version },
    { "JoyPort1Device", scpu64_JoyPort1Device },
    { "JoyPort2Device", scpu64_JoyPort2Device },
    { "JoyPort3Device", scpu64_JoyPort3Device },
    { "JoyPort4Device", scpu64_JoyPort4Device },
    { "DoodleMultiColorHandling", DoodleMultiColor },
    { "IOCollisionHandling", IOCollisions },
    { "RRClockPort", RRClockPortDevice },
    { "MMCRClockPort", MMCRClockPortDevice },
    { "MMC64ClockPort", MMC64ClockPortDevice },
    { "SBDIGIMAXbase", IDE64DigimaxBase },
#ifdef HAVE_RAWNET
    { "SBETFEbase", IDE64ETFEBase },
#endif
    { "IDE64ClockPort", IDE64ClockPortDevice },
    { NULL, NULL }
};

static ui_cartridge_t *scpu64_ui_cartridges = NULL;

static void scpu64_ui_attach_cartridge(int menu)
{
    int i = 0;

    while (menu != scpu64_ui_cartridges[i].menu_item && scpu64_ui_cartridges[i].menu_item) {
        i++;
    }

    if (!scpu64_ui_cartridges[i].menu_item) {
        ui_error("Bad cartridge config in UI");
        return;
    }

    ui_select_file(B_OPEN_PANEL, C64_CARTRIDGE_FILE, &scpu64_ui_cartridges[i]);
}       

static int scpu64_sid_address_base[] = { 0xd4, 0xd5, 0xd6, 0xd7, 0xde, 0xdf, -1 };

static void scpu64_ui_specific(void *msg, void *window)
{
    if (((BMessage*)msg)->what >= MENU_GENERIC_CARTS && ((BMessage*)msg)->what < MENU_END_CARTS) {
        scpu64_ui_attach_cartridge(((BMessage*)msg)->what);
    } else {
        switch (((BMessage*)msg)->what) {
            case MENU_CART_ATTACH_CRT:      
                scpu64_ui_attach_cartridge(((BMessage*)msg)->what);
                break;
            case MENU_CART_SET_DEFAULT:
                cartridge_set_default();
                break;
            case MENU_CART_DETACH:
                cartridge_detach_image(-1);
                break;
            case ATTACH_C64_CART:
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
            case MENU_VIDEO_SETTINGS:
                ui_video(UI_VIDEO_CHIP_VICII);
                break;
            case MENU_VICII_SETTINGS:
                ui_vicii();
                break;
            case MENU_SID_SETTINGS:
                ui_sid(scpu64_sid_address_base);
                break;
            case MENU_CIA_SETTINGS:
                ui_cia(2);
                break;
            case MENU_JOYSTICK_SETTINGS:
                ui_joystick(1, 2);
                break;
            case MENU_USERPORT_JOY_SETTINGS:
                ui_joystick(3, 4);
                break;
            case MENU_DRIVE_SETTINGS:
                ui_drive(scpu64_drive_types, HAS_PARA_CABLE | HAS_PROFDOS);
                break;
            case MENU_PRINTER_SETTINGS:
                ui_printer(HAS_USERPORT_PRINTER | HAS_IEC_BUS);
                break;
            case MENU_IDE64_FILE1:
                ui_select_file(B_SAVE_PANEL, IDE64_FILE1, (void*)0);
                break;
            case MENU_IDE64_SIZE1:
                ui_ide64(1);
                break;
            case MENU_IDE64_FILE2:
                ui_select_file(B_SAVE_PANEL, IDE64_FILE2, (void*)0);
                break;
            case MENU_IDE64_SIZE2:
                ui_ide64(2);
                break;
            case MENU_IDE64_FILE3:
                ui_select_file(B_SAVE_PANEL, IDE64_FILE3, (void*)0);
                break;
            case MENU_IDE64_SIZE3:
                ui_ide64(3);
                break;
            case MENU_IDE64_FILE4:
                ui_select_file(B_SAVE_PANEL, IDE64_FILE4, (void*)0);
                break;
            case MENU_IDE64_SIZE4:
                ui_ide64(4);
                break;
            case MENU_IDE64_USB_SERVER:
                ui_ide64usb();
                break;
            case MENU_MAGICVOICE_FILE:
                ui_select_file(B_SAVE_PANEL, MAGICVOICE_FILE, (void*)0);
                break;
            case MENU_REU_FILE:
                ui_select_file(B_SAVE_PANEL, REU_FILE, (void*)0);
                break;
            case MENU_GEORAM_FILE:
                ui_select_file(B_SAVE_PANEL, GEORAM_FILE, (void*)0);
                break;
            case MENU_RAMCART_FILE:
                ui_select_file(B_SAVE_PANEL, RAMCART_FILE, (void*)0);
                break;
            case MENU_DQBB_FILE:
                ui_select_file(B_SAVE_PANEL, DQBB_FILE, (void*)0);
                break;
            case MENU_ISEPIC_FILE:
                ui_select_file(B_SAVE_PANEL, ISEPIC_FILE, (void*)0);
                break;
            case MENU_MMC64_BIOS_FILE:
                ui_select_file(B_OPEN_PANEL, MMC64_BIOS_FILE, (void*)0);
                break;
            case MENU_MMC64_IMAGE_FILE:
                ui_select_file(B_OPEN_PANEL, MMC64_IMAGE_FILE, (void*)0);
                break;
            case MENU_MMCR_EEPROM_FILE:
                ui_select_file(B_OPEN_PANEL, MMCR_EEPROM_FILE, (void*)0);
                break;
            case MENU_MMCR_IMAGE_FILE:
                ui_select_file(B_OPEN_PANEL, MMCR_IMAGE_FILE, (void*)0);
                break;
            case MENU_GMOD2_EEPROM_FILE:
                ui_select_file(B_OPEN_PANEL, GMOD2_EEPROM_FILE, (void*)0);
                break;
            case MENU_EXPERT_FILE:
                ui_select_file(B_OPEN_PANEL, EXPERT_FILE, (void*)0);
                break;
            case MENU_EASYFLASH_SAVE_NOW:
                if (cartridge_flush_image(CARTRIDGE_EASYFLASH) < 0) {
                    ui_error("Error saving EasyFlash .crt file");
                }
                break;
            case MENU_C64_MODEL_C64_PAL:
                c64model_set(C64MODEL_C64_PAL);
                break;
            case MENU_C64_MODEL_C64C_PAL:
                c64model_set(C64MODEL_C64C_PAL);
                break;
            case MENU_C64_MODEL_C64_OLD_PAL:
                c64model_set(C64MODEL_C64_OLD_PAL);
                break;
            case MENU_C64_MODEL_C64_NTSC:
                c64model_set(C64MODEL_C64_NTSC);
                break;
            case MENU_C64_MODEL_C64C_NTSC:
                c64model_set(C64MODEL_C64C_NTSC);
                break;
            case MENU_C64_MODEL_C64_OLD_NTSC:
                c64model_set(C64MODEL_C64_OLD_NTSC);
                break;
            case MENU_C64_MODEL_DREAN:
                c64model_set(C64MODEL_C64_PAL_N);
                break;
            case MENU_COMPUTER_SCPU64_ROM_FILE:
                ui_select_file(B_SAVE_PANEL, COMPUTER_SCPU64_ROM_FILE, (void*)0);
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
            case MENU_DRIVE_PROFDOS_ROM_FILE:
                ui_select_file(B_SAVE_PANEL, DRIVE_PROFDOS_ROM_FILE, (void*)0);
                break;
            case MENU_DRIVE_SUPERCARD_ROM_FILE:
                ui_select_file(B_SAVE_PANEL, DRIVE_SUPERCARD_ROM_FILE, (void*)0);
                break;
            default:
                break;
        }
    }
}

int scpu64ui_init_early(void)
{
    vicemenu_set_joyport_func(joyport_get_valid_devices, joyport_get_port_name, 1, 1, 1, 1, 0);
    vicemenu_set_cart_func(cartridge_get_info_list);
    return 0;
}

static void build_cart_list(void)
{
    int i = 0;
    int j;

    cartridge_info_t *cartlist = cartridge_get_info_list();

    for (j = 0; cartlist[j].name; ++j) {}
    j += 2;

    scpu64_ui_cartridges = (ui_cartridge_t *)lib_malloc(sizeof(ui_cartridge_t) * j);

    scpu64_ui_cartridges[0].menu_item = MENU_CART_ATTACH_CRT;
    scpu64_ui_cartridges[0].cart_type = CARTRIDGE_CRT;
    scpu64_ui_cartridges[0].cart_name = "CRT";

    j = 1;

    for (i = 0; cartlist[i].name; ++i) {
        if (cartlist[i].flags &= CARTRIDGE_GROUP_GENERIC) {
            scpu64_ui_cartridges[j].menu_item = MENU_GENERIC_CARTS + cartlist[i].crtid + 256;
            scpu64_ui_cartridges[j].cart_type = cartlist[i].crtid;
            scpu64_ui_cartridges[j].cart_name = cartlist[i].name;
            ++j;
        }
    }

    for (i = 0; cartlist[i].name; ++i) {
        if (cartlist[i].flags &= CARTRIDGE_GROUP_RAMEX) {
            scpu64_ui_cartridges[j].menu_item = MENU_RAMEX_CARTS + cartlist[i].crtid + 256;
            scpu64_ui_cartridges[j].cart_type = cartlist[i].crtid;
            scpu64_ui_cartridges[j].cart_name = cartlist[i].name;
            ++j;
        }
    }

    for (i = 0; cartlist[i].name; ++i) {
        if (cartlist[i].flags &= CARTRIDGE_GROUP_GAME) {
            scpu64_ui_cartridges[j].menu_item = MENU_GAME_CARTS + cartlist[i].crtid + 256;
            scpu64_ui_cartridges[j].cart_type = cartlist[i].crtid;
            scpu64_ui_cartridges[j].cart_name = cartlist[i].name;
            ++j;
        }
    }

    for (i = 0; cartlist[i].name; ++i) {
        if (cartlist[i].flags &= CARTRIDGE_GROUP_UTIL) {
            scpu64_ui_cartridges[j].menu_item = MENU_GAME_CARTS + cartlist[i].crtid + 256;
            scpu64_ui_cartridges[j].cart_type = cartlist[i].crtid;
            scpu64_ui_cartridges[j].cart_name = cartlist[i].name;
            ++j;
        }
    }

    scpu64_ui_cartridges[j].menu_item = 0;
    scpu64_ui_cartridges[j].cart_type = 0;
    scpu64_ui_cartridges[j].cart_name = NULL;
}

static void build_joyport_values(void)
{
    int i;

    for (i = 0; i < JOYPORT_MAX_DEVICES; ++i) {
        scpu64_JoyPort1Device[i].value = i;
        scpu64_JoyPort1Device[i].item_id = MENU_JOYPORT1 + i;
        scpu64_JoyPort2Device[i].value = i;
        scpu64_JoyPort2Device[i].item_id = MENU_JOYPORT2 + i;
        scpu64_JoyPort3Device[i].value = i;
        scpu64_JoyPort3Device[i].item_id = MENU_JOYPORT3 + i;
        scpu64_JoyPort4Device[i].value = i;
        scpu64_JoyPort4Device[i].item_id = MENU_JOYPORT4 + i;
    }
    scpu64_JoyPort1Device[i].value = -1;
    scpu64_JoyPort1Device[i].item_id = 0;
    scpu64_JoyPort2Device[i].value = -1;
    scpu64_JoyPort2Device[i].item_id = 0;
    scpu64_JoyPort3Device[i].value = -1;
    scpu64_JoyPort3Device[i].item_id = 0;
    scpu64_JoyPort4Device[i].value = -1;
    scpu64_JoyPort4Device[i].item_id = 0;
}

int scpu64ui_init(void)
{
    build_joyport_values();
    build_cart_list();
    ui_register_machine_specific(scpu64_ui_specific);
    ui_register_menu_toggles(scpu64_ui_menu_toggles);
    ui_register_res_values(scpu64_ui_res_values);
    ui_update_menus();
    return 0;
}

void scpu64ui_shutdown(void)
{
    if (scpu64_ui_cartridges) {
        lib_free(scpu64_ui_cartridges);
    }
}
