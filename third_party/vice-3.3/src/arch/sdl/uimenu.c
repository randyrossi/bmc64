/*
 * uimenu.c - Common SDL menu functions.
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

#include "archdep.h"
#include "interrupt.h"
#include "ioutil.h"
#include "joy.h"
#include "kbd.h"
#include "lib.h"
#include "machine.h"
#include "menu_common.h"
#include "raster.h"
#include "resources.h"
#include "screenshot.h"
#include "sound.h"
#include "types.h"
#include "ui.h"
#include "uihotkey.h"
#include "uimenu.h"
#include "util.h"
#include "video.h"
#include "videoarch.h"
#include "vkbd.h"
#include "vsidui_sdl.h"
#include "vsync.h"

#ifdef ANDROID_COMPILE
#include "loader.h"
#endif

#include "vice_sdl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int sdl_menu_state = 0;

void (*sdl_ui_set_menu_params)(int index, menu_draw_t *menu_draw);

/* ------------------------------------------------------------------ */
/* static variables */

static ui_menu_entry_t *main_menu = NULL;

static uint16_t sdl_active_translation[256];
static uint16_t sdl_menu_translation[256];
static uint16_t sdl_image_translation[256];
static uint16_t sdl_monitor_translation[256];

/** \brief  Reference to draw buffer backup
 *
 * Used to properly clean up when 'Quit emu' is triggered
 */
static uint8_t *draw_buffer_backup = NULL;

/** \brief  Reference to menu offsets allocated in sdl_ui_menu_get_offsets()
 *
 * Used to properly clean up when 'Quit emu' is triggered
 */
static int *menu_offsets = NULL;


static menufont_t activefont = { NULL, sdl_active_translation, 0, 0 };
static menufont_t menufont = { NULL, sdl_menu_translation, 0, 0 };
static menufont_t imagefont = { NULL, sdl_image_translation, 0, 0 };
static menufont_t monitorfont = { NULL, sdl_monitor_translation, 0, 0 };

static menu_draw_t menu_draw = {
    0, 0,   /* pitch, offset */
    40, 25, /* max_text_x, max_text_y */
    0, 0,   /* extra_x, extra_y */
    0, 1,   /* color_back, color_front */
    0, 1,   /* color_default_back, color_default_front */
    6, 0,   /* color_cursor_back, color_cursor_revers */
    13, 2,  /* color_active_green, color_inactive_red */
    15, 11  /* color_active_grey, color_inactive_grey */
};

/* charset #1 - standard ascii. this is used by the menus and file browser */
static const uint8_t sdl_char_to_screen[256] = {
    /* 00-1f non printable (01-1f reserved for ui-gfx) */
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,

    /* 20-7f ascii */

    /* 20-2f  !"#$%&´()*+,-./ */
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    /* 30-3f 0123456789:;<=>? */
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    /* 40-4f @ABCDEFGHIJKLMNO */
    0x00, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    /* 50-5f PQRSTUVWXYZ[\]^_ */
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    /* 60-6f `abcdefghijklmno */
    0x40, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    /* 70-7f pqrstuvwxyz{|}~  */ /* 7f non printable */
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,

    /* 80-ff extended ascii */
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff
};

/* charset #2 - uppercase petscii. this is used when browsing inside emulator images */
static const uint8_t sdl_char_to_screen_uppercase[256] = {
    /* 00-1f non printable */
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,

    /* 20-2f  !"#$%&´()*+,-./ */
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 
    /* 30-3f 0123456789:;<=>? */
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 
    /* 40-4f @ABCDEFGHIJKLMNO */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 
    /* 50-5f PQRSTUVWXYZ[\]^_ */
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 
    /* 60-6f @ABCDEFGHIJKLMNO (shifted)*/
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 
    /* 70-7f PQRSTUVWXYZ[\]^_ (shifted) */
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,

    /* 80-9f non printable */
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 

    /* a0-bf gfx chars */
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 

    /* c0-cf @ABCDEFGHIJKLMNO (shifted) alternatives */
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 
    /* d0-df PQRSTUVWXYZ[\]^_ (shifted) alternatives */
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,

    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x5e
};

/* charset #3 - lowercase petscii. this is used by the monitor and virtual keyboard */
static const uint8_t sdl_char_to_screen_monitor[256] = {
    /* 00-1f non printable */
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,

    /* 20-2f  !"#$%&´()*+,-./ */
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 
    /* 30-3f 0123456789:;<=>? */
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 
    /* 40-4f @ABCDEFGHIJKLMNO */
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 
    /* 50-5f PQRSTUVWXYZ[\]^_ */
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 
    /* 60-6f @ABCDEFGHIJKLMNO (shifted)*/
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 
    /* 70-7f PQRSTUVWXYZ[\]^_ (shifted) */
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,

    /* 80-9f non printable */
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf, 

    /* a0-bf gfx chars */
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f, 

    /* c0-cf @ABCDEFGHIJKLMNO (shifted) alternatives */
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f, 
    /* d0-df PQRSTUVWXYZ[\]^_ (shifted) alternatives */
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,

    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f, 
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f
};

/* ------------------------------------------------------------------ */
/* static functions */

static ui_menu_retval_t sdl_ui_menu_item_activate(ui_menu_entry_t *item);

static void sdl_ui_putchar(uint8_t c, int pos_x, int pos_y)
{
    int x, y;
    uint8_t fontchar;
    uint8_t *font_pos;
    uint8_t *draw_pos;

    font_pos = &(activefont.font[activefont.translate[(int)c]]);

    if (machine_class == VICE_MACHINE_VSID) {
        draw_pos = &(sdl_active_canvas->draw_buffer_vsid->draw_buffer[pos_x * activefont.w + pos_y * activefont.h * menu_draw.pitch]);
    } else {
        draw_pos = &(sdl_active_canvas->draw_buffer->draw_buffer[pos_x * activefont.w + pos_y * activefont.h * menu_draw.pitch]);
    }

    draw_pos += menu_draw.offset;

    for (y = 0; y < activefont.h; ++y) {
        fontchar = *font_pos;
        for (x = 0; x < activefont.w; ++x) {
            draw_pos[x] = (fontchar & (0x80 >> x)) ? menu_draw.color_front : menu_draw.color_back;
        }
        ++font_pos;
        draw_pos += menu_draw.pitch;
    }
}

static int sdl_ui_print_wrap(const char *text, int pos_x, int *pos_y_ptr)
{
    int i = 0;
    int pos_y = *pos_y_ptr;
    uint8_t c;

    if (text == NULL) {
        return 0;
    }

    while (pos_x >= menu_draw.max_text_x) {
        pos_x -= menu_draw.max_text_x;
        ++pos_y;
    }

    while ((c = text[i]) != 0) {
        if (pos_x == menu_draw.max_text_x) {
            ++pos_y;
            pos_x = 0;
        }

        if (pos_y == menu_draw.max_text_y) {
            sdl_ui_scroll_screen_up();
            --pos_y;
            --(*pos_y_ptr);
        }

        sdl_ui_putchar(c, pos_x++, pos_y);
        ++i;
    }

    return i;
}

