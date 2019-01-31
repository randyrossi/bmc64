/*
 * ahi.h
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#ifndef VICE_AHI_H_
#define VICE_AHI_H_

/* 8bit */
typedef signed char s8;
typedef unsigned char u8;

/* 16bit */
typedef signed short s16;
typedef unsigned short u16;

/* 32bit */
typedef signed long s32;
typedef unsigned long u32;
typedef float f32;

/* 64bit */
typedef signed long long s64;
typedef unsigned long long u64;
typedef double f64;

/* format */
#define AUDIO_MODE_8BIT   (0x00) /*  8 bit */
#define AUDIO_MODE_16BIT  (0x01) /* 16 bit */
#define AUDIO_MODE_MONO   (0x00) /*   mono */
#define AUDIO_MODE_STEREO (0x02) /* stereo */

/* format helpers */
#define AUDIO_M8S  (AUDIO_MODE_MONO | AUDIO_MODE_8BIT )   /*  8 bit signed, mono */
#define AUDIO_M16S (AUDIO_MODE_MONO | AUDIO_MODE_16BIT)   /* 16 bit signed, mono */
#define AUDIO_S8S  (AUDIO_MODE_STEREO | AUDIO_MODE_8BIT ) /*  8 bit signed, stereo */
#define AUDIO_S16S (AUDIO_MODE_STEREO | AUDIO_MODE_16BIT) /* 16 bit signed, stereo */

/* time */
#define TIMEBASE (1000000) /* us */
#define NOTIME   ((s64) - 1) /* when no timestamp is available */
#define NOWAIT   (0)
#define DOWAIT   (1)

/* sample functions */
extern s32 ahi_open(s32 frequency, u32 mode, s32 fragsize, s32 frags, void (*callback)(s64 time));
extern s32 ahi_samples_to_bytes(s32 samples);
extern s32 ahi_bytes_to_samples(s32 bytes);
extern void ahi_play_samples(void *data, s32 samples, s64 time, s32 wait);
extern s32 ahi_samples_buffered(void);
extern s32 ahi_samples_free(void);
extern void ahi_pause(void);
extern void ahi_close(void);

/* byte functions */
#define ahi_play_bytes(data, bytes, time, wait) ahi_play_samples(data, ahi_bytes_to_samples(bytes), time, wait)
#define ahi_bytes_buffered() ahi_samples_to_bytes(ahi_samples_buffered())
#define ahi_bytes_free() ahi_samples_to_bytes(ahi_samples_free())

#endif /* _AHI_H_ */
