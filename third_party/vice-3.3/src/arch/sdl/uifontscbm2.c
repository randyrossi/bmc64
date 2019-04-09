/*
 * uifontscbm2.c - generate fonts used by the xcbm2 ui
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

#define CBM2FONTSIZE    (256 * 8)

static unsigned char *cbm2fontasc = NULL;
static unsigned char *cbm2fontpet = NULL;
static unsigned char *cbm2fontmon = NULL;

/* FIXME: tweak to match the appearence of the other chars */
static unsigned char cbm2extrachars[8 * 4] = {
    0x0c, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0c, 0x00, /* { */
    0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, /* | */
    0x30, 0x18, 0x18, 0x0c, 0x18, 0x18, 0x30, 0x00, /* } */
    0x60, 0x92, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, /* ~ */
};

#if 0
    if (model == 0) {
        menu_draw->extra_y = 8;
        for (i = 0; i < 256; i++) {
            for (j = 0; j < 14; j++) {
                cbm2_font_14[(i * 14) + j] = mem_chargen_rom[(i * 16) + j + 1];
            }
        }
        for (i = 0; i < 128; i++) {
            for (j = 0; j < 8; j++) {
                 /* FIXME */
                cbm2_font_14_uppercase[(i * 14) + j] = mem_chargen_rom[(i * 16) + j + 1 + (0 * 16) + 0x800];
            }
        }
        for (i = 0; i < 128; i++) {
            for (j = 0; j < 8; j++) {
                 /* FIXME */
                cbm2_font_14_uppercase[(i * 14) + j + (128 * 14)] = mem_chargen_rom[(i * 16) + j + 1 + (0 * 16) + 0x800];
            }
        }
        /* init menu font last, since set_menu_font will also make the font active */
        sdl_ui_set_image_font(cbm2_font_14_uppercase, 8, 14);
        sdl_ui_set_menu_font(cbm2_font_14, 8, 14);
    } else {
        menu_draw->extra_y = 32;
        for (i = 0; i < 256; i++) {
            for (j = 0; j < 8; j++) {
                cbm2_font_8[(i * 8) + j] = mem_chargen_rom[(i * 16) + j];
            }
        }
        for (i = 0; i < 128; i++) {
            for (j = 0; j < 8; j++) {
                 /* FIXME */
                cbm2_font_8_uppercase[(i * 8) + j] = mem_chargen_rom[(i * 16) + j + (0 * 16) + 0x1000];
            }
        }
        for (i = 0; i < 128; i++) {
            for (j = 0; j < 8; j++) {
                 /* FIXME */
                cbm2_font_8_uppercase[(i * 8) + j + (128 * 8)] = mem_chargen_rom[(i * 16) + j + (0 * 16) + 0x1000];
            }
        }
        /* init menu font last, since set_menu_font will also make the font active */
        sdl_ui_set_image_font(cbm2_font_8_uppercase, 8, 8);
        sdl_ui_set_menu_font(cbm2_font_8, 8, 8);
    }
#endif

static int loadchar(char *name)
{
    FILE *f;
    unsigned int i;
    int res = 0;

    for(i = 0; i < CBM2FONTSIZE; i+=2) {
        cbm2fontasc[i] = 0x55;
        cbm2fontasc[i + 1] = 0xaa;
    }
    memcpy(cbm2fontpet, cbm2fontasc, CBM2FONTSIZE);
    memcpy(cbm2fontmon, cbm2fontasc, CBM2FONTSIZE);

    if((f = fopen(name, "rb")) == NULL) {
        return -1;
    }
#if 0
    fseek(f, 0, SEEK_SET);
    fread(cbm2fontpet, 0x400, 1, f);
    fseek(f, 0x400, SEEK_SET);
    fread(cbm2fontmon, 0x400, 1, f);
    fseek(f, 0x400, SEEK_SET);
    fread(cbm2fontasc, 0x400 - (8 * 37), 1, f);
#else
    for(i = 0; i < 128; i++) {
        fseek(f, (128 * 16) + i * 16, SEEK_SET);
        if(fread(&cbm2fontpet[i * 8], 1, 8, f) != 8) {
            res = -1;
        }
    }
    for(i = 0; i < 128; i++) {
        fseek(f, (0 * 16) + i * 16, SEEK_SET);
        if(fread(&cbm2fontmon[i * 8], 1, 8, f) != 8) {
            res = -1;
        }
    }
    for(i = 0; i < (128 - 37); i++) {
        fseek(f, (0 * 16) + i * 16, SEEK_SET);
        if(fread(&cbm2fontasc[i * 8], 1, 8, f) != 8) {
            res = -1;
        }
    }
#endif
    fclose(f);
    /* create inverted second half */
    for(i = 0; i < 0x400; i++) {
        cbm2fontpet[i + 0x400] = cbm2fontpet[i] ^ 0xff;
        cbm2fontmon[i + 0x400] = cbm2fontmon[i] ^ 0xff;
        /* cbm2fontasc[i + 0x400] = cbm2fontasc[i] ^ 0xff; */
    }
    return res;
}

int sdl_ui_cbm2_font_init(void)
{
    int i;
    char *name = "chargen.600";
    char *path = NULL;

    if (sysfile_locate(name, &path) != 0) {
           log_error(LOG_DEFAULT, "could not locate menu charset `%s'.\n", name ? name : "(null)");
           return -1;
    }

    cbm2fontasc = lib_malloc(CBM2FONTSIZE);
    cbm2fontpet = lib_malloc(CBM2FONTSIZE);
    cbm2fontmon = lib_malloc(CBM2FONTSIZE);

    if (loadchar(path) != 0) {
        log_error(LOG_DEFAULT, "could not load menu charset `%s'.\n", name ? name : "(null)");
    }
    lib_free(path);

    /* 1f copy _ to arrow-left */
    memset(&cbm2fontasc[0x1f * 8], 0, 8);
    cbm2fontasc[0x1f * 8 + 6] = 0xff;
    /* 1e fix ^ */
    for (i = 3; i < 8; i++) {
        cbm2fontasc[0x1e * 8 + i] &= ~0x18;
    }
    /* 1c fix \ */
    for (i = 0; i < 7; i++) {
        cbm2fontasc[0x1c * 8 + i] = cbm2fontasc[0x2f * 8 + (7 - i)];
    }
    /* 40 fix ´ */
    for (i = 0; i < 3; i++) {
        cbm2fontasc[0x40 * 8 + i] = cbm2fontasc[0x27 * 8 + (3 - i)];
        cbm2fontasc[0x40 * 8 + i + 3] = 0;
    }
    /* fix 7b..7f */
    memcpy(&cbm2fontasc[0x5b * 8], &cbm2extrachars[0], 8 * 4);

    /* TODO: add extended ascii codes 80-ff */

    /* extra menu gfx goes into unprintable codes 0..1f */
    sdl_ui_copy_ui_font(&cbm2fontasc[0x81 * 8]);

    /* init menu font last, since set_menu_font will also make the font active */
    sdl_ui_set_monitor_font(cbm2fontmon, 8, 8);
    sdl_ui_set_image_font(cbm2fontpet, 8, 8);
    sdl_ui_set_menu_font(cbm2fontasc, 8, 8);

    return 0;
}

void sdl_ui_cbm2_font_shutdown(void)
{
    lib_free(cbm2fontasc);
    cbm2fontasc = NULL;
    lib_free(cbm2fontpet);
    cbm2fontpet = NULL;
    lib_free(cbm2fontmon);
    cbm2fontmon = NULL;
}
