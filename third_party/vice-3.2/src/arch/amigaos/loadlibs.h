/*
 * loadlibs.h
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

#ifndef VICE_LOADLIBS_H
#define VICE_LOADLIBS_H

extern int load_libs(void);
extern void close_libs(void);

extern int pci_lib_loaded;

#ifdef AMIGA_OS4
extern int amigainput_lib_loaded;
#else
extern int lowlevel_lib_loaded;
#endif

#if (defined(AMIGA_M68K) && defined(HAVE_PROTO_CYBERGRAPHICS_H)) || defined(AMIGA_MORPHOS) || defined(AMIGA_AROS)
#ifdef HAVE_XVIDEO
extern int xvideo_lib_loaded;
#endif
#endif

#if defined(HAVE_PROTO_OPENPCI_H) || defined(AMIGA_OS4)
extern int pci_lib_loaded;
#endif

#endif