static int *sdl_ui_menu_get_offsets(ui_menu_entry_t *menu, int num_items)
{
    int i, j, len, max_len;
    ui_menu_entry_type_t block_type;

    menu_offsets = lib_malloc(num_items * sizeof(int));

    for (i = 0; i < num_items; ++i) {
        block_type = menu[i].type;

        switch (block_type) {
            case MENU_ENTRY_SUBMENU:
            case MENU_ENTRY_DYNAMIC_SUBMENU:
            case MENU_ENTRY_TEXT:
                menu_offsets[i] = 1;
                break;
            default:
                max_len = 0;
                j = i;

                while ((j < num_items) && (menu[j].type == block_type)) {
                    len = strlen(menu[j].string);
                    menu_offsets[j] = len;
                    if (len > max_len) {
                        max_len = len;
                    }
                    ++j;
                }

                while (i < j) {
                    menu_offsets[i] = max_len + 2;
                    ++i;
                }
                --i;
                break;
        }
    }

    return menu_offsets;
}

int sdl_ui_set_cursor_colors(void)
{
    uint8_t color = menu_draw.color_back;
    menu_draw.color_back = menu_draw.color_cursor_back;
    if (menu_draw.color_cursor_revers) {
        sdl_ui_reverse_colors();
    }
    return color;
}

int sdl_ui_reset_cursor_colors(uint8_t color)
{
    if (menu_draw.color_cursor_revers) {
        sdl_ui_reverse_colors();
    }
    menu_draw.color_back = color;
    return color;
}

int sdl_ui_set_tickmark_colors(int state)
{
    uint8_t color = menu_draw.color_front;
    menu_draw.color_front = state ? menu_draw.color_active_green : menu_draw.color_inactive_red;
    return color;
}

int sdl_ui_reset_tickmark_colors(uint8_t color)
{
    menu_draw.color_front = color;
    return color;
}

int sdl_ui_set_toggle_colors(int state)
{
    uint8_t color = menu_draw.color_front;
    menu_draw.color_front = (state == 1) ? menu_draw.color_active_grey : menu_draw.color_inactive_grey;
    return color;
}

int sdl_ui_set_default_colors(void)
{
    uint8_t color = menu_draw.color_front;
    menu_draw.color_front = menu_draw.color_default_front;
    return color;
}

static int sdl_ui_display_item(ui_menu_entry_t *item, int y_pos, int value_offset, int iscursor)
{
    int n, i = 0, istoggle = 0, status = 0;
    char string[3] = {0x20, 0x20, 0};
    const char *itemdata;
    uint8_t oldbg = 0, oldfg = 1;

    if ((item->string == NULL) || (item->string[0] == 0)) {
        return -1;
    }

    sdl_ui_set_default_colors();

    if ((item->type == MENU_ENTRY_TEXT) && (vice_ptr_to_int(item->data) != 0)) {
        /* print a section header */
        i += sdl_ui_print(MENU_MENUHEADERLEFT_STRING, MENU_FIRST_X, y_pos + MENU_FIRST_Y);
        sdl_ui_reverse_colors();
        i += sdl_ui_print(" ", MENU_FIRST_X + i, y_pos + MENU_FIRST_Y);
        i += sdl_ui_print(item->string, MENU_FIRST_X + i, y_pos + MENU_FIRST_Y);
        i += sdl_ui_print(" ", MENU_FIRST_X+i, y_pos + MENU_FIRST_Y);
        sdl_ui_reverse_colors();
        i += sdl_ui_print(MENU_MENUHEADERRIGHT_STRING, MENU_FIRST_X+i, y_pos + MENU_FIRST_Y);
        return i;
    }

    if (iscursor) {
        oldbg = sdl_ui_set_cursor_colors();
    }
    istoggle = (item->type == MENU_ENTRY_RESOURCE_TOGGLE) ||
                (item->type == MENU_ENTRY_RESOURCE_RADIO) ||
                (item->type == MENU_ENTRY_OTHER_TOGGLE);

    itemdata = item->callback(0, item->data);

    if ((itemdata != NULL) && !strcmp(itemdata, MENU_NOT_AVAILABLE_STRING)) {
        /* menu is not available */
        status = 2;
    } else {
        /* print tick-mark for toggles and radio buttons at the start of the line */
        if (istoggle) {
            status = (itemdata == NULL) ? 0 : 1;
            string[0] = (itemdata == NULL) ? MENU_CHECKMARK_UNCHECKED_CHAR : itemdata[0];
        }
    }

    if (!iscursor) {
        oldfg = sdl_ui_set_tickmark_colors(status);
    }

    if ((n = sdl_ui_print(string, MENU_FIRST_X+i, y_pos + MENU_FIRST_Y)) < 0) {
        goto dispitemexit;
    }
    i += n;

    if (!iscursor) {
        sdl_ui_reset_tickmark_colors(oldfg);
    }

    if (!iscursor && istoggle) {
        sdl_ui_set_toggle_colors(status);
    }

    /* print the actual menu item */
    if ((n = sdl_ui_print(item->string, MENU_FIRST_X + i, y_pos + MENU_FIRST_Y)) < 0) {
        goto dispitemexit;
    }
    i += n;

    while (i < (value_offset)) {
        if ((n = sdl_ui_print(" ", MENU_FIRST_X + i, y_pos + MENU_FIRST_Y)) < 0) {
            goto dispitemexit;
        }
        i += n;
    }

    if (!iscursor && istoggle) {
        sdl_ui_set_default_colors();
    }

    /* print a space after the item */
    if ((itemdata != NULL) && (itemdata[0] != 0) /* && ((itemdata[0] & 0xff) != UIFONT_SUBMENULEFT_CHAR) */) {
        if ((n = sdl_ui_print(" ", MENU_FIRST_X + i, y_pos + MENU_FIRST_Y)) < 0) {
            goto dispitemexit;
        }
        i += n;
    }

    /* if the item was not an available "toggle", then print the item data */
    if (!istoggle || (status == 2)) {
        if ((n = sdl_ui_print(itemdata, MENU_FIRST_X + i, y_pos + MENU_FIRST_Y)) < 0) {
            goto dispitemexit;
        }
        i += n;
    }

dispitemexit:
    if (iscursor) {
        sdl_ui_print_eol(MENU_FIRST_X + i, y_pos + MENU_FIRST_Y);
        sdl_ui_reset_cursor_colors(oldbg);
    }
    sdl_ui_set_default_colors();

    return i;
}

static void sdl_ui_menu_redraw(ui_menu_entry_t *menu, const char *title, int offset, int *value_offsets, int cur_offset)
{
    int i = 0;

    sdl_ui_init_draw_params();
    sdl_ui_clear();
    sdl_ui_display_title(title);

    while ((menu[i + offset].string != NULL) && (i <= (menu_draw.max_text_y - MENU_FIRST_Y))) {
        sdl_ui_display_item(&(menu[i + offset]), i, value_offsets[i + offset], (i == cur_offset));
        ++i;
    }
}

