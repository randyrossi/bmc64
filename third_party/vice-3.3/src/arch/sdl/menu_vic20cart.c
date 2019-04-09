/*
 * menu_vic20cart.c - Implementation of the vic20 cartridge settings menu for the SDL UI.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cartridge.h"
#include "cartio.h"
#include "keyboard.h"
#include "lib.h"
#include "menu_common.h"
#include "menu_vic20cart.h"
#include "resources.h"
#include "ui.h"
#include "uifilereq.h"
#include "uimenu.h"

static UI_MENU_CALLBACK(attach_cart_callback)
{
    char *title;
    char *name = NULL;

    if (activated) {
        switch (vice_ptr_to_int(param)) {
            case CARTRIDGE_VIC20_BEHRBONZ:
                title = "Select " CARTRIDGE_VIC20_NAME_BEHRBONZ " image";
                break;
            case CARTRIDGE_VIC20_UM:
                title = "Select " CARTRIDGE_VIC20_NAME_UM " image";
                break;
            case CARTRIDGE_VIC20_FP:
                title = "Select " CARTRIDGE_VIC20_NAME_FP " image";
                break;
            case CARTRIDGE_VIC20_MEGACART:
                title = "Select " CARTRIDGE_VIC20_NAME_MEGACART " image";
                break;
            case CARTRIDGE_VIC20_FINAL_EXPANSION:
                title = "Select " CARTRIDGE_VIC20_NAME_FINAL_EXPANSION " image";
                break;
            case CARTRIDGE_VIC20_DETECT:
            case CARTRIDGE_VIC20_GENERIC:
                title = "Select cartridge image";
                break;
            case CARTRIDGE_VIC20_16KB_2000:
            case CARTRIDGE_VIC20_16KB_4000:
            case CARTRIDGE_VIC20_16KB_6000:
                title = "Select 4/8/16kB image";
                break;
            case CARTRIDGE_VIC20_8KB_A000:
                title = "Select 4/8kB image";
                break;
            case CARTRIDGE_VIC20_4KB_B000:
            default:
                title = "Select 4kB image";
                break;
        }
        name = sdl_ui_file_selection_dialog(title, FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            if (cartridge_attach_image(vice_ptr_to_int(param), name) < 0) {
                ui_error("Cannot load cartridge image.");
            }
            lib_free(name);
        }
    }
    return NULL;
}

static const ui_menu_entry_t add_to_generic_cart_submenu[] = {
    { "Smart-attach cartridge image",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_VIC20_DETECT },
    { "Attach 4/8/16kB image at $2000",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_VIC20_16KB_2000 },
    { "Attach 4/8/16kB image at $4000",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_VIC20_16KB_4000 },
    { "Attach 4/8/16kB image at $6000",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_VIC20_16KB_6000 },
    { "Attach 4/8kB image at $A000",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_VIC20_8KB_A000 },
    { "Attach 4kB image at $B000",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_VIC20_4KB_B000 },
    SDL_MENU_LIST_END
};

static UI_MENU_CALLBACK(detach_cart_callback)
{
    if (activated) {
        cartridge_detach_image(-1);
    }
    return NULL;
}

static UI_MENU_CALLBACK(set_cart_default_callback)
{
    if (activated) {
        cartridge_set_default();
    }
    return NULL;
}

/* GEORAM */

UI_MENU_DEFINE_TOGGLE(GEORAM)
UI_MENU_DEFINE_TOGGLE(GEORAMIOSwap)
UI_MENU_DEFINE_RADIO(GEORAMsize)
UI_MENU_DEFINE_FILE_STRING(GEORAMfilename)
UI_MENU_DEFINE_TOGGLE(GEORAMImageWrite)

