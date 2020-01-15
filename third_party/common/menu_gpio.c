/*
 * menu_gpio.c
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
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

#include "menu_gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// RASPI includes
#include "circle.h"
#include "menu.h"
#include "ui.h"

static int pins[NUM_GPIO_PINS] = { 4, 17, 27, 22, 5,
                                   6, 13, 19, 26, 18,
                                   23, 24, 25, 8, 7,
                                   12, 16, 20, 21 };
// Button function and port (if applicable)
static int menu_items_list[NUM_GPIO_BINDINGS][2] = {
    { BTN_ASSIGN_UNDEF, 0 },
    { BTN_ASSIGN_MENU, 0 },
    { BTN_ASSIGN_WARP, 0 },
    { BTN_ASSIGN_STATUS_TOGGLE, 0 },
    { BTN_ASSIGN_SWAP_PORTS, 0 },
    { BTN_ASSIGN_UP, 1 },
    { BTN_ASSIGN_UP, 2 },
    { BTN_ASSIGN_DOWN, 1 },
    { BTN_ASSIGN_DOWN, 2 },
    { BTN_ASSIGN_LEFT, 1 },
    { BTN_ASSIGN_LEFT, 2 },
    { BTN_ASSIGN_RIGHT, 1 },
    { BTN_ASSIGN_RIGHT, 2 },
    { BTN_ASSIGN_FIRE, 1 },
    { BTN_ASSIGN_FIRE, 2 },
    { BTN_ASSIGN_POTX, 1 },
    { BTN_ASSIGN_POTX, 2 },
    { BTN_ASSIGN_POTY, 1 },
    { BTN_ASSIGN_POTY, 2 },
    { BTN_ASSIGN_TAPE_MENU, 0 },
    { BTN_ASSIGN_CART_MENU, 0 },
    { BTN_ASSIGN_CART_FREEZE, 0 },
    { BTN_ASSIGN_RESET_HARD, 0 },
    { BTN_ASSIGN_RESET_SOFT, 0 },
    { BTN_ASSIGN_RUN_STOP_BACK, 0 },
    { BTN_ASSIGN_CUSTOM_KEY_1, 0 },
    { BTN_ASSIGN_CUSTOM_KEY_2, 0 },
    { BTN_ASSIGN_CUSTOM_KEY_3, 0 },
    { BTN_ASSIGN_CUSTOM_KEY_4, 0 },
    { BTN_ASSIGN_CUSTOM_KEY_5, 0 },
    { BTN_ASSIGN_CUSTOM_KEY_6, 0 },
    { BTN_ASSIGN_ACTIVE_DISPLAY, 0 },
    { BTN_ASSIGN_PIP_LOCATION, 0 },
    { BTN_ASSIGN_PIP_SWAP, 0 },
    { BTN_ASSIGN_40_80_COLUMN, 0 },
    { BTN_ASSIGN_VKBD_TOGGLE, 0 },
};

static struct menu_item* binding_item[NUM_GPIO_PINS];

static void menu_value_changed(struct menu_item *item) {
   int pin_index = item->id;
   gpio_bindings[pin_index] = item->choice_ints[item->value];
}

void build_gpio_menu(struct menu_item *root) {
   for (int i=0;i<NUM_GPIO_PINS;i++) {
     binding_item[i] = ui_menu_add_multiple_choice(i, root, "");
     binding_item[i]->num_choices = NUM_GPIO_BINDINGS;
     sprintf (binding_item[i]->name, "GPIO%02d Binding", pins[i]);

     for (int j = 0; j < NUM_GPIO_BINDINGS; j++) {
        // Lower = func, Upper = port arg
        int func = menu_items_list[j][0];
        int port = menu_items_list[j][1];
        int binding_value = func | (port << 8);
        strcpy(binding_item[i]->choices[j], function_to_string(func));

        if (port != 0) {
           char tmp[16];
           sprintf (tmp, " (Port %d)", port);
           strcat (binding_item[i]->choices[j], tmp);
        }

        binding_item[i]->choice_ints[j] = binding_value;

        if (gpio_bindings[i] == binding_value) {
           binding_item[i]->value = j;
        }
     }
     binding_item[i]->on_value_changed = menu_value_changed;
   }
}
