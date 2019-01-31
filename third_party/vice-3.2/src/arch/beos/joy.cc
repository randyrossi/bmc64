/*
 * joy.cc - Joystick support for BeOS
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
 *  Marcus Sutton <loggedoubt@gmail.com>
 *
 * FIXME: Due to some problems with Be's BJoystick implementation
 * the current implementation is quite ugly and should be rewritten
 * in the future. For now it works...
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

#include <Joystick.h>
#include <stdlib.h>
#include <string.h>

extern "C" {
#include "vice.h"

#include "cmdline.h"
#include "joy.h"
#include "joyport.h"
#include "joystick.h"
#include "keyboard.h"
#include "log.h"
#include "resources.h"
#include "translate.h"
#include "types.h"
#include "ui.h"
#include "util.h"
}

/* objects to access hardware devices */
static BJoystick *bjoystick[4];

/* stick index in the open BJoystick devices */ 
static int stick_nr[4];

/* axes-pair index in the open BJoystick devices */ 
static int axes_nr[4];

/* to allocate buffers for the current axes values */
static int16 *axes[4];

/* to avoid problems opening a BJoystick device twice, we have to remember them */
static int device_used_by[MAX_HARDWARE_JOYSTICK];

/* to check if the joystick module is already initialized */
static int joystick_initialized = 0;

/* array that holds all recognized hardware sticks */
hardware_joystick_t hardware_joystick[MAX_HARDWARE_JOYSTICK];
int hardware_joystick_count = 0;

static void joystick_close_device(int port_idx)
{
    int device_num;
    int used_by;
    int joy_dev = joystick_port_map[port_idx];
    int hardware_dev;

    if (joy_dev < NUM_OF_SOFTDEVICES) {
        return;
    }

    hardware_dev = joy_dev - NUM_OF_SOFTDEVICES;
    if (hardware_dev < hardware_joystick_count) {
        /* it's a hardware-stick; close the device if necessary */
        device_num = hardware_joystick[hardware_dev].device_num;
        used_by = device_used_by[device_num];
        device_used_by[device_num] &= ~(1 << port_idx);
        if (!device_used_by[device_num] && used_by) {
            bjoystick[used_by - 1]->Close();
            delete bjoystick[used_by - 1];
            free (axes[used_by - 1]);
            log_message(LOG_DEFAULT, "Joystick: Closed hardware %s as device %d",
            hardware_joystick[hardware_dev].device_name, port_idx + 1);
        }
    }
}

static void joystick_open_device(int port_idx, int joy_dev)
{
    int used_by;
    int hardware_dev;

    if (joy_dev < NUM_OF_SOFTDEVICES) {
        return;
    }

    hardware_dev = joy_dev - NUM_OF_SOFTDEVICES;
    if (hardware_dev < hardware_joystick_count) {
        /* is the needed device already open? */
        int device_num = hardware_joystick[hardware_dev].device_num;
            if (used_by = device_used_by[device_num]) {
                bjoystick[port_idx] = bjoystick[used_by - 1];
                axes[port_idx] = axes[used_by - 1];
                log_message(LOG_DEFAULT, "Joystick: Device %d uses hardware port already opened by device %d", port_idx + 1, used_by);
            } else {
                bjoystick[port_idx] = new BJoystick();
                if (bjoystick[port_idx]->Open(hardware_joystick[hardware_dev].device_name, true) == B_ERROR) {
                    log_message(LOG_DEFAULT, "Joystick: Warning, couldn't open hardware device %d", port_idx + 1);
                    return;
                } else {
                    log_message(LOG_DEFAULT, "Joystick: Opened hardware %s as device %d", hardware_joystick[hardware_dev].device_name, port_idx + 1);
            }
            axes[port_idx] = (int16*) malloc(sizeof(int16) * bjoystick[port_idx]->CountAxes());
        }
        device_used_by[device_num] |= (1 << port_idx);

        stick_nr[port_idx] = hardware_joystick[hardware_dev].stick; 
        axes_nr[port_idx] = hardware_joystick[hardware_dev].axes;
    }
}

int joy_arch_set_device(int port_idx, int new_dev)
{
    if (new_dev < 0 || new_dev > NUM_OF_SOFTDEVICES + MAX_HARDWARE_JOYSTICK) {
        return -1;
    }

    if (joystick_initialized) {
        joystick_close_device(port_idx);
        joystick_open_device(port_idx, new_dev);
    }

    return 0;
}

int joy_arch_resources_init(void)
{
    return 0;
}

/* ------------------------------------------------------------------------- */

