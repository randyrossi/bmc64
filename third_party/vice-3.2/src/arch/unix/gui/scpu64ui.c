/*
 * scpu64ui.c - Implementation of the SCPU64-specific part of the UI.
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
#include <string.h>

#include "c64model.h"
#include "cartio.h"
#include "cartridge.h"
#include "cia.h"
#include "debug.h"
#include "icon.h"
#include "machine.h"
#include "machine-video.h"
#include "resources.h"
#include "sid.h"
#include "uiapi.h"
#include "uiattach.h"
#include "uic64cart.h"
#include "uicommands.h"
#ifdef HAVE_RAWNET
#include "uics8900.h"
#endif
#include "uidigimax.h"
#include "uidqbb.h"
#include "uidrive.h"
#include "uidrivec64.h"
#include "uidrivec64c128.h"
#include "uidrivec64vic20.h"
#include "uids12c887rtc.h"
#include "uieasyflash.h"
#include "uiedit.h"

#ifdef HAVE_RAWNET
#include "uiethernetcart.h"
#endif

#include "uiexpert.h"
#include "uigeoram.h"
#include "uigmod2.h"
#include "uiide64.h"
#include "uiisepic.h"
#include "uijoyport.h"
#include "uijoystick2.h"
#include "uikeyboard.h"
#include "uimagicvoice.h"
#include "uimenu.h"
#include "uimidi.h"
#include "uimmc64.h"
#include "uimmcreplay.h"
#include "uimouse.h"
#include "uinetplay.h"
#include "uiperipheraliec.h"
#include "uiprinteriec.h"
#include "uiram.h"
#include "uiramcart.h"
#include "uiretroreplay.h"
#include "uireu.h"
#include "uirrnetmk3.h"
#include "uiromset.h"

#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
#include "uirs232c64c128.h"
#endif

#include "uisampler.h"
#include "uiscpu64.h"
#include "uiscreenshot.h"
#include "uisettings.h"
#include "uisid.h"
#include "uisound.h"
#include "uisoundexpander.h"
#include "uisupersnapshot.h"
#include "uitpi.h"
#include "uiuserport.h"
#include "uivicii.h"
#include "vsync.h"
#include "vicii.h"

#include "scpu64ui.h"


/* ------------------------------------------------------------------------- */

static UI_CALLBACK(radio_c64model)
{
    int model, selected;

    selected = vice_ptr_to_int(UI_MENU_CB_PARAM);

    if (!CHECK_MENUS) {
        c64model_set(selected);
        ui_update_menus();
    } else {
        model = c64model_get();

        if (selected == model) {
            ui_menu_set_tick(w, 1);
        } else {
            ui_menu_set_tick(w, 0);
        }
    }
}

