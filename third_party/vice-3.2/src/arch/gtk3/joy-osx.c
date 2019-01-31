/** \file   joy-osx.c
 * \brief   Mac OS X joystick support
 *
 * \author  Christian Vogelgsang <chris@vogelgsang.org>
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

#define JOY_INTERNAL

#include "vice.h"

#ifdef MACOSX_SUPPORT

#include "cmdline.h"
#include "joy.h"
#include "joyport.h"
#include "joystick.h"
#include "keyboard.h"
#include "lib.h"
#include "log.h"
#include "resources.h"
#include "translate.h"
#include "types.h"
#include "util.h"

#ifdef HAS_JOYSTICK

/* ----- Static Data ------------------------------------------------------ */

static int joy_done_init = 0;

/* number of joyports and extra joyports */
int joy_num_ports;
int joy_num_extra_ports;

joy_hid_descriptor_t hid_a;
joy_hid_descriptor_t hid_b;

/* the driver holds up to two USB joystick definitions */
joystick_descriptor_t joy_a = { .hid = &hid_a };
joystick_descriptor_t joy_b = { .hid = &hid_b };

/* ----- VICE Resources --------------------------------------------------- */

static void setup_axis_mapping(joystick_descriptor_t *joy);
static void setup_button_mapping(joystick_descriptor_t *joy);
static void setup_auto_button_mapping(joystick_descriptor_t *joy);
static void setup_hat_switch_mapping(joystick_descriptor_t *joy);
static void setup_auto(void);

/* HID settings */

static int set_joy_a_device_name(const char *val,void *param)
{
    util_string_set(&joy_a.device_name, val);
    if (joy_done_init) {
        setup_auto();
    }
    return 0;
}

static int set_joy_a_x_axis_name(const char *val,void *param)
{
    util_string_set(&joy_a.axis[HID_X_AXIS].name, val);
    if (joy_done_init) {
        setup_axis_mapping(&joy_a);
    }
    return 0;
}

static int set_joy_a_y_axis_name(const char *val,void *param)
{
    util_string_set(&joy_a.axis[HID_Y_AXIS].name, val);
    if (joy_done_init) {
        setup_axis_mapping(&joy_a);
    }
    return 0;
}

static int set_joy_a_button_mapping(const char *val,void *param)
{
    util_string_set(&joy_a.button_mapping, val);
    if (joy_done_init) {
        setup_button_mapping(&joy_a);
    }
    return 0;
}

static int set_joy_a_auto_button_mapping(const char *val,void *param)
{
    util_string_set(&joy_a.auto_button_mapping, val);
    if (joy_done_init) {
        setup_auto_button_mapping(&joy_a);
    }
    return 0;
}

/* a threshold */

static int set_joy_a_x_threshold(int val, void *param)
{
    if (val < 0 || val > 100) {
        return -1;
    }

    if (joy_a.axis[HID_X_AXIS].threshold != val) {
        joy_a.axis[HID_X_AXIS].threshold = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_a);
        }
    }
    return 0;
}

static int set_joy_a_y_threshold(int val, void *param)
{
    if (val < 0 || val > 100) {
        return -1;
    }

    if (joy_a.axis[HID_Y_AXIS].threshold != val) {
        joy_a.axis[HID_Y_AXIS].threshold = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_a);
        }
    }
    return 0;
}

/* min */

static int set_joy_a_x_min(int val, void *param)
{
    if (joy_a.axis[HID_X_AXIS].min != val) {
        joy_a.axis[HID_X_AXIS].min = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_a);
        }
    }
    return 0;
}

static int set_joy_a_y_min(int val, void *param)
{
    if (joy_a.axis[HID_Y_AXIS].min != val) {
        joy_a.axis[HID_Y_AXIS].min = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_a);
        }
    }
    return 0;
}

/* max */

static int set_joy_a_x_max(int val, void *param)
{
    if (joy_a.axis[HID_X_AXIS].max != val) {
        joy_a.axis[HID_X_AXIS].max = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_a);
        }
    }
    return 0;
}

static int set_joy_a_y_max(int val, void *param)
{
    if (joy_a.axis[HID_Y_AXIS].max != val) {
        joy_a.axis[HID_Y_AXIS].max = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_a);
        }
    }
    return 0;
}

/* logical */

static int set_joy_a_x_logical(int val, void *param)
{
    if (joy_a.axis[HID_X_AXIS].logical != val) {
        joy_a.axis[HID_X_AXIS].logical = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_a);
        }
    }
    return 0;
}

