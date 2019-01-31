/*
 * uicart.c
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
#include "intl.h"
#include "lib.h"
#include "translate.h"
#include "ui.h"
#include "uicart.h"
#include "uilib.h"

void uicart_attach(video_canvas_t *canvas, int idm, const uicart_params_t *cartridges)
{
    int i;
    char *name;

    i = 0;

    while ((cartridges[i].idm != idm) && (cartridges[i].idm != 0)) {
        i++;
    }

    if (cartridges[i].idm == 0) {
        ui_error(translate_text(IDMES_BAD_CART_CONFIG_IN_UI));
        return;
    }

    if ((name = uilib_select_file(translate_text(cartridges[i].title),
                                  cartridges[i].filter, UILIB_SELECTOR_TYPE_FILE_LOAD,
                                  UILIB_SELECTOR_STYLE_CART)) != NULL) {
        if (cartridge_attach_image(cartridges[i].type, name) < 0) {
            ui_error(translate_text(IDMES_INVALID_CART_IMAGE));
        }
        lib_free(name);
    }
}

void uicart_attach_special(video_canvas_t *canvas, char *title, uint32_t filterlist, unsigned int type)
{
    char *name;

    if ((name = uilib_select_file(title, filterlist, UILIB_SELECTOR_TYPE_FILE_LOAD, UILIB_SELECTOR_STYLE_CART)) != NULL) {
        if (cartridge_attach_image(type, name) < 0) {
            ui_error(translate_text(IDMES_INVALID_CART_IMAGE));
        }
        lib_free(name);
    }
}
