/*
 * petsound.c - implementation of PET sound code
 *
 * Written by
 *  Teemu Rantanen <tvr@cs.hut.fi>
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

#include "vice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "lib.h"
#include "machine.h"
#include "petsound.h"
#include "sid.h"
#include "sidcart.h"
#include "sid-resources.h"
#include "sound.h"
#include "types.h"

/* ------------------------------------------------------------------------- */

/* Some prototypes are needed */
static int pet_sound_machine_init(sound_t *psid, int speed, int cycles_per_sec);
static int pet_sound_machine_calculate_samples(sound_t **psid, int16_t *pbuf, int nr, int sound_output_channels, int sound_chip_channels, int *delta_t);
static void pet_sound_machine_store(sound_t *psid, uint16_t addr, uint8_t val);
static uint8_t pet_sound_machine_read(sound_t *psid, uint16_t addr);
static void pet_sound_reset(sound_t *psid, CLOCK cpu_clk);

static int pet_sound_machine_cycle_based(void)
{
    return 0;
}

static int pet_sound_machine_channels(void)
{
    return 1;
}

static sound_chip_t pet_sound_chip = {
    NULL, /* no open */
    pet_sound_machine_init,
    NULL, /* no close */
    pet_sound_machine_calculate_samples,
    pet_sound_machine_store,
    pet_sound_machine_read,
    pet_sound_reset,
    pet_sound_machine_cycle_based,
    pet_sound_machine_channels,
    1 /* chip enabled */
};

static uint16_t pet_sound_chip_offset = 0;

void pet_sound_chip_init(void)
{
    pet_sound_chip_offset = sound_chip_register(&pet_sound_chip);
}

/* ------------------------------------------------------------------------- */

/* dummy function for now */
int machine_sid2_check_range(unsigned int sid2_adr)
{
    return 0;
}

/* dummy function for now */
int machine_sid3_check_range(unsigned int sid3_adr)
{
    return 0;
}

/* dummy function for now */
int machine_sid4_check_range(unsigned int sid4_adr)
{
    return 0;
}

void machine_sid2_enable(int val)
{
}

struct pet_sound_s {
    int on;             /* are we even making sound? */
    CLOCK t;            /* clocks between shifts: (VIA timer T2 + 2) * 2 */
    uint8_t waveform;   /* value from the VIA Shift Register; msb first */

    double b;           /* SR bit# being output [0,8> (counting from left) */
    double bs;          /* SR bits per output sample */

    int speed;          /* sample rate * 100 / speed_percent */
    int cycles_per_sec;

    int manual;       /* 1 if CB2 set to manual control "high", 0 otherwise */
};

static struct pet_sound_s snd;

/* XXX: this used to be not correct; is a lot better now */
/*
 * This function averages several output bits from the Shift Register
 * into an output sample.
 * Because the shifting speed and the sample speed don't need to line up
 * nicely, this can involve fractional bit times at the start and the end.
 *
 * s: starting bit in the shift register (remember: counting from left)
 * e: ending bit in the SR.
 *
 * This doesn't work well if s and e are within the same bit time 
 * (which can happen if snd.bs < 1.0).
 */
static uint16_t pet_makesample_downsampled(double s, double e, uint8_t waveform)
{
    double v;
    int sc, sf, ef, i, nr;

    /* Determine whole-bit boundaries */
    sf = (int)floor(s);         /* start floor ("rounded down") */
    sc = sf + 1;                /* start ceiling ("rounded up"); floor+1 */
    ef = (int)floor(e);         /* end floor */
    nr = 0;

    /*
     * Count the value of the signal over whole bit-periods falling in
     * the interval.
     *
     * For example, a time line; | is when the next bit is shifted out:
     *
     *      <-----------snd.bs------------->
     *      s                               e
     * |----------|----------|----------|----------|....more...bits...
     * sf         sc                    ef
     *
     *            [-------------------->
     *            i          i
     */
    for (i = sc; i < ef; i++) {
        if (waveform & (0x80 >> (i % 8))) {
            nr++;
        }
    }

    v = nr;

    /*
     * Now add the signal during fractional bit times.
     * The bit at the start is (if set) sc - s wide.
     */
    if (waveform & (0x80 >> (sf % 8))) {
        v += sc - s;
    }
    /* And similar at the end. */
    if (waveform & (0x80 >> (ef % 8))) {
        v += e - ef;
    }
    /*
     * The method of counting fractional bits above doesn't work if s and e
     * fall within the same bit time:
     * - sc-s is more than the time period we're processing
     * - e-ef is also more than the time period we're processing
     *
     *      <-----------snd.bs------------->
     *      s                               e
     * |-------------------------------------------|....more...bits...
     * sf                                          sc
     * ef
     * 
     * so for that case the contribution of the bit would be
     *
     *     if (waveform & (0x80 >> (sf % 8))) {
     *        v += e - s;
     *     }
     *
     * Since nr == 0 for this case, the final result is always 0 or 4095.
     * pet_makesample_exact() calculates the same thing much simpler.
     *
     * Now that we are only called when snd.bs > 1.0 this case cannot happen
     * any more.
     */

    /*
     * Average over the whole period (e - s) and scale
     * to a range of 0 ... 4095.
     */
    return (uint16_t)(v * 4095.0 / (e - s));
}