static void sdl_ui_menu_redraw_cursor(ui_menu_entry_t *menu, int offset, int *value_offsets, int cur_offset, int old_offset)
{
    int i = 0, n;

    while ((menu[i + offset].string != NULL) && (i <= (menu_draw.max_text_y - MENU_FIRST_Y))) {
        if (i == cur_offset) {
            sdl_ui_display_item(&(menu[i + offset]), i, value_offsets[i + offset], 1);
        } else if (i == old_offset) {
            n = sdl_ui_display_item(&(menu[i + offset]), i, value_offsets[i + offset], 0);
            sdl_ui_print_eol(MENU_FIRST_X + n, MENU_FIRST_Y + i);
        }
        ++i;
    }
}

static ui_menu_retval_t sdl_ui_menu_display(ui_menu_entry_t *menu, const char *title, int allow_mapping)
{
    int num_items = 0, cur = 0, cur_old = -1, cur_offset = 0, in_menu = 1, redraw = 1;
    int *value_offsets = NULL;
    ui_menu_retval_t menu_retval = MENU_RETVAL_DEFAULT;
    int i;

    /* SDL mode: prevent core dump - pressing menu key in -console mode causes menu to be NULL */
    if (menu == NULL) {
        return MENU_RETVAL_EXIT_UI;
    }

    while (menu[num_items].string != NULL) {
        ++num_items;
    }

    if (num_items == 0) {
        return MENU_RETVAL_DEFAULT;
    }

    value_offsets = sdl_ui_menu_get_offsets(menu, num_items);

    /* If a subtitle is at the top of the menu, then start at the next line. */
    if (menu[0].type == MENU_ENTRY_TEXT) {
        cur = 1;
    }

    while (in_menu) {
        if (redraw) {
            sdl_ui_menu_redraw(menu, title, cur_offset, value_offsets, cur);
            cur_old = -1;
            redraw = 0;
        } else {
            sdl_ui_menu_redraw_cursor(menu, cur_offset, value_offsets, cur, cur_old);
        }
        sdl_ui_refresh();

        switch (sdl_ui_menu_poll_input()) {
            case MENU_ACTION_HOME:
                cur_old = cur;
                /* If a subtitle is at the top of the menu, then start at the next line. */
                if (menu[0].type == MENU_ENTRY_TEXT) {
                    cur = 1;
                } else {
                    cur = 0;
                }
                cur_offset = 0;
                redraw = 1;
                break;
            case MENU_ACTION_END:
                redraw = 1;
                cur_offset = num_items - (menu_draw.max_text_y - MENU_FIRST_Y);
                cur = (menu_draw.max_text_y - MENU_FIRST_Y) - 1;
                if (cur_offset < 0) {
                    cur += cur_offset;
                    cur_offset = 0;
                }
                break;
            case MENU_ACTION_UP:
                cur_old = cur;
                do {
                    if (cur > 0) {
                        --cur;
                    } else {
                        if (cur_offset > 0) {
                            --cur_offset;
                        } else {
                            cur_offset = num_items - (menu_draw.max_text_y - MENU_FIRST_Y);
                            cur = (menu_draw.max_text_y - MENU_FIRST_Y) - 1;
                            if (cur_offset < 0) {
                                cur += cur_offset;
                                cur_offset = 0;
                            }
                        }
                        redraw = 1;
                    }
                /* Skip subtitles and blank lines. */
                } while (menu[cur + cur_offset].type == MENU_ENTRY_TEXT);
                break;
            case MENU_ACTION_PAGEUP:
                cur_old = cur;
                for (i = 0; i < (menu_draw.max_text_y - MENU_FIRST_Y - 1); i++) {
                    do {
                        if (cur > 0) {
                            --cur;
                        } else {
                            if (cur_offset > 0) {
                                --cur_offset;
                            }
                        }
                    /* Skip subtitles and blank lines. */
                    } while (menu[cur + cur_offset].type == MENU_ENTRY_TEXT);
                }
                redraw = 1;
                break;
            case MENU_ACTION_DOWN:
                cur_old = cur;
                do {
                    if ((cur + cur_offset) < (num_items - 1)) {
                        if (++cur == (menu_draw.max_text_y - MENU_FIRST_Y)) {
                            --cur;
                            ++cur_offset;
                            redraw = 1;
                        }
                    } else {
                        cur = cur_offset = 0;
                        redraw = 1;
                    }

                /* Skip subtitles and blank lines. */
                } while (menu[cur + cur_offset].type == MENU_ENTRY_TEXT);
                break;
            case MENU_ACTION_PAGEDOWN:
                cur_old = cur;
                for (i = 0; i < (menu_draw.max_text_y - MENU_FIRST_Y - 1); i++) {
                    do {
                        if ((cur + cur_offset) < (num_items - 1)) {
                            if (++cur == (menu_draw.max_text_y - MENU_FIRST_Y)) {
                                --cur;
                                ++cur_offset;
                            }
                        }
                    /* Skip subtitles and blank lines. */
                    } while (menu[cur + cur_offset].type == MENU_ENTRY_TEXT);
                }
                redraw = 1;
                break;
            case MENU_ACTION_RIGHT:
                if ((menu[cur + cur_offset].type != MENU_ENTRY_SUBMENU) && (menu[cur + cur_offset].type != MENU_ENTRY_DYNAMIC_SUBMENU)) {
                    break;
                }
            /* fall through */
            case MENU_ACTION_SELECT:
                if (sdl_ui_menu_item_activate(&(menu[cur + cur_offset])) == MENU_RETVAL_EXIT_UI) {
                    in_menu = 0;
                    menu_retval = MENU_RETVAL_EXIT_UI;
                } else {
                    sdl_ui_menu_redraw(menu, title, cur_offset, value_offsets, cur);
                }
                break;
            case MENU_ACTION_EXIT:
                menu_retval = MENU_RETVAL_EXIT_UI;
            /* fall through */
            case MENU_ACTION_LEFT:
            case MENU_ACTION_CANCEL:
                in_menu = 0;
                break;
            case MENU_ACTION_MAP:
                if (allow_mapping && sdl_ui_hotkey_map(&(menu[cur + cur_offset]))) {
                    sdl_ui_menu_redraw(menu, title, cur_offset, value_offsets, cur);
                }
                break;
            default:
                SDL_Delay(10);
                break;
        }
    }

    lib_free(value_offsets);
    menu_offsets = NULL;
    return menu_retval;
}

