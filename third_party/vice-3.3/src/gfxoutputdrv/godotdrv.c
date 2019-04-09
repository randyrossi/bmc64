/*
 * godotdrv.c - Create a godot file.
 *
 * Written by
 *  Kajtar Zsolt <soci@c64.rulez.org>
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
#include <string.h>
#include <stdlib.h>

#include "archdep.h"
#include "godotdrv.h"
#include "lib.h"
#include "log.h"
#include "gfxoutput.h"
#include "palette.h"
#include "screenshot.h"
#include "types.h"
#include "util.h"
#include "nativedrv.h"


typedef struct gfxoutputdrv_data_s {
    FILE *fd;
    char *ext_filename;
    uint8_t *data;
    unsigned int line;
    unsigned int count;
    unsigned int byte;
} gfxoutputdrv_data_t;

STATIC_PROTOTYPE gfxoutputdrv_t godot_drv;

static int godotdrv_write_file_header(screenshot_t *screenshot)
{
    FILE *fd = screenshot->gfxoutputdrv_data->fd;

    if (screenshot->width == 320 &&  screenshot->height == 200) {
        if (fprintf(fd, "GOD0") < 0) {
            return -1;
        }
        return 0;
    }
    if (fprintf(fd, "GOD1%c%c%c%c", 0, 0, screenshot->width / 8, screenshot->height / 8) < 0) {
        return -1;
    }
    return 0;
}

static int godotdrv_open(screenshot_t *screenshot, const char *filename)
{
    gfxoutputdrv_data_t *sdata;

    sdata = lib_malloc(sizeof(gfxoutputdrv_data_t));
    screenshot->gfxoutputdrv_data = sdata;
    sdata->line = 0;
    sdata->count = 0;
    sdata->byte = 0;
    sdata->ext_filename = util_add_extension_const(filename, godot_drv.default_extension);
    sdata->fd = fopen(sdata->ext_filename, "wb");

    if (sdata->fd == NULL) {
        lib_free(sdata->ext_filename);
        lib_free(sdata);
        return -1;
    }

    if (godotdrv_write_file_header(screenshot) < 0) {
        fclose(sdata->fd);
        lib_free(sdata->ext_filename);
        lib_free(sdata);
        return -1;
    }

    sdata->data = lib_malloc(screenshot->width * 8);

    return 0;
}

static const unsigned char godotpalette[16]={
    0, 15, 4, 12, 5, 10, 1, 13, 6, 2, 9, 3, 7, 14, 8, 11
};

static int godotdrv_write(screenshot_t *screenshot)
{
    gfxoutputdrv_data_t *sdata;

    sdata = screenshot->gfxoutputdrv_data;
    (screenshot->convert_line)(screenshot, sdata->data + (sdata->line & 7) * screenshot->width, sdata->line, SCREENSHOT_MODE_PALETTE);

    if ((sdata->line & 7) == 7) {
        native_data_t native;
        int x, y, x1;

        native.xsize = screenshot->width;
        native.ysize = 8;
        native.colormap = sdata->data;
        if (!(strcmp(screenshot->chipid, "VICII"))) {
            /* nothing */
        } else if (!(strcmp(screenshot->chipid, "VDC"))) {
            vdc_color_to_vicii_color_colormap(&native);
        } else if (!(strcmp(screenshot->chipid, "VIC"))) {
            vic_color_to_vicii_color_colormap(&native);
        } else  if (!(strcmp(screenshot->chipid, "TED"))) {
            ted_color_to_vicii_color_colormap(&native, NATIVE_SS_TED_LUM_IGNORE);
        } else if (!(strcmp(screenshot->chipid, "CRTC"))) {
            /* nothing */
        }

        for (x1 = 0; x1 < (int)screenshot->width / 8; x1++) {
            for (y = 0; y < 8; y++) {
                unsigned char *p1 = &sdata->data[y * screenshot->width + x1 * 8];
                for (x = 0; x < 8; x += 2) {
                    unsigned char c = godotpalette[p1[x + 1] & 15] | (godotpalette[p1[x] & 15] << 4);
                    if (c == sdata->byte && sdata->count < 256) {
                        sdata->count++;
                        continue;
                    }
                    if (sdata->count < 4 && sdata->byte != 0xad) {
                        while (sdata->count--) putc(sdata->byte, sdata->fd);
                    } else {
                        putc(0xad, sdata->fd);
                        putc(sdata->count, sdata->fd);
                        putc(sdata->byte, sdata->fd);
                    }
                    sdata->byte = c;
                    sdata->count = 1;
                }
            }
        }
    }
    return 0;
}

static int godotdrv_close(screenshot_t *screenshot)
{
    gfxoutputdrv_data_t *sdata;

    sdata = screenshot->gfxoutputdrv_data;

    if (sdata->count < 4 && sdata->byte != 0xad) {
        while (sdata->count--) putc(sdata->byte, sdata->fd);
    } else {
        putc(0xad, sdata->fd);
        putc(sdata->count, sdata->fd);
        putc(sdata->byte, sdata->fd);
    }
    putc(0xad, sdata->fd);

    fclose(sdata->fd);
    lib_free(sdata->data);
    lib_free(sdata->ext_filename);
    lib_free(sdata);

    return 0;
}

static int godotdrv_save(screenshot_t *screenshot, const char *filename)
{
    if (godotdrv_open(screenshot, filename) < 0) {
        return -1;
    }

    for (screenshot->gfxoutputdrv_data->line = 0;
         screenshot->gfxoutputdrv_data->line < screenshot->height;
         (screenshot->gfxoutputdrv_data->line)++) {
        godotdrv_write(screenshot);
    }

    if (godotdrv_close(screenshot) < 0) {
        return -1;
    }

    return 0;
}

static gfxoutputdrv_t godot_drv =
{
    "4BT",
    "Godot screenshot",
    "4bt",
    NULL, /* formatlist */
    godotdrv_open,
    godotdrv_close,
    godotdrv_write,
    godotdrv_save,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
#ifdef FEATURE_CPUMEMHISTORY
    , NULL
#endif
};

void gfxoutput_init_godot(int help)
{
    if (help) {
        return;
    }
    gfxoutput_register(&godot_drv);
}
