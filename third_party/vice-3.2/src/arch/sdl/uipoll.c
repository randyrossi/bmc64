/*
 * uipoll.c - UI key/button polling.
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

#include "vice.h"
#include "types.h"

#include "vice_sdl.h"
#include <stdio.h>

#include "joy.h"
#include "kbd.h"
#include "lib.h"
#include "ui.h"
#include "uimenu.h"
#include "uipoll.h"

/* ------------------------------------------------------------------ */
/* static functions */

static inline int is_not_modifier(SDLKey k)
{
    return ((k != SDLK_RSHIFT) &&
            (k != SDLK_LSHIFT) &&
            (k != SDLK_RMETA) &&
            (k != SDLK_LMETA) &&
            (k != SDLK_RCTRL) &&
            (k != SDLK_LCTRL) &&
            (k != SDLK_RALT) &&
            (k != SDLK_LALT)) ? 1 : 0;
}

static void sdl_poll_print_timeout(int x, int y, int time)
{
    char *timestr = NULL;

    timestr = lib_msprintf("Timeout in %i...", time);
    sdl_ui_print(timestr, x, y);
    sdl_ui_refresh();
    lib_free(timestr);
}

/* ------------------------------------------------------------------ */
/* External interface */

SDL_Event sdl_ui_poll_event(const char *what, const char *target, int options, int timeout)
{
    SDL_Event e;

    int count = 0;
    int polling = 1;
    int i;

    int allow_keyboard = options & SDL_POLL_KEYBOARD;
    int allow_modifier = options & SDL_POLL_MODIFIER;
#ifdef HAVE_SDL_NUMJOYSTICKS
    int allow_joystick = options & SDL_POLL_JOYSTICK;
#endif

    sdl_ui_clear();
    i = sdl_ui_print("Polling ", 0, 0);
    i = i + sdl_ui_print(what, i, 0);
    sdl_ui_print(" for:", i, 0);
    sdl_ui_print(target, 0, 1);

    if (timeout > 0) {
        sdl_poll_print_timeout(0, 2, timeout);
    }

    /* TODO check if key/event is suitable */
#ifdef ANDROID_COMPILE
    e.type = SDL_USEREVENT;
    polling = 0;
#else
    while (polling) {
        while (polling && SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_KEYDOWN:
                    if (allow_keyboard && (allow_modifier || is_not_modifier(e.key.keysym.sym))) {
                        polling = 0;
                    }
                    break;
#ifdef HAVE_SDL_NUMJOYSTICKS
                case SDL_JOYBUTTONDOWN:
                    if (allow_joystick) {
                        polling = 0;
                    }
                    break;
                case SDL_JOYHATMOTION:
                    if (allow_joystick && (sdljoy_check_hat_movement(e) != 0)) {
                        polling = 0;
                    }
                    break;
                case SDL_JOYAXISMOTION:
                    if (allow_joystick && (sdljoy_check_axis_movement(e) != 0)) {
                        polling = 0;
                    }
                    break;
#endif
                default:
                    ui_handle_misc_sdl_event(e);
                    break;
            }
        }
        SDL_Delay(20);

        if ((timeout > 0) && (++count == 1000 / 20)) {
            count = 0;
            if (--timeout == 0) {
                e.type = SDL_USEREVENT;
                polling = 0;
            } else {
                sdl_poll_print_timeout(0, 2, timeout);
            }
        }
    }
#endif

    if (polling == 1) {
        e.type = SDL_USEREVENT;
    }

    return e;
}
