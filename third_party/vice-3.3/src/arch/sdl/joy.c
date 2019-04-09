/*
 * joy.c - SDL joystick support.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Bernhard Kuhn <kuhn@eikon.e-technik.tu-muenchen.de>
 *  Ulmer Lionel <ulmer@poly.polytechnique.fr>
 *  Andreas Boose <viceteam@t-online.de>
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

#include "vice_sdl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archdep.h"
#include "cmdline.h"
#include "joy.h"
#include "joyport.h"
#include "joystick.h"
#include "kbd.h"
#include "keyboard.h"
#include "lib.h"
#include "log.h"
#include "resources.h"
#include "sysfile.h"
#include "util.h"
#include "uihotkey.h"
#include "uimenu.h"
#include "vkbd.h"

#define DEFAULT_JOYSTICK_THRESHOLD 10000
#define DEFAULT_JOYSTICK_FUZZ      1000

#ifdef HAVE_SDL_NUMJOYSTICKS
static log_t sdljoy_log = LOG_ERR;

/* Autorepeat in menu & vkbd */
static ui_menu_action_t autorepeat;
static int autorepeat_delay;

/* Joystick mapping filename */
static char *joymap_file = NULL;

/* Total number of joysticks */
static int num_joysticks;

/* Joystick threshold (0..32767) */
static int joystick_threshold;

/* Joystick fuzz (0..32767) */
static int joystick_fuzz;

/* Different types of joystick input */
typedef enum {
    AXIS = 0,
    BUTTON = 1,
    HAT = 2,
    BALL = 3,
    NUM_INPUT_TYPES
} sdljoystick_input_t;

/* Number of actions per input source */
static const int input_mult[NUM_INPUT_TYPES] = {
    2, /* Axis: actions for positive and negative */
    1, /* Button */
    4, /* Hat: actions for all 4 directions */
    1  /* Ball */
};

/* Actions to perform on joystick input */
typedef enum {
    NONE = 0,

    /* Joystick movement or button press */
    JOYSTICK = 1,

    /* Keyboard key press */
    KEYBOARD = 2,

    /* Map button */
    MAP = 3,

    /* (De)Activate UI */
    UI_ACTIVATE = 4,

    /* Call UI function */
    UI_FUNCTION = 5
} sdljoystick_action_t;

/* Input mapping for each direction/button/etc */
struct sdljoystick_mapping_s {
    /* Action to perform */
    sdljoystick_action_t action;

    /* Previous state of input */
    uint8_t prev;

    union {
        /* joy[0] = port number (0,1), joy[1] = pin number */
        uint8_t joy[2];

        /* key[0] = row, key[1] = column */
        int key[2];

        /* pointer to the menu item */
        ui_menu_entry_t *ui_function;
    } value;
};
typedef struct sdljoystick_mapping_s sdljoystick_mapping_t;

struct sdljoystick_s {
    SDL_Joystick *joyptr;
    const char *name;
    int input_max[NUM_INPUT_TYPES];
    sdljoystick_mapping_t *input[NUM_INPUT_TYPES];
};
typedef struct sdljoystick_s sdljoystick_t;

static sdljoystick_t *sdljoystick = NULL;

#endif /* HAVE_SDL_NUMJOYSTICKS */

/* ------------------------------------------------------------------------- */

int joy_arch_set_device(int port_idx, int new_dev)
{
    if (new_dev < 0 || new_dev > JOYDEV_MAX) {
        return -1;
    }

    return 0;
}

/* Resources.  */

#ifdef HAVE_SDL_NUMJOYSTICKS
static int set_joystick_threshold(int val, void *param)
{
    if (val < 0 || val > 32767) {
        return -1;
    }

    joystick_threshold = val;
    return 0;
}

static int set_joystick_fuzz(int val, void *param)
{
    if (val < 0 || val > 32767) {
        return -1;
    }

    joystick_fuzz = val;
    return 0;
}

static int joymap_file_set(const char *val, void *param)
{
    if (util_string_set(&joymap_file, val)) {
        return 0;
    }

    return joy_arch_mapping_load(joymap_file);
}

static resource_string_t resources_string[] = {
    { "JoyMapFile", NULL, RES_EVENT_NO, NULL,
      &joymap_file, joymap_file_set, (void *)0 },
    RESOURCE_STRING_LIST_END
};

