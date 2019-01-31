/*
 * gifdrv.c - .dll GIF file handling.
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
#include <windows.h>

#include "archdep.h"
#include "lib.h"
#include "log.h"
#include "gfxoutput.h"
#include "palette.h"
#include "screenshot.h"
#include "types.h"
#include "util.h"

#include "gfxoutputdrv/gifdrv.h"

#ifdef USE_GIF_DLL

#define GIF_ERROR 0
#define GIF_OK    1

#define VoidPtr void *

typedef int GifWord;
typedef unsigned char GifPixelType;
typedef unsigned char GifByteType;

typedef struct ExtensionBlock {
    int ByteCount;
    char *Bytes;    /* on malloc(3) heap */
    int Function;   /* Holds the type of the Extension block. */
} ExtensionBlock;

typedef struct GifColorType {
    GifByteType Red, Green, Blue;
} GifColorType;

typedef struct ColorMapObject {
    int ColorCount;
    int BitsPerPixel;
    GifColorType *Colors;    /* on malloc(3) heap */
} ColorMapObject;

typedef struct GifImageDesc {
    GifWord Left, Top, Width, Height;  /* Current image dimensions. */
    GifWord Interlace;                 /* Sequential/Interlaced lines. */
    ColorMapObject *ColorMap;          /* The local color map */
} GifImageDesc;

typedef struct SavedImage {
    GifImageDesc ImageDesc;
    unsigned char *RasterBits;  /* on malloc(3) heap */
    int Function;   /* DEPRECATED: Use ExtensionBlocks[x].Function instead */
    int ExtensionBlockCount;
    ExtensionBlock *ExtensionBlocks;    /* on malloc(3) heap */
} SavedImage;

typedef struct GifFileType {
    GifWord SWidth, SHeight;    /* Screen dimensions. */
    GifWord SColorResolution;   /* How many colors can we generate? */
    GifWord SBackGroundColor;   /* I hope you understand this one... */
    ColorMapObject *SColorMap;  /* NULL if not exists. */
    int ImageCount;             /* Number of current image */
    GifImageDesc Image;         /* Block describing current image */
    SavedImage *SavedImages;    /* Use this to accumulate file state */
    VoidPtr UserData;           /* hook to attach user data (TVT) */
    VoidPtr Private;            /* Don't mess with this! */
} GifFileType;

typedef GifFileType *(CALLBACK* EGifOpenFileName_t)(const char *, int);
typedef ColorMapObject *(CALLBACK* MakeMapObject_t)(int, const GifColorType *);
typedef void (CALLBACK* EGifSetGifVersion_t)(const char *);
typedef int (CALLBACK* EGifPutScreenDesc_t)(GifFileType *, int, int, int, int, const ColorMapObject *);
typedef int (CALLBACK* EGifPutImageDesc_t)(GifFileType *, int, int, int, int, int, const ColorMapObject *);
typedef int (CALLBACK* EGifCloseFile_t)(GifFileType *);
typedef void (CALLBACK* FreeMapObject_t)(ColorMapObject *);
typedef int (CALLBACK* EGifPutLine_t)(GifFileType *, GifPixelType *, int);

static EGifOpenFileName_t EGifOpenFileName;
static MakeMapObject_t MakeMapObject;
static EGifSetGifVersion_t EGifSetGifVersion;
static EGifPutScreenDesc_t EGifPutScreenDesc;
static EGifPutImageDesc_t EGifPutImageDesc;
static EGifCloseFile_t EGifCloseFile;
static FreeMapObject_t FreeMapObject;
static EGifPutLine_t EGifPutLine;

static HINSTANCE dll = NULL;

static char *gifdll_name[] = {
    "giflib4.dll",
    "giflib3.dll",
    "giflib.dll",
    "libungif4.dll",
    "libungif3.dll",
    "libungif.dll",
    NULL
};

static int loadgifdll(void)
{
    int i;

    for (i = 0; gifdll_name[i] != NULL; i++) {
        dll = LoadLibrary(gifdll_name[i]);
        if (dll != NULL) {
            return 0;
        }
    }
    return -1;
}