static ui_menu_retval_t sdl_ui_menu_item_activate(ui_menu_entry_t *item)
{
    const char *p = NULL;

    switch (item->type) {
        case MENU_ENTRY_OTHER:
        case MENU_ENTRY_OTHER_TOGGLE:
        case MENU_ENTRY_DIALOG:
        case MENU_ENTRY_RESOURCE_TOGGLE:
        case MENU_ENTRY_RESOURCE_RADIO:
        case MENU_ENTRY_RESOURCE_INT:
        case MENU_ENTRY_RESOURCE_STRING:
            p = item->callback(1, item->data);
            if (p == sdl_menu_text_exit_ui) {
                return MENU_RETVAL_EXIT_UI;
            }
            break;
        case MENU_ENTRY_SUBMENU:
            return sdl_ui_menu_display((ui_menu_entry_t *)item->data, item->string, 1);
            break;
        case MENU_ENTRY_DYNAMIC_SUBMENU:
            return sdl_ui_menu_display((ui_menu_entry_t *)item->data, item->string, 0);
            break;
        default:
            break;
    }
    return MENU_RETVAL_DEFAULT;
}

static void sdl_ui_trap(uint16_t addr, void *data)
{
    unsigned int width;
    unsigned int height;
    static char msg[0x40];

    width = sdl_active_canvas->draw_buffer->draw_buffer_width;
    height = sdl_active_canvas->draw_buffer->draw_buffer_height;
    draw_buffer_backup = lib_malloc(width * height);

    memcpy(draw_buffer_backup, sdl_active_canvas->draw_buffer->draw_buffer, width * height);

    sdl_ui_activate_pre_action();
    if (machine_class != VICE_MACHINE_VSID) {
        memset(sdl_active_canvas->draw_buffer->draw_buffer, 0, width * height);
    }

    if (data == NULL) {
        sprintf(&msg[0], "VICE %s - main menu", machine_name);
        sdl_ui_menu_display(main_menu, msg, 1);
    } else {
        sdl_ui_init_draw_params();
        sdl_ui_menu_item_activate((ui_menu_entry_t *)data);
    }

    if (machine_class == VICE_MACHINE_VSID) {
        memset(sdl_active_canvas->draw_buffer_vsid->draw_buffer, 0, width * height);
        sdl_ui_refresh();
    } else {
        if (ui_emulation_is_paused() && (width == sdl_active_canvas->draw_buffer->draw_buffer_width) && (height == sdl_active_canvas->draw_buffer->draw_buffer_height)) {
            memcpy(sdl_active_canvas->draw_buffer->draw_buffer, draw_buffer_backup, width * height);
            sdl_ui_refresh();
        }
    }

    sdl_ui_activate_post_action();

    lib_free(draw_buffer_backup);
    draw_buffer_backup = NULL;
}

/* ------------------------------------------------------------------ */
/* Readline static functions/variables */

#define PC_VKBD_ACTIVATE VICE_SDLK_F10
#define PC_VKBD_W 17
#define PC_VKBD_H 4

static const char *keyb_pc[] = {
    "X`1234567890-= <-",
    "escQWERTYUIOP[]\\ ",
    "   ASDFGHJKL;' rt",
    "spc ZXCVBNM,./ <>",
    NULL
};

static const uint8_t keytable_pc[] =
    "\x87`1234567890-=\xff\x80\x80"
    "\x81\x81\x81qwertyuiop[]\\\xff"
    "\xff\xff\xff\x61sdfghjkl;'\xff\x82\x82"
    "   \xffzxcvbnm,./\xff\x83\x84";

static const uint8_t keytable_pc_shift[] =
    "\x87~!@#$%^&*()_+\x87\x80\x80"
    "\x81\x81\x81QWERTYUIOP{}|\x87"
    "\x87\x87\xff\x41SDFGHJKL:\"\x87\x82\x82"
    "   \x87ZXCVBNM<>?\x87\x85\x86";

static const SDLKey keytable_pc_special[] = {
    VICE_SDLK_BACKSPACE,
    VICE_SDLK_ESCAPE,
    VICE_SDLK_RETURN,
    VICE_SDLK_LEFT,
    VICE_SDLK_RIGHT,
    VICE_SDLK_HOME,
    VICE_SDLK_END,
    PC_VKBD_ACTIVATE,
    -1
};

static int pc_vkbd_pos_x, pc_vkbd_pos_y, pc_vkbd_x, pc_vkbd_y;

static void sdl_ui_readline_vkbd_draw(void)
{
    int i;

    for (i = 0; i < PC_VKBD_H; ++i) {
        sdl_ui_print(keyb_pc[i], pc_vkbd_pos_x, pc_vkbd_pos_y + i);
    }

    sdl_ui_invert_char(pc_vkbd_pos_x + pc_vkbd_x, pc_vkbd_pos_y + pc_vkbd_y);
}

static void sdl_ui_readline_vkbd_erase(void)
{
    int i;

    for (i = 0; i < PC_VKBD_H; ++i) {
        sdl_ui_print("                 ", pc_vkbd_pos_x, pc_vkbd_pos_y + i);
    }
}

static void sdl_ui_readline_vkbd_move(int *var, int amount, int min, int max)
{
    sdl_ui_invert_char(pc_vkbd_pos_x + pc_vkbd_x, pc_vkbd_pos_y + pc_vkbd_y);

    *var += amount;

    if (*var < min) {
        *var = max - 1;
    } else if (*var >= max) {
        *var = min;
    }

    sdl_ui_invert_char(pc_vkbd_pos_x + pc_vkbd_x, pc_vkbd_pos_y + pc_vkbd_y);
    sdl_ui_refresh();
}

static int sdl_ui_readline_vkbd_press(SDLKey *key, SDLMod *mod, Uint16 *c_uni, int shift)
{
    const uint8_t *table;
    uint8_t b;

    table = (shift == 0) ? keytable_pc : keytable_pc_shift;
    b = table[pc_vkbd_x + pc_vkbd_y * PC_VKBD_W];

    if (b == 0xff) {
        return 0;
    }

    if (b & 0x80) {
        *key = keytable_pc_special[b & 0x7f];
        *c_uni = 0;
    } else {
        *key = SDLK_UNKNOWN;
        *c_uni = (Uint16)b;
    }

    return 1;
}

static int sdl_ui_readline_vkbd_input(SDLKey *key, SDLMod *mod, Uint16 *c_uni)
{
    int done = 0;

    do {
        switch (sdl_ui_menu_poll_input()) {
            case MENU_ACTION_UP:
                sdl_ui_readline_vkbd_move(&pc_vkbd_y, -1, 0, PC_VKBD_H);
                break;
            case MENU_ACTION_DOWN:
                sdl_ui_readline_vkbd_move(&pc_vkbd_y, 1, 0, PC_VKBD_H);
                break;
            case MENU_ACTION_LEFT:
                sdl_ui_readline_vkbd_move(&pc_vkbd_x, -1, 0, PC_VKBD_W);
                break;
            case MENU_ACTION_RIGHT:
                sdl_ui_readline_vkbd_move(&pc_vkbd_x, 1, 0, PC_VKBD_W);
                break;
            case MENU_ACTION_SELECT:
                if (sdl_ui_readline_vkbd_press(key, mod, c_uni, 0)) {
                    done = 1;
                }
                break;
            case MENU_ACTION_CANCEL:
                if (sdl_ui_readline_vkbd_press(key, mod, c_uni, 1)) {
                    done = 1;
                }
                break;
            case MENU_ACTION_MAP:
            case MENU_ACTION_EXIT:
                *key = SDL2x_to_SDL1x_Keys(PC_VKBD_ACTIVATE);
                done = 1;
                break;
            default:
                break;
        }
    } while (!done);

    return 1;
}

