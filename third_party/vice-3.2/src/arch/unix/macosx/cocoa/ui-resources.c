/*
 * ui-resources.c - MacVICE resources
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *  Michael Klein <michael.klein@puffin.lb.shuttle.de>
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
#include "lib.h"
#include "resources.h"
#include "util.h"

struct ui_resources_s {
    int save_resources_on_exit;
    int confirm_on_exit;
};
typedef struct ui_resources_s ui_resources_t;

static ui_resources_t ui_resources;

static unsigned int ui_resources_initialized = 0;

static int set_save_resources_on_exit(int val, void *param)
{
    ui_resources.save_resources_on_exit = val ? 1 : 0;

    return 0;
}

static int set_confirm_on_exit(int val, void *param)
{
    ui_resources.confirm_on_exit = val ? 1 : 0;

    return 0;
}

static const resource_int_t resources_int[] = {
    { "SaveResourcesOnExit", 0, RES_EVENT_NO, NULL,
      &ui_resources.save_resources_on_exit, set_save_resources_on_exit, NULL },
    { "ConfirmOnExit", 1, RES_EVENT_NO, NULL,
      &ui_resources.confirm_on_exit, set_confirm_on_exit, NULL },
    RESOURCE_INT_LIST_END
};

int ui_resources_init(void)
{
    ui_resources_initialized = 1;
    return resources_register_int(resources_int);
}

void ui_resources_shutdown(void)
{
}

