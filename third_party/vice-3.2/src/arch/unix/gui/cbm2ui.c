/*
 * cbm2ui.c - Implementation of the CBM-II specific part of the UI.
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
#include "cbm2.h"
#include "cbm2mem.h"
#include "cbm2model.h"
#include "cbm2ui.h"
#include "debug.h"
#include "icon.h"
#include "machine.h"
#include "machine-video.h"
#include "resources.h"
#include "uiapi.h"
#include "uiattach.h"
#include "uicommands.h"
#include "uicbm2cart.h"
#include "uicrtc.h"
#include "uidatasette.h"
#include "uidrive.h"
#include "uidrivepetcbm2.h"
#include "uiedit.h"
#include "uijoyport.h"
#include "uijoystick2.h"
#include "uikeyboard.h"
#include "uimouse.h"
#include "uiperipheralieee.h"
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
#include "uimenu.h"
#include "uinetplay.h"
#include "uitapeport.h"
#include "uiuserport.h"
#include "uivicii.h"
#include "vsync.h"

UI_MENU_DEFINE_RADIO(MachineVideoStandard)

static ui_menu_entry_t set_viciimodel_submenu[] = {
    { "PAL-G", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MachineVideoStandard, (ui_callback_data_t)MACHINE_SYNC_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "NTSC-M", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_MachineVideoStandard, (ui_callback_data_t)MACHINE_SYNC_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_TOGGLE(CartridgeReset)

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

static ui_menu_entry_t io_extensions_cbm2_submenu[] = {
    { "Userport devices", UI_MENU_TYPE_NORMAL,
      NULL, NULL, userport_c64_cbm2_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Tape port devices", UI_MENU_TYPE_NORMAL,
      NULL, NULL, tapeport_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("I/O collision handling ($D800-$DFFF)"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, iocollision_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Reset on cart change"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_CartridgeReset, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t io_extensions_cbm5x0_submenu[] = {
    { "Tape port devices", UI_MENU_TYPE_NORMAL,
      NULL, NULL, tapeport_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("I/O collision handling ($D800-$DFFF)"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, iocollision_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Reset on cart change"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_CartridgeReset, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

static ui_menu_entry_t cbm2ui_main_romset_submenu[] = {
    { N_("Load new kernal ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"KernalName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new character ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"ChargenName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new BASIC ROM"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"BasicName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t cbm5x0_romset_submenu[] = {
    { N_("Load default ROMs"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"rom500.vrs", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Load new computer ROM"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm2ui_main_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new drive ROM"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_drivepetcbm2_romset_submenu,
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

static ui_menu_entry_t cbm6x0_romset_submenu[] = {
    { N_("Basic 128k, low chars"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"rom128l.vrs", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Basic 256k, low chars"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"rom256l.vrs", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Basic 128k, high chars"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"rom128h.vrs", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Basic 256k, high chars"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"rom256h.vrs", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Load new computer ROM"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm2ui_main_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new drive ROM"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_drivepetcbm2_romset_submenu,
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

UI_MENU_DEFINE_RADIO(ModelLine)

static ui_menu_entry_t cbm6x0_modelline_submenu[] = {
    { "7x0 (50 Hz)", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_ModelLine, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "6x0 60 Hz", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_ModelLine, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "6x0 50 Hz", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_ModelLine, (ui_callback_data_t)2, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t cbm5x0_modelline_submenu[] = {
    { "6x0 60 Hz", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_ModelLine, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "6x0 50 Hz", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_ModelLine, (ui_callback_data_t)2, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_RADIO(RamSize)

static ui_menu_entry_t cbm2_memsize_submenu[] = {
    { "64 kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RamSize, (ui_callback_data_t)64, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "128 kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RamSize, (ui_callback_data_t)128, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "256 kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RamSize, (ui_callback_data_t)256, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "512 kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RamSize, (ui_callback_data_t)512, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "1024 kB", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_RamSize, (ui_callback_data_t)1024, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static UI_CALLBACK(radio_cbm2model)
{
    int model, selected;

    selected = vice_ptr_to_int(UI_MENU_CB_PARAM);

    if (!CHECK_MENUS) {
        cbm2model_set(selected);
        ui_update_menus();
    } else {
        model = cbm2model_get();

        if (selected == model) {
            ui_menu_set_tick(w, 1);
        } else {
            ui_menu_set_tick(w, 0);
        }
    }
}

static ui_menu_entry_t cbm5x0_model_defaults_submenu[] = {
    { "CBM 510 PAL", UI_MENU_TYPE_TICK, (ui_callback_t)radio_cbm2model,
      (ui_callback_data_t)CBM2MODEL_510_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "CBM 510 NTSC", UI_MENU_TYPE_TICK, (ui_callback_t)radio_cbm2model,
      (ui_callback_data_t)CBM2MODEL_510_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t cbm6x0_model_defaults_submenu[] = {
    { "CBM 610 PAL", UI_MENU_TYPE_TICK, (ui_callback_t)radio_cbm2model,
      (ui_callback_data_t)CBM2MODEL_610_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "CBM 610 NTSC", UI_MENU_TYPE_TICK, (ui_callback_t)radio_cbm2model,
      (ui_callback_data_t)CBM2MODEL_610_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "CBM 620 PAL", UI_MENU_TYPE_TICK, (ui_callback_t)radio_cbm2model,
      (ui_callback_data_t)CBM2MODEL_620_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "CBM 620 NTSC", UI_MENU_TYPE_TICK, (ui_callback_t)radio_cbm2model,
      (ui_callback_data_t)CBM2MODEL_620_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "CBM 620+ (1M) PAL", UI_MENU_TYPE_TICK, (ui_callback_t)radio_cbm2model,
      (ui_callback_data_t)CBM2MODEL_620PLUS_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "CBM 620+ (1M) NTSC", UI_MENU_TYPE_TICK, (ui_callback_t)radio_cbm2model,
      (ui_callback_data_t)CBM2MODEL_620PLUS_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "CBM 710 NTSC", UI_MENU_TYPE_TICK, (ui_callback_t)radio_cbm2model,
      (ui_callback_data_t)CBM2MODEL_710_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "CBM 720 NTSC", UI_MENU_TYPE_TICK, (ui_callback_t)radio_cbm2model,
      (ui_callback_data_t)CBM2MODEL_720_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "CBM 720+ (1M) NTSC", UI_MENU_TYPE_TICK, (ui_callback_t)radio_cbm2model,
      (ui_callback_data_t)CBM2MODEL_720PLUS_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

#if 0

/* this is partially modeled after the radio_* callbacks */
static UI_CALLBACK(set_KeyboardType)
{
    int current_value, new_value = 2 * (int)UI_MENU_CB_PARAM;
    extern char *keymap_file_resource_names[];

    resources_get_int("KeymapIndex", &current_value);

    if (!call_data) {
        if ((current_value & ~1) != new_value) {
            resources_set_int("KeymapIndex", (current_value & 1) + new_value);
            ui_update_menus();
        }
    } else {
        ui_menu_set_tick(w, current_value == new_value);
    }
}

