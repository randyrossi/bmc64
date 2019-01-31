/*
 * plus4ui.c - Implementation of the Plus4-specific part of the UI.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#include "cartio.h"
#include "debug.h"
#include "icon.h"
#include "machine.h"
#include "machine-video.h"
#include "plus4memhacks.h"
#include "plus4model.h"
#include "plus4ui.h"
#include "resources.h"
#include "uiapi.h"
#include "uiattach.h"
#include "uicommands.h"
#include "uidatasette.h"
#include "uidrive.h"
#include "uidriveplus4.h"
#include "uiedit.h"
#include "uijoyport.h"
#include "uijoystick2.h"
#include "uikeyboard.h"
#include "uimenu.h"
#include "uimouse.h"
#include "uinetplay.h"
#include "uiperipheraliec.h"
#include "uiplus4cart.h"
#include "uiprinteriecplus4.h"
#include "uiram.h"
#include "uiromset.h"

#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
#include "uirs232petplus4cbm2.h"
#endif

#include "uisampler.h"
#include "uiscreenshot.h"
#include "uisettings.h"
#include "uisid.h"
#include "uisound.h"
#include "uitapeport.h"
#include "uited.h"
#include "uiuserport.h"
#include "uiv364speech.h"
#include "vsync.h"
#ifdef HAVE_OPENGL_SYNC
#include <stdlib.h>             /* strtol() */
#include "openGL_sync.h"
#include "lib.h"
#endif

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

/* ------------------------------------------------------------------------- */

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

UI_MENU_DEFINE_TOGGLE(SidCart)
UI_MENU_DEFINE_TOGGLE(SidFilters)

UI_MENU_DEFINE_RADIO(SidAddress)

