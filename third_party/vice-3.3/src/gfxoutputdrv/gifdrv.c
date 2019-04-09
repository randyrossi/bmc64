/*
 * gifdrv.c - Create a GIF file.
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

#if defined(HAVE_GIF) && !defined(USE_GIF_DLL)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <gif_lib.h>

#include "archdep.h"
#include "lib.h"
#include "log.h"
#include "gfxoutput.h"
#include "palette.h"
#include "screenshot.h"
#include "types.h"
#include "util.h"

#include "gifdrv.h"

#if GIFLIB_MAJOR >= 5
#  define VICE_EGifOpenFileName(x, y, z) EGifOpenFileName(x, y, z)
#  define VICE_MakeMapObject GifMakeMapObject
#  define VICE_FreeMapObject GifFreeMapObject
#  if GIFLIB_MINOR >= 1
#    define VICE_EGifCloseFile(x) EGifCloseFile(x, NULL)
#  else
#    define VICE_EGifCloseFile(x) EGifCloseFile(x)
#  endif
#else
#  define VICE_EGifOpenFileName(x, y, z) EGifOpenFileName(x, y)
#  define VICE_MakeMapObject MakeMapObject
#  define VICE_FreeMapObject FreeMapObject
#  define VICE_EGifCloseFile(x) EGifCloseFile(x)
#endif

typedef struct gfxoutputdrv_data_s {
    GifFileType *fd;
    char *ext_filename;
    uint8_t *data;
    unsigned int line;
} gfxoutputdrv_data_t;

STATIC_PROTOTYPE gfxoutputdrv_t gif_drv;

static ColorMapObject *gif_colors = NULL;

static int gifdrv_open(screenshot_t *screenshot, const char *filename)
{
    unsigned int i;
    gfxoutputdrv_data_t *sdata;
    GifColorType ColorMap256[256];
#if GIFLIB_MAJOR >= 5
    int ec;
#endif

    if (screenshot->palette->num_entries > 256) {
        log_error(LOG_DEFAULT, "Max 256 colors supported.");
        return -1;
    }

    sdata = lib_malloc(sizeof(gfxoutputdrv_data_t));

    screenshot->gfxoutputdrv_data = sdata;

    sdata->line = 0;

    sdata->ext_filename = util_add_extension_const(filename, gif_drv.default_extension);

    sdata->fd = VICE_EGifOpenFileName(sdata->ext_filename, 0, &ec);

    if (sdata->fd == NULL) {
        lib_free(sdata->ext_filename);
        lib_free(sdata);
        return -1;
    }

    sdata->data = lib_malloc(screenshot->width);

    gif_colors = VICE_MakeMapObject(screenshot->palette->num_entries, ColorMap256);

    for (i = 0; i < screenshot->palette->num_entries; i++) {
        gif_colors->Colors[i].Blue = screenshot->palette->entries[i].blue;
        gif_colors->Colors[i].Green = screenshot->palette->entries[i].green;
        gif_colors->Colors[i].Red = screenshot->palette->entries[i].red;
    }

#if GIFLIB_MAJOR < 5
    EGifSetGifVersion("87a");
#endif

    if (EGifPutScreenDesc(sdata->fd, screenshot->width, screenshot->height, 8, 0, gif_colors) == GIF_ERROR ||
        EGifPutImageDesc(sdata->fd, 0, 0, screenshot->width, screenshot->height, 0, NULL) == GIF_ERROR) {
        VICE_EGifCloseFile(sdata->fd);
        VICE_FreeMapObject(gif_colors);
        lib_free(sdata->data);
        lib_free(sdata->ext_filename);
        lib_free(sdata);
        return -1;
    }

    return 0;
}

static int gifdrv_write(screenshot_t *screenshot)
{
    gfxoutputdrv_data_t *sdata;

    sdata = screenshot->gfxoutputdrv_data;

    (screenshot->convert_line)(screenshot, sdata->data, sdata->line, SCREENSHOT_MODE_PALETTE);

    if (EGifPutLine(sdata->fd, sdata->data, screenshot->width) == GIF_ERROR) {
        return -1;
    }

    return 0;
}

static int gifdrv_close(screenshot_t *screenshot)
{
    gfxoutputdrv_data_t *sdata;

    sdata = screenshot->gfxoutputdrv_data;

    VICE_EGifCloseFile(sdata->fd);
    VICE_FreeMapObject(gif_colors);

    /* for some reason giflib will create a file with unexpected
       permissions. for this reason we alter them according to
       the current umask.

       Carefull: oddly enough still true as of 2017-07-18, so do not remove the
       following function call without doing research/testing first (compyx)
    */
    archdep_fix_permissions(sdata->ext_filename);

    lib_free(sdata->data);
    lib_free(sdata->ext_filename);
    lib_free(sdata);

    return 0;
}

