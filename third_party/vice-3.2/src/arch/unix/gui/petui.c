/*
 * petui.c - Implementation of the PET-specific part of the UI.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andre Fachat <a.fachat@physik.tu-chemnitz.de>
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
#include <string.h>

#include "cartio.h"
#include "debug.h"
#include "icon.h"
#include "keyboard.h"
#include "lib.h"
#include "machine.h"
#include "machine-video.h"
#include "pet-resources.h"
#include "petmodel.h"
#include "pets.h"
#include "petui.h"
#include "resources.h"
#include "uiapi.h"
#include "uiattach.h"
#include "uicommands.h"
#include "uicrtc.h"
#include "uidatasette.h"
#include "uidrive.h"
#include "uidrivepetcbm2.h"
#include "uiedit.h"
#include "uijoyport.h"
#include "uijoystick2.h"
#include "uikeyboard.h"
#include "uimenu.h"
#include "uimouse.h"
#include "uinetplay.h"
#include "uiperipheralieee.h"
#include "uipetcolour.h"
#include "uipetdww.h"
#include "uipethre.h"
#include "uipetreu.h"
#include "uiprinterieee.h"
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
#include "uiuserport.h"
#include "vsync.h"

#include "petui.h"

UI_MENU_DEFINE_TOGGLE(DiagPin)
UI_MENU_DEFINE_TOGGLE(Crtc)
UI_MENU_DEFINE_TOGGLE(Ram9)
UI_MENU_DEFINE_TOGGLE(RamA)
UI_MENU_DEFINE_RADIO(VideoSize)
UI_MENU_DEFINE_RADIO(RamSize)
UI_MENU_DEFINE_RADIO(IOSize)
UI_MENU_DEFINE_TOGGLE(Basic1)
UI_MENU_DEFINE_TOGGLE(Basic1Chars)
UI_MENU_DEFINE_TOGGLE(EoiBlank)
UI_MENU_DEFINE_RADIO(KeyboardType)

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
    { "$8F00", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidAddress, (ui_callback_data_t)0x8f00, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$E900", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidAddress, (ui_callback_data_t)0xe900, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_RADIO(SidClock)

static ui_menu_entry_t sidcart_clock_submenu[] = {
    { "C64", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_SidClock, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "PET", UI_MENU_TYPE_TICK,
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

static ui_menu_entry_t pet_memsize_submenu[] = {
    { "4 kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RamSize, (ui_callback_data_t)4, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "8 kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RamSize, (ui_callback_data_t)8, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "16 kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RamSize, (ui_callback_data_t)16, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "32 kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RamSize, (ui_callback_data_t)32, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "96 kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RamSize, (ui_callback_data_t)96, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "128 kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RamSize, (ui_callback_data_t)128, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t pet_iosize_submenu[] = {
    { "2 kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_IOSize, (ui_callback_data_t)0x800, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "256 B", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_IOSize, (ui_callback_data_t)0x100, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t pet_video_submenu[] = {
    { N_("Auto (from ROM)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_VideoSize, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("40 Columns"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_VideoSize, (ui_callback_data_t)40, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("80 Columns"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_VideoSize, (ui_callback_data_t)80, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static UI_CALLBACK(radio_petmodel)
{
    int model, selected;

    selected = vice_ptr_to_int(UI_MENU_CB_PARAM);

    if (!CHECK_MENUS) {
        petmodel_set(selected);
        ui_update_menus();
    } else {
        model = petmodel_get();

        if (selected == model) {
            ui_menu_set_tick(w, 1);
        } else {
            ui_menu_set_tick(w, 0);
        }
    }
}

static ui_menu_entry_t model_defaults_submenu[] = {
    { "PET 2001-8N", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_petmodel, (ui_callback_data_t)PETMODEL_2001, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "PET 3008", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_petmodel, (ui_callback_data_t)PETMODEL_3008, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "PET 3016", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_petmodel, (ui_callback_data_t)PETMODEL_3016, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "PET 3032", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_petmodel, (ui_callback_data_t)PETMODEL_3032, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "PET 3032B", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_petmodel, (ui_callback_data_t)PETMODEL_3032B, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "PET 4016", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_petmodel, (ui_callback_data_t)PETMODEL_4016, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "PET 4032", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_petmodel, (ui_callback_data_t)PETMODEL_4032, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "PET 4032B", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_petmodel, (ui_callback_data_t)PETMODEL_4032B, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "PET 8032", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_petmodel, (ui_callback_data_t)PETMODEL_8032, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "PET 8096", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_petmodel, (ui_callback_data_t)PETMODEL_8096, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "PET 8296", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_petmodel, (ui_callback_data_t)PETMODEL_8296, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "SuperPET", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_petmodel, (ui_callback_data_t)PETMODEL_SUPERPET, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_STRING_RADIO(ChargenName)

static ui_menu_entry_t petui_main_romset_submenu[] = {
    { N_("Load new kernal ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"KernalName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new editor ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"EditorName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new BASIC ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"BasicName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t io_extensions_submenu[] = {
    { N_("PET RAM and Expansion Unit"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, petreu_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("PET Colour graphics"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, petcolour_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("PET DWW hi-res graphics"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, petdww_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("PET HRE hi-res graphics"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, pethre_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("SID cartridge"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sidcart_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Userport devices", UI_MENU_TYPE_NORMAL,
      NULL, NULL, userport_pet_vic20_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Tape port devices", UI_MENU_TYPE_NORMAL,
      NULL, NULL, tapeport_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("PET userport diagnostic pin"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_DiagPin, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("I/O collision handling ($8800-$8FFF / $E900-$EEFF)"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, iocollision_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t pet_romset_submenu[] = {
    { N_("Basic 1"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"rom1g.vrs", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Basic 2, graphics"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"rom2g.vrs", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Basic 2, Business"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"rom2b.vrs", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Basic 4, 40 cols, graphics"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"rom4g40.vrs", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Basic 4, 40 cols, business"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"rom4b40.vrs", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Basic 4, 80 cols, business"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"rom4b80.vrs", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Basic 1 Patch (if loaded)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Basic1, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Basic 1 character set"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Basic1Chars, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Load new computer ROM"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, petui_main_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new drive ROM"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_drivepetcbm2_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Load new character ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"ChargenName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Original character set"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_ChargenName, (ui_callback_data_t)"chargen", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("German character set"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_ChargenName, (ui_callback_data_t)"chargen.de", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Load new $9*** ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"RomModule9Name", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Unload $9*** ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_unload_rom_file, (ui_callback_data_t)"RomModule9Name", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Load new $A*** ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"RomModuleAName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Unload $A*** ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_unload_rom_file, (ui_callback_data_t)"RomModuleAName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Load new $B*** ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"RomModuleBName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Unload $B*** ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_unload_rom_file, (ui_callback_data_t)"RomModuleBName", NULL,
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

UI_MENU_DEFINE_TOGGLE(SuperPET)
UI_MENU_DEFINE_RADIO(CPUswitch)

static ui_menu_entry_t superpet_cpuswitch_submenu[] = {
    { "6502", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_CPUswitch, (ui_callback_data_t)SUPERPET_CPU_6502, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "6809", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_CPUswitch, (ui_callback_data_t)SUPERPET_CPU_6809, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Prog", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_CPUswitch, (ui_callback_data_t)SUPERPET_CPU_PROG, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t superpet_6809_roms_submenu[] = {
    { N_("Load new $A*** ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"H6809RomAName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new $B*** ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"H6809RomBName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new $C*** ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"H6809RomCName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new $D*** ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"H6809RomDName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new $E*** ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"H6809RomEName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new $F*** ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"H6809RomFName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t model_settings_submenu[] = {
    { N_("PET model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, model_defaults_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
      /* Do not change position as position 2 is hard coded. */
    { N_("Keyboard type"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Video size"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, pet_video_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Memory size"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, pet_memsize_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("I/O size"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, pet_iosize_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("CRTC chip enable"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Crtc, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("EOI blanks screen"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_EoiBlank, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("SuperPET I/O enable (disables 8x96)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_SuperPET, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    { N_("SuperPET ACIA"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uirs232petplus4cbm2_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { N_("SuperPET CPU Switch"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, superpet_cpuswitch_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("SuperPET 6809 ROMs"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, superpet_6809_roms_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("$9*** as RAM (8296 only)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Ram9, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("$A*** as RAM (8296 only)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_RamA, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

static ui_menu_entry_t pet_menu[] = {
    { N_("Model settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, model_settings_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("RAM reset pattern"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_ram_pattern_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("ROM settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, pet_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("CRTC settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, crtc_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("I/O extensions"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, io_extensions_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

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

static ui_menu_entry_t petui_left_menu[] = {
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

static ui_menu_entry_t petui_tape_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, uiattach_tape_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, datasette_control_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t petui_file_menu[] = {
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
static ui_menu_entry_t petui_edit_submenu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_edit_commands_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
#endif

static ui_menu_entry_t petui_snapshot_menu[] = {
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

static ui_menu_entry_t petui_settings_menu[] = {
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
      NULL, NULL, ui_drivepetcbm2_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Printer settings"), UI_MENU_TYPE_NORMAL, 
      NULL, NULL, printerieee_cbm2_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable Virtual Devices"), UI_MENU_TYPE_TICK, 
      (ui_callback_t)toggle_VirtualDevices, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_joyport_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, joystick_settings_pet_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_MOUSE
    { N_("Mouse emulation"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, mouse_grab_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, pet_menu,
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

static ui_menu_entry_t petui_top_menu[] = {
    { N_("File"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, petui_file_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef USE_GNOMEUI
    { N_("Edit"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, petui_edit_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { N_("Snapshot"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, petui_snapshot_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, petui_settings_menu,
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

static ui_menu_entry_t petui_speed_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_performance_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_runmode_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static void uipetkeyboard_menu_create(void)
{
    unsigned int i, num;
    ui_menu_entry_t *keyboard_layouttype_submenu;
    kbdtype_info_t *list;

    num = machine_get_num_keyboard_types();

    if (num == 0) {
        return;
    }

    keyboard_layouttype_submenu = lib_calloc((size_t)(num + 1), sizeof(ui_menu_entry_t));
    list = machine_get_keyboard_info_list();

    for (i = 0; i < num ; i++) {
        keyboard_layouttype_submenu[i].string = (ui_callback_data_t)lib_msprintf("%s", list->name);
        keyboard_layouttype_submenu[i].type = UI_MENU_TYPE_TICK;
        keyboard_layouttype_submenu[i].callback = (ui_callback_t)radio_KeyboardType;
        keyboard_layouttype_submenu[i].callback_data = (ui_callback_data_t)uint_to_void_ptr(list->type);
        ++list;
    }

    model_settings_submenu[2].sub_menu = keyboard_layouttype_submenu;
}

static void uipetkeyboard_menu_shutdown(void)
{
    unsigned int i;
    ui_menu_entry_t *keyboard_layouttype_submenu = NULL;

    keyboard_layouttype_submenu = model_settings_submenu[2].sub_menu;

    if (keyboard_layouttype_submenu == NULL) {
        return;
    }

    model_settings_submenu[2].sub_menu = NULL;

    i = 0;

    while (keyboard_layouttype_submenu[i].string != NULL) {
        lib_free(keyboard_layouttype_submenu[i].string);
        i++;
    }

    lib_free(keyboard_layouttype_submenu);
}

static void petui_dynamic_menu_create(void)
{
    uisound_menu_create();
    uisampler_menu_create();
    uicrtc_menu_create();
    uikeyboard_menu_create();
    uipetkeyboard_menu_create();
    uijoyport_menu_create(0, 0, 1, 1, 0);
    uisid_model_menu_create();
}

static void petui_dynamic_menu_shutdown(void)
{
    uicrtc_menu_shutdown();
    uisound_menu_shutdown();
    uisampler_menu_shutdown();
    uikeyboard_menu_shutdown();
    uipetkeyboard_menu_shutdown();
    uijoyport_menu_shutdown();
    uisid_model_menu_shutdown();
}

int petui_init(void)
{
    ui_set_application_icon(pet_icon_data);
    petui_dynamic_menu_create();
    ui_set_left_menu(petui_left_menu);

    ui_set_right_menu(petui_settings_menu);

    ui_set_tape_menu(petui_tape_menu);
    ui_set_topmenu(petui_top_menu);
    ui_set_speedmenu(petui_speed_menu);

    ui_set_drop_callback(uiattach_autostart_file);

    ui_update_menus();

    return 0;
}

void petui_shutdown(void)
{
    petui_dynamic_menu_shutdown();
}
