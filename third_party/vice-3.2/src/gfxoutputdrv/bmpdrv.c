/*
 * bmpdrv.c - Create a BMP file.
 *
 * Written by
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "archdep.h"
#include "bmpdrv.h"
#include "lib.h"
#include "log.h"
#include "gfxoutput.h"
#include "palette.h"
#include "screenshot.h"
#include "types.h"
#include "util.h"


#define BMP_HDR_OFFSET (14 + 40 + 4 * screenshot->palette->num_entries)
#define BMP_HDR_OFFSET24 (14 + 40)

typedef struct gfxoutputdrv_data_s {
    FILE *fd;
    char *ext_filename;
    uint8_t *data;
    uint8_t *bmp_data;
    int line;
    unsigned int bpp;
} gfxoutputdrv_data_t;

STATIC_PROTOTYPE gfxoutputdrv_t bmp_drv;

static int bmpdrv_bytes_per_row(screenshot_t *screenshot)
{
    int bits_per_row = screenshot->gfxoutputdrv_data->bpp * screenshot->width;
    return (bits_per_row / 32 + (bits_per_row % 32 != 0)) * 4;
}

static uint32_t bmpdrv_bmp_size(screenshot_t *screenshot)
{
    return BMP_HDR_OFFSET + bmpdrv_bytes_per_row(screenshot) * screenshot->height;
}

static int bmpdrv_write_file_header(screenshot_t *screenshot)
{
    gfxoutputdrv_data_t *sdata;
    uint8_t header[14];

    sdata = screenshot->gfxoutputdrv_data;

    memset(header, 0, sizeof(header));

    header[0] = 'B';
    header[1] = 'M';

    util_dword_to_le_buf(&header[2], bmpdrv_bmp_size(screenshot));

    if (sdata->bpp == 24) {
        util_dword_to_le_buf(&header[10], BMP_HDR_OFFSET24);
    } else {
        util_dword_to_le_buf(&header[10], BMP_HDR_OFFSET);
    }

    if (fwrite(header, sizeof(header), 1, screenshot->gfxoutputdrv_data->fd)
        < 1) {
        return -1;
    }

    return 0;
}

static int bmpdrv_write_bitmap_info(screenshot_t *screenshot)
{
    uint8_t binfo[40];
    uint8_t *bcolor;
    unsigned int i;

    memset(binfo, 0, sizeof(binfo));

    util_dword_to_le_buf(&binfo[0], sizeof(binfo));
    util_dword_to_le_buf(&binfo[4], screenshot->width);
    util_dword_to_le_buf(&binfo[8], screenshot->height);

    binfo[12] = 1;
    binfo[13] = 0;

    binfo[14] = screenshot->gfxoutputdrv_data->bpp;
    binfo[15] = 0;

    util_dword_to_le_buf(&binfo[16], 0); /* BI_RGB */
    util_dword_to_le_buf(&binfo[20], 0);

    /* DPI in Pixels per Meter*/
    util_dword_to_le_buf(&binfo[24], screenshot->dpi_x * 10000 / 254);
    util_dword_to_le_buf(&binfo[28], screenshot->dpi_y * 10000 / 254);

    if (screenshot->gfxoutputdrv_data->bpp == 24) {
        util_dword_to_le_buf(&binfo[32], 0);
        util_dword_to_le_buf(&binfo[36], 0);
    } else {
        util_dword_to_le_buf(&binfo[32], screenshot->palette->num_entries);
        util_dword_to_le_buf(&binfo[36], screenshot->palette->num_entries);
    }

    if (fwrite(binfo, sizeof(binfo), 1, screenshot->gfxoutputdrv_data->fd) < 1) {
        return -1;
    }

    if (screenshot->gfxoutputdrv_data->bpp != 24) {
        bcolor = lib_malloc(screenshot->palette->num_entries * 4);

        for (i = 0; i < screenshot->palette->num_entries; i++) {
            bcolor[i * 4] = screenshot->palette->entries[i].blue;
            bcolor[i * 4 + 1] = screenshot->palette->entries[i].green;
            bcolor[i * 4 + 2] = screenshot->palette->entries[i].red;
            bcolor[i * 4 + 3] = 0;
        }

        if (fwrite(bcolor, screenshot->palette->num_entries * 4, 1,
                   screenshot->gfxoutputdrv_data->fd) < 1) {
            lib_free(bcolor);
            return -1;
        }

        lib_free(bcolor);
    }

    return 0;
}

