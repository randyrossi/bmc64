/*
 * uiplus4cart.c - PLUS4-specific cartridge user interface.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Tibor Biczo <crown@axelero.hu>
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
#include <windows.h>

#include "cartridge.h"
#include "intl.h"
#include "plus4cart.h"
#include "keyboard.h"
#include "lib.h"
#include "res.h"
#include "system.h"
#include "translate.h"
#include "uiapi.h"
#include "uicart.h"
#include "uiplus4cart.h"
#include "uilib.h"
#include "resources.h"

static const uicart_params_t plus4_ui_cartridges[] = {
    { IDM_CART_SMART_ATTACH, 0, IDS_SELECT_CARTRIDGE_IMAGE, NULL, UILIB_FILTER_ALL | UILIB_FILTER_BIN },
    { IDM_CART_ATTACH_C0LO, 0, IDS_ATTACH_CART0_LOW, NULL,UILIB_FILTER_ALL | UILIB_FILTER_BIN },
    { IDM_CART_ATTACH_C0HI, 0, IDS_ATTACH_CART0_HIGH, NULL, UILIB_FILTER_ALL | UILIB_FILTER_BIN },
    { IDM_CART_ATTACH_C1LO, 0, IDS_ATTACH_CART1_LOW, NULL, UILIB_FILTER_ALL | UILIB_FILTER_BIN },
    { IDM_CART_ATTACH_C1HI, 0, IDS_ATTACH_CART1_HIGH, NULL, UILIB_FILTER_ALL | UILIB_FILTER_BIN },
    { IDM_CART_ATTACH_C2LO, 0, IDS_ATTACH_CART2_LOW, NULL, UILIB_FILTER_ALL | UILIB_FILTER_BIN },
    { IDM_CART_ATTACH_C2HI, 0, IDS_ATTACH_CART2_HIGH, NULL, UILIB_FILTER_ALL | UILIB_FILTER_BIN },
    { 0, 0, 0, NULL, 0 }
};

static int uiplus4cart_attach_image(int type, char *s)
{
    switch (type) {
        case IDM_CART_SMART_ATTACH:
            return cartridge_attach_image(CARTRIDGE_PLUS4_DETECT, s);
        case IDM_CART_ATTACH_C0LO:
            return cartridge_attach_image(CARTRIDGE_PLUS4_16KB_C0LO, s);
        case IDM_CART_ATTACH_C0HI:
            return cartridge_attach_image(CARTRIDGE_PLUS4_16KB_C0HI, s);
        case IDM_CART_ATTACH_C1LO:
            return cartridge_attach_image(CARTRIDGE_PLUS4_16KB_C1LO, s);
        case IDM_CART_ATTACH_C1HI:
            return cartridge_attach_image(CARTRIDGE_PLUS4_16KB_C1HI, s);
        case IDM_CART_ATTACH_C2LO:
            return cartridge_attach_image(CARTRIDGE_PLUS4_16KB_C2LO, s);
        case IDM_CART_ATTACH_C2HI:
            return cartridge_attach_image(CARTRIDGE_PLUS4_16KB_C2HI, s);
    }
    return -1;
}

static void uiplus4cart_attach(WPARAM wparam, HWND hwnd, const uicart_params_t *cartridges)
{
    int i;
    TCHAR *st_name;

    i = 0;

    while ((cartridges[i].wparam != wparam) && (cartridges[i].wparam != 0)) {
        i++;
    }

    if (cartridges[i].wparam == 0) {
        ui_error(translate_text(IDS_BAD_CARTRIDGE_CONFIG));
        return;
    }

    if ((st_name = uilib_select_file(hwnd, translate_text(cartridges[i].title), cartridges[i].filter, UILIB_SELECTOR_TYPE_FILE_LOAD, UILIB_SELECTOR_STYLE_CART)) != NULL) {
        char *name;

        name = system_wcstombs_alloc(st_name);
        if (uiplus4cart_attach_image((int)cartridges[i].wparam, name) < 0) {
            ui_error(translate_text(IDS_INVALID_CARTRIDGE_IMAGE));
        }
        system_wcstombs_free(name);
        lib_free(st_name);
    }
}

void uiplus4cart_proc(WPARAM wparam, HWND hwnd)
{
    switch (wparam) {
        case IDM_CART_SMART_ATTACH:
        case IDM_CART_ATTACH_C0LO:
        case IDM_CART_ATTACH_C0HI:
        case IDM_CART_ATTACH_C1LO:
        case IDM_CART_ATTACH_C1HI:
        case IDM_CART_ATTACH_C2LO:
        case IDM_CART_ATTACH_C2HI:
            uiplus4cart_attach(wparam, hwnd, plus4_ui_cartridges);
            break;
        case IDM_CART_DETACH:
            cartridge_detach_image(-1);
            break;
    }
}

void uiplus4cart_init(void)
{
}
