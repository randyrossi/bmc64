/*
 * grabkey.c - Simple key grabber for MS-DOS.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <stdio.h>

#include "kbd.h"

/* Segment info for the standard keyboard handler.  */
static _go32_dpmi_seginfo std_kbd_handler_seginfo;

/* Grabbed key.  K_NONE = none yet.  */
static volatile kbd_code_t grabbed_key;

/* Temporary grabbed key used by interrupt handler to filter spurious
   keypress/keyreleases.  */
static volatile kbd_code_t tmp_grabbed_key;

static void my_kbd_interrupt_handler(void)
{
    static int extended = 0;	/* Extended key count.  */
    static int skip_count = 0;
    unsigned int kcode;

    kcode = inportb(0x60);

    if (skip_count > 0) {
        skip_count--;
        outportb(0x20, 0x20);
        return;
    } else if (kcode == 0xe0) {
        /* Extended key: at the next interrupt we'll get its extended keycode
           or 0xe0 again.  */
        extended++;
        outportb(0x20, 0x20);
        return;
    } else if (kcode == 0xe1) {
        /* Damn Pause key.  It sends 0xe1 0x1d 0x52 0xe1 0x9d 0xd2.  This is
           awesome, but at least we know it's the only sequence starting by
           0xe1, so we can just skip the next 5 codes.  Btw, there is no
           release code.  */
        skip_count = 5;
        kcode = K_PAUSE;
    }

    if (!(kcode & 0x80)) {	/* Key pressed.  */

        /* Derive the extended keycode.  */
        if (extended == 1) {
            kcode = _kbd_extended_key_tab[kcode];
        }

        /* Grab only one key.  */
        if (tmp_grabbed_key == K_NONE) {
            tmp_grabbed_key = (kbd_code_t) kcode;
            if (tmp_grabbed_key == K_PAUSE) {
                /* Pause is never released.  */
                grabbed_key = K_PAUSE;
            }
        }

    } else {			/* Key released.  */

        /* Remove release bit.  */
        kcode &= 0x7F;

        /* Derive the extended keycode.  */
        if (extended == 1) {
            kcode = _kbd_extended_key_tab[kcode];
        }

        if (tmp_grabbed_key == (kbd_code_t) kcode) {
            grabbed_key =  tmp_grabbed_key;
        }
    }

    extended = 0;
    outportb(0x20, 0x20);
}

/* FIXME: This routine is not used.
static void my_kbd_interrupt_handler_end() { }

static void lock_stuff(void)
{
    _go32_dpmi_lock_code(my_kbd_interrupt_handler, (unsigned long)my_kbd_interrupt_handler_end - (unsigned long)my_kbd_interrupt_handler);
    _go32_dpmi_lock_data(_kbd_extended_key_tab, sizeof _kbd_extended_key_tab);
    _go32_dpmi_lock_data(&std_kbd_handler_seginfo, sizeof std_kbd_handler_seginfo);
    _go32_dpmi_lock_data((void *) &grabbed_key, sizeof grabbed_key);
    _go32_dpmi_lock_data((void *) &tmp_grabbed_key, sizeof tmp_grabbed_key);
}
*/

static int install_kbd_handler(void)
{
    static _go32_dpmi_seginfo my_kbd_handler_seginfo;
    int r;

    _go32_dpmi_get_protected_mode_interrupt_vector(9, &std_kbd_handler_seginfo);
    my_kbd_handler_seginfo.pm_offset = (int) my_kbd_interrupt_handler;
    my_kbd_handler_seginfo.pm_selector = _go32_my_cs();
    r = _go32_dpmi_allocate_iret_wrapper(&my_kbd_handler_seginfo);
    if (r) {
        return -1;
    }
    r = _go32_dpmi_set_protected_mode_interrupt_vector(9, &my_kbd_handler_seginfo);
    if (r) {
        return -1;
    }

    return 0;
}

static int uninstall_kbd_handler(void)
{
    int r;

    r = _go32_dpmi_set_protected_mode_interrupt_vector(9, &std_kbd_handler_seginfo);
    if (r) {
        return -1;
    }

    return 0;
}

kbd_code_t grab_key(void)
{
    grabbed_key = tmp_grabbed_key = K_NONE;

    if (install_kbd_handler() < 0) {
        return K_NONE;
    }

    while (grabbed_key == K_NONE) {
    }

    uninstall_kbd_handler();
    return grabbed_key;
}
