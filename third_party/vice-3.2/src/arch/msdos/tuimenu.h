/*
 * tuimenu.h - A (very) simple text-based menu.
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

#ifndef VICE_TUI_MENU_H
#define VICE_TUI_MENU_H

#include "vicemaxpath.h"

#define TUI_MENU_HOT_KEY_PREFIX		'_'

typedef enum tui_menu_item_behavior {
    TUI_MENU_BEH_CONTINUE,	/* Stay in the menu. */
    TUI_MENU_BEH_CLOSE,		/* Close the current menu. */
    TUI_MENU_BEH_RESUME		/* Leave all the menus. */
} tui_menu_item_behavior_t;

/* Menu item callback: every menu item is associated with a function of this
   type.  `been_activated' is nonzero if the menu has been activated; the
   function must return a pointer to the parameter string to be displayed on
   the right of the menu item.
   If `*become_default' is set to a nonzero value when the callback returns,
   the item that triggered the callback becomes the default item for this
   menu next time the menu is open.  It is nonzero when the callback is
   called, but the callback can set it to zero if needed.
   Likewise, `*behavior' defines the behavior of the item after it has been
   activated; the callback can leave it to the default (specified when the
   menu is created), or change it. */
typedef const char *(*tui_menu_callback_t)(int been_activated, void *callback_param, int *become_default, tui_menu_item_behavior_t *behavior);

/* Menu type. */
typedef struct tui_menu *tui_menu_t;

/* Struct to define a menu item.  */
struct tui_menu_item_def_s {
    char *label;
    const char *help_string;
    tui_menu_callback_t callback;
    void *callback_param;
    int par_string_max_len;
    tui_menu_item_behavior_t behavior;
    struct tui_menu_item_def_s *submenu;
    const char *submenu_title;
};
typedef struct tui_menu_item_def_s tui_menu_item_def_t;

#define TUI_MENU_ITEM_DEF_LIST_END { NULL, NULL, NULL, NULL, 0, (tui_menu_item_behavior_t)0, NULL, NULL }

extern tui_menu_t tui_menu_create(const char *title, int spacing);
extern void tui_menu_free(tui_menu_t menu);
extern void tui_menu_add_item(tui_menu_t menu, const char *label,
                              const char *help_string, tui_menu_callback_t callback,
                              void *callback_param, int par_string_max_len,
                              tui_menu_item_behavior_t behavior);
extern void tui_menu_add_submenu(tui_menu_t menu, const char *label,
                                 const char *help_string,
                                 tui_menu_t submenu,
                                 tui_menu_callback_t callback,
                                 void *callback_param, int par_string_max_len);
extern void tui_menu_add_separator(tui_menu_t menu);
extern void tui_menu_add(tui_menu_t menu, const tui_menu_item_def_t *d);
extern int tui_menu_handle(tui_menu_t menu, char hotkey);
extern void tui_menu_update(tui_menu_t menu);

/* ------------------------------------------------------------------------- */

#define TUI_MENU_CALLBACK(name) \
    const char *name(int been_activated, void *param, int *become_default, tui_menu_item_behavior_t *behavior)

#define TUI_MENU_DEFINE_TOGGLE(resource)                           \
    static TUI_MENU_CALLBACK(toggle_##resource##_callback)         \
    {                                                              \
        return _tui_menu_toggle_helper(been_activated, #resource); \
    }
 
#define TUI_MENU_DEFINE_RADIO(resource)                                                  \
    static TUI_MENU_CALLBACK(radio_##resource##_callback)                                \
    {                                                                                    \
        return _tui_menu_radio_helper(been_activated, param, become_default, #resource); \
    }

#define TUI_MENU_DEFINE_FILENAME(resource, name)                                                             \
    static TUI_MENU_CALLBACK(filename_##resource##_callback)                                                 \
    {                                                                                                        \
        char filename[PATH_MAX];                                                                             \
        const char *v;                                                                                       \
        char *tmp;                                                                                           \
                                                                                                             \
        if (been_activated) {                                                                                \
                                                                                                             \
            *filename = '\0';                                                                                \
                                                                                                             \
            if (tui_input_string("Change "name" image name", "New image name:", filename, PATH_MAX) == -1) { \
                return NULL;                                                                                 \
            }                                                                                                \
                                                                                                             \
            util_remove_spaces(filename);                                                                    \
                                                                                                             \
            if (*filename == '\0') {                                                                         \
                tmp = tui_file_selector("Choose "name" image", NULL, "*", NULL, NULL, NULL, NULL);           \
                if (tmp != NULL) {                                                                           \
                    strcpy(filename, tmp);                                                                   \
                    lib_free(tmp);                                                                           \
                } else {                                                                                     \
                    return NULL;                                                                             \
                }                                                                                            \
            }                                                                                                \
                                                                                                             \
            resources_set_string(#resource, filename);                                                       \
       }                                                                                                     \
                                                                                                             \
       resources_get_string(#resource, &v);                                                                  \
                                                                                                             \
       return v;                                                                                             \
    }

extern const char *_tui_menu_toggle_helper(int been_activated, const char *resource_name);
extern const char *_tui_menu_radio_helper(int been_activated, void *param, int *become_default, const char *resource_name);
extern char *_tui_menu_filename_helper(int been_activated, char *name, const char *resource_name);

#endif
