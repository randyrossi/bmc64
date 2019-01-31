/*
 * qnxshm.h
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

#ifndef VICE_QNXSHM_H
#define VICE_QNXSHM_H

#include "qnxipc.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define SHM_R 0400
#define SHM_W 0200

struct shmid_ds {
    int dummy;
    int shm_nattch;
};

extern void *shmat(int shmid, const void *shmaddr, int shmflg);
extern int shmdt(const void *addr);
extern int shmctl(int shmid, int cmd, struct shmid_ds * buf);
extern int shmget(key_t key, size_t size, int flags);

#ifdef __cplusplus
}
#endif

#endif
