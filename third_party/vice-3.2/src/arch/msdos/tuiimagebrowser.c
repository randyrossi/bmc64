/*
 * tuiimagebrowser.c - Explore disk/tape image contents.
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

#include <conio.h>
#include <keys.h>
#include <pc.h>
#include <stdio.h>

#include "cbmcharsets.h"
#include "imagecontents.h"
#include "lib.h"
#include "tui.h"
#include "tui_backend.h"
#include "types.h"

#include "tuiimagebrowser.h"

#define WIDTH  35
#define HEIGHT 17

/* Character conversion table being used to translate from PETSCII to the VGA
   CBM-enhanched character set.  */
static uint8_t *char_conv_table;

static int petscii_display(int x, int y, uint8_t *data)
{
    int i;

    for (i = 0; data[i] != 0; i++, x++) {
        uint8_t c;

        c = char_conv_table[(unsigned int)data[i]];
        tui_put_char(x, y, c);
    }

    return x;
}

static void display_item(int x, int y, int width, int offset, image_contents_file_list_t *element, int is_selected)
{
    int x1, y1;

    y1 = y + offset;

    if (is_selected) {
        tui_set_attr(MENU_FORE, MENU_HIGHLIGHT, 0);
    } else {
        tui_set_attr(MENU_FORE, MENU_BACK, 0);
    }
    tui_display(x, y1, 6, "%-6d", element->size);

    x1 = x + 6;
    tui_put_char(x1++, y1, '\"');
    x1 = petscii_display(x1, y1, element->name);
    tui_put_char(x1++, y1, '\"');

    for (; x1 < x + width - IMAGE_CONTENTS_TYPE_LEN; x1++) {
        tui_put_char(x1, y1, ' ');
    }
    x1 = petscii_display(x1, y1, element->type);
    for (; x1 < x + width; x1++) {
        tui_put_char(x1, y1, ' ');
    }
}

static void update(int x, int y, int width, int height, image_contents_file_list_t *first, image_contents_file_list_t *selected)
{
    image_contents_file_list_t *p;
    int i;

    for (p = first, i = 0; p != NULL && i < height; p = p->next, i++) {
        display_item(x, y, width, i, p, p == selected);
    }

    tui_set_attr(MENU_FORE, MENU_BACK, 0);
    for (; i < height; i++) {
        tui_hline(x, y + i, ' ', width);
    }
}

static void display_title(int x, int y, uint8_t *name, uint8_t *id)
{
    tui_set_attr(MENU_FORE, MENU_BACK, 0);

    tui_put_char(x, y, '\"');
    x = petscii_display(x + 1, y, name);
    tui_put_char(x++, y, '\"');
    if (*id != 0) {
        tui_put_char(x++, y, ' ');
        petscii_display(x, y, id);
    }
}

