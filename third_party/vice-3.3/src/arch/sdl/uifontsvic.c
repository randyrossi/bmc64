/*
 * uifontsvic.c - generate fonts used by the xvic ui
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
#include "sysfile.h"
#include "uifonts.h"
#include "uimenu.h"

#define VICFONTSIZE    (256 * 8)

static unsigned char *vicfontasc = NULL;
static unsigned char *vicfontpet = NULL;
static unsigned char *vicfontmon = NULL;

/* FIXME: tweak to match the appearence of the other chars */
static unsigned char vicextrachars[8 * 4] = {
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

    for(i = 0; i < VICFONTSIZE; i+=2) {
        vicfontasc[i] = 0x55;
        vicfontasc[i + 1] = 0xaa;
    }
    memcpy(vicfontpet, vicfontasc, VICFONTSIZE);
    memcpy(vicfontmon, vicfontasc, VICFONTSIZE);

    if((f = fopen(name, "rb")) == NULL) {
        return -1;
    }
    fseek(f, 0, SEEK_SET);
    if (fread(vicfontpet, 1, 0x400, f) != 0x400) {
        res = -1;
    }
    fseek(f, 0x800, SEEK_SET);
    if (fread(vicfontmon, 1, 0x400, f) != 0x400) {
        res = -1;
    }
    fseek(f, 0x800, SEEK_SET);
    if (fread(vicfontasc, 1, 0x400 - (8 * 37), f) != (0x400 - (8 * 37))) {
        res = -1;
    }
    fclose(f);

    /* create inverted second half */
    for(i = 0; i < 0x400; i++) {
        vicfontpet[i + 0x400] = vicfontpet[i] ^ 0xff;
        vicfontmon[i + 0x400] = vicfontmon[i] ^ 0xff;
        /* vicfontasc[i + 0x400] = vicfontasc[i] ^ 0xff; */
    }
    return res;
}

int sdl_ui_vic_font_init(void)
{
    int i;
    char *name = "chargen";
    char *path = NULL;

    if (sysfile_locate(name, &path) != 0) {
           log_error(LOG_DEFAULT, "could not locate menu charset `%s'.\n", name ? name : "(null)");
           return -1;
    }

    vicfontasc = lib_malloc(VICFONTSIZE);
    vicfontpet = lib_malloc(VICFONTSIZE);
    vicfontmon = lib_malloc(VICFONTSIZE);

    if (loadchar(path) != 0) {
        log_error(LOG_DEFAULT, "could not load menu charset `%s'.\n", name ? name : "(null)");
    }
    lib_free(path);
    
    /* 1f copy _ to arrow-left */
    memset(&vicfontasc[0x1f * 8], 0, 8);
    vicfontasc[0x1f * 8 + 6] = 0xff;
    /* 1e fix ^ */
    for (i = 3; i < 8; i++) {
        vicfontasc[0x1e * 8 + i] &= ~0x18;
    }
    /* 1c fix \ */
    for (i = 0; i < 7; i++) {
        vicfontasc[0x1c * 8 + i] = vicfontasc[0x2f * 8 + (7 - i)];
    }
    /* 40 fix ´ */
    for (i = 0; i < 3; i++) {
        vicfontasc[0x40 * 8 + i] = vicfontasc[0x27 * 8 + (3 - i)];
        vicfontasc[0x40 * 8 + i + 3] = 0;
    }
    /* fix 7b..7f */
    memcpy(&vicfontasc[0x5b * 8], &vicextrachars[0], 8 * 4);

    /* TODO: add extended ascii codes 80-ff */

    /* extra menu gfx goes into unprintable codes 0..1f */
    sdl_ui_copy_ui_font(&vicfontasc[0x81 * 8]);

    /* init menu font last, since set_menu_font will also make the font active */
    sdl_ui_set_monitor_font(vicfontmon, 8, 8);
    sdl_ui_set_image_font(vicfontpet, 8, 8);
    sdl_ui_set_menu_font(vicfontasc, 8, 8);

    return 0;
}

void sdl_ui_vic_font_shutdown(void)
{
    lib_free(vicfontasc);
    vicfontasc = NULL;
    lib_free(vicfontpet);
    vicfontpet = NULL;
    lib_free(vicfontmon);
    vicfontmon = NULL;
}
