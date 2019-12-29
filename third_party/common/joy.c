/*
 * joy.c
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

#include "joy.h"

#include <stdio.h>
#include <string.h>

// RASPI includes
#include "emux_api.h"
#include "kbd.h"
#include "menu.h"
#include "menu_keyset.h"
#include "ui.h"
#include "overlay.h"

// Global usb gamepad info.
int joy_num_pads;
int joy_num_buttons[MAX_USB_DEVICES];
int joy_num_axes[MAX_USB_DEVICES];
int joy_num_hats[MAX_USB_DEVICES];

// Holds previous button states for the purpose of
// reporting button functions press/release events.
unsigned joy_prev_buttons[MAX_USB_DEVICES];

struct joydev_config joydevs[MAX_JOY_PORTS];

// Called from ISR
// Return non-zero if the event was handled.
int joy_key_up(unsigned int device, int key) {
  if (emu_is_ui_activated()) {
    // When the ui is showing, we want these
    // to turn into navigation for the ui.
    if (joydevs[device].device == JOYDEV_NUMS_1) {
      switch (key) {
      case KEYCODE_KP8:
        emu_ui_key_interrupt(KEYCODE_Up, 0 /* up */);
        return 1;
      case KEYCODE_KP2:
        emu_ui_key_interrupt(KEYCODE_Down, 0 /* up */);
        return 1;
      case KEYCODE_KP4:
        emu_ui_key_interrupt(KEYCODE_Left, 0 /* up */);
        return 1;
      case KEYCODE_KP6:
        emu_ui_key_interrupt(KEYCODE_Right, 0 /* up */);
        return 1;
      case KEYCODE_KP5:
        emu_ui_key_interrupt(KEYCODE_Return, 0 /* up */);
        return 1;
      default:
        return 0;
      }
    } else if (joydevs[device].device == JOYDEV_NUMS_2) {
      switch (key) {
      case KEYCODE_KP9:
        emu_ui_key_interrupt(KEYCODE_Up, 0 /* up */);
        return 1;
      case KEYCODE_KP3:
        emu_ui_key_interrupt(KEYCODE_Down, 0 /* up */);
        return 1;
      case KEYCODE_KP7:
        emu_ui_key_interrupt(KEYCODE_Left, 0 /* up */);
        return 1;
      case KEYCODE_KP1:
        emu_ui_key_interrupt(KEYCODE_Right, 0 /* up */);
        return 1;
      case KEYCODE_KP0:
        emu_ui_key_interrupt(KEYCODE_Return, 0 /* up */);
        return 1;
      default:
        return 0;
      }
    }
    return 0;
  }

  int port = joydevs[device].port;
  int devd = joydevs[device].device;
  if (devd == JOYDEV_NUMS_1) {
    switch (key) {
    case KEYCODE_KP8:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x01);
      return 1;
    case KEYCODE_KP2:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x02);
      return 1;
    case KEYCODE_KP4:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x04);
      return 1;
    case KEYCODE_KP6:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x08);
      return 1;
    case KEYCODE_KP5:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x10);
      return 1;
    default:
      return 0;
    }
  } else if (devd == JOYDEV_NUMS_2) {
    switch (key) {
    case KEYCODE_KP9:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x01);
      return 1;
    case KEYCODE_KP3:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x02);
      return 1;
    case KEYCODE_KP7:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x04);
      return 1;
    case KEYCODE_KP1:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x08);
      return 1;
    case KEYCODE_KP0:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x10);
      return 1;
    default:
      return 0;
    }
  } else if (devd == JOYDEV_CURS_SP ||
             devd == JOYDEV_CURS_LC) {
    switch (key) {
    case KEYCODE_Up:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x01);
      return 1;
    case KEYCODE_Down:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x02);
      return 1;
    case KEYCODE_Left:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x04);
      return 1;
    case KEYCODE_Right:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x08);
      return 1;
    case KEYCODE_Space:
      if (devd == JOYDEV_CURS_SP) {
        emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x10);
        return 1;
      }
      break;
    case KEYCODE_LeftControl:
      if (devd == JOYDEV_CURS_LC) {
        emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x10);
        return 1;
      }
      break;
    default:
      return 0;
    }
  } else if (devd == JOYDEV_KEYSET1) {
    if (keyset_codes[0][KEYSET_UP] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x01);
      return 1;
    } else if (keyset_codes[0][KEYSET_DOWN] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x02);
      return 1;
    } else if (keyset_codes[0][KEYSET_LEFT] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x04);
      return 1;
    } else if (keyset_codes[0][KEYSET_RIGHT] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x08);
      return 1;
    } else if (keyset_codes[0][KEYSET_FIRE] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x10);
      return 1;
    } else if (keyset_codes[0][KEYSET_POTX] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~(pot_x_low_value << 5));
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, pot_x_high_value << 5);
      return 1;
    } else if (keyset_codes[0][KEYSET_POTY] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~(pot_y_low_value << 13));
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, pot_y_high_value << 13);
      return 1;
    }
    return 0;
  } else if (devd == JOYDEV_KEYSET2) {
    if (keyset_codes[1][KEYSET_UP] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x01);
      return 1;
    } else if (keyset_codes[1][KEYSET_DOWN] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x02);
      return 1;
    } else if (keyset_codes[1][KEYSET_LEFT] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x04);
      return 1;
    } else if (keyset_codes[1][KEYSET_RIGHT] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x08);
      return 1;
    } else if (keyset_codes[1][KEYSET_FIRE] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~0x10);
      return 1;
    } else if (keyset_codes[1][KEYSET_POTX] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~(pot_x_low_value << 5));
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, pot_x_high_value << 5);
      return 1;
    } else if (keyset_codes[1][KEYSET_POTY] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~(pot_y_low_value << 13));
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, pot_y_high_value << 13);
      return 1;
    }
  }
  return 0;
}

