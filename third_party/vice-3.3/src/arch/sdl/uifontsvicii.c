/*
 * uifontsvicii.c - generate fonts used by the x64(sc)/xscpu64/x64dtv/vsid uis
 *
 * Written by
 *  groepaz <groepaz@gmx.net>
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

#include "lib.h"
#include "log.h"
#include "machine.h"
#include "sysfile.h"
#include "uifonts.h"
#include "uimenu.h"

#define VICIIFONTSIZE    (256 * 8)

static unsigned char *viciifontasc = NULL;
static unsigned char *viciifontpet = NULL;
static unsigned char *viciifontmon = NULL;

/* FIXME: tweak to match the appearence of the other chars */
static unsigned char viciiextrachars[8 * 4] = {
    0x0c, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0c, 0x00, /* { */
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, /* | */
    0x30, 0x18, 0x18, 0x0c, 0x18, 0x18, 0x30, 0x00, /* } */
    0x60, 0x92, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, /* ~ */
};

static int loadchar(char *name)
{
    FILE *f;
    unsigned int i;
    int res = 0;

    for(i = 0; i < VICIIFONTSIZE; i+=2) {
        viciifontasc[i] = 0x55;
        viciifontasc[i + 1] = 0xaa;
    }
    memcpy(viciifontpet, viciifontasc, VICIIFONTSIZE);
    memcpy(viciifontmon, viciifontasc, VICIIFONTSIZE);

    if((f = fopen(name, "rb")) == NULL) {
        return -1;
    }
    fseek(f, 0, SEEK_SET);
    if (fread(viciifontpet, 1, 0x400, f) != 0x400) {
        res = -1;
    }
    fseek(f, 0x800, SEEK_SET);
    if (fread(viciifontmon, 1, 0x400, f) != 0x400) {
        res = -1;
    }
    fseek(f, 0x800, SEEK_SET);
    if (fread(viciifontasc, 1, 0x400 - (8 * 37), f) != (0x400 - (8 * 37))) {
        res = -1;
    }
    fclose(f);

    /* create inverted second half */
    for(i = 0; i < 0x400; i++) {
        viciifontpet[i + 0x400] = viciifontpet[i] ^ 0xff;
        viciifontmon[i + 0x400] = viciifontmon[i] ^ 0xff;
        /* viciifontasc[i + 0x400] = viciifontasc[i] ^ 0xff; */
    }
    return res;
}

int sdl_ui_vicii_font_init(void)
{
    int i;
    char *name = (machine_class == VICE_MACHINE_CBM5x0) ? "chargen.500" : "chargen";
    char *path = NULL;

    if (sysfile_locate(name, &path) != 0) {
           log_error(LOG_DEFAULT, "could not locate menu charset `%s'.\n", name ? name : "(null)");
           return -1;
    }

    viciifontasc = lib_malloc(VICIIFONTSIZE);
    viciifontpet = lib_malloc(VICIIFONTSIZE);
    viciifontmon = lib_malloc(VICIIFONTSIZE);

    if (loadchar(path) != 0) {
        log_error(LOG_DEFAULT, "could not load menu charset `%s'.\n", name ? name : "(null)");
    }
    lib_free(path);

    /* 1f copy _ to arrow-left */
    memset(&viciifontasc[0x1f * 8], 0, 8);
    viciifontasc[0x1f * 8 + 6] = 0xff;
    /* 1e fix ^ */
    for (i = 3; i < 8; i++) {
        viciifontasc[0x1e * 8 + i] &= ~0x18;
    }
    /* 1c fix \ */
    for (i = 0; i < 7; i++) {
        viciifontasc[0x1c * 8 + i] = viciifontasc[0x2f * 8 + (7 - i)];
    }
    /* 40 fix ´ */
    for (i = 0; i < 3; i++) {
        viciifontasc[0x40 * 8 + i] = viciifontasc[0x27 * 8 + (3 - i)];
        viciifontasc[0x40 * 8 + i + 3] = 0;
    }
    /* fix 7b..7f */
    memcpy(&viciifontasc[0x5b * 8], &viciiextrachars[0], 8 * 4);

    /* TODO: add extended ascii codes 80-ff */

    /* extra menu gfx goes into unprintable codes 0..1f */
    sdl_ui_copy_ui_font(&viciifontasc[0x81 * 8]);

    /* init menu font last, since set_menu_font will also make the font active */
    sdl_ui_set_image_font(viciifontpet, 8, 8);
    sdl_ui_set_monitor_font(viciifontmon, 8, 8);
    sdl_ui_set_menu_font(viciifontasc, 8, 8);

    return 0;
}

void sdl_ui_vicii_font_shutdown(void)
{
    lib_free(viciifontasc);
    viciifontasc = NULL;
    lib_free(viciifontpet);
    viciifontpet = NULL;
    lib_free(viciifontmon);
    viciifontmon = NULL;
}
