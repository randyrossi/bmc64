/** \file   joy-unix.c
 * \brief   Linux/BSD joystick support
 *
 * \author  Bernhard Kuhn <kuhn@eikon.e-technik.tu-muenchen.de>
 * \author  Ulmer Lionel <ulmer@poly.polytechnique.fr>
 * \author  Marco van den Heuvel <blackystardust68@yahoo.com>
 * \author  Daniel Sladic <sladic@eecg.toronto.edu>
 * \author  Krister Walfridsson <cato@df.lth.se>
 * \author  Luca Montecchiani  <m.luca@usa.net> (http://i.am/m.luca)
 */

/*
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

#if defined(UNIX_COMPILE) && !defined(MACOSX_SUPPORT)

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "cmdline.h"
#include "joy.h"
#include "joyport.h"
#include "joystick.h"
#include "keyboard.h"
#include "log.h"
#include "resources.h"
#include "types.h"

int joy_arch_set_device(int port, int new_dev)
{
    if (new_dev < 0 || new_dev > JOYDEV_MAX) {
        return -1;
    }

    return 0;
}

/* Resources.  */

int joy_arch_resources_init(void)
{
    return 0;
}

/* Command-line options.  */

static const cmdline_option_t joydev1cmdline_options[] =
{
    { "-joydev1", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice1", NULL,
#ifdef HAS_JOYSTICK
#  ifdef HAS_USB_JOYSTICK
      "<0-13>", "Set device for joystick port 1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1, 12: USB joystick 0, 13: USB joystick 1)" },
#  else
#    ifdef HAS_DIGITAL_JOYSTICK
      "<0-11>", "Set device for joystick port 1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1)" },
#    else
      "<0-9>", "Set device for joystick port 1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5)" },
#    endif
#  endif
#else
      "<0-3>", "Set device for joystick port 1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2)" },
#endif
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev2cmdline_options[] =
{
    { "-joydev2", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice2", NULL,
#ifdef HAS_JOYSTICK
#  ifdef HAS_USB_JOYSTICK
      "<0-13>", "Set device for joystick port 2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1, 12: USB joystick 0, 13: USB joystick 1)" },
#  else
#    ifdef HAS_DIGITAL_JOYSTICK
      "<0-11>", "Set device for joystick port 2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1)" },
#    else
      "<0-9>", "Set device for joystick port 2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5)" },
#    endif
#  endif
#else
      "<0-3>", "Set device for joystick port 2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2)" },
#endif
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev3cmdline_options[] =
{
    { "-extrajoydev1", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice3", NULL,
#ifdef HAS_JOYSTICK
#  ifdef HAS_USB_JOYSTICK
      "<0-13>", "Set device for extra joystick port 1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1, 12: USB joystick 0, 13: USB joystick 1)" },
#  else
#    ifdef HAS_DIGITAL_JOYSTICK
      "<0-11>", "Set device for extra joystick port 1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1)" },
#    else
      "<0-9>", "Set device for extra joystick port 1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5)" },
#    endif
#  endif
#else
      "<0-3>", "Set device for extra joystick port 1 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2)" },
#endif
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev4cmdline_options[] =
{
    { "-extrajoydev2", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice4", NULL,
#ifdef HAS_JOYSTICK
#  ifdef HAS_USB_JOYSTICK
      "<0-13>", "Set device for extra joystick port 2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1, 12: USB joystick 0, 13: USB joystick 1)" },
#  else
#    ifdef HAS_DIGITAL_JOYSTICK
      "<0-11>", "Set device for extra joystick port 2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1)" },
#    else
      "<0-9>", "Set device for extra joystick port 2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5)" },
#    endif
#  endif
#else
      "<0-3>", "Set device for extra joystick port 2 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2)" },
#endif
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev5cmdline_options[] =
{
    { "-extrajoydev3", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice5", NULL,
#ifdef HAS_JOYSTICK
#  ifdef HAS_USB_JOYSTICK
      "<0-13>", "Set device for extra joystick port 3 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1, 12: USB joystick 0, 13: USB joystick 1)" },
#  else
#    ifdef HAS_DIGITAL_JOYSTICK
      "<0-11>", "Set device for extra joystick port 3 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5, 10: Digital joystick 0, 11: Digital joystick 1)" },
