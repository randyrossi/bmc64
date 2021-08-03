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

// These are listed in the same order as the array in viceapp.cpp
// in the kernel layer. They must match.  TODO: Set these from kernel
// in an init method instead of this duplicated list.
int custom_gpio_pins[NUM_GPIO_PINS] = {
    5, 20, 19, 16, 13, 6, 12, 26, 8, 25, 24,
    18, 23, 27, 17, 22, 4, 7, 21, 2, 3, 9, 10 };

#define NUM_GPIO_BINDINGS 38

// Button function and bank (if applicable)
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
    { BTN_ASSIGN_RESET_MENU, 0 },
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
    { BTN_ASSIGN_FLUSH_DISK, 0 },
};

static void menu_value_changed(struct menu_item *item) {
   int pin_index = item->id;
   gpio_bindings[pin_index] = item->choice_ints[item->value];
}

void build_gpio_menu(struct menu_item *root) {
   struct menu_item* item;
   for (int i=0;i<NUM_GPIO_PINS;i++) {
     item = ui_menu_add_multiple_choice(i, root, "");
     item->num_choices = NUM_GPIO_BINDINGS;
     sprintf (item->name, "GPIO%02d Binding", custom_gpio_pins[i]);

     for (int j = 0; j < NUM_GPIO_BINDINGS; j++) {
        // Lower = func, Upper = bank arg
        unsigned int func = menu_items_list[j][0];
        unsigned int bank = menu_items_list[j][1];
        unsigned int binding_value = func | (bank << 8);
        strcpy(item->choices[j], function_to_string(func));

        if (bank > 0) {
           char tmp[16];
           sprintf (tmp, " (Bank %d)", bank);
           strcat (item->choices[j], tmp);
        }

        item->choice_ints[j] = binding_value;

        if (gpio_bindings[i] == binding_value) {
           item->value = j;
        }
     }
     item->on_value_changed = menu_value_changed;
   }
}
