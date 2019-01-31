/*
 * uic64cart.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#include "private.h"
#include "vice.h"
#include "cartridge.h"
#include "c64cart.h"
#include "intl.h"
#include "keyboard.h"
#include "translate.h"
#include "ui.h"
#include "uic64cart.h"
#include "uicart.h"
#include "uilib.h"
#include "uires.h"

static const uicart_params_t c64_ui_cartridges[] = {
    {
        IDM_CART_ATTACH_CRT,
        CARTRIDGE_CRT,
        IDS_ATTACH_CRT_IMAGE,
        UILIB_FILTER_ALL | UILIB_FILTER_CRT
    },
    {
        0, 0, 0, 0
    }
};

static void uic64cart_attach(video_canvas_t *canvas, int idm, const uicart_params_t *cartridges)
{
    uicart_attach(canvas, idm, cartridges);
}

void uic64cart_proc(video_canvas_t *canvas, int idm)
{
    switch (idm) {
        case IDM_CART_ATTACH_CRT:
            uic64cart_attach(canvas, idm, c64_ui_cartridges);
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