#    else
      "<0-9>", "Set device for extra joystick port 3 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2, 4: Analog joystick 0, 5: Analog joystick 1, 6: Analog joystick 2, 7: Analog joystick 3, 8: Analog joystick 4, 9: Analog joystick 5)" },
#    endif
#  endif
#else
      "<0-3>", "Set device for extra joystick port 3 (0: None, 1: Numpad, 2: Keyset 1, 3: Keyset 2)" },
#endif
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

#  ifdef HAS_JOYSTICK
#    ifdef LINUX_JOYSTICK
#      include <linux/joystick.h>

/* Compile time New 1.1.xx API presence check */
#      ifdef JS_VERSION
#        include <sys/ioctl.h>
#        include <errno.h>
#        define NEW_JOYSTICK 1
#        undef HAS_DIGITAL_JOYSTICK
static int use_old_api=0;
#      else
static int use_old_api=1;
#      endif

#    elif defined(BSD_JOYSTICK)
#      ifdef HAVE_MACHINE_JOYSTICK_H
#        include <machine/joystick.h>
#      endif
#      ifdef HAVE_SYS_JOYSTICK_H
#        include <sys/joystick.h>
#      endif
#      define JS_DATA_TYPE joystick
#      define JS_RETURN sizeof(struct joystick)
int use_old_api=1;
#    else
#      error Unknown Joystick
#    endif

#    define ANALOG_JOY_NUM (JOYDEV_ANALOG_5-JOYDEV_ANALOG_0+1)

/* file handles for the joystick device files */

static int ajoyfd[ANALOG_JOY_NUM] = { -1, -1, -1, -1, -1, -1 };
static int djoyfd[2] = { -1, -1 };

#    define JOYCALLOOPS 100
#    define JOYSENSITIVITY 5
static int joyxcal[2];
static int joyycal[2];
static int joyxmin[2];
static int joyxmax[2];
static int joyymin[2];
static int joyymax[2];

log_t joystick_log = LOG_ERR;

/* ------------------------------------------------------------------------- */

/**********************************************************
 * Generic high level joy routine                         *
 **********************************************************/
int joy_arch_init(void)
{
    if (use_old_api) {
        old_joystick_init();
    } else {
        new_joystick_init();
    }
#    ifdef HAS_USB_JOYSTICK
    usb_joystick_init();
#    endif
    return 0;
}

void joystick_close(void)
{
    if (use_old_api) {
        old_joystick_close();
    } else {
        new_joystick_close();
    }
#    ifdef HAS_USB_JOYSTICK
    usb_joystick_close();
#    endif
}

void joystick(void)
{
    if (use_old_api) {
        old_joystick();
    } else {
        new_joystick();
    }
#    ifdef HAS_USB_JOYSTICK
    usb_joystick();
#    endif
}

/** \brief  Struct containing device name and id
 */
typedef struct device_info_s {
    const char *name;   /**< device name */
    int         id;     /**< device ID (\see joy.h) */
} device_info_t;

static device_info_t predefined_device_list[] = {
#ifdef HAS_JOYSTICK
    { "Analog joystick 0",  JOYDEV_ANALOG_0 },
    { "Analog joystick 1",  JOYDEV_ANALOG_1 },
    { "Analog joystick 2",  JOYDEV_ANALOG_2 },
    { "Analog joystick 3",  JOYDEV_ANALOG_3 },
    { "Analog joystick 4",  JOYDEV_ANALOG_4 },
    { "Analog joystick 5",  JOYDEV_ANALOG_5 },
#endif
#ifdef HAS_DIGITAL_JOYSTICK
    { "Digital joystick 0", JOYDEV_DIGITAL_0 },
    { "Digital joystick 1", JOYDEV_DIGITAL_1 },
#endif
#ifdef HAS_USB_JOYSTICK
    { "USB joystick 0",     JOYDEV_USB_0 },
    { "USB joystick 1",     JOYDEV_USB_1 },
#endif
    { NULL, -1 }
};

static int joystickdeviceidx = 0;

void joystick_ui_reset_device_list(void)
{
    joystickdeviceidx = 0;
}

