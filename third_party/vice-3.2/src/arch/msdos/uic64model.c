/*
 * uic64model.c - C64 model selection UI for MS-DOS.
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

#include "c64model.h"
#include "c64-resources.h"
#include "machine.h"
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uiapi.h"
#include "uic64model.h"
#include "vicii.h"

TUI_MENU_DEFINE_RADIO(CIA1Model)
TUI_MENU_DEFINE_RADIO(CIA2Model)
TUI_MENU_DEFINE_TOGGLE(IECReset)
TUI_MENU_DEFINE_RADIO(KernalRev)

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

static TUI_MENU_CALLBACK(radio_VICIIModel_callback)
{
    if (been_activated) {
        vicmodel_set(param);
        *become_default = 1;
    } else {
        int v = vicmodel_get();

        if (v == param) {
            *become_default = 1;
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(vicii_model_submenu_callback)
{
    int value = vicmodel_get();
    char *s;

    switch (value) {
        default:
        case VICII_MODEL_PALG:
            s = "PAL-G";
            break;
        case VICII_MODEL_PALG_OLD:
            s = "Old PAL-G";
            break;
        case VICII_MODEL_NTSCM:
            s = "NTSC-M";
            break;
        case VICII_MODEL_NTSCM_OLD:
            s = "Old NTSC-M";
            break;
        case VICII_MODEL_PALN:
            s = "PAL-N";
            break;
    }
    return s;
}

static tui_menu_item_def_t vicii_model_submenu[] = {
    { "PAL-G", NULL, radio_VICIIModel_callback,
      (void *)VICII_MODEL_PALG, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Old PAL-G", NULL, radio_VICIIModel_callback,
      (void *)VICII_MODEL_PALG_OLD, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "NTSC-M", NULL, radio_VICIIModel_callback,
      (void *)VICII_MODEL_NTSCM, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Old NTSC-M", NULL, radio_VICIIModel_callback,
      (void *)VICII_MODEL_NTSCM_OLD, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "PAL-N", NULL, radio_VICIIModel_callback,
      (void *)VICII_MODEL_PALN, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static char *get_cia_model(int value)
{
    char *retval;

    switch (value) {
        default:
        case 0:
            retval = "6526 (old)";
            break;
        case 1:
            retval = "8521 (new)";
            break;
    }
    return retval;
}

static TUI_MENU_CALLBACK(cia1_model_submenu_callback)
{
    int value;

    resources_get_int("CIA1Model", &value);

    return get_cia_model(value);
}

static tui_menu_item_def_t cia1_model_submenu[] = {
    { "6526 (old)", NULL, radio_CIA1Model_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "8521 (new)", NULL, radio_CIA1Model_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(cia2_model_submenu_callback)
{
    int value;

    resources_get_int("CIA2Model", &value);

    return get_cia_model(value);
}

static tui_menu_item_def_t cia2_model_submenu[] = {
    { "6526 (old)", NULL, radio_CIA2Model_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "8521 (new)", NULL, radio_CIA2Model_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static char *get_kernal_rev(int value)
{
    char *retval;

    switch (value) {
        default:
        case C64_KERNAL_REV1:
            retval = "Rev 1";
            break;
        case C64_KERNAL_REV2:
            retval = "Rev 2";
            break;
        case C64_KERNAL_REV3:
            retval = "Rev 3";
            break;
        case C64_KERNAL_SX64:
            retval = "SX-64";
            break;
        case C64_KERNAL_4064:
            retval = "4064";
            break;
    }
    return retval;
}

static TUI_MENU_CALLBACK(kernal_rev_submenu_callback)
{
    int value;

    resources_get_int("KernalRev", &value);

    return get_kernal_rev(value);
}

static tui_menu_item_def_t kernal_rev_submenu[] = {
    { "Rev 1", NULL, radio_KernalRev_callback,
      (void *)C64_KERNAL_REV1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Rev 2", NULL, radio_KernalRev_callback,
      (void *)C64_KERNAL_REV2, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Rev 3", NULL, radio_KernalRev_callback,
      (void *)C64_KERNAL_REV3, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "SX-64", NULL, radio_KernalRev_callback,
      (void *)C64_KERNAL_SX64, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "4064", NULL, radio_KernalRev_callback,
      (void *)C64_KERNAL_4064, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t c64_custom_model_menu_items[] = {
    { "_VICII model:", "Select the VICII model",
      vicii_model_submenu_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, vicii_model_submenu,
      "VICII model" },
    { "_Reset IEC bus with computer:",
      "Enable resetting of the IEC bus when the computer resets",
      toggle_IECReset_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "CIA _1 model:", "Select the CIA #1 model",
      cia1_model_submenu_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, cia1_model_submenu,
      "CIA 1 model" },
    { "CIA _2 model:", "Select the CIA #2 model",
      cia2_model_submenu_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, cia2_model_submenu,
      "CIA 2 model" },
    { "_Kernal revision:", "Select the Kernal revision",
      kernal_rev_submenu_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, kernal_rev_submenu,
      "Kernal revision" },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(set_model_callback)
{
    if (been_activated) {
        c64model_set((int)param);
        ui_update_menus();
    }

    /* This way, the "Not Really!" item is always the default one.  */
    *become_default = 0;
    return NULL;
}