static int set_joy_a_y_logical(int val, void *param)
{
    if (joy_a.axis[HID_Y_AXIS].logical != val) {
        joy_a.axis[HID_Y_AXIS].logical = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_a);
        }
    }
    return 0;
}

static int set_joy_b_device_name(const char *val,void *param)
{
    util_string_set(&joy_b.device_name, val);
    if (joy_done_init) {
        setup_auto();
    }
    return 0;
}

static int set_joy_b_x_axis_name(const char *val,void *param)
{
    util_string_set(&joy_b.axis[HID_X_AXIS].name, val);
    if (joy_done_init) {
        setup_axis_mapping(&joy_b);
    }
    return 0;
}

static int set_joy_b_y_axis_name(const char *val,void *param)
{
    util_string_set(&joy_b.axis[HID_Y_AXIS].name, val);
    if (joy_done_init) {
        setup_axis_mapping(&joy_b);
    }
    return 0;
}

static int set_joy_b_button_mapping(const char *val,void *param)
{
    util_string_set(&joy_b.button_mapping, val);
    if (joy_done_init) {
        setup_button_mapping(&joy_b);
    }
    return 0;
}

static int set_joy_b_auto_button_mapping(const char *val,void *param)
{
    util_string_set(&joy_b.auto_button_mapping, val);
    if (joy_done_init) {
        setup_auto_button_mapping(&joy_b);
    }
    return 0;
}

/* threshold */

static int set_joy_b_x_threshold(int val, void *param)
{
    if (val < 0 || val > 100) {
        return -1;
    }

    if (joy_b.axis[HID_X_AXIS].threshold != val) {
        joy_b.axis[HID_X_AXIS].threshold = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_b);
        }
    }
    return 0;
}

static int set_joy_b_y_threshold(int val, void *param)
{
    if (val < 0 || val > 100) {
        return -1;
    }

    if (joy_b.axis[HID_Y_AXIS].threshold != val) {
        joy_b.axis[HID_Y_AXIS].threshold = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_b);
        }
    }
    return 0;
}

/* min */

static int set_joy_b_x_min(int val, void *param)
{
    if (joy_b.axis[HID_X_AXIS].min != val) {
        joy_b.axis[HID_X_AXIS].min = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_b);
        }
    }
    return 0;
}

static int set_joy_b_y_min(int val, void *param)
{
    if (joy_b.axis[HID_Y_AXIS].min != val) {
        joy_b.axis[HID_Y_AXIS].min = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_b);
        }
    }
    return 0;
}

/* max */

static int set_joy_b_x_max(int val, void *param)
{
    if (joy_b.axis[HID_X_AXIS].max != val) {
        joy_b.axis[HID_X_AXIS].max = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_b);
        }
    }
    return 0;
}

static int set_joy_b_y_max(int val, void *param)
{
    if (joy_b.axis[HID_Y_AXIS].max != val) {
        joy_b.axis[HID_Y_AXIS].max = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_b);
        }
    }
    return 0;
}

/* logical */

static int set_joy_b_x_logical(int val, void *param)
{
    if (joy_b.axis[HID_X_AXIS].logical != val) {
        joy_b.axis[HID_X_AXIS].logical = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_b);
        }
    }
    return 0;
}

static int set_joy_b_y_logical(int val, void *param)
{
    if (joy_b.axis[HID_Y_AXIS].logical != val) {
        joy_b.axis[HID_Y_AXIS].logical = val;
        if (joy_done_init) {
            setup_axis_mapping(&joy_b);
        }
    }
    return 0;
}

static int set_joy_a_hat_switch(int val, void *param)
{
    if (val != joy_a.hat_switch.id) {
        joy_a.hat_switch.id = val;
        if (joy_done_init) {
            setup_hat_switch_mapping(&joy_a);
        }
    }
    return 0;
}

static int set_joy_b_hat_switch(int val, void *param)
{
    if (val != joy_b.hat_switch.id) {
        joy_b.hat_switch.id = val;
        if (joy_done_init) {
            setup_hat_switch_mapping(&joy_b);
        }
    }
    return 0;
}

