/*
 * joy.c - Joystick support for MS-DOS.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include "types.h"

#include <conio.h>
#include <stdio.h>

#include <allegro.h>

#include "cmdline.h"
#include "keyboard.h"
#include "joy.h"
#include "joyport.h"
#include "joystick.h"
#include "resources.h"
#include "translate.h"
#include "tui.h"

/* ------------------------------------------------------------------------- */

/* Joystick devices.  */
int joy_arch_set_device(int port_idx, int new_dev)
{
    int old_dev = joystick_port_map[port_idx];

    switch (new_dev) {
        case JOYDEV_NONE:
        case JOYDEV_NUMPAD:
        case JOYDEV_KEYSET1:
        case JOYDEV_KEYSET2:
        case JOYDEV_HW1:
        case JOYDEV_HW2:
            break;
        default:
            return -1;
    }

    if (new_dev == JOYDEV_NONE && old_dev != JOYDEV_NONE) {
        joystick_set_value_absolute(port_idx + 1, 0);
    }
    return 0;
}

static int joystick_hw_type;

static int set_joystick_hw_type(int val, void *param)
{
    if (joystick_hw_type != val) {
        int old_joystick_hw_type = joystick_hw_type;
        int old_num_joysticks = num_joysticks;

        joystick_hw_type = val;
        remove_joystick();
        if (joystick_hw_type != 0) {
            if (install_joystick(joystick_hw_type)) {
                tui_error("Initialization of joystick device failed");
                joystick_hw_type = 0;
            } else if (num_joysticks < 4 && old_num_joysticks >= 4) {
                if (joystick_port_map[0] == JOYDEV_HW2) {
                    joystick_set_value_absolute(1, 0);
                }
                if (joystick_port_map[1] == JOYDEV_HW2) {
                    joystick_set_value_absolute(2, 0);
                }
                if (joystick_port_map[2] == JOYDEV_HW2) {
                    joystick_set_value_absolute(3, 0);
                }
                if (joystick_port_map[3] == JOYDEV_HW2) {
                    joystick_set_value_absolute(4, 0);
                }
                if (joystick_port_map[4] == JOYDEV_HW2) {
                    joystick_set_value_absolute(5, 0);
                }
            }
        }
        if (joystick_hw_type == 0 && old_joystick_hw_type != 0) {
            if (joystick_port_map[0] == JOYDEV_HW1 || joystick_port_map[0] == JOYDEV_HW2) {
                joystick_set_value_absolute(1, 0);
            }
            if (joystick_port_map[1] == JOYDEV_HW1 || joystick_port_map[1] == JOYDEV_HW2) {
                joystick_set_value_absolute(2, 0);
            }
            if (joystick_port_map[2] == JOYDEV_HW1 || joystick_port_map[2] == JOYDEV_HW2) {
                joystick_set_value_absolute(3, 0);
            }
            if (joystick_port_map[3] == JOYDEV_HW1 || joystick_port_map[3] == JOYDEV_HW2) {
                joystick_set_value_absolute(4, 0);
            }
            if (joystick_port_map[4] == JOYDEV_HW1 || joystick_port_map[4] == JOYDEV_HW2) {
                joystick_set_value_absolute(5, 0);
            }
        }
    }
    
    return 0;
}

static const resource_int_t hwtype_resources_int[] = {
    { "HwJoyType", 0, RES_EVENT_NO, NULL,
      &joystick_hw_type, set_joystick_hw_type, NULL },
    RESOURCE_INT_LIST_END
};

int joy_arch_resources_init(void)
{
    return resources_register_int(hwtype_resources_int);
}

/* ------------------------------------------------------------------------- */

typedef struct joymodel_s {
    const char *name;
    unsigned int joymodel;
} joymodel_t;