static tui_menu_item_def_t c64_model_items[] = {
    { "_Not Really!",
      "Keep the current settings",
      NULL, NULL, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "--" },
    { "_A: C64 PAL",
      "Configure the emulator to emulate a C64 PAL machine and do a soft RESET",
      set_model_callback, (void *)C64MODEL_C64_PAL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_B: C64C PAL",
      "Configure the emulator to emulate a C64C PAL machine and do a soft RESET",
      set_model_callback, (void *)C64MODEL_C64C_PAL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_C: C64 old PAL",
      "Configure the emulator to emulate a C64 old PAL machine and do a soft RESET",
      set_model_callback, (void *)C64MODEL_C64_OLD_PAL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_D: C64 NTSC",
      "Configure the emulator to emulate a C64 NTSC machine and do a soft RESET",
      set_model_callback, (void *)C64MODEL_C64_NTSC, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_E: C64C NTSC",
      "Configure the emulator to emulate a C64C NTSC machine and do a soft RESET",
      set_model_callback, (void *)C64MODEL_C64C_NTSC, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_F: C64 old NTSC",
      "Configure the emulator to emulate a C64 old NTSC machine and do a soft RESET",
      set_model_callback, (void *)C64MODEL_C64_OLD_NTSC, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_G: Drean",
      "Configure the emulator to emulate a drean machine and do a soft RESET",
      set_model_callback, (void *)C64MODEL_C64_PAL_N, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_H: C64 SX PAL",
      "Configure the emulator to emulate a C64 SX PAL machine and do a soft RESET",
      set_model_callback, (void *)C64MODEL_C64SX_PAL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_I: C64 SX NTSC",
      "Configure the emulator to emulate a C64 SX NTSC machine and do a soft RESET",
      set_model_callback, (void *)C64MODEL_C64SX_NTSC, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_J: Japanese",
      "Configure the emulator to emulate a Japanese C64 machine and do a soft RESET",
      set_model_callback, (void *)C64MODEL_C64_JAP, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_K: C64 GS",
      "Configure the emulator to emulate a C64 GS machine and do a soft RESET",
      set_model_callback, (void *)C64MODEL_C64_GS, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_L: PET64 PAL",
      "Configure the emulator to emulate a PET64 PAL machine and do a soft RESET",
      set_model_callback, (void *)C64MODEL_PET64_PAL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_M: PET64 NTSC",
      "Configure the emulator to emulate a PET64 NTSC machine and do a soft RESET",
      set_model_callback, (void *)C64MODEL_PET64_NTSC, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "_N: MAX Machine",
      "Configure the emulator to emulate a MAX Machine and do a soft RESET",
      set_model_callback, (void *)C64MODEL_ULTIMAX, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "--" },
    { "_O: Custom",
      "Set custom model options",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, c64_custom_model_menu_items, "Set custom C64 model" },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uic64model_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_c64model_submenu;

    ui_c64model_submenu = tui_menu_create("C64 model settings", 1);

    tui_menu_add(ui_c64model_submenu, c64_model_items);

    tui_menu_add_submenu(parent_submenu, "C64 _model settings...",
                         "C64 model settings",
                         ui_c64model_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}