static int gifdrv_save(screenshot_t *screenshot, const char *filename)
{
    if (gifdrv_open(screenshot, filename) < 0) {
        return -1;
    }

    for (screenshot->gfxoutputdrv_data->line = 0;
         screenshot->gfxoutputdrv_data->line < screenshot->height;
         (screenshot->gfxoutputdrv_data->line)++) {
        gifdrv_write(screenshot);
    }

    if (gifdrv_close(screenshot) < 0) {
        return -1;
    }

    return 0;
}

#ifdef FEATURE_CPUMEMHISTORY
static GifFileType *gifdrv_memmap_fd;
static char *gifdrv_memmap_ext_filename;

static int gifdrv_close_memmap(void)
{
    VICE_EGifCloseFile(gifdrv_memmap_fd);
    VICE_FreeMapObject(gif_colors);
    lib_free(gifdrv_memmap_ext_filename);

    return 0;
}

static int gifdrv_write_memmap(int line, int x_size, uint8_t *gfx)
{
    if (EGifPutLine(gifdrv_memmap_fd, gfx + (line * x_size), x_size) == GIF_ERROR) {
        return -1;
    }

    return 0;
}

static int gifdrv_open_memmap(const char *filename, int x_size, int y_size, uint8_t *palette)
{
    unsigned int i;
    GifColorType ColorMap256[256];
#if GIFLIB_MAJOR >= 5
    int ec;
#endif

    gifdrv_memmap_ext_filename = util_add_extension_const(filename, gif_drv.default_extension);

    gifdrv_memmap_fd = VICE_EGifOpenFileName(gifdrv_memmap_ext_filename, 0, &ec);

    if (gifdrv_memmap_fd == NULL) {
        lib_free(gifdrv_memmap_ext_filename);
        return -1;
    }

    gif_colors = VICE_MakeMapObject(256, ColorMap256);

    for (i = 0; i < 256; i++) {
        gif_colors->Colors[i].Blue = palette[(i * 3) + 2];
        gif_colors->Colors[i].Green = palette[(i * 3) + 1];
        gif_colors->Colors[i].Red = palette[i * 3];
    }

#if GIFLIB_MAJOR < 5
    EGifSetGifVersion("87a");
#endif

    if (EGifPutScreenDesc(gifdrv_memmap_fd, x_size, y_size, 8, 0, gif_colors) == GIF_ERROR ||
        EGifPutImageDesc(gifdrv_memmap_fd, 0, 0, x_size, y_size, 0, NULL) == GIF_ERROR) {
        VICE_EGifCloseFile(gifdrv_memmap_fd);
        VICE_FreeMapObject(gif_colors);
        lib_free(gifdrv_memmap_ext_filename);
        return -1;
    }

    return 0;
}

static int gifdrv_save_memmap(const char *filename, int x_size, int y_size, uint8_t *gfx, uint8_t *palette)
{
    int line;

    if (gifdrv_open_memmap(filename, x_size, y_size, palette) < 0) {
        return -1;
    }

    for (line = 0; line < y_size; line++) {
        gifdrv_write_memmap(line, x_size, gfx);
    }

    if (gifdrv_close_memmap() < 0) {
        return -1;
    }

    return 0;
}
#endif

static gfxoutputdrv_t gif_drv =
{
    "GIF",
    "GIF screenshot",
    "gif",
    NULL, /* formatlist */
    gifdrv_open,
    gifdrv_close,
    gifdrv_write,
    gifdrv_save,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
#ifdef FEATURE_CPUMEMHISTORY
    , gifdrv_save_memmap
#endif
};

void gfxoutput_init_gif(int help)
{
    if (help) {
        return;
    }
    gfxoutput_register(&gif_drv);
}
#endif
