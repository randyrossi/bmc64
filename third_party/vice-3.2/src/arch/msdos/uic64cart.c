/*
 * uic64cart.c
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#include <stdio.h>
#include <string.h>

#include "cartridge.h"
#include "keyboard.h"
#include "lib.h"
#include "machine.h"
#include "menudefs.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "ui.h"
#include "uic64cart.h"
#include "util.h"

TUI_MENU_DEFINE_TOGGLE(CartridgeReset)

static TUI_MENU_CALLBACK(attach_cartridge_callback)
{
    if (been_activated) {
        char *default_item, *directory;
        char *name;
        const char *s, *filter;
        int type = (int)param;

        s = cartridge_get_file_name((uint16_t)0);
        util_fname_split(s, &directory, &default_item);

        filter = (type == CARTRIDGE_CRT) ? "*.crt" : "*";

        name = tui_file_selector("Attach cartridge image", directory, filter, default_item, NULL, NULL, NULL);
        if (name != NULL && (s == NULL || strcasecmp(name, s) != 0) && cartridge_attach_image(type, name) < 0) {
            tui_error("Invalid cartridge image.");
        }
        ui_update_menus();
        lib_free(name);
    }

    return NULL;
}

static TUI_MENU_CALLBACK(cartridge_set_default_callback)
{
    if (been_activated) {
        cartridge_set_default();
    }

    return NULL;
}


static TUI_MENU_CALLBACK(cartridge_callback)
{
    const char *s = cartridge_get_file_name((uint16_t)0);

    if (s == NULL || *s == '\0') {
        return "(none)";
    } else {
        return s;
    }
}

static tui_menu_item_def_t *attach_generic_cartridge_submenu_items = NULL;
static tui_menu_item_def_t *attach_ramex_cartridge_submenu_items = NULL;
static tui_menu_item_def_t *attach_freezer_cartridge_submenu_items = NULL;
static tui_menu_item_def_t *attach_game_cartridge_submenu_items = NULL;
static tui_menu_item_def_t *attach_util1_cartridge_submenu_items = NULL;
static tui_menu_item_def_t *attach_util2_cartridge_submenu_items = NULL;

static tui_menu_item_def_t attach_c64_cartridge_menu_items[] = {
    { "--" },
    { "Attach _CRT image...",
      "Attach a CRT image, autodetecting its type",
      attach_cartridge_callback, (void *)CARTRIDGE_CRT, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Generic cartridges:",
      "Attach a generic cartridge image",
      cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL,
      "Attach generic cartridge" },
    { "_RAM expansion cartridges:",
      "Attach a RAM expansion cartridge image",
      cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL,
      "Attach RAM expansion cartridge" },
    { "_Freezer cartridges:",
      "Attach a freezer cartridge image",
      cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL,
      "Attach freezer cartridge" },
    { "G_ame cartridges:",
      "Attach a game cartridge image",
      cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL,
      "Attach game cartridge" },
    { "_Utility cartridges (part 1):",
      "Attach a utility cartridge image",
      cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL,
      "Attach utility cartridge" },
    { "_Utility cartridges (part 2):",
      "Attach a utility cartridge image",
      cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL,
      "Attach utility cartridge" },
    { "--" },
    { "Set cartridge as _default",
      "Save the current cartridge to the settings",
      cartridge_set_default_callback, NULL, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Reset on cart change",
      "Reset the machine when a cart is inserted or detached",
      toggle_CartridgeReset_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t attach_scpu64_cartridge_menu_items[] = {
    { "--" },
    { "Attach _CRT image...",
      "Attach a CRT image, autodetecting its type",
      attach_cartridge_callback, (void *)CARTRIDGE_CRT, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_Generic cartridge:",
      "Attach a generic cartridge image",
      cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL,
      "Attach generic cartridge" },
    { "_RAM expansion cartridge:",
      "Attach a RAM expansion cartridge image",
      cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL,
      "Attach RAM expansion cartridge" },
    { "G_ame cartridge:",
      "Attach a game cartridge image",
      cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL,
      "Attach game cartridge" },
    { "_Utility cartridge:",
      "Attach a utility cartridge image",
      cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL,
      "Attach utility cartridge" },
    { "--" },
    { "Set cartridge as _default",
      "Save the current cartridge to the settings",
      cartridge_set_default_callback, NULL, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Reset on cart change",
      "Reset the machine when a cart is inserted or detached",
      toggle_CartridgeReset_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(detach_cartridge_callback)
{
    const char *s;

    if (been_activated) {
        cartridge_detach_image(-1);
    }

    s = cartridge_get_file_name((uint16_t)0);

    if (s == NULL || *s == '\0') {
        return "(none)";
    } else {
        return s;
    }
}

static tui_menu_item_def_t detach_cartridge_menu_items[] = {
    { "--" },
    { "_Cartridge:",
      "Detach attached cartridge image",
      detach_cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(freeze_cartridge_callback)
{
    if (been_activated) {
        keyboard_clear_keymatrix();
        cartridge_trigger_freeze();
    }
    /* This way, the "Not Really!" item is always the default one.  */
    *become_default = 0;

    return NULL;
}