static int sdl_ui_readline_input(SDLKey *key, SDLMod *mod, Uint16 *c_uni)
{
    SDL_Event e;
    int got_key = 0;
    ui_menu_action_t action = MENU_ACTION_NONE;
#ifdef USE_SDLUI2
    int i;
#endif

    *mod = KMOD_NONE;
    *c_uni = 0;

#ifdef USE_SDLUI2
    SDL_StartTextInput();
#endif

    do {
#ifdef ANDROID_COMPILE
        struct locnet_al_event event1;
        struct locnet_al_event *event = &event1;
        ui_menu_action_t action2;
#endif
        action = MENU_ACTION_NONE;

#ifdef ANDROID_COMPILE
        while (!Android_PollEvent(&event1)) {
            SDL_Delay(20);
        }
        switch (event->eventType) {
            case SDL_JOYAXISMOTION:
                {
                    action = sdljoy_axis_event(0, 0, event->x / 256.0f * 32767);
                    action2 = sdljoy_axis_event(0, 1, event->y / 256.0f * 32767);
                    if (action == MENU_ACTION_NONE) {
                        action = action2;
                    }
                }
                break;
            case SDL_JOYBUTTONDOWN:
                {
                    action = sdljoy_button_event(0, event->keycode, 1);
                }
                break;
            case SDL_JOYBUTTONUP:
                {
                    action = sdljoy_button_event(0, event->keycode, 0);
                }
                break;
            case SDL_KEYDOWN:
                {
                    unsigned long modifier = event->modifier;
                    int ctrl = ((modifier & KEYBOARD_CTRL_FLAG) != 0);
                    int alt = ((modifier & KEYBOARD_ALT_FLAG) != 0);
                    int shift = ((modifier & KEYBOARD_SHIFT_FLAG) != 0);
                    unsigned long kcode = (unsigned long)event->keycode;

                    int kmod = 0;

                    if (ctrl) {
                        kmod |= KMOD_LCTRL;
                    }
                    if (alt) {
                        kmod |= KMOD_LALT;
                    }
                    if (shift) {
                        kmod |= KMOD_LSHIFT;
                    }

                    *key = kcode;
                    *mod = kmod;
                    *c_uni = event->unicode;
                    got_key = 1;
                }
                break;
        }
#else
        SDL_WaitEvent(&e);

        switch (e.type) {
            case SDL_KEYDOWN:
                *key = SDL2x_to_SDL1x_Keys(e.key.keysym.sym);
                *mod = e.key.keysym.mod;
#ifdef USE_SDLUI2
                /* For SDL2x only get 'special' keys from keydown event. */
                for (i = 0; keytable_pc_special[i] != -1; ++i) {
                    SDLKey special = SDL2x_to_SDL1x_Keys(e.key.keysym.sym);
                    if (special == keytable_pc_special[i]) {
                        *c_uni = special;
                        got_key = 1;
                    }
                }
#else
                *c_uni = e.key.keysym.unicode;
                got_key = 1;
#endif
                break;

#ifdef USE_SDLUI2
            case SDL_TEXTINPUT:
                if (e.text.text[0] != 0) {
                    *key = e.text.text[0];
                    *c_uni = (Uint16)e.text.text[0];
                    SDL_StopTextInput();
                    SDL_StartTextInput();
                    got_key = 1;
                }
                break;
#endif

#ifdef HAVE_SDL_NUMJOYSTICKS
            case SDL_JOYAXISMOTION:
                action = sdljoy_axis_event(e.jaxis.which, e.jaxis.axis, e.jaxis.value);
                break;
            case SDL_JOYBUTTONDOWN:
                action = sdljoy_button_event(e.jbutton.which, e.jbutton.button, 1);
                break;
            case SDL_JOYHATMOTION:
                action = sdljoy_hat_event(e.jhat.which, e.jhat.hat, e.jhat.value);
                break;
#endif
            default:
                ui_handle_misc_sdl_event(e);
                break;
        }
#endif

        switch (action) {
            case MENU_ACTION_LEFT:
                *key = VICE_SDLK_LEFT;
                got_key = 1;
                break;
            case MENU_ACTION_RIGHT:
                *key = VICE_SDLK_RIGHT;
                got_key = 1;
                break;
            case MENU_ACTION_SELECT:
                *key = VICE_SDLK_RETURN;
                got_key = 1;
                break;
            case MENU_ACTION_CANCEL:
            case MENU_ACTION_MAP:
                *key = PC_VKBD_ACTIVATE;
                got_key = 1;
                break;
            case MENU_ACTION_UP:
            case MENU_ACTION_DOWN:
            default:
                break;
        }

    } while (!got_key);

#ifdef USE_SDLUI2
    SDL_StopTextInput();
#endif

    return got_key;
}

/*
 * - if resource is not NULL, update the respective resource in realtime for preview. however when leaving this
 *   function the original value will always get restored.
 */