static ui_menu_entry_t cbm2_keybd_submenu[] = {
    { N_("Graphics"), UI_MENU_TYPE_TICK,
      (ui_callback_t)set_KeyboardType, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Business (UK)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)set_KeyboardType, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

#endif

UI_MENU_DEFINE_RADIO(CIA1Model)

static ui_menu_entry_t set_cia1model_submenu[] = {
    { N_("6526 (old)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_CIA1Model, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("8521 (new)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_CIA1Model, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_TOGGLE(Ram08)
UI_MENU_DEFINE_TOGGLE(Ram1)
UI_MENU_DEFINE_TOGGLE(Ram2)
UI_MENU_DEFINE_TOGGLE(Ram4)
UI_MENU_DEFINE_TOGGLE(Ram6)
UI_MENU_DEFINE_TOGGLE(RamC)

static ui_menu_entry_t cbm5x0_model_settings_submenu[] = {
    { N_("Model defaults"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm5x0_model_defaults_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("VIC-II model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_viciimodel_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("SID model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("CIA model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_cia1model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Memory size"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm2_memsize_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Hardwired switches"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm5x0_modelline_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Bank 15 $0800-$0FFF RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Ram08, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Bank 15 $1000-$1FFF RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Ram1, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Bank 15 $2000-$3FFF RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Ram2, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Bank 15 $4000-$5FFF RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Ram4, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Bank 15 $6000-$7FFF RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Ram6, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Bank 15 $C000-$CFFF RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_RamC, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#if 0
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Keyboard type"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm2_keybd_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t cbm6x0_model_settings_submenu[] = {
    { N_("Model defaults"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm6x0_model_defaults_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("SID model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("CIA 1 model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_cia1model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Memory size"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm2_memsize_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Hardwired switches"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm6x0_modelline_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Bank 15 $0800-$0FFF RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Ram08, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Bank 15 $1000-$1FFF RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Ram1, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Bank 15 $2000-$3FFF RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Ram2, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Bank 15 $4000-$5FFF RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Ram4, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Bank 15 $6000-$7FFF RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_Ram6, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Bank 15 $C000-$CFFF RAM"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_RamC, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#if 0
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Keyboard type"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm2_keybd_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

UI_MENU_DEFINE_TOGGLE(SidFilters)

static ui_menu_entry_t sid_submenu[] = {
    { N_("SID model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("SID filters"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_SidFilters, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_RESID
    UI_MENU_ENTRY_SEPERATOR,
    { N_("reSID sampling method"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_resid_sampling_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#if !defined(USE_GNOMEUI)
    { N_("reSID resampling passband"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_sid_resid_passband, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
#endif
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

static ui_menu_entry_t cbm6x0_menu[] = {
    { N_("Model settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm6x0_model_settings_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("RAM reset pattern"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_ram_pattern_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("ROM settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm6x0_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("CRTC settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, crtc_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("SID settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("I/O extensions"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, io_extensions_cbm2_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    { N_("RS232 settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uirs232petplus4cbm2_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t cbm5x0_menu[] = {
    { N_("Model settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm5x0_model_settings_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("RAM settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_ram_pattern_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("ROM settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm5x0_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("VIC-II settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, vicii_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("SID settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("I/O extensions"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, io_extensions_cbm5x0_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    { N_("RS232 settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uirs232petplus4cbm2_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_LIST_END
};

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

static ui_menu_entry_t cbm2_left_menu[] = {
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
      NULL, NULL, ui_cbm2cart_commands_menu,
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

static ui_menu_entry_t cbm2_tape_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL,  uiattach_tape_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, datasette_control_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t cbm2_file_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL,  uiattach_smart_attach_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, uiattach_disk_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, uiattach_tape_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL,  ui_datasette_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_cbm2cart_commands_menu,
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
static ui_menu_entry_t cbm2_edit_submenu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_edit_commands_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
#endif

static ui_menu_entry_t cbm2_snapshot_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL,  ui_snapshot_commands_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_screenshot_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL,  ui_sound_record_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_TOGGLE(VirtualDevices)

static ui_menu_entry_t cbm5x0_settings_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_performance_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_runmode_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL,  uikeyboard_settings_menu,
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
      NULL, NULL, printerieee_cbm5x0_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Enable Virtual Devices"), UI_MENU_TYPE_TICK, 
      (ui_callback_t)toggle_VirtualDevices, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_joyport_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, joystick_settings_cbm5x0_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_MOUSE
    { N_("Mouse emulation"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, mouse_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, cbm5x0_menu,
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

static ui_menu_entry_t cbm6x0_settings_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_performance_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_runmode_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL,  uikeyboard_settings_menu,
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
      NULL, NULL, cbm6x0_menu,
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

static ui_menu_entry_t cbm5x0_top_menu[] = {
    { N_("File"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm2_file_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef USE_GNOMEUI
    { N_("Edit"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm2_edit_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { N_("Snapshot"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm2_snapshot_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm5x0_settings_menu,
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

static ui_menu_entry_t cbm6x0_top_menu[] = {
    { N_("File"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm2_file_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef USE_GNOMEUI
    { N_("Edit"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm2_edit_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { N_("Snapshot"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm2_snapshot_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, cbm6x0_settings_menu,
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

static ui_menu_entry_t cbm2_speed_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_performance_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_runmode_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

static void cbm2ui_dynamic_menu_create(void)
{
    uisound_menu_create();
    uisampler_menu_create();
    if (machine_class == VICE_MACHINE_CBM5x0) {
        uivicii_menu_create();
        uijoyport_menu_create(1, 1, 0, 0, 0);
    } else {
        uicrtc_menu_create();
        uijoyport_menu_create(0, 0, 1, 1, 0);
    }
    uikeyboard_menu_create();
    uisid_model_menu_create();
    uicbm2cart_menu_create();
}

static void cbm2ui_dynamic_menu_shutdown(void)
{
    if (machine_class == VICE_MACHINE_CBM5x0) {
        uivicii_menu_shutdown();
    } else {
        uicrtc_menu_shutdown();
    }
    uisound_menu_shutdown();
    uisampler_menu_shutdown();
    uikeyboard_menu_shutdown();
    uijoyport_menu_shutdown();
    uisid_model_menu_shutdown();
    uicbm2cart_menu_shutdown();
}

int cbm2ui_init(void)
{
    ui_set_application_icon(cbm2_icon_data);
    cbm2ui_dynamic_menu_create();
    ui_set_left_menu(cbm2_left_menu);

    ui_set_right_menu(cbm6x0_settings_menu);
    ui_set_topmenu(cbm6x0_top_menu);

    ui_set_tape_menu(cbm2_tape_menu);
    ui_set_speedmenu(cbm2_speed_menu);
    ui_update_menus();

    return 0;
}

void cbm2ui_shutdown(void)
{
    cbm2ui_dynamic_menu_shutdown();
}

/* ------------------------------------------------------------------------- */

int cbm5x0ui_init(void)
{
    ui_set_application_icon(cbm2_icon_data);
    cbm2ui_dynamic_menu_create();
    ui_set_left_menu(cbm2_left_menu);

    ui_set_right_menu(cbm5x0_settings_menu);
    ui_set_topmenu(cbm5x0_top_menu);

    ui_set_tape_menu(cbm2_tape_menu);
    ui_set_speedmenu(cbm2_speed_menu);

    ui_set_drop_callback(uiattach_autostart_file);

    ui_update_menus();

    return 0;
}

void cbm5x0ui_shutdown(void)
{
    cbm2ui_dynamic_menu_shutdown();
}
