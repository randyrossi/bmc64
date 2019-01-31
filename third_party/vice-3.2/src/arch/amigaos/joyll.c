/*
 * joyll.c
 *
 * Written by
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

#ifndef AMIGA_OS4
#include <proto/lowlevel.h> /* include before WORD etc are #defined */

#include "cmdline.h"
#include "resources.h"

#include "lib.h"
#include "joyll.h"
#include "joyport.h"
#include "joystick.h"
#include "keyboard.h"
#include "loadlibs.h"
#include "types.h"
#include "ui.h"

#include "intl.h"
#include "translate.h"

int joystick_inited = 0;

static int joystick_fire[5];

int joy_arch_init(void)
{
    if (joystick_inited == 0) {
        joystick_inited = 1;
    }

    return 0;
}

void joystick_close(void)
{
    joystick_inited = 0;
}

int joy_arch_set_device(int port_idx, int joy_dev)
{
    ULONG portstate;

    switch (joy_dev) {
        case JOYDEV_NONE:
        case JOYDEV_NUMPAD:
        case JOYDEV_KEYSET1:
        case JOYDEV_KEYSET2:
        case JOYDEV_JOY0:
        case JOYDEV_JOY1:
        case JOYDEV_JOY2:
        case JOYDEV_JOY3:
            break;
        default:
            return -1;
    }

    joy_arch_init();

    if (joy_dev >= JOYDEV_JOY0) {
        portstate = ReadJoyPort(joy_dev - JOYDEV_JOY0);
        if ((portstate & JP_TYPE_MASK) == JP_TYPE_JOYSTK) {
            joystick_fire[port_idx] = JPF_BUTTON_RED;
        }
    }

    return 0;
}

static int set_joystick_fire(int value, void *param)
{
    ULONG portstate;
    int port_idx = vice_ptr_to_int(param);
    int joy_dev = joystick_port_map[port_idx];

    joy_arch_init();

    if (joy_dev != JOYDEV_NONE) {
        if (joy_dev >= JOYDEV_JOY0 && joy_dev <= JOYDEV_JOY3) {
            portstate = ReadJoyPort(joy_dev - JOYDEV_JOY0);
            if ((portstate & JP_TYPE_MASK) != JP_TYPE_GAMECTLR) {
                if (value != JPF_BUTTON_RED) {
                    ui_error(translate_text(IDMES_DEVICE_NOT_GAMEPAD));
                    value = JPF_BUTTON_RED;
                }
            }
        } else {
            ui_error(translate_text(IDMES_NOT_MAPPED_TO_AMIGA_PORT));
            return -1;
        }
    } else {
        if (value != JPF_BUTTON_RED) {
            ui_error(translate_text(IDMES_NOT_MAPPED_TO_AMIGA_PORT));
            return -1;
        }
    }
    joystick_fire[port_idx] = value;

    return 0;
}

static const resource_int_t joy1_resources_int[] = {
    { "JoyFire1", JPF_BUTTON_RED, RES_EVENT_NO, NULL,
      &joystick_fire[0], set_joystick_fire, (void *)0 },
    RESOURCE_INT_LIST_END
};

static const resource_int_t joy2_resources_int[] = {
    { "JoyFire2", JPF_BUTTON_RED, RES_EVENT_NO, NULL,
      &joystick_fire[1], set_joystick_fire, (void *)1 },
    RESOURCE_INT_LIST_END
};

static const resource_int_t joy3_resources_int[] = {
    { "JoyFire3", JPF_BUTTON_RED, RES_EVENT_NO, NULL,
      &joystick_fire[2], set_joystick_fire, (void *)2 },
    RESOURCE_INT_LIST_END
};

static const resource_int_t joy4_resources_int[] = {
    { "JoyFire4", JPF_BUTTON_RED, RES_EVENT_NO, NULL,
      &joystick_fire[3], set_joystick_fire, (void *)3 },
    RESOURCE_INT_LIST_END
};