static int sdl_ui_slider(const char* title, const int cur, const int min, const int max, int pos_x, int pos_y, const char *resource)
{
    int i = 0, done = 0, loop = 0, screen_dirty = 1, step = 1, xsize = menu_draw.max_text_x, oldvalue = 0;
    float segment = 0, segment2 = 0, parts = 0, parts2 = 0;
    char *new_string = NULL, *value = NULL;

    new_string = lib_malloc(xsize + 1);

    if (resource) {
        resources_get_int(resource, &oldvalue);
    }

    /* sanity check */
    i = cur;
    if (i < min) {
        i = min;
    }
    if (i > max) {
        i = max;
    }

    /* adjust step to about 1% */
    if ((max - min) > 100) {
        step = (max - min) / 100;
    }

    xsize -= pos_x;

    segment = ((float)(max - min) / (float)(xsize - 0.5f));
    segment2 = ((float)(max - min) / (float)(((xsize - 0.5f) * 2)));

    do {
        if (screen_dirty) {
            sprintf(new_string, "Step: %-10i", step);
            sdl_ui_print_wrap(new_string, pos_x, &pos_y);
            pos_y++;

            parts = (i - min) / segment;
            parts2 = (i - min) / segment2;
            for (loop = 0; loop < xsize; loop++) {
                new_string[loop] = (i - min) ? (loop < parts ? UIFONT_SLIDERACTIVE_CHAR : UIFONT_SLIDERINACTIVE_CHAR) : UIFONT_SLIDERINACTIVE_CHAR;
            }
            if ((i > min) && (i < max)) {
                new_string[(int)parts] = UIFONT_SLIDERACTIVE_CHAR + ((((int)parts2) & 1) ^ 1);
            }
            new_string[loop] = 0;
            sdl_ui_print_wrap(new_string, pos_x, &pos_y);
            pos_y++;

            sprintf(new_string, "%-10i %3i%%", i, (100 * i) / max);
            sdl_ui_print_wrap(new_string, pos_x, &pos_y);
            pos_y = pos_y - 2;

            sdl_ui_refresh();
            screen_dirty = 0;
            /* update resource value. this is needed for example for the realtime
               video updates in the colour- and CRT emulation settings */
            if (resource) {
                resources_set_int(resource, i);
            }
        }

        switch (sdl_ui_menu_poll_input()) {
            case MENU_ACTION_LEFT:
                if (i > min) {
                    i = i - step;
                    if (i < min) {
                        i = min;
                    }
                    screen_dirty = 1;
                }
                break;

            case MENU_ACTION_RIGHT:
                if (i < max) {
                    i = i + step;
                    if (i > max) {
                        i = max;
                    }
                    screen_dirty = 1;
                }
                break;

            case MENU_ACTION_UP:
                step /= 10;
                if (step < 1) {
                    step = 1;
                }
                screen_dirty = 1;
                break;

            case MENU_ACTION_DOWN:
                if (step * 10 < max) {
                    step *= 10;
                }
                screen_dirty = 1;
                break;

            case MENU_ACTION_CANCEL:
            case MENU_ACTION_EXIT:
                i = cur;
                done = 1;
                break;

            case MENU_ACTION_SELECT:
                done = 1;
                break;

            case MENU_ACTION_MAP:
                sprintf(new_string, "%i", i);
                value = sdl_ui_text_input_dialog(title, new_string);

                /* accept value from user, convert and free */
                if (value) {
                    i = strtol(value, NULL, 0);

                    if (i < min) {
                        i = min;
                    }
                    if (i > max) {
                        i = max;
                    }
                    lib_free(value);
                }
                screen_dirty = 1;
                break;

            default:
                break;
        }
    } while (!done);

    lib_free(new_string);
    new_string = NULL;

    if (resource) {
        resources_set_int(resource, oldvalue);
    }
    return i;
}


/* ------------------------------------------------------------------ */
/* External UI interface */

ui_menu_retval_t sdl_ui_external_menu_activate(ui_menu_entry_t *item)
{
    if (item && ((item->type == MENU_ENTRY_SUBMENU) || (item->type == MENU_ENTRY_DYNAMIC_SUBMENU))) {
        return sdl_ui_menu_display((ui_menu_entry_t *)item->data, item->string, 0);
    }

    return MENU_RETVAL_DEFAULT;
}

uint8_t *sdl_ui_get_draw_buffer(void)
{
    return draw_buffer_backup;
}

menu_draw_t *sdl_ui_get_menu_param(void)
{
    return &menu_draw;
}

menufont_t *sdl_ui_get_menu_font(void)
{
    return &activefont;
}

void sdl_ui_activate_pre_action(void)
{
#ifdef HAVE_FFMPEG
    if (screenshot_is_recording()) {
        screenshot_stop_recording();
    }
#endif

    vsync_suspend_speed_eval();
    sound_suspend();

    if (sdl_vkbd_state & SDL_VKBD_ACTIVE) {
        sdl_vkbd_close();
    }

    if (sdl_vsid_state & SDL_VSID_ACTIVE) {
        sdl_vsid_close();
    }

#ifndef USE_SDLUI2
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
#endif
    sdl_menu_state = 1;
    ui_check_mouse_cursor();
}

void sdl_ui_activate_post_action(void)
{
    int warp_state;

    sdl_menu_state = 0;
    ui_check_mouse_cursor();
#ifndef USE_SDLUI2
    SDL_EnableKeyRepeat(0, 0);
#endif

    /* Do not resume sound if in warp mode */
    resources_get_int("WarpMode", &warp_state);
    if (warp_state == 0) {
        sound_resume();
    }

    if (machine_class == VICE_MACHINE_VSID) {
        sdl_vsid_activate();
    }

    /* Force a video refresh */
    /* SDL mode: prevent core dump - pressing menu key in -console mode causes parent_raster to be NULL */
    if (sdl_active_canvas->parent_raster) {
        raster_force_repaint(sdl_active_canvas->parent_raster);
    }
}

void sdl_ui_init_draw_params(void)
{
    if (sdl_ui_set_menu_params != NULL) {
        sdl_ui_set_menu_params(sdl_active_canvas->index, &menu_draw);
    }

    menu_draw.pitch = sdl_active_canvas->draw_buffer->draw_buffer_pitch;
    menu_draw.offset = sdl_active_canvas->geometry->gfx_position.x + menu_draw.extra_x
                       + (sdl_active_canvas->geometry->gfx_position.y + menu_draw.extra_y) * menu_draw.pitch
                       + sdl_active_canvas->geometry->extra_offscreen_border_left;
}

void sdl_ui_reverse_colors(void)
{
    uint8_t color;

    color = menu_draw.color_front;
    menu_draw.color_front = menu_draw.color_back;
    menu_draw.color_back = color;
}

ui_menu_action_t sdl_ui_menu_poll_input(void)
{
    ui_menu_action_t retval = MENU_ACTION_NONE;

    do {
        SDL_Delay(20);
        retval = ui_dispatch_events();
#ifdef HAVE_SDL_NUMJOYSTICKS
        if (retval == MENU_ACTION_NONE || retval == MENU_ACTION_NONE_RELEASE) {
            retval = sdljoy_autorepeat();
        }
#endif
    } while (retval == MENU_ACTION_NONE || retval == MENU_ACTION_NONE_RELEASE);
    return retval;
}

int sdl_ui_print(const char *text, int pos_x, int pos_y)
{
    int i = 0;
    uint8_t c;

    if (text == NULL) {
        return 0;
    }

    if ((pos_x >= menu_draw.max_text_x) || (pos_y >= menu_draw.max_text_y)) {
        return -1;
    }

    while (((c = text[i]) != 0) && ((pos_x + i) < menu_draw.max_text_x)) {
        sdl_ui_putchar(c, pos_x + i, pos_y);
        ++i;
    }

    return i;
}

int sdl_ui_print_eol(int pos_x, int pos_y)
{
    int i = 0;
    if ((pos_x >= menu_draw.max_text_x) || (pos_y >= menu_draw.max_text_y)) {
        return -1;
    }
    while ((pos_x + i) < menu_draw.max_text_x) {
        sdl_ui_putchar(' ', pos_x + i, pos_y);
        ++i;
    }
    return i;
}