static int bmpdrv_open(screenshot_t *screenshot, const char *filename)
{
    gfxoutputdrv_data_t *sdata;

    sdata = lib_malloc(sizeof(gfxoutputdrv_data_t));

    screenshot->gfxoutputdrv_data = sdata;

    if (screenshot->palette->num_entries <= 2) {
        sdata->bpp = 1;
    } else if (screenshot->palette->num_entries <= 16) {
        sdata->bpp = 4;
    } else if (screenshot->palette->num_entries <= 256) {
        sdata->bpp = 8;
    } else {
        sdata->bpp = 24;
    }

    sdata->line = 0;

    sdata->ext_filename
        = util_add_extension_const(filename, bmp_drv.default_extension);

    sdata->fd = fopen(sdata->ext_filename, MODE_WRITE);

    if (sdata->fd == NULL) {
        lib_free(sdata->ext_filename);
        lib_free(sdata);
        return -1;
    }

    if (bmpdrv_write_file_header(screenshot) < 0) {
        fclose(screenshot->gfxoutputdrv_data->fd);
        lib_free(screenshot->gfxoutputdrv_data->ext_filename);
        lib_free(screenshot->gfxoutputdrv_data);
        return -1;
    }

    if (bmpdrv_write_bitmap_info(screenshot) < 0) {
        fclose(sdata->fd);
        lib_free(sdata->ext_filename);
        lib_free(sdata);
        return -1;
    }

    if (sdata->bpp == 24) {
        sdata->data = lib_malloc(screenshot->width * 3);
    } else {
        sdata->data = lib_malloc(screenshot->width);
    }

    sdata->bmp_data = lib_malloc(bmpdrv_bytes_per_row(screenshot) * screenshot->height);

    return 0;
}

static int bmpdrv_write(screenshot_t *screenshot)
{
    unsigned int row;
    gfxoutputdrv_data_t *sdata;
    int bmp_width = bmpdrv_bytes_per_row(screenshot);

    sdata = screenshot->gfxoutputdrv_data;

    if (sdata->bpp == 24) {
        (screenshot->convert_line)(screenshot, sdata->data, sdata->line, SCREENSHOT_MODE_RGB24);
    } else {
        (screenshot->convert_line)(screenshot, sdata->data, sdata->line, SCREENSHOT_MODE_PALETTE);
    }

    switch (sdata->bpp) {
        case 1:
            {
                int i, j;
                memset(sdata->bmp_data + (screenshot->height - 1 - sdata->line)
                       * bmp_width, 0, bmp_width);

                for (i = 0; i < (int)screenshot->width / 8; i++)
                {
                    uint8_t b = 0;
                    for (j = 0; j < 8; j++) {
                        b |= sdata->data[i * 8 + j] ? (1 << (7 - j)) : 0;
                    }
                    sdata->bmp_data[((screenshot->height - 1 - sdata->line)
                                     * bmp_width) + i] = b;
                }
            }
            break;
        case 4:
            for (row = 0; row < screenshot->width / 2; row++) {
                sdata->bmp_data[((screenshot->height - 1 - sdata->line)
                                 * bmp_width) + row]
                    = ((sdata->data[row * 2] & 0xf) << 4)
                      | (sdata->data[row * 2 + 1] & 0xf);
            }
            break;
        case 8:
            memcpy(sdata->bmp_data + (screenshot->height - 1 - sdata->line)
                   * bmp_width, sdata->data, screenshot->width);
            break;
        case 24:
            memcpy(sdata->bmp_data + (screenshot->height - 1 - sdata->line)
                   * bmp_width, sdata->data, screenshot->width * 3);
            break;
    }

    sdata->line++;

    return 0;
}

static int bmpdrv_close(screenshot_t *screenshot)
{
    int res = -1;
    size_t len = bmpdrv_bytes_per_row(screenshot) * screenshot->height;

    if (fwrite(screenshot->gfxoutputdrv_data->bmp_data, len, 1, screenshot->gfxoutputdrv_data->fd) == 1) {
        res = 0;
    }

    lib_free(screenshot->gfxoutputdrv_data->data);
    lib_free(screenshot->gfxoutputdrv_data->bmp_data);
    fclose(screenshot->gfxoutputdrv_data->fd);
    lib_free(screenshot->gfxoutputdrv_data->ext_filename);
    lib_free(screenshot->gfxoutputdrv_data);
    return res;
}

static int bmpdrv_save(screenshot_t *screenshot, const char *filename)
{
    unsigned int i;

    if (bmpdrv_open(screenshot, filename) < 0) {
        return -1;
    }

    for (i = 0; i < screenshot->height; i++) {
        if (bmpdrv_write(screenshot) < 0) {
            fclose(screenshot->gfxoutputdrv_data->fd);
            lib_free(screenshot->gfxoutputdrv_data->ext_filename);
            lib_free(screenshot->gfxoutputdrv_data);
            return -1;
        }
    }

    if (bmpdrv_close(screenshot) < 0) {
        return -1;
    }

    return 0;
}

#ifdef FEATURE_CPUMEMHISTORY
static FILE *bmpdrv_memmap_fd;
static char *bmpdrv_memmap_ext_filename;
static uint8_t *bmpdrv_memmap_bmp_data;

static int bmpdrv_memmap_bytes_per_row(int x_size)
{
    int bits_per_row = 8 * x_size;  /* 8 bits per pixel */
    return (bits_per_row / 32 + (bits_per_row % 32 != 0)) * 4;
}

static int bmpdrv_close_memmap(int x_size, int y_size)
{
    int res = 0;
    size_t len = bmpdrv_memmap_bytes_per_row(x_size) * y_size;

    if (fwrite(bmpdrv_memmap_bmp_data, len, 1, bmpdrv_memmap_fd) != 1) {
        res = -1;
    }

    fclose(bmpdrv_memmap_fd);
    lib_free(bmpdrv_memmap_ext_filename);
    lib_free(bmpdrv_memmap_bmp_data);
    return res;
}