static const resource_string_t resources_string[] = {
    { "JoyADevice", "", RES_EVENT_NO, NULL,
      &joy_a.device_name, set_joy_a_device_name, NULL },
    { "JoyAXAxis", "X", RES_EVENT_NO, NULL,
      &joy_a.axis[HID_X_AXIS].name, set_joy_a_x_axis_name, NULL },
    { "JoyAYAxis", "Y", RES_EVENT_NO, NULL,
      &joy_a.axis[HID_Y_AXIS].name, set_joy_a_y_axis_name, NULL },
    { "JoyAButtons", "1:2:0:0:0:0", RES_EVENT_NO, NULL,
      &joy_a.button_mapping, set_joy_a_button_mapping, NULL },
    { "JoyAAutoButtons", "3:4:2:2:4:4", RES_EVENT_NO, NULL,
      &joy_a.auto_button_mapping, set_joy_a_auto_button_mapping, NULL },
    { "JoyBDevice", "", RES_EVENT_NO, NULL,
      &joy_b.device_name, set_joy_b_device_name, NULL },
    { "JoyBXAxis", "X", RES_EVENT_NO, NULL,
      &joy_b.axis[HID_X_AXIS].name, set_joy_b_x_axis_name, NULL },
    { "JoyBYAxis", "Y", RES_EVENT_NO, NULL,
      &joy_b.axis[HID_Y_AXIS].name, set_joy_b_y_axis_name, NULL },
    { "JoyBButtons", "1:2:0:0:0:0", RES_EVENT_NO, NULL,
      &joy_b.button_mapping, set_joy_b_button_mapping, NULL },
    { "JoyBAutoButtons", "3:4:2:2:4:4", RES_EVENT_NO, NULL,
      &joy_b.auto_button_mapping, set_joy_b_auto_button_mapping, NULL },
    RESOURCE_STRING_LIST_END
};

static const resource_int_t resources_int[] = {
    /* Axis ... Threshold */
    { "JoyAXThreshold", 50, RES_EVENT_NO, NULL,
      &joy_a.axis[HID_X_AXIS].threshold, set_joy_a_x_threshold, NULL },
    { "JoyAYThreshold", 50, RES_EVENT_NO, NULL,
      &joy_a.axis[HID_Y_AXIS].threshold, set_joy_a_y_threshold, NULL },
    { "JoyBXThreshold", 50, RES_EVENT_NO, NULL,
      &joy_b.axis[HID_X_AXIS].threshold, set_joy_b_x_threshold, NULL },
    { "JoyBYThreshold", 50, RES_EVENT_NO, NULL,
      &joy_b.axis[HID_Y_AXIS].threshold, set_joy_b_y_threshold, NULL },

    /* Axis ... Min */
    { "JoyAXMin", 0, RES_EVENT_NO, NULL,
      &joy_a.axis[HID_X_AXIS].min, set_joy_a_x_min, NULL },
    { "JoyAYMin", 0, RES_EVENT_NO, NULL,
      &joy_a.axis[HID_Y_AXIS].min, set_joy_a_y_min, NULL },
    { "JoyBXMin", 0, RES_EVENT_NO, NULL,
      &joy_b.axis[HID_X_AXIS].min, set_joy_b_x_min, NULL },
    { "JoyBYMin", 0, RES_EVENT_NO, NULL,
      &joy_b.axis[HID_Y_AXIS].min, set_joy_b_y_min, NULL },

    /* Axis ... Max */
    { "JoyAXMax", 0, RES_EVENT_NO, NULL,
      &joy_a.axis[HID_X_AXIS].max, set_joy_a_x_max, NULL },
    { "JoyAYMax", 0, RES_EVENT_NO, NULL,
      &joy_a.axis[HID_Y_AXIS].max, set_joy_a_y_max, NULL },
    { "JoyBXMax", 0, RES_EVENT_NO, NULL,
      &joy_b.axis[HID_X_AXIS].max, set_joy_b_x_max, NULL },
    { "JoyBYMax", 0, RES_EVENT_NO, NULL,
      &joy_b.axis[HID_Y_AXIS].max, set_joy_b_y_max, NULL },

    /* Axis ... Logical */
    { "JoyAXLogical", 0, RES_EVENT_NO, NULL,
      &joy_a.axis[HID_X_AXIS].logical, set_joy_a_x_logical, NULL },
    { "JoyAYLogical", 0, RES_EVENT_NO, NULL,
      &joy_a.axis[HID_Y_AXIS].logical, set_joy_a_y_logical, NULL },
    { "JoyBXLogical", 0, RES_EVENT_NO, NULL,
      &joy_b.axis[HID_X_AXIS].logical, set_joy_b_x_logical, NULL },
    { "JoyBYLogical", 0, RES_EVENT_NO, NULL,
      &joy_b.axis[HID_Y_AXIS].logical, set_joy_b_y_logical, NULL },

    { "JoyAHatSwitch", 1, RES_EVENT_NO, NULL,
      &joy_a.hat_switch.id, set_joy_a_hat_switch, NULL },
    { "JoyBHatSwitch", 1, RES_EVENT_NO, NULL,
      &joy_b.hat_switch.id, set_joy_b_hat_switch, NULL },
    RESOURCE_INT_LIST_END
};

