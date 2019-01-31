/*
 * uipoll.h - UI key/button polling.
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

#ifndef VICE_UIPOLL_H
#define VICE_UIPOLL_H

#include "vice.h"
#include "types.h"

#include "vice_sdl.h"

#include "uimenu.h"

#define SDL_POLL_JOYSTICK (1 << 0)
#define SDL_POLL_KEYBOARD (1 << 1)
#define SDL_POLL_MODIFIER (1 << 2)

extern SDL_Event sdl_ui_poll_event(const char *what, const char *target, int options, int timeout);

#endif
