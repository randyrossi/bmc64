/*
 * menu_c64cart.c - Implementation of the C64/C128 cartridge settings menu for the SDL UI.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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
#include <stdlib.h>
#include <string.h>

#include "c64cart.h"
#include "cartridge.h"
#include "cartio.h"
#include "clockport.h"
#include "keyboard.h"
#include "lib.h"
#include "machine.h"
#include "menu_common.h"
#include "menu_c64_common_expansions.h"
#include "menu_c64cart.h"
#include "resources.h"
#include "ui.h"
#include "uifilereq.h"
#include "uimenu.h"

static UI_MENU_CALLBACK(attach_c64_cart_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Select cartridge image", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            if (cartridge_attach_image(vice_ptr_to_int(param), name) < 0) {
                ui_error("Cannot load cartridge image.");
            }
            lib_free(name);
        }
    }
    return NULL;
}

ui_menu_entry_t ui_c64cart_entry = {
    NULL, MENU_ENTRY_DIALOG, (ui_callback_t)attach_c64_cart_callback,
    (ui_callback_data_t)0
};

static int countgroup(cartridge_info_t *cartlist, int flags)
{
    int num = 0;
    while(cartlist->name) {
        if (cartlist->flags & flags) {
            num++;
        }
        cartlist++;
    }
    return num;
}

static void makegroup(cartridge_info_t *cartlist, ui_menu_entry_t *entry, int flags)
{
    while(cartlist->name) {
        if (cartlist->flags & flags) {
            ui_c64cart_entry.string = cartlist->name;
            ui_c64cart_entry.data = (ui_callback_data_t)(int_to_void_ptr(cartlist->crtid));
            memcpy(entry, &ui_c64cart_entry, sizeof(ui_menu_entry_t));
            entry++;
        }
        cartlist++;
    }
    memset(entry, 0, sizeof(ui_menu_entry_t));
}

static ui_menu_entry_t *attach_raw_cart_menu = NULL;

void uicart_menu_create(void)
{
    int num;
    cartridge_info_t *cartlist = cartridge_get_info_list();

    num = countgroup(cartlist, CARTRIDGE_GROUP_GENERIC | CARTRIDGE_GROUP_FREEZER | CARTRIDGE_GROUP_GAME | CARTRIDGE_GROUP_UTIL);
    attach_raw_cart_menu = lib_malloc(sizeof(ui_menu_entry_t) * (num + 1));
    makegroup(cartlist, attach_raw_cart_menu, CARTRIDGE_GROUP_GENERIC | CARTRIDGE_GROUP_FREEZER | CARTRIDGE_GROUP_GAME | CARTRIDGE_GROUP_UTIL);
    if (machine_class == VICE_MACHINE_SCPU64) {
        scpu64cart_menu[1].data = attach_raw_cart_menu;
    } else if (machine_class == VICE_MACHINE_C128) {
        c128cart_menu[1].data = attach_raw_cart_menu;
    } else {
        c64cart_menu[1].data = attach_raw_cart_menu;
    }
}


void uicart_menu_shutdown(void)
{
    if (attach_raw_cart_menu != NULL) {
        lib_free(attach_raw_cart_menu);
    }
}


static UI_MENU_CALLBACK(detach_c64_cart_callback)
{
    if (activated) {
        cartridge_detach_image(-1);
    }
    return NULL;
}

static UI_MENU_CALLBACK(c64_cart_freeze_callback)
{
    if (activated) {
        keyboard_clear_keymatrix();
        cartridge_trigger_freeze();
    }
    return NULL;
}

static UI_MENU_CALLBACK(set_c64_cart_default_callback)
{
    if (activated) {
        cartridge_set_default();
    }
    return NULL;
}

/* FIXME: we need an error reporting system, so all this
          stuff can go away. */
typedef struct c64_cart_flush_s {
    int cartid;
    char *enable_res;
    char *image_res;
} c64_cart_flush_t;

static c64_cart_flush_t carts[] = {
    { CARTRIDGE_RAMCART, "RAMCART", "RAMCARTfilename" },
    { CARTRIDGE_REU, "REU", "REUfilename" },
    { CARTRIDGE_EXPERT, "ExpertCartridgeEnabled", "Expertfilename" },
    { CARTRIDGE_DQBB, "DQBB", "DQBBfilename" },
    { CARTRIDGE_ISEPIC, "IsepicCartridgeEnabled", "Isepicfilename" },
    { CARTRIDGE_EASYFLASH, NULL, NULL },
    { CARTRIDGE_GEORAM, "GEORAM", "GEORAMfilename" },
    { CARTRIDGE_MMC64, "MMC64", "MMC64BIOSfilename" },
    { CARTRIDGE_MMC_REPLAY, NULL, "MMCREEPROMImage" },
    { CARTRIDGE_GMOD2, NULL, "GMod2EEPROMImage" },
    { CARTRIDGE_RETRO_REPLAY, NULL, NULL },
    { 0, NULL, NULL }
};

