/*
 * menu_confirm_osd.c
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

#include "menu_confirm_osd.h"

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <math.h>
#include "ui.h"
#include "menu.h"
#include "datasette.h"
#include "resources.h"

static void popped(struct menu_item* item) {
   osd_active = 0;
}

static void menu_item_changed(struct menu_item* item) {
  switch (item->id) {
      case MENU_CONFIRM_OK:
         // Pass along to main loop as quick func invoke
         circle_emu_quick_func_interrupt(item->value);
         ui_pop_all_and_toggle();
         return;
      case MENU_CONFIRM_CANCEL:
         ui_pop_all_and_toggle();
         return;
  }
}

void show_confirm_osd_menu(int func) {
  // We only show OSD when the emulator is running. (not in the trap)
  if (ui_activated) {
     if (osd_active) { ui_pop_all_and_toggle(); osd_active = 0;}
     return;
  }
  struct menu_item* root = ui_push_menu(8, 2);
  root->on_value_changed = popped;

  struct menu_item* child;
  child = ui_menu_add_button(MENU_CONFIRM_OK, root, "OK");
  child->on_value_changed = menu_item_changed;
  child->value = func;

  child = ui_menu_add_button(MENU_CONFIRM_CANCEL, root, "CANCEL");
  child->on_value_changed = menu_item_changed;

  // This will turn on ui rendering from the emuation side which will
  // now see the OSD we just created.
  ui_activated = 1;
  osd_active = 1;
}