static tui_menu_item_def_t freeze_cartridge_menu_items[] = {
    { "Cartridge _Freeze",
      "Activates the cartridge's freeze button",
      freeze_cartridge_callback, NULL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static int cart_count(cartridge_info_t *cartlist, int group)
{
    int count = 0;

    while (cartlist->name) {
        if (cartlist->flags & group) {
            ++count;
        }
        ++cartlist;
    }
    return count;
}

static void cart_menu_set(cartridge_info_t *cartlist, int group, tui_menu_item_def_t *submenu_items, int items_offset, int max_items)
{
    int i = 0;
    int done = 0;

    while (cartlist->name && i < max_items) {
        if (cartlist->flags & group) {
            if (done >= items_offset) {
                submenu_items[i].label = cartlist->name;
                submenu_items[i].help_string = cartlist->name;
                submenu_items[i].callback = attach_cartridge_callback;
                submenu_items[i].callback_param = (void *)cartlist->crtid;
                submenu_items[i].par_string_max_len = 0;
                submenu_items[i].behavior = TUI_MENU_BEH_CLOSE;
                submenu_items[i].submenu = NULL;
                submenu_items[i].submenu_title = NULL;
                ++i;
            }
            ++done;
        }
        ++cartlist;
    }
    submenu_items[i].label = NULL;
    submenu_items[i].help_string = NULL;
    submenu_items[i].callback = NULL;
    submenu_items[i].callback_param = NULL;
    submenu_items[i].par_string_max_len = 0;
    submenu_items[i].behavior = 0;
    submenu_items[i].submenu = NULL;
    submenu_items[i].submenu_title = NULL;
}

static void uicart_menu_create(void)
{
    int num;
    cartridge_info_t *cartlist = cartridge_get_info_list();

    num = cart_count(cartlist, CARTRIDGE_GROUP_GENERIC);
    attach_generic_cartridge_submenu_items = lib_malloc(sizeof(tui_menu_item_def_t) * (num + 1));
    cart_menu_set(cartlist, CARTRIDGE_GROUP_GENERIC, attach_generic_cartridge_submenu_items, 0, num);
    
    num = cart_count(cartlist, CARTRIDGE_GROUP_RAMEX);
    attach_ramex_cartridge_submenu_items = lib_malloc(sizeof(tui_menu_item_def_t) * (num + 1));
    cart_menu_set(cartlist, CARTRIDGE_GROUP_RAMEX, attach_ramex_cartridge_submenu_items, 0, num);

    num = cart_count(cartlist, CARTRIDGE_GROUP_FREEZER);
    attach_freezer_cartridge_submenu_items = lib_malloc(sizeof(tui_menu_item_def_t) * (num + 1));
    cart_menu_set(cartlist, CARTRIDGE_GROUP_FREEZER, attach_freezer_cartridge_submenu_items, 0, num);

    num = cart_count(cartlist, CARTRIDGE_GROUP_GAME);
    attach_game_cartridge_submenu_items = lib_malloc(sizeof(tui_menu_item_def_t) * (num + 1));
    cart_menu_set(cartlist, CARTRIDGE_GROUP_GAME, attach_game_cartridge_submenu_items, 0, num);

    num = cart_count(cartlist, CARTRIDGE_GROUP_UTIL);
    attach_util1_cartridge_submenu_items = lib_malloc(sizeof(tui_menu_item_def_t) * (num + 1));
    cart_menu_set(cartlist, CARTRIDGE_GROUP_UTIL, attach_util1_cartridge_submenu_items, 0, num / 2);

    attach_util2_cartridge_submenu_items = lib_malloc(sizeof(tui_menu_item_def_t) * (num + 1));
    cart_menu_set(cartlist, CARTRIDGE_GROUP_UTIL, attach_util2_cartridge_submenu_items, num / 2, num - (num / 2));

    attach_c64_cartridge_menu_items[2].submenu = attach_generic_cartridge_submenu_items;
    attach_scpu64_cartridge_menu_items[2].submenu = attach_generic_cartridge_submenu_items;

    attach_c64_cartridge_menu_items[3].submenu = attach_ramex_cartridge_submenu_items;
    attach_scpu64_cartridge_menu_items[3].submenu = attach_ramex_cartridge_submenu_items;

    attach_c64_cartridge_menu_items[4].submenu = attach_freezer_cartridge_submenu_items;

    attach_c64_cartridge_menu_items[5].submenu = attach_game_cartridge_submenu_items;
    attach_scpu64_cartridge_menu_items[4].submenu = attach_game_cartridge_submenu_items;

    attach_c64_cartridge_menu_items[6].submenu = attach_util1_cartridge_submenu_items;
    attach_scpu64_cartridge_menu_items[5].submenu = attach_util1_cartridge_submenu_items;

    attach_c64_cartridge_menu_items[7].submenu = attach_util2_cartridge_submenu_items;
    attach_scpu64_cartridge_menu_items[6].submenu = attach_util2_cartridge_submenu_items;
}

void uic64cart_init(struct tui_menu *parent_submenu)
{
    uicart_menu_create();
    tui_menu_add(ui_detach_submenu, detach_cartridge_menu_items);
    if (machine_class != VICE_MACHINE_SCPU64) {
        tui_menu_add(ui_attach_submenu, attach_c64_cartridge_menu_items);
        tui_menu_add(ui_reset_submenu, freeze_cartridge_menu_items);
    } else {
        tui_menu_add(ui_attach_submenu, attach_scpu64_cartridge_menu_items);
    }
}
