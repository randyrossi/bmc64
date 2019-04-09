/*
 * uiimagefilereq.c - Functions to select a file inside a D64/T64 Image
 *
 * Written by
 *  groepaz <groepaz@gmx.net>
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
#include <string.h>

#include "archdep.h"
#include "diskcontents.h"
#include "tapecontents.h"
#include "imagecontents.h"
#include "ioutil.h"
#include "lib.h"
#include "ui.h"
#include "uimenu.h"
#include "uifilereq.h"
#include "util.h"
#include "menu_common.h"

#define SDL_FILEREQ_META_NUM 0

#define IMAGE_FIRST_Y 1

static menu_draw_t *menu_draw;

static void sdl_ui_image_file_selector_redraw(image_contents_t *contents, const char *title, int offset, int num_items, int more, ui_menu_filereq_mode_t mode, int cur_offset)
{
    int i, j;
    char* title_string;
    char* name;
    uint8_t oldbg;
    image_contents_file_list_t *entry;

    title_string = image_contents_to_string(contents, 0);
    
    sdl_ui_clear();
    sdl_ui_display_title(title_string);
    lib_free(title_string);

    entry = contents->file_list;
    for (i = 0; i < offset; ++i) {
        entry = entry->next;
    }
    for (i = 0; i < num_items; ++i) {
        if (i == cur_offset) {
            oldbg = sdl_ui_set_cursor_colors();
        }

        j = MENU_FIRST_X;
        name = image_contents_file_to_string(entry, IMAGE_CONTENTS_STRING_PETSCII);
        j += sdl_ui_print(name, j, i + IMAGE_FIRST_Y + SDL_FILEREQ_META_NUM);

        if (i == cur_offset) {
            sdl_ui_print_eol(j, i + IMAGE_FIRST_Y + SDL_FILEREQ_META_NUM);
            sdl_ui_reset_cursor_colors(oldbg);
        }
        entry = entry->next;
    }
    name = lib_msprintf("%d BLOCKS FREE.", contents->blocks_free);
    sdl_ui_print(name, MENU_FIRST_X, i + IMAGE_FIRST_Y + SDL_FILEREQ_META_NUM);
    lib_free(name);
}

static void sdl_ui_image_file_selector_redraw_cursor(image_contents_t *contents, int offset, int num_items, ui_menu_filereq_mode_t mode, int cur_offset, int old_offset)
{
    int i, j;
    char* name;
    uint8_t oldbg = 0;
    image_contents_file_list_t *entry;

    entry = contents->file_list;
    for (i = 0; i < offset; ++i) {
        entry = entry->next;
    }
    for (i = 0; i < num_items; ++i) {
        if ((i == cur_offset) || (i == old_offset)){
            if (i == cur_offset) {
                oldbg = sdl_ui_set_cursor_colors();
            }
            j = MENU_FIRST_X;
            name = image_contents_file_to_string(entry, IMAGE_CONTENTS_STRING_PETSCII);
            j += sdl_ui_print(name, j, i + IMAGE_FIRST_Y + SDL_FILEREQ_META_NUM);

            sdl_ui_print_eol(j, i + IMAGE_FIRST_Y + SDL_FILEREQ_META_NUM);
            if (i == cur_offset) {
                sdl_ui_reset_cursor_colors(oldbg);
            }
        }
        entry = entry->next;
    }
}

/* ------------------------------------------------------------------ */
/* External UI interface */

int sdl_ui_image_file_selection_dialog(const char* filename, ui_menu_filereq_mode_t mode)
{
    int total, dirs = 0, files, menu_max;
    int active = 1;
    int offset = 0;
    int redraw = 1;
    int cur = 0, cur_old = -1;

    image_contents_t *contents = NULL;
    image_contents_file_list_t *entry;
    int retval = -1;

    menu_draw = sdl_ui_get_menu_param();

    /* FIXME: it might be a good idea to wrap this into a common imagecontents_read */
    contents = tapecontents_read(filename);
    if (contents == NULL) {
        contents = diskcontents_read(filename, 0);
        if (contents == NULL) {
            return 0;
        }
    }

    /* count files in the list */
    files = 0;
    for (entry = contents->file_list; entry != NULL; entry = entry->next) {
        files++;
    }

    total = dirs + files + SDL_FILEREQ_META_NUM;
    menu_max = menu_draw->max_text_y - (IMAGE_FIRST_Y + SDL_FILEREQ_META_NUM);
    if (menu_max > 0) { menu_max--; } /* make room for BLOCKS FREE */

    while (active) {

        sdl_ui_set_active_font(MENU_FONT_IMAGES);

        if (redraw) {
            sdl_ui_image_file_selector_redraw(contents, filename, offset, 
                (total - offset > menu_max) ? menu_max : total - offset, 
                (total - offset > menu_max) ? 1 : 0, mode, cur);
            redraw = 0;
        } else {
            sdl_ui_image_file_selector_redraw_cursor(contents, offset, 
                    (total - offset > menu_max) ? menu_max : total - offset, 
                    mode, cur, cur_old);
        }

        sdl_ui_set_active_font(MENU_FONT_ASCII);

        sdl_ui_refresh();

        switch (sdl_ui_menu_poll_input()) {
            case MENU_ACTION_HOME:
                cur_old = cur;
                cur = 0;
                offset = 0;
                redraw = 1;
                break;

            case MENU_ACTION_END:
                cur_old = cur;
                if (total < (menu_max - 1)) {
                    cur = total - 1;
                    offset = 0;
                } else {
                    cur = menu_max - 1;
                    offset = total - menu_max;
                }
                redraw = 1;
                break;

            case MENU_ACTION_UP:
                if (cur > 0) {
                    cur_old = cur;
                    --cur;
                } else {
                    if (offset > 0) {
                        offset--;
                        redraw = 1;
                    }
                }
                break;

            case MENU_ACTION_PAGEUP:
            case MENU_ACTION_LEFT:
                offset -= menu_max;
                if (offset < 0) {
                    offset = 0;
                    cur_old = -1;
                    cur = 0;
                }
                redraw = 1;
                break;

            case MENU_ACTION_DOWN:
                if (cur < (menu_max - 1)) {
                    if ((cur + offset) < total - 1) {
                        cur_old = cur;
                        ++cur;
                    }
                } else {
                    if (offset < (total - menu_max)) {
                        offset++;
                        redraw = 1;
                    }
                }
                break;

            case MENU_ACTION_PAGEDOWN:
            case MENU_ACTION_RIGHT:
                offset += menu_max;
                if (offset >= total) {
                    offset = total - 1;
                    cur_old = -1;
                    cur = 0;
                } else if ((cur + offset) >= total) {
                    cur_old = -1;
                    cur = total - offset - 1;
                }
                redraw = 1;
                break;

            case MENU_ACTION_SELECT:
                active = 0;
                retval = offset + cur - dirs - SDL_FILEREQ_META_NUM + 1;
                break;

            case MENU_ACTION_CANCEL:
            case MENU_ACTION_EXIT:
                active = 0;
                break;

            default:
                SDL_Delay(10);
                break;
        }
    }

    return retval;
}
