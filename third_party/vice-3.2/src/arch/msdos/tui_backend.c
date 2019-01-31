/*
 * tui_backend.c - MS-DOS backend for the text-based user interface.
 *
 * Written by
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

#include <conio.h>
#include <ctype.h>
#include <go32.h>
#include <keys.h>
#include <pc.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/farptr.h>
#include <unistd.h>

#include <allegro.h>

#include "lib.h"
#include "tui.h"
#include "tui_backend.h"
#include "util.h"
#include "version.h"

typedef uint8_t attr_t;
attr_t current_attr;

struct tui_area {
    int width, height;
    uint8_t *mem;
};

struct text_info text_mode_info;

/* ------------------------------------------------------------------------- */

static attr_t make_attr(int foreground_color, int background_color, int blink)
{
    return foreground_color | (background_color << 4) | (blink ? 0x80 : 0);
}

static unsigned long screen_addr(int x, int y)
{
    return 0xb8000 + 2 * (x + y * text_mode_info.screenwidth);
}

/* ------------------------------------------------------------------------- */

void tui_init(void)
{
    set_gfx_mode(GFX_TEXT, 80, 25, 0, 0);

    /* FIXME: this should be made more flexible, to handle other screen modes
       automatically. */
    _set_screen_lines(25);

    gettextinfo(&text_mode_info);
    /* _setcursortype(_NOCURSOR); */
}

int tui_num_lines(void)
{
    if (text_mode_info.screenheight == 0) {
        tui_init();
    }
    return text_mode_info.screenheight;
}

int tui_num_cols(void)
{
    if (text_mode_info.screenwidth == 0) {
        tui_init();
    }
    return text_mode_info.screenwidth;
}

void tui_set_attr(int foreground_color, int background_color, int blink)
{
    current_attr = make_attr(foreground_color, background_color, blink);
}

uint8_t tui_ascii_conversion_table[] = {
    128, 129, 130, 131, 132, 133, 134, 135,
    136, 137, 'S', 139, 'E', 141, 'Z', 143,
    144, 145, 146, 147, 148, 149, 150, 151,
    152, 153, 's', 155, 156, 157, 'z', 'Y',
    160, 161, 162, 163, 'Y', 165, 'S', 167,
    168, 169, 170, 171, 172, 173, 174, 175,
    176, 177, 178, 179, 180, 'u', 182, 183,
    184, 185, 186, 187, 188, 189, 190, 191,
    'A', 'A', 'A', 'A', 'A', 'A', 198, 'C',
    200, 201, 'E', 'E', 'I', 'I', 'I', 'I',
    'D', 'N', 'O', 'O', 'O', 'O', 'O', 215,
    'O', 'U', 'U', 'U', 'U', 'Y', 222, 223,
    'a', 'a', 'a', 'a', 'a', 'a', 230, 'c',
    'e', 'e', 'e', 'e', 'i', 'i', 'i', 'i',
    240, 'n', 'o', 'o', 'o', 'o', 'o', 247,
    248, 'u', 'u', 'u', 'u', 'y', 254, 'y',
};

void tui_put_char(int x, int y, uint8_t c)
{
    unsigned long addr = screen_addr(x, y);
    uint8_t attr_byte = (uint8_t)current_attr;

    _farsetsel(_dos_ds);
    if (c > 127) {
        _farnspokeb(addr, tui_ascii_conversion_table[c - 128]);
    } else {
        _farnspokeb(addr, c);
    }
    _farnspokeb(addr + 1, attr_byte);
}

void tui_hline(int x, int y, uint8_t c, int count)
{
    unsigned long addr = screen_addr(x, y);
    uint8_t attr_byte = (uint8_t)current_attr;
    int i;

    _farsetsel(_dos_ds);
    for (i = 0; i < count; i++) {
        _farnspokeb(addr, c);
        _farnspokeb(addr + 1, attr_byte);
        addr += 2;
    }
}

void tui_vline(int x, int y, uint8_t c, int count)
{
    unsigned long addr = screen_addr(x, y);
    uint8_t attr_byte = (uint8_t)current_attr;
    int i;

    _farsetsel(_dos_ds);
    for (i = 0; i < count; i++) {
        _farnspokeb(addr, c);
        _farnspokeb(addr + 1, attr_byte);
        addr += tui_num_cols();
    }
}

void tui_gotoxy(int x, int y)
{
    gotoxy(x + 1, y + 1);
}

void tui_flush_keys(void)
{
    while (kbhit()) {
        getkey();
    }
}

