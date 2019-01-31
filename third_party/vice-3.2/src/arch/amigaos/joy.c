/*
 * joy.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#ifdef AMIGA_OS4
#include "cmdline.h"
#include "resources.h"

#include "lib.h"
#include "joy.h"
#include "joyai.h"
#include "joyport.h"
#include "joystick.h"
#include "keyboard.h"
#include "types.h"
#include "ui.h"
#include "intl.h"
#include "translate.h"

int joystick_inited = 0;

int joy_arch_init(void)
{
    if (joystick_inited == 0) {
        joystick_inited = 1;

        joyai_open();
    }

    return 0;
}

void joystick_close(void)
{
    joyai_close();

    joystick_inited = 0;
}

int joy_arch_set_device(int port_idx, int joy_dev)
{
    switch (joy_dev) {
        case JOYDEV_NONE:
        case JOYDEV_NUMPAD:
        case JOYDEV_KEYSET1:
        case JOYDEV_KEYSET2:
            break;
        default:
            return -1;
    }
    joy_arch_init();

    return 0;
}

int joy_arch_resources_init(void)
{
    return joyai_init_resources();
}


/* ------------------------------------------------------------------------- */

static const cmdline_option_t joydev1cmdline_options[] = {
    { "-joydev1", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice1", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NUMBER, IDS_SET_INPUT_JOYSTICK_1,
      NULL, NULL },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev2cmdline_options[] = {
    { "-joydev2", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice2", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NUMBER, IDS_SET_INPUT_JOYSTICK_2,
      NULL, NULL },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev3cmdline_options[] = {
    { "-extrajoydev1", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice3", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NUMBER, IDS_SET_INPUT_EXTRA_JOYSTICK_1,
      NULL, NULL },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev4cmdline_options[] = {
    { "-extrajoydev2", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice4", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NUMBER, IDS_SET_INPUT_EXTRA_JOYSTICK_2,
      NULL, NULL },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev5cmdline_options[] = {
    { "-extrajoydev3", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice5", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NUMBER, IDS_SET_INPUT_EXTRA_JOYSTICK_3,
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
    int joy_dev, port_idx;
    uint8_t value = 0;

    for (port_idx = 0; port_idx < JOYSTICK_NUM; port_idx++) {
        joy_dev = joystick_port_map[port_idx];

        /* The numpad case is handled specially because it allows users to
           use both `5' and `2' for "down".  */
        if (joy_dev == JOYDEV_NUMPAD) {
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

        if (joy_dev == JOYDEV_KEYSET1) {
            value |= joyai_key(1, port_idx + 1, kcode, pressed);
        }
        if (joy_dev == JOYDEV_KEYSET2) {
            value |= joyai_key(2, port_idx + 1, kcode, pressed);
        }
    }

    return value;
}

/* Update hardware joystick parts of the keysets!  */
int joystick_update(void)
{
    int joy_dev, port_idx;

    for (port_idx = 0; port_idx < JOYSTICK_NUM; port_idx++) {
        joy_dev = joystick_port_map[port_idx];
        if (joy_dev == JOYDEV_KEYSET1) {
            joyai_update(1, port_idx + 1);
        }
        if (joy_dev == JOYDEV_KEYSET2) {
            joyai_update(2, port_idx + 1);
        }
    }

    return 0;
}
#endif
