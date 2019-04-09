/** \file   kbd.h
 * \brief   Native GTK3 specfic keyboard driver - header
 *
 * \author  Marco van den Heuvel <blackystardust68@yahoo.com>
 */

/*
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README file for copyright notice.
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

#ifndef VICE_KBD_H
#define VICE_KBD_H

#include <gtk/gtk.h>


/** \brief  Gtk3 keyboard shortcut to be used without Gtk3's accelerators
 *
 * In Gtk3 there is no normal method to create a keyboard shortcut without
 * it being connected to a G(tk)MenuItem. This object "solves" this problem
 * by having the kbd_event_handler() function scan a list of these objects
 * and trigger a callback when a specific key press matches.
 */
typedef struct kbd_gtk3_hotkey_s {
    guint code;                 /**< key code */
    guint mask;                 /**< key mask bits */
    void (*callback)(void);     /**< function to call when the key matches */
} kbd_gtk3_hotkey_t;


void kbd_arch_init(void);
void kbd_arch_shutdown(void);
int kbd_arch_get_host_mapping(void);
void kbd_initialize_numpad_joykeys(int *joykeys);
void kbd_connect_handlers(GtkWidget *widget, void *data);

#define KBD_PORT_PREFIX "gtk3"

/* add more function prototypes as needed below */

signed long kbd_arch_keyname_to_keynum(char *keyname);
const char *kbd_arch_keynum_to_keyname(signed long keynum);

void kbd_hotkey_init(void);
void kbd_hotkey_shutdown(void);
gboolean kbd_hotkey_add(guint code, guint mask, void (*callback)(void));
gboolean kbd_hotkey_add_list(kbd_gtk3_hotkey_t *list);

#endif