char *tui_image_browser(const char *filename, read_contents_func_type contents_func, unsigned int *file_number)
{
    image_contents_t *contents;
    tui_area_t backing_store = NULL;
    image_contents_file_list_t *first, *current;
    int first_number, current_number;
    int x, y;
    int real_width, real_height;
    int real_x, real_y;
    int need_update;

    if (char_conv_table == NULL) {
        char_conv_table = cbm_petscii_graphics_to_charset;
    }

    contents = contents_func(filename);
    if (contents == NULL) {
        tui_error("Invalid image");
        return NULL;
    }

    x = CENTER_X(WIDTH);
    y = CENTER_Y(HEIGHT);
    tui_display_window(x, y, WIDTH, HEIGHT, MENU_BORDER, MENU_BACK, NULL, &backing_store);

    real_width = WIDTH - 4;
    real_height = HEIGHT - 4;
    real_x = x + 2;
    real_y = y + 3;

    tui_set_attr(MENU_BORDER, MENU_BACK, 0);
    tui_hline(real_x - 1, y + 2, 0xc4, real_width + 2);

    if (contents->blocks_free >= 0) {
        real_height -= 2;

        tui_set_attr(MENU_FORE, MENU_BACK, 0);
        tui_display(real_x, y + HEIGHT - 2, real_width, "%d blocks free.", contents->blocks_free);
        tui_set_attr(MENU_BORDER, MENU_BACK, 0);
        tui_hline(real_x - 1, y + HEIGHT - 3, 0xc4, real_width + 2);
    }

    first_number = current_number = 0;
    first = current = contents->file_list;

    if (current != NULL) {
        need_update = 1;
    } else {
        need_update = 0;
        tui_display(real_x, real_y, real_width, "(Empty image)");
    }

    tui_set_attr(FIRST_LINE_FORE, FIRST_LINE_BACK, 0);
    tui_display(0, tui_num_lines() - 1, tui_num_cols(), "\030\031: Move  <Enter>: Autostart  <Backspace>: Switch charset");

    while (1) {
        int key;

        if (need_update) {
            display_title(real_x, y + 1, contents->name, contents->id);
            update(real_x, real_y, real_width, real_height, first, current);
            need_update = 0;
        } else if (current != NULL) {
            display_item(real_x, real_y, real_width, current_number - first_number, current, 1);
        }

        key = getkey();

        if (current != NULL) {
            display_item(real_x, real_y, real_width, current_number - first_number, current, 0);
        }

        if (key == K_Escape || key == K_Left) {
            tui_area_put(backing_store, x, y);
            tui_area_free(backing_store);
            image_contents_destroy(contents);
            return NULL;
        } else if (current != NULL) {
            switch (key) {
                /* Backspace switches charset.  */
                case K_BackSpace:
                    if (char_conv_table == cbm_petscii_business_to_charset) {
                        char_conv_table = cbm_petscii_graphics_to_charset;
                    } else {
                        char_conv_table = cbm_petscii_business_to_charset;
                    }
                    need_update = 1;
                    break;
                /* Return autostarts the selected file.  */
                case K_Return:
                case K_Right:
                    {
                        char *retval;

                        tui_area_put(backing_store, x, y);
                        tui_area_free(backing_store);
                        retval = lib_stralloc(current->name);
                        image_contents_destroy(contents);
                        if (file_number != NULL) {
                            *file_number = current_number + 1;
                        }
                        return retval;
                    }
                /* Movement commands.  */
                case K_Up:
                    if (contents->file_list != NULL && current_number > 0) {
                        current_number--;
                        current = current->prev;
                        if (current_number < first_number) {
                            first_number--;
                            first = first->prev;
                            need_update = 1;
                        }
                    }
                    break;
                case K_Home:
                    if (contents->file_list != NULL) {
                        first_number = current_number = 0;
                        first = current = contents->file_list;
                        need_update = 1;
                    }
                    break;
                case K_PageUp:
                    if (current != NULL) {
                        int i;

                        for (i = 0; i < real_height && first_number > 0; i++) {
                            first_number--;
                            first = first->prev;
                            current_number--;
                            current = current->prev;
                        }
                        need_update = 1;
                    }
                    break;
                case K_Down:
                    if (contents->file_list != NULL && current->next != NULL) {
                        current_number++;
                        current = current->next;
                        if (current_number - first_number >= real_height) {
                            first_number++;
                            first = first->next;
                            need_update = 1;
                        }
                    }
                    break;
                case K_End:
                    if (contents->file_list != NULL) {
                        while (current->next != NULL) {
                            current_number++;
                            current = current->next;
                            if (current_number - first_number >= real_height) {
                                first_number++;
                                first = first->next;
                                need_update = 1;
                            }
                        }
                    }
                    break;
                case K_PageDown:
                    if (contents->file_list != NULL) {
                        int i;

                        for (i = 0; i < real_height && current->next != NULL; i++) {
                            current_number++;
                            current = current->next;
                            first_number++;
                            first = first->next;
                        }
                        need_update = 1;
                    }
                    break;
            }
        }
    }
}
