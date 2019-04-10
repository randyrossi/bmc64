/*
 * soundraspi.c - Bare metal raspberry pi sound driver
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

#include "vice.h"

#include "debug.h"
#include "log.h"
#include "sound.h"

extern int circle_sound_init(const char *param, int *speed, 
                             int *fragsize, int *fragnr, int *channels);
extern int circle_sound_write(int16_t *pbuf, size_t nr);
extern void circle_sound_close(void);
extern int circle_sound_suspend(void);
extern int circle_sound_resume(void);
extern unsigned int circle_sound_bufferspace(void);

static int raspi_init(const char *param, int *speed, int *fragsize, int *fragnr, int *channels)
{
    return circle_sound_init(param, speed, fragsize, fragnr, channels);
}

static int raspi_write(int16_t *pbuf, size_t nr)
{
    return circle_sound_write(pbuf, nr);
}

static void raspi_close(void)
{
   circle_sound_close();
}

static int raspi_suspend(void)
{
   return circle_sound_suspend();
}

static int raspi_resume(void)
{
   return circle_sound_resume();
}

static int raspi_bufferspace(void) {
  return circle_sound_bufferspace();
}

static sound_device_t raspi_device =
{
    "raspi",
    raspi_init,
    raspi_write,
    NULL,
    NULL,
    raspi_bufferspace,
    raspi_close,
    raspi_suspend,
    raspi_resume,
    1,
    2
};

int sound_init_raspi_device(void)
{
    return sound_register_device(&raspi_device);
}