static const ui_menu_entry_t georam_menu[] = {
    { "Enable " CARTRIDGE_NAME_GEORAM,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_GEORAM_callback,
      NULL },
    { "Swap " CARTRIDGE_NAME_GEORAM " I/O",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_GEORAMIOSwap_callback,
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
    SDL_MENU_LIST_END
};


/* SFX Sound Expander */

UI_MENU_DEFINE_TOGGLE(SFXSoundExpander)
UI_MENU_DEFINE_TOGGLE(SFXSoundExpanderIOSwap)
UI_MENU_DEFINE_RADIO(SFXSoundExpanderChip)

static const ui_menu_entry_t soundexpander_menu[] = {
    { "Enable " CARTRIDGE_NAME_SFX_SOUND_EXPANDER,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SFXSoundExpander_callback,
      NULL },
    { "Swap " CARTRIDGE_NAME_SFX_SOUND_EXPANDER " I/O",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SFXSoundExpanderIOSwap_callback,
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


/* SFX Sound Sampler */

UI_MENU_DEFINE_TOGGLE(SFXSoundSampler)
UI_MENU_DEFINE_TOGGLE(SFXSoundSamplerIOSwap)

static const ui_menu_entry_t soundsampler_menu[] = {
    { "Enable " CARTRIDGE_NAME_SFX_SOUND_SAMPLER,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SFXSoundSampler_callback,
      NULL },
    { "Swap " CARTRIDGE_NAME_SFX_SOUND_SAMPLER " I/O",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SFXSoundSamplerIOSwap_callback,
      NULL },
    SDL_MENU_LIST_END
};


/* DIGIMAX MENU */

UI_MENU_DEFINE_TOGGLE(DIGIMAX)
UI_MENU_DEFINE_RADIO(DIGIMAXbase)

static const ui_menu_entry_t digimax_vic20_menu[] = {
    { "Enable " CARTRIDGE_NAME_DIGIMAX,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DIGIMAX_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Base address"),
    { "$9800",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x9800 },
    { "$9820",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x9820 },
    { "$9840",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x9840 },
    { "$9860",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x9860 },
    { "$9880",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x9880 },
    { "$98A0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x98a0 },
    { "$98C0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x98c0 },
    { "$98E0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x98e0 },
    { "$9C00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x9c00 },
    { "$9C20",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x9c20 },
    { "$9C40",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x9c40 },
    { "$9C60",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x9c60 },
    { "$9C80",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x9c80 },
    { "$9CA0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x9ca0 },
    { "$9CC0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x9cc0 },
    { "$9CE0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DIGIMAXbase_callback,
      (ui_callback_data_t)0x9ce0 },
    SDL_MENU_LIST_END
};


/* DS12C887 RTC MENU */

UI_MENU_DEFINE_TOGGLE(DS12C887RTC)
UI_MENU_DEFINE_TOGGLE(DS12C887RTCRunMode)
UI_MENU_DEFINE_RADIO(DS12C887RTCbase)
UI_MENU_DEFINE_TOGGLE(DS12C887RTCSave)

static const ui_menu_entry_t ds12c887rtc_vic20_menu[] = {
    { "Enable " CARTRIDGE_NAME_DS12C887RTC,
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DS12C887RTC_callback,
      NULL },
    { "Start with running oscillator",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DS12C887RTCRunMode_callback,
      NULL },
    { "Save RTC data when changed",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_DS12C887RTCSave_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Base address"),
    { "$9800",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DS12C887RTCbase_callback,
      (ui_callback_data_t)0x9800 },
    { "$9C00",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DS12C887RTCbase_callback,
      (ui_callback_data_t)0x9c00 },
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

UI_MENU_DEFINE_TOGGLE(CartridgeReset)
UI_MENU_DEFINE_TOGGLE(FinalExpansionWriteBack)
UI_MENU_DEFINE_TOGGLE(UltiMemWriteBack)
UI_MENU_DEFINE_TOGGLE(VicFlashPluginWriteBack)
UI_MENU_DEFINE_TOGGLE(MegaCartNvRAMWriteBack)
UI_MENU_DEFINE_FILE_STRING(MegaCartNvRAMfilename)

UI_MENU_DEFINE_TOGGLE(IO2RAM)
UI_MENU_DEFINE_TOGGLE(IO3RAM)

const ui_menu_entry_t vic20cart_menu[] = {
    { "Attach generic cartridge image",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_VIC20_GENERIC },
    { "Attach " CARTRIDGE_VIC20_NAME_BEHRBONZ " image",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_VIC20_BEHRBONZ },
    { "Attach " CARTRIDGE_VIC20_NAME_MEGACART " image",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_VIC20_MEGACART },
    { "Attach " CARTRIDGE_VIC20_NAME_FINAL_EXPANSION " image",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_VIC20_FINAL_EXPANSION },
    { "Attach " CARTRIDGE_VIC20_NAME_UM " image",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_VIC20_UM },
    { "Attach " CARTRIDGE_VIC20_NAME_FP " image",
      MENU_ENTRY_DIALOG,
      attach_cart_callback,
      (ui_callback_data_t)CARTRIDGE_VIC20_FP },
    SDL_MENU_ITEM_SEPARATOR,
    { "Add to generic cartridge",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)add_to_generic_cart_submenu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Detach cartridge image",
      MENU_ENTRY_OTHER,
      detach_cart_callback,
      NULL },
    { "Set current cartridge as default",
      MENU_ENTRY_OTHER,
      set_cart_default_callback,
      NULL },
    { "I/O collision handling ($9000-$93FF / $9800-$9FFF)",
      MENU_ENTRY_SUBMENU,
      iocollision_show_type_callback,
      (ui_callback_data_t)iocollision_menu },
    { "Reset on cartridge change",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CartridgeReset_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { CARTRIDGE_VIC20_NAME_FINAL_EXPANSION " write back",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_FinalExpansionWriteBack_callback,
      NULL },
    { CARTRIDGE_VIC20_NAME_UM " write back",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UltiMemWriteBack_callback,
      NULL },
    { CARTRIDGE_VIC20_NAME_FP " write back",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VicFlashPluginWriteBack_callback,
      NULL },
    { CARTRIDGE_VIC20_NAME_MEGACART " NvRAM write back",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_MegaCartNvRAMWriteBack_callback,
      NULL },
    { CARTRIDGE_VIC20_NAME_MEGACART " NvRAM file",
      MENU_ENTRY_DIALOG,
      file_string_MegaCartNvRAMfilename_callback,
      (ui_callback_data_t)"Select " CARTRIDGE_VIC20_NAME_MEGACART " NvRAM image" },
    { "I/O-2 RAM",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IO2RAM_callback,
      NULL },
    { "I/O-3 RAM",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IO3RAM_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("MasC=uerade specific cart settings"),
    { CARTRIDGE_NAME_GEORAM,
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)georam_menu },
    { CARTRIDGE_NAME_SFX_SOUND_EXPANDER " settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)soundexpander_menu },
    { CARTRIDGE_NAME_SFX_SOUND_SAMPLER " settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)soundsampler_menu },
    { CARTRIDGE_NAME_DIGIMAX " settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)digimax_vic20_menu },
    { CARTRIDGE_NAME_DS12C887RTC " settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ds12c887rtc_vic20_menu },
    SDL_MENU_LIST_END
};