static UI_MENU_CALLBACK(c64_cart_flush_callback)
{
    int i;
    int found = 0;
    int enabled = 1;
    const char *filename = "a";

    if (activated) {
        int cartid = vice_ptr_to_int(param);

        if (cartridge_flush_image(cartid) < 0) {
            /* find cartid in carts */
            for (i = 0; carts[i].cartid != 0 && !found; i++) {
                if (carts[i].cartid == cartid) {
                    found = 1;
                }
            }
            i--;

            /* check if cart was enabled */
            if (found) {
                if (carts[i].enable_res) {
                    resources_get_int(carts[i].enable_res, &enabled);
                }
            }

            /* check if cart has image */
            if (found) {
                if (carts[i].image_res) {
                    resources_get_string(carts[i].image_res, &filename);
                }
            }

            if (!enabled) {
                ui_error("Cartridge is not enabled.");
            } else if (!filename) {
                ui_error("No name defined for cart image.");
            } else if (!*filename) {
                ui_error("No name defined for cart image.");
            } else {
                ui_error("Cannot save cartridge image.");
            }
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(c64_cart_save_callback)
{
    if (activated) {
        int cartid = vice_ptr_to_int(param);
        char *name = NULL;

        name = sdl_ui_file_selection_dialog("Choose cartridge file", FILEREQ_MODE_SAVE_FILE);

        if (name != NULL) {
            if (cartridge_save_image(cartid, name) < 0) {
                ui_error("Cannot save cartridge image.");
            }
            lib_free(name);
        }
    }
    return NULL;
}

/* RAMCART */

UI_MENU_DEFINE_TOGGLE(RAMCART)
UI_MENU_DEFINE_TOGGLE(RAMCART_RO)
UI_MENU_DEFINE_RADIO(RAMCARTsize)
UI_MENU_DEFINE_FILE_STRING(RAMCARTfilename)
UI_MENU_DEFINE_TOGGLE(RAMCARTImageWrite)

static const ui_menu_entry_t ramcart_menu[] = {
    { "Enable " CARTRIDGE_NAME_RAMCART,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_RAMCART_callback,
      NULL },
    { "Read-only",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_RAMCART_RO_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Memory size"),
    { "64kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMCARTsize_callback,
      (ui_callback_data_t)64 },
    { "128kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RAMCARTsize_callback,
      (ui_callback_data_t)128 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("RAM image"),
    { "Image file",
      MENU_ENTRY_DIALOG,
      file_string_RAMCARTfilename_callback,
      (ui_callback_data_t)"Select " CARTRIDGE_NAME_RAMCART " image" },
    { "Save image on detach",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_RAMCARTImageWrite_callback,
      NULL },
    { "Save image now",
      MENU_ENTRY_OTHER,
      c64_cart_flush_callback,
      (ui_callback_data_t)CARTRIDGE_RAMCART },
    { "Save image as",
      MENU_ENTRY_OTHER,
      c64_cart_save_callback,
      (ui_callback_data_t)CARTRIDGE_RAMCART },
    SDL_MENU_LIST_END
};


/* REU */

UI_MENU_DEFINE_TOGGLE(REU)
UI_MENU_DEFINE_RADIO(REUsize)
UI_MENU_DEFINE_FILE_STRING(REUfilename)
UI_MENU_DEFINE_TOGGLE(REUImageWrite)

static const ui_menu_entry_t reu_menu[] = {
    { "Enable " CARTRIDGE_NAME_REU,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_REU_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Memory size"),
    { "128kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_REUsize_callback,
      (ui_callback_data_t)128 },
    { "256kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_REUsize_callback,
      (ui_callback_data_t)256 },
    { "512kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_REUsize_callback,
      (ui_callback_data_t)512 },
    { "1024kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_REUsize_callback,
      (ui_callback_data_t)1024 },
    { "2048kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_REUsize_callback,
      (ui_callback_data_t)2048 },
    { "4096kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_REUsize_callback,
      (ui_callback_data_t)4096 },
    { "8192kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_REUsize_callback,
      (ui_callback_data_t)8192 },
    { "16384kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_REUsize_callback,
      (ui_callback_data_t)16384 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("RAM image"),
    { "Image file",
      MENU_ENTRY_DIALOG,
      file_string_REUfilename_callback,
      (ui_callback_data_t)"Select " CARTRIDGE_NAME_REU " image" },
    { "Save image on detach",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_REUImageWrite_callback,
      NULL },
    { "Save image now",
      MENU_ENTRY_OTHER,
      c64_cart_flush_callback,
      (ui_callback_data_t)CARTRIDGE_REU },
    { "Save image as",
      MENU_ENTRY_OTHER,
      c64_cart_save_callback,
      (ui_callback_data_t)CARTRIDGE_REU },
    SDL_MENU_LIST_END
};


/* Expert cartridge */

UI_MENU_DEFINE_TOGGLE(ExpertCartridgeEnabled)
UI_MENU_DEFINE_RADIO(ExpertCartridgeMode)
UI_MENU_DEFINE_FILE_STRING(Expertfilename)
UI_MENU_DEFINE_TOGGLE(ExpertImageWrite)

static const ui_menu_entry_t expert_cart_menu[] = {
    { "Enable " CARTRIDGE_NAME_EXPERT,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_ExpertCartridgeEnabled_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Mode"),
    { "Off",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_ExpertCartridgeMode_callback,
      (ui_callback_data_t)EXPERT_MODE_OFF },
    { "Prg",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_ExpertCartridgeMode_callback,
      (ui_callback_data_t)EXPERT_MODE_PRG },
    { "On",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_ExpertCartridgeMode_callback,
      (ui_callback_data_t)EXPERT_MODE_ON },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("RAM image"),
    { "Image file",
      MENU_ENTRY_DIALOG,
      file_string_Expertfilename_callback,
      (ui_callback_data_t)"Select " CARTRIDGE_NAME_EXPERT " image" },
    { "Save image on detach",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_ExpertImageWrite_callback,
      NULL },
    { "Save image now",
      MENU_ENTRY_OTHER,
      c64_cart_flush_callback,
      (ui_callback_data_t)CARTRIDGE_EXPERT },
    { "Save image as",
      MENU_ENTRY_OTHER,
      c64_cart_save_callback,
      (ui_callback_data_t)CARTRIDGE_EXPERT },
    SDL_MENU_LIST_END
};


/* Double Quick Brown Box */

UI_MENU_DEFINE_TOGGLE(DQBB)
UI_MENU_DEFINE_FILE_STRING(DQBBfilename)
UI_MENU_DEFINE_TOGGLE(DQBBImageWrite)

static const ui_menu_entry_t dqbb_cart_menu[] = {
    { "Enable " CARTRIDGE_NAME_DQBB,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DQBB_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("RAM image"),
    { "Image file",
      MENU_ENTRY_DIALOG,
      file_string_DQBBfilename_callback,
      (ui_callback_data_t)"Select " CARTRIDGE_NAME_DQBB " image" },
    { "Save image on detach",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DQBBImageWrite_callback,
      NULL },
    { "Save image now",
      MENU_ENTRY_OTHER,
      c64_cart_flush_callback,
      (ui_callback_data_t)CARTRIDGE_DQBB },
    { "Save image as",
      MENU_ENTRY_OTHER,
      c64_cart_save_callback,
      (ui_callback_data_t)CARTRIDGE_DQBB },
    SDL_MENU_LIST_END
};


/* ISEPIC */

UI_MENU_DEFINE_TOGGLE(IsepicCartridgeEnabled)
UI_MENU_DEFINE_TOGGLE(IsepicSwitch)
UI_MENU_DEFINE_FILE_STRING(Isepicfilename)
UI_MENU_DEFINE_TOGGLE(IsepicImageWrite)

static const ui_menu_entry_t isepic_cart_menu[] = {
    { "Enable " CARTRIDGE_NAME_ISEPIC,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IsepicCartridgeEnabled_callback,
      NULL },
    { "Switch",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IsepicSwitch_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("RAM image"),
    { "Image file",
      MENU_ENTRY_DIALOG,
      file_string_Isepicfilename_callback,
      (ui_callback_data_t)"Select " CARTRIDGE_NAME_ISEPIC " image" },
    { "Save image on detach",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IsepicImageWrite_callback,
      NULL },
    { "Save image now",
      MENU_ENTRY_OTHER,
      c64_cart_flush_callback,
      (ui_callback_data_t)CARTRIDGE_ISEPIC },
    { "Save image as",
      MENU_ENTRY_OTHER,
      c64_cart_save_callback,
      (ui_callback_data_t)CARTRIDGE_ISEPIC },
    SDL_MENU_LIST_END
};


/* EasyFlash */

UI_MENU_DEFINE_TOGGLE(EasyFlashJumper)
UI_MENU_DEFINE_TOGGLE(EasyFlashWriteCRT)
UI_MENU_DEFINE_TOGGLE(EasyFlashOptimizeCRT)


static const ui_menu_entry_t easyflash_cart_menu[] = {
    { "Jumper",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_EasyFlashJumper_callback,
      NULL },
    { "Save image on detach",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_EasyFlashWriteCRT_callback,
      NULL },
    { "Optimize image on write",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_EasyFlashOptimizeCRT_callback,
      NULL },
    { "Save image now",
      MENU_ENTRY_OTHER,
      c64_cart_flush_callback,
      (ui_callback_data_t)CARTRIDGE_EASYFLASH },
    { "Save image as",
      MENU_ENTRY_OTHER,
      c64_cart_save_callback,
      (ui_callback_data_t)CARTRIDGE_EASYFLASH },
    SDL_MENU_LIST_END
};


/* GEORAM */

UI_MENU_DEFINE_TOGGLE(GEORAM)
UI_MENU_DEFINE_RADIO(GEORAMsize)
UI_MENU_DEFINE_FILE_STRING(GEORAMfilename)
UI_MENU_DEFINE_TOGGLE(GEORAMImageWrite)

static const ui_menu_entry_t georam_menu[] = {
    { "Enable " CARTRIDGE_NAME_GEORAM,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_GEORAM_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Memory size"),
    { "64kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_GEORAMsize_callback,
      (ui_callback_data_t)64 },
    { "128kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_GEORAMsize_callback,
      (ui_callback_data_t)128 },
    { "256kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_GEORAMsize_callback,
      (ui_callback_data_t)256 },
    { "512kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_GEORAMsize_callback,
      (ui_callback_data_t)512 },
    { "1024kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_GEORAMsize_callback,
      (ui_callback_data_t)1024 },
    { "2048kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_GEORAMsize_callback,
      (ui_callback_data_t)2048 },
    { "4096kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_GEORAMsize_callback,
      (ui_callback_data_t)4096 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("RAM image"),
    { "Image file",
      MENU_ENTRY_DIALOG,
      file_string_GEORAMfilename_callback,
      (ui_callback_data_t)"Select " CARTRIDGE_NAME_GEORAM " image" },
    { "Save image on detach",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_GEORAMImageWrite_callback,
      NULL },
    { "Save image now",
      MENU_ENTRY_OTHER,
      c64_cart_flush_callback,
      (ui_callback_data_t)CARTRIDGE_GEORAM },
    { "Save image as",
      MENU_ENTRY_OTHER,
      c64_cart_save_callback,
      (ui_callback_data_t)CARTRIDGE_GEORAM },
    SDL_MENU_LIST_END
};


/* MMC64 */

UI_MENU_DEFINE_RADIO(MMC64ClockPort)

static ui_menu_entry_t mmc64_clockport_device_menu[CLOCKPORT_MAX_ENTRIES + 1];

UI_MENU_DEFINE_RADIO(MMC64_sd_type)

static const ui_menu_entry_t mmc64_sd_type_menu[] = {
    { "Auto",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MMC64_sd_type_callback,
      (ui_callback_data_t)0 },
    { "MMC",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MMC64_sd_type_callback,
      (ui_callback_data_t)1 },
    { "SD",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MMC64_sd_type_callback,
      (ui_callback_data_t)2 },
    { "SDHC",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MMC64_sd_type_callback,
      (ui_callback_data_t)3 },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_TOGGLE(MMC64)
UI_MENU_DEFINE_RADIO(MMC64_revision)
UI_MENU_DEFINE_TOGGLE(MMC64_flashjumper)
UI_MENU_DEFINE_TOGGLE(MMC64_bios_write)
UI_MENU_DEFINE_FILE_STRING(MMC64BIOSfilename)
UI_MENU_DEFINE_TOGGLE(MMC64_RO)
UI_MENU_DEFINE_FILE_STRING(MMC64imagefilename)

static const ui_menu_entry_t mmc64_cart_menu[] = {
    { "Enable " CARTRIDGE_NAME_MMC64,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_MMC64_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Revision"),
    { "Rev A",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MMC64_revision_callback,
      (ui_callback_data_t)0 },
    { "Rev B",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MMC64_revision_callback,
      (ui_callback_data_t)1 },
    SDL_MENU_ITEM_SEPARATOR,
    { "BIOS flash jumper",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_MMC64_flashjumper_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("BIOS image"),
    { "BIOS image file",
      MENU_ENTRY_DIALOG,
      file_string_MMC64BIOSfilename_callback,
      (ui_callback_data_t)"Select " CARTRIDGE_NAME_MMC64 " BIOS image" },
    { "Save image on detach",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_MMC64_bios_write_callback,
      NULL },
    { "Save image now",
      MENU_ENTRY_OTHER,
      c64_cart_flush_callback,
      (ui_callback_data_t)CARTRIDGE_MMC64 },
    { "Save image as",
      MENU_ENTRY_OTHER,
      c64_cart_save_callback,
      (ui_callback_data_t)CARTRIDGE_MMC64 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("MMC/SD image"),
    { "MMC/SD image file",
      MENU_ENTRY_DIALOG,
      file_string_MMC64imagefilename_callback,
      (ui_callback_data_t)"Select MMC/SD image" },
    { "Image read-only",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_MMC64_RO_callback,
      NULL },
    { "Card type",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)mmc64_sd_type_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Clockport device",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mmc64_clockport_device_menu },
    SDL_MENU_LIST_END
};


/* MMC Replay */

UI_MENU_DEFINE_RADIO(MMCRClockPort)

static ui_menu_entry_t mmcreplay_clockport_device_menu[CLOCKPORT_MAX_ENTRIES + 1];

UI_MENU_DEFINE_RADIO(MMCRSDType)

static const ui_menu_entry_t mmcreplay_sd_type_menu[] = {
    { "Auto",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MMCRSDType_callback,
      (ui_callback_data_t)0 },
    { "MMC",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MMCRSDType_callback,
      (ui_callback_data_t)1 },
    { "SD",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MMCRSDType_callback,
      (ui_callback_data_t)2 },
    { "SDHC",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MMCRSDType_callback,
      (ui_callback_data_t)3 },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_FILE_STRING(MMCRCardImage)
UI_MENU_DEFINE_FILE_STRING(MMCREEPROMImage)
UI_MENU_DEFINE_TOGGLE(MMCRCardRW)
UI_MENU_DEFINE_TOGGLE(MMCREEPROMRW)
UI_MENU_DEFINE_TOGGLE(MMCRRescueMode)
UI_MENU_DEFINE_TOGGLE(MMCRImageWrite)

static const ui_menu_entry_t mmcreplay_cart_menu[] = {
    { "Rescue mode",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_MMCRRescueMode_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("BIOS image"),
    { "Save image on detach",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_MMCRImageWrite_callback,
      NULL },
    { "Save image now",
      MENU_ENTRY_OTHER,
      c64_cart_flush_callback,
      (ui_callback_data_t)CARTRIDGE_MMC_REPLAY },
    { "Save image as",
      MENU_ENTRY_OTHER,
      c64_cart_save_callback,
      (ui_callback_data_t)CARTRIDGE_MMC_REPLAY },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("EEPROM image"),
    { "EEPROM image file",
      MENU_ENTRY_DIALOG,
      file_string_MMCREEPROMImage_callback,
      (ui_callback_data_t)"Select " CARTRIDGE_NAME_MMC_REPLAY " EEPROM image" },
    { "Enable writes to EEPROM image",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_MMCREEPROMRW_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("MMC/SD image"),
    { "Card image file",
      MENU_ENTRY_DIALOG,
      file_string_MMCRCardImage_callback,
      (ui_callback_data_t)"Select card image" },
    { "Enable writes to card image",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_MMCRCardRW_callback,
      NULL },
    { "Card type",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)mmcreplay_sd_type_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Clockport device",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mmcreplay_clockport_device_menu },
    SDL_MENU_LIST_END
};


/* Retro Replay */

UI_MENU_DEFINE_RADIO(RRClockPort)

static ui_menu_entry_t retroreplay_clockport_device_menu[CLOCKPORT_MAX_ENTRIES + 1];

UI_MENU_DEFINE_TOGGLE(RRBankJumper)
UI_MENU_DEFINE_RADIO(RRRevision)
UI_MENU_DEFINE_TOGGLE(RRFlashJumper)
UI_MENU_DEFINE_TOGGLE(RRBiosWrite)

static const ui_menu_entry_t retroreplay_revision_menu[] = {
    { "Retro Replay",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RRRevision_callback,
      (ui_callback_data_t)0 },
    { "Nordic Replay",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RRRevision_callback,
      (ui_callback_data_t)1 },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t retroreplay_cart_menu[] = {
    { "Revision",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)retroreplay_revision_menu },
    { "Bank jumper",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_RRBankJumper_callback,
      NULL },
    { "BIOS flash jumper",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_RRFlashJumper_callback,
      NULL },
    { "Save image on detach",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_RRBiosWrite_callback,
      NULL },
    { "Save image now",
      MENU_ENTRY_OTHER,
      c64_cart_flush_callback,
      (ui_callback_data_t)CARTRIDGE_RETRO_REPLAY },
    { "Save image as",
      MENU_ENTRY_OTHER,
      c64_cart_save_callback,
      (ui_callback_data_t)CARTRIDGE_RETRO_REPLAY },
    SDL_MENU_ITEM_SEPARATOR,
    { "Clockport device",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)retroreplay_clockport_device_menu },
    SDL_MENU_LIST_END
};


/* GMod2 */

UI_MENU_DEFINE_FILE_STRING(GMod2EEPROMImage)
UI_MENU_DEFINE_TOGGLE(GMOD2EEPROMRW)
UI_MENU_DEFINE_TOGGLE(GMod2FlashWrite)

static const ui_menu_entry_t gmod2_cart_menu[] = {
    SDL_MENU_ITEM_TITLE("EEPROM image"),
    { "EEPROM image file",
      MENU_ENTRY_DIALOG,
      file_string_GMod2EEPROMImage_callback,
      (ui_callback_data_t)"Select " CARTRIDGE_NAME_GMOD2 " EEPROM image" },
    { "Enable writes to EEPROM image",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_GMOD2EEPROMRW_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Flash image"),
    { "Enable writes to flash image",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_GMod2FlashWrite_callback,
      NULL },
    SDL_MENU_LIST_END
};

/* RRNET MK3 */
UI_MENU_DEFINE_TOGGLE(RRNETMK3_flashjumper)
UI_MENU_DEFINE_TOGGLE(RRNETMK3_bios_write)

static const ui_menu_entry_t rrnet_mk3_cart_menu[] = {
    { "BIOS flash jumper",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_RRNETMK3_flashjumper_callback,
      NULL },
    { "Save image on detach",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_RRNETMK3_bios_write_callback,
      NULL },
    { "Save image now",
      MENU_ENTRY_OTHER,
      c64_cart_flush_callback,
      (ui_callback_data_t)CARTRIDGE_RRNETMK3 },
    { "Save image as",
      MENU_ENTRY_OTHER,
      c64_cart_save_callback,
      (ui_callback_data_t)CARTRIDGE_RRNETMK3 },
    SDL_MENU_LIST_END
};


/* Magic Voice */

UI_MENU_DEFINE_TOGGLE(MagicVoiceCartridgeEnabled)
UI_MENU_DEFINE_FILE_STRING(MagicVoiceImage)

static const ui_menu_entry_t magicvoice_cart_menu[] = {
    { "Enable " CARTRIDGE_NAME_MAGIC_VOICE,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_MagicVoiceCartridgeEnabled_callback,
      NULL },
    { "ROM image file",
      MENU_ENTRY_DIALOG,
      file_string_MagicVoiceImage_callback,
      (ui_callback_data_t)"Select " CARTRIDGE_NAME_MAGIC_VOICE " ROM image" },
    SDL_MENU_LIST_END
};


/* SFX Sound Expander */

UI_MENU_DEFINE_TOGGLE(SFXSoundExpander)
UI_MENU_DEFINE_RADIO(SFXSoundExpanderChip)

static const ui_menu_entry_t soundexpander_menu[] = {
    { "Enable " CARTRIDGE_NAME_SFX_SOUND_EXPANDER,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SFXSoundExpander_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("YM chip type"),
    { "3526",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SFXSoundExpanderChip_callback,
      (ui_callback_data_t)3526 },
    { "3812",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SFXSoundExpanderChip_callback,
      (ui_callback_data_t)3812 },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_RADIO(IOCollisionHandling)

static const ui_menu_entry_t iocollision_menu[] = {
    { "Detach all",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_IOCollisionHandling_callback,
      (ui_callback_data_t)IO_COLLISION_METHOD_DETACH_ALL },
    { "Detach last",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_IOCollisionHandling_callback,
      (ui_callback_data_t)IO_COLLISION_METHOD_DETACH_LAST },
    { "AND values",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_IOCollisionHandling_callback,
      (ui_callback_data_t)IO_COLLISION_METHOD_AND_WIRES },
    SDL_MENU_LIST_END
};

static UI_MENU_CALLBACK(iocollision_show_type_callback)
{
    int type;

    resources_get_int("IOCollisionHandling", &type);
    switch (type) {
        case IO_COLLISION_METHOD_DETACH_ALL:
            return MENU_SUBMENU_STRING " detach all";
            break;
        case IO_COLLISION_METHOD_DETACH_LAST:
            return MENU_SUBMENU_STRING " detach last";
            break;
        case IO_COLLISION_METHOD_AND_WIRES:
            return MENU_SUBMENU_STRING " AND values";
            break;
    }
    return "n/a";
}

/* Cartridge menu */

UI_MENU_DEFINE_TOGGLE(CartridgeReset)
UI_MENU_DEFINE_TOGGLE(SFXSoundSampler)
UI_MENU_DEFINE_TOGGLE(CPMCart)
UI_MENU_DEFINE_TOGGLE(SSRamExpansion)

ui_menu_entry_t c64cart_menu[] = {
    { "Attach CRT image",
      MENU_ENTRY_DIALOG,
      attach_c64_cart_callback,
      (ui_callback_data_t)CARTRIDGE_CRT },
    /* CAUTION: the position of this item is hardcoded above */
    { "Attach raw image",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      NULL },
    { "Detach cartridge image",
      MENU_ENTRY_OTHER,
      detach_c64_cart_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Cartridge freeze",
      MENU_ENTRY_OTHER,
      c64_cart_freeze_callback,
      NULL },
    { "Set current cartridge as default",
      MENU_ENTRY_OTHER,
      set_c64_cart_default_callback,
      NULL },
    { "I/O collision handling ($D000-$DFFF)",
      MENU_ENTRY_SUBMENU,
      iocollision_show_type_callback,
      (ui_callback_data_t)iocollision_menu },
    { "Reset on cartridge change",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CartridgeReset_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Cartridge specific settings"),
    { CARTRIDGE_NAME_RAMCART,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ramcart_menu },
    { CARTRIDGE_NAME_REU,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)reu_menu },
    { CARTRIDGE_NAME_GEORAM,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)georam_menu },
    { CARTRIDGE_NAME_IDE64,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ide64_menu },
    { CARTRIDGE_NAME_EXPERT,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)expert_cart_menu },
    { CARTRIDGE_NAME_ISEPIC,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)isepic_cart_menu },
    { CARTRIDGE_NAME_DQBB,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)dqbb_cart_menu },
    { CARTRIDGE_NAME_EASYFLASH,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)easyflash_cart_menu },
    { CARTRIDGE_NAME_MMC64,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mmc64_cart_menu },
    { CARTRIDGE_NAME_MMC_REPLAY,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mmcreplay_cart_menu },
    { CARTRIDGE_NAME_RETRO_REPLAY,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)retroreplay_cart_menu },
    { CARTRIDGE_NAME_GMOD2,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)gmod2_cart_menu },
    { CARTRIDGE_NAME_RRNETMK3,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)rrnet_mk3_cart_menu },
    { CARTRIDGE_NAME_MAGIC_VOICE,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)magicvoice_cart_menu },
    { CARTRIDGE_NAME_SFX_SOUND_EXPANDER " settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)soundexpander_menu },
    { CARTRIDGE_NAME_SFX_SOUND_SAMPLER,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SFXSoundSampler_callback,
      NULL },
    { "CP/M Cartridge",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CPMCart_callback,
      NULL },
    { "Super Snapshot 32k RAM",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SSRamExpansion_callback,
      NULL },
    SDL_MENU_LIST_END
};

ui_menu_entry_t c128cart_menu[] = {
    { "Attach CRT image",
      MENU_ENTRY_DIALOG,
      attach_c64_cart_callback,
      (ui_callback_data_t)CARTRIDGE_CRT },
    /* CAUTION: the position of this item is hardcoded above */
    { "Attach raw image",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      NULL },
    { "Detach cartridge image",
      MENU_ENTRY_OTHER,
      detach_c64_cart_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Cartridge freeze",
      MENU_ENTRY_OTHER,
      c64_cart_freeze_callback,
      NULL },
    { "Set current cartridge as default",
      MENU_ENTRY_OTHER,
      set_c64_cart_default_callback,
      NULL },
    { "I/O collision handling ($D000-$DFFF)",
      MENU_ENTRY_SUBMENU,
      iocollision_show_type_callback,
      (ui_callback_data_t)iocollision_menu },
    { "Reset on cartridge change",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CartridgeReset_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Cartridge specific settings"),
    { CARTRIDGE_NAME_RAMCART,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ramcart_menu },
    { CARTRIDGE_NAME_REU,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)reu_menu },
    { CARTRIDGE_NAME_GEORAM,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)georam_menu },
    { CARTRIDGE_NAME_IDE64,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ide64_menu },
    { CARTRIDGE_NAME_EXPERT,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)expert_cart_menu },
    { CARTRIDGE_NAME_ISEPIC,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)isepic_cart_menu },
    { CARTRIDGE_NAME_DQBB,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)dqbb_cart_menu },
    { CARTRIDGE_NAME_EASYFLASH,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)easyflash_cart_menu },
    { CARTRIDGE_NAME_MMC64,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mmc64_cart_menu },
    { CARTRIDGE_NAME_MMC_REPLAY,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mmcreplay_cart_menu },
    { CARTRIDGE_NAME_RETRO_REPLAY,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)retroreplay_cart_menu },
    { CARTRIDGE_NAME_GMOD2,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)gmod2_cart_menu },
    { CARTRIDGE_NAME_MAGIC_VOICE,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)magicvoice_cart_menu },
    { CARTRIDGE_NAME_SFX_SOUND_EXPANDER " settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)soundexpander_menu },
    { CARTRIDGE_NAME_SFX_SOUND_SAMPLER,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SFXSoundSampler_callback,
      NULL },
    SDL_MENU_LIST_END
};

ui_menu_entry_t scpu64cart_menu[] = {
    { "Attach CRT image",
      MENU_ENTRY_DIALOG,
      attach_c64_cart_callback,
      (ui_callback_data_t)CARTRIDGE_CRT },
    /* CAUTION: the position of this item is hardcoded above */
    { "Attach raw image",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      NULL },
    { "Detach cartridge image",
      MENU_ENTRY_OTHER,
      detach_c64_cart_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Set current cartridge as default",
      MENU_ENTRY_OTHER,
      set_c64_cart_default_callback,
      NULL },
    { "I/O collision handling ($D000-$DFFF)",
      MENU_ENTRY_SUBMENU,
      iocollision_show_type_callback,
      (ui_callback_data_t)iocollision_menu },
    { "Reset on cartridge change",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CartridgeReset_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Cartridge specific settings"),
    { CARTRIDGE_NAME_RAMCART,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ramcart_menu },
    { CARTRIDGE_NAME_REU,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)reu_menu },
    { CARTRIDGE_NAME_GEORAM,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)georam_menu },
    { CARTRIDGE_NAME_IDE64,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ide64_menu },
    { CARTRIDGE_NAME_EXPERT,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)expert_cart_menu },
    { CARTRIDGE_NAME_ISEPIC,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)isepic_cart_menu },
    { CARTRIDGE_NAME_DQBB,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)dqbb_cart_menu },
    { CARTRIDGE_NAME_EASYFLASH,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)easyflash_cart_menu },
    { CARTRIDGE_NAME_MMC64,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mmc64_cart_menu },
    { CARTRIDGE_NAME_MMC_REPLAY,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mmcreplay_cart_menu },
    { CARTRIDGE_NAME_RETRO_REPLAY,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)retroreplay_cart_menu },
    { CARTRIDGE_NAME_GMOD2,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)gmod2_cart_menu },
    { CARTRIDGE_NAME_MAGIC_VOICE,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)magicvoice_cart_menu },
    { CARTRIDGE_NAME_SFX_SOUND_EXPANDER " settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)soundexpander_menu },
    { CARTRIDGE_NAME_SFX_SOUND_SAMPLER,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SFXSoundSampler_callback,
      NULL },
    SDL_MENU_LIST_END
};

