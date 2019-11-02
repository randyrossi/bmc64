/*
 * vice_api.c - VICE specific impl of emux_api.h
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

#include "emux_api.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// VICE includes
#include "attach.h"
#include "cartridge.h"
#include "interrupt.h"
#include "machine.h"
#include "videoarch.h"
#include "menu.h"
#include "menu_timing.h"
#include "ui.h"
#include "keyboard.h"
#include "demo.h"

// RASPI includes
#include "circle.h"

static void pause_trap(uint16_t addr, void *data) {
  menu_about_to_activate();
  circle_show_fbl(FB_LAYER_UI);
  while (ui_enabled) {
    circle_check_gpio();
    ui_check_key();

    ui_handle_toggle_or_quick_func();

    ui_render_single_frame();
    hdmi_timing_hook();
    ensure_video();
  }
  menu_about_to_deactivate();
  circle_hide_fbl(FB_LAYER_UI);
}

void emu_machine_init(void) {
  switch (machine_class) {
    case VICE_MACHINE_C64:
       emux_machine_class = BMC64_MACHINE_CLASS_C64;
       break;
    case VICE_MACHINE_C128:
       emux_machine_class = BMC64_MACHINE_CLASS_C128;
       break;
    case VICE_MACHINE_VIC20:
       emux_machine_class = BMC64_MACHINE_CLASS_VIC20;
       break;
    case VICE_MACHINE_PLUS4:
       emux_machine_class = BMC64_MACHINE_CLASS_PLUS4;
       break;
    default:
       assert(0);
       break;
  }
}

void emux_trap_main_loop_ui(void) {
  interrupt_maincpu_trigger_trap(pause_trap, 0);
}

void emux_trap_main_loop(void (*trap_func)(uint16_t, void *data), void* data) {
  interrupt_maincpu_trigger_trap(trap_func, data);
}

void emux_kbd_set_latch_keyarr(int row, int col, int value) {
  demo_reset_timeout();
  keyboard_set_keyarr(row, col, value);
}

int emux_attach_disk_image(int unit, char* filename) {
  return file_system_attach_disk(unit, filename);
}

void emux_detach_disk(int unit) {
  file_system_detach_disk(unit);
}

int emux_attach_cart(int bank, char* filename) {
  // Ignore bank for vice
  return cartridge_attach_image(CARTRIDGE_CRT, filename);
}

void emux_detach_cart(int bank) {
  // Ignore bank for vice
  cartridge_detach_image(CARTRIDGE_NONE);
}

void emux_reset(int soft) {
  machine_trigger_reset(soft ? 
      MACHINE_RESET_MODE_SOFT : MACHINE_RESET_MODE_HARD);
}

int emux_save_state(char *filename) {
  return machine_write_snapshot(filename, 1, 1, 0);
}

int emux_load_state(char *filename) {
  return machine_read_snapshot(filename, 0);
}
