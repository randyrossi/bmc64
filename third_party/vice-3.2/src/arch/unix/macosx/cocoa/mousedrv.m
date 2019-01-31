/*
 * mousedrv.m - MacVICE mouse driver
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
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

#include "cmdline.h"
#include "mousedrv.h"
#include "resources.h"
#include "translate.h"
#include "vicemachine.h"
#include "vsyncapi.h"

// mouse.c
extern int _mouse_enabled;

static mouse_func_t mouse_funcs;
static BOOL firstMove;
static float last_x;
static float last_y;
static float mouse_x;
static float mouse_y;
static unsigned long mouse_timestamp = 0;

static int scale_x;
static int scale_y;

static int set_scale_x(int val, void *param)
{
    if ((val > 0) && (val <= 64)) {
        scale_x = val;
    }
    return 0;
}

static int set_scale_y(int val, void *param)
{
    if ((val > 0) && (val <= 64)) {
        scale_y = val;
    }
    return 0;
}

static resource_int_t resources_int[] =
{
    { "MouseScaleX", 1, RES_EVENT_NO, NULL,
       &scale_x, set_scale_x, NULL },
    { "MouseScaleY", 1, RES_EVENT_NO, NULL,
       &scale_y, set_scale_y, NULL },
    RESOURCE_INT_LIST_END
};

int mousedrv_resources_init(mouse_func_t *funcs)
{
    mouse_funcs.mbl = funcs->mbl;
    mouse_funcs.mbr = funcs->mbr;
    mouse_funcs.mbm = funcs->mbm;
    mouse_funcs.mbu = funcs->mbu;
    mouse_funcs.mbd = funcs->mbd;
    return resources_register_int(resources_int);
}

static const cmdline_option_t cmdline_options[] = {
    { "-mousescalex", SET_RESOURCE, 1,
      NULL, NULL, "MouseScaleX", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<1-8>", N_("Set scaling factor for mouse movement along X") },
    { "-mousescaley", SET_RESOURCE, 1,
      NULL, NULL, "MouseScaleY", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<1-8>", N_("Set scaling factor for mouse movement along Y") },
    CMDLINE_LIST_END
};

int mousedrv_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

void mousedrv_init(void)
{
}

void mousedrv_mouse_changed(void)
{
    [[theVICEMachine machineNotifier] postToggleMouseNotification:_mouse_enabled];

    if(_mouse_enabled) {
        firstMove = YES;
        mouse_x = 0.0f;
        mouse_y = 0.0f;
    }
}

// the HW polls the position
int mousedrv_get_x(void)
{
    return (int)(mouse_x) & 0xffff;
}

int mousedrv_get_y(void)
{
    return (int)(mouse_y) & 0xffff;
}

// this call receives absolute mouse pos on canvas
void mouse_move_f(float x, float y)
{
    mouse_timestamp = vsyncarch_gettime();

    if(firstMove) {
        // ignore first move
        firstMove = NO;
    } else {
        // calc delta movement
        float dx = x - last_x;
        float dy = y - last_y;
    
        // apply (optional) scale on movement
        dx *= scale_x;
        dy *= scale_y;
    
        // add onto current mouse pos
        mouse_x += dx;
        mouse_y += dy;
        
        // map to 0 .. 0xffff range
        while (mouse_x < 0.0) {
            mouse_x += 65536.0;
        }
        while (mouse_x >= 65536.0) {
            mouse_x -= 65536.0;
        }
        while (mouse_y < 0.0) {
            mouse_y += 65536.0;
        }
        while (mouse_y >= 65536.0) {
            mouse_y -= 65536.0;
        }
    }
    
    // store last pos
    last_x = x;
    last_y = y;
}

unsigned long mousedrv_get_timestamp(void)
{
    return mouse_timestamp;
}

void mousedrv_button_left(int pressed)
{
    mouse_funcs.mbl(pressed);
}

void mousedrv_button_right(int pressed)
{
    mouse_funcs.mbr(pressed);
}

void mousedrv_button_middle(int pressed)
{
    mouse_funcs.mbm(pressed);
}