int sdl_ui_print_center(const char *text, int pos_y)
{
    int len, pos_x;
    int i = 0;
    uint8_t c;

    if (text == NULL) {
        return 0;
    }

    len = strlen(text);

    if (len == 0) {
        return 0;
    }

    pos_x = (menu_draw.max_text_x - len) / 2;
    if (pos_x < 0) {
        return -1;
    }

    if ((pos_x >= menu_draw.max_text_x) || (pos_y >= menu_draw.max_text_y)) {
        return -1;
    }

    while (((c = text[i]) != 0) && ((pos_x + i) < menu_draw.max_text_x)) {
        sdl_ui_putchar(c, pos_x + i, pos_y);
        ++i;
    }

    return i;
}

/* print a headline in the first row of the screen */
int sdl_ui_display_title(const char *title)
{
    int dummy = 0, i;
    sdl_ui_reverse_colors();
    i = sdl_ui_print_wrap(title, 0, &dummy);
    i += sdl_ui_print_eol(i, 0);
    sdl_ui_reverse_colors();
    return i;
}

void sdl_ui_invert_char(int pos_x, int pos_y)
{
    int x, y;
    uint8_t *draw_pos;

    while (pos_x >= menu_draw.max_text_x) {
        pos_x -= menu_draw.max_text_x;
        ++pos_y;
    }

    if (machine_class == VICE_MACHINE_VSID) {
        draw_pos = &(sdl_active_canvas->draw_buffer_vsid->draw_buffer[pos_x * activefont.w + pos_y * activefont.h * menu_draw.pitch]);
    } else {
        draw_pos = &(sdl_active_canvas->draw_buffer->draw_buffer[pos_x * activefont.w + pos_y * activefont.h * menu_draw.pitch]);
    }

    draw_pos += menu_draw.offset;

    for (y = 0; y < activefont.h; ++y) {
        for (x = 0; x < activefont.w; ++x) {
            if (draw_pos[x] == menu_draw.color_front) {
                draw_pos[x] = menu_draw.color_back;
            } else {
                draw_pos[x] = menu_draw.color_front;
            }
        }
        draw_pos += menu_draw.pitch;
    }
}

void sdl_ui_activate(void)
{
    if (ui_emulation_is_paused()) {
        ui_pause_emulation(0);
    }
    interrupt_maincpu_trigger_trap(sdl_ui_trap, NULL);
}

void sdl_ui_clear(void)
{
    int x, y;
    const char c = ' ';

    for (y = 0; y < menu_draw.max_text_y; ++y) {
        for (x = 0; x < menu_draw.max_text_x; ++x) {
            sdl_ui_putchar(c, x, y);
        }
    }
}

int sdl_ui_hotkey(ui_menu_entry_t *item)
{
    if (item == NULL) {
        return 0;
    }

    switch (item->type) {
        case MENU_ENTRY_OTHER:
        case MENU_ENTRY_OTHER_TOGGLE:
        case MENU_ENTRY_RESOURCE_TOGGLE:
        case MENU_ENTRY_RESOURCE_RADIO:
            return sdl_ui_menu_item_activate(item);
            break;
        case MENU_ENTRY_RESOURCE_INT:
        case MENU_ENTRY_RESOURCE_STRING:
        case MENU_ENTRY_DIALOG:
        case MENU_ENTRY_SUBMENU:
        case MENU_ENTRY_DYNAMIC_SUBMENU:
            interrupt_maincpu_trigger_trap(sdl_ui_trap, (void *)item);
        default:
            break;
    }
    return 0;
}


char* sdl_ui_readline(const char* previous, int pos_x, int pos_y)
{
    int i = 0, prev = -1, done = 0, got_key = 0, string_changed = 0, screen_dirty = 1, escaped = 0;
    int pc_vkbd_state = 0, screen_redraw = 0;
    size_t size = 0, max;
    char *new_string = NULL;
    SDLKey key = SDLK_UNKNOWN;
    SDLMod mod;
    Uint16 c_uni = 0;
    char c;

    /* restrict maximum length to screen size, leaving room for the prompt and the cursor*/
    max = menu_draw.max_text_y * menu_draw.max_text_x - pos_x - 1;

    pc_vkbd_state = archdep_require_vkbd();

    if (previous) {
        new_string = lib_stralloc(previous);
        size = strlen(new_string) + 1;
        if (size < max) {
            new_string = lib_realloc(new_string, max);
        } else {
            ui_error("Readline: previous %i >= max %i, returning NULL.", size, max);
            lib_free(new_string);
            return NULL;
        }
    } else {
        new_string = lib_malloc(max);
        new_string[0] = 0;
    }

    /* set vkbd location away from the prompt */
    if (pos_y < (menu_draw.max_text_y / 2)) {
        pc_vkbd_pos_y = menu_draw.max_text_y - PC_VKBD_H;
    } else {
        pc_vkbd_pos_y = 0;
    }
    pc_vkbd_pos_x = menu_draw.max_text_x - PC_VKBD_W;
    pc_vkbd_x = 0;
    pc_vkbd_y = 0;

    /* draw previous string (if any), initialize size and cursor position */
    size = i = sdl_ui_print_wrap(new_string, pos_x, &pos_y);

#ifndef USE_SDLUI2
    SDL_EnableUNICODE(1);
#endif

    do {
        if (i != prev) {
            if ((pos_y * menu_draw.max_text_x + pos_x + i) >= (menu_draw.max_text_y * menu_draw.max_text_x)) {
                sdl_ui_scroll_screen_up();
                --pos_y;

                if (pc_vkbd_state) {
                    screen_redraw = 1;
                }
            }

            sdl_ui_invert_char(pos_x + i, pos_y);

            if (prev >= 0) {
                sdl_ui_invert_char(pos_x + prev, pos_y);
            }
            prev = i;
            screen_dirty = 1;
        }

        if (screen_redraw) {
            sdl_ui_print_wrap(new_string, pos_x, &pos_y);
            screen_redraw = 0;
        }

        if (screen_dirty) {
            if (pc_vkbd_state) {
                sdl_ui_readline_vkbd_draw();
            }
            sdl_ui_refresh();
            screen_dirty = 0;
        }

        /* get input */
        if (pc_vkbd_state) {
            got_key = sdl_ui_readline_vkbd_input(&key, &mod, &c_uni);
        } else {
            got_key = sdl_ui_readline_input(&key, &mod, &c_uni);
        }

        switch (key) {
            case VICE_SDLK_LEFT:
                if (i > 0) {
                    --i;
                }
                break;
            case VICE_SDLK_RIGHT:
                if (i < (int)size) {
                    ++i;
                }
                break;
            case VICE_SDLK_HOME:
                i = 0;
                break;
            case VICE_SDLK_END:
                i = size;
                break;
            case PC_VKBD_ACTIVATE:
                pc_vkbd_state ^= 1;
                screen_dirty = 1;
                if (!pc_vkbd_state) {
                    sdl_ui_readline_vkbd_erase();
                    screen_redraw = 1;
                    if (archdep_require_vkbd()) {
                        string_changed = 0;
                        escaped = 1;
                        done = 1;
                    }
                }
                break;
            case VICE_SDLK_BACKSPACE:
                if (i > 0) {
                    memmove(new_string + i - 1, new_string + i, size - i + 1);
                    --size;
                    new_string[size] = ' ';
                    sdl_ui_print_wrap(new_string + i - 1, pos_x + i - 1, &pos_y);
                    new_string[size] = 0;
                    --i;
                    if (i != (int)size) {
                        prev = -1;
                    }
                    string_changed = 1;
                }
                break;
            case VICE_SDLK_ESCAPE:
                string_changed = 0;
                escaped = 1;
            /* fall through */
            case VICE_SDLK_RETURN:
                if (pc_vkbd_state) {
                    sdl_ui_readline_vkbd_erase();
                }
                sdl_ui_invert_char(pos_x + i, pos_y);
                done = 1;
                break;
            default:
                got_key = 0; /* got unicode value */
                break;
        }

        if (!got_key && (size < max) && ((c_uni & 0xff80) == 0) && ((c_uni & 0x7f) != 0)) {
            c = c_uni & 0x7f;
            memmove(new_string + i + 1, new_string + i, size - i);
            new_string[i] = c;
            ++size;
            new_string[size] = 0;
            sdl_ui_print_wrap(new_string + i, pos_x + i, &pos_y);
            ++i;
            prev = -1;
            string_changed = 1;
        }
    } while (!done);

#ifndef USE_SDLUI2
    SDL_EnableUNICODE(0);
#endif

    if ((!string_changed && previous) || escaped) {
        lib_free(new_string);
        new_string = NULL;
    }
    return new_string;
}

