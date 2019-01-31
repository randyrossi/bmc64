/*
 * vkbd.h - SDL virtual keyboard.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Mike Dawson <mike@gp2x.org>
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

#ifndef VICE_VKBD_H
#define VICE_VKBD_H

#include "vice.h"
#include "types.h"

#include "uimenu.h"

typedef struct vkbd_s {
    const char **keyb;
    const uint8_t *keytable;
    const uint8_t shift;
} vkbd_t;

extern void sdl_vkbd_set_vkbd(const vkbd_t *machine_vkbd);

extern vkbd_t vkbd_c64;
extern vkbd_t vkbd_vic20;
extern vkbd_t vkbd_c64dtv;
extern vkbd_t vkbd_c128;
extern vkbd_t vkbd_plus4;
extern vkbd_t vkbd_cbm2;
extern vkbd_t vkbd_pet_uk;
extern vkbd_t vkbd_pet_gr;

#define SDL_VKBD_ACTIVE  (1 << 0)
#define SDL_VKBD_REPAINT (1 << 1)
extern int sdl_vkbd_state;

extern void sdl_vkbd_activate(void);
extern void sdl_vkbd_close(void);
extern void sdl_vkbd_draw(void);
extern int sdl_vkbd_process(ui_menu_action_t input);

#endif
