/*
 * menu_reset.c - Implementation of the reset settings menu for the SDL UI.
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

#include <stdio.h>

#include "drive.h"
#include "machine.h"
#include "menu_common.h"
#include "uimenu.h"
#include "vsync.h"

static UI_MENU_CALLBACK(maincpu_hard_reset_callback)
{
    if (activated) {
        vsync_suspend_speed_eval();
        machine_trigger_reset(MACHINE_RESET_MODE_HARD);
        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

static UI_MENU_CALLBACK(maincpu_soft_reset_callback)
{
    if (activated) {
        vsync_suspend_speed_eval();
        machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

static UI_MENU_CALLBACK(drive8cpu_reset_callback)
{
    if (activated) {
        vsync_suspend_speed_eval();
        drive_cpu_trigger_reset(0);
        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

static UI_MENU_CALLBACK(drive9cpu_reset_callback)
{
    if (activated) {
        vsync_suspend_speed_eval();
        drive_cpu_trigger_reset(1);
        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

static UI_MENU_CALLBACK(drive10cpu_reset_callback)
{
    if (activated) {
        vsync_suspend_speed_eval();
        drive_cpu_trigger_reset(2);
        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

static UI_MENU_CALLBACK(drive11cpu_reset_callback)
{
    if (activated) {
        vsync_suspend_speed_eval();
        drive_cpu_trigger_reset(3);
        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

const ui_menu_entry_t reset_menu[] = {
    { "Soft",
      MENU_ENTRY_OTHER,
      maincpu_soft_reset_callback,
      NULL },
    { "Hard",
      MENU_ENTRY_OTHER,
      maincpu_hard_reset_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Drive 8",
      MENU_ENTRY_OTHER,
      drive8cpu_reset_callback,
      NULL },
    { "Drive 9",
      MENU_ENTRY_OTHER,
      drive9cpu_reset_callback,
      NULL },
    { "Drive 10",
      MENU_ENTRY_OTHER,
      drive10cpu_reset_callback,
      NULL },
    { "Drive 11",
      MENU_ENTRY_OTHER,
      drive11cpu_reset_callback,
      NULL },
    SDL_MENU_LIST_END
};
