/*
 * tuimenu.c - A (very) simple text-based menu.
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
#include <ctype.h>
#include <keys.h>
#include <pc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"
#include "log.h"
#include "resources.h"
#include "tui.h"
#include "tui_backend.h"
#include "tuimenu.h"

#undef MENU_STARTS_UNHIGHLIGHTED

/* ------------------------------------------------------------------------- */

typedef struct tui_menu_item tui_menu_item_t;

struct tui_menu_item {
    /* Item type.  */
    enum {
        TUI_MENU_COMMAND,       /* Call a callback function.  */
        TUI_MENU_SUBMENU,       /* Activate a submenu.  */
        TUI_MENU_SEPARATOR      /* Unreachable separator.  */
    } type;

    /* Label for this menu item.  */
    char *label;

    /* Help string for this menu item.  */
    char *help_string;

    /* Hot key.  */
    char hot_key;

    /* Place of the hot key on the label.  */
    int hot_key_offset;

    /* Function to call when this item is activated.  */
    tui_menu_callback_t callback;

    /* Extra parameter to pass to the callback function.  */
    void *callback_param;

    /* Optional parameter string.  If NULL, no parameter is displayed.
       (Always NULL for separators and submenus.)  */
    char *par_string;

    /* Maximum length of the parameter string.  (Always zero for separators
       and submenus.)  */
    int par_string_max_len;

    /* Behavior of this item when activated.  */
    tui_menu_item_behavior_t behavior;

    /* Link to the submenu (if needed).  */
    tui_menu_t submenu;

    /* Links to the next and previous items.  */
    tui_menu_item_t *next, *prev;
};

struct tui_menu {
    /* Menu title (can be NULL).  */
    char *title;

    /* Number of items in this menu.  */
    int num_items;

    /* Maximum width of the largest item.  This also considers the parameter
       on the right.  */
    int width;

    /* Specifies the number of lines between menu items; e.g. 1 means one
       item per line, 2 one item every two lines.  */
    int spacing;

    /* Menu items.  */
    tui_menu_item_t *item_list;

    /* Default item when the menu is open.  */
    int default_item;
};

/* ------------------------------------------------------------------------- */

static void tui_menu_call_callback(tui_menu_item_t *item, int param, int *become_default, tui_menu_item_behavior_t *behavior)
{
    tui_menu_callback_t callback = item->callback;
    tui_menu_item_behavior_t dummy;
    const char *new_par_string;

    if (callback == NULL) {
        lib_free(item->par_string);
        item->par_string = NULL;
        return;
    }

    new_par_string = (*callback)(param, item->callback_param, become_default, behavior == NULL ? &dummy : behavior);

    if (new_par_string == NULL) {
        lib_free(item->par_string);
        item->par_string = NULL;
    } else {
        if (item->par_string != NULL) {
            item->par_string = lib_realloc(item->par_string, strlen(new_par_string) + 1);
            strcpy(item->par_string, new_par_string);
        } else {
            item->par_string = lib_stralloc(new_par_string);
        }
    }
}

void tui_menu_update(tui_menu_t menu)
{
    tui_menu_item_t *p;
    int i;

    for (p = menu->item_list, i = 0; p != NULL; p = p->next, i++) {
        int become_default = 0;

        switch (p->type) {
            case TUI_MENU_COMMAND:
                tui_menu_call_callback(p, 0, &become_default, NULL);
                break;
            case TUI_MENU_SUBMENU:
                tui_menu_call_callback(p, 0, &become_default, NULL);
                tui_menu_update(p->submenu);
                break;
            default:
                break;
        }

        if (become_default) {
            menu->default_item = i;
        }
    }
}

static void tui_menu_item_free(tui_menu_item_t *item)
{
    lib_free(item->label);
    item->label = NULL;
    lib_free(item->help_string);
    item->help_string = NULL;
    lib_free(item->par_string);
    item->par_string = NULL;
}

tui_menu_t tui_menu_create(const char *title, int spacing)
{
    tui_menu_t new;

    new = lib_malloc(sizeof(struct tui_menu));

    if (title != NULL) {
        new->title = lib_stralloc(title);
        new->width = strlen(title) + 4;
        if (new->width % 2 != 0) {
            new->width++;
        }
    } else {
        new->title = NULL;
        new->width = 0;
    }

    new->spacing = spacing;
    new->num_items = 0;
    new->item_list = NULL;
    new->default_item = 0;

    return new;
}

void tui_menu_free(tui_menu_t menu)
{
    tui_menu_item_t *p, *pnext;

    lib_free(menu->title);
    menu->title = NULL;
    for (p = menu->item_list; p != NULL; p = pnext) {
        pnext = p->next;
        tui_menu_item_free(p);
    }
}