void uiclockport_rr_mmc_menu_create(void)
{
    int i;

    for (i = 0; clockport_supported_devices[i].name; ++i) {
        mmc64_clockport_device_menu[i].string = clockport_supported_devices[i].name;
        mmc64_clockport_device_menu[i].type = MENU_ENTRY_RESOURCE_RADIO;
        mmc64_clockport_device_menu[i].callback = radio_MMC64ClockPort_callback;
        mmc64_clockport_device_menu[i].data = (ui_callback_data_t)int_to_void_ptr(clockport_supported_devices[i].id);

        mmcreplay_clockport_device_menu[i].string = clockport_supported_devices[i].name;
        mmcreplay_clockport_device_menu[i].type = MENU_ENTRY_RESOURCE_RADIO;
        mmcreplay_clockport_device_menu[i].callback = radio_MMCRClockPort_callback;
        mmcreplay_clockport_device_menu[i].data = (ui_callback_data_t)int_to_void_ptr(clockport_supported_devices[i].id);

        retroreplay_clockport_device_menu[i].string = clockport_supported_devices[i].name;
        retroreplay_clockport_device_menu[i].type = MENU_ENTRY_RESOURCE_RADIO;
        retroreplay_clockport_device_menu[i].callback = radio_RRClockPort_callback;
        retroreplay_clockport_device_menu[i].data = (ui_callback_data_t)int_to_void_ptr(clockport_supported_devices[i].id);
    }

    mmc64_clockport_device_menu[i].string = NULL;
    mmc64_clockport_device_menu[i].type = MENU_ENTRY_TEXT;
    mmc64_clockport_device_menu[i].callback = NULL;
    mmc64_clockport_device_menu[i].data = NULL;

    mmcreplay_clockport_device_menu[i].string = NULL;
    mmcreplay_clockport_device_menu[i].type = MENU_ENTRY_TEXT;
    mmcreplay_clockport_device_menu[i].callback = NULL;
    mmcreplay_clockport_device_menu[i].data = NULL;

    retroreplay_clockport_device_menu[i].string = NULL;
    retroreplay_clockport_device_menu[i].type = MENU_ENTRY_TEXT;
    retroreplay_clockport_device_menu[i].callback = NULL;
    retroreplay_clockport_device_menu[i].data = NULL;
}