static ui_menu_entry_t sidcart_address_submenu[] = {
    { "$FD40", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidAddress, (ui_callback_data_t)0xfd40, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$FE80", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidAddress, (ui_callback_data_t)0xfe80, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_RADIO(SidClock)

static ui_menu_entry_t sidcart_clock_submenu[] = {
    { "C64", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidClock, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "PLUS4", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidClock, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_TOGGLE(Acia1Enable)

static ui_menu_entry_t acia_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Acia1Enable, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_TOGGLE(DIGIBLASTER)

static ui_menu_entry_t digiblaster_submenu[] = {
    { N_("Enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DIGIBLASTER, NULL, NULL,
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

static ui_menu_entry_t plus4ui_main_romset_submenu[] = {
    { N_("Load new kernal ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"KernalName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new BASIC ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"BasicName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 3 plus 1 LO ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"FunctionLowName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new 3 plus 1 HI ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"FunctionHighName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new c1 LO ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"c1loName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new c1 HI ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"c1hiName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new c2 LO ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"c2loName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new c2 HI ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"c2hiName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t plus4_romset_submenu[] = {
    { N_("Load default ROMs"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"default.vrs", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Load new computer ROM"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, plus4ui_main_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new drive ROM"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_driveplus4_romset_submenu,
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

/* ------------------------------------------------------------------------- */

static UI_CALLBACK(radio_model)
{
    int model, selected;

    selected = vice_ptr_to_int(UI_MENU_CB_PARAM);

    if (!CHECK_MENUS) {
        plus4model_set(selected);
        ui_update_menus();
    } else {
        model = plus4model_get();

        if (selected == model) {
            ui_menu_set_tick(w, 1);
        } else {
            ui_menu_set_tick(w, 0);
        }
    }
}

static ui_menu_entry_t set_model_submenu[] = {
    { "C16/116 PAL", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_model, (ui_callback_data_t)PLUS4MODEL_C16_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "C16/116 NTSC", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_model, (ui_callback_data_t)PLUS4MODEL_C16_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Plus4 PAL", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_model, (ui_callback_data_t)PLUS4MODEL_PLUS4_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Plus4 NTSC", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_model, (ui_callback_data_t)PLUS4MODEL_PLUS4_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "V364 NTSC", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_model, (ui_callback_data_t)PLUS4MODEL_V364_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "C232 NTSC", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_model, (ui_callback_data_t)PLUS4MODEL_232_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_RADIO(RamSize)
UI_MENU_DEFINE_RADIO(MemoryHack)

static ui_menu_entry_t set_ram_submenu[] = {
    { "16kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RamSize, (ui_callback_data_t)16, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "32kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RamSize, (ui_callback_data_t)32, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "64kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RamSize, (ui_callback_data_t)64, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "256kB (CSORY)", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MemoryHack, (ui_callback_data_t)MEMORY_HACK_C256K, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "256kB (HANNES)", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MemoryHack, (ui_callback_data_t)MEMORY_HACK_H256K, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1024kB (HANNES)", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MemoryHack, (ui_callback_data_t)MEMORY_HACK_H1024K, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "4096kB (HANNES)", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MemoryHack, (ui_callback_data_t)MEMORY_HACK_H4096K, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

UI_MENU_DEFINE_TOGGLE(CartridgeReset)

static ui_menu_entry_t io_extensions_submenu[] = {
    { N_("ACIA"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, acia_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Digiblaster add-on"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, digiblaster_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("SID cartridge"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sidcart_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    /* Translators: "V364 Speech" is the speech extension present in the V364 prototype */
    { N_("V364 Speech"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, speech_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Userport devices", UI_MENU_TYPE_NORMAL,
      NULL, NULL, userport_plus4_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Tape port devices", UI_MENU_TYPE_NORMAL,
      NULL, NULL, tapeport_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("I/O collision handling ($FD00-$FEFF)"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, iocollision_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Reset on cart change"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_CartridgeReset, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};


UI_MENU_DEFINE_RADIO(MachineVideoStandard)

static ui_menu_entry_t set_ted_model_submenu[] = {
    { "PAL-G", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MachineVideoStandard, (ui_callback_data_t)MACHINE_SYNC_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "NTSC-M", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MachineVideoStandard, (ui_callback_data_t)MACHINE_SYNC_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t plus4_model_submenu[] = {
    { N_("Model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("TED model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_ted_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("RAM settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_ram_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

static ui_menu_entry_t plus4_menu[] = {
    { N_("Model settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, plus4_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("RAM reset pattern"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_ram_pattern_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("ROM settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, plus4_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("TED settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ted_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("I/O extensions"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, io_extensions_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    { N_("RS232 settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uirs232petplus4cbm2_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t plus4_left_menu[] = {
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

static ui_menu_entry_t plus4_tape_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, uiattach_tape_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, datasette_control_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t plus4_file_menu[] = {
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
      NULL, NULL, ui_plus4cart_commands_menu,
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
static ui_menu_entry_t plus4_edit_submenu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_edit_commands_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
#endif

static ui_menu_entry_t plus4_snapshot_menu[] = {
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

static ui_menu_entry_t plus4_settings_menu[] = {
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
      NULL, NULL, ui_driveplus4_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Printer settings"), UI_MENU_TYPE_NORMAL, 
      NULL, NULL, printeriec_plus4_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable Virtual Devices"), UI_MENU_TYPE_TICK, 
      (ui_callback_t)toggle_VirtualDevices, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_joyport_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, joystick_settings_plus4_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_MOUSE
    { N_("Mouse emulation"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, mouse_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, plus4_menu,
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

static ui_menu_entry_t plus4_top_menu[] = {
    { N_("File"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, plus4_file_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef USE_GNOMEUI
    { N_("Edit"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, plus4_edit_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { N_("Snapshot"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, plus4_snapshot_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, plus4_settings_menu,
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

static ui_menu_entry_t plus4_speed_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_performance_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_runmode_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static void plus4ui_dynamic_menu_create(void)
{
    uisound_menu_create();
    uisampler_menu_create();
    uited_menu_create();
    uikeyboard_menu_create();
    uijoyport_menu_create(1, 1, 1, 1, 1);
    uisid_model_menu_create();
}

static void plus4ui_dynamic_menu_shutdown(void)
{
    uited_menu_shutdown();
    uisound_menu_shutdown();
    uisampler_menu_shutdown();
    uikeyboard_menu_shutdown();
    uijoyport_menu_shutdown();
    uisid_model_menu_shutdown();
}

int plus4ui_init(void)
{
    ui_set_application_icon(plus4_icon_data);
    plus4ui_dynamic_menu_create();
    ui_set_left_menu(plus4_left_menu);

    ui_set_right_menu(plus4_settings_menu);

    ui_set_topmenu(plus4_top_menu);

    ui_set_speedmenu(plus4_speed_menu);
    ui_set_tape_menu(plus4_tape_menu);

    ui_set_drop_callback(uiattach_autostart_file);

    ui_update_menus();

    return 0;
}

void plus4ui_shutdown(void)
{
    plus4ui_dynamic_menu_shutdown();
}