static tui_menu_item_t *tui_menu_find_last_item(tui_menu_t menu)
{
    tui_menu_item_t *p;

    if (menu->item_list == NULL) {
        return NULL;
    }

    p = menu->item_list;
    while (p->next != NULL) {
        p = p->next;
    }

    return p;
}

static tui_menu_item_t *tui_menu_add_generic(tui_menu_t menu)
{
    tui_menu_item_t *p;

    p = tui_menu_find_last_item(menu);
    if (p == NULL) {
        p = menu->item_list = lib_malloc(sizeof(tui_menu_item_t));
        memset(p, 0, sizeof(*p));
    } else {
        p->next = lib_malloc(sizeof(tui_menu_item_t));
        memset(p->next, 0, sizeof(*p));
        p->next->prev = p;
        p = p->next;
    }

    return p;
}

static int set_label(tui_menu_item_t *item, const char *label)
{
    char *p;

    /* Find the hot key and allocate the label removing the corresponding
       hot key prefix.  */
    p = strchr(label, TUI_MENU_HOT_KEY_PREFIX);
    if (p == NULL) {
        item->hot_key_offset = -1;
        item->hot_key = '\0';
        item->label = lib_stralloc(label);
        return strlen(item->label);
    } else {
        item->hot_key_offset = p - label;
        item->hot_key = toupper(*(p + 1));
        item->label = lib_malloc(strlen(label));
        if (item->hot_key_offset != 0) {
            memcpy(item->label, label, item->hot_key_offset);
        }
        strcpy(item->label + item->hot_key_offset, p + 1);
        return strlen(item->label);
    }
}

void tui_menu_add_item(tui_menu_t menu, const char *label,
                       const char *help_string,
                       tui_menu_callback_t callback,
                       void *callback_param, int par_string_max_len,
                       tui_menu_item_behavior_t behavior)
{
    tui_menu_item_t *new;
    int width, dummy;

    if (menu == NULL) {
        log_message(LOG_DEFAULT, "Invalid menu!");
        return;
    }

    new = tui_menu_add_generic(menu);

    width = set_label(new, label);

    if (par_string_max_len != 0) {
        width += par_string_max_len + 1;
    }

    new->type = TUI_MENU_COMMAND;
    new->callback = callback;
    new->callback_param = callback_param;
    new->par_string = NULL;
    new->par_string_max_len = par_string_max_len;
    new->behavior = behavior;

    menu->num_items++;

    new->help_string = help_string != NULL ? lib_stralloc(help_string) : NULL;

    if (width > menu->width) {
        menu->width = width;
    }

    /* Make sure `par_string' is initialized.  */
    tui_menu_call_callback(new, 0, &dummy, NULL);
}

void tui_menu_add_separator(tui_menu_t menu)
{
    tui_menu_item_t *new = tui_menu_add_generic(menu);

    new->type = TUI_MENU_SEPARATOR;
    menu->num_items++;
}

void tui_menu_add_submenu(tui_menu_t menu, const char *label,
                          const char *help_string,
                          tui_menu_t submenu,
                          tui_menu_callback_t callback,
                          void *callback_param, int par_string_max_len)
{
    tui_menu_item_t *new = tui_menu_add_generic(menu);
    int width;

    new->type = TUI_MENU_SUBMENU;
    new->callback = callback;
    new->callback_param = callback_param;
    new->par_string = NULL;
    new->par_string_max_len = par_string_max_len;
    new->submenu = submenu;
    new->behavior = TUI_MENU_BEH_CONTINUE;

    width = set_label(new, label);
    if (par_string_max_len != 0) {
        width += par_string_max_len + 1;
    }

    if (width > menu->width) {
        menu->width = width;
    }

    new->help_string = help_string != NULL ? lib_stralloc(help_string) : NULL;

    menu->num_items++;
}

void tui_menu_add(tui_menu_t menu, const tui_menu_item_def_t *def)
{
    const tui_menu_item_def_t *p = def;

    while (p->label != NULL) {
        if (p->submenu != NULL) {
            tui_menu_t s = tui_menu_create(p->submenu_title, 1);

            tui_menu_add(s, p->submenu);
            tui_menu_add_submenu(menu, p->label,
                                 p->help_string,
                                 s,
                                 p->callback,
                                 p->callback_param, p->par_string_max_len);
        } else if (*p->label == '-') {
           tui_menu_add_separator(menu);
        } else {
            tui_menu_add_item(menu, p->label,
                              p->help_string,
                              p->callback,
                              p->callback_param, p->par_string_max_len,
                              p->behavior);
        }
        p++;
    }
}