/* FIXME: proper device names will be returned only when using the "new" API
          and only for "analog" joysticks. */
const char *joystick_ui_get_next_device_name(int *id)
{
    const char *name;
    static char jname[0x80];
    int idx;

    /* printf("joystick_ui_get_next_device_name  id: %d\n", joystickdeviceidx); */

    if ((name = predefined_device_list[joystickdeviceidx].name)) {
        *id = predefined_device_list[joystickdeviceidx].id;
        joystickdeviceidx++;

        if (!use_old_api) {
#ifdef HAS_JOYSTICK
#if !defined(__FreeBSD__) && !defined(__OpenBSD__) && !defined(__NetBSD__) && !defined (__DragonflyBSD__)
            if ((*id >= JOYDEV_ANALOG_0) && (*id <= JOYDEV_ANALOG_5)) {
                idx = *id - JOYDEV_ANALOG_0;
                if (ajoyfd[idx] >= 0) {
                    sprintf(jname, "%u: ", idx);
                    ioctl(ajoyfd[idx], JSIOCGNAME (sizeof (jname) - 4), &jname[3]);
                    *id = idx + JOYDEV_ANALOG_0;
                    /* printf("joystick_ui_get_next_device_name  got name: %d: %s: %s\n", *id, name, jname); */
                    return jname;
                } else {
                    /* no joystick at this port */
                    return NULL;
                }
            }
#endif /* BSD */
#endif
        }
        /* return name from the predefined list instead */
        return name;
    }
    return NULL;
}

/**********************************************************
 * Older Joystick routine 0.8x Linux/BSD driver           *
 **********************************************************/
void old_joystick_init(void)
{
    int i;

    joystick_log = log_open("Joystick");

    /* close all device files */
    for (i = 0; i < 2; i++) {
        if (ajoyfd[i] != -1) {
            close(ajoyfd[i]);
        }
        if (djoyfd[i] != -1) {
            close(djoyfd[i]);
        }
    }

    /* open analog device files */
    for (i = 0; i < 2; i++) {

        const char *dev;
#    ifdef LINUX_JOYSTICK
        dev = (i == 0) ? "/dev/js0" : "/dev/js1";
#    elif defined(BSD_JOYSTICK)
        dev = (i == 0) ? "/dev/joy0" : "/dev/joy1";
#    endif

        ajoyfd[i] = open(dev, O_RDONLY);
        if (ajoyfd[i] < 0) {
            log_warning(joystick_log, "Cannot open joystick device `%s'.", dev);
        } else {
            int j;

            /* calibration loop */
            for (j = 0; j < JOYCALLOOPS; j++) {
                struct JS_DATA_TYPE js;
                int status = read(ajoyfd[i], &js, JS_RETURN);

                if (status != JS_RETURN) {
                    log_warning(joystick_log, "Error reading joystick device `%s'.", dev);
                } else {
                    /* determine average */
                    joyxcal[i] += js.x;
                    joyycal[i] += js.y;
                }
            }

            /* correct average */
            joyxcal[i] /= JOYCALLOOPS;
            joyycal[i] /= JOYCALLOOPS;

            /* determine treshoulds */
            joyxmin[i] = joyxcal[i] - joyxcal[i] / JOYSENSITIVITY;
            joyxmax[i] = joyxcal[i] + joyxcal[i] / JOYSENSITIVITY;
            joyymin[i] = joyycal[i] - joyycal[i] / JOYSENSITIVITY;
            joyymax[i] = joyycal[i] + joyycal[i] / JOYSENSITIVITY;

            log_message(joystick_log, "Hardware joystick calibration for device `%s':", dev);
            log_message(joystick_log, "  X: min: %i , mid: %i , max: %i.", joyxmin[i], joyxcal[i], joyxmax[i]);
            log_message(joystick_log, "  Y: min: %i , mid: %i , max: %i.", joyymin[i], joyycal[i], joyymax[i]);
        }
    }

#    ifdef HAS_DIGITAL_JOYSTICK
    /* open device files for digital joystick */
    for (i = 0; i < 2; i++) {
        const char *dev;
        dev = (i == 0) ? "/dev/djs0" : "/dev/djs1";

        djoyfd[i] = open(dev, O_RDONLY);
        if (djoyfd[i] < 0) {
            log_message(joystick_log, "Cannot open joystick device `%s'.", dev);
        }
    }
#    endif
}

