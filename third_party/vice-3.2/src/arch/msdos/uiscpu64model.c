/*
 * uiscpu64model.c - SCPU64 model selection UI for MS-DOS.
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
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uiapi.h"
#include "uiscpu64model.h"
#include "vicii.h"

TUI_MENU_DEFINE_RADIO(VICIIModel)
TUI_MENU_DEFINE_RADIO(CIA1Model)
TUI_MENU_DEFINE_RADIO(CIA2Model)
TUI_MENU_DEFINE_RADIO(GlueLogic)
TUI_MENU_DEFINE_TOGGLE(IECReset)

static TUI_MENU_CALLBACK(vicii_model_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("VICIIModel", &value);
    switch (value) {
        default:
        case VICII_MODEL_6569:
            s = "6569 (PAL)";
            break;
        case VICII_MODEL_8565:
            s = "8565 (PAL)";
            break;
        case VICII_MODEL_6569R1:
            s = "6569R1 (old PAL)";
            break;
        case VICII_MODEL_6567:
            s = "6567 (NTSC)";
            break;
        case VICII_MODEL_8562:
            s = "8562 (NTSC)";
            break;
        case VICII_MODEL_6567R56A:
            s = "6567R56A (old NTSC)";
            break;
        case VICII_MODEL_6572:
            s = "6572 (PAL-N)";
            break;
    }
    return s;
}

static tui_menu_item_def_t vicii_model_submenu[] = {
    { "6569 (PAL)", NULL, radio_VICIIModel_callback,
      (void *)VICII_MODEL_6569, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "8565 (PAL)", NULL, radio_VICIIModel_callback,
      (void *)VICII_MODEL_8565, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "6569R1 (old PAL)", NULL, radio_VICIIModel_callback,
      (void *)VICII_MODEL_6569R1, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "6567 (NTSC)", NULL, radio_VICIIModel_callback,
      (void *)VICII_MODEL_6567, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "8562 (NTSC)", NULL, radio_VICIIModel_callback,
      (void *)VICII_MODEL_8562, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "6567R56A (old NTSC)", NULL, radio_VICIIModel_callback,
      (void *)VICII_MODEL_6567R56A, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "6572 (PAL-N)", NULL, radio_VICIIModel_callback,
      (void *)VICII_MODEL_6572, 20, TUI_MENU_BEH_CLOSE, NULL, NULL },
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

static TUI_MENU_CALLBACK(gluelogic_submenu_callback)
{
    int value;
    char *s;

    resources_get_int("GlueLogic", &value);
    switch (value) {
        default:
        case 0:
            s = "Discrete";
            break;
        case 1:
            s = "Custom IC";
            break;
    }
    return s;
}

static tui_menu_item_def_t gluelogic_submenu[] = {
    { "Discrete", NULL, radio_GlueLogic_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Custom IC", NULL, radio_GlueLogic_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
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
    { "Glue logic:", "Select the kind of glue logic",
      gluelogic_submenu_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, gluelogic_submenu,
      "Glue logic" },
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
    { "--" },
    { "_L: Custom",
      "Set custom model options",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, c64_custom_model_menu_items, "Set custom C64 model" },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiscpu64model_init(struct tui_menu *parent_submenu)
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