static uint16_t pet_makesample_exact(uint8_t bit, uint8_t waveform)
{
    uint8_t output_bit = waveform & (0x80 >> bit);

    if (output_bit) {
        return 4095;
    } else {
        return 0;
    }
}

static int pet_sound_machine_calculate_samples(sound_t **psid, int16_t *pbuf, int nr, int soc, int scc, int *delta_t)
{
    int i;
    uint16_t v = 0;

    for (i = 0; i < nr; i++) {
        if (snd.on) {
            if (snd.bs <= 1.0) {
                v = pet_makesample_exact(snd.b, snd.waveform);
            } else {
                v = pet_makesample_downsampled(snd.b, snd.b + snd.bs, snd.waveform);
            }
        } else if (snd.manual) {
            v = 4095;
        }

        pbuf[i * soc] = sound_audio_mix(pbuf[i * soc], (int16_t)v);
        if (soc > 1) {
            pbuf[(i * soc) + 1] = sound_audio_mix(pbuf[(i * soc) + 1], (int16_t)v);
        }

        snd.b += snd.bs;
        while (snd.b >= 8.0) {
            snd.b -= 8.0;
        }
    }
    return nr;
}

#define PETSOUND_ONOFF          0
#define PETSOUND_WAVEFORM       1
#define PETSOUND_RATE_LO        2
#define PETSOUND_RATE_HI        3
#define PETSOUND_MANUAL         4

void petsound_store_onoff(int value)
{
    sound_store(pet_sound_chip_offset | PETSOUND_ONOFF, value, 0);
}

void petsound_store_rate(CLOCK t)
{
    uint8_t lo = (uint8_t)(t & 0xff);
    uint8_t hi = (uint8_t)((t >> 8) & 0xff);
    sound_store((uint16_t)(pet_sound_chip_offset | PETSOUND_RATE_LO), lo, 0);
    sound_store((uint16_t)(pet_sound_chip_offset | PETSOUND_RATE_HI), hi, 0);
}

void petsound_store_waveform(uint8_t waveform)
{
    sound_store((uint16_t)(pet_sound_chip_offset | PETSOUND_WAVEFORM),
                waveform, 0);
}

/* For manual control of CB2 sound using $E84C */
void petsound_store_manual(int value)
{
    sound_store((uint16_t)(pet_sound_chip_offset | PETSOUND_MANUAL),
                (uint8_t)value, 0);
}

static void pet_sound_machine_store(sound_t *psid, uint16_t addr, uint8_t val)
{
    switch (addr) {
        case PETSOUND_ONOFF:
            snd.on = val;
            break;
        case PETSOUND_WAVEFORM:
            snd.waveform = val;
            while (snd.b >= 1.0) {
                snd.b -= 1.0;
            }
            break;
        case PETSOUND_RATE_LO:
            snd.t = val;
            break;
        case PETSOUND_RATE_HI:
            snd.t = (snd.t & 0xff) | (val << 8);
            snd.bs = (double)snd.cycles_per_sec / (snd.t * snd.speed);
            break;
        case PETSOUND_MANUAL:
            snd.manual = val;
            break;
        default:
            abort();
    }
}

static int pet_sound_machine_init(sound_t *psid, int speed, int cycles_per_sec)
{
    snd.speed = speed;
    snd.cycles_per_sec = cycles_per_sec;
    snd.b = 0.0;
    petsound_store_rate(32);

    return 1;
}

static void pet_sound_reset(sound_t *psid, CLOCK cpu_clk)
{
    petsound_store_onoff(0);
}

void petsound_reset(sound_t *psid, CLOCK cpu_clk)
{
    sound_reset();
}

static uint8_t pet_sound_machine_read(sound_t *psid, uint16_t addr)
{
    return 0;
}

void sound_machine_prevent_clk_overflow(sound_t *psid, CLOCK sub)
{
    sid_sound_machine_prevent_clk_overflow(psid, sub);
}

char *sound_machine_dump_state(sound_t *psid)
{
    return sid_sound_machine_dump_state(psid);
}

void sound_machine_enable(int enable)
{
    sid_sound_machine_enable(enable);
}
