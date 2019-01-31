/*
 * uic64cart.c - C64-specific cartridge user interface.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Tibor Biczo <crown@axelero.hu>
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
#include <tchar.h>
#include <windows.h>

#include "cartridge.h"
#include "c64cart.h"
#include "intl.h"
#include "keyboard.h"
#include "lib.h"
#include "res.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uiapi.h"
#include "uic64cart.h"
#include "uicart.h"
#include "uilib.h"

static const uicart_params_t c64_ui_cartridges[] = {
    { IDM_CART_ATTACH_CRT, CARTRIDGE_CRT, IDS_ATTACH_CRT_CART_IMAGE, NULL, UILIB_FILTER_ALL | UILIB_FILTER_CRT },
    { 0, 0, 0, NULL, 0 }
};

static uicart_params_t *built_cartridges = NULL;

void uic64cart_build_carts(int start_id)
{
    int i;
    int j = 0;
    cartridge_info_t *cart_info = cartridge_get_info_list();

    if (built_cartridges) {
        lib_free(built_cartridges);
        built_cartridges = NULL;
    }

    /* count */
    for (i = 0; cart_info[i].name; ++i) { }

    built_cartridges = lib_malloc((i + 1) * sizeof(uicart_params_t));

    /* generic */
    for (i = 0; cart_info[i].name; ++i) {
        if (cart_info[i].flags == CARTRIDGE_GROUP_GENERIC) {
            built_cartridges[j].wparam = start_id + j;
            built_cartridges[j].type = cart_info[i].crtid;
            built_cartridges[j].title = 0;
            built_cartridges[j].trans_title = cart_info[i].name;
            built_cartridges[j].filter = UILIB_FILTER_ALL | UILIB_FILTER_BIN;
            ++j;
        }
    }

    /* ramex */
    for (i = 0; cart_info[i].name; ++i) {
        if (cart_info[i].flags == CARTRIDGE_GROUP_RAMEX) {
            built_cartridges[j].wparam = start_id + j;
            built_cartridges[j].type = cart_info[i].crtid;
            built_cartridges[j].title = 0;
            built_cartridges[j].trans_title = cart_info[i].name;
            built_cartridges[j].filter = UILIB_FILTER_ALL | UILIB_FILTER_BIN;
            ++j;
        }
    }

    /* freezer */
    for (i = 0; cart_info[i].name; ++i) {
        if (cart_info[i].flags == CARTRIDGE_GROUP_FREEZER) {
            built_cartridges[j].wparam = start_id + j;
            built_cartridges[j].type = cart_info[i].crtid;
            built_cartridges[j].title = 0;
            built_cartridges[j].trans_title = cart_info[i].name;
            built_cartridges[j].filter = UILIB_FILTER_ALL | UILIB_FILTER_BIN;
            ++j;
        }
    }

    /* game */
    for (i = 0; cart_info[i].name; ++i) {
        if (cart_info[i].flags == CARTRIDGE_GROUP_GAME) {
            built_cartridges[j].wparam = start_id + j;
            built_cartridges[j].type = cart_info[i].crtid;
            built_cartridges[j].title = 0;
            built_cartridges[j].trans_title = cart_info[i].name;
            built_cartridges[j].filter = UILIB_FILTER_ALL | UILIB_FILTER_BIN;
            ++j;
        }
    }

    /* util */
    for (i = 0; cart_info[i].name; ++i) {
        if (cart_info[i].flags == CARTRIDGE_GROUP_UTIL) {
            built_cartridges[j].wparam = start_id + j;
            built_cartridges[j].type = cart_info[i].crtid;
            built_cartridges[j].title = 0;
            built_cartridges[j].trans_title = cart_info[i].name;
            built_cartridges[j].filter = UILIB_FILTER_ALL | UILIB_FILTER_BIN;
            ++j;
        }
    }
    built_cartridges[j].wparam = 0;
    built_cartridges[j].type = 0;
    built_cartridges[j].title = 0;
    built_cartridges[j].trans_title = NULL;
    built_cartridges[j].filter = 0;
}

static void uic64cart_attach(WPARAM wparam, HWND hwnd,
                             const uicart_params_t *cartridges)
{
    uicart_attach(wparam, hwnd, cartridges);
}

void uic64cart_add_menu_item(HMENU menu, const char *name, int menu_id)
{
    TCHAR st_name[32];
    TCHAR st_text[64];

    system_mbstowcs(st_name, name, 32);
    lib_sntprintf(st_text, 64, TEXT("%s %s..."), st_name, intl_translate_tcs(IDS_IMAGE));
    AppendMenu(menu, MF_STRING, menu_id, st_text);
}

void uic64cart_proc(WPARAM wparam, HWND hwnd, int min_id, int max_id)
{
    if ((wparam & 0xffff) >= (unsigned int)min_id && (wparam & 0xffff) <= (unsigned int)max_id) {
        uic64cart_attach(wparam, hwnd, built_cartridges);
    }

    switch (wparam & 0xffff) {
        case IDM_CART_ATTACH_CRT:
            uic64cart_attach(wparam, hwnd, c64_ui_cartridges);
            break;
        case IDM_CART_SET_DEFAULT:
            cartridge_set_default();
            break;
        case IDM_CART_DETACH:
            cartridge_detach_image(-1);
            break;
        case IDM_CART_FREEZE:
            keyboard_clear_keymatrix();
            cartridge_trigger_freeze();
            break;
    }
}

void uic64cart_init(void)
{
}
