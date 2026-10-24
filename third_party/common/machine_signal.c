/*
 * machine_signal.c
 *
 * BMC64 - A bare metal Commodore emulator for Raspberry Pi
 * Copyright (C) 2026 aminch <https://github.com/aminch>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "machine_signal.h"

// RASPI includes
#include "circle.h"
#include "emux_api.h"

// Machine codes. These are fixed so other projects can rely on them: add
// new ones, never change existing ones. 0 is never sent, and 7 is also what
// the pins read as before BMC64 sets them up (inputs with pull-ups).
#define MACHINE_CODE_C64 1
#define MACHINE_CODE_C128 2
#define MACHINE_CODE_PET_BUSINESS 3
#define MACHINE_CODE_PET_GRAPHICS 4
#define MACHINE_CODE_VIC20 5
#define MACHINE_CODE_PLUS4 6
#define MACHINE_CODE_UNKNOWN 7

// VICE's KBD_TYPE_GRAPHICS_US; every other PET keyboard type is a Business
// one.
#define PET_KEYBOARD_GRAPHICS_US 4

#define LED_RED 1   // GPIO 9
#define LED_GREEN 2 // GPIO 10
#define LED_BLUE 4  // GPIO 11
#define LED_ALL (LED_RED | LED_GREEN | LED_BLUE)

static int machine_code(void) {
  switch (emux_machine_class) {
  case BMC64_MACHINE_CLASS_C64:
    return MACHINE_CODE_C64;
  case BMC64_MACHINE_CLASS_C128:
    return MACHINE_CODE_C128;
  case BMC64_MACHINE_CLASS_VIC20:
    return MACHINE_CODE_VIC20;
  case BMC64_MACHINE_CLASS_PLUS4:
  case BMC64_MACHINE_CLASS_PLUS4EMU:
    return MACHINE_CODE_PLUS4;
  case BMC64_MACHINE_CLASS_PET: {
    int keyboard_type = 0;
    emux_get_int(Setting_KeyboardType, &keyboard_type);
    return keyboard_type == PET_KEYBOARD_GRAPHICS_US
               ? MACHINE_CODE_PET_GRAPHICS
               : MACHINE_CODE_PET_BUSINESS;
  }
  default:
    return MACHINE_CODE_UNKNOWN;
  }
}

static int led_colour(int code) {
  switch (code) {
  case MACHINE_CODE_C64:
    return LED_RED;
  case MACHINE_CODE_C128:
    return LED_BLUE;
  case MACHINE_CODE_VIC20:
    return LED_GREEN;
  case MACHINE_CODE_PLUS4:
    return LED_GREEN | LED_BLUE;
  case MACHINE_CODE_PET_GRAPHICS:
    return LED_RED | LED_GREEN;
  case MACHINE_CODE_PET_BUSINESS:
    return LED_RED | LED_BLUE;
  default:
    return 0;
  }
}

void machine_signal_update(void) {
  switch (circle_gpio_machine_signal()) {
  case GPIO_MACHINE_SIGNAL_CODE:
    circle_set_machine_signal(machine_code());
    break;
  case GPIO_MACHINE_SIGNAL_LED_HIGH:
    circle_set_machine_signal(led_colour(machine_code()));
    break;
  case GPIO_MACHINE_SIGNAL_LED_LOW:
    circle_set_machine_signal(led_colour(machine_code()) ^ LED_ALL);
    break;
  default:
    // Off: the pins are left alone.
    break;
  }
}

int machine_signal_reserved(void) {
  return circle_gpio_machine_signal() != GPIO_MACHINE_SIGNAL_OFF;
}
