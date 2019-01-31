/*
 * tuicharset.c - Setup custom character set.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include <dos.h>
#include <dpmi.h>
#include <stdio.h>
#include <sys/movedata.h>

#include "cbmcharsets.h"
#include "log.h"
#include "tuicharset.h"

static int charset_initialized = 0;

struct {
    int segment;
    int selector;
} cbm_charset_mem[2];

int tui_charset_init(void)
{
    int i;

    if (charset_initialized) {
        return 0;
    }

    /* Allocate 256 paragraphs (4096 bytes) for each set.  */
    for (i = 0; i < 2; i++) {
        cbm_charset_mem[i].segment = __dpmi_allocate_dos_memory(256, &cbm_charset_mem[i].selector);
        if (cbm_charset_mem[i].segment < 0) {
            log_error(LOG_DEFAULT, "Cannot allocate 256 paragraphs of DOS memory for the CBM charset #%d.", i + 1);
            return -1;
        }
    }

    dosmemput(cbm_charset_1, 4096, cbm_charset_mem[0].segment * 16);
    dosmemput(cbm_charset_2, 4096, cbm_charset_mem[1].segment * 16);

    charset_initialized = 1;

    log_message(LOG_DEFAULT, "Prepared custom CBM character sets.");

    return 0;
}

static void load_charset(int segment)
{
    __dpmi_regs r;

    r.x.ax = 0x1110;
    r.x.bx = 16 << 8;
    r.x.cx = 256;
    r.x.dx = 0x0000;
    r.x.es = segment;
    r.x.bp = 0;
    __dpmi_int(0x10, &r);
}

int tui_charset_set(tui_charset_t which)
{
    tui_charset_init();

    switch (which) {
        case TUI_CHARSET_DEFAULT:
            log_debug("Default charset not available.");
            return -1;
        case TUI_CHARSET_CBM_1:
            load_charset(cbm_charset_mem[0].segment);
            return 0;
        case TUI_CHARSET_CBM_2:
            load_charset(cbm_charset_mem[1].segment);
            return 0;
        default:
            log_debug("Unknown character set %d.", which);
            return -1;
    }
}
