/*
 * menu_c64model.c - C64 model menu for SDL UI.
 *
 * Written by
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

#include "types.h"

#include "c64model.h"
#include "c64-resources.h"
#include "cia.h"
#include "machine.h"
#include "menu_common.h"
#include "menu_sid.h"
#include "resources.h"
#include "uimenu.h"
#include "vicii.h"

/* ------------------------------------------------------------------------- */
/* common */

static UI_MENU_CALLBACK(custom_C64Model_callback)
{
    int model, selected;

    selected = vice_ptr_to_int(param);

    if (activated) {
        c64model_set(selected);
    } else {
        model = c64model_get();

        if (selected == model) {
            return sdl_menu_text_tick;
        }
    }

    return NULL;
}

static const ui_menu_entry_t c64_model_submenu[] = {
    { "C64 PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64_PAL },
    { "C64C PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64C_PAL },
    { "C64 old PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64_OLD_PAL },
    { "C64 NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64_NTSC },
    { "C64C NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64C_NTSC },
    { "C64 old NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64_OLD_NTSC },
    { "Drean",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64_PAL_N },
    { "C64 SX PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64SX_PAL },
    { "C64 SX NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64SX_NTSC },
    { "Japanese",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64_JAP },
    { "C64 GS",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64_GS },
    { "PET64 PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_PET64_PAL },
    { "PET64 NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_PET64_NTSC },
    { "MAX Machine",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_ULTIMAX },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t scpu64_model_submenu[] = {
    { "C64 PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64_PAL },
    { "C64C PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64C_PAL },
    { "C64 old PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64_OLD_PAL },
    { "C64 NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64_NTSC },
    { "C64C NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64C_NTSC },
    { "C64 old NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64_OLD_NTSC },
    { "Drean",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64_PAL_N },
    { "C64 SX PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64SX_PAL },
    { "C64 SX NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64SX_NTSC },
    { "Japanese",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64_JAP },
    { "C64 GS",
      MENU_ENTRY_RESOURCE_RADIO,
      custom_C64Model_callback,
      (ui_callback_data_t)C64MODEL_C64_GS },
    SDL_MENU_LIST_END
};

static UI_MENU_CALLBACK(custom_sidsubmenu_callback)
{
    /* Display the SID model by using the submenu radio callback
       on the first submenu (SID model) of the SID settings. */
    return submenu_radio_callback(0, sid_c64_menu[0].data);
}

#define CIA_MODEL_MENU(xyz)                                     \
    UI_MENU_DEFINE_RADIO(CIA##xyz##Model)                       \
    static const ui_menu_entry_t cia##xyz##_model_submenu[] = { \
        { "6526  (old)",                                        \
          MENU_ENTRY_RESOURCE_TOGGLE,                           \
          radio_CIA##xyz##Model_callback,                       \
          (ui_callback_data_t)CIA_MODEL_6526 },                 \
        { "8521 (new)",                                        \
          MENU_ENTRY_RESOURCE_TOGGLE,                           \
          radio_CIA##xyz##Model_callback,                       \
          (ui_callback_data_t)CIA_MODEL_6526A },                \
        SDL_MENU_LIST_END                                       \
    };

CIA_MODEL_MENU(1)
CIA_MODEL_MENU(2)

UI_MENU_DEFINE_TOGGLE(IECReset)

UI_MENU_DEFINE_RADIO(KernalRev)

static const ui_menu_entry_t kernal_rev_submenu[] = {
    { "Rev 1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_KernalRev_callback,
      (ui_callback_data_t)C64_KERNAL_REV1 },
    { "Rev 2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_KernalRev_callback,
      (ui_callback_data_t)C64_KERNAL_REV2 },
    { "Rev 3",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_KernalRev_callback,
      (ui_callback_data_t)C64_KERNAL_REV3 },
    { "SX-64",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_KernalRev_callback,
      (ui_callback_data_t)C64_KERNAL_SX64 },
    { "4064",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_KernalRev_callback,
      (ui_callback_data_t)C64_KERNAL_4064 },
    SDL_MENU_LIST_END
};

/* ------------------------------------------------------------------------- */
/* x64sc */

UI_MENU_DEFINE_RADIO(VICIIModel)

static const ui_menu_entry_t viciisc_model_submenu[] = {
    { "6569 (PAL)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VICIIModel_callback,
      (ui_callback_data_t)VICII_MODEL_6569 },
    { "8565 (PAL)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VICIIModel_callback,
      (ui_callback_data_t)VICII_MODEL_8565 },
    { "6569R1 (old PAL)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VICIIModel_callback,
      (ui_callback_data_t)VICII_MODEL_6569R1 },
    { "6567 (NTSC)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VICIIModel_callback,
      (ui_callback_data_t)VICII_MODEL_6567 },
    { "8562 (NTSC)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VICIIModel_callback,
      (ui_callback_data_t)VICII_MODEL_8562 },
    { "6567R56A (old NTSC)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VICIIModel_callback,
      (ui_callback_data_t)VICII_MODEL_6567R56A },
    { "6572 (PAL-N)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VICIIModel_callback,
      (ui_callback_data_t)VICII_MODEL_6572 },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_RADIO(GlueLogic)

const ui_menu_entry_t c64sc_model_menu[] = {
    { "C64 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)c64_model_submenu },
    SDL_MENU_ITEM_SEPARATOR,
    { "VICII model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)viciisc_model_submenu },
    SDL_MENU_ITEM_SEPARATOR,
    { "SID settings",
      MENU_ENTRY_SUBMENU,
      custom_sidsubmenu_callback,
      (ui_callback_data_t)sid_c64_menu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("CIA models"),
    { "CIA 1 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)cia1_model_submenu },
    { "CIA 2 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)cia2_model_submenu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Glue logic"),
    { "Discrete",
      MENU_ENTRY_RESOURCE_TOGGLE,
      radio_GlueLogic_callback,
      (ui_callback_data_t)0 },
    { "Custom IC",
      MENU_ENTRY_RESOURCE_TOGGLE,
      radio_GlueLogic_callback,
      (ui_callback_data_t)1 },
    SDL_MENU_ITEM_SEPARATOR,
    { "Reset goes to IEC",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IECReset_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Kernal revision",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)kernal_rev_submenu },
    SDL_MENU_LIST_END
};

/* ------------------------------------------------------------------------- */
/* xscpu64 */

const ui_menu_entry_t scpu64_model_menu[] = {
    { "C64 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)scpu64_model_submenu },
    SDL_MENU_ITEM_SEPARATOR,
    { "VICII model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)viciisc_model_submenu },
    SDL_MENU_ITEM_SEPARATOR,
    { "SID settings",
      MENU_ENTRY_SUBMENU,
      custom_sidsubmenu_callback,
      (ui_callback_data_t)sid_c64_menu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("CIA models"),
    { "CIA 1 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)cia1_model_submenu },
    { "CIA 2 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)cia2_model_submenu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Glue logic"),
    { "Discrete",
      MENU_ENTRY_RESOURCE_TOGGLE,
      radio_GlueLogic_callback,
      (ui_callback_data_t)0 },
    { "Custom IC",
      MENU_ENTRY_RESOURCE_TOGGLE,
      radio_GlueLogic_callback,
      (ui_callback_data_t)1 },
    SDL_MENU_ITEM_SEPARATOR,
    { "Reset goes to IEC",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IECReset_callback,
      NULL },
    SDL_MENU_LIST_END
};

/* ------------------------------------------------------------------------- */
/* x64 */

#define VICMODEL_UNKNOWN -1
#define VICMODEL_NUM 5

struct vicmodel_s {
    int video;
    int luma;
};

static struct vicmodel_s vicmodels[] = {
    { MACHINE_SYNC_PAL,     1 }, /* VICII_MODEL_PALG */
    { MACHINE_SYNC_PAL,     0 }, /* VICII_MODEL_PALG_OLD */
    { MACHINE_SYNC_NTSC,    1 }, /* VICII_MODEL_NTSCM */
    { MACHINE_SYNC_NTSCOLD, 0 }, /* VICII_MODEL_NTSCM_OLD */
    { MACHINE_SYNC_PALN,    1 }  /* VICII_MODEL_PALN */
};

static int vicmodel_get_temp(int video)
{
    int i;

    for (i = 0; i < VICMODEL_NUM; ++i) {
        if (vicmodels[i].video == video) {
            return i;
        }
    }

    return VICMODEL_UNKNOWN;
}

static int vicmodel_get(void)
{
    int video;

    if (resources_get_int("MachineVideoStandard", &video) < 0) {
        return -1;
    }

    return vicmodel_get_temp(video);
}

static void vicmodel_set(int model)
{
    int old_model;

    old_model = vicmodel_get();

    if ((model == old_model) || (model == VICMODEL_UNKNOWN)) {
        return;
    }

    resources_set_int("MachineVideoStandard", vicmodels[model].video);
}

static const char *x64_ui_vicii_model(int activated, ui_callback_data_t param)
{
    int val = vice_ptr_to_int(param);

    if (activated) {
        vicmodel_set(val);
    } else {
        int v = vicmodel_get();

        if (v == val) {
            return sdl_menu_text_tick;
        }
    }
    return NULL;
}

static const ui_menu_entry_t vicii_model_submenu[] = {
    { "PAL-G",
      MENU_ENTRY_RESOURCE_RADIO,
      x64_ui_vicii_model,
      (ui_callback_data_t)VICII_MODEL_PALG },
    { "Old PAL-G",
      MENU_ENTRY_RESOURCE_RADIO,
      x64_ui_vicii_model,
      (ui_callback_data_t)VICII_MODEL_PALG_OLD },
    { "NTSC-M",
      MENU_ENTRY_RESOURCE_RADIO,
      x64_ui_vicii_model,
      (ui_callback_data_t)VICII_MODEL_NTSCM },
    { "Old NTSC-M",
      MENU_ENTRY_RESOURCE_RADIO,
      x64_ui_vicii_model,
      (ui_callback_data_t)VICII_MODEL_NTSCM_OLD },
    { "PAL-N",
      MENU_ENTRY_RESOURCE_RADIO,
      x64_ui_vicii_model,
      (ui_callback_data_t)VICII_MODEL_PALN },
    SDL_MENU_LIST_END
};

const ui_menu_entry_t c64_model_menu[] = {
    { "C64 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)c64_model_submenu },
    SDL_MENU_ITEM_SEPARATOR,
    { "VICII model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)vicii_model_submenu },
    SDL_MENU_ITEM_SEPARATOR,
    { "SID settings",
      MENU_ENTRY_SUBMENU,
      custom_sidsubmenu_callback,
      (ui_callback_data_t)sid_c64_menu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("CIA models"),
    { "CIA 1 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)cia1_model_submenu },
    { "CIA 2 model",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)cia2_model_submenu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Reset goes to IEC",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IECReset_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Kernal revision",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)kernal_rev_submenu },
    SDL_MENU_LIST_END
};