/* ------------------------------------------------------------------------- */

static void tui_menu_display_item(tui_menu_item_t *item, int width, int x, int y, int highlight)
{
    int x_stop = x + width - 1;
    int background_color = highlight ? MENU_HIGHLIGHT : MENU_BACK;
    int i;

    tui_set_attr(MENU_FORE, background_color, 0);

    tui_put_char(x, y, ' ');
    x++;
    for (i = 0; item->label[i] != '\0' && x < x_stop; x++, i++) {
        if (i == item->hot_key_offset) {
            tui_set_attr(MENU_HOTKEY, background_color, 0);
            tui_put_char(x, y, item->label[i]);
            tui_set_attr(MENU_FORE, background_color, 0);
        } else {
            tui_put_char(x, y, item->label[i]);
        }
    }

    if (item->par_string != NULL) {
        int len = strlen(item->par_string);
        char *p;

        if (len > item->par_string_max_len) {
            p = item->par_string + len - item->par_string_max_len;
            len = item->par_string_max_len;
            tui_display(x_stop - len, y, len + 1, "%s", p);
            tui_display(x_stop - len, y, 0, "..");
        } else {
            p = item->par_string;
            tui_display(x_stop - len, y, len + 1, "%s", p);
        }
        tui_hline(x, y, ' ', x_stop - len - x);
    } else {
        tui_hline(x, y, ' ', x_stop - x + 1);
    }
}