static joymodel_t joyhwtypes[] = {
    { "auto", JOY_TYPE_AUTODETECT },
    { "none", JOY_TYPE_NONE },
    { "standard", JOY_TYPE_STANDARD },
    { "dual", JOY_TYPE_2PADS },
    { "4button", JOY_TYPE_4BUTTON },
    { "6button", JOY_TYPE_6BUTTON },
    { "8button", JOY_TYPE_8BUTTON },
    { "fspro", JOY_TYPE_FSPRO },
    { "wingex", JOY_TYPE_WINGEX },
    { "sidewinderag", JOY_TYPE_SIDEWINDER_AG },
    { "sidewinderpp", JOY_TYPE_SIDEWINDER_PP },
    { "sidewinder", JOY_TYPE_SIDEWINDER },
    { "gamepadpro", JOY_TYPE_GAMEPAD_PRO },
    { "grip4", JOY_TYPE_GRIP4 },
    { "grip", JOY_TYPE_GRIP },
    { "sneslpt1", JOY_TYPE_SNESPAD_LPT1 },
    { "sneslpt2", JOY_TYPE_SNESPAD_LPT2 },
    { "sneslpt3", JOY_TYPE_SNESPAD_LPT3 },
    { "psxlpt1", JOY_TYPE_PSXPAD_LPT1 },
    { "psxlpt2", JOY_TYPE_PSXPAD_LPT2 },
    { "psxlpt3", JOY_TYPE_PSXPAD_LPT3 },
    { "n64lpt1", JOY_TYPE_N64PAD_LPT1 },
    { "n64lpt2", JOY_TYPE_N64PAD_LPT2 },
    { "n64lpt3", JOY_TYPE_N64PAD_LPT3 },
    { "db9lpt1", JOY_TYPE_DB9_LPT1 },
    { "db9lpt2", JOY_TYPE_DB9_LPT2 },
    { "db9lpt3", JOY_TYPE_DB9_LPT3 },
    { "tgxlpt1", JOY_TYPE_TURBOGRAFX_LPT1 },
    { "tgxlpt2", JOY_TYPE_TURBOGRAFX_LPT2 },
    { "tgxlpt3", JOY_TYPE_TURBOGRAFX_LPT3 },
    { "wingwar", JOY_TYPE_WINGWARRIOR },
    { "ifsegaisa", JOY_TYPE_IFSEGA_ISA },
    { "ifsegapcifast", JOY_TYPE_IFSEGA_PCI_FAST },
    { "ifsegapci", JOY_TYPE_IFSEGA_PCI },
    { NULL, 0 }
};

#define JOY_TYPE_UNKNOWN -2

static int set_hw_joy_type(const char *param, void *extra_param)
{
    int joymodel = JOY_TYPE_UNKNOWN;
    int i = 0;

    if (!param) {
        return -1;
    }

    do {
        if (strcmp(joyhwtypes[i].name, param) == 0) {
            joymodel = joyhwtypes[i].joymodel;
        }
        i++;
    } while ((joymodel == JOY_TYPE_UNKNOWN) && (joyhwtypes[i].name != NULL));

    if (joymodel == JOY_TYPE_UNKNOWN) {
        return -1;
    }

    return resources_set_int("HwJoyType", joymodel);
}