static int get_gif_functions(void)
{
    EGifOpenFileName  = (EGifOpenFileName_t)GetProcAddress(dll, "EGifOpenFileName");
    MakeMapObject = (MakeMapObject_t)GetProcAddress(dll, "MakeMapObject");
    EGifSetGifVersion = (EGifSetGifVersion_t)GetProcAddress(dll, "EGifSetGifVersion");
    EGifPutScreenDesc = (EGifPutScreenDesc_t)GetProcAddress(dll, "EGifPutScreenDesc");
    EGifPutImageDesc  = (EGifPutImageDesc_t)GetProcAddress(dll, "EGifPutImageDesc");
    EGifCloseFile = (EGifCloseFile_t)GetProcAddress(dll, "EGifCloseFile");
    FreeMapObject = (FreeMapObject_t)GetProcAddress(dll, "FreeMapObject");
    EGifPutLine = (EGifPutLine_t)GetProcAddress(dll, "EGifPutLine");

    if (!EGifOpenFileName || !MakeMapObject || !EGifSetGifVersion || !EGifPutScreenDesc || !EGifPutImageDesc || !EGifCloseFile || !FreeMapObject || !EGifPutLine) {
        return -1;
    }

    return 0;
}

static int gifdrv_init(void)
{
    if (loadgifdll() < 0) {
        return -1;
    }

    if (get_gif_functions() < 0) {
        return -1;
    }

    return 0;
}

typedef struct gfxoutputdrv_data_s {
    GifFileType *fd;
    char *ext_filename;
    BYTE *data;
    unsigned int line;
} gfxoutputdrv_data_t;

static gfxoutputdrv_t gif_drv;

static ColorMapObject *gif_colors = NULL;

static int gifdrv_open(screenshot_t *screenshot, const char *filename)
{
    unsigned int i;
    gfxoutputdrv_data_t *sdata;
    GifColorType ColorMap256[256];

    if (screenshot->palette->num_entries > 256) {
        log_error(LOG_DEFAULT, "Max 256 colors supported.");
        return -1;
    }

    sdata = lib_malloc(sizeof(gfxoutputdrv_data_t));

    screenshot->gfxoutputdrv_data = sdata;

    sdata->line = 0;

    sdata->ext_filename = util_add_extension_const(filename, gif_drv.default_extension);

    sdata->fd=EGifOpenFileName(sdata->ext_filename, FALSE);

    if (sdata->fd == NULL) {
        lib_free(sdata->ext_filename);
        lib_free(sdata);
        return -1;
    }

    sdata->data = lib_malloc(screenshot->width);

    gif_colors = MakeMapObject(screenshot->palette->num_entries, ColorMap256);

    for (i = 0; i < screenshot->palette->num_entries; i++) {
        gif_colors->Colors[i].Blue = screenshot->palette->entries[i].blue;
        gif_colors->Colors[i].Green = screenshot->palette->entries[i].green;
        gif_colors->Colors[i].Red = screenshot->palette->entries[i].red;
    }

    EGifSetGifVersion("87a");

    if (EGifPutScreenDesc(sdata->fd, screenshot->width, screenshot->height, 8, 0, gif_colors) == GIF_ERROR ||
        EGifPutImageDesc(sdata->fd, 0, 0, screenshot->width, screenshot->height, FALSE, NULL) == GIF_ERROR) {
        EGifCloseFile(sdata->fd);
        FreeMapObject(gif_colors);
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

    EGifCloseFile(sdata->fd);
    FreeMapObject(gif_colors);
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

    for (screenshot->gfxoutputdrv_data->line = 0; screenshot->gfxoutputdrv_data->line < screenshot->height; (screenshot->gfxoutputdrv_data->line)++) {
        gifdrv_write(screenshot);
    }

    if (gifdrv_close(screenshot) < 0) {
        return -1;
    }

    return 0;
}

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
    ,NULL
#endif
};

void gfxoutput_init_gif(int help)
{
    if (help) {
        return;
    }
    if (gifdrv_init() == 0) {
        gfxoutput_register(&gif_drv);
    }
}
#endif