static const cmdline_option_t joydev1cmdline_options[] = {
    { "-joydev1", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice1", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<number>", "Set input device for joystick #1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4..19: Hardware joysticks)" },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev2cmdline_options[] = {
    { "-joydev2", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice2", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<number>", "Set input device for joystick #2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4..19: Hardware joysticks)" },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev3cmdline_options[] = {
    { "-extrajoydev1", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice3", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<number>", "Set input device for extra joystick #1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4..19: Hardware joysticks)" },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev4cmdline_options[] = {
    { "-extrajoydev2", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice4", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<number>", "Set input device for extra joystick #2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4..19: Hardware joysticks)" },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev5cmdline_options[] = {
    { "-extrajoydev3", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice5", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<number>", "Set input device for extra joystick #3 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4..19: Hardware joysticks)" },
    CMDLINE_LIST_END
};

int joy_arch_cmdline_options_init(void)
{
    if (joyport_get_port_name(JOYPORT_1)) {
        if (cmdline_register_options(joydev1cmdline_options) < 0) {
            return -1;
        }
    }
    if (joyport_get_port_name(JOYPORT_2)) {
        if (cmdline_register_options(joydev2cmdline_options) < 0) {
            return -1;
        }
    }
    if (joyport_get_port_name(JOYPORT_3)) {
        if (cmdline_register_options(joydev3cmdline_options) < 0) {
            return -1;
        }
    }
    if (joyport_get_port_name(JOYPORT_4)) {
        if (cmdline_register_options(joydev4cmdline_options) < 0) {
            return -1;
        }
    }
    if (joyport_get_port_name(JOYPORT_5)) {
        if (cmdline_register_options(joydev5cmdline_options) < 0) {
            return -1;
        }
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

int joy_arch_init(void)
{
    BJoystick testbjoystick;
    char current_devicename[B_OS_NAME_LENGTH];
    int device_iter, stick_iter, axes_iter;
    int device_count;

    if (joystick_initialized) {
        return 0;
    }

    device_count = testbjoystick.CountDevices();
    for (device_iter = 0; device_iter < device_count; device_iter++) {
        device_used_by[device_iter] = 0;
        testbjoystick.GetDeviceName(device_iter, current_devicename);

        if (testbjoystick.Open(current_devicename, true) != B_ERROR) {
            for (stick_iter = 0; stick_iter < testbjoystick.CountSticks(); stick_iter++) {
                for (axes_iter = 0; axes_iter * 2 < testbjoystick.CountAxes(); axes_iter++) {
                    strcpy(hardware_joystick[hardware_joystick_count].device_name, current_devicename);
                    hardware_joystick[hardware_joystick_count].device_num = device_iter;
                    hardware_joystick[hardware_joystick_count].stick = stick_iter;
                    hardware_joystick[hardware_joystick_count++].axes = axes_iter;

                    if (hardware_joystick_count >= MAX_HARDWARE_JOYSTICK) {
                        return 0;
                    }
                }
            }
            testbjoystick.Close();
        }
    }
    joystick_initialized = 1;
    joystick_open_device(0, joystick_port_map[0]);
    joystick_open_device(1, joystick_port_map[1]);
    joystick_open_device(2, joystick_port_map[2]);
    joystick_open_device(3, joystick_port_map[3]);
    joystick_open_device(4, joystick_port_map[4]);

    return 0;
}

int joystick_close(void)
{
    int i;

    for (i = 0; i < 4; i++) {
        if (device_used_by[i]) {
            delete bjoystick[device_used_by[i] >> 1];
            free(axes[device_used_by[i]>>1]);
        }
    }
    return 0;
}


void joystick_update(void)
{
    int value;
    int port_idx;
    int joy_dev;
    uint32 buttons;
    BJoystick *last_joy = NULL;

    for (port_idx = 0; port_idx < JOYSTICK_NUM; port_idx++) {
        value = 0;
        joy_dev = joystick_port_map[port_idx];
 
        if (joy_dev >= NUM_OF_SOFTDEVICES && joy_dev < NUM_OF_SOFTDEVICES+hardware_joystick_count) {
            if (!last_joy) {
                if (bjoystick[port_idx]->Update() == B_ERROR) {
                    log_error(LOG_DEFAULT,"Joystick: Warning. Couldn't get Joystick value for device %d", port_idx + 1);
                    break;
                } else {
                    bjoystick[port_idx]->GetAxisValues(axes[port_idx], stick_nr[port_idx]);
                    buttons = bjoystick[port_idx]->ButtonValues(stick_nr[port_idx]);
                }
                last_joy = bjoystick[port_idx];
            }

            value = 0;
            if (axes[port_idx][2 * axes_nr[port_idx]] < JOYBORDER_MINX) {
                value |= 4;
            }
            if (axes[port_idx][2 * axes_nr[port_idx]] > JOYBORDER_MAXX) {
                value |= 8;
            }
            if (axes[port_idx][2 * axes_nr[port_idx] + 1] < JOYBORDER_MINY) {
                value |= 1;
            }
            if (axes[port_idx][2 * axes_nr[port_idx] + 1] > JOYBORDER_MAXY) {
                value |= 2;
            }
            if (buttons & (1 << axes_nr[port_idx] * 2) || buttons & (1 << axes_nr[port_idx]* 2 + 1)) {
                value |= 16;
            }
            joystick_set_value_absolute(port_idx + 1, value);
        }
    }
}

/* FIXME: src/joystick.c has code which handles keysets and also numpad.
   The numpad code here functions slightly differently and overrides the
   common code. The two should be merged. */
int joystick_handle_key(kbd_code_t kcode, int pressed)
{
    int joy_dev, port_idx;
    uint8_t value = 0;

    for (port_idx = 0; port_idx < JOYSTICK_NUM; port_idx++) {
        joy_dev = joystick_port_map[port_idx];

        /* The numpad case is handled specially because it allows users to
           use both `5' and `2' for "down".  */
        if (joy_dev == JOYDEV_NUMPAD) {
            switch (kcode) {
                case K_KP7:            /* North-West */
                    value = 5;
                    break;
                case K_KP8:            /* North */
                    value = 1;
                    break;
                case K_KP9:            /* North-East */
                    value = 9;
                    break;
                case K_KP6:            /* East */
                    value = 8;
                    break;
                case K_KP3:            /* South-East */
                    value = 10;
                    break;
                case K_KP2:            /* South */
                case K_KP5:
                    value = 2;
                    break;
                case K_KP1:            /* South-West */
                    value = 6;
                    break;
                case K_KP4:            /* West */
                    value = 4;
                    break;
                case K_KP0:
                case K_RIGHTCTRL:
                    value = 16;
                    break;
                default:
                    /* (make compiler happy) */
                    break;
            }

            if (pressed) {
                joystick_set_value_or(port_idx + 1, value);
            } else {
                joystick_set_value_and(port_idx + 1, (uint8_t) ~value);
            }
        }
    }

    return value;
}
