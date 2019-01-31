/*
 * menu_reset.c - Implementation of the cpu jam settings menu for the SDL UI.
 *
 * Written by
 *  groepaz <groepaz@gmx.net>
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

#include "machine.h"
#include "menu_common.h"
#include "uimenu.h"

UI_MENU_DEFINE_RADIO(JAMAction)

const ui_menu_entry_t jam_menu[] = {
    { "Ask", MENU_ENTRY_RESOURCE_RADIO, radio_JAMAction_callback, (ui_callback_data_t)MACHINE_JAM_ACTION_DIALOG },
    { "Continue", MENU_ENTRY_RESOURCE_RADIO, radio_JAMAction_callback, (ui_callback_data_t)MACHINE_JAM_ACTION_CONTINUE },
    { "Start monitor", MENU_ENTRY_RESOURCE_RADIO, radio_JAMAction_callback, (ui_callback_data_t)MACHINE_JAM_ACTION_MONITOR },
    { "Reset", MENU_ENTRY_RESOURCE_RADIO, radio_JAMAction_callback, (ui_callback_data_t)MACHINE_JAM_ACTION_RESET },
    { "Hard reset", MENU_ENTRY_RESOURCE_RADIO, radio_JAMAction_callback, (ui_callback_data_t)MACHINE_JAM_ACTION_HARD_RESET },
    { "Quit emulator", MENU_ENTRY_RESOURCE_RADIO, radio_JAMAction_callback, (ui_callback_data_t)MACHINE_JAM_ACTION_QUIT },
    SDL_MENU_LIST_END
};
