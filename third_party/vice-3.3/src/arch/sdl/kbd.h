/*
 * kbd.h - SDL specfic keyboard driver.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README file for copyright notice.
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

#ifndef VICE_KBD_H
#define VICE_KBD_H

#include "vice_sdl.h"
#include "uimenu.h"

extern void kbd_arch_init(void);
extern int kbd_arch_get_host_mapping(void);

extern signed long kbd_arch_keyname_to_keynum(char *keyname);
extern const char *kbd_arch_keynum_to_keyname(signed long keynum);
extern void kbd_initialize_numpad_joykeys(int *joykeys);

#define KBD_PORT_PREFIX "sdl"

#ifdef USE_SDLUI2
#define SDLKey SDL_Keycode
#define SDLMod SDL_Keymod
#define SDLK_KP0 SDLK_KP_0
#define SDLK_KP1 SDLK_KP_1
#define SDLK_KP2 SDLK_KP_2
#define SDLK_KP3 SDLK_KP_3
#define SDLK_KP4 SDLK_KP_4
#define SDLK_KP5 SDLK_KP_5
#define SDLK_KP6 SDLK_KP_6
#define SDLK_KP7 SDLK_KP_7
#define SDLK_KP8 SDLK_KP_8
#define SDLK_KP9 SDLK_KP_9
#define KMOD_META KMOD_GUI
#define SDLK_LMETA SDLK_LGUI
#define SDLK_RMETA SDLK_RGUI
#define SDLK_COMPOSE	SDLK_APPLICATION
#define SDLK_PRINT SDLK_PRINTSCREEN
#define SDLK_NUMLOCK SDLK_NUMLOCKCLEAR
#define SDLK_SCROLLOCK SDLK_SCROLLLOCK
#else
#define SDL_NUM_SCANCODES   512 /* this must be the same value as in SDL2 headers */
#endif

#define VICE_SDLK_RIGHT     275
#define VICE_SDLK_LEFT      276
#define VICE_SDLK_HOME      278
#define VICE_SDLK_END       279
#define VICE_SDLK_F10       291

#define VICE_SDLK_BACKSPACE   SDLK_BACKSPACE
#define VICE_SDLK_ESCAPE      SDLK_ESCAPE
#define VICE_SDLK_RETURN      SDLK_RETURN

extern SDLKey SDL2x_to_SDL1x_Keys(SDLKey key);
extern SDLKey SDL1x_to_SDL2x_Keys(SDLKey key);

extern ui_menu_action_t sdlkbd_press(SDLKey key, SDLMod mod);
extern ui_menu_action_t sdlkbd_release(SDLKey key, SDLMod mod);

extern void sdlkbd_set_hotkey(SDLKey key, SDLMod mod, ui_menu_entry_t *value);

extern int sdlkbd_hotkeys_load(const char *filename);
extern int sdlkbd_hotkeys_load(const char *filename);
extern int sdlkbd_hotkeys_dump(const char *filename);

extern int sdlkbd_init_resources(void);
extern void sdlkbd_resources_shutdown(void);

extern int sdlkbd_init_cmdline(void);

extern void kbd_enter_leave(void);
extern void kbd_focus_change(void);

extern int sdl_ui_menukeys[];

extern const char *kbd_get_menu_keyname(void);

#endif
