/*
 * menu_tape_osd.c
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

#include "menu_tape_osd.h"

#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// RASPI includes
#include "emux_api.h"
#include "menu.h"
#include "ui.h"

static void menu_item_changed(struct menu_item *item) {
  switch (item->id) {
  case MENU_TAPE_START:
    emux_tape_control(EMUX_TAPE_PLAY);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_STOP:
    emux_tape_control(EMUX_TAPE_STOP);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_REWIND:
    emux_tape_control(EMUX_TAPE_REWIND);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_FASTFWD:
    emux_tape_control(EMUX_TAPE_FASTFORWARD);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_RECORD:
    emux_tape_control(EMUX_TAPE_RECORD);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_RESET:
    emux_tape_control(EMUX_TAPE_RESET);
    ui_pop_all_and_toggle();
    return;
  case MENU_TAPE_RESET_COUNTER:
    emux_tape_control(EMUX_TAPE_ZERO);
    ui_pop_all_and_toggle();
    return;
  default:
    break;
  }
}

void show_tape_osd_menu(void) {
  // We only show OSD when the emulator is running. (not in the trap)
  if (ui_enabled) {
    ui_dismiss_osd_if_active();
    return;
  }
  struct menu_item *root = ui_push_menu(7, 7);
  root->on_popped_off = glob_osd_popped;

  struct menu_item *child;
  child = ui_menu_add_button(MENU_TAPE_START, root, "PLAY");
  child->on_value_changed = menu_item_changed;
  child = ui_menu_add_button(MENU_TAPE_STOP, root, "STOP");
  child->on_value_changed = menu_item_changed;
  child = ui_menu_add_button(MENU_TAPE_REWIND, root, "REW");
  child->on_value_changed = menu_item_changed;
  child = ui_menu_add_button(MENU_TAPE_FASTFWD, root, "FF");
  child->on_value_changed = menu_item_changed;
  child = ui_menu_add_button(MENU_TAPE_RECORD, root, "REC");
  child->on_value_changed = menu_item_changed;
  child = ui_menu_add_button(MENU_TAPE_RESET, root, "RESET");
  child->on_value_changed = menu_item_changed;
  child = ui_menu_add_button(MENU_TAPE_RESET_COUNTER, root, "ZERO");
  child->on_value_changed = menu_item_changed;

  // This will turn on ui rendering from the emuation side which will
  // now see the OSD we just created.
  ui_enable_osd();
}