// Called from ISR
// Return non-zero if the event was handled.
int joy_key_down(unsigned int device, int key) {
  if (emu_is_ui_activated()) {
    if (joydevs[device].device == JOYDEV_NUMS_1) {
      switch (key) {
      case KEYCODE_KP8:
        emu_ui_key_interrupt(KEYCODE_Up, 1 /* down */);
        return 1;
      case KEYCODE_KP2:
        emu_ui_key_interrupt(KEYCODE_Down, 1 /* down */);
        return 1;
      case KEYCODE_KP4:
        emu_ui_key_interrupt(KEYCODE_Left, 1 /* down */);
        return 1;
      case KEYCODE_KP6:
        emu_ui_key_interrupt(KEYCODE_Right, 1 /* down */);
        return 1;
      case KEYCODE_KP5:
        emu_ui_key_interrupt(KEYCODE_Return, 1 /* down */);
        return 1;
      default:
        return 0;
      }
    } else if (joydevs[device].device == JOYDEV_NUMS_2) {
      switch (key) {
      case KEYCODE_KP9:
        emu_ui_key_interrupt(KEYCODE_Up, 1 /* down */);
        return 1;
      case KEYCODE_KP3:
        emu_ui_key_interrupt(KEYCODE_Down, 1 /* down */);
        return 1;
      case KEYCODE_KP7:
        emu_ui_key_interrupt(KEYCODE_Left, 1 /* down */);
        return 1;
      case KEYCODE_KP1:
        emu_ui_key_interrupt(KEYCODE_Right, 1 /* down */);
        return 1;
      case KEYCODE_KP0:
        emu_ui_key_interrupt(KEYCODE_Return, 1 /* down */);
        return 1;
      default:
        return 0;
      }
    } 
    return 0;
  }

  int port = joydevs[device].port;
  int devd = joydevs[device].device;
  if (devd == JOYDEV_NUMS_1) {
    switch (key) {
    case KEYCODE_KP8:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x01);
      return 1;
    case KEYCODE_KP2:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x02);
      return 1;
    case KEYCODE_KP4:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x04);
      return 1;
    case KEYCODE_KP6:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x08);
      return 1;
    case KEYCODE_KP5:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x10);
      return 1;
    default:
      return 0;
    }
  } else if (devd == JOYDEV_NUMS_2) {
    switch (key) {
    case KEYCODE_KP9:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x01);
      return 1;
    case KEYCODE_KP3:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x02);
      return 1;
    case KEYCODE_KP7:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x04);
      return 1;
    case KEYCODE_KP1:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x08);
      return 1;
    case KEYCODE_KP0:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x10);
      return 1;
    default:
      return 0;
    }
  } else if (devd == JOYDEV_CURS_SP ||
             devd == JOYDEV_CURS_LC) {
    switch (key) {
    case KEYCODE_Up:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x01);
      return 1;
    case KEYCODE_Down:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x02);
      return 1;
    case KEYCODE_Left:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x04);
      return 1;
    case KEYCODE_Right:
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x08);
      return 1;
    case KEYCODE_Space:
      if (devd == JOYDEV_CURS_SP) {
        emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x10);
        return 1;
      }
      break;
    case KEYCODE_LeftControl:
      if (devd == JOYDEV_CURS_LC) {
        emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x10);
        return 1;
      }
      break;
    default:
      return 0;
    }
  } else if (devd == JOYDEV_KEYSET1) {
    if (keyset_codes[0][KEYSET_UP] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x01);
      return 1;
    } else if (keyset_codes[0][KEYSET_DOWN] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x02);
      return 1;
    } else if (keyset_codes[0][KEYSET_LEFT] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x04);
      return 1;
    } else if (keyset_codes[0][KEYSET_RIGHT] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x08);
      return 1;
    } else if (keyset_codes[0][KEYSET_FIRE] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x10);
      return 1;
    } else if (keyset_codes[0][KEYSET_POTX] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~(pot_x_high_value << 5));
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, pot_x_low_value << 5);
      return 1;
    } else if (keyset_codes[0][KEYSET_POTY] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~(pot_y_high_value << 13));
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, pot_y_low_value << 13);
      return 1;
    }
    return 0;
  } else if (devd == JOYDEV_KEYSET2) {
    if (keyset_codes[1][KEYSET_UP] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x01);
      return 1;
    } else if (keyset_codes[1][KEYSET_DOWN] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x02);
      return 1;
    } else if (keyset_codes[1][KEYSET_LEFT] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x04);
      return 1;
    } else if (keyset_codes[1][KEYSET_RIGHT] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x08);
      return 1;
    } else if (keyset_codes[1][KEYSET_FIRE] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, 0x10);
      return 1;
    } else if (keyset_codes[1][KEYSET_POTX] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~(pot_x_high_value << 5));
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, pot_x_low_value << 5);
      return 1;
    } else if (keyset_codes[1][KEYSET_POTY] == key) {
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_AND, port, devd, ~(pot_y_high_value << 13));
      emux_joy_interrupt(PENDING_EMU_JOY_TYPE_OR, port, devd, pot_y_low_value << 13);
      return 1;
    }
  }
  return 0;
}