static const resource_int_t joy5_resources_int[] = {
    { "JoyFire5", JPF_BUTTON_RED, RES_EVENT_NO, NULL,
      &joystick_fire[4], set_joystick_fire, (void *)4 },
    RESOURCE_INT_LIST_END
};

int joy_arch_resources_init(void)
{
    if (joyport_get_port_name(JOYPORT_1)) {
        if (resources_register_int(joy1_resources_int) < 0) {
            return -1;
        }
    }
    if (joyport_get_port_name(JOYPORT_2)) {
        if (resources_register_int(joy2_resources_int) < 0) {
            return -1;
        }
    }
    if (joyport_get_port_name(JOYPORT_3)) {
        if (resources_register_int(joy3_resources_int) < 0) {
            return -1;
        }
    }
    if (joyport_get_port_name(JOYPORT_4)) {
        if (resources_register_int(joy4_resources_int) < 0) {
            return -1;
        }
    }

    return 0;
}

/* ------------------------------------------------------------------------- */

static const cmdline_option_t joydev1cmdline_options[] = {
    { "-joydev1", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice1", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NUMBER, IDS_SET_INPUT_JOYLL_1,
      NULL, NULL },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev2cmdline_options[] = {
    { "-joydev2", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice2", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NUMBER, IDS_SET_INPUT_JOYLL_2,
      NULL, NULL },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev3cmdline_options[] = {
    { "-extrajoydev1", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice3", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NUMBER, IDS_SET_INPUT_EXTRA_JOYLL_1,
      NULL, NULL },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev4cmdline_options[] = {
    { "-extrajoydev2", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice4", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NUMBER, IDS_SET_INPUT_EXTRA_JOYLL_2,
      NULL, NULL },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev5cmdline_options[] = {
    { "-extrajoydev3", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice5", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NUMBER, IDS_SET_INPUT_EXTRA_JOYLL_3,
      NULL, NULL },
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

/* Joystick-through-keyboard.  */

int joystick_handle_key(unsigned long kcode, int pressed)
{
    int port_idx;
    uint8_t value = 0;

    for (port_idx = 0; port_idx < JOYSTICK_NUM; port_idx++) {

        /* The numpad case is handled specially because it allows users to
           use both `5' and `2' for "down".  */
        if (joystick_port_map[port_idx] == JOYDEV_NUMPAD) {
            switch (kcode) {
                case 61:               /* North-West */
                    value = 5;
                    break;
                case 62:               /* North */
                    value = 1;
                    break;
                case 63:               /* North-East */
                    value = 9;
                    break;
                case 47:               /* East */
                    value = 8;
                    break;
                case 31:               /* South-East */
                    value = 10;
                    break;
                case 30:               /* South */
                case 46:
                    value = 2;
                    break;
                case 29:               /* South-West */
                    value = 6;
                    break;
                case 45:               /* West */
                    value = 4;
                    break;
                case 15:
                case 99:
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

static void joyll_update(ULONG amiga_dev, int port_idx)
{
    ULONG portstate;
    uint8_t value = 0;

    if (!lowlevel_lib_loaded) {
        return;
    }

    portstate = ReadJoyPort(amiga_dev);

    if (portstate & JPF_JOY_UP) {
        value |= 1;
    }
    if (portstate & JPF_JOY_DOWN) {
        value |= 2;
    }
    if (portstate & JPF_JOY_LEFT) {
        value |= 4;
    }
    if (portstate & JPF_JOY_RIGHT) {
        value |= 8;
    }
    if (portstate & joystick_fire[port_idx]) {
        value |= 16;
    }

    joystick_set_value_absolute(port_idx + 1, value);
}

/* Update hardware joysticks.  */
int joystick_update(void)
{
    int joy_dev, port_idx;

    for (port_idx = 0; port_idx < JOYSTICK_NUM; port_idx++) {
        joy_dev = joystick_port_map[port_idx];
        if (joy_dev >= JOYDEV_JOY0 && joy_dev <= JOYDEV_JOY3) {
            joyll_update(joy_dev - JOYDEV_JOY0, port_idx);
        }
    }

    return 0;
}
#endif
