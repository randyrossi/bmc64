/*
 * uimsgbox.c - Common SDL message box functions.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *  Greg King <greg.king5@verizon.net>
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

#include "vice_sdl.h"
#include "types.h"

#include <assert.h>
#include <string.h>

#include "lib.h"
#include "resources.h"
#include "sound.h"
#include "ui.h"
#include "uifonts.h"
#include "uimenu.h"
#include "uimsgbox.h"
#include "vsync.h"

#define MAX_MSGBOX_LEN 28


/** \brief  Table of number of buttons for each `message mode` enum value
 */
static int msg_mode_buttons[] = {
    1,  /* MESSAGE_OK */
    2,  /* MESSAGE_YESNO */
    3,  /* MESSAGE_CPUJAM */
    5,  /* MESSAGE_UNIT_SELECT */
};



static menu_draw_t *menu_draw;

/** \brief  Show a message box with some buttons
 *
 * \param[in]   title           Message box title
 * \param[in]   message         Message box message/prompt
 * \param[in]   message_mode    Type of message box to display (defined in .h)
 *
 * \return  index of selected button, or -1 on cancel (Esc)
 */
static int handle_message_box(const char *title, const char *message, int message_mode)
{
    char *text, *pos;
    unsigned int msglen, len;
    int before;
    int x;
    char template[] = UIFONT_VERTICAL_STRING "                            " UIFONT_VERTICAL_STRING;
    unsigned int j = 5;
    int active = 1;
    int cur_pos = 0;
    int button_count;

    /* determine button count for requested `message mode` */
    if (message_mode < 0 || message_mode >= (int)(sizeof msg_mode_buttons / sizeof msg_mode_buttons[0])) {
#ifdef __func__
        fprintf(stderr, "%s:%d:%s: illegal `message_mode` value %d\n",
               __FILE__, __LINE__, __func__, message_mode);
#endif
        return -1;
    }
    button_count = msg_mode_buttons[message_mode];

    /* print the top edge of the dialog. */
    sdl_ui_clear();
    sdl_ui_print_center(UIFONT_TOPLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_TOPRIGHT_STRING, 2);

    /* make sure that the title length is not more than 28 chars. */
    len = strlen(title);
    assert(len <= MAX_MSGBOX_LEN);

    /* calculate the position in the template to copy the title to. */
    before = (MAX_MSGBOX_LEN - len) / 2;

    /* copy the title into the template. */
    memcpy(template + 1 + before, title, len);

    /* print the title part of the dialog. */
    sdl_ui_print_center(template, 3);

    /* print the title/text separator part of the dialog. */
    sdl_ui_print_center(UIFONT_RIGHTTEE_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_LEFTTEE_STRING, 4);

    text = lib_stralloc(message);
    msglen = (unsigned int)strlen(text);

    /* substitute forward slashes and spaces for backslashes and newline
     * characters.
     */
    pos = text + msglen;
    do {
        switch (*--pos) {
            case '\\':
                *pos = '/';
                break;
            case '\n':
                *pos = ' ';
            default:
                break;
        }
    } while (pos != text);

    while (msglen != 0) {
        len = msglen;
        if (len > MAX_MSGBOX_LEN) {
            /* fold lines at the space that is closest to the right edge. */
            len = MAX_MSGBOX_LEN + 1;
            while (pos[--len] != ' ') {
                /* if a word is too long, fold it anyway! */
                if (len == 0) {
                    len = MAX_MSGBOX_LEN;
                    break;
                }
            }
        }

        /* erase the old line. */
        memset(template + 1, ' ', MAX_MSGBOX_LEN);

        /* calculate the position in the template to copy the message line to. */
        before = (MAX_MSGBOX_LEN - len) / 2;

        /* copy the message line into the template. */
        memcpy(template + 1 + before, pos, len);

        /* print the message line. */
        sdl_ui_print_center(template, j);

        /* advance to the next message line. */
        j++;
        msglen -= len;
        pos += len;

        /* if the text was folded at a space, then move beyond that space. */
        if (*pos == ' ') {
            msglen--;
            pos++;
        }
    }
    lib_free(text);

    /* print any needed buttons. */
    sdl_ui_print_center(UIFONT_VERTICAL_STRING "                            " UIFONT_VERTICAL_STRING, j);
    switch (message_mode) {
        case MESSAGE_OK:
            sdl_ui_print_center(UIFONT_VERTICAL_STRING "            " UIFONT_TOPLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_TOPRIGHT_STRING "            " UIFONT_VERTICAL_STRING, j + 1);
            x = sdl_ui_print_center(UIFONT_VERTICAL_STRING "            " UIFONT_VERTICAL_STRING "OK" UIFONT_VERTICAL_STRING "            " UIFONT_VERTICAL_STRING, j + 2);
            sdl_ui_print_center(UIFONT_VERTICAL_STRING "            " UIFONT_BOTTOMLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_BOTTOMRIGHT_STRING "            " UIFONT_VERTICAL_STRING, j + 3);
            break;
        case MESSAGE_YESNO:
            sdl_ui_print_center(UIFONT_VERTICAL_STRING "      " UIFONT_TOPLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_TOPRIGHT_STRING "       " UIFONT_TOPLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_TOPRIGHT_STRING "      " UIFONT_VERTICAL_STRING, j + 1);
            x = sdl_ui_print_center(UIFONT_VERTICAL_STRING "      " UIFONT_VERTICAL_STRING "YES" UIFONT_VERTICAL_STRING "       " UIFONT_VERTICAL_STRING "NO" UIFONT_VERTICAL_STRING "      " UIFONT_VERTICAL_STRING, j + 2);
            sdl_ui_print_center(UIFONT_VERTICAL_STRING "      " UIFONT_BOTTOMLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_BOTTOMRIGHT_STRING "       " UIFONT_BOTTOMLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_BOTTOMRIGHT_STRING "      " UIFONT_VERTICAL_STRING, j + 3);
            break;
        case MESSAGE_UNIT_SELECT:
            /* Present a selection of '8', '9', '10', '11' or 'SKIP' */
            sdl_ui_print_center(UIFONT_VERTICAL_STRING "  " UIFONT_TOPLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_TOPRIGHT_STRING " " UIFONT_TOPLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_TOPRIGHT_STRING " " UIFONT_TOPLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_TOPRIGHT_STRING " " UIFONT_TOPLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_TOPRIGHT_STRING " " UIFONT_TOPLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_TOPRIGHT_STRING "  " UIFONT_VERTICAL_STRING, j + 1);
            x = sdl_ui_print_center(UIFONT_VERTICAL_STRING "  " UIFONT_VERTICAL_STRING "8" UIFONT_VERTICAL_STRING " " UIFONT_VERTICAL_STRING "9" UIFONT_VERTICAL_STRING " " UIFONT_VERTICAL_STRING "10" UIFONT_VERTICAL_STRING " " UIFONT_VERTICAL_STRING "11" UIFONT_VERTICAL_STRING " " UIFONT_VERTICAL_STRING "SKIP" UIFONT_VERTICAL_STRING "  " UIFONT_VERTICAL_STRING, j + 2);
            sdl_ui_print_center(UIFONT_VERTICAL_STRING "  " UIFONT_BOTTOMLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_BOTTOMRIGHT_STRING " " UIFONT_BOTTOMLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_BOTTOMRIGHT_STRING " " UIFONT_BOTTOMLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_BOTTOMRIGHT_STRING " " UIFONT_BOTTOMLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_BOTTOMRIGHT_STRING " " UIFONT_BOTTOMLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_BOTTOMRIGHT_STRING "  " UIFONT_VERTICAL_STRING, j + 3);
            break;
        case MESSAGE_CPUJAM:
        default:
            sdl_ui_print_center(UIFONT_VERTICAL_STRING " " UIFONT_TOPLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_TOPRIGHT_STRING "  " UIFONT_TOPLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_TOPRIGHT_STRING "  " UIFONT_TOPLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_TOPRIGHT_STRING " " UIFONT_VERTICAL_STRING, j + 1);
            x = sdl_ui_print_center(UIFONT_VERTICAL_STRING " " UIFONT_VERTICAL_STRING "RESET" UIFONT_VERTICAL_STRING "  " UIFONT_VERTICAL_STRING "MONITOR" UIFONT_VERTICAL_STRING "  " UIFONT_VERTICAL_STRING "CONT" UIFONT_VERTICAL_STRING " " UIFONT_VERTICAL_STRING, j + 2);
            sdl_ui_print_center(UIFONT_VERTICAL_STRING " " UIFONT_BOTTOMLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_BOTTOMRIGHT_STRING "  " UIFONT_BOTTOMLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_BOTTOMRIGHT_STRING "  " UIFONT_BOTTOMLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_BOTTOMRIGHT_STRING " " UIFONT_VERTICAL_STRING, j + 3);
            break;
    }

    /* print the bottom part of the dialog. */
    sdl_ui_print_center(UIFONT_BOTTOMLEFT_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING 
                        UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_HORIZONTAL_STRING UIFONT_BOTTOMRIGHT_STRING, j + 4);

    x += (menu_draw->max_text_x - 30) / 2;
    while (active) {
        switch (message_mode) {
            case MESSAGE_OK:
                sdl_ui_reverse_colors();
                sdl_ui_print_center("OK", j + 2);
                sdl_ui_reverse_colors();
                break;
            case MESSAGE_YESNO:
                if (cur_pos == 0) {
                    sdl_ui_reverse_colors();
                }
                sdl_ui_print("YES", x - 22, j + 2);
                sdl_ui_reverse_colors();
                sdl_ui_print("NO", x - 10, j + 2);
                if (cur_pos == 1) {
                    sdl_ui_reverse_colors();
                }
                break;

            case MESSAGE_UNIT_SELECT:
                sdl_ui_print_center(UIFONT_VERTICAL_STRING "  " UIFONT_VERTICAL_STRING "8" UIFONT_VERTICAL_STRING " " UIFONT_VERTICAL_STRING "9" UIFONT_VERTICAL_STRING " " UIFONT_VERTICAL_STRING "10" UIFONT_VERTICAL_STRING " " UIFONT_VERTICAL_STRING "11" UIFONT_VERTICAL_STRING " " UIFONT_VERTICAL_STRING "SKIP" UIFONT_VERTICAL_STRING "  " UIFONT_VERTICAL_STRING, j + 2);
 
                sdl_ui_reverse_colors();
                switch (cur_pos) {
                    case 0:
                        sdl_ui_print("8", x - 26, j + 2);
                        break;
                    case 1:
                        sdl_ui_print("9", x - 22, j + 2);
                        break;
                    case 2:
                        sdl_ui_print("10", x - 18, j + 2);
                        break;
                    case 3:
                        sdl_ui_print("11", x - 13, j + 2);
                        break;
                    case 4:
                        sdl_ui_print("SKIP", x - 8, j + 2);
                        break;
                    default:
                        break;
                }
                sdl_ui_reverse_colors();
                break;
            case MESSAGE_CPUJAM:
            default:
                if (cur_pos == 0) {
                    sdl_ui_reverse_colors();
                }
                sdl_ui_print("RESET", x - 27, j + 2);
                if (cur_pos < 2) {
                    sdl_ui_reverse_colors();
                }
                sdl_ui_print("MONITOR", x - 18, j + 2);
                if (cur_pos > 0) {
                    sdl_ui_reverse_colors();
                }
                sdl_ui_print("CONT", x - 7, j + 2);
                if (cur_pos == 2) {
                    sdl_ui_reverse_colors();
                }
                break;
        }

        sdl_ui_refresh();

        switch (sdl_ui_menu_poll_input()) {
            case MENU_ACTION_CANCEL:
            case MENU_ACTION_EXIT:
                cur_pos = -1;
                active = 0;
                break;
            case MENU_ACTION_SELECT:
                active = 0;
                break;
            case MENU_ACTION_LEFT:
            case MENU_ACTION_UP:
                if (message_mode != MESSAGE_OK) {
                    cur_pos--;
                    if (cur_pos < 0) {
                        cur_pos = button_count - 1;
                    }
                }
                break;
            case MENU_ACTION_RIGHT:
            case MENU_ACTION_DOWN:
                if (message_mode != MESSAGE_OK) {
                    cur_pos++;
                    if (cur_pos >= button_count) {
                        cur_pos = 0;
                    }
                }
                break;
            default:
                SDL_Delay(10);
                break;
        }
    }
    return cur_pos;
}

static int activate_dialog(const char *title, const char *message, int message_mode)
{
    int retval;

    sdl_ui_activate_pre_action();
    retval = handle_message_box(title, message, message_mode);
    sdl_ui_activate_post_action();

    return retval;
}

int message_box(const char *title, char *message, int message_mode)
{
    sdl_ui_init_draw_params();

    menu_draw = sdl_ui_get_menu_param();

    if (!sdl_menu_state) {
        return activate_dialog(title, (const char *)message, message_mode);
    }
    return handle_message_box(title, (const char *)message, message_mode);
}
