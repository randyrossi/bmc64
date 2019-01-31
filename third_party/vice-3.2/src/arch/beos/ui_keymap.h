/*
 * ui_keymap.h - Keymap settings dialog
 *
 * Written by
 *   Bas Wassink <b.wassink@ziggo.nl>
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

#ifndef VICE_UI_KEYMAP_H
#define VICE_UI_KEYMAP_H

// probably not required anymore
const uint32 MESSAGE_KEYMAP_SETTINGS_APPLY  = 'MKM1';

/** \brief  Message in case the keymapping radio button index changed
 *
 * An 'index' int32 is set to the index of the radion button
 */
const uint32 MESSAGE_KEYMAP_INDEX_CHANGED   = 'MKM2';

/**\brief   Message on pressing 'Browse ...' in the keymapping box
 *
 * A 'keymap_type' int32 is set to either KBD_INDEX_USERSYM or KBD_USERPOS
 * \see src/keyboard.h)
 */
const uint32 MESSAGE_KEYMAP_BROWSE          = 'MKM3';

/** \brief  Message on changing the text in the user's keymap file entries
 *
 * A 'keymap_type' int32 is set to either KBD_INDEX_USERSYM or KBD_USERPOS
 * \see src/keyboard.h)
 */
const uint32 MESSAGE_KEYMAP_TEXT_CHANGED    = 'MKM4';


const uint32 MESSAGE_KEYMAP_BROWSE_DONE     = 'MKM5';

const uint32 MESSAGE_KEYBOARD_TYPE_CHANGED = 'MKB1';

const uint32 MESSAGE_KEYBOARD_MAPPING_CHANGED = 'MKB2';

extern void ui_keymap(void);

#endif

