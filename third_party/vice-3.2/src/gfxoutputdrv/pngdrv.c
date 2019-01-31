/*
 * pngdrv.c - Create a PNG file.
 *
 * Written by
 *  Daniel Sladic <dsladic@cs.cmu.edu>
 *  Andreas Boose <viceteam@t-online.de>
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

#ifdef HAVE_PNG
#include <stdio.h>
#include <stdlib.h>

#include <png.h>
#include <zlib.h>

#include "archdep.h"
#include "gfxoutput.h"
#include "lib.h"
#include "pngdrv.h"
#include "screenshot.h"
#include "types.h"
#include "util.h"


typedef struct gfxoutputdrv_data_s {
    FILE *fd;
    char *ext_filename;
    png_structp png_ptr;
    png_infop info_ptr;
    uint8_t *data;
    unsigned int line;
} gfxoutputdrv_data_t;

STATIC_PROTOTYPE gfxoutputdrv_t png_drv;

static int pngdrv_open(screenshot_t *screenshot, const char *filename)
{
    gfxoutputdrv_data_t *sdata;

    sdata = lib_malloc(sizeof(gfxoutputdrv_data_t));

    screenshot->gfxoutputdrv_data = sdata;

    sdata->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
                                             (void *)NULL, NULL, NULL);

    if (sdata->png_ptr == NULL) {
        lib_free(sdata);
        return -1;
    }

    sdata->info_ptr = png_create_info_struct(sdata->png_ptr);

    if (sdata->info_ptr == NULL) {
        png_destroy_write_struct(&(sdata->png_ptr), (png_infopp)NULL);
        lib_free(sdata);
        return -1;
    }

#if (PNG_LIBPNG_VER < 10006)
    if (setjmp(screenshot->gfxoutputdrv_data->png_ptr->jmpbuf)) {
#else
    if (setjmp(png_jmpbuf(screenshot->gfxoutputdrv_data->png_ptr))) {
#endif
        png_destroy_write_struct(&(screenshot->gfxoutputdrv_data->png_ptr),
                                 &(screenshot->gfxoutputdrv_data->info_ptr));
        lib_free(sdata);
        return -1;
    }

    sdata->ext_filename
        = util_add_extension_const(filename, png_drv.default_extension);

    sdata->fd = fopen(sdata->ext_filename, MODE_WRITE);

    if (sdata->fd == NULL) {
        lib_free(sdata->ext_filename);
        lib_free(sdata);
        return -1;
    }

    sdata->data = lib_malloc(screenshot->width * 4);

    png_init_io(sdata->png_ptr, sdata->fd);
    png_set_compression_level(sdata->png_ptr, Z_BEST_COMPRESSION);

    png_set_IHDR(sdata->png_ptr, sdata->info_ptr, screenshot->width, screenshot->height,
                 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(sdata->png_ptr, sdata->info_ptr);

#ifdef PNG_READ_INVERT_ALPHA_SUPPORTED
    png_set_invert_alpha(sdata->png_ptr);
#endif

    return 0;
}

static int pngdrv_write(screenshot_t *screenshot)
{
    gfxoutputdrv_data_t *sdata;

    sdata = screenshot->gfxoutputdrv_data;

    (screenshot->convert_line)(screenshot, sdata->data, sdata->line,
                               SCREENSHOT_MODE_RGB32);
    png_write_row(sdata->png_ptr, (png_bytep)(sdata->data));

    return 0;
}

static int pngdrv_close(screenshot_t *screenshot)
{
    gfxoutputdrv_data_t *sdata;

    sdata = screenshot->gfxoutputdrv_data;

    png_write_end(sdata->png_ptr, sdata->info_ptr);

    png_destroy_write_struct(&(sdata->png_ptr), &(sdata->info_ptr));

    fclose(sdata->fd);
    lib_free(sdata->data);
    lib_free(sdata->ext_filename);
    lib_free(sdata);

    return 0;
}

static int pngdrv_save(screenshot_t *screenshot, const char *filename)
{
    if (pngdrv_open(screenshot, filename) < 0) {
        return -1;
    }

    for (screenshot->gfxoutputdrv_data->line = 0;
         screenshot->gfxoutputdrv_data->line < screenshot->height;
         (screenshot->gfxoutputdrv_data->line)++) {
        pngdrv_write(screenshot);
    }

    if (pngdrv_close(screenshot) < 0) {
        return -1;
    }

    return 0;
}

#ifdef FEATURE_CPUMEMHISTORY
static FILE *pngdrv_memmap_fd;
static char *pngdrv_memmap_ext_filename;
static png_structp pngdrv_memmap_png_ptr;
static png_infop pngdrv_memmap_info_ptr;
static uint8_t *pngdrv_memmap_png_data;

static int pngdrv_close_memmap(void)
{
    png_write_end(pngdrv_memmap_png_ptr, pngdrv_memmap_info_ptr);

    png_destroy_write_struct(&(pngdrv_memmap_png_ptr), &(pngdrv_memmap_info_ptr));

    fclose(pngdrv_memmap_fd);
    lib_free(pngdrv_memmap_png_data);
    lib_free(pngdrv_memmap_ext_filename);

    return 0;
}

static int pngdrv_write_memmap(int line, int x_size, uint8_t *gfx, uint8_t *palette)
{
    int i;
    uint8_t pixval;

    for (i = 0; i < x_size; i++) {
        pixval = gfx[(line * x_size) + i];
        pngdrv_memmap_png_data[i * 4] = palette[pixval * 3];
        pngdrv_memmap_png_data[(i * 4) + 1] = palette[(pixval * 3) + 1];
        pngdrv_memmap_png_data[(i * 4) + 2] = palette[(pixval * 3) + 2];
        pngdrv_memmap_png_data[(i * 4) + 3] = 0;
    }

    png_write_row(pngdrv_memmap_png_ptr, (png_bytep)(pngdrv_memmap_png_data));

    return 0;
}

static int pngdrv_open_memmap(const char *filename, int x_size, int y_size, uint8_t *palette)
{
    pngdrv_memmap_png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (void *)NULL, NULL, NULL);

    if (pngdrv_memmap_png_ptr == NULL) {
        return -1;
    }

    pngdrv_memmap_info_ptr = png_create_info_struct(pngdrv_memmap_png_ptr);

    if (pngdrv_memmap_info_ptr == NULL) {
        png_destroy_write_struct(&(pngdrv_memmap_png_ptr), (png_infopp)NULL);
        return -1;
    }
#if (PNG_LIBPNG_VER < 10006)
    if (setjmp(pngdrv_memmap_png_ptr)) {
#else
    if (setjmp(png_jmpbuf(pngdrv_memmap_png_ptr))) {
#endif
        png_destroy_write_struct(&(pngdrv_memmap_png_ptr), &(pngdrv_memmap_info_ptr));
        return -1;
    }

    pngdrv_memmap_ext_filename = util_add_extension_const(filename, png_drv.default_extension);

    pngdrv_memmap_fd = fopen(pngdrv_memmap_ext_filename, MODE_WRITE);

    if (pngdrv_memmap_fd == NULL) {
        lib_free(pngdrv_memmap_ext_filename);
        return -1;
    }

    pngdrv_memmap_png_data = lib_malloc(x_size * 4);

    png_init_io(pngdrv_memmap_png_ptr, pngdrv_memmap_fd);
    png_set_compression_level(pngdrv_memmap_png_ptr, Z_BEST_COMPRESSION);

    png_set_IHDR(pngdrv_memmap_png_ptr, pngdrv_memmap_info_ptr, x_size, y_size,
                 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(pngdrv_memmap_png_ptr, pngdrv_memmap_info_ptr);

#ifdef PNG_READ_INVERT_ALPHA_SUPPORTED
    png_set_invert_alpha(pngdrv_memmap_png_ptr);
#endif

    return 0;
}

static int pngdrv_save_memmap(const char *filename, int x_size, int y_size, uint8_t *gfx, uint8_t *palette)
{
    int line;

    if (pngdrv_open_memmap(filename, x_size, y_size, palette) < 0) {
        return -1;
    }

    for (line = 0; line < y_size; line++) {
        pngdrv_write_memmap(line, x_size, gfx, palette);
    }

    if (pngdrv_close_memmap() < 0) {
        return -1;
    }

    return 0;
}
#endif

static gfxoutputdrv_t png_drv =
{
    "PNG",
    "PNG screenshot",
    "png",
    NULL, /* formatlist */
    pngdrv_open,
    pngdrv_close,
    pngdrv_write,
    pngdrv_save,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
#ifdef FEATURE_CPUMEMHISTORY
    , pngdrv_save_memmap
#endif
};

void gfxoutput_init_png(int help)
{
    if (help) {
        return;
    }
    gfxoutput_register(&png_drv);
}
#endif
