/*
 * pcxdrv.c - Create a PCX file.
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
#include "pcxdrv.h"
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
    uint8_t *pcx_data;
    unsigned int line;
} gfxoutputdrv_data_t;

STATIC_PROTOTYPE gfxoutputdrv_t pcx_drv;

static int pcxdrv_write_file_header(screenshot_t *screenshot)
{
    uint8_t header[128];

    memset(header, 0, sizeof(header));

    header[0] = 0xa;
    header[1] = 5;
    header[2] = 1;
    header[3] = 8;

    util_word_to_le_buf(&header[8], (uint16_t)(screenshot->width - 1));
    util_word_to_le_buf(&header[10], (uint16_t)(screenshot->height - 1));

    util_word_to_le_buf(&header[12], (uint16_t)(screenshot->dpi_x));
    util_word_to_le_buf(&header[14], (uint16_t)(screenshot->dpi_x));

    header[65] = 1;
    util_word_to_le_buf(&header[66], (uint16_t)(screenshot->width));

    if (fwrite(header, sizeof(header), 1, screenshot->gfxoutputdrv_data->fd) < 1) {
        return -1;
    }
    return 0;
}

static int pcxdrv_open(screenshot_t *screenshot, const char *filename)
{
    gfxoutputdrv_data_t *sdata;

    if (screenshot->palette->num_entries > 256) {
        log_error(LOG_DEFAULT, "Max 256 colors supported.");
        return -1;
    }

    sdata = lib_malloc(sizeof(gfxoutputdrv_data_t));
    screenshot->gfxoutputdrv_data = sdata;
    sdata->line = 0;
    sdata->ext_filename = util_add_extension_const(filename, pcx_drv.default_extension);
    sdata->fd = fopen(sdata->ext_filename, "wb");

    if (sdata->fd == NULL) {
        lib_free(sdata->ext_filename);
        lib_free(sdata);
        return -1;
    }

    if (pcxdrv_write_file_header(screenshot) < 0) {
        fclose(sdata->fd);
        lib_free(sdata->ext_filename);
        lib_free(sdata);
        return -1;
    }

    sdata->data = lib_malloc(screenshot->width);
    sdata->pcx_data = lib_malloc(screenshot->width * 2);

    return 0;
}

static int pcxdrv_write(screenshot_t *screenshot)
{
    gfxoutputdrv_data_t *sdata;
    uint8_t color, amount;
    unsigned int i, j = 0;

    sdata = screenshot->gfxoutputdrv_data;
    (screenshot->convert_line)(screenshot, sdata->data, sdata->line, SCREENSHOT_MODE_PALETTE);

    color = sdata->data[0];
    amount = 1;

    for (i = 1; i < screenshot->width; i++) {
        if (sdata->data[i] == color) {
            amount = amount + 1;
            if (amount == 63) {
                sdata->pcx_data[j] = 0xff;
                sdata->pcx_data[j + 1] = color;
                j = j + 2;
                amount = 0;
            }
        } else {
            if (amount == 0) {
                color = sdata->data[i];
                amount = 1;
            } else {
                if (amount > 1) {
                    sdata->pcx_data[j] = 0xc0 | amount;
                    sdata->pcx_data[j + 1] = color;
                    j = j + 2;
                    color = sdata->data[i];
                    amount = 1;
                } else {
                    if (color > 0xbf) {
                        sdata->pcx_data[j] = 0xc1;
                        sdata->pcx_data[j + 1] = color;
                        j = j + 2;
                    } else {
                        sdata->pcx_data[j] = color;
                        j++;
                    }
                    color = sdata->data[i];
                    amount = 1;
                }
            }
        }
    }

    if (amount == 1) {
        if (color > 0xbf) {
            sdata->pcx_data[j] = 0xc1;
            sdata->pcx_data[j + 1] = color;
            j = j + 2;
        } else {
            sdata->pcx_data[j] = color;
            j++;
        }
    } else {
        if (amount > 1) {
            sdata->pcx_data[j] = 0xc0 | amount;
            sdata->pcx_data[j + 1] = color;
            j = j + 2;
        }
    }

    if (fwrite(sdata->pcx_data, j, 1, sdata->fd) < 1) {
        return -1;
    }
    return 0;
}

static int pcxdrv_close(screenshot_t *screenshot)
{
    gfxoutputdrv_data_t *sdata;
    unsigned int i;
    int res = -1;
    unsigned char pcx_color_prefix[2] = "\x0c";
    uint8_t pcx_colors[256 * 3];

    sdata = screenshot->gfxoutputdrv_data;

    if (fwrite(pcx_color_prefix, 1, 1, sdata->fd) == 1) {
        for (i = 0; i < screenshot->palette->num_entries; i++) {
            pcx_colors[(i * 3)] = screenshot->palette->entries[i].red;
            pcx_colors[(i * 3) + 1] = screenshot->palette->entries[i].green;
            pcx_colors[(i * 3) + 2] = screenshot->palette->entries[i].blue;
        }

        if (fwrite(pcx_colors, (3 * 256), 1, sdata->fd) == 1) {
            res = 0;
        }
    }

    fclose(sdata->fd);
    lib_free(sdata->data);
    lib_free(sdata->pcx_data);
    lib_free(sdata->ext_filename);
    lib_free(sdata);

    return res;
}

static int pcxdrv_save(screenshot_t *screenshot, const char *filename)
{
    if (pcxdrv_open(screenshot, filename) < 0) {
        return -1;
    }

    for (screenshot->gfxoutputdrv_data->line = 0;
         screenshot->gfxoutputdrv_data->line < screenshot->height;
         (screenshot->gfxoutputdrv_data->line)++) {
        pcxdrv_write(screenshot);
    }

    if (pcxdrv_close(screenshot) < 0) {
        return -1;
    }

    return 0;
}

#ifdef FEATURE_CPUMEMHISTORY
static FILE *pcxdrv_memmap_fd;
static char *pcxdrv_memmap_ext_filename;
static uint8_t *pcxdrv_memmap_pcx_data;

static int pcxdrv_close_memmap(uint8_t *palette)
{
    int res = 0;
    uint8_t pcx_color_prefix[2] = "\x0c";

    if (fwrite(pcx_color_prefix, 1, 1, pcxdrv_memmap_fd) != 1) {
        res = -1;
    } else if (fwrite(palette, 3 * 256, 1, pcxdrv_memmap_fd) != 1) {
        res = -1;
    }

    fclose(pcxdrv_memmap_fd);
    lib_free(pcxdrv_memmap_pcx_data);
    lib_free(pcxdrv_memmap_ext_filename);

    return res;
}

static int pcxdrv_write_memmap(int line, int x_size, uint8_t *gfx)
{
    uint8_t color, amount;
    int i, j = 0;

    color = gfx[(line * x_size)];
    amount = 1;

    for (i = 1; i < x_size; i++) {
        if (gfx[(line * x_size) + i] == color) {
            amount = amount + 1;
            if (amount == 63) {
                pcxdrv_memmap_pcx_data[j] = 0xff;
                pcxdrv_memmap_pcx_data[j + 1] = color;
                j = j + 2;
                amount = 0;
            }
        } else {
            if (amount == 0) {
                color = gfx[(line * x_size) + i];
                amount = 1;
            } else {
                if (amount > 1) {
                    pcxdrv_memmap_pcx_data[j] = 0xc0 | amount;
                    pcxdrv_memmap_pcx_data[j + 1] = color;
                    j = j + 2;
                    color = gfx[(line * x_size) + i];
                    amount = 1;
                } else {
                    if (color > 0xbf) {
                        pcxdrv_memmap_pcx_data[j] = 0xc1;
                        pcxdrv_memmap_pcx_data[j + 1] = color;
                        j = j + 2;
                    } else {
                        pcxdrv_memmap_pcx_data[j] = color;
                        j++;
                    }
                    color = gfx[(line * x_size) + i];
                    amount = 1;
                }
            }
        }
    }

    if (amount == 1) {
        if (color > 0xbf) {
            pcxdrv_memmap_pcx_data[j] = 0xc1;
            pcxdrv_memmap_pcx_data[j + 1] = color;
            j = j + 2;
        } else {
            pcxdrv_memmap_pcx_data[j] = color;
            j++;
        }
    } else {
        if (amount > 1) {
            pcxdrv_memmap_pcx_data[j] = 0xc0 | amount;
            pcxdrv_memmap_pcx_data[j + 1] = color;
            j = j + 2;
        }
    }

    if (fwrite(pcxdrv_memmap_pcx_data, j, 1, pcxdrv_memmap_fd) < 1) {
        return -1;
    }
    return 0;
}

static int pcxdrv_write_file_header_memmap(int x_size, int y_size)
{
    uint8_t header[128];

    memset(header, 0, sizeof(header));

    header[0] = 0xa;
    header[1] = 5;
    header[2] = 1;
    header[3] = 8;

    util_word_to_le_buf(&header[8], (uint16_t)(x_size - 1));
    util_word_to_le_buf(&header[10], (uint16_t)(y_size - 1));

    util_word_to_le_buf(&header[12], (uint16_t)(0));
    util_word_to_le_buf(&header[14], (uint16_t)(0));

    header[65] = 1;
    util_word_to_le_buf(&header[66], (uint16_t)(x_size));

    if (fwrite(header, sizeof(header), 1, pcxdrv_memmap_fd) < 1) {
        return -1;
    }
    return 0;
}

static int pcxdrv_open_memmap(const char *filename, int x_size, int y_size)
{
    pcxdrv_memmap_ext_filename = util_add_extension_const(filename, pcx_drv.default_extension);
    pcxdrv_memmap_fd = fopen(pcxdrv_memmap_ext_filename, "wb");

    if (pcxdrv_memmap_fd == NULL) {
        lib_free(pcxdrv_memmap_ext_filename);
        return -1;
    }

    if (pcxdrv_write_file_header_memmap(x_size, y_size) < 0) {
        fclose(pcxdrv_memmap_fd);
        lib_free(pcxdrv_memmap_ext_filename);
        return -1;
    }

    pcxdrv_memmap_pcx_data = lib_malloc(x_size * 2);

    return 0;
}

static int pcxdrv_save_memmap(const char *filename, int x_size, int y_size, uint8_t *gfx, uint8_t *palette)
{
    int line;

    if (pcxdrv_open_memmap(filename, x_size, y_size) < 0) {
        return -1;
    }

    for (line = 0; line < y_size; line++) {
        pcxdrv_write_memmap(line, x_size, gfx);
    }

    if (pcxdrv_close_memmap(palette) < 0) {
        return -1;
    }

    return 0;
}
#endif

static gfxoutputdrv_t pcx_drv =
{
    "PCX",
    "PCX screenshot",
    "pcx",
    NULL, /* formatlist */
    pcxdrv_open,
    pcxdrv_close,
    pcxdrv_write,
    pcxdrv_save,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
#ifdef FEATURE_CPUMEMHISTORY
    , pcxdrv_save_memmap
#endif
};

void gfxoutput_init_pcx(int help)
{
    if (help) {
        return;
    }
    gfxoutput_register(&pcx_drv);
}
