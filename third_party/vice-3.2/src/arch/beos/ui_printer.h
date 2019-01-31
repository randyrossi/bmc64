/*
 * ui_printer.h - Printer settings
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

#ifndef VICE__UI_PRINTER_H__
#define VICE__UI_PRINTER_H__

enum ui_printer_caps {
    NO_PRINTER_CAPS = 0,
    HAS_USERPORT_PRINTER = (1 << 0),
    HAS_IEC_BUS = (1 << 1)
};

const uint32 MESSAGE_PRINTER_EMULATION     = 'MPR1';
const uint32 MESSAGE_PRINTER_DRIVER        = 'MPR2';
const uint32 MESSAGE_PRINTER_OUTPUT_TYPE   = 'MPR3';
const uint32 MESSAGE_PRINTER_OUTPUT_DEVICE = 'MPR4';
const uint32 MESSAGE_PRINTER_SEND_FF       = 'MPR5';
const uint32 MESSAGE_PRINTER_IEC_DEVICE    = 'MPR6';
const uint32 MESSAGE_PRINTER_DEVICE_FILE   = 'MPR7';

extern void ui_printer(int caps);

#endif