void old_joystick_close(void)
{
    if (ajoyfd[0] > 0) {
        close(ajoyfd[0]);
    }
    if (ajoyfd[1] > 0) {
        close(ajoyfd[1]);
    }
    if (djoyfd[0] > 0) {
        close(djoyfd[0]);
    }
    if (djoyfd[1] > 0) {
        close(djoyfd[1]);
    }
}

void old_joystick(void)
{
    int i;

    for (i = 1; i <= 5; i++) {
        int joyport = joystick_port_map[i - 1];

#    ifdef HAS_DIGITAL_JOYSTICK
        if (joyport == JOYDEV_DIGITAL_0 || joyport == JOYDEV_DIGITAL_1) {
            int status;
            struct DJS_DATA_TYPE djs;
            int djoyport = joyport - JOYDEV_DIGITAL_0;

            if (djoyfd[djoyport] > 0) {
                status = read(djoyfd[djoyport], &djs, DJS_RETURN);
                if (status != DJS_RETURN) {
                    log_error(joystick_log, "Error reading digital joystick device.");
                } else {
                    BYTE newval;

                    newval = ((joystick_get_value_absolute & 0xe0) | ((~(djs.switches >> 3)) & 0x1f));
                    joystick_set_value_absolute(i, newval);
                }
            }
        } else
#    endif
        if (joyport == JOYDEV_ANALOG_0 || joyport == JOYDEV_ANALOG_1) {
            int status;
            struct JS_DATA_TYPE js;
            int ajoyport = joyport - JOYDEV_ANALOG_0;

            if (ajoyfd[ajoyport] > 0) {
                status = read(ajoyfd[ajoyport], &js, JS_RETURN);
                if (status != JS_RETURN) {
                    log_error(joystick_log, "Error reading joystick device.");
                } else {
                    joystick_set_value_absolute(i, 0);

                    if (js.y < joyymin[ajoyport]) {
                        joystick_set_value_or(i, 1);
                    }
                    if (js.y > joyymax[ajoyport]) {
                        joystick_set_value_or(i, 2);
                    }
                    if (js.x < joyxmin[ajoyport]) {
                        joystick_set_value_or(i, 4);
                    }
                    if (js.x > joyxmax[ajoyport]) {
                        joystick_set_value_or(i, 8);
                    }
#    ifdef LINUX_JOYSTICK
                    if (js.buttons) {
                        joystick_set_value_or(i, 16);
                    }
#    elif defined(BSD_JOYSTICK)
                    if (js.b1 || js.b2) {
                        joystick_set_value_or(i, 16);
                    }
#    endif
                }
            }
        }
    }
}

#    ifndef NEW_JOYSTICK
void new_joystick_init(void)
{
}

void new_joystick_close(void)
{
}

