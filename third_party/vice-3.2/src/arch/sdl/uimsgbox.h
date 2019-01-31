/*
 * uimsgbox.h - Common SDL message box functions.
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

#ifndef VICE_UIMSGBOX_H
#define VICE_UIMSGBOX_H

#include "vice.h"
#include "types.h"

/** \brief  Message mode enumerator for message_box()
 *
 * When adding or removing a value, please don't forget to update the
 * `msg_mode_buttons` table in uimsg.c accordingly.
 */
enum {
    MESSAGE_OK = 0,         /**< simple OK dialog */
    MESSAGE_YESNO,          /**< YES/NO selection */
    MESSAGE_CPUJAM,         /**< CPU-jam dialog */
    MESSAGE_UNIT_SELECT     /**< Drive unit number selection dialog */
};


extern int message_box(const char *title, char *message, int message_mode);

#endif
