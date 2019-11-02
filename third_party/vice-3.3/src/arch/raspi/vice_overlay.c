/*
 * vice_overlay.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// VICE includes
#include "datasette.h"
#include "uiapi.h"

// RASPI includes
#include "emux_api.h"

// Called by VICE to enable a drive status lights
void ui_enable_drive_status(ui_drive_enable_t state, int *drive_led_color) {
  int st = state;
  emux_enable_drive_status(st, drive_led_color);
}

// Called by VICE to show drive led
void ui_display_drive_led(int drive, unsigned int pwm1, unsigned int pwm2) {
  emux_display_drive_led(drive, pwm1, pwm2);
}

// Called by VICE to show tape counter text
void ui_display_tape_counter(int counter) {
  emux_display_tape_counter(counter);
}

// Called by VICE to draw tape motor status light
void ui_display_tape_motor_status(int motor) {
  emux_display_tape_motor_status(motor);
}

// Called by VICE to show tape control status
void ui_display_tape_control_status(int control) {
  int xlated_control;

  switch(control) {
    case DATASETTE_CONTROL_START:
      xlated_control = EMUX_TAPE_PLAY;
      break;
    case DATASETTE_CONTROL_STOP:
      xlated_control = EMUX_TAPE_STOP;
      break;
    case DATASETTE_CONTROL_REWIND:
      xlated_control = EMUX_TAPE_REWIND;
      break;
    case DATASETTE_CONTROL_FORWARD:
      xlated_control = EMUX_TAPE_FASTFORWARD;
      break;
    case DATASETTE_CONTROL_RECORD:
      xlated_control = EMUX_TAPE_RECORD;
      break;
    case DATASETTE_CONTROL_RESET:
      xlated_control = EMUX_TAPE_RESET;
      break;
    case DATASETTE_CONTROL_RESET_COUNTER:
      xlated_control = EMUX_TAPE_ZERO;
      break;
    default:
      xlated_control = EMUX_TAPE_STOP;
      break;
  }

  emux_display_tape_control_status(xlated_control);
}