int tui_menu_handle(tui_menu_t menu, char hotkey)
{
    tui_menu_item_t *item_ptr;
    tui_area_t backing_store = NULL;
    int total_width, total_height;
    int menu_x, menu_y;
    int current_item;
    int need_update;
    int y;

    _setcursortype(_NOCURSOR);

    tui_flush_keys();

    total_width = menu->width + 4;

    if (total_width > tui_num_cols() - 2) {
        total_width = tui_num_cols() - 2;
    }

    total_height = menu->spacing * menu->num_items + 2;
    if (menu->spacing > 1) {
        total_height -= menu->spacing - 1;
    }

    menu_x = CENTER_X(total_width);
    menu_y = CENTER_Y(total_height);

    tui_display_window(menu_x, menu_y, total_width, total_height, MENU_BORDER, MENU_BACK, menu->title, &backing_store);

    {
        int i;

        current_item = menu->default_item;
        for (i = 0, item_ptr = menu->item_list; i < current_item; i++) {
            item_ptr = item_ptr->next;
        }
    }

    need_update = 1;

    while (1) {
        int key;

        if (need_update) {
            tui_menu_item_t *p;

            /* Redraw the menu.  */
            for (p = menu->item_list, y = menu_y + 1; p != NULL; p = p->next, y += menu->spacing) {
                if (p->type != TUI_MENU_SEPARATOR) {
                    tui_menu_display_item(p, total_width - 2, menu_x + 1, y, 0);
                }
            }

            need_update = 0;
        }

        y = menu_y + menu->spacing * current_item + 1;
        if (item_ptr != NULL) {
            tui_menu_display_item(item_ptr, total_width - 2, menu_x + 1, y, 1);
            tui_set_attr(FIRST_LINE_FORE, FIRST_LINE_BACK, 0);
            tui_display(0, tui_num_lines() - 1, tui_num_cols(), "%s", item_ptr->help_string != NULL ? item_ptr->help_string : "");
        } else {
            tui_set_attr(FIRST_LINE_FORE, FIRST_LINE_BACK, 0);
            tui_display(0, tui_num_lines() - 1, tui_num_cols(), "");
        }

        /* The first keypress is the caller-specified one.  */
        if (hotkey != 0) {
            key = hotkey;
            hotkey = 0;
        } else {
            key = getkey();
        }

        if (item_ptr != NULL) {
            tui_menu_display_item(item_ptr, total_width - 2, menu_x + 1, y, 0);
        }

        switch (key) {
            case K_Escape:
            case K_Left:
                tui_area_put(backing_store, menu_x, menu_y);
                tui_area_free(backing_store);
                return 0;           /* Leave this menu.  */
            case K_Tab:
                tui_area_put(backing_store, menu_x, menu_y);
                tui_area_free(backing_store);
                return 1;           /* Resume emulation.  */
            case K_Up:
                if (item_ptr == NULL || item_ptr->prev == NULL) {
                    current_item = menu->num_items - 1;
                    item_ptr = tui_menu_find_last_item(menu);
                } else if (current_item > 0) {
                    do {
                        item_ptr = item_ptr->prev;
                        current_item--;
                    } while (item_ptr != NULL && item_ptr->type == TUI_MENU_SEPARATOR);
                    if (item_ptr == NULL) {
                        current_item = menu->num_items - 1;
                        item_ptr = tui_menu_find_last_item(menu);
                    }
                }
                break;
            case K_Down:
                if (item_ptr == NULL || item_ptr->next == NULL) {
                    item_ptr = menu->item_list;
                    current_item = 0;
                } else {
                    do {
                        item_ptr = item_ptr->next;
                        current_item++;
                    } while (item_ptr != NULL && item_ptr->type == TUI_MENU_SEPARATOR);
                    if (item_ptr == NULL) {
                        item_ptr = menu->item_list;
                        current_item = 0;
                    }
                }
                break;
            case K_Home:
                item_ptr = menu->item_list;
                current_item = 0;
                break;
            case K_End:
                current_item = menu->num_items - 1;
                item_ptr = tui_menu_find_last_item(menu);
                break;
            case ' ':
            case K_Return:
            case K_Right:
                if (item_ptr != NULL) {
                    int ret = 0;
                    int become_default = 1;
                    tui_menu_item_behavior_t behavior;

                    tui_menu_display_item(item_ptr, total_width - 2, menu_x + 1, y, 1);

                    if (item_ptr->type == TUI_MENU_SUBMENU) {
                        ret = tui_menu_handle(item_ptr->submenu, 0);
                    }

                    behavior = item_ptr->behavior;

                    tui_menu_call_callback(item_ptr, 1, &become_default, &behavior);
                    if (become_default) {
                        menu->default_item = current_item;
                    }

                    if (ret || behavior != TUI_MENU_BEH_CONTINUE) {
                        tui_area_put(backing_store, menu_x, menu_y);
                        tui_area_free(backing_store);
                        if (behavior == TUI_MENU_BEH_RESUME) {
                            return 1;
                        } else {
                            return ret;
                        }
                    }

                    need_update = 1;
                }
                break;
            default:
                if (key <= 0xff && isalnum((char)key)) {
                    int key_char = toupper((char)key);
                    tui_menu_item_t *p;
                    int i;

                    for (p = menu->item_list, i = 0; p != NULL; p = p->next, i++) {
                        if (p->hot_key == key_char) {
                            int ret = 0;
                            int become_default = 1;
                            tui_menu_item_behavior_t behavior;

                            item_ptr = p;
                            current_item = i;

                            /* The action could change values in the current
                               menu.  */
                            need_update = 1;

                            y = menu_y + menu->spacing * current_item + 1;
                            tui_menu_display_item(item_ptr, total_width - 2, menu_x + 1, y, 1);
                            if (p->type == TUI_MENU_SUBMENU) {
                                ret = tui_menu_handle(p->submenu, 0);
                            }

                            behavior = p->behavior;

                            tui_menu_call_callback(p, 1, &become_default, &behavior);
                            if (become_default) {
                                menu->default_item = current_item;
                            }

                            if (ret || behavior != TUI_MENU_BEH_CONTINUE) {
                                tui_area_put(backing_store, menu_x, menu_y);
                                tui_area_free(backing_store);
                                if (behavior == TUI_MENU_BEH_RESUME) {
                                    return 1;
                                } else {
                                    return ret;
                                }
                            }
                            break;
                        }
                    }
                }
        }
    }
}

/* ------------------------------------------------------------------------- */

/* These functions are called by radio and toggle menu items if the callback
   functions are defined through `TUI_MENU_DEFINE_TOGGLE()',
   `TUI_MENU_DEFINE_RADIO()' or `TUI_MENU_DEFINE_STRING_RADIO()'.  */

const char *_tui_menu_toggle_helper(int been_activated, const char *resource_name)
{
    int value, r;

    if (been_activated) {
        r = resources_toggle(resource_name, &value);
        if (r < 0) {
            r = resources_get_int(resource_name, &value);
        }
    } else {
        r = resources_get_int(resource_name, &value);
    }

    if (r < 0) {
        return "Unknown";
    } else {
        return value ? "On" : "Off";
    }
}

const char *_tui_menu_radio_helper(int been_activated, void *param, int *become_default, const char *resource_name)
{
    if (been_activated) {
        resources_set_value(resource_name, (resource_value_t)param);
        *become_default = 1;
    } else {
        resource_value_t v;
        resources_get_value(resource_name, (void *)&v);
        if (v == (resource_value_t)param) {
            *become_default = 1;
        }
    }
    return NULL;
}
