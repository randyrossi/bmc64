/*
 * vsidui_sdl.h - SDL extensions to VSID UI API.
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

#ifndef VICE_VSID_UI_SDL_H
#define VICE_VSID_UI_SDL_H

#include "vice.h"

#define SDL_VSID_ACTIVE  (1 << 0)
#define SDL_VSID_REPAINT (1 << 1)
extern int sdl_vsid_state;

typedef void (*ui_draw_func_t) (void);

extern void sdl_vsid_activate(void);
extern void sdl_vsid_close(void);
extern void sdl_vsid_draw_init(ui_draw_func_t func);
extern void sdl_vsid_draw(void);

#endif
