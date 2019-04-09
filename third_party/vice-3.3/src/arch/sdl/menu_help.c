/*
 * menu_help.c - SDL help menu functions.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#include <stdlib.h>
#include "vice_sdl.h"

#include "cmdline.h"
#include "info.h"
#include "lib.h"
#include "menu_common.h"
#include "menu_help.h"
#include "ui.h"
#include "uimenu.h"
#include "util.h"
#include "version.h"
#include "vicefeatures.h"

#ifdef USE_SVN_REVISION
#include "svnversion.h"
#endif

#ifdef WINMIPS
static char *concat_all(char **text)
{
    int i;
    char *new_text;
    char *old_text = lib_stralloc("\n");

    for (i = 0; text[i] != NULL; i++) {
        new_text = util_concat(old_text, text[i], NULL);
        lib_free(old_text);
        old_text = new_text;
    }
    return new_text;
}
#endif

static void make_n_cols(char *text, int len, int cols)
{
    int i = cols;

    while (i < len) {
        while (text[i] != ' ' && (i > 0)) {
            i--;
        }

        if (i == 0) {
            /* line with >40 chars and no spaces;
               abort and let sdl_ui_print truncate */
            return;
        }

        text[i] = '\n';
        text += i + 1;
        len -= i + 1;
        i = cols;
    }
}

static char *convert_cmdline_to_n_cols(const char *text, int cols)
{
    char *new_text;
    int num_options;
    int current_line;
    int i, j, index;

    num_options = cmdline_get_num_options();
    new_text = lib_malloc(strlen(text) + num_options);

    new_text[0] = '\n';
    index = 1;
    current_line = 1;
    for (i = 0; i < num_options; i++) {
        for (j = 0; text[current_line + j] != '\n'; j++) {
            new_text[index] = text[current_line + j];
            index++;
        }
        new_text[index] = '\n';
        index++;
        current_line += j + 2;
        for (j = 0; text[current_line + j] != '\n'; j++) {
            new_text[index + j] = text[current_line + j];
        }

        new_text[index + j] = '\n';

        if (j > cols) {
            make_n_cols(&(new_text[index]), j, cols);
        }

        current_line += j + 1;
        index += j + 1;
        new_text[index] = '\n';
        index++;
    }
    return new_text;
}

static char *contrib_convert(const char *text, int cols)
{
    char *new_text;
    char *pos;
    unsigned int i = 0;
    unsigned int j = 0;
    int single = 0;
    int len;
    size_t size;

    size = strlen(text);
    new_text = lib_malloc(size);
    while (i < size) {
        if (text[i] == ' ' && text[i + 1] == ' ' && text[i - 1] == '\n') {
            i += 2;
        } else {
            if ((text[i] == ' ' || text[i] == '\n') && text[i + 1] == '<') {
                while (text[i] != '>') {
                    i++;
                }
                i++;
            } else {
                new_text[j] = text[i];
                j++;
                i++;
            }
        }
    }
    new_text[j] = 0;

    i = 0;
    j = strlen(new_text);

    while (i < j) {
        if (new_text[i] == '\n') {
            if (new_text[i + 1] == '\n') {
                if (single) {
                    single = 0;
                }
                if (new_text[i - 1] == ':' && new_text[i - 2] == 'e') {
                    single = 1;
                }
                new_text[i + 1] = 0;
                i++;
            } else {
                if (!single) {
                    new_text[i] = ' ';
                }
            }
        }
        i++;
    }

    pos = new_text;
    while (*pos != 0) {
        len = strlen(pos);
        make_n_cols(pos, len, cols);
        pos += len + 1;
    }

    for (i = 0; i < j; i++) {
        if (new_text[i] == 0) {
            new_text[i] = '\n';
        }
    }

    return new_text;
}

/* gets an offset into the text and an amount of lines to scroll
   up, returns a new offset */
static unsigned int scroll_up(const char *text, int offset, int amount)
{
    while ((amount--) && (offset >= 2)) {
        int i = offset - 2;

        while ((i >= 0) && (text[i] != '\n')) {
            --i;
        }

        offset = i + 1;
    }
    if (offset < 0) {
        offset = 0;
    }
    return offset;
}

#define CHARCODE_UMLAUT_A_LOWER         ((char)0xe4)
#define CHARCODE_UMLAUT_A_UPPER         ((char)0xc4)

#define CHARCODE_UMLAUT_O_LOWER         ((char)0xf6)
#define CHARCODE_UMLAUT_O_UPPER         ((char)0xd6)

#define CHARCODE_UMLAUT_U_LOWER         ((char)0xfc)
#define CHARCODE_UMLAUT_U_UPPER         ((char)0xdc)

#define CHARCODE_GRAVE_E_LOWER          ((char)0xe8)
#define CHARCODE_AIGU_E_LOWER           ((char)0xe9)