void new_joystick(void)
{
}
#    else /* NEW_JOYSTICK */
void new_joystick_init(void)
{
    int i;
    int ver = 0;
    int axes, buttons;
    char name[60];
    struct JS_DATA_TYPE js;

    const char *joydevs[ANALOG_JOY_NUM][2] = {
        { "/dev/js0", "/dev/input/js0" },
        { "/dev/js1", "/dev/input/js1" },
        { "/dev/js2", "/dev/input/js2" },
        { "/dev/js3", "/dev/input/js3" },
        { "/dev/js4", "/dev/input/js4" },
        { "/dev/js5", "/dev/input/js5" }
    };

    if (joystick_log == LOG_ERR) {
        joystick_log = log_open("Joystick");
    }

    log_message(joystick_log, "Linux joystick interface initialization...");
    /* close all device files */
    for (i = 0; i < ANALOG_JOY_NUM; i++) {
        if (ajoyfd[i] != -1) {
            close (ajoyfd[i]);
        }
    }

    /* open analog device files */

    for (i = 0; i < ANALOG_JOY_NUM; i++) {
        const char *dev;
        int j;
        for (j = 0; j < 2; j++) {
            dev = joydevs[i][j];
            ajoyfd[i] = open(dev, O_RDONLY);
            if (ajoyfd[i] >= 0) {
                break;
            }
        }

        if (ajoyfd[i] >= 0) {
            if (read (ajoyfd[i], &js, sizeof(struct JS_DATA_TYPE)) < 0) {
                close (ajoyfd[i]);
                ajoyfd[i] = -1;
                continue;
            }
            if (ioctl(ajoyfd[i], JSIOCGVERSION, &ver)) {
                log_message(joystick_log, "%s unknown type", dev);
                log_message(joystick_log, "Built in driver version: %d.%d.%d", JS_VERSION >> 16, (JS_VERSION >> 8) & 0xff, JS_VERSION & 0xff);
                log_message(joystick_log, "Kernel driver version  : 0.8 ??");
                log_message(joystick_log, "Please update your Joystick driver!");
                log_message(joystick_log, "Fall back to old api routine");
                use_old_api = 1;
                old_joystick_init();
                return;
            }
            ioctl(ajoyfd[i], JSIOCGVERSION, &ver);
            ioctl(ajoyfd[i], JSIOCGAXES, &axes);
            ioctl(ajoyfd[i], JSIOCGBUTTONS, &buttons);
            ioctl(ajoyfd[i], JSIOCGNAME (sizeof (name)), name);
            log_message(joystick_log, "%s is %s", dev, name);
            log_message(joystick_log, "Built in driver version: %d.%d.%d", JS_VERSION >> 16, (JS_VERSION >> 8) & 0xff, JS_VERSION & 0xff);
            log_message(joystick_log, "Kernel driver version  : %d.%d.%d", ver >> 16, (ver >> 8) & 0xff, ver & 0xff);
            fcntl(ajoyfd[i], F_SETFL, O_NONBLOCK);
        } else {
            log_warning(joystick_log, "Cannot open joystick device `%s'.", dev);
        }
    }
}

void new_joystick_close(void)
{
    int i;

    for (i=0; i<ANALOG_JOY_NUM; ++i) {
        if (ajoyfd[i] > 0) {
            close (ajoyfd[i]);
        }
    }
}

void new_joystick(void)
{
    int i;
    struct js_event e;
    int ajoyport;

    for (i = 1; i <= 5; i++) {
        int joyport = joystick_port_map[i - 1];

        if ((joyport < JOYDEV_ANALOG_0) || (joyport > JOYDEV_ANALOG_5)) {
            continue;
        }

        ajoyport = joyport - JOYDEV_ANALOG_0;

        if (ajoyfd[ajoyport] < 0) {
            continue;
        }

        /* Read all queued events. */
        while (read(ajoyfd[ajoyport], &e, sizeof(struct js_event)) == sizeof(struct js_event)) {
            switch (e.type & ~JS_EVENT_INIT) {
            case JS_EVENT_BUTTON:
                /* Generally, only the first few buttons are "fire" on a modern
                   joystick, the others being reserved for more esoteric things
                   like "SELECT", "START", "PAUSE", and directional movement.
                   The following treats only the first four buttons on a joystick
                   as fire buttons and ignores the rest.
                */
                if (! (e.number & ~3)) { /* only first four buttons are fire */
                    joystick_set_value_and(i, ~16); /* reset fire bit */
                    if (e.value) {
                        joystick_set_value_or(i, 16);
                    }
                }
                break;
            case JS_EVENT_AXIS:
                if (e.number == 0) {
                    joystick_set_value_and(i, 19); /* reset 2 bit */
                    if (e.value > 16384) {
                        joystick_set_value_or(i, 8);
                    } else if (e.value < -16384) {
                        joystick_set_value_or(i, 4);
                    }
                }
                if (e.number == 1) {
                    joystick_set_value_and(i, 28); /* reset 2 bit */
                    if (e.value > 16384) {
                        joystick_set_value_or(i, 2);
                    } else if (e.value < -16384) {
                        joystick_set_value_or(i, 1);
                    }
                }
                break;
            }
        }
    }
}
#    endif  /* NEW_JOYSTICK */
#  else /* HAS_JOYSTICK */
int joy_arch_init(void)
{
    return 0;
}

void joystick_close(void)
{
    /* NOP */
}

#  endif /* HAS_JOYSTICK */

#endif
