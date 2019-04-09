/*
 * menu_common.c - Common SDL menu functions.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * Based on code by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include <stdlib.h>
#include <string.h>

#include "archdep.h"
#include "autostart.h"
#include "lib.h"
#include "menu_common.h"
#include "monitor.h"
#include "resources.h"
#include "ui.h"
#include "uifilereq.h"
#include "uimenu.h"
#include "vkbd.h"
#include "vsyncapi.h"


/* ------------------------------------------------------------------ */
/* Common strings */

const char* sdl_menu_text_tick = MENU_CHECKMARK_CHECKED_STRING;
const char* sdl_menu_text_unknown = MENU_UNKNOWN_STRING;
const char* sdl_menu_text_exit_ui = MENU_EXIT_UI_STRING;

/* ------------------------------------------------------------------ */
/* Common callbacks */

UI_MENU_CALLBACK(submenu_callback)
{
    return MENU_SUBMENU_STRING;
}

UI_MENU_CALLBACK(submenu_radio_callback)
{
    static char buf[100] = MENU_SUBMENU_STRING " ";
    char *dest = &(buf[3]);
    const char *src = NULL;
    ui_menu_entry_t *item = (ui_menu_entry_t *)param;

    while (item->string != NULL) {
        if (item->callback(0, item->data) != NULL) {
            src = item->string;
            break;
        }
        ++item;
    }

    if (src == NULL) {
        return MENU_SUBMENU_STRING " ???";
    }

    while ((*dest++ = *src++)) {
    }

    return buf;
}

UI_MENU_CALLBACK(seperator_callback)
{
    return NULL;
}

UI_MENU_CALLBACK(autostart_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose autostart image", FILEREQ_MODE_CHOOSE_FILE_IN_IMAGE);
        if (name != NULL) {
            /* FIXME: using last_selected_image_pos is kindof a hack */
            if (autostart_autodetect(name, NULL, last_selected_image_pos, AUTOSTART_MODE_RUN) < 0) {
                ui_error("could not start auto-image");
            }
            lib_free(name);
            return sdl_menu_text_exit_ui;
        }
    }
    return NULL;
}

UI_MENU_CALLBACK(pause_callback)
{
    int paused = ui_emulation_is_paused();

    if (activated) {
        ui_pause_emulation(!paused);
        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

UI_MENU_CALLBACK(advance_frame_callback)
{
    int paused = ui_emulation_is_paused();

    if (activated) {
        if (paused) {
            vsyncarch_advance_frame();
        } else {
            ui_pause_emulation(1);
        }
        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

UI_MENU_CALLBACK(vkbd_callback)
{
    if (activated) {
        sdl_vkbd_activate();
        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

UI_MENU_CALLBACK(statusbar_callback)
{
    return sdl_ui_menu_toggle_helper(activated, "SDLStatusbar");
}

UI_MENU_CALLBACK(quit_callback)
{
    if (activated) {
        ui_sdl_quit();
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Menu helpers */

const char *sdl_ui_menu_toggle_helper(int activated, const char *resource_name)
{
    int value, r;

    if (activated) {
        r = resources_toggle(resource_name, &value);
        if (r < 0) {
            r = resources_get_int(resource_name, &value);
        }
    } else {
        r = resources_get_int(resource_name, &value);
    }

    if (r < 0) {
        return sdl_menu_text_unknown;
    } else {
        return value ? sdl_menu_text_tick : NULL;
    }
}

const char *sdl_ui_menu_radio_helper(int activated, ui_callback_data_t param, const char *resource_name)
{
    if (activated) {
        if (resources_query_type(resource_name) == RES_INTEGER) {
            resources_set_int(resource_name, vice_ptr_to_int(param));
        } else {
            resources_set_string(resource_name, (char *)param);
        }
    } else {
        int v;
        const char *w;
        if (resources_query_type(resource_name) == RES_INTEGER) {
            resources_get_int(resource_name, &v);
            if (v == vice_ptr_to_int(param)) {
                return sdl_menu_text_tick;
            }
        } else {
            resources_get_string(resource_name, &w);
            if (!strcmp(w, (char *)param)) {
                return sdl_menu_text_tick;
            }
        }
    }
    return NULL;
}

const char *sdl_ui_menu_string_helper(int activated, ui_callback_data_t param, const char *resource_name)
{
    char *value = NULL;
    const char *previous = NULL;

    if (resources_get_string(resource_name, &previous)) {
        return sdl_menu_text_unknown;
    }

    if (activated) {
        value = sdl_ui_text_input_dialog((const char*)param, previous);
        if (value) {
            resources_set_value_string(resource_name, value);
            lib_free(value);
        }
    } else {
        return previous;
    }
    return NULL;
}

const char *sdl_ui_menu_int_helper(int activated, ui_callback_data_t param, const char *resource_name)
{
    static char buf[20];
    char *value = NULL;
    int previous, new_value;

    if (resources_get_int(resource_name, &previous)) {
        return sdl_menu_text_unknown;
    }

    sprintf(buf, "%i", previous);

    if (activated) {
        value = sdl_ui_text_input_dialog((const char*)param, buf);
        if (value) {
            new_value = strtol(value, NULL, 0);
            resources_set_int(resource_name, new_value);
            lib_free(value);
        }
    } else {
        return buf;
    }
    return NULL;
}

#if (FSDEV_DIR_SEP_CHR == '\\')
char win32_path_buf[80];

static char *sdl_ui_menu_file_translate_seperator(const char *text)
{
    int len;
    int i;

    len = strlen(text);

    for (i = 0; i < len; i++) {
        if (text[i] == '\\') {
            win32_path_buf[i] = '/';
        } else {
            win32_path_buf[i] = text[i];
        }
    }
    win32_path_buf[i] = 0;
    return win32_path_buf;
}
#endif

const char *sdl_ui_menu_file_string_helper(int activated, ui_callback_data_t param, const char *resource_name)
{
    char *value = NULL;
    const char *previous = NULL;

    if (resources_get_string(resource_name, &previous)) {
        return sdl_menu_text_unknown;
    }

    if (activated) {
        value = sdl_ui_file_selection_dialog((const char*)param, FILEREQ_MODE_CHOOSE_FILE);
        if (value) {
            resources_set_value_string(resource_name, value);
            lib_free(value);
        }
    } else {
#if (FSDEV_DIR_SEP_CHR == '\\')
        if (previous != NULL && previous[0] != 0) {
            return (const char *)sdl_ui_menu_file_translate_seperator(previous);
        }
#endif
        return previous;
    }
    return NULL;
}

const char *sdl_ui_menu_slider_helper(int activated, ui_callback_data_t param, const char *resource_name, const int min, const int max)
{
    static char buf[20];
    int previous, new_value;

    if (resources_get_int(resource_name, &previous)) {
        return sdl_menu_text_unknown;
    }

    sprintf(buf, "%i", previous);

    if (activated) {
        new_value = sdl_ui_slider_input_dialog((const char *)param, previous, min, max);
        if (new_value != previous) {
            resources_set_int(resource_name, new_value);
        }
    } else {
        return buf;
    }

    return NULL;
}
