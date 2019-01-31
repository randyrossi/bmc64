/*
 * kbd.h - MacVICE keyboard driver
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

#ifndef VICE_KBD_H
#define VICE_KBD_H

extern void kbd_arch_init(void);
extern int kbd_arch_get_host_mapping(void);

extern signed long kbd_arch_keyname_to_keynum(char *keyname);
extern const char *kbd_arch_keynum_to_keyname(signed long keynum);
extern void kbd_initialize_numpad_joykeys(int *joykeys);

/* FIXME: remove these */
#define KBD_C64_SYM_US  "osx_sym_us.vkm"
#define KBD_C64_SYM_DE  "osx_sym_de.vkm"
#define KBD_C64_POS     "osx_pos.vkm"
#define KBD_C128_SYM    "osx_sym.vkm"
#define KBD_C128_POS    "osx_pos.vkm"
#define KBD_VIC20_SYM   "osx_sym.vkm"
#define KBD_VIC20_POS   "osx_pos.vkm"
#define KBD_PET_SYM_UK  "osx_sym_uk.vkm"
#define KBD_PET_POS_UK  "osx_pos_uk.vkm"
#define KBD_PET_SYM_DE  "osx_sym_de.vkm"
#define KBD_PET_POS_DE  "osx_pos_de.vkm"
#define KBD_PET_SYM_GR  "osx_sym_gr.vkm"
#define KBD_PET_POS_GR  "osx_pos_gr.vkm"
#define KBD_PLUS4_SYM   "osx_sym.vkm"
#define KBD_PLUS4_POS   "osx_pos.vkm"
#define KBD_CBM2_SYM_UK "osx_sym_uk.vkm"
#define KBD_CBM2_POS_UK "osx_pos_uk.vkm"
#define KBD_CBM2_SYM_DE "osx_sym_de.vkm"
#define KBD_CBM2_POS_DE "osx_pos_de.vkm"
#define KBD_CBM2_SYM_GR "osx_sym_gr.vkm"
#define KBD_CBM2_POS_GR "osx_pos_gr.vkm"

#define KBD_INDEX_C64_DEFAULT   KBD_INDEX_C64_SYM
#define KBD_INDEX_C128_DEFAULT  KBD_INDEX_C128_SYM
#define KBD_INDEX_VIC20_DEFAULT KBD_INDEX_VIC20_SYM
#define KBD_INDEX_PET_DEFAULT   KBD_INDEX_PET_BUKP
#define KBD_INDEX_PLUS4_DEFAULT KBD_INDEX_PLUS4_SYM
#define KBD_INDEX_CBM2_DEFAULT  KBD_INDEX_CBM2_BUKP

#define KBD_PORT_PREFIX "osx"

#endif  /* _KBD_H */
