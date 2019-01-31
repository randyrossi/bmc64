/*
 * vic20ui.c - Implementation of the VIC20-specific part of the UI.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later vers;on.
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

#include "cartio.h"
#include "cartridge.h"
#include "debug.h"
#include "icon.h"
#include "joy.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "machine-video.h"
#include "resources.h"
#include "uiapi.h"
#include "uiattach.h"
#include "uicommands.h"
#ifdef HAVE_RAWNET
#include "uics8900.h"
#endif
#include "uidatasette.h"
#include "uidigimax.h"
#include "uidrive.h"
#include "uidrivec64vic20.h"
#include "uidrivevic20.h"
#include "uids12c887rtc.h"
#include "uiedit.h"

#ifdef HAVE_RAWNET
#include "uiethernetcart.h"
#endif

#include "uigeoram.h"
#include "uijoyport.h"
#include "uijoystick2.h"
#include "uikeyboard.h"
#include "uilib.h"
#include "uimenu.h"
#include "uimidi.h"
#include "uimouse.h"
#include "uinetplay.h"
#include "uiperipheraliec.h"
#include "uiprinteriec.h"
#include "uiram.h"
#include "uiromset.h"

#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
#include "uirs232c64c128.h"
#endif

#include "uisampler.h"
#include "uiscreenshot.h"
#include "uisettings.h"
#include "uisid.h"
#include "uisound.h"
#include "uisoundexpander.h"
#include "uisoundsampler.h"
#include "uitapeport.h"
#include "uiuserport.h"
#include "uivic.h"
#include "util.h"
#include "vic20model.h"
#include "vsync.h"

#include "vic20ui.h"


/* description would be nice */
enum {
    MEM_NONE,
    MEM_ALL,
    MEM_3K,
    MEM_8K,
    MEM_16K,
    MEM_24K
};


/* description would be nice */
enum {
    BLOCK_0 = 1,
    BLOCK_1 = 1 << 1,
    BLOCK_2 = 1 << 2,
    BLOCK_3 = 1 << 3,
    BLOCK_5 = 1 << 5
};

static UI_CALLBACK(set_common_memory_configuration)
{
    int blocks = 0;

    switch (vice_ptr_to_int(UI_MENU_CB_PARAM)) {
        case MEM_ALL:
            blocks = BLOCK_0 | BLOCK_1 | BLOCK_2 | BLOCK_3 | BLOCK_5;
            break;
        case MEM_3K:
            blocks = BLOCK_0;
            break;
        case MEM_8K:
            blocks = BLOCK_1;
            break;
        case MEM_16K:
            blocks = BLOCK_1 | BLOCK_2;
            break;
        case MEM_24K:
            blocks = BLOCK_1 | BLOCK_2 | BLOCK_3;
            break;
        default:
            /* Shouldn't happen.  */
            log_error(LOG_DEFAULT, "vic20ui:set_common_memory_configuration bogus memory config.");
        case MEM_NONE:
            blocks = 0;
            break;
    }
    resources_set_int("RamBlock0", blocks & BLOCK_0 ? 1 : 0);
    resources_set_int("RamBlock1", blocks & BLOCK_1 ? 1 : 0);
    resources_set_int("RamBlock2", blocks & BLOCK_2 ? 1 : 0);
    resources_set_int("RamBlock3", blocks & BLOCK_3 ? 1 : 0);
    resources_set_int("RamBlock5", blocks & BLOCK_5 ? 1 : 0);
    ui_menu_update_all();
    vsync_suspend_speed_eval();
}

UI_MENU_DEFINE_TOGGLE(SidCart)
UI_MENU_DEFINE_TOGGLE(SidFilters)

UI_MENU_DEFINE_RADIO(SidAddress)

