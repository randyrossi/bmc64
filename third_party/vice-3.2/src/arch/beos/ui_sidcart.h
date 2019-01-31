/*
 * ui_sidcart.h - SID cart settings
 *
 * Written by
 *  Marcus Sutton <loggedoubt@gmail.com>
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

#ifndef VICE__UI_SIDCART_H__
#define VICE__UI_SIDCART_H__

const uint32 MESSAGE_SIDCART_ENABLE        = 'MSC1';
const uint32 MESSAGE_SIDCART_MODEL         = 'MSC2';
const uint32 MESSAGE_SIDCART_ADDRESS       = 'MSC3';
const uint32 MESSAGE_SIDCART_CLOCK         = 'MSC4';
const uint32 MESSAGE_SIDCART_FILTERS       = 'MSC5';
const uint32 MESSAGE_SIDCART_RESIDSAMPLING = 'MSC6';
const uint32 MESSAGE_SIDCART_RESIDSLIDER   = 'MSC7';
const uint32 MESSAGE_SIDCART_DIGIBLASTER   = 'MSC8';

extern void ui_sidcart(const char **cartaddresstextpair, const char **cartclockpair, const int *cartaddressintpair);

#endif