#define CHARCODE_KROUZEK_A_LOWER        ((char)0xe5)

static void show_text(const char *text)
{
    int first_line = 0;
    int last_line = 0;
    int next_line = 0;
    int next_page = 0;
    unsigned int current_line = 0;
    unsigned int this_line = 0;
    unsigned int len;
    int x, y, z;
    int active = 1;
    int active_keys;
    char *string;
    menu_draw_t *menu_draw;

    menu_draw = sdl_ui_get_menu_param();

    string = lib_malloc(0x400);
    len = strlen(text);

    /* find out how many lines */
    for (x = 0, z = 0; text[x] != 0; x++) {
        if (text[x] == '\n') {
            last_line++;
        }
    }

    last_line -= menu_draw->max_text_y;
    for (x = 0, z = 0; text[x] != 0; x++) {
        if (last_line == 0) {
            break;
        }
        if (text[x] == '\n') {
            last_line--;
        }
    }
    last_line = x; /* save the offset */

    while (active) {
        sdl_ui_clear();
        first_line = current_line;
        this_line = current_line;
        for (y = 0; (y < menu_draw->max_text_y) && (this_line < len); y++) {
            z = 0;
            for (x = 0; (text[this_line + x] != '\n') &&
                        (text[this_line + x] != 0); x++) {
                switch (text[this_line + x]) {
                    case '`':
                        string[x + z] = '\'';
                        break;
                    /* FIXME: we should actually be able to handle some of these */
                    case CHARCODE_UMLAUT_A_LOWER:
                    case CHARCODE_KROUZEK_A_LOWER:
                        string[x + z] = 'a';
                        break;
                    case CHARCODE_UMLAUT_A_UPPER:
                        string[x + z] = 'A';
                        break;
                    case '~':
                        string[x + z] = '-';
                        break;
                    case CHARCODE_GRAVE_E_LOWER:
                    case CHARCODE_AIGU_E_LOWER:
                        string[x + z] = 'e';
                        break;
                    case CHARCODE_UMLAUT_O_UPPER:
                        string[x + z] = 'O';
                        break;
                    case CHARCODE_UMLAUT_O_LOWER:
                        string[x + z] = 'o';
                        break;
                    case CHARCODE_UMLAUT_U_UPPER:
                        string[x + z] = 'U';
                        break;
                    case CHARCODE_UMLAUT_U_LOWER:
                        string[x + z] = 'u';
                        break;
                    case '\t':
                        string[x + z] = ' ';
                        string[x + z + 1] = ' ';
                        string[x + z + 2] = ' ';
                        string[x + z + 3] = ' ';
                        z += 3;
                        break;
                    default:
                        string[x + z] = text[this_line + x];
                        break;
                }
            }
            if (x != 0) {
                string[x + z] = 0;
                sdl_ui_print(string, 0, y);
            }
            if (y == 0) {
                next_line = this_line + x + 1;
            }
            this_line += x + 1;
        }
        next_page = this_line;
        active_keys = 1;
        sdl_ui_refresh();

        while (active_keys) {
            switch (sdl_ui_menu_poll_input()) {
                case MENU_ACTION_CANCEL:
                case MENU_ACTION_EXIT:
                case MENU_ACTION_SELECT:
                    active_keys = 0;
                    active = 0;
                    break;
                case MENU_ACTION_RIGHT:
                case MENU_ACTION_PAGEDOWN:
                    active_keys = 0;
                    if (current_line < last_line) {
                        current_line = next_page;
                        if (current_line > last_line) {
                            current_line = last_line;
                        }
                    }
                    break;
                case MENU_ACTION_DOWN:
                    active_keys = 0;
                    if (current_line < last_line) {
                        current_line = next_line;
                        if (current_line > last_line) {
                            current_line = last_line;
                        }
                    }
                    break;
                case MENU_ACTION_LEFT:
                case MENU_ACTION_PAGEUP:
                    active_keys = 0;
                    current_line = scroll_up(text, first_line, menu_draw->max_text_y);
                    break;
                case MENU_ACTION_UP:
                    active_keys = 0;
                    current_line = scroll_up(text, first_line, 1);
                    break;
                case MENU_ACTION_HOME:
                    active_keys = 0;
                    current_line = 0;
                    break;
                case MENU_ACTION_END:
                    active_keys = 0;
                    if (current_line < last_line) {
                        current_line = last_line;
                    }
                    break;
                default:
                    SDL_Delay(10);
                    break;
            }
        }
    }
    lib_free(string);
}