static ui_menu_entry_t sidcart_address_submenu[] = {
    { "$9800", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidAddress, (ui_callback_data_t)0x9800, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$9C00", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidAddress, (ui_callback_data_t)0x9c00, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_RADIO(SidClock)

static ui_menu_entry_t sidcart_clock_submenu[] = {
    { "C64", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidClock, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "VIC20", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidClock, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t sidcart_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_SidCart, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("SID model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("SID filters"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_SidFilters, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("SID address"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sidcart_address_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    /* Translators: "SID clock" as in "CPU Frequency" */
    { N_("SID clock"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sidcart_clock_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_RESID
    { N_("ReSID sampling method"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_resid_sampling_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vic20ui_main_romset_submenu[] = {
    { N_("Load new kernal ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"KernalName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new BASIC ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"BasicName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new character ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"ChargenName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vic20_romset_submenu[] = {
    { N_("Load default ROMs"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"default.vrs", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Load new computer ROM"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, vic20ui_main_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new drive ROM"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_drivec64vic20_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("ROM set archive"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uiromset_archive_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("ROM set file"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uiromset_file_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t common_memory_configurations_submenu[] = {
    { N_("No expansion memory"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t) set_common_memory_configuration, (ui_callback_data_t)MEM_NONE, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("3K (block 0)"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t) set_common_memory_configuration, (ui_callback_data_t)MEM_3K, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("8K (block 1)"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t) set_common_memory_configuration, (ui_callback_data_t)MEM_8K, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("16K (blocks 1/2)"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t) set_common_memory_configuration, (ui_callback_data_t)MEM_16K, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("24K (blocks 1/2/3)"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t) set_common_memory_configuration, (ui_callback_data_t)MEM_24K, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("All (blocks 0/1/2/3/5)"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t) set_common_memory_configuration, (ui_callback_data_t)MEM_ALL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_TOGGLE(RAMBlock0)
UI_MENU_DEFINE_TOGGLE(RAMBlock1)
UI_MENU_DEFINE_TOGGLE(RAMBlock2)
UI_MENU_DEFINE_TOGGLE(RAMBlock3)
UI_MENU_DEFINE_TOGGLE(RAMBlock5)

static ui_menu_entry_t memory_settings_submenu[] = {
    { N_("Common configurations"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, common_memory_configurations_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Block 0 (3K at $0400-$0FFF)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_RAMBlock0, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Block 1 (8K at $2000-$3FFF)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_RAMBlock1, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Block 2 (8K at $4000-$5FFF)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_RAMBlock2, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Block 3 (8K at $6000-$7FFF)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_RAMBlock3, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Block 5 (8K at $A000-$BFFF)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_RAMBlock5, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

static char *last_dir;

static UI_CALLBACK(attach_cartridge)
{
    int type = vice_ptr_to_int(UI_MENU_CB_PARAM);
    char *filename;
    ui_button_t button;
    uilib_file_filter_enum_t filter[] = { UILIB_FILTER_VIC20CART, UILIB_FILTER_ALL };

    vsync_suspend_speed_eval();
    filename = ui_select_file(_("Attach cartridge image"), NULL, 0, last_dir, filter, sizeof(filter) / sizeof(*filter), &button, 0, NULL, UI_FC_LOAD);
    switch (button) {
        case UI_BUTTON_OK:
            if (cartridge_attach_image(type, filename) < 0) {
                ui_error(_("Invalid cartridge image"));
            }
            lib_free(last_dir);
            util_fname_split(filename, &last_dir, NULL);
            ui_update_menus();
            break;
        default:
            /* Do nothing special.  */
            break;
    }
    lib_free(filename);
}

static ui_menu_entry_t add_to_generic_cart_submenu[] = {
    { N_("Smart-attach cartridge image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_VIC20_DETECT, NULL,
      KEYSYM_c, UI_HOTMOD_META },
    { N_("Attach 4/8/16KB cartridge image at $2000"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_VIC20_16KB_2000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Attach 4/8/16KB cartridge image at $4000"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_VIC20_16KB_4000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Attach 4/8/16KB cartridge image at $6000"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_VIC20_16KB_6000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Attach 4/8KB cartridge image at $A000"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_VIC20_8KB_A000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Attach 4KB cartridge image at $B000"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_VIC20_4KB_B000, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static UI_CALLBACK(detach_cartridge)
{
    cartridge_detach_image(-1);
}

static UI_CALLBACK(default_cartridge)
{
    cartridge_set_default();
}

static ui_menu_entry_t attach_cartridge_image_submenu[] = {
    { N_("Attach generic image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_VIC20_GENERIC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Attach Behr Bonz image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_VIC20_BEHRBONZ, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Attach Mega-Cart image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_VIC20_MEGACART, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Attach Final Expansion image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_VIC20_FINAL_EXPANSION, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Attach UltiMem image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_VIC20_UM, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Attach Vic Flash Plugin image"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)attach_cartridge, (ui_callback_data_t)CARTRIDGE_VIC20_FP, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Add to generic cartridge"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, add_to_generic_cart_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Set cartridge as default"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)default_cartridge, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_TOGGLE(CartridgeReset)
UI_MENU_DEFINE_TOGGLE(FinalExpansionWriteBack)
UI_MENU_DEFINE_TOGGLE(VicFlashPluginWriteBack)
UI_MENU_DEFINE_TOGGLE(UltiMemWriteBack)
UI_MENU_DEFINE_TOGGLE(MegaCartNvRAMWriteBack)

UI_MENU_DEFINE_TOGGLE(IO2RAM)
UI_MENU_DEFINE_TOGGLE(IO3RAM)

static UI_CALLBACK(set_mc_nvram_image_name)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM, _("Mega-Cart NvRAM image"), UILIB_FILTER_ALL);
}

static ui_menu_entry_t fexp_submenu[] = {
    { N_("Enable image write back"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_FinalExpansionWriteBack, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vicflash_submenu[] = {
    { N_("Enable image write back"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_VicFlashPluginWriteBack, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t ultimem_submenu[] = {
    { N_("Enable image write back"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_UltiMemWriteBack, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t megacart_submenu[] = {
    { N_("Enable NvRAM image write back"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_MegaCartNvRAMWriteBack, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("NvRAM image file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_mc_nvram_image_name, (ui_callback_data_t)"MegaCartNvRAMfilename", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t io_ram_submenu[] = {
    { N_("Enable I/O-2 RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_IO2RAM, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable I/O-3 RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_IO3RAM, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vic20_cartridge_commands_menu[] = {
    { N_("Attach cartridge image"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, attach_cartridge_image_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Detach cartridge image(s)"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)detach_cartridge, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/*------------------------------------------------------------*/

static UI_CALLBACK(save_screenshot)
{
    /* Where does the 1024 come from?  */
    char filename[1024];
    unsigned int wid = vice_ptr_to_uint(UI_MENU_CB_PARAM);

    vsync_suspend_speed_eval();

    /* The following code depends on a zeroed filename.  */
    memset(filename, 0, 1024);

    if (ui_screenshot_dialog(filename, machine_video_canvas_get(wid)) < 0) {
        return;
    }
}

static ui_menu_entry_t ui_screenshot_commands_menu[] = {
    { N_("Save media file"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)save_screenshot, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_RADIO(MachineVideoStandard)

static UI_CALLBACK(radio_vic20model)
{
    int model, selected;

    selected = vice_ptr_to_int(UI_MENU_CB_PARAM);

    if (!CHECK_MENUS) {
        vic20model_set(selected);
        ui_update_menus();
    } else {
        model = vic20model_get();

        if (selected == model) {
            ui_menu_set_tick(w, 1);
        } else {
            ui_menu_set_tick(w, 0);
        }
    }
}

static ui_menu_entry_t set_vic20_model_submenu[] = {
    { "VIC20 PAL", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_vic20model, (ui_callback_data_t)VIC20MODEL_VIC20_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "VIC20 NTSC", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_vic20model, (ui_callback_data_t)VIC20MODEL_VIC20_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "VIC21", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_vic20model, (ui_callback_data_t)VIC20MODEL_VIC21, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t set_vic_model_submenu[] = {
    { "PAL-G", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MachineVideoStandard, (ui_callback_data_t)MACHINE_SYNC_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "NTSC-M", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MachineVideoStandard, (ui_callback_data_t)MACHINE_SYNC_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vic20_model_submenu[] = {
    { N_("Model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_vic20_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("VIC model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_vic_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Memory expansions"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, memory_settings_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_RADIO(IOCollisionHandling)

static ui_menu_entry_t iocollision_submenu[] = {
    { N_("detach all"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_IOCollisionHandling, (ui_callback_data_t)IO_COLLISION_METHOD_DETACH_ALL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("detach last"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_IOCollisionHandling, (ui_callback_data_t)IO_COLLISION_METHOD_DETACH_LAST, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("AND values"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_IOCollisionHandling, (ui_callback_data_t)IO_COLLISION_METHOD_AND_WIRES, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_TOGGLE(IEEE488)
UI_MENU_DEFINE_TOGGLE(VFLImod)

static ui_menu_entry_t io_extensions_submenu[] = {
    { CARTRIDGE_VIC20_NAME_MEGACART, UI_MENU_TYPE_NORMAL,
      NULL, NULL, megacart_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_VIC20_NAME_FINAL_EXPANSION, UI_MENU_TYPE_NORMAL,
      NULL, NULL, fexp_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_VIC20_NAME_FP, UI_MENU_TYPE_NORMAL,
      NULL, NULL, vicflash_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_VIC20_NAME_UM, UI_MENU_TYPE_NORMAL,
      NULL, NULL, ultimem_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("SID cartridge"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sidcart_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("VIC-1112 IEEE488 interface"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_IEEE488, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("I/O RAM"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, io_ram_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("VFLI modification"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_VFLImod, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { CARTRIDGE_NAME_DIGIMAX " (MasC=uerade)", UI_MENU_TYPE_NORMAL,
      NULL, NULL, digimax_vic20_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_DS12C887RTC " (MasC=uerade)", UI_MENU_TYPE_NORMAL,
      NULL, NULL, ds12c887rtc_vic20_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_GEORAM " (MasC=uerade)", UI_MENU_TYPE_NORMAL,
      NULL, NULL, georam_vic20_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_SFX_SOUND_EXPANDER " (MasC=uerade)", UI_MENU_TYPE_NORMAL,
      NULL, NULL, soundexpander_vic20_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_SFX_SOUND_SAMPLER " (MasC=uerade)", UI_MENU_TYPE_NORMAL,
      NULL, NULL, soundsampler_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_RAWNET
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Ethernet cartridge (MasC=uerade)"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ethernetcart_vic20_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_SEPERATOR,
#ifdef HAVE_MIDI
    { N_("MIDI emulation"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, midi_vic20_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { "Userport devices", UI_MENU_TYPE_NORMAL,
      NULL, NULL, userport_pet_vic20_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Tape port devices", UI_MENU_TYPE_NORMAL,
      NULL, NULL, tapeport_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("I/O collision handling ($9000-$93FF / $9800-$9FFF)"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, iocollision_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Reset on cart change"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_CartridgeReset, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

static ui_menu_entry_t vic20_menu[] = {
    { N_("Model settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, vic20_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("RAM reset pattern"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_ram_pattern_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("ROM settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, vic20_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("VIC settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, vic_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("I/O extensions"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, io_extensions_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    { N_("RS232 settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uirs232_vic20_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
#ifdef HAVE_RAWNET
    { N_("Ethernet settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uics8900_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vic20_left_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, uiattach_smart_attach_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, uiattach_disk_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, uiattach_tape_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_datasette_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, vic20_cartridge_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_directory_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_snapshot_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_screenshot_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_sound_record_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_edit_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_tool_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_NETWORK
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, netplay_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_run_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#if defined(USE_XAWUI)
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_help_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_exit_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vic20_tape_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, uiattach_tape_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, datasette_control_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vic20_file_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, uiattach_smart_attach_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, uiattach_disk_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, uiattach_tape_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_datasette_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, vic20_cartridge_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_directory_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_tool_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_NETWORK
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, netplay_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_run_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_exit_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

#ifdef USE_GNOMEUI
static ui_menu_entry_t vic20_edit_submenu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_edit_commands_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
#endif

static ui_menu_entry_t vic20_snapshot_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_snapshot_commands_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_screenshot_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_sound_record_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_TOGGLE(VirtualDevices)

static ui_menu_entry_t vic20_settings_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_performance_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_runmode_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, uikeyboard_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_sound_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_sampler_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_drivevic20_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Printer settings"), UI_MENU_TYPE_NORMAL, 
      NULL, NULL, printeriec_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable Virtual Devices"), UI_MENU_TYPE_TICK, 
      (ui_callback_t)toggle_VirtualDevices, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_joyport_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, joystick_settings_vic20_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_MOUSE
    { N_("Mouse emulation"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, mouse_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, vic20_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_settings_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef DEBUG
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_debug_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vic20_top_menu[] = {
    { N_("File"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, vic20_file_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef USE_GNOMEUI
    { N_("Edit"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, vic20_edit_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { N_("Snapshot"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, vic20_snapshot_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, vic20_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef DEBUG
    { N_("Debug"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, debug_settings_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    /* Translators: RJ means right justify and should be
        saved in your tranlation! e.g. german "RJHilfe" */
    { N_("RJHelp"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_help_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t vic20_speed_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_performance_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_runmode_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static void vic20ui_dynamic_menu_create(void)
{
    uisound_menu_create();
    uisampler_menu_create();
    uivic_menu_create();
    uikeyboard_menu_create();
    uijoyport_menu_create(1, 0, 1, 1, 0);
    uisid_model_menu_create();
#ifdef HAVE_RAWNET
    uiethernetcart_menu_create();
#endif
}

static void vic20ui_dynamic_menu_shutdown(void)
{
    uivic_menu_shutdown();
    uisound_menu_shutdown();
    uisampler_menu_shutdown();
    uikeyboard_menu_shutdown();
    uijoyport_menu_shutdown();
    uisid_model_menu_shutdown();
#ifdef HAVE_RAWNET
    uiethernetcart_menu_shutdown();
#endif
}

int vic20ui_init(void)
{
    ui_set_application_icon(vic20_icon_data);
    vic20ui_dynamic_menu_create();
    ui_set_left_menu(vic20_left_menu);

    ui_set_right_menu(vic20_settings_menu);

    ui_set_tape_menu(vic20_tape_menu);
    ui_set_topmenu(vic20_top_menu);
    ui_set_speedmenu(vic20_speed_menu);

    ui_set_drop_callback(uiattach_autostart_file);

    ui_update_menus();

    return 0;
}

void vic20ui_shutdown(void)
{
    if (last_dir != NULL) {
        lib_free(last_dir);
    }
    vic20ui_dynamic_menu_shutdown();
}
