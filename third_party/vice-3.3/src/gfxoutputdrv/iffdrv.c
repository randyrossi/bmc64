/*
 * iffdrv.c - Create an Amiga IFF file.
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "archdep.h"
#include "iffdrv.h"
#include "lib.h"
#include "log.h"
#include "gfxoutput.h"
#include "palette.h"
#include "screenshot.h"
#include "types.h"
#include "util.h"


typedef struct gfxoutputdrv_data_s {
    FILE *fd;
    char *ext_filename;
    uint8_t *data;
    uint8_t *iff_data;
    unsigned int line;
    int iff_rowbytes;
} gfxoutputdrv_data_t;

STATIC_PROTOTYPE gfxoutputdrv_t iff_drv;

static uint8_t powers[8]= { 1, 2, 4, 8, 16, 32, 64, 128 };

static int iffdrv_write_file_header(screenshot_t *screenshot)
{
    gfxoutputdrv_data_t *sdata;
    uint8_t header[836];
    int i;
    int totalsize;

    memset(header, 0, sizeof(header));

    sdata = screenshot->gfxoutputdrv_data;

    sdata->iff_rowbytes = ((screenshot->width + 15) >> 4) << 1;

    totalsize = 828 + (sdata->iff_rowbytes * screenshot->height * 8);

    header[0] = 'F';
    header[1] = 'O';
    header[2] = 'R';
    header[3] = 'M';
    util_dword_to_be_buf(&header[4], totalsize);
    header[8] = 'I';
    header[9] = 'L';
    header[10] = 'B';
    header[11] = 'M';
    header[12] = 'B';
    header[13] = 'M';
    header[14] = 'H';
    header[15] = 'D';
    util_dword_to_be_buf(&header[16], 20);
    util_word_to_be_buf(&header[20], (uint16_t)(screenshot->width));
    util_word_to_be_buf(&header[22], (uint16_t)(screenshot->height));
    header[28] = 8;
    util_word_to_be_buf(&header[36], (uint16_t)(screenshot->width));
    util_word_to_be_buf(&header[38], (uint16_t)(screenshot->height));
    header[40] = 'C';
    header[41] = 'M';
    header[42] = 'A';
    header[43] = 'P';
    util_dword_to_be_buf(&header[44], 3 * 256);

    for (i = 0; i < 256; i++) {
        header[48 + (i * 3)] = screenshot->palette->entries[i].red;
        header[49 + (i * 3)] = screenshot->palette->entries[i].green;
        header[50 + (i * 3)] = screenshot->palette->entries[i].blue;
    }
    header[816] = 'C';
    header[817] = 'A';
    header[818] = 'M';
    header[819] = 'G';
    util_dword_to_be_buf(&header[820], 4);
    header[828] = 'B';
    header[829] = 'O';
    header[830] = 'D';
    header[831] = 'Y';
    util_dword_to_be_buf(&header[832], sdata->iff_rowbytes * screenshot->height * 8);

    if (fwrite(header, 836, 1, sdata->fd) < 1) {
        return -1;
    }

    return 0;
}

static int iffdrv_open(screenshot_t *screenshot, const char *filename)
{
    gfxoutputdrv_data_t *sdata;

    if (screenshot->palette->num_entries > 256) {
        log_error(LOG_DEFAULT, "Max 256 colors supported.");
        return -1;
    }

    sdata = lib_malloc(sizeof(gfxoutputdrv_data_t));
    screenshot->gfxoutputdrv_data = sdata;
    sdata->line = 0;
    sdata->ext_filename = util_add_extension_const(filename, iff_drv.default_extension);
    sdata->fd = fopen(sdata->ext_filename, "wb");

    if (sdata->fd == NULL) {
        lib_free(sdata->ext_filename);
        lib_free(sdata);
        return -1;
    }

    if (iffdrv_write_file_header(screenshot) < 0) {
        fclose(sdata->fd);
        lib_free(sdata->ext_filename);
        lib_free(sdata);
        return -1;
    }

    sdata->data = lib_malloc(sdata->iff_rowbytes * 8);
    sdata->iff_data = lib_malloc(sdata->iff_rowbytes);

    return 0;
}

static void iff_c2p(uint8_t *chunky, uint8_t *planar, int amount, int plane)
{
    int i;

    for (i = 0; i < amount; i++) {
        planar[i] = ((chunky[i * 8] & powers[plane]) / powers[plane] * 128) +
                    ((chunky[(i * 8) + 1] & powers[plane]) / powers[plane] * 64) +
                    ((chunky[(i * 8) + 2] & powers[plane]) / powers[plane] * 32) +
                    ((chunky[(i * 8) + 3] & powers[plane]) / powers[plane] * 16) +
                    ((chunky[(i * 8) + 4] & powers[plane]) / powers[plane] * 8) +
                    ((chunky[(i * 8) + 5] & powers[plane]) / powers[plane] * 4) +
                    ((chunky[(i * 8) + 6] & powers[plane]) / powers[plane] * 2) +
                    (chunky[(i * 8) + 7] & powers[plane]) / powers[plane];
    }
}

static int iffdrv_write(screenshot_t *screenshot)
{
    gfxoutputdrv_data_t *sdata;
    int j;

    sdata = screenshot->gfxoutputdrv_data;
    (screenshot->convert_line)(screenshot, sdata->data, sdata->line, SCREENSHOT_MODE_PALETTE);
    for (j = 0; j < 8; j++) {
        iff_c2p(sdata->data, sdata->iff_data, sdata->iff_rowbytes, j);
        if (fwrite(sdata->iff_data, sdata->iff_rowbytes, 1, sdata->fd) < 1) {
            return -1;
        }
    }
    return 0;
}

static int iffdrv_close(screenshot_t *screenshot)
{
    gfxoutputdrv_data_t *sdata;

    sdata = screenshot->gfxoutputdrv_data;

    fclose(sdata->fd);
    lib_free(sdata->data);
    lib_free(sdata->iff_data);
    lib_free(sdata->ext_filename);
    lib_free(sdata);

    return 0;
}

static int iffdrv_save(screenshot_t *screenshot, const char *filename)
{
    if (iffdrv_open(screenshot, filename) < 0) {
        return -1;
    }

    for (screenshot->gfxoutputdrv_data->line = 0;
         screenshot->gfxoutputdrv_data->line < screenshot->height;
         (screenshot->gfxoutputdrv_data->line)++) {
        iffdrv_write(screenshot);
    }

    if (iffdrv_close(screenshot) < 0) {
        return -1;
    }

    return 0;
}

#ifdef FEATURE_CPUMEMHISTORY
static FILE *iffdrv_memmap_fd;
static char *iffdrv_memmap_ext_filename;
static uint8_t *iffdrv_memmap_iff_data;
static int iffdrv_memmap_iff_rowbytes;

static int iffdrv_close_memmap(void)
{
    fclose(iffdrv_memmap_fd);
    lib_free(iffdrv_memmap_iff_data);
    lib_free(iffdrv_memmap_ext_filename);

    return 0;
}

static int iffdrv_write_memmap(int line, int x_size, uint8_t *gfx)
{
    int j;

    for (j = 0; j < 8; j++) {
        iff_c2p(gfx + (x_size * line), iffdrv_memmap_iff_data, iffdrv_memmap_iff_rowbytes, j);
        if (fwrite(iffdrv_memmap_iff_data, iffdrv_memmap_iff_rowbytes, 1, iffdrv_memmap_fd) < 1) {
            return -1;
        }
    }
    return 0;
}

static int iffdrv_write_file_header_memmap(int x_size, int y_size, uint8_t *palette)
{
    uint8_t header[836];
    int i;
    int totalsize;

    memset(header, 0, sizeof(header));

    iffdrv_memmap_iff_rowbytes = ((x_size + 15) >> 4) << 1;

    totalsize = 828 + (iffdrv_memmap_iff_rowbytes * y_size * 8);

    header[0] = 'F';
    header[1] = 'O';
    header[2] = 'R';
    header[3] = 'M';
    util_dword_to_be_buf(&header[4], totalsize);
    header[8] = 'I';
    header[9] = 'L';
    header[10] = 'B';
    header[11] = 'M';
    header[12] = 'B';
    header[13] = 'M';
    header[14] = 'H';
    header[15] = 'D';
    util_dword_to_be_buf(&header[16], 20);
    util_word_to_be_buf(&header[20], (uint16_t)(x_size));
    util_word_to_be_buf(&header[22], (uint16_t)(y_size));
    header[28] = 8;
    util_word_to_be_buf(&header[36], (uint16_t)(x_size));
    util_word_to_be_buf(&header[38], (uint16_t)(y_size));
    header[40] = 'C';
    header[41] = 'M';
    header[42] = 'A';
    header[43] = 'P';
    util_dword_to_be_buf(&header[44], 3 * 256);

    for (i = 0; i < 256; i++) {
        header[48 + (i * 3)] = palette[i * 3];
        header[49 + (i * 3)] = palette[(i * 3) + 1];
        header[50 + (i * 3)] = palette[(i * 3) + 2];
    }
    header[816] = 'C';
    header[817] = 'A';
    header[818] = 'M';
    header[819] = 'G';
    util_dword_to_be_buf(&header[820], 4);
    header[828] = 'B';
    header[829] = 'O';
    header[830] = 'D';
    header[831] = 'Y';
    util_dword_to_be_buf(&header[832], iffdrv_memmap_iff_rowbytes * y_size * 8);

    if (fwrite(header, 836, 1, iffdrv_memmap_fd) < 1) {
        return -1;
    }

    return 0;
}

static int iffdrv_open_memmap(const char *filename, int x_size, int y_size, uint8_t *palette)
{
    iffdrv_memmap_ext_filename = util_add_extension_const(filename, iff_drv.default_extension);
    iffdrv_memmap_fd = fopen(iffdrv_memmap_ext_filename, "wb");

    if (iffdrv_memmap_fd == NULL) {
        lib_free(iffdrv_memmap_ext_filename);
        return -1;
    }

    if (iffdrv_write_file_header_memmap(x_size, y_size, palette) < 0) {
        fclose(iffdrv_memmap_fd);
        lib_free(iffdrv_memmap_ext_filename);
        return -1;
    }

    iffdrv_memmap_iff_data = lib_malloc(iffdrv_memmap_iff_rowbytes);

    return 0;
}

static int iffdrv_save_memmap(const char *filename, int x_size, int y_size, uint8_t *gfx, uint8_t *palette)
{
    int line;

    if (iffdrv_open_memmap(filename, x_size, y_size, palette) < 0) {
        return -1;
    }

    for (line = 0; line < y_size; line++) {
        iffdrv_write_memmap(line, x_size, gfx);
    }

    if (iffdrv_close_memmap() < 0) {
        return -1;
    }

    return 0;
}
#endif

static gfxoutputdrv_t iff_drv =
{
    "IFF",
    "IFF screenshot",
    "iff",
    NULL, /* formatlist */
    iffdrv_open,
    iffdrv_close,
    iffdrv_write,
    iffdrv_save,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
#ifdef FEATURE_CPUMEMHISTORY
    , iffdrv_save_memmap
#endif
};

void gfxoutput_init_iff(int help)
{
    if (help) {
        return;
    }
    gfxoutput_register(&iff_drv);
}