static UI_MENU_CALLBACK(about_callback)
{
    int active = 1;
    int i;
    int j;
    char *tmp;

    if (activated) {
        sdl_ui_clear();
        i = 0;
        sdl_ui_print_center("VICE", i++);
        sdl_ui_print_center("Versatile Commodore Emulator", i++);
#ifdef USE_SVN_REVISION
        sdl_ui_print_center("Version " VERSION " rev " VICE_SVN_REV_STRING, i);
#else
        sdl_ui_print_center("Version " VERSION, i);
#endif
        i++;
#ifdef USE_SDLUI2
        sdl_ui_print_center("SDL2", i++);
#else
        sdl_ui_print_center("SDL", i++);
#endif
        i++;
        sdl_ui_print_center("The VICE Team", i++);
        for (j = 0; core_team[j].name; j++) {
            tmp = util_concat("(C) ", core_team[j].years, " ", core_team[j].name, NULL);
            sdl_ui_print_center(tmp, i++);
            lib_free(tmp);
        }
        sdl_ui_refresh();

        while (active) {
            switch (sdl_ui_menu_poll_input()) {
                case MENU_ACTION_CANCEL:
                case MENU_ACTION_EXIT:
                case MENU_ACTION_SELECT:
                    active = 0;
                    break;
                default:
                    SDL_Delay(10);
                    break;
            }
        }
    }
    return NULL;
}

static char *get_compiletime_features(void)
{
    feature_list_t *list;
    char *str, *lstr;
    unsigned int len = 0;

    list = vice_get_feature_list();
    while (list->symbol) {
        len += strlen(list->descr) + strlen(list->symbol) + (15);
        ++list;
    }
    str = lib_malloc(len);
    lstr = str;
    list = vice_get_feature_list();
    while (list->symbol) {
        sprintf(lstr, "%4s %s\n%s\n\n", list->isdefined ? "yes " : "no  ", list->symbol, list->descr);
        lstr += strlen(lstr);
        ++list;
    }
    return str;
}

static UI_MENU_CALLBACK(features_callback)
{
    /* menu_draw_t *menu_draw; */
    char *features;

    if (activated) {
        /* menu_draw = sdl_ui_get_menu_param(); */
        features = get_compiletime_features();
        show_text((const char *)features);
        lib_free(features);
    }
    return NULL;
}

static UI_MENU_CALLBACK(cmdline_callback)
{
    menu_draw_t *menu_draw;
    char *options;
    char *options_n;

    if (activated) {
        menu_draw = sdl_ui_get_menu_param();
        options = cmdline_options_string();
        options_n = convert_cmdline_to_n_cols(options, menu_draw->max_text_x);
        lib_free(options);
        show_text((const char *)options_n);
        lib_free(options_n);
    }
    return NULL;
}

static UI_MENU_CALLBACK(contributors_callback)
{
    menu_draw_t *menu_draw;
    char *info_contrib_text_n;
#ifdef WINMIPS
    char *new_text = NULL;
#endif

    if (activated) {
        menu_draw = sdl_ui_get_menu_param();
#ifdef WINMIPS
        new_text = concat_all(info_contrib_text);
        info_contrib_text_n = contrib_convert(new_text, menu_draw->max_text_x);
        lib_free(new_text);
#else
        info_contrib_text_n = contrib_convert(info_contrib_text, menu_draw->max_text_x);
#endif
        show_text((const char *)info_contrib_text_n);
        lib_free(info_contrib_text_n);
    }
    return NULL;
}

static UI_MENU_CALLBACK(license_callback)
{
    menu_draw_t *menu_draw;
#ifdef WINMIPS
    char *new_text = NULL;
#endif

    if (activated) {
        menu_draw = sdl_ui_get_menu_param();
        if (menu_draw->max_text_x > 60) {
#ifdef WINMIPS
            new_text = concat_all(info_license_text);
            show_text(new_text);
            lib_free(new_text);
#else
            show_text(info_license_text);
#endif
        } else {
#ifdef WINMIPS
            new_text = concat_all(info_license_text40);
            show_text(new_text);
            lib_free(new_text);
#else
            show_text(info_license_text40);
#endif
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(warranty_callback)
{
    menu_draw_t *menu_draw;

    if (activated) {
        menu_draw = sdl_ui_get_menu_param();
        if (menu_draw->max_text_x > 60) {
            show_text(info_warranty_text);
        } else {
            show_text(info_warranty_text40);
        }
    }
    return NULL;
}

const ui_menu_entry_t help_menu[] = {
    { "About",
      MENU_ENTRY_DIALOG,
      about_callback,
      NULL },
    { "Command-line options",
      MENU_ENTRY_DIALOG,
      cmdline_callback,
      NULL },
    { "Compile time features",
      MENU_ENTRY_DIALOG,
      features_callback,
      NULL },
    { "Contributors",
      MENU_ENTRY_DIALOG,
      contributors_callback,
      NULL },
    { "License",
      MENU_ENTRY_DIALOG,
      license_callback,
      NULL },
    { "Warranty",
      MENU_ENTRY_DIALOG,
      warranty_callback,
      NULL },
    SDL_MENU_LIST_END
};
