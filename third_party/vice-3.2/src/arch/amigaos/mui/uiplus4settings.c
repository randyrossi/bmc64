/*
 * uiplus4settings.c
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
#ifdef AMIGA_M68K
#define _INLINE_MUIMASTER_H
#endif
#include "mui.h"

#include "plus4memhacks.h"
#include "uiplus4settings.h"
#include "intl.h"
#include "translate.h"

static APTR plus4_ram_size;

static char *ui_plus4_ram_size[] = {
    "16K",
    "32K",
    "64K",
    "256K (CSORY)",
    "256K (HANNES)",
    "1024K (HANNES)",
    "4096K (HANNES)",
    NULL
};

static APTR build_gui(void)
{
    APTR app, ui, ok, cancel;

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(plus4_ram_size, translate_text(IDS_MEMORY), ui_plus4_ram_size)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);
    }

    return ui;
}

void ui_plus4_settings_dialog(void)
{
    APTR window;
    int memoryhack, ramsize, val;

    window = mui_make_simple_window(build_gui(), translate_text(IDS_PLUS4_SETTINGS));

    if (window != NULL) {
        mui_add_window(window);
        resources_get_value("MemoryHack", (void *)&memoryhack);
        resources_get_value("RamSize", (void *)&ramsize);
        if (memoryhack) {
            switch (memoryhack) {
                case MEMORY_HACK_C256K:
                    set(plus4_ram_size, MUIA_Cycle_Active, 3);
                    break;
                case MEMORY_HACK_H256K:
                    set(plus4_ram_size, MUIA_Cycle_Active, 4);
                    break;
                case MEMORY_HACK_H1024K:
                    set(plus4_ram_size, MUIA_Cycle_Active, 5);
                    break;
                case MEMORY_HACK_H4096K:
                    set(plus4_ram_size, MUIA_Cycle_Active, 6);
                    break;
            }
        } else {
            switch(ramsize) {
                case 16:
                    set(plus4_ram_size, MUIA_Cycle_Active, 0);
                    break;
                case 32:
                    set(plus4_ram_size, MUIA_Cycle_Active, 1);
                    break;
                case 64:
                    set(plus4_ram_size, MUIA_Cycle_Active, 2);
                    break;
            }
        }
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            get(plus4_ram_size, MUIA_Cycle_Active, (APTR)&val);
            switch (val) {
                case 0:
                    resources_set_int("RamSize", 16);
                    break;
                case 1:
                    resources_set_int("RamSize", 32);
                    break;
                case 2:
                    resources_set_int("RamSize", 64);
                    break;
                case 3:
                    resources_set_int("MemoryHack", MEMORY_HACK_C256K);
                    break;
                case 4:
                    resources_set_int("MemoryHack", MEMORY_HACK_H256K);
                    break;
                case 5:
                    resources_set_int("MemoryHack", MEMORY_HACK_H1024K);
                    break;
                case 6:
                    resources_set_int("MemoryHack", MEMORY_HACK_H4096K);
                    break;
            }
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}
