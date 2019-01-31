/*
 * ps-unix.h - Unix PARallel port SID support wrapper.
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

#ifndef VICE_PS_UNIX_H
#define VICE_PS_UNIX_H

#include "types.h"

extern void ps_file_out_ctr(uint8_t parsid_ctrport, int chipno);
extern void ps_io_out_ctr(uint8_t parsid_ctrport, int chipno);
extern void ps_ieee1284_out_ctr(uint8_t parsid_ctrport, int chipno);

extern uint8_t ps_file_in_ctr(int chipno);
extern uint8_t ps_io_in_ctr(int chipno);
extern uint8_t ps_ieee1284_in_ctr(int chipno);

extern int ps_file_open(void);
extern int ps_io_open(void);
extern int ps_ieee1284_open(void);

extern int ps_file_close(void);
extern int ps_io_close(void);
extern int ps_ieee1284_close(void);

extern uint8_t ps_file_in_data(int chipno);
extern uint8_t ps_io_in_data(int chipno);
extern uint8_t ps_ieee1284_in_data(int chipno);

extern void ps_file_out_data(uint8_t addr, int chipno);
extern void ps_io_out_data(uint8_t addr, int chipno);
extern void ps_ieee1284_out_data(uint8_t addr, int chipno);

extern int ps_file_available(void);
extern int ps_io_available(void);
extern int ps_ieee1284_available(void);

#endif