static const resource_int_t resources_int[] = {
    { "JoyThreshold", DEFAULT_JOYSTICK_THRESHOLD, RES_EVENT_NO, NULL,
      &joystick_threshold, set_joystick_threshold, NULL },
    { "JoyFuzz", DEFAULT_JOYSTICK_FUZZ, RES_EVENT_NO, NULL,
      &joystick_fuzz, set_joystick_fuzz, NULL },
    RESOURCE_INT_LIST_END
};
#endif /* HAVE_SDL_NUMJOYSTICKS */

/* Command-line options.  */

#ifdef HAVE_SDL_NUMJOYSTICKS
static const cmdline_option_t cmdline_options[] =
{
    { "-joymap", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyMapFile", NULL,
      "<name>", "Specify name of joystick map file" },
    { "-joythreshold", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyThreshold", NULL,
      "<0-32767>", "Set joystick threshold" },
    { "-joyfuzz", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyFuzz", NULL,
      "<0-32767>", "Set joystick fuzz" },
    CMDLINE_LIST_END
};
#endif

static const cmdline_option_t joydev1cmdline_options[] =
{
    { "-joydev1", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice1", NULL,
      JOYDEV_RANGE_TEXT, JOYDEV_DESCRIPTION_1 },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev2cmdline_options[] =
{
    { "-joydev2", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice2", NULL,
      JOYDEV_RANGE_TEXT, JOYDEV_DESCRIPTION_2 },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev3cmdline_options[] =
{
    { "-extrajoydev1", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice3", NULL,
      JOYDEV_RANGE_TEXT, JOYDEV_DESCRIPTION_3 },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev4cmdline_options[] =
{
    { "-extrajoydev2", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice4", NULL,
      JOYDEV_RANGE_TEXT, JOYDEV_DESCRIPTION_4 },
    CMDLINE_LIST_END
};

static const cmdline_option_t joydev5cmdline_options[] =
{
    { "-extrajoydev3", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "JoyDevice5", NULL,
      JOYDEV_RANGE_TEXT, JOYDEV_DESCRIPTION_5 },
    CMDLINE_LIST_END
};

int joy_arch_resources_init(void)
{
    /* Init the keyboard resources here before resources_set_defaults is called */
    if (sdlkbd_init_resources() < 0) {
        return -1;
    }

#ifdef HAVE_SDL_NUMJOYSTICKS
    resources_string[0].factory_value = archdep_default_joymap_file_name();

    if (resources_register_string(resources_string) < 0) {
        return -1;
    }
    if (resources_register_int(resources_int) < 0) {
        return -1;
    }
#endif

    return 0;
}

void joy_arch_resources_shutdown(void)
{
#ifdef HAVE_SDL_NUMJOYSTICKS
    lib_free(resources_string[0].factory_value);
    resources_string[0].factory_value = NULL;
    lib_free(joymap_file);
    joymap_file = NULL;
#endif
}

int joy_arch_cmdline_options_init(void)
{
#ifdef HAVE_SDL_NUMJOYSTICKS
    if (cmdline_register_options(cmdline_options) < 0) {
        return -1;
    }
#endif

    if (sdlkbd_init_cmdline() < 0) {
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

#ifdef HAVE_SDL_NUMJOYSTICKS

/**********************************************************
 * Generic high level joy routine                         *
 **********************************************************/
int joy_arch_init(void)
{
    int i, axis, button, hat, ball;
    sdljoystick_input_t j;
    SDL_Joystick *joy;

    sdljoystick = NULL;

    sdljoy_log = log_open("SDLJoystick");

    if (SDL_InitSubSystem(SDL_INIT_JOYSTICK)) {
        log_error(sdljoy_log, "Subsystem init failed!");
        return -1;
    }

    num_joysticks = SDL_NumJoysticks();

    if (num_joysticks == 0) {
        log_message(sdljoy_log, "No joysticks found");
        return 0;
    }

    log_message(sdljoy_log, "%i joysticks found", num_joysticks);

    sdljoystick = lib_malloc(sizeof(sdljoystick_t) * num_joysticks);

    for (i = 0; i < num_joysticks; ++i) {
        joy = sdljoystick[i].joyptr = SDL_JoystickOpen(i);
        if (joy) {
#ifndef USE_SDLUI2
            sdljoystick[i].name = lib_stralloc(SDL_JoystickName(i));
#else
            sdljoystick[i].name = lib_stralloc(SDL_JoystickName(sdljoystick[i].joyptr));
#endif
            axis = sdljoystick[i].input_max[AXIS] = SDL_JoystickNumAxes(joy);
            button = sdljoystick[i].input_max[BUTTON] = SDL_JoystickNumButtons(joy);
            hat = sdljoystick[i].input_max[HAT] = SDL_JoystickNumHats(joy);
            ball = sdljoystick[i].input_max[BALL] = SDL_JoystickNumBalls(joy);

            for (j = AXIS; j < NUM_INPUT_TYPES; ++j) {
                if (sdljoystick[i].input_max[j] > 0) {
                    sdljoystick[i].input[j] = lib_malloc(sizeof(sdljoystick_mapping_t) * sdljoystick[i].input_max[j] * input_mult[j]);
                } else {
                    sdljoystick[i].input[j] = NULL;
                }
            }

            log_message(sdljoy_log, "Device %i \"%s\" (%i axes, %i buttons, %i hats, %i balls)", i, sdljoystick[i].name, axis, button, hat, ball);

            joy_arch_init_default_mapping(i);
        } else {
            log_warning(sdljoy_log, "Couldn't open joystick %i", i);
        }
    }

    joy_arch_mapping_load(joymap_file);

    SDL_JoystickEventState(SDL_ENABLE);
    return 0;
}

void joystick_close(void)
{
    int i;
    sdljoystick_input_t j;

#ifdef SDL_DEBUG
    fprintf(stderr, "%s\n", __func__);
#endif

    if (sdljoystick == NULL) {
        return;
    }

    for (i = 0; i < num_joysticks; ++i) {
        lib_free(sdljoystick[i].name);
        sdljoystick[i].name = NULL;

        if (sdljoystick[i].joyptr) {
            SDL_JoystickClose(sdljoystick[i].joyptr);
        }

        for (j = AXIS; j < NUM_INPUT_TYPES; ++j) {
            lib_free(sdljoystick[i].input[j]);
            sdljoystick[i].input[j] = NULL;
        }
    }

    lib_free(sdljoystick);
    sdljoystick = NULL;
}

/* ------------------------------------------------------------------------- */

void joy_arch_init_default_mapping(int joynum)
{
    int i, j, joyport, pin, state = 0;

    SDL_JoystickUpdate();

    /* FIXME some joysticks seem to need some time to self-calibrate after the
       first time it's opened/polled/read, otherwise the reported axis values
       are incorrect and get mapped to NONE. Quitting and restarting seems to
       fix this. Loading the joymap later in the init gets around this issue. */

    for (i = 0; i < sdljoystick[joynum].input_max[AXIS] * input_mult[AXIS]; ++i) {
        joyport = ((1 + joynum + ((i & 4) >> 2)) & 1);
        pin = 8 >> (i & 3);

        /* Poll each joystick axis once */
        if ((i % input_mult[AXIS]) == 0) {
            state = SDL_JoystickGetAxis(sdljoystick[joynum].joyptr, i / input_mult[AXIS]);
        }

        /* Check that the default joystick value is within the threshold.
           Some devices have axes that are +/-32767 when idle; mapping
           those to NONE (by default) avoids some problems. */
        if ((state > joystick_threshold) || (state < -joystick_threshold)) {
            log_warning(sdljoy_log, "Axis %i exceeds threshold, mapping to NONE", i / input_mult[AXIS]);
            for (j = 0; j < input_mult[AXIS]; ++j) {
                sdljoystick[joynum].input[AXIS][i + j].action = NONE;
            }
            i += (j - 1);
        } else {
            sdljoystick[joynum].input[AXIS][i].action = JOYSTICK;
            sdljoystick[joynum].input[AXIS][i].value.joy[0] = joyport;
            sdljoystick[joynum].input[AXIS][i].value.joy[1] = pin;
        }
    }

    for (i = 0; i < sdljoystick[joynum].input_max[BUTTON] * input_mult[BUTTON]; ++i) {
        switch (i & 3) {
            case 0:
            case 3:
            default:
                joyport = ((1 + joynum + (i & 1)) & 1);
                pin = 1 << 4;
                sdljoystick[joynum].input[BUTTON][i].action = JOYSTICK;
                sdljoystick[joynum].input[BUTTON][i].value.joy[0] = joyport;
                sdljoystick[joynum].input[BUTTON][i].value.joy[1] = pin;
                break;
            case 1:
                sdljoystick[joynum].input[BUTTON][i].action = UI_ACTIVATE;
                break;
            case 2:
                sdljoystick[joynum].input[BUTTON][i].action = MAP;
                break;
        }
    }

    for (i = 0; i < sdljoystick[joynum].input_max[HAT] * input_mult[HAT]; ++i) {
        joyport = ((1 + joynum + ((i & 4) >> 2)) & 1);
        pin = 1 << (i & 3);

        sdljoystick[joynum].input[HAT][i].action = JOYSTICK;
        sdljoystick[joynum].input[HAT][i].value.joy[0] = joyport;
        sdljoystick[joynum].input[HAT][i].value.joy[1] = pin;
    }

    for (i = 0; i < sdljoystick[joynum].input_max[BALL] * input_mult[BALL]; ++i) {
        sdljoystick[joynum].input[BALL][i].action = NONE;
    }
}

int joy_arch_mapping_dump(const char *filename)
{
    FILE *fp;
    int i, k;
    sdljoystick_input_t j;
    sdljoystick_action_t t;
    char *hotkey_path = NULL;

#ifdef SDL_DEBUG
    fprintf(stderr, "%s\n", __func__);
#endif

    if (filename == NULL) {
        return -1;
    }

    fp = fopen(filename, MODE_WRITE_TEXT);

    if (fp == NULL) {
        return -1;
    }

    fprintf(fp, "# VICE joystick mapping file\n"
            "#\n"
            "# A joystick map is read in as patch to the current map.\n"
            "#\n"
            "# File format:\n"
            "# - comment lines start with '#'\n"
            "# - keyword lines start with '!keyword'\n"
            "# - normal line has 'joynum inputtype inputindex action'\n"
            "#\n"
            "# Keywords and their lines are:\n"
            "# '!CLEAR'    clear all mappings\n"
            "#\n"
            );

    fprintf(fp, "# inputtype:\n"
            "# 0      axis\n"
            "# 1      button\n"
            "# 2      hat\n"
            "# 3      ball\n"
            "#\n"
            "# Note that each axis has 2 inputindex entries and each hat has 4.\n"
            "#\n"
            "# action [action_parameters]:\n"
            "# 0               none\n"
            "# 1 port pin      joystick (pin: 1/2/4/8/16 = u/d/l/r/fire)\n"
            "# 2 row col       keyboard\n"
            "# 3               map\n"
            "# 4               UI activate\n"
            "# 5 path&to&item  UI function\n"
            "#\n\n"
            );

    fprintf(fp, "!CLEAR\n\n");

    for (i = 0; i < num_joysticks; ++i) {
        fprintf(fp, "# %s\n", sdljoystick[i].name);
        for (j = AXIS; j < NUM_INPUT_TYPES; ++j) {
            for (k = 0; k < sdljoystick[i].input_max[j] * input_mult[j]; ++k) {
                t = sdljoystick[i].input[j][k].action;
                fprintf(fp, "%i %i %i %i", i, j, k, t);
                switch (t) {
                    case JOYSTICK:
                        fprintf(fp, " %i %i",
                                sdljoystick[i].input[j][k].value.joy[0],
                                sdljoystick[i].input[j][k].value.joy[1]
                                );
                        break;
                    case KEYBOARD:
                        fprintf(fp, " %i %i",
                                sdljoystick[i].input[j][k].value.key[0],
                                sdljoystick[i].input[j][k].value.key[1]
                                );
                        break;
                    case UI_FUNCTION:
                        hotkey_path = sdl_ui_hotkey_path(sdljoystick[i].input[j][k].value.ui_function);
                        fprintf(fp, " %s", hotkey_path);
                        lib_free(hotkey_path);
                        break;
                    default:
                        break;
                }
                fprintf(fp, "\n");
            }
            fprintf(fp, "\n");
        }
        fprintf(fp, "\n\n");
    }

    fclose(fp);

    return 0;
}

static void joy_arch_keyword_clear(void)
{
    int i, k;
    sdljoystick_input_t j;

    for (i = 0; i < num_joysticks; ++i) {
        for (j = AXIS; j < NUM_INPUT_TYPES; ++j) {
            for (k = 0; k < sdljoystick[i].input_max[j] * input_mult[j]; ++k) {
                sdljoystick[i].input[j][k].action = NONE;
            }
        }
    }
}

static void joy_arch_parse_keyword(char *buffer)
{
    char *key;

    key = strtok(buffer + 1, " \t:");

    if (!strcmp(key, "CLEAR")) {
        joy_arch_keyword_clear();
    }
}

static void joy_arch_parse_entry(char *buffer)
{
    char *p;
    int joynum, inputindex, data1 = 0, data2 = 0;
    sdljoystick_input_t inputtype;
    sdljoystick_action_t action;

    p = strtok(buffer, " \t:");

    joynum = atoi(p);

    if (joynum >= num_joysticks) {
        log_error(sdljoy_log, "Could not find joystick %i!", joynum);
        return;
    }

    p = strtok(NULL, " \t,");
    if (p != NULL) {
        inputtype = (sdljoystick_input_t)atoi(p);
        p = strtok(NULL, " \t,");
        if (p != NULL) {
            inputindex = atoi(p);
            p = strtok(NULL, " \t");
            if (p != NULL) {
                action = (sdljoystick_action_t)atoi(p);

                switch (action) {
                    case UI_FUNCTION:
                        p = strtok(NULL, "\t\r\n");
                        break;
                    case JOYSTICK:
                    case KEYBOARD:
                        p = strtok(NULL, " \t");
                        data1 = atoi(p);
                        p = strtok(NULL, " \t");
                        data2 = atoi(p);
                        break;
                    default:
                        break;
                }

                if (inputindex < sdljoystick[joynum].input_max[inputtype] * input_mult[inputtype]) {
                    sdljoystick[joynum].input[inputtype][inputindex].action = action;

                    switch (action) {
                        case JOYSTICK:
                            sdljoystick[joynum].input[inputtype][inputindex].value.joy[0] = data1;
                            sdljoystick[joynum].input[inputtype][inputindex].value.joy[1] = data2;
                            break;
                        case KEYBOARD:
                            sdljoystick[joynum].input[inputtype][inputindex].value.key[0] = data1;
                            sdljoystick[joynum].input[inputtype][inputindex].value.key[1] = data2;
                            break;
                        case UI_FUNCTION:
                            sdljoystick[joynum].input[inputtype][inputindex].value.ui_function = sdl_ui_hotkey_action(p);
                            break;
                        default:
                            break;
                    }
                } else {
                    log_warning(sdljoy_log, "inputindex %i too large for inputtype %i, joynum %i!", inputindex, inputtype, joynum);
                }
            }
        }
    }
}

int joy_arch_mapping_load(const char *filename)
{
    FILE *fp;
    char *complete_path;
    char buffer[1000];

#ifdef SDL_DEBUG
    fprintf(stderr, "%s, %s\n", __func__, filename);
#endif

    /* Silently ignore keymap load on resource & cmdline init */
    if (sdljoystick == NULL) {
        return 0;
    }

    if (filename == NULL) {
        return -1;
    }

    fp = sysfile_open(filename, &complete_path, MODE_READ_TEXT);

    if (fp == NULL) {
        log_warning(sdljoy_log, "Failed to open `%s'.", filename);
        return -1;
    }

    log_message(sdljoy_log, "Loading joystick map `%s'.", complete_path);

    lib_free(complete_path);

    do {
        buffer[0] = 0;
        if (fgets(buffer, 999, fp)) {
            char *p;

            if (strlen(buffer) == 0) {
                break;
            }

            buffer[strlen(buffer) - 1] = 0; /* remove newline */

            /* remove comments */
            if ((p = strchr(buffer, '#'))) {
                *p = 0;
            }

            switch (*buffer) {
                case 0:
                    break;
                case '!':
                    /* keyword handling */
                    joy_arch_parse_keyword(buffer);
                    break;
                default:
                    /* table entry handling */
                    joy_arch_parse_entry(buffer);
                    break;
            }
        }
    } while (!feof(fp));
    fclose(fp);

    return 0;
}

/* ------------------------------------------------------------------------- */

static inline uint8_t sdljoy_axis_direction(Sint16 value, uint8_t prev)
{
    int thres = joystick_threshold;

    if (prev == 0) {
        thres += joystick_fuzz;
    } else {
        thres -= joystick_fuzz;
    }

    if (value < -thres) {
        return 2;
    } else if (value > thres) {
        return 1;
    } else if ((value < thres) && (value > -thres)) {
        return 0;
    }

    return prev;
}

static inline uint8_t sdljoy_hat_direction(Uint8 value, uint8_t prev)
{
    uint8_t b;

    b = (value ^ prev) & value;
    b &= SDL_HAT_UP | SDL_HAT_DOWN | SDL_HAT_LEFT | SDL_HAT_RIGHT;

    switch (b) {
        case SDL_HAT_UP:
            return 1;
        case SDL_HAT_DOWN:
            return 2;
        case SDL_HAT_LEFT:
            return 3;
        case SDL_HAT_RIGHT:
            return 4;
        default:
            /* ignore diagonals and releases */
            break;
    }

    return 0;
}

static sdljoystick_mapping_t *sdljoy_get_mapping(SDL_Event e)
{
    sdljoystick_mapping_t *retval = NULL;
    uint8_t cur;

    switch (e.type) {
        case SDL_JOYAXISMOTION:
            cur = sdljoy_axis_direction(e.jaxis.value, 0);
            if (cur > 0) {
                --cur;
                retval = &(sdljoystick[e.jaxis.which].input[AXIS][e.jaxis.axis * input_mult[AXIS] + cur]);
            }
            break;
        case SDL_JOYHATMOTION:
            cur = sdljoy_hat_direction(e.jhat.value, 0);
            if (cur > 0) {
                --cur;
                retval = &(sdljoystick[e.jhat.which].input[HAT][e.jhat.hat * input_mult[HAT] + cur]);
            }
            break;
        case SDL_JOYBUTTONDOWN:
            retval = &(sdljoystick[e.jbutton.which].input[BUTTON][e.jbutton.button]);
            break;
        default:
            break;
    }
    return retval;
}

static ui_menu_action_t sdljoy_perform_event(sdljoystick_mapping_t *event, int value)
{
    uint8_t t;
    ui_menu_action_t retval = MENU_ACTION_NONE;

    autorepeat = MENU_ACTION_NONE;

    if (sdl_menu_state || (sdl_vkbd_state & SDL_VKBD_ACTIVE)) {
        if (event->action == JOYSTICK) {
            switch (event->value.joy[1]) {
                case 0x01:
                    retval = autorepeat = MENU_ACTION_UP;
                    break;
                case 0x02:
                    retval = autorepeat = MENU_ACTION_DOWN;
                    break;
                case 0x04:
                    retval = autorepeat = MENU_ACTION_LEFT;
                    break;
                case 0x08:
                    retval = autorepeat = MENU_ACTION_RIGHT;
                    break;
                case 0x10:
                    retval = MENU_ACTION_SELECT;
                    break;
                default:
                    break;
            }
        } else if (event->action == UI_ACTIVATE) {
            retval = MENU_ACTION_CANCEL;
        } else if (event->action == MAP) {
            retval = MENU_ACTION_MAP;
        }
        if (!value) {
            autorepeat = MENU_ACTION_NONE;
            autorepeat_delay = 30;
            retval += MENU_ACTION_NONE_RELEASE;
        }
        return retval;
    }

    autorepeat_delay = 30;

    switch (event->action) {
        case JOYSTICK:
            t = event->value.joy[0];
            if (joystick_port_map[t] == JOYDEV_JOYSTICK) {
                if (value) {
                    joystick_set_value_or(t + 1, event->value.joy[1]);
                } else {
                    joystick_set_value_and(t + 1, (uint8_t) ~(event->value.joy[1]));
                }
            }
            break;
        case KEYBOARD:
            keyboard_set_keyarr_any(event->value.key[0], event->value.key[1], value);
            break;
        case UI_ACTIVATE:
            if (value) {
                sdl_ui_activate();
            }
            break;
        case UI_FUNCTION:
            if (value && event->value.ui_function) {
                sdl_ui_hotkey(event->value.ui_function);
            }
            break;
        case NONE:
        default:
            break;
    }

    return retval;
}

/* ------------------------------------------------------------------------- */

ui_menu_action_t sdljoy_autorepeat(void)
{
    if (autorepeat_delay) {
        if (autorepeat != MENU_ACTION_NONE) {
            --autorepeat_delay;
        }
        return MENU_ACTION_NONE;
    } else {
        autorepeat_delay = 4;
    }
    return autorepeat;
}

uint8_t sdljoy_check_axis_movement(SDL_Event e)
{
    uint8_t cur, prev;
    int index;
    Uint8 joynum;
    Uint8 axis;
    Sint16 value;

    joynum = e.jaxis.which;
    axis = e.jaxis.axis;
    value = e.jaxis.value;

    index = axis * input_mult[AXIS];
    prev = sdljoystick[joynum].input[AXIS][index].prev;

    cur = sdljoy_axis_direction(value, prev);

    if (cur == prev) {
        return 0;
    }

    sdljoystick[joynum].input[AXIS][index].prev = cur;
    return cur;
}

uint8_t sdljoy_check_hat_movement(SDL_Event e)
{
    uint8_t cur, prev;
    int index;
    Uint8 joynum;
    Uint8 hat;
    Uint8 value;

    joynum = e.jhat.which;
    hat = e.jhat.hat;
    value = e.jhat.value;

    index = hat * input_mult[HAT];
    prev = sdljoystick[joynum].input[HAT][index].prev;

    if (value == prev) {
        return 0;
    }

    cur = sdljoy_hat_direction(value, prev);

    sdljoystick[joynum].input[HAT][index].prev = value;
    return cur;
}

ui_menu_action_t sdljoy_axis_event(Uint8 joynum, Uint8 axis, Sint16 value)
{
    uint8_t cur, prev;
    int index;
    ui_menu_action_t retval = MENU_ACTION_NONE;

    index = axis * input_mult[AXIS];
    prev = sdljoystick[joynum].input[AXIS][index].prev;

    cur = sdljoy_axis_direction(value, prev);

    if (cur == prev) {
        return retval;
    }

    if (cur == 1) {
        if (prev == 2) {
            sdljoy_perform_event(&(sdljoystick[joynum].input[AXIS][index + 1]), 0);
        }
        retval = sdljoy_perform_event(&(sdljoystick[joynum].input[AXIS][index]), 1);
    } else if (cur == 2) {
        if (prev == 1) {
            sdljoy_perform_event(&(sdljoystick[joynum].input[AXIS][index]), 0);
        }
        retval = sdljoy_perform_event(&(sdljoystick[joynum].input[AXIS][index + 1]), 1);
    } else {
        if (prev == 1) {
            sdljoy_perform_event(&(sdljoystick[joynum].input[AXIS][index]), 0);
        } else {
            sdljoy_perform_event(&(sdljoystick[joynum].input[AXIS][index + 1]), 0);
        }
    }

    sdljoystick[joynum].input[AXIS][index].prev = cur;
    return retval;
}

ui_menu_action_t sdljoy_button_event(Uint8 joynum, Uint8 button, Uint8 value)
{
    return sdljoy_perform_event(&(sdljoystick[joynum].input[BUTTON][button]), value);
}

ui_menu_action_t sdljoy_hat_event(Uint8 joynum, Uint8 hat, Uint8 value)
{
    uint8_t prev;
    int index;
    ui_menu_action_t retval = MENU_ACTION_NONE;

    index = hat * input_mult[HAT];
    prev = sdljoystick[joynum].input[HAT][index].prev;

    if (value == prev) {
        return retval;
    }

    if ((value & SDL_HAT_UP) && (!(prev & SDL_HAT_UP))) {
        if (prev & SDL_HAT_DOWN) {
            sdljoy_perform_event(&(sdljoystick[joynum].input[HAT][index + 1]), 0);
        }
        retval = sdljoy_perform_event(&(sdljoystick[joynum].input[HAT][index]), 1);
    } else if ((value & SDL_HAT_DOWN) && (!(prev & SDL_HAT_DOWN))) {
        if (prev & SDL_HAT_UP) {
            sdljoy_perform_event(&(sdljoystick[joynum].input[HAT][index]), 0);
        }
        retval = sdljoy_perform_event(&(sdljoystick[joynum].input[HAT][index + 1]), 1);
    } else {
        if ((!(value & SDL_HAT_UP)) && (prev & SDL_HAT_UP)) {
            sdljoy_perform_event(&(sdljoystick[joynum].input[HAT][index]), 0);
        } else if ((!(value & SDL_HAT_DOWN)) && (prev & SDL_HAT_DOWN)) {
            sdljoy_perform_event(&(sdljoystick[joynum].input[HAT][index + 1]), 0);
        }
    }

    /* TODO a retval is missed on simultaneous X/Y events */
    if ((value & SDL_HAT_LEFT) && (!(prev & SDL_HAT_LEFT))) {
        if (prev & SDL_HAT_RIGHT) {
            sdljoy_perform_event(&(sdljoystick[joynum].input[HAT][index + 3]), 0);
        }
        retval = sdljoy_perform_event(&(sdljoystick[joynum].input[HAT][index + 2]), 1);
    } else if ((value & SDL_HAT_RIGHT) && (!(prev & SDL_HAT_RIGHT))) {
        if (prev & SDL_HAT_LEFT) {
            sdljoy_perform_event(&(sdljoystick[joynum].input[HAT][index + 2]), 0);
        }
        retval = sdljoy_perform_event(&(sdljoystick[joynum].input[HAT][index + 3]), 1);
    } else {
        if ((!(value & SDL_HAT_LEFT)) && (prev & SDL_HAT_LEFT)) {
            sdljoy_perform_event(&(sdljoystick[joynum].input[HAT][index + 2]), 0);
        } else if ((!(value & SDL_HAT_RIGHT)) && (prev & SDL_HAT_RIGHT)) {
            sdljoy_perform_event(&(sdljoystick[joynum].input[HAT][index + 3]), 0);
        }
    }

    sdljoystick[joynum].input[HAT][index].prev = value;
    return retval;
}

/* ------------------------------------------------------------------------- */

/* unused at the moment (2014-07-19, compyx) */
#if 0
static ui_menu_entry_t *sdljoy_get_hotkey(SDL_Event e)
{
    ui_menu_entry_t *retval = NULL;
    sdljoystick_mapping_t *joyevent = sdljoy_get_mapping(e);

    if ((joyevent != NULL) && (joyevent->action == UI_FUNCTION)) {
        retval = joyevent->value.ui_function;
    }

    return retval;
}
#endif

void sdljoy_set_joystick(SDL_Event e, int port, int bits)
{
    sdljoystick_mapping_t *joyevent = sdljoy_get_mapping(e);

    if (joyevent != NULL) {
        joyevent->action = JOYSTICK;
        joyevent->value.joy[0] = (uint8_t)port;
        joyevent->value.joy[1] = (uint8_t)bits;
    }
}

void sdljoy_set_hotkey(SDL_Event e, ui_menu_entry_t *value)
{
    sdljoystick_mapping_t *joyevent = sdljoy_get_mapping(e);

    if (joyevent != NULL) {
        joyevent->action = UI_FUNCTION;
        joyevent->value.ui_function = value;
    }
}

void sdljoy_set_keypress(SDL_Event e, int row, int col)
{
    sdljoystick_mapping_t *joyevent = sdljoy_get_mapping(e);

    if (joyevent != NULL) {
        joyevent->action = KEYBOARD;
        joyevent->value.key[0] = row;
        joyevent->value.key[1] = col;
    }
}

void sdljoy_set_extra(SDL_Event e, int type)
{
    sdljoystick_mapping_t *joyevent = sdljoy_get_mapping(e);

    if (joyevent != NULL) {
        joyevent->action = type ? MAP : UI_ACTIVATE;
    }
}

void sdljoy_unset(SDL_Event e)
{
    sdljoystick_mapping_t *joyevent = sdljoy_get_mapping(e);

    if (joyevent != NULL) {
        joyevent->action = NONE;
    }
}

/* ------------------------------------------------------------------------- */

static int _sdljoy_swap_ports = 0;
static int _sdljoy_swap_userport_ports = 0;

void sdljoy_swap_ports(void)
{
    int i, k;
    sdljoystick_input_t j;

    resources_get_int("JoyDevice1", &i);
    resources_get_int("JoyDevice2", &k);
    resources_set_int("JoyDevice1", k);
    resources_set_int("JoyDevice2", i);
    _sdljoy_swap_ports ^= 1;

    for (i = 0; i < num_joysticks; ++i) {
        for (j = AXIS; j < NUM_INPUT_TYPES; ++j) {
            for (k = 0; k < sdljoystick[i].input_max[j] * input_mult[j]; ++k) {
                if (sdljoystick[i].input[j][k].action == JOYSTICK) {
                    sdljoystick[i].input[j][k].value.joy[0] ^= 1;
                }
            }
        }
    }
}

int sdljoy_get_swap_ports(void) 
{
    return _sdljoy_swap_ports;
}

void sdljoy_swap_userport_ports(void)
{
    int i, k;
 
    resources_get_int("JoyDevice3", &i);
    resources_get_int("JoyDevice4", &k);
    resources_set_int("JoyDevice3", k);
    resources_set_int("JoyDevice4", i);
    _sdljoy_swap_userport_ports ^= 1;
}

int sdljoy_get_swap_userport_ports(void) 
{
    return _sdljoy_swap_userport_ports;
}


/* ------------------------------------------------------------------------- */

#else
/* !HAVE_SDL_NUMJOYSTICKS */

void sdljoy_swap_ports(void)
{
    int i, k;

    resources_get_int("JoyDevice1", &i);
    resources_get_int("JoyDevice2", &k);
    resources_set_int("JoyDevice1", k);
    resources_set_int("JoyDevice2", i);
}

void joystick_close(void)
{
}

int joy_arch_init(void)
{
    return 0;
}

#endif
