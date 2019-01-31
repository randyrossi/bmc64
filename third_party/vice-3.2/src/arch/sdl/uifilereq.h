/*
 * uifilereq.h - Common SDL file selection dialog functions.
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

#ifndef VICE_UIFILEREQ_H
#define VICE_UIFILEREQ_H

#include "vice.h"
#include "types.h"

typedef enum {
    FILEREQ_MODE_CHOOSE_FILE = 0,
    FILEREQ_MODE_SAVE_FILE,
    FILEREQ_MODE_CHOOSE_DIR
} ui_menu_filereq_mode_t;

extern char *sdl_ui_file_selection_dialog(const char* title, ui_menu_filereq_mode_t mode);
extern void sdl_ui_file_selection_dialog_shutdown(void);

typedef enum {
    SLOTREQ_MODE_CHOOSE_SLOT = 0,
    SLOTREQ_MODE_SAVE_SLOT,
} ui_menu_slot_mode_t;

extern char *sdl_ui_slot_selection_dialog(const char* title, ui_menu_slot_mode_t mode);

typedef struct {
    int used;
    char *slot_name;
    char *slot_string;
} ui_menu_slot_entry;

typedef struct {
    int number_of_elements;
    ui_menu_slot_entry *entries;
} ui_menu_slots;

#endif
