/*
 * demo.h
 *
 * Written by
 *  Randy Rossi <randy.rossi@gmail.com>
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

#ifndef RASPI_DEMO_H
#define RASPI_DEMO_H

extern int raspi_demo_mode;

#define OP_RESET 0
#define OP_SNAPSHOT 1
#define OP_DISKSWAP 2
#define OP_CARTRIDGE 3
#define OP_DETACH_CART 4
#define OP_DETACH_DISK 5

#define MODE_INTERRUPTABLE 0
#define MODE_UNINTERRUPTABLE 1

struct demo_entry {
  int operation;
  char file[256];
  long timeout;
  int mode;
  int joyswap;

  struct demo_entry *next;
};

void demo_check(void);
void demo_reset_timeout(void);
void demo_reset(void);

#endif
