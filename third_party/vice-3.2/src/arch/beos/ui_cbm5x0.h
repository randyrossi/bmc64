/*
 * ui_cbm5x0.h - CBM5x0 settings
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

#ifndef VICE__UI_CBM5X0_H__
#define VICE__UI_CBM5X0_H__

const uint32 MESSAGE_CBM5X0_MODELLINE	= 'MCC2';
const uint32 MESSAGE_CBM5X0_MEMORY = 'MCC3';
const uint32 MESSAGE_CBM5X0_RAMBANK = 'MCC4';

extern void ui_cbm5x0(void);

#endif