static const cmdline_option_t joyhwtypecmdline_options[] = {
    { "-joyhwtype", CALL_FUNCTION, 1,
      set_hw_joy_type, NULL, NULL, NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<type>", "Set joystick hardware type (auto/none/standard/dual/4button/6button/8button/fspro/wingex/sidewinderag/sidewinderpp/sidewinder/gamepadpro/grip4/grip/sneslpt1/sneslpt2/sneslpt3/psxlpt1/psxlpt2/psxlpt3/n64lpt1/n64lpt2/n64lpt3/db9lpt1/db9lpt2/db9lpt3/tgxlpt1/tgxlpt2/tgxlpt3/wingwar/ifsegaisa/ifsegapcifast/ifsegapci)" },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev1cmdline_options[] = {
    { "-joydev1", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice1", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<number>", "Set input device for joystick #1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Joystick 1, 5: Joystick 2)" },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev2cmdline_options[] = {
    { "-joydev2", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice2", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<number>", "Set input device for joystick #2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Joystick 1, 5: Joystick 2)" },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev3cmdline_options[] = {
    { "-extrajoydev1", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice3", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<number>", "Set input device for extra joystick #1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Joystick 1, 5: Joystick 2)" },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev4cmdline_options[] = {
    { "-extrajoydev2", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice4", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<number>", "Set input device for extra joystick #2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Joystick 1, 5: Joystick 2)" },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev5cmdline_options[] = {
    { "-extrajoydev3", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice5", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<number>", "Set input device for extra joystick #3 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Joystick 1, 5: Joystick 2)" },
    CMDLINE_LIST_END
};

int joy_arch_cmdline_options_init(void)
{
    if (cmdline_register_options(joyhwtypecmdline_options) < 0) {
        return -1;
    }

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


/* Initialize joystick support.  */
int joy_arch_init(void)
{
    return 0;
}

/* Update the `joystick_value' variables according to the joystick status.  */
void joystick_update(void)
{
    if (num_joysticks == 0) {
        return;
    }

    poll_joystick();

    if (joystick_port_map[0] == JOYDEV_HW1 || joystick_port_map[1] == JOYDEV_HW1 || joystick_port_map[2] == JOYDEV_HW1 || joystick_port_map[3] == JOYDEV_HW1 || joystick_port_map[4] == JOYDEV_HW1) {
        int value = 0;

        if (joy_left) {
            value |= 4;
        }
        if (joy_right) {
            value |= 8;
        }
        if (joy_up) {
            value |= 1;
        }
        if (joy_down) {
            value |= 2;
        }
        if (joy_b1 || joy_b2) {
            value |= 16;
        }
        if (joystick_port_map[0] == JOYDEV_HW1) {
            joystick_set_value_absolute(1, value);
        }
        if (joystick_port_map[1] == JOYDEV_HW1) {
            joystick_set_value_absolute(2, value);
        }
        if (joystick_port_map[2] == JOYDEV_HW1) {
            joystick_set_value_absolute(3, value);
        }
        if (joystick_port_map[3] == JOYDEV_HW1) {
            joystick_set_value_absolute(4, value);
        }
        if (joystick_port_map[4] == JOYDEV_HW1) {
            joystick_set_value_absolute(5, value);
        }
    }

    if (num_joysticks >= 2 && (joystick_port_map[0] == JOYDEV_HW2 || joystick_port_map[1] == JOYDEV_HW2 || joystick_port_map[2] == JOYDEV_HW2 || joystick_port_map[3] == JOYDEV_HW2 || joystick_port_map[4] == JOYDEV_HW2)) {
        int value = 0;

        if (joy2_left) {
            value |= 4;
        }
        if (joy2_right) {
            value |= 8;
        }
        if (joy2_up) {
            value |= 1;
        }
        if (joy2_down) {
            value |= 2;
        }
        if (joy2_b1 || joy2_b2) {
            value |= 16;
        }
        if (joystick_port_map[0] == JOYDEV_HW2) {
            joystick_set_value_absolute(1, value);
        }
        if (joystick_port_map[1] == JOYDEV_HW2) {
            joystick_set_value_absolute(2, value);
        }
        if (joystick_port_map[2] == JOYDEV_HW2) {
            joystick_set_value_absolute(3, value);
        }
        if (joystick_port_map[3] == JOYDEV_HW2) {
            joystick_set_value_absolute(4, value);
        }
        if (joystick_port_map[4] == JOYDEV_HW2) {
            joystick_set_value_absolute(5, value);
        }
    }
}

void joystick_close(void)
{
    /* Nothing to do on MSDOS.  */
    return;
}


/* ------------------------------------------------------------------------- */