// NOTE: This is called from the ISR so we can't call into the
// emulator API directly from here. Instead, we have to queue up
// the event for processing on the main loop.
void emu_set_joy_usb_interrupt(unsigned int device, int value) {
  static int want_dev[MAX_USB_DEVICES] = {
     JOYDEV_USB_0, JOYDEV_USB_1, JOYDEV_USB_2, JOYDEV_USB_3,
  };

  // This convoluted loop figures out which joy port if any is
  // assigned to the usb device for which an event is arriving.
  for (int k = 0; k < MAX_USB_DEVICES; k++) {
    for (int j = 0; j < MAX_JOY_PORTS; j++) {
       if (device == k && joydevs[j].device == want_dev[k]) {
          emux_joy_interrupt(PENDING_EMU_JOY_TYPE_ABSOLUTE, joydevs[j].port,
                             want_dev[k], value);
          return;
       }
    }
  }

  if (vkbd_enabled) {
    // The port doesn't really matter here since this will not go to the
    // emulator. The value is ignored.
    emux_joy_interrupt(PENDING_EMU_JOY_TYPE_ABSOLUTE, 0,
                             want_dev[device], value);
  }
}

// Setup stuff
int joy_arch_init(void) { return 0; }

void emu_set_gamepad_info(int num_pads,
                          int num_buttons[MAX_USB_DEVICES],
                          int num_axes[MAX_USB_DEVICES],
                          int num_hats[MAX_USB_DEVICES]) {
  joy_num_pads = MIN(num_pads , MAX_USB_DEVICES);
  for (int k = 0 ; k < MAX_USB_DEVICES; k++) {
    joy_num_axes[k] = MIN(num_axes[k] , MAX_USB_AXES);
    joy_num_hats[k] = MIN(num_hats[k] , MAX_USB_HATS);
    joy_num_buttons[k] = MIN(num_buttons[k] , MAX_USB_BUTTONS);
  }
}

void joystick_close(void) {}
void joystick(void) {}
void old_joystick_init(void) {}
void old_joystick_close(void) {}
void old_joystick(void) {}
void new_joystick_init(void) {}
void new_joystick_close(void) {}
void new_joystick(void) {}

#ifdef HAS_USB_JOYSTICK
int usb_joystick_init(void) { return 0; }
void usb_joystick_close(void) {}
void usb_joystick(void) {}
#endif