char* sdl_ui_text_input_dialog(const char* title, const char* previous)
{
    int i;

    sdl_ui_clear();
    i = sdl_ui_display_title(title) / menu_draw.max_text_x;
    return sdl_ui_readline(previous, 0, i + MENU_FIRST_Y);
}

int sdl_ui_slider_input_dialog(const char* title, const int cur, const int min, const int max)
{
    int i;

    sdl_ui_clear();
    i = sdl_ui_display_title(title) / menu_draw.max_text_x;
    return sdl_ui_slider(title, cur, min, max, 0, i + MENU_FIRST_Y, NULL);
}

ui_menu_entry_t *sdl_ui_get_main_menu(void)
{
    return main_menu;
}

void sdl_ui_refresh(void)
{
    video_canvas_refresh_all(sdl_active_canvas);
}

void sdl_ui_scroll_screen_up(void)
{
    int i, j;
    uint8_t *draw_pos;

    if (machine_class == VICE_MACHINE_VSID) {
        draw_pos = sdl_active_canvas->draw_buffer_vsid->draw_buffer + menu_draw.offset;
    } else {
        draw_pos = sdl_active_canvas->draw_buffer->draw_buffer + menu_draw.offset;
    }

    for (i = 0; i < menu_draw.max_text_y - 1; ++i) {
        for (j = 0; j < activefont.h; ++j) {
            memmove(draw_pos + (i * activefont.h + j) * menu_draw.pitch, draw_pos + (((i + 1) * activefont.h) + j) * menu_draw.pitch, menu_draw.max_text_x * activefont.w);
        }
    }

    for (j = 0; j < activefont.h; ++j) {
        memset(draw_pos + (i * activefont.h + j) * menu_draw.pitch, (char)menu_draw.color_back, menu_draw.max_text_x * activefont.w);
    }
}

/* handler for the sliders used in the colour- and CRT emulation settings 
   a custom handler is needed so we can print the colour matrix on the screen
   with the slider, and to call sdl_ui_slider to enable realtime updates of the
   respective resources.
 */
const char *sdl_ui_menu_video_slider_helper(int activated, ui_callback_data_t param, const char *resource_name, const int min, const int max)
{
    static char buf[20];
    int previous = min, new_value;
    int i, x, y;

    if (resources_get_int(resource_name, &previous)) {
        return sdl_menu_text_unknown;
    }
    sprintf(buf, "%i", previous);

    if (activated) {
        sdl_ui_clear();

        /* print 16x16 colors at the bottom, works for all emus */
        menu_draw.color_front =  0;
        for (y = 0; y < 16; y++) {
            for (x = 0; x < 16; x++) {
                menu_draw.color_back = y * 16 + x;
                sdl_ui_print("  ", 0 + x * 2, 9 + y);
            }
        }
        menu_draw.color_front = menu_draw.color_default_front;
        menu_draw.color_back = menu_draw.color_default_back;

        i = sdl_ui_display_title((const char *)param) / menu_draw.max_text_x;
        new_value = sdl_ui_slider((const char *)param, previous, min, max, 0, i + MENU_FIRST_Y, resource_name);

        if (new_value != previous) {
            resources_set_int(resource_name, new_value);
        }
    } else {
        return buf;
    }

    return NULL;
}

/* ------------------------------------------------------------------ */
/* Initialization/setting */

void sdl_ui_set_main_menu(const ui_menu_entry_t *menu)
{
    main_menu = (ui_menu_entry_t *)menu;
}

void sdl_ui_set_active_font(ui_menu_active_font_t f)
{
    int i;
    menufont_t *font;
    
    switch (f) {
        case MENU_FONT_IMAGES:
            font = &imagefont;
            break;
        case MENU_FONT_MONITOR:
            font = &monitorfont;
            break;
        default:
        case MENU_FONT_ASCII:
            font = &menufont;
            break;
    }
 
    activefont.font = font->font;
    activefont.w = font->w;
    activefont.h = font->h;

    for (i = 0; i < 256; ++i) {
        activefont.translate[i] = font->translate[i];
    }
}

void sdl_ui_set_menu_font(uint8_t *font, int w, int h)
{
    int i;

    menufont.font = font;
    menufont.w = w;
    menufont.h = h;

    for (i = 0; i < 256; ++i) {
        menufont.translate[i] = h * sdl_char_to_screen[i];
    }
    sdl_ui_set_active_font(MENU_FONT_ASCII);
}

void sdl_ui_set_image_font(uint8_t *font, int w, int h)
{
    int i;

    imagefont.font = font;
    imagefont.w = w;
    imagefont.h = h;

    for (i = 0; i < 256; ++i) {
        imagefont.translate[i] = h * sdl_char_to_screen_uppercase[i];
    }
    sdl_ui_set_active_font(MENU_FONT_IMAGES);
}

void sdl_ui_set_monitor_font(uint8_t *font, int w, int h)
{
    int i;

    monitorfont.font = font;
    monitorfont.w = w;
    monitorfont.h = h;

    for (i = 0; i < 256; ++i) {
        monitorfont.translate[i] = h * sdl_char_to_screen_monitor[i];
    }
    sdl_ui_set_active_font(MENU_FONT_MONITOR);
}

/** \brief  Free resources
 */
void sdl_ui_menu_shutdown(void)
{
    if (draw_buffer_backup != NULL) {
        lib_free(draw_buffer_backup);
    }
    if (menu_offsets != NULL) {
        lib_free(menu_offsets);
    }
}
