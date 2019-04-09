/*
 * menu_common.h - Common SDL menu functions.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * Based on code by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#ifndef VICE_MENU_COMMON_H
#define VICE_MENU_COMMON_H

#include "vice.h"
#include "types.h"
#include "uifonts.h"
#include "uimenu.h"

#define MENU_MENUHEADERLEFT_STRING UIFONT_MENUHEADERLEFT_STRING
#define MENU_MENUHEADERRIGHT_STRING UIFONT_MENUHEADERRIGHT_STRING

#define MENU_SUBMENU_STRING UIFONT_SUBMENU_STRING

#define MENU_CHECKMARK_UNCHECKED_STRING UIFONT_CHECKMARK_UNCHECKED_STRING
#define MENU_CHECKMARK_UNCHECKED_CHAR UIFONT_CHECKMARK_UNCHECKED_CHAR
#define MENU_CHECKMARK_CHECKED_STRING UIFONT_CHECKMARK_CHECKED_STRING

#define MENU_NOT_AVAILABLE_STRING "(N/A)"

#define MENU_UNKNOWN_STRING "?"
#define MENU_EXIT_UI_STRING "\1"

extern const char *sdl_menu_text_tick;
extern const char *sdl_menu_text_unknown;
extern const char *sdl_menu_text_exit_ui;

#define SDL_MENU_ITEM_SEPARATOR { "", MENU_ENTRY_TEXT, seperator_callback, NULL }
#define SDL_MENU_ITEM_TITLE(title) { title, MENU_ENTRY_TEXT, seperator_callback, (ui_callback_data_t)1 }

#define UI_MENU_CALLBACK(name) \
    const char *name(int activated, ui_callback_data_t param)

#define UI_MENU_DEFINE_TOGGLE(resource)                              \
    static UI_MENU_CALLBACK(toggle_##resource##_callback)            \
    {                                                                \
        return sdl_ui_menu_toggle_helper(activated, #resource);      \
    }

#define UI_MENU_DEFINE_RADIO(resource)                                \
    static UI_MENU_CALLBACK(radio_##resource##_callback)              \
    {                                                                 \
        return sdl_ui_menu_radio_helper(activated, param, #resource); \
    }

#define UI_MENU_DEFINE_STRING(resource)                                \
    static UI_MENU_CALLBACK(string_##resource##_callback)              \
    {                                                                  \
        return sdl_ui_menu_string_helper(activated, param, #resource); \
    }

#define UI_MENU_DEFINE_INT(resource)                                \
    static UI_MENU_CALLBACK(int_##resource##_callback)              \
    {                                                               \
        return sdl_ui_menu_int_helper(activated, param, #resource); \
    }

#define UI_MENU_DEFINE_FILE_STRING(resource)                                \
    static UI_MENU_CALLBACK(file_string_##resource##_callback)              \
    {                                                                       \
        return sdl_ui_menu_file_string_helper(activated, param, #resource); \
    }

#define UI_MENU_DEFINE_SLIDER(resource, min, max)                                  \
    static UI_MENU_CALLBACK(slider_##resource##_callback)                          \
    {                                                                              \
        return sdl_ui_menu_slider_helper(activated, param, #resource, min, max);   \
    }


extern UI_MENU_CALLBACK(submenu_callback);
extern UI_MENU_CALLBACK(submenu_radio_callback);
extern UI_MENU_CALLBACK(seperator_callback);

extern const char *sdl_ui_menu_toggle_helper(int activated, const char *resource_name);
extern const char *sdl_ui_menu_radio_helper(int activated, ui_callback_data_t param, const char *resource_name);
extern const char *sdl_ui_menu_string_helper(int activated, ui_callback_data_t param, const char *resource_name);
extern const char *sdl_ui_menu_int_helper(int activated, ui_callback_data_t param, const char *resource_name);
extern const char *sdl_ui_menu_file_string_helper(int activated, ui_callback_data_t param, const char *resource_name);
extern const char *sdl_ui_menu_slider_helper(int activated, ui_callback_data_t param, const char *resource_name, const int min, const int max);

extern UI_MENU_CALLBACK(autostart_callback);
extern UI_MENU_CALLBACK(pause_callback);
extern UI_MENU_CALLBACK(advance_frame_callback);
extern UI_MENU_CALLBACK(vkbd_callback);
extern UI_MENU_CALLBACK(statusbar_callback);
extern UI_MENU_CALLBACK(quit_callback);

#endif
