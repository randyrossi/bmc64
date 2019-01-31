/*
 * ui.h - user interface for BeOS
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#ifndef VICE_UI_BEOS_H
#define VICE_UI_BEOS_H

#include "types.h"
#include "uiapi.h"

/* Here some stuff for the connection of menuitems and resources */
typedef struct {
    /* Name of resource.  */
    const char *name;
    /* ID of the corresponding menu item.  */
    int item_id;
} ui_menu_toggle;

typedef struct {
    int value;
    int item_id; /* The last item_id has to be zero. */
} ui_res_possible_values;

typedef struct {
    const char *name;
    ui_res_possible_values *vals;
} ui_res_value_list;

typedef struct {
    const char *string;
    int item_id; /* The last item_id has to be zero. */
} ui_res_possible_strings;

typedef struct {
    const char *name;
    ui_res_possible_strings *strings;
} ui_res_string_list;

typedef struct { 
    int menu_item;
    int cart_type;
    const char *cart_name;
} ui_cartridge_t;

typedef void (*ui_machine_specific_t) (void* msg, void* window);

/*-------------------------------------------------------------------------*/

extern void ui_register_machine_specific(ui_machine_specific_t func);
extern void ui_register_menu_toggles(ui_menu_toggle *toggles);
extern void ui_register_res_values(ui_res_value_list *valuelist);
extern void ui_main(char hotkey);
extern void ui_set_warp_status(int status);
extern void ui_dispatch_events(void);
extern void ui_add_event(void *msg);
extern void ui_display_speed(float percent, float framerate, int warp_flag);
extern void ui_message(const char *format,...);
extern void ui_error(const char *format,...);
extern void ui_error_string(const char *text);
extern void ui_show_text(const char *caption, const char *header,const char *text);
extern void ui_update_menus(void);
extern void ui_enable_joyport(void);
extern void ui_statusbar_update(void);
extern int ui_set_window_mode(int use_direct_window);
extern void ui_pause_emulation(int flag);
extern int ui_emulation_is_paused(void);

extern int CheckForHaiku(void);

#endif
