/*
 * tui.c - A (very) simple text-based user interface.
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
#include <pc.h>
#include <keys.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/farptr.h>

#include <allegro.h>

#include "lib.h"
#include "tui.h"
#include "tui_backend.h"
#include "tuifs.h"


static int _tui_input_string(int x, int y, char *buf, int max_length, int displayed_length, int foreground_color, int background_color)
{
    int cursor_pos = 0;
    int scroll_offset = 0;
    int string_length = strlen(buf);
    int need_redisplay = 1;

    tui_set_attr(foreground_color, background_color, 0);
    _setcursortype(_SOLIDCURSOR);
    while (1) {
        int key;

        if (need_redisplay) {
            tui_display(x, y, displayed_length, "%s", buf + scroll_offset);
            need_redisplay = 0;
        }
        tui_gotoxy(x + cursor_pos - scroll_offset, y);
        key = getkey();
        switch (key) {
            case K_Left:
                if (cursor_pos > 0) {
                    cursor_pos--;
                    if (cursor_pos < scroll_offset) {
                        scroll_offset = cursor_pos;
                        need_redisplay = 1;
                    }
                }
                break;
            case K_Right:
                if (cursor_pos < string_length) {
                    cursor_pos++;
                    if (cursor_pos >= (scroll_offset + displayed_length)) {
                        scroll_offset = cursor_pos - displayed_length + 1;
                        need_redisplay = 1;
                    }
                }
                break;
            case K_Home:
                cursor_pos = 0;
                scroll_offset = 0;
                need_redisplay = 1;
                break;
            case K_End:
                cursor_pos = string_length;
                scroll_offset = cursor_pos - displayed_length + 1;
                if (scroll_offset < 0) {
                    scroll_offset = 0;
                }
                need_redisplay = 1;
                break;
            case K_BackSpace:
                if (cursor_pos > 0 && cursor_pos <= string_length) {
                    memmove(buf + cursor_pos - 1, buf + cursor_pos, string_length - cursor_pos + 1);
                    string_length--;
                    cursor_pos--;
                    if (cursor_pos < scroll_offset) {
                        scroll_offset = cursor_pos;
                    }
                    need_redisplay = 1;
                }
                break;
            case K_Delete:
                if (cursor_pos < string_length) {
                    memmove(buf + cursor_pos, buf + cursor_pos + 1, string_length - cursor_pos);
                    string_length--;
                    need_redisplay = 1;
                }
                break;
            case K_Return:
                _setcursortype(_NOCURSOR);
                return 0;
            case K_Escape:
                _setcursortype(_NOCURSOR);
                return -1;
            default:
                if (key <= 0xff && isprint((char)key) && string_length < max_length) {
                    memmove(buf + cursor_pos + 1, buf + cursor_pos, string_length - cursor_pos + 1);
                    buf[cursor_pos] = (char)key;
                    cursor_pos++;
                    string_length++;
                    if (cursor_pos - scroll_offset >= displayed_length) {
                        scroll_offset = cursor_pos - displayed_length + 1;
                    }
                    need_redisplay = 1;
                }
                break;
        }
    }
}

int tui_input_string(const char *title, const char *prompt, char *buf, int buflen)
{
    int field_width, x, y, width, height, retval;
    tui_area_t backing_store = NULL;

    if (buflen + 1 < tui_num_cols() - 12) {
        field_width = buflen + 1;
    } else {
        field_width = tui_num_cols() - 12;
    }

    width = field_width + 4;
    height = 4;
    x = CENTER_X(width);
    y = CENTER_Y(height);

    tui_display_window(x, y, width, height, REQUESTER_BORDER, REQUESTER_BACK, title, &backing_store);
    tui_set_attr(REQUESTER_FORE, REQUESTER_BACK, 0);
    tui_display(x + 2, y + 1, field_width, prompt);

    retval = _tui_input_string(x + 2, y + 2, buf, buflen, field_width, FIELD_FORE, FIELD_BACK);

    tui_area_put(backing_store, x, y);
    tui_area_free(backing_store);

    return retval;
}

/* ------------------------------------------------------------------------- */

void tui_error(const char *format,...)
{
    int x, y, width, height;
    char *str;
    int str_length;
    va_list ap;
    tui_area_t backing_store = NULL;

    va_start(ap, format);
    str = lib_mvsprintf(format, ap);
    va_end(ap);
    str_length = strlen(str);
    if (str_length > tui_num_cols() - 10) {
        str_length = tui_num_cols() - 10;
        str[str_length] = 0;
    }
    x = CENTER_X(str_length + 6);
    y = CENTER_Y(5);
    width = str_length + 6;
    height = 5;

    tui_display_window(x, y, width, height, ERROR_BORDER, ERROR_BACK, "Error!", &backing_store);
    tui_set_attr(ERROR_FORE, ERROR_BACK, 0);
    tui_display(CENTER_X(str_length), y + 2, 0, str);
    getkey();

    tui_area_put(backing_store, x, y);
    tui_area_free(backing_store);
    lib_free(str);
}

void tui_message(const char *format,...)
{
    int x, y, width, height;
    char *str;
    int str_length;
    va_list ap;
    tui_area_t backing_store = NULL;

    va_start(ap, format);
    str = lib_mvsprintf(format, ap);
    va_end(ap);
    str_length = strlen(str);
    if (str_length > tui_num_cols() - 10) {
        str_length = tui_num_cols() - 10;
        str[str_length] = 0;
    }
    x = CENTER_X(str_length + 6);
    y = CENTER_Y(5);
    width = str_length + 6;
    height = 5;

    tui_display_window(x, y, width, height, MESSAGE_BORDER, MESSAGE_BACK, NULL, &backing_store);
    tui_set_attr(MESSAGE_FORE, MESSAGE_BACK, 0);
    tui_display(CENTER_X(str_length), y + 2, 0, str);
    getkey();

    tui_area_put(backing_store, x, y);
    tui_area_free(backing_store);
    lib_free(str);
}

int tui_ask_confirmation(const char *format, ...)
{
    int x, y, width, height;
    char *str;
    int str_length;
    va_list ap;
    tui_area_t backing_store = NULL;
    int c;

    va_start(ap, format);
    str = lib_mvsprintf(format, ap);
    va_end(ap);
    str_length = strlen(str);
    if (str_length > tui_num_cols() - 10) {
        str_length = tui_num_cols() - 10;
        str[str_length] = 0;
    }
    x = CENTER_X(str_length + 6);
    y = CENTER_Y(5);
    width = str_length + 6;
    height = 5;

    tui_display_window(x, y, width, height, MESSAGE_BORDER, MESSAGE_BACK, NULL, &backing_store);
    tui_set_attr(MESSAGE_FORE, MESSAGE_BACK, 0);
    tui_display(CENTER_X(str_length), y + 2, 0, str);

    do {
        c = getkey();
    } while (toupper(c) != 'Y' && toupper(c) != 'N');

    tui_area_put(backing_store, x, y);
    tui_area_free(backing_store);

    lib_free(str);

    return toupper(c) == 'Y';
}

/* ------------------------------------------------------------------------- */

void tui_display_text(int x, int y, int width, int height, const char *text)
{
    const char *p = text;
    int i, j;

    for (j = 0; j < height; j++) {
        for (i = 0; i < width; i++) {
            if (*p != '\n' && *p != '\0') {
                tui_put_char(x + i, y + j, *p);
                p++;
            } else {
                tui_put_char(x + i, y + j, ' ');
            }
        }
        if (*p == '\n') {
            p++;
        }
    }
}
