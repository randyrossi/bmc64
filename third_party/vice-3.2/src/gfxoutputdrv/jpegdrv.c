/*
 * jpegdrv.c - Create a JPEG file.
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

#include "vice.h"

#ifdef HAVE_JPEG
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "archdep.h"
#include "jpegdrv.h"
#include "lib.h"
#include "log.h"
#include "gfxoutput.h"
#include "palette.h"
#include "screenshot.h"
#include "types.h"
#include "util.h"

#include <jpeglib.h>

typedef struct gfxoutputdrv_data_s {
    FILE *fd;
    char *ext_filename;
    uint8_t *data;
    unsigned int line;
} gfxoutputdrv_data_t;

STATIC_PROTOTYPE gfxoutputdrv_t jpeg_drv;

struct jpeg_compress_struct cinfo;
struct jpeg_error_mgr jerr;

static int jpegdrv_open(screenshot_t *screenshot, const char *filename)
{
    gfxoutputdrv_data_t *sdata;

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    sdata = lib_malloc(sizeof(gfxoutputdrv_data_t));
    screenshot->gfxoutputdrv_data = sdata;
    sdata->line = 0;
    sdata->ext_filename = util_add_extension_const(filename, jpeg_drv.default_extension);
    sdata->fd = fopen(sdata->ext_filename, "wb");
    if (sdata->fd == NULL) {
        jpeg_destroy_compress(&cinfo);
        lib_free(sdata->ext_filename);
        lib_free(sdata);
        return -1;
    }
    jpeg_stdio_dest(&cinfo, sdata->fd);
    sdata->data = lib_malloc(screenshot->width * 3);
    cinfo.image_width = screenshot->width;
    cinfo.image_height = screenshot->height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);

    return 0;
}

static int jpegdrv_write(screenshot_t *screenshot)
{
    JSAMPROW rowpointer[1];
    gfxoutputdrv_data_t *sdata;

    sdata = screenshot->gfxoutputdrv_data;
    (screenshot->convert_line)(screenshot, sdata->data, sdata->line, SCREENSHOT_MODE_RGB24);
    rowpointer[0] = sdata->data;
    jpeg_write_scanlines(&cinfo, rowpointer, 1);

    return 0;
}

static int jpegdrv_close(screenshot_t *screenshot)
{
    gfxoutputdrv_data_t *sdata;

    sdata = screenshot->gfxoutputdrv_data;
    jpeg_finish_compress(&cinfo);
    fclose(sdata->fd);
    jpeg_destroy_compress(&cinfo);
    lib_free(sdata->data);
    lib_free(sdata->ext_filename);
    lib_free(sdata);

    return 0;
}

static int jpegdrv_save(screenshot_t *screenshot, const char *filename)
{
    if (jpegdrv_open(screenshot, filename) < 0) {
        return -1;
    }

    for (screenshot->gfxoutputdrv_data->line = 0;
         screenshot->gfxoutputdrv_data->line < screenshot->height;
         (screenshot->gfxoutputdrv_data->line)++) {
        jpegdrv_write(screenshot);
    }

    if (jpegdrv_close(screenshot) < 0) {
        return -1;
    }

    return 0;
}

#ifdef FEATURE_CPUMEMHISTORY
static FILE *jpegdrv_memmap_fd;
static char *jpegdrv_memmap_ext_filename;
static uint8_t *jpegdrv_memmap_data;

static int jpegdrv_close_memmap(void)
{
    jpeg_finish_compress(&cinfo);
    fclose(jpegdrv_memmap_fd);
    jpeg_destroy_compress(&cinfo);
    lib_free(jpegdrv_memmap_data);
    lib_free(jpegdrv_memmap_ext_filename);

    return 0;
}

static int jpegdrv_write_memmap(int line, int x_size, uint8_t *gfx, uint8_t *palette)
{
    int i;
    uint8_t pixval;
    JSAMPROW rowpointer[1];

    for (i = 0; i < x_size; i++) {
        pixval = gfx[(line * x_size) + i];
        jpegdrv_memmap_data[i * 3] = palette[pixval * 3];
        jpegdrv_memmap_data[(i * 3) + 1] = palette[(pixval * 3) + 1];
        jpegdrv_memmap_data[(i * 3) + 2] = palette[(pixval * 3) + 2];
    }
    rowpointer[0] = jpegdrv_memmap_data;
    jpeg_write_scanlines(&cinfo, rowpointer, 1);

    return 0;
}

static int jpegdrv_open_memmap(const char *filename, int x_size, int y_size)
{
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&cinfo);
    jpegdrv_memmap_ext_filename = util_add_extension_const(filename, jpeg_drv.default_extension);
    jpegdrv_memmap_fd = fopen(jpegdrv_memmap_ext_filename, "wb");
    if (jpegdrv_memmap_fd == NULL) {
        jpeg_destroy_compress(&cinfo);
        lib_free(jpegdrv_memmap_ext_filename);
        return -1;
    }
    jpeg_stdio_dest(&cinfo, jpegdrv_memmap_fd);
    jpegdrv_memmap_data = lib_malloc(x_size * 3);
    cinfo.image_width = x_size;
    cinfo.image_height = y_size;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_start_compress(&cinfo, TRUE);

    return 0;
}

static int jpegdrv_save_memmap(const char *filename, int x_size, int y_size, uint8_t *gfx, uint8_t *palette)
{
    int line;

    if (jpegdrv_open_memmap(filename, x_size, y_size) < 0) {
        return -1;
    }

    for (line = 0; line < y_size; line++) {
        jpegdrv_write_memmap(line, x_size, gfx, palette);
    }

    if (jpegdrv_close_memmap() < 0) {
        return -1;
    }

    return 0;
}
#endif

static gfxoutputdrv_t jpeg_drv =
{
    "JPEG",
    "JPEG screenshot",
    "jpg",
    NULL, /* formatlist */
    jpegdrv_open,
    jpegdrv_close,
    jpegdrv_write,
    jpegdrv_save,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
#ifdef FEATURE_CPUMEMHISTORY
    , jpegdrv_save_memmap
#endif
};

void gfxoutput_init_jpeg(int help)
{
    if (help) {
        return;
    }
    gfxoutput_register(&jpeg_drv);
}
#endif