static ui_menu_entry_t set_c64_model_submenu[] = {
    { "C64 PAL", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_c64model, (ui_callback_data_t)C64MODEL_C64_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "C64C PAL", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_c64model, (ui_callback_data_t)C64MODEL_C64C_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("C64 old PAL"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_c64model, (ui_callback_data_t)C64MODEL_C64_OLD_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "C64 NTSC", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_c64model, (ui_callback_data_t)C64MODEL_C64_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "C64C NTSC", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_c64model, (ui_callback_data_t)C64MODEL_C64C_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("C64 old NTSC"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_c64model, (ui_callback_data_t)C64MODEL_C64_OLD_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Drean", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_c64model, (ui_callback_data_t)C64MODEL_C64_PAL_N, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "C64 SX PAL", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_c64model, (ui_callback_data_t)C64MODEL_C64SX_PAL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "C64 SX NTSC", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_c64model, (ui_callback_data_t)C64MODEL_C64SX_NTSC, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Japanese", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_c64model, (ui_callback_data_t)C64MODEL_C64_JAP, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "C64 GS", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_c64model, (ui_callback_data_t)C64MODEL_C64_GS, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_RADIO(VICIIModel)

static ui_menu_entry_t set_vicii_model_submenu[] = {
    { "6569 (PAL)", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_VICIIModel, (ui_callback_data_t)VICII_MODEL_6569, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "8565 (PAL)", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_VICIIModel, (ui_callback_data_t)VICII_MODEL_8565, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("6569R1 (old PAL)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_VICIIModel, (ui_callback_data_t)VICII_MODEL_6569R1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "6567 (NTSC)", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_VICIIModel, (ui_callback_data_t)VICII_MODEL_6567, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "8562 (NTSC)", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_VICIIModel, (ui_callback_data_t)VICII_MODEL_8562, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("6567R56A (old NTSC)"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_VICIIModel, (ui_callback_data_t)VICII_MODEL_6567R56A, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "6572 (PAL-N)", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_VICIIModel, (ui_callback_data_t)VICII_MODEL_6572, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_RADIO(CIA1Model)
UI_MENU_DEFINE_RADIO(CIA2Model)

#define SET_CIAMODEL_MENU(x)                                                         \
static ui_menu_entry_t set_cia##x##model_submenu[] = {                               \
    { N_("6526 (old)"), UI_MENU_TYPE_TICK,                                           \
      (ui_callback_t)radio_CIA##x##Model, (ui_callback_data_t)CIA_MODEL_6526, NULL,  \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                     \
    { N_("8521 (new)"), UI_MENU_TYPE_TICK,                                           \
      (ui_callback_t)radio_CIA##x##Model, (ui_callback_data_t)CIA_MODEL_6526A, NULL, \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                                     \
    UI_MENU_ENTRY_LIST_END                                                           \
}

SET_CIAMODEL_MENU(1);
SET_CIAMODEL_MENU(2);

UI_MENU_DEFINE_RADIO(GlueLogic)

static ui_menu_entry_t set_gluelogic_submenu[] = {
    { N_("Discrete"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_GlueLogic, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Custom IC"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_GlueLogic, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_RADIO(IECReset)

static ui_menu_entry_t set_iecreset_submenu[] = {
    { N_("yes"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_IECReset, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("no"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_IECReset, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t c64_model_submenu[] = {
    { N_("C64 model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_c64_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("VIC-II model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_vicii_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("SID model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("CIA 1 model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_cia1model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("CIA 2 model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_cia2model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Glue logic"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_gluelogic_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Reset goes to IEC"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_iecreset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

UI_MENU_DEFINE_RADIO(SidStereoAddressStart)
UI_MENU_DEFINE_RADIO(SidTripleAddressStart)
UI_MENU_DEFINE_RADIO(SidQuadAddressStart)

SID_D4XX_MENU(set_sid_stereo_address_d4xx_submenu, radio_SidStereoAddressStart)
SID_D5XX_MENU(set_sid_stereo_address_d5xx_submenu, radio_SidStereoAddressStart)
SID_D6XX_MENU(set_sid_stereo_address_d6xx_submenu, radio_SidStereoAddressStart)
SID_D7XX_MENU(set_sid_stereo_address_d7xx_submenu, radio_SidStereoAddressStart)
SID_DEXX_MENU(set_sid_stereo_address_dexx_submenu, radio_SidStereoAddressStart)
SID_DFXX_MENU(set_sid_stereo_address_dfxx_submenu, radio_SidStereoAddressStart)

static ui_menu_entry_t set_sid_stereo_address_submenu[] = {
    { "$D4xx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_stereo_address_d4xx_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$D5xx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_stereo_address_d5xx_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$D6xx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_stereo_address_d6xx_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$D7xx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_stereo_address_d7xx_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DExx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_stereo_address_dexx_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DFxx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_stereo_address_dfxx_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

SID_D4XX_MENU(set_sid_triple_address_d4xx_submenu, radio_SidTripleAddressStart)
SID_D5XX_MENU(set_sid_triple_address_d5xx_submenu, radio_SidTripleAddressStart)
SID_D6XX_MENU(set_sid_triple_address_d6xx_submenu, radio_SidTripleAddressStart)
SID_D7XX_MENU(set_sid_triple_address_d7xx_submenu, radio_SidTripleAddressStart)
SID_DEXX_MENU(set_sid_triple_address_dexx_submenu, radio_SidTripleAddressStart)
SID_DFXX_MENU(set_sid_triple_address_dfxx_submenu, radio_SidTripleAddressStart)

static ui_menu_entry_t set_sid_triple_address_submenu[] = {
    { "$D4xx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_triple_address_d4xx_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$D5xx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_triple_address_d5xx_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$D6xx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_triple_address_d6xx_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$D7xx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_triple_address_d7xx_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DExx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_triple_address_dexx_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DFxx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_triple_address_dfxx_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

SID_D4XX_MENU(set_sid_quad_address_d4xx_submenu, radio_SidQuadAddressStart)
SID_D5XX_MENU(set_sid_quad_address_d5xx_submenu, radio_SidQuadAddressStart)
SID_D6XX_MENU(set_sid_quad_address_d6xx_submenu, radio_SidQuadAddressStart)
SID_D7XX_MENU(set_sid_quad_address_d7xx_submenu, radio_SidQuadAddressStart)
SID_DEXX_MENU(set_sid_quad_address_dexx_submenu, radio_SidQuadAddressStart)
SID_DFXX_MENU(set_sid_quad_address_dfxx_submenu, radio_SidQuadAddressStart)

    static ui_menu_entry_t set_sid_quad_address_submenu[] = {
    { "$D4xx", UI_MENU_TYPE_NORMAL,
        NULL, NULL, set_sid_quad_address_d4xx_submenu,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$D5xx", UI_MENU_TYPE_NORMAL,
        NULL, NULL, set_sid_quad_address_d5xx_submenu,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$D6xx", UI_MENU_TYPE_NORMAL,
        NULL, NULL, set_sid_quad_address_d6xx_submenu,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$D7xx", UI_MENU_TYPE_NORMAL,
        NULL, NULL, set_sid_quad_address_d7xx_submenu,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DExx", UI_MENU_TYPE_NORMAL,
        NULL, NULL, set_sid_quad_address_dexx_submenu,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DFxx", UI_MENU_TYPE_NORMAL,
        NULL, NULL, set_sid_quad_address_dfxx_submenu,
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_TOGGLE(SidFilters)

static ui_menu_entry_t sid_submenu[] = {
    { N_("SID model"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Extra SIDs"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_extra_sids_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Second SID base address"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_stereo_address_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Third SID base address"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_triple_address_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Fourth SID base address"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, set_sid_quad_address_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
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

UI_MENU_DEFINE_RADIO(BurstMod)

static ui_menu_entry_t burstmod_submenu[] = {
    { N_("None"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_BurstMod, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("CIA1"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_BurstMod, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("CIA2"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_BurstMod, (ui_callback_data_t)2, NULL,
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

UI_MENU_DEFINE_TOGGLE(SFXSoundSampler)
UI_MENU_DEFINE_TOGGLE(CartridgeReset)

static ui_menu_entry_t io_extensions_submenu[] = {
    { CARTRIDGE_NAME_GEORAM, UI_MENU_TYPE_NORMAL,
      NULL, NULL, georam_c64_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_REU, UI_MENU_TYPE_NORMAL,
      NULL, NULL, reu_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_RAMCART, UI_MENU_TYPE_NORMAL,
      NULL, NULL, ramcart_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { CARTRIDGE_NAME_DQBB, UI_MENU_TYPE_NORMAL,
      NULL, NULL, dqbb_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_EXPERT, UI_MENU_TYPE_NORMAL,
      NULL, NULL, expert_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_ISEPIC, UI_MENU_TYPE_NORMAL,
      NULL, NULL, isepic_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { CARTRIDGE_NAME_EASYFLASH, UI_MENU_TYPE_NORMAL,
      NULL, NULL, easyflash_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_GMOD2, UI_MENU_TYPE_NORMAL,
      NULL, NULL, gmod2_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_IDE64, UI_MENU_TYPE_NORMAL,
      NULL, NULL, ide64_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_MMC64, UI_MENU_TYPE_NORMAL,
      NULL, NULL, mmc64_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_MMC_REPLAY, UI_MENU_TYPE_NORMAL,
      NULL, NULL, mmcreplay_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_RETRO_REPLAY, UI_MENU_TYPE_NORMAL,
      NULL, NULL, retroreplay_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_SUPER_SNAPSHOT_V5, UI_MENU_TYPE_NORMAL,
      NULL, NULL, supersnapshot_v5_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_RAWNET
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Ethernet cartridge"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ethernetcart_c64_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_RRNETMK3, UI_MENU_TYPE_NORMAL,
      NULL, NULL, rrnetmk3_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_SEPERATOR,
    { CARTRIDGE_NAME_IEEE488, UI_MENU_TYPE_NORMAL,
      NULL, NULL, tpi_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Burst Mode Modification"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, burstmod_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { CARTRIDGE_NAME_DIGIMAX, UI_MENU_TYPE_NORMAL,
      NULL, NULL, digimax_c64_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_MAGIC_VOICE, UI_MENU_TYPE_NORMAL,
      NULL, NULL, magicvoice_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_MIDI
    { N_("MIDI emulation"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, midi_c64_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { CARTRIDGE_NAME_SFX_SOUND_EXPANDER, UI_MENU_TYPE_NORMAL,
      NULL, NULL, soundexpander_c64_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { CARTRIDGE_NAME_SFX_SOUND_SAMPLER, UI_MENU_TYPE_NORMAL,
      (ui_callback_t)toggle_SFXSoundSampler, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { CARTRIDGE_NAME_DS12C887RTC, UI_MENU_TYPE_NORMAL,
      NULL, NULL, ds12c887rtc_c64_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "Userport devices", UI_MENU_TYPE_NORMAL,
      NULL, NULL, userport_c64_cbm2_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("I/O collision handling ($D000-$DFFF)"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, iocollision_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Reset on cart change"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_CartridgeReset, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

/* ------------------------------------------------------------------------- */

static ui_menu_entry_t scpu64ui_main_romset_submenu[] = {
    { N_("Load new SCPU64 ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"SCPU64Name", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new character ROM"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_load_rom_file, (ui_callback_data_t)"ChargenName", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t scpu64_romset_submenu[] = {
    { N_("Load default ROMs"), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"default.vrs", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Load new computer ROM"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, scpu64ui_main_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new drive ROM"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_drivec64vic20_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Load new drive expansion ROM"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uidrivec64c128_expansion_romset_submenu,
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

static ui_menu_entry_t scpu64_menu[] = {
    { N_("Model settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, c64_model_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("SuperCPU64 settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, scpu64_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("RAM reset pattern"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ui_ram_pattern_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("ROM settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, scpu64_romset_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("VIC-II settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, vicii_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("SID settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, sid_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("I/O extensions"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, io_extensions_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    { N_("RS232 settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, uirs232_c64_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
#ifdef HAVE_RAWNET
    { N_("Ethernet settings"), UI_MENU_TYPE_NORMAL,
        NULL, NULL, uics8900_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t xscpu64_left_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, uiattach_smart_attach_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, uiattach_disk_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_c64cart_commands_menu,
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

static ui_menu_entry_t xscpu64_file_submenu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, uiattach_smart_attach_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, uiattach_disk_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_c64scpucart_commands_menu,
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
static ui_menu_entry_t xscpu64_edit_submenu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_edit_commands_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};
#endif

static ui_menu_entry_t xscpu64_snapshot_submenu[] = {
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

static ui_menu_entry_t xscpu64_settings_submenu[] = {
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
      NULL, NULL, ui_drivec64_settings_menu,
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
      NULL, NULL, joystick_settings_c64_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_MOUSE
    { N_("Mouse emulation"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, mouse_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, scpu64_menu,
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

static ui_menu_entry_t xscpu64_main_menu[] = {
    { N_("File"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, xscpu64_file_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef USE_GNOMEUI
    { N_("Edit"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, xscpu64_edit_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    { N_("Snapshot"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, xscpu64_snapshot_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, xscpu64_settings_submenu,
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

static ui_menu_entry_t xscpu64_speed_menu[] = {
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, ui_performance_settings_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "--", UI_MENU_TYPE_SEPARATOR,
      NULL, NULL, ui_runmode_commands_menu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static void scpu64ui_dynamic_menu_create(void)
{
    uisound_menu_create();
    uisampler_menu_create();
    uivicii_menu_create();
    uicart_menu_create();
    uikeyboard_menu_create();
    uijoyport_menu_create(1, 1, 1, 1, 0);
    uisid_model_menu_create();
#ifdef HAVE_RAWNET
    uiethernetcart_menu_create();
#endif
}

static void scpu64ui_dynamic_menu_shutdown(void)
{
    uivicii_menu_shutdown();
    uicart_menu_shutdown();
    uisound_menu_shutdown();
    uisampler_menu_shutdown();
    uikeyboard_menu_shutdown();
    uijoyport_menu_shutdown();
    uisid_model_menu_shutdown();
#ifdef HAVE_RAWNET
    uiethernetcart_menu_shutdown();
#endif
}

int scpu64ui_init(void)
{
    ui_set_application_icon(scpu64_icon_data);
    scpu64ui_dynamic_menu_create();

    ui_set_left_menu(xscpu64_left_menu);
    ui_set_right_menu(xscpu64_settings_submenu);
    ui_set_topmenu(xscpu64_main_menu);
    ui_set_speedmenu(xscpu64_speed_menu);

    ui_set_drop_callback(uiattach_autostart_file);

    ui_update_menus();

    return 0;
}

void scpu64ui_shutdown(void)
{
    scpu64ui_dynamic_menu_shutdown();
}