static uint32_t bmpdrv_memmap_bmp_size(int x_size, int y_size)
{
    return 14 + 40 + 4 * 256 + bmpdrv_memmap_bytes_per_row(x_size) * y_size;
}

static int bmpdrv_memmap_write_bitmap_info(int x_size, int y_size, uint8_t *palette)
{
    uint8_t binfo[40];
    uint8_t *bcolor;
    unsigned int i;

    memset(binfo, 0, sizeof(binfo));

    util_dword_to_le_buf(&binfo[0], sizeof(binfo));
    util_dword_to_le_buf(&binfo[4], x_size);
    util_dword_to_le_buf(&binfo[8], y_size);

    binfo[12] = 1;
    binfo[13] = 0;

    binfo[14] = 8;  /* 8 bits per pixel */
    binfo[15] = 0;

    util_dword_to_le_buf(&binfo[16], 0); /* BI_RGB */
    util_dword_to_le_buf(&binfo[20], 0);

    /* DPI in Pixels per Meter*/
    util_dword_to_le_buf(&binfo[24], 0 * 10000 / 254);
    util_dword_to_le_buf(&binfo[28], 0 * 10000 / 254);

    util_dword_to_le_buf(&binfo[32], 256);
    util_dword_to_le_buf(&binfo[36], 256);

    if (fwrite(binfo, sizeof(binfo), 1, bmpdrv_memmap_fd) < 1) {
        return -1;
    }

    bcolor = lib_malloc(256 * 4);

    for (i = 0; i < 256; i++) {
        bcolor[i * 4] = palette[(i * 3) + 2];
        bcolor[i * 4 + 1] = palette[(i * 3) + 1];
        bcolor[i * 4 + 2] = palette[(i * 3)];
        bcolor[i * 4 + 3] = 0;
    }

    if (fwrite(bcolor, 256 * 4, 1, bmpdrv_memmap_fd) < 1) {
        lib_free(bcolor);
        return -1;
    }

    lib_free(bcolor);
    return 0;
}

static int bmpdrv_memmap_write_file_header(int x_size, int y_size)
{
    uint8_t header[14];

    memset(header, 0, sizeof(header));

    header[0] = 'B';
    header[1] = 'M';

    util_dword_to_le_buf(&header[2], bmpdrv_memmap_bmp_size(x_size, y_size));

    util_dword_to_le_buf(&header[10], (14 + 40 + 4 * 256));

    if (fwrite(header, sizeof(header), 1, bmpdrv_memmap_fd) < 1) {
        return -1;
    }

    return 0;
}

static int bmpdrv_open_memmap(const char *filename, int x_size, int y_size, uint8_t *palette)
{
    bmpdrv_memmap_ext_filename = util_add_extension_const(filename, bmp_drv.default_extension);

    bmpdrv_memmap_fd = fopen(bmpdrv_memmap_ext_filename, MODE_WRITE);

    if (bmpdrv_memmap_fd == NULL) {
        lib_free(bmpdrv_memmap_ext_filename);
        return -1;
    }

    if (bmpdrv_memmap_write_file_header(x_size, y_size) < 0) {
        fclose(bmpdrv_memmap_fd);
        lib_free(bmpdrv_memmap_ext_filename);
        return -1;
    }

    if (bmpdrv_memmap_write_bitmap_info(x_size, y_size, palette) < 0) {
        fclose(bmpdrv_memmap_fd);
        lib_free(bmpdrv_memmap_ext_filename);
        return -1;
    }

    bmpdrv_memmap_bmp_data = lib_malloc(bmpdrv_memmap_bytes_per_row(x_size) * y_size);

    return 0;
}

static void bmpdrv_write_memmap(int line, int x_size, int y_size, uint8_t *gfx)
{
    int bmp_width = bmpdrv_memmap_bytes_per_row(x_size);
    memcpy(bmpdrv_memmap_bmp_data + (y_size - 1 - line) * bmp_width, gfx + (line * x_size), x_size);
}

static int bmpdrv_memmap_save(const char *filename, int x_size, int y_size, uint8_t *gfx, uint8_t *palette)
{
    int line;

    if (bmpdrv_open_memmap(filename, x_size, y_size, palette) < 0) {
        return -1;
    }

    for (line = 0; line < y_size; line++) {
        bmpdrv_write_memmap(line, x_size, y_size, gfx);
    }

    if (bmpdrv_close_memmap(x_size, y_size) < 0) {
        return -1;
    }

    return 0;
}
#endif

static gfxoutputdrv_t bmp_drv =
{
    "BMP",
    "BMP screenshot",
    "bmp",
    NULL, /* formatlist */
    bmpdrv_open,
    bmpdrv_close,
    bmpdrv_write,
    bmpdrv_save,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
#ifdef FEATURE_CPUMEMHISTORY
    , bmpdrv_memmap_save
#endif
};

void gfxoutput_init_bmp(int help)
{
    if (help) {
        return;
    }
    gfxoutput_register(&bmp_drv);
}
