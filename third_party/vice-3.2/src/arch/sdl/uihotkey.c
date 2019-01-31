/*
 * uihotkey.c - UI hotkey functions.
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

#include "joy.h"
#include "lib.h"
#include "kbd.h"
#include "ui.h"
#include "uihotkey.h"
#include "uimenu.h"
#include "uipoll.h"
#include "util.h"

#include "vice_sdl.h"
#include <string.h>

/* ------------------------------------------------------------------ */
/* static functions */

static char* sdl_ui_hotkey_path_find(ui_menu_entry_t *action, ui_menu_entry_t *menu)
{
    char *p = NULL;
    char *q = NULL;

    while (menu->string) {
        if (menu == action) {
            return util_concat(menu->string, NULL);
        }
        if ((menu->type) == MENU_ENTRY_SUBMENU) {
            p = sdl_ui_hotkey_path_find(action, (ui_menu_entry_t *)menu->data);
            if (p) {
                q = util_concat(menu->string, SDL_UI_HOTKEY_DELIM, p, NULL);
                lib_free(p);
                return q;
            }
        }
        ++menu;
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/* External interface */

char *sdl_ui_hotkey_path(ui_menu_entry_t *action)
{
    return sdl_ui_hotkey_path_find(action, sdl_ui_get_main_menu());
}

ui_menu_entry_t *sdl_ui_hotkey_action(char *path)
{
    ui_menu_entry_t *menupos = sdl_ui_get_main_menu();
    char *p;

    if (menupos == NULL) {
        return NULL;
    }

    p = strtok(path, SDL_UI_HOTKEY_DELIM);

    if (p == NULL) {
        return NULL;
    }

    while (menupos->string) {
        if (strcmp(p, menupos->string) == 0) {
            p = strtok(NULL, SDL_UI_HOTKEY_DELIM);
            if (p == NULL) {
                return menupos;
            } else {
                if (menupos->type == MENU_ENTRY_SUBMENU) {
                    menupos = (ui_menu_entry_t *)menupos->data;
                } else {
                    return NULL;
                }
            }
        } else {
            ++menupos;
        }
    }
    return NULL;
}

int sdl_ui_hotkey_map(ui_menu_entry_t *item)
{
    SDL_Event e;

    if (item == NULL) {
        return -1;
    }

    /* Use text item for unsetting hotkeys */
    if (item->type == MENU_ENTRY_TEXT) {
        item = NULL;
    }

    e = sdl_ui_poll_event("hotkey", item ? item->string : "(unmap hotkey)", SDL_POLL_JOYSTICK | SDL_POLL_KEYBOARD, 5);

    /* TODO check if key/event is suitable */
    switch (e.type) {
        case SDL_KEYDOWN:
            sdlkbd_set_hotkey(SDL2x_to_SDL1x_Keys(e.key.keysym.sym), e.key.keysym.mod, item);
            break;
#ifdef HAVE_SDL_NUMJOYSTICKS
        case SDL_JOYAXISMOTION:
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYHATMOTION:
            sdljoy_set_hotkey(e, item);
            break;
#endif
        default:
            break;
    }
    return 1;
}