void tui_display(int x, int y, int len, const char *format, ...)
{
    uint8_t attr_byte = (uint8_t)current_attr;
    unsigned long addr = screen_addr(x, y);
    char *buf;
    int i, buf_len;
    va_list vl;

    va_start(vl, format);
    buf = lib_mvsprintf(format, vl);
    va_end(vl);
    buf_len = strlen(buf);
    if (len == 0) {
        len = buf_len;
    } else if (buf_len > len) {
        buf_len = len;
    }

    _farsetsel(_dos_ds);
    for (i = 0; i < buf_len; i++) {
        _farnspokeb(addr, buf[i]);
        _farnspokeb(addr + 1, attr_byte);
        addr += 2;
    }
    for (; i < len; i++) {
        _farnspokeb(addr, ' ');
        _farnspokeb(addr + 1, attr_byte);
        addr += 2;
    }
    lib_free(buf);
}

void tui_beep(void)
{
    sound(2000);
    nosound();
}

void tui_area_get(tui_area_t *a, int x, int y, int width, int height)
{
    uint8_t *p;
    int i, j;

    if (*a == NULL) {
        *a = lib_malloc(sizeof (struct tui_area));
        (*a)->mem = lib_malloc(2 * width * height);
    } else {
        (*a)->mem = lib_realloc((*a)->mem, 2 * width * height);
    }
    (*a)->width = width;
    (*a)->height = height;

    _farsetsel(_dos_ds);

    for (p = (*a)->mem, i = 0; i < height; i++) {
        int addr = screen_addr(x, y + i);

        for (j = 0; j < 2 * width; j++) {
            *(p++) = _farnspeekb(addr + j);
        }
    }
}

void tui_area_put(tui_area_t a, int x, int y)
{
    uint8_t *p = a->mem;
    int i, j;

    _farsetsel(_dos_ds);

    for (i = 0; i < a->height; i++) {
        int addr = screen_addr(x, y + i);

        for (j = 0; j < 2 * a->width; j++) {
            _farnspokeb(addr + j, *(p++));
        }
    }
}

void tui_area_free(tui_area_t a)
{
    if (a != NULL) {
        lib_free(a->mem);
        lib_free(a);
    }
}

void tui_clear_screen(void)
{
    int i;

    _setcursortype(_NOCURSOR);

    tui_set_attr(FIRST_LINE_FORE, FIRST_LINE_BACK, 0);

#ifndef UNSTABLE
    tui_display(0, 0, tui_num_cols(), "VICE version %s", VERSION);
#else
    tui_display(0, 0, tui_num_cols(), "VICE version %s (unstable)", VERSION);
#endif

    tui_set_attr(BACKPATTERN_FORE, BACKPATTERN_BACK, 0);
    for (i = 1; i < tui_num_lines() - 1; i++) {
        tui_hline(0, i, BACKCHAR, tui_num_cols());
    }
}

void tui_make_shadow(int x, int y, int width, int height)
{
    int i, j;

    _farsetsel(_dos_ds);
    for (i = 0; i < height; i++) {
        int addr = screen_addr(x, y + i) + 1;

        for (j = 0; j < width; j++, addr += 2) {
            _farnspokeb(addr, make_attr(DARKGRAY, BLACK, 0));
        }
    }
}

void tui_display_window(int x, int y, int width, int height, int foreground_color, int background_color, const char *title, tui_area_t *backing_store)
{
    int i;

    if (backing_store != NULL) {
        /* 2 more chars on right, 1 more on bottom because of the "shadow".  */
        tui_area_get(backing_store, x, y, width + 2, height + 1);
    }

    tui_make_shadow(x + 2, y + 1, width, height);

    tui_set_attr(foreground_color, background_color, 0);
    tui_put_char(x, y, 0xc9);
    tui_hline(x + 1, y, 0xcd, width - 2);
    tui_put_char(x + width - 1, y, 0xbb);
    tui_put_char(x, y + height - 1, 0xc8);
    tui_hline(x + 1, y + height - 1, 0xcd, width - 2);
    tui_put_char(x + width - 1, y + height - 1, 0xbc);
    for (i = 0; i < height - 2; i++) {
        tui_put_char(x, y + i + 1, 0xba);
        tui_hline(x + 1, y + i + 1, ' ', width - 2);
        tui_put_char(x + width - 1, y + i + 1, 0xba);
    }

    if (!util_check_null_string(title)) {
        int title_x, title_length;

        title_length = strlen(title);
        title_x = x + (width - title_length - 4) / 2;
        tui_display(title_x, y, 0, "\x10 %s \x11", title);
    }
}