int joy_arch_resources_init(void)
{
    if (resources_register_string(resources_string) < 0) {
        return -1;
    }

    return resources_register_int(resources_int);
}

/* ----- VICE Command-line options ----- */

static const cmdline_option_t cmdline_options[] = {
    { "-joyAdevice", SET_RESOURCE, 1,
      NULL, NULL, "JoyADevice", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<vid:pid:sn>", N_("Set HID A device") },
    { "-joyAxaxis", SET_RESOURCE, 1,
      NULL, NULL, "JoyAXAxis", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<X,Y,Z,Rx,Ry,Rz>", N_("Set X Axis for HID A device") },
    { "-joyAyaxis", SET_RESOURCE, 1,
      NULL, NULL, "JoyAYAxis", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<X,Y,Z,Rx,Ry,Rz>", N_("Set Y Axis for HID A device") },
    { "-joyAbuttons", SET_RESOURCE, 1,
      NULL, NULL, "JoyAButtons", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<f:af:l:r:u:d>", N_("Set Buttons for HID A device") },
    { "-joyAautobuttons", SET_RESOURCE, 1,
      NULL, NULL, "JoyAAutoButtons", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<af1:af2:af1p:af1r:af2p:af2r>", N_("Set Auto Fire Buttons for HID A device") },
    { "-joyAxthreshold", SET_RESOURCE, 1,
      NULL, NULL, "JoyAXThreshold", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<0-100>", N_("Set X Axis Threshold in Percent of HID A device") },
    { "-joyAythreshold", SET_RESOURCE, 1,
      NULL, NULL, "JoyAYThreshold", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<0-100>", N_("Set Y Axis Threshold in Percent of HID A device") },
    { "-joyBdevice", SET_RESOURCE, 1,
      NULL, NULL, "JoyBDevice", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<vid:pid:sn>", N_("Set HID B device") },
    { "-joyBxaxis", SET_RESOURCE, 1,
      NULL, NULL, "JoyBXAxis", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<X,Y,Z,Rx,Ry,Rz>", N_("Set X Axis for HID B device") },
    { "-joyByaxis", SET_RESOURCE, 1,
      NULL, NULL, "JoyBYAxis", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<X,Y,Z,Rx,Ry,Rz>", N_("Set Y Axis for HID B device") },
    { "-joyBbuttons", SET_RESOURCE, 1,
      NULL, NULL, "JoyBButtons", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<f:af:l:r:u:d>", N_("Set Buttons for HID B device") },
    { "-joyBautobuttons", SET_RESOURCE, 1,
      NULL, NULL, "JoyBAutoButtons", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<af1:af2:af1p:af1r:af2p:af2r>", N_("Set Auto Fire Buttons for HID B device") },
    { "-joyBxthreshold", SET_RESOURCE, 1,
      NULL, NULL, "JoyBXThreshold", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<0-100>", N_("Set X Axis Threshold in Percent of HID B device") },
    { "-joyBythreshold", SET_RESOURCE, 1,
      NULL, NULL, "JoyBYThreshold", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<0-100>", N_("Set Y Axis Threshold in Percent of HID B device") },
    { "-joyAhatswitch", SET_RESOURCE, 1,
      NULL, NULL, "JoyAHatSwitch", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<0-n>", N_("Set Hat Switch for Joystick of HID A device") },
    { "-joyBhatswitch", SET_RESOURCE, 1,
      NULL, NULL, "JoyBHatSwitch", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<0-n>", N_("Set Hat Switch for Joystick of HID B device") },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev1cmdline_options[] = {
    { "-joydev1", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice1", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<0-5>", N_("Set device for joystick port 1") },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev2cmdline_options[] = {
    { "-joydev2", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice2", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<0-5>", N_("Set device for joystick port 2") },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev3cmdline_options[] = {
    { "-extrajoydev1", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice3", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<0-5>", N_("Set device for extra joystick port 1") },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev4cmdline_options[] = {
    { "-extrajoydev2", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice4", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<0-5>", N_("Set device for extra joystick port 2") },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev5cmdline_options[] = {
    { "-extrajoydev3", SET_RESOURCE, 1,
      NULL, NULL, "JoyDevice5", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<0-5>", N_("Set device for extra joystick port 3") },
    CMDLINE_LIST_END
};

int joy_arch_cmdline_options_init(void)
{
    int num_ports = 0, num_extra_ports = 0;

    if (joyport_get_port_name(JOYPORT_1)) {
        if (cmdline_register_options(joydev1cmdline_options) < 0) {
            return -1;
        }
        num_ports++;
    }
    if (joyport_get_port_name(JOYPORT_2)) {
        if (cmdline_register_options(joydev2cmdline_options) < 0) {
            return -1;
        }
        num_ports++;
    }
    if (joyport_get_port_name(JOYPORT_3)) {
        if (cmdline_register_options(joydev3cmdline_options) < 0) {
            return -1;
        }
        num_extra_ports++;
    }
    if (joyport_get_port_name(JOYPORT_4)) {
        if (cmdline_register_options(joydev4cmdline_options) < 0) {
            return -1;
        }
        num_extra_ports++;
    }
    if (joyport_get_port_name(JOYPORT_5)) {
        if (cmdline_register_options(joydev5cmdline_options) < 0) {
            return -1;
        }
        num_extra_ports++;
    }

    joy_num_ports = num_ports;
    joy_num_extra_ports = num_extra_ports;

    return cmdline_register_options(cmdline_options);
}

/* ----- Setup Joystick Descriptor ---------------------------------------- */

void joy_calc_threshold(int min, int max, int threshold, int *t_min, int *t_max)
{
    int range = max - min;;
    int safe  = range * threshold / 200;
    *t_min = min + safe;
    *t_max = max - safe;
}

static const char *desc[] = { "horizontal", "vertical" }; 

static void setup_axis_mapping(joystick_descriptor_t *joy)
{
    int i;

    for (i = 0; i < 2; i++) {
        joy_axis_t *axis = &joy->axis[i];

        /* map axis name to HID usage */
        int usage = joy_hid_get_axis_usage(axis->name);
        if (usage == -1) {
            log_message(LOG_DEFAULT, "mac_joy:   %s axis not mapped", 
                        desc[i]);
            axis->mapped = 0;
        } else {
            /* if axis range is invalid then try to read axis range from device */
            if (axis->min >= axis->max) {
                int err = joy_hid_reset_axis_range(joy, i, usage, axis->logical);
                if (err == 0) {
                    log_message(LOG_DEFAULT, "mac_joy:   %s axis mapped to HID '%s': reset range",
                                desc[i], axis->name);
                }
            }

            /* try to map axis with given HID usage */
            int err = joy_hid_assign_axis(joy, i, usage, axis->logical);
            if (err == 0) {
                log_message(LOG_DEFAULT, "mac_joy:   %s axis mapped to HID '%s'. threshold=%d -> min=%d max=%d  [%d;%d] %s",
                            desc[i], axis->name,
                            axis->threshold, axis->min_threshold, axis->max_threshold,
                            axis->min, axis->max, axis->logical ? "logical" : "physical");
            } else {
                log_message(LOG_DEFAULT, "mac_joy:   NO %s axis not found on HID device",
                            axis->name);
            }
        }
    }
}

void joy_reset_axis_range(joystick_descriptor_t *joy, int id)
{
    joy_axis_t *axis = &joy->axis[id];
    int old_min = axis->min;
    int old_max = axis->max;

    int new_min, new_max;
    joy_hid_info_axis(joy, id, &new_min, &new_max, axis->logical);

    int changed = 0;
    if (new_min != old_min) {
        axis->min = new_min;
        changed = 1;
    }
    if (new_max != old_max) {
        axis->max = new_max;
        changed = 1;
    }
    if (changed) {
        log_message(LOG_DEFAULT, "mac_joy:   %s axis mapped to HID '%s': reset range",
                    desc[id], axis->name);
        setup_axis_mapping(joy);
    }
}

static void setup_button_mapping(joystick_descriptor_t *joy)
{
    int i;
    int ids[HID_NUM_BUTTONS];

    /* preset button id */
    for (i = 0; i < HID_NUM_BUTTONS; i++) {
        ids[i] = (i<2) ? i+1 : HID_INVALID_BUTTON;
    }

    /* decode button mapping resource */
    if (joy->button_mapping && strlen(joy->button_mapping) > 0) {
        if (sscanf(joy->button_mapping, "%d:%d:%d:%d:%d:%d", &ids[0], &ids[1], &ids[2], &ids[3], &ids[4], &ids[5]) != 6) {
            log_message(LOG_DEFAULT, "mac_joy: invalid button mapping!");
        }
    }

    /* try to map buttons in HID device */
    for (i = 0; i < HID_NUM_BUTTONS; i++) {
        joy->buttons[i].id = ids[i];
        joy->buttons[i].press = 0;
        joy->buttons[i].release = 0;
        if (ids[i] != HID_INVALID_BUTTON) {
            if (joy_hid_assign_button(joy, i, ids[i]) != 0) {
                log_message(LOG_DEFAULT, "mac_joy:   NO button %d on HID device!", ids[i]);
            }
        }
    }

    /* show button mapping */
    log_message(LOG_DEFAULT, "mac_joy:   buttons: fire_a=%d fire_b=%d left=%d right=%d up=%d down=%d",
        ids[HID_FIRE], ids[HID_ALT_FIRE], ids[HID_LEFT], ids[HID_RIGHT], ids[HID_UP], ids[HID_DOWN]);
}

static void setup_auto_button_mapping(joystick_descriptor_t *joy)
{
    int i;
    int ids[HID_NUM_AUTO_BUTTONS * 3];

    /* preset button id */
    int offset = HID_NUM_AUTO_BUTTONS;
    for (i = 0; i < HID_NUM_AUTO_BUTTONS; i++) {
        ids[i] = i+3;
        ids[offset++] = 5 * (i+1);
        ids[offset++] = 5 * (i+1);
    }

    /* decode auto button mapping resource */
    if (joy->auto_button_mapping && strlen(joy->auto_button_mapping) > 0) {
        if (sscanf(joy->auto_button_mapping, "%d:%d:%d:%d:%d:%d", &ids[0], &ids[1], &ids[2], &ids[3], &ids[4], &ids[5]) != 6) {
            log_message(LOG_DEFAULT, "mac_joy: invalid auto button mapping!");
        }
    }

    /* try to map auto buttons in HID device */
    offset = HID_NUM_AUTO_BUTTONS;
    for (i = 0; i < HID_NUM_AUTO_BUTTONS; i++) {
        int b = i + HID_NUM_BUTTONS; /* auto buttons are behind buttons */
        joy->buttons[b].id = ids[i];
        joy->buttons[b].press = ids[offset++];
        joy->buttons[b].release = ids[offset++];
        if (ids[i] != HID_INVALID_BUTTON) {
            if (joy_hid_assign_button(joy, b, ids[i]) != 0) {
                log_message(LOG_DEFAULT, "mac_joy:   NO auto button %d on HID device!", ids[i]);
            }
        }
    }

    /* show button mapping */
    log_message(LOG_DEFAULT, "mac_joy:   autofire buttons: autofire_a=%d [press=%d release=%d] autofire_b=%d [press=%d release=%d]",
        ids[0], ids[2], ids[3], ids[1], ids[4], ids[5]);
}

static void setup_hat_switch_mapping(joystick_descriptor_t *joy)
{
    int id = joy->hat_switch.id;
    if (id != HID_INVALID_BUTTON) {
        if (joy_hid_assign_hat_switch(joy, id) == 0) {
            log_message(LOG_DEFAULT, "mac_joy:   mapped hat switch: %d", id);
        } else {
            log_message(LOG_DEFAULT, "mac_joy:   NO hat switch %d on HID device!", id);
            joy->hat_switch.mapped = 0;
        }
    } else {
        joy->hat_switch.mapped = 0;
        log_message(LOG_DEFAULT, "mac_joy:   hat switch not mapped");
    }
}

/* determine if the given device matches the joystick descriptor */
static int match_joystick(joystick_descriptor_t *joy, joy_hid_device_t *dev)
{
    /* match by device name */
    if (joy->device_name && strlen(joy->device_name) > 0) {
        int vid, pid, serial;
        if (sscanf(joy->device_name, "%x:%x:%d", &vid, &pid, &serial) != 3) {
            return 0;
        }
        return (vid == dev->vendor_id) && (pid == dev->product_id) && (dev->serial == serial);
    }
    /* no match */
    return 0;
}

static void setup_joystick(joystick_descriptor_t *joy, joy_hid_device_t *dev, const char *desc)
{
    if (joy_hid_map_device(joy, dev) >= 0) {
        log_message(LOG_DEFAULT, "mac_joy: set up %s HID joystick (%d buttons, %d axis, %d hat switches)", 
                    desc, joy->num_hid_buttons, joy->num_hid_axis, joy->num_hid_hat_switches);
        setup_axis_mapping(joy);
        setup_button_mapping(joy);
        setup_auto_button_mapping(joy);
        setup_hat_switch_mapping(joy);
    } else {
        log_message(LOG_DEFAULT, "mac_joy: ERROR setting up %s HID joystick", desc);
    }
}

/* is the joystick auto assignable? */
static int do_auto_assign(joystick_descriptor_t *joy)
{
    return ((joy->device_name == NULL) || (strlen(joy->device_name) == 0));
}

static void setup_auto(void)
{
    int auto_assign_a = do_auto_assign(&joy_a);
    int auto_assign_b = do_auto_assign(&joy_b);
    int i;
    int num_devices;
    const joy_hid_device_array_t *devices;

    /* unmap both joysticks */
    joy_a.mapped = 0;
    joy_b.mapped = 0;

    /* query device list */
    devices = joy_hid_get_devices();
    if (devices == NULL) {
        log_message(LOG_DEFAULT, "mac_joy: can't find any HID devices!");
        return;
    }
    num_devices = devices->num_devices;

    /* walk through all enumerated devices */
    log_message(LOG_DEFAULT, "mac_joy: (auto) found %d HID devices. HID A='%s' B='%s'", 
                num_devices, joy_a.device_name, joy_b.device_name);
    for (i = 0; i < num_devices; i++) {
        joy_hid_device_t *dev = &devices->devices[i];

        log_message(LOG_DEFAULT, "mac_joy: found #%d joystick/gamepad: %04x:%04x:%d %s",
                    i, dev->vendor_id, dev->product_id, dev->serial, dev->product_name);

        /* query joy A */
        int assigned = 0;
        if (!auto_assign_a && match_joystick(&joy_a, dev)) {
            setup_joystick(&joy_a, dev, "matched A");
            assigned = 1;
        }
        /* query joy B */
        if (!auto_assign_b && match_joystick(&joy_b, dev)) {
            setup_joystick(&joy_b, dev, "matched B");
            assigned = 1;
        }

        if (!assigned) {
            /* auto assign a */
            if (auto_assign_a && (joy_a.mapped == 0)) {
                setup_joystick(&joy_a, dev, "auto-assigned A");
            }
            /* auto assign b */
            else if (auto_assign_b && (joy_b.mapped == 0)) {
                setup_joystick(&joy_b, dev, "auto-assigned B");
            }
        }
    }

    /* check if matched */
    if (!auto_assign_a && (joy_a.mapped == 0)) {
        log_message(LOG_DEFAULT, "mac_joy: joystick A not matched!");
    }
    if (!auto_assign_b && (joy_b.mapped == 0)) {
        log_message(LOG_DEFAULT, "mac_joy: joystick B not matched!");
    }
    log_message(LOG_DEFAULT, "mac_joy: (auto) done");
}

/* ----- API ----- */

/* check if a new joystick mapping is valid */
int joy_arch_set_device(int port, int new_dev)
{
    switch (new_dev) {
        case JOYDEV_NONE:
        case JOYDEV_NUMPAD:
        case JOYDEV_KEYSET1:
        case JOYDEV_KEYSET2:
        case JOYDEV_HID_0:
        case JOYDEV_HID_1:
            break;
        default:
            return -1;
    }

    return 0;
}

/* helper for UI to reload device list */
void joy_reload_device_list(void)
{
    if (joy_hid_reload() < 0) {
        joy_done_init = 0;
        log_message(LOG_DEFAULT, "mac_joy: ERROR loading HID device list! Disabling devices. Try again!");
    } else {
        const joy_hid_device_array_t *devices = joy_hid_get_devices();
        log_message(LOG_DEFAULT, "mac_joy: reloaded HID device list with %s. found %d devices",
                    devices->driver_name, devices->num_devices);
        setup_auto();
    }
}

/* query for available joysticks and set them up */
int joy_arch_init(void)
{
    if (joy_hid_init() < 0) {
        return 0;
    }

    joy_done_init = 1;

    /* print initial device list info */
    const joy_hid_device_array_t *devices = joy_hid_get_devices();
    log_message(LOG_DEFAULT, "mac_joy: loaded HID device list with %s. found %d devices",
                devices->driver_name, devices->num_devices);

    /* now assign HID joystick A,B if available */
    setup_auto();

    return 0;
}

/* close the device */
void joystick_close(void)
{
    joy_hid_unmap_device(&joy_a);
    joy_hid_unmap_device(&joy_b);
    joy_hid_exit();

    lib_free(joy_a.device_name);
    lib_free(joy_a.axis[HID_X_AXIS].name);
    lib_free(joy_a.axis[HID_Y_AXIS].name);
    lib_free(joy_a.button_mapping);
    lib_free(joy_a.auto_button_mapping);
    lib_free(joy_b.device_name);
    lib_free(joy_b.axis[HID_X_AXIS].name);
    lib_free(joy_b.axis[HID_Y_AXIS].name);
    lib_free(joy_b.button_mapping);
    lib_free(joy_b.auto_button_mapping);
}

/* ----- Read Joystick ----- */

static uint8_t read_button(joystick_descriptor_t *joy, int id, uint8_t resValue)
{
    /* button not mapped? */
    if (joy->buttons[id].mapped == 0) {
        return 0;
    }

    int value;
    if (joy_hid_read_button(joy, id, &value) != 0) {
        return 0;
    }

    return value ? resValue : 0;
}

static uint8_t read_auto_button(joystick_descriptor_t *joy, int id, uint8_t resValue)
{
    /* button not mapped? */
    joy_button_t *button = &joy->buttons[id];

    if (button->mapped == 0) {
        return 0;
    }

    int value;
    if (joy_hid_read_button(joy, id, &value) != 0) {
        return 0;
    }

    /* perform auto fire operation */
    int result = 0;
    if (value) {
        if(button->counter < button->press) {
            result = resValue;
        }
        button->counter ++;
        if (button->counter == (button->press + button->release)) {
            button->counter = 0;
        }
    } else {
        button->counter = 0;
    }
    return result;
}

static uint8_t read_axis(joystick_descriptor_t *joy, int id, uint8_t min, uint8_t max)
{
    joy_axis_t *axis = &joy->axis[id];
    if (axis->mapped == 0) {
        return 0;
    }

    int value;
    if (joy_hid_read_axis(joy, id, &value, axis->logical) != 0) {
        return 0;
    }

    if (value < axis->min_threshold) {
        return min;
    } else if (value > axis->max_threshold) {
        return max;
    } else {
        return 0;
    }
}

static uint8_t read_hat_switch(joystick_descriptor_t *joy)
{
    static const int map_hid_to_joy[8] = {
        1,   /* 1=N */
        1+8, /* 2=NE */
        8,   /* 3=E */
        8+2, /* 4=SE */
        2,   /* 5=S */
        4+2, /* 6=SW */
        4,   /* 7=W */
        1+4, /* 8=NW */
    };

    if (joy->hat_switch.mapped == 0) {
        return 0;
    }

    int value;
    if (joy_hid_read_hat_switch(joy, &value) != 0) {
        return 0;
    }

    /* valid hat directions: 1..8 */
    if ((value < 0) || (value > 7)) {
        return 0;
    }

    return map_hid_to_joy[value];
}

static uint8_t read_joystick(joystick_descriptor_t *joy)
{
    /* read buttons */
    uint8_t joy_bits = read_button(joy, HID_FIRE, 16) |
                    read_button(joy, HID_ALT_FIRE, 16) |
                    read_button(joy, HID_LEFT, 4) |
                    read_button(joy, HID_RIGHT, 8) |
                    read_button(joy, HID_UP, 1) |
                    read_button(joy, HID_DOWN, 2) |
                    read_auto_button(joy, HID_AUTO_FIRE, 16) |
                    read_auto_button(joy, HID_AUTO_ALT_FIRE, 16);

    /* axis */
    joy_bits |= read_axis(joy, HID_X_AXIS, 4, 8) |
                read_axis(joy, HID_Y_AXIS, 1, 2);

    /* hat switch */
    joy_bits |= read_hat_switch(joy);

    return joy_bits;
}

/* poll joystick */
void joystick(void)
{
    int i;

    /* handle both virtual cbm joystick ports */
    for (i = 0; i < 5; i++) {
        /* what kind of device is connected to the virtual port? */ 
        int joy_port = joystick_port_map[i];

        /* is HID joystick A mapped? */
        if (joy_port == JOYDEV_HID_0) {
            if (joy_a.mapped) {
                uint8_t joy_bits = read_joystick(&joy_a);
                joystick_set_value_absolute(i + 1, joy_bits);
            }
        }
        /* is HID joystick B mapped? */
        else if (joy_port == JOYDEV_HID_1) {
            if (joy_b.mapped) {
                uint8_t joy_bits = read_joystick(&joy_b);
                joystick_set_value_absolute(i + 1, joy_bits);
            }
        }
    }
}

#endif /* HAS_JOYSTICK */

#endif /* MACOSX_SUPPORT */
