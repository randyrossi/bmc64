/*
 * wininpoutp.h - I/O inp/outp prototypes/includes.
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

#ifndef VICE_WININPOUTP_H
#define VICE_WININPOUTP_H

extern int __cdecl _inp(unsigned short);
extern unsigned short __cdecl _inpw(unsigned short);
extern unsigned long __cdecl _inpd(unsigned short);
extern int __cdecl _outp(unsigned short, int);
extern unsigned short __cdecl _outpw(unsigned short, unsigned short);
extern unsigned long __cdecl _outpd(unsigned short, unsigned long);

#endif
