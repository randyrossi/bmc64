/*
 * koaladrv.c - Create a c64 koala type file.
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
#include "cmdline.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "mem.h"
#include "gfxoutput.h"
#include "nativedrv.h"
#include "palette.h"
#include "resources.h"
#include "screenshot.h"
#include "translate.h"
#include "types.h"
#include "uiapi.h"
#include "util.h"
#include "vsync.h"

/* TODO:
 * - add VICII FLI / mixed mode handling
 * - add VICII super hires handling
 * - add VICII super hires FLI handling
 * - add VDC text mode
 * - add VDC bitmap mode
 * - add TED FLI / mixed mode handling
 * - add TED multi-color text mode
 * - add VIC mixed mode handling
 * - add possible CRTC mixed mode handling
 * - add C64DTV specific modes handling
 */

#define KOALA_SCREEN_PIXEL_WIDTH   320
#define KOALA_SCREEN_PIXEL_HEIGHT  200

#define KOALA_SCREEN_BYTE_WIDTH    KOALA_SCREEN_PIXEL_WIDTH / 8
#define KOALA_SCREEN_BYTE_HEIGHT   KOALA_SCREEN_PIXEL_HEIGHT / 8

/* define offsets in the koala file */
#define BITMAP_OFFSET 2
#define SCREENRAM_OFFSET 8002
#define VIDEORAM_OFFSET 9002
#define BGCOLOR_OFFSET 10002

STATIC_PROTOTYPE gfxoutputdrv_t koala_drv;
STATIC_PROTOTYPE gfxoutputdrv_t koala_compressed_drv;

/* ------------------------------------------------------------------------ */

static int oversize_handling;
static int undersize_handling;
static int ted_lum_handling;
static int crtc_text_color;
static uint8_t crtc_fgcolor;

static int set_oversize_handling(int val, void *param)
{
    switch (val) {
        case NATIVE_SS_OVERSIZE_SCALE:
        case NATIVE_SS_OVERSIZE_CROP_LEFT_TOP:
        case NATIVE_SS_OVERSIZE_CROP_CENTER_TOP:
        case NATIVE_SS_OVERSIZE_CROP_RIGHT_TOP:
        case NATIVE_SS_OVERSIZE_CROP_LEFT_CENTER:
        case NATIVE_SS_OVERSIZE_CROP_CENTER:
        case NATIVE_SS_OVERSIZE_CROP_RIGHT_CENTER:
        case NATIVE_SS_OVERSIZE_CROP_LEFT_BOTTOM:
        case NATIVE_SS_OVERSIZE_CROP_CENTER_BOTTOM:
        case NATIVE_SS_OVERSIZE_CROP_RIGHT_BOTTOM:
            break;
        default:
            return -1;
    }

    oversize_handling = val;

    return 0;
}

static int set_undersize_handling(int val, void *param)
{
    switch (val) {
        case NATIVE_SS_UNDERSIZE_SCALE:
        case NATIVE_SS_UNDERSIZE_BORDERIZE:
            break;
        default:
            return -1;
    }

    undersize_handling = val;

    return 0;
}

static int set_ted_lum_handling(int val, void *param)
{
    switch (val) {
        case NATIVE_SS_TED_LUM_IGNORE:
        case NATIVE_SS_TED_LUM_DITHER:
            break;
        default:
            return -1;
    }

    ted_lum_handling = val;

    return 0;
}

static int set_crtc_text_color(int val, void *param)
{
    switch (val) {
        case NATIVE_SS_CRTC_WHITE:
            crtc_fgcolor = 1;
            break;
        case NATIVE_SS_CRTC_AMBER:
            crtc_fgcolor = 8;
            break;
        case NATIVE_SS_CRTC_GREEN:
            crtc_fgcolor = 5;
            break;
        default:
            return -1;
    }

    crtc_text_color = val;

    return 0;
}

static const resource_int_t resources_int[] = {
    { "KoalaOversizeHandling", NATIVE_SS_OVERSIZE_SCALE, RES_EVENT_NO, NULL,
      &oversize_handling, set_oversize_handling, NULL },
    { "KoalaUndersizeHandling", NATIVE_SS_UNDERSIZE_SCALE, RES_EVENT_NO, NULL,
      &undersize_handling, set_undersize_handling, NULL },
    RESOURCE_INT_LIST_END
};

static const resource_int_t resources_int_plus4[] = {
    { "KoalaTEDLumHandling", NATIVE_SS_TED_LUM_IGNORE, RES_EVENT_NO, NULL,
      &ted_lum_handling, set_ted_lum_handling, NULL },
    RESOURCE_INT_LIST_END
};

static const resource_int_t resources_int_crtc[] = {
    { "KoalaCRTCTextColor", NATIVE_SS_CRTC_WHITE, RES_EVENT_NO, NULL,
      &crtc_text_color, set_crtc_text_color, NULL },
    RESOURCE_INT_LIST_END
};

static int koaladrv_resources_init(void)
{
    if (machine_class == VICE_MACHINE_PLUS4) {
        if (resources_register_int(resources_int_plus4) < 0) {
            return -1;
        }
    }

    if (machine_class == VICE_MACHINE_CBM6x0 || machine_class == VICE_MACHINE_PET) {
        if (resources_register_int(resources_int_crtc) < 0) {
            return -1;
        }
    }

    return resources_register_int(resources_int);
}

static const cmdline_option_t cmdline_options[] = {
    { "-koalaoversize", SET_RESOURCE, 1,
      NULL, NULL, "KoalaOversizeHandling", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_METHOD, IDCLS_OVERSIZED_HANDLING,
      NULL, NULL },
    { "-koalaundersize", SET_RESOURCE, 1,
      NULL, NULL, "KoalaUndersizeHandling", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_METHOD, IDCLS_UNDERSIZED_HANDLING,
      NULL, NULL },
    CMDLINE_LIST_END
};

static const cmdline_option_t cmdline_options_plus4[] = {
    { "-koalatedlum", SET_RESOURCE, 1,
      NULL, NULL, "KoalaTEDLumHandling", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_METHOD, IDCLS_TED_LUM_HANDLING,
      NULL, NULL },
    CMDLINE_LIST_END
};

static const cmdline_option_t cmdline_options_crtc[] = {
    { "-koalacrtctextcolor", SET_RESOURCE, 1,
      NULL, NULL, "KoalaCRTCTextColor", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_COLOR, IDCLS_CRTC_TEXT_COLOR,
      NULL, NULL },
    CMDLINE_LIST_END
};

static int koaladrv_cmdline_options_init(void)
{
    if (machine_class == VICE_MACHINE_PLUS4) {
        if (cmdline_register_options(cmdline_options_plus4) < 0) {
            return -1;
        }
    }

    if (machine_class == VICE_MACHINE_CBM6x0 || machine_class == VICE_MACHINE_PET) {
        if (cmdline_register_options(cmdline_options_crtc) < 0) {
            return -1;
        }
    }

    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------ */

static void koala_multicolorize_colormap(native_data_t *source)
{
    int i, j;

    for (i = 0; i < KOALA_SCREEN_PIXEL_HEIGHT; i++) {
        for (j = 0; j < (KOALA_SCREEN_PIXEL_WIDTH / 2); j++) {
            source->colormap[(i * KOALA_SCREEN_PIXEL_WIDTH) + (j * 2) + 1] = source->colormap[(i * KOALA_SCREEN_PIXEL_WIDTH) + (j * 2)];
        }
    }
}

static void koala_check_and_correct_cell(native_data_t *source, uint8_t bgcolor)
{
    native_data_t *dest = lib_malloc(sizeof(native_data_t));
    int i, j, k, l, bgcolor_included;
    native_color_sort_t *colors = NULL;

    dest->xsize = 8;
    dest->ysize = 8;
    dest->colormap = lib_malloc(8 * 8);

    for (i = 0; i < KOALA_SCREEN_BYTE_HEIGHT; i++) {
        for (j = 0; j < KOALA_SCREEN_BYTE_WIDTH; j++) {
            for (k = 0; k < 8; k++) {
                for (l = 0; l < 8; l++) {
                    dest->colormap[(k * 8) + l] = source->colormap[(i * 8 * KOALA_SCREEN_PIXEL_WIDTH) + (j * 8) + (k * KOALA_SCREEN_PIXEL_WIDTH) + l];
                }
            }
            colors = native_sort_colors_colormap(dest, 16);
            bgcolor_included = 0;
            for (l = 0; l < 4; l++) {
                if (colors[l].amount != 0 && colors[l].color == bgcolor) {
                    bgcolor_included = 1;
                }
            }
            if (bgcolor_included == 0) {
                colors[3].amount = colors[2].amount;
                colors[3].color = colors[2].color;
                colors[2].amount = colors[1].amount;
                colors[2].color = colors[1].color;
                colors[1].amount = colors[0].amount;
                colors[1].color = colors[0].color;
                colors[0].amount = 8000;
                colors[0].color = bgcolor;
            }
            if (colors[4].amount != 0) {
                colors[4].color = 255;
                vicii_color_to_nearest_vicii_color_colormap(dest, colors);
                for (k = 0; k < 8; k++) {
                    for (l = 0; l < 8; l++) {
                        source->colormap[(i * 8 * KOALA_SCREEN_PIXEL_WIDTH) + (j * 8) + (k * KOALA_SCREEN_PIXEL_WIDTH) + l] = dest->colormap[(k * 8) + l];
                    }
                }
            }
            lib_free(colors);
        }
    }
    lib_free(dest->colormap);
    lib_free(dest);
}

static int koala_render_and_save(native_data_t *source, int compress)
{
    FILE *fd;
    char *filename_ext = NULL;
    uint8_t *filebuffer = NULL;
    uint8_t *result = NULL;
    int i, j, k, l;
    int m = 0;
    int n = 0;
    int retval = 0;
    uint8_t color1 = 255;
    uint8_t color2 = 255;
    uint8_t color3 = 255;
    uint8_t colorbyte;
    uint8_t bgcolor;
    native_color_sort_t *color_order = NULL;

    /* allocate file buffer */
    filebuffer = lib_malloc(10003);

    /* clear filebuffer */
    memset(filebuffer, 0, 10003);

    /* set load addy */
    filebuffer[0] = 0x00;
    filebuffer[1] = 0x60;

    /* make multicolor */
    koala_multicolorize_colormap(source);

    /* find out bgcolor */
    color_order = native_sort_colors_colormap(source, 16);
    bgcolor = color_order[0].color;
    lib_free(color_order);

    /* check and correct cells */
    koala_check_and_correct_cell(source, bgcolor);

    for (i = 0; i < KOALA_SCREEN_BYTE_HEIGHT; i++) {
        for (j = 0; j < KOALA_SCREEN_BYTE_WIDTH; j++) {
            for (k = 0; k < 8; k++) {
                filebuffer[BITMAP_OFFSET + m] = 0;
                for (l = 0; l < 4; l++) {
                    colorbyte = source->colormap[(i * KOALA_SCREEN_PIXEL_WIDTH * 8) + (j * 8) + (k * KOALA_SCREEN_PIXEL_WIDTH) + (l * 2)];
                    if (k == 0 && l == 0) {
                        color1 = 255;
                        color2 = 255;
                        color3 = 255;
                    }
                    if (colorbyte != bgcolor) {
                        if (color1 == 255) {
                            color1 = colorbyte;
                        } else {
                            if (color1 != colorbyte && color2 == 255) {
                                color2 = colorbyte;
                            } else {
                                if (color2 != colorbyte && color3 == 255) {
                                    color3 = colorbyte;
                                }
                            }
                        }
                    }
                    if (colorbyte != bgcolor) {
                        if (colorbyte == color1) {
                            filebuffer[BITMAP_OFFSET + m] |= (1 << ((3 - l) * 2));
                        }
                        if (colorbyte == color2) {
                            filebuffer[BITMAP_OFFSET + m] |= (2 << ((3 - l) * 2));
                        } else {
                            filebuffer[BITMAP_OFFSET + m] |= (3 << ((3 - l) * 2));
                        }
                    }
                }
                m++;
            }
            filebuffer[SCREENRAM_OFFSET + n] = ((color1 & 0xf) << 4) | (color2 & 0xf);
            filebuffer[VIDEORAM_OFFSET + n++] = color3 & 0xf;
        }
    }
    filebuffer[BGCOLOR_OFFSET] = bgcolor;

    if (compress) {
        filename_ext = util_add_extension_const(source->filename, koala_compressed_drv.default_extension);
    } else {
        filename_ext = util_add_extension_const(source->filename, koala_drv.default_extension);
    }

    fd = fopen(filename_ext, MODE_WRITE);
    if (fd == NULL) {
        retval = -1;
    }

    if (retval != -1) {
        if (compress) {
            result = lib_malloc(10003 * 4);
            j = 0;
            i = 2;
            result[j++] = 0;
            result[j++] = 0x60;
            while (i < 9999) {
                if (filebuffer[i] == filebuffer[i + 1] && filebuffer[i] == filebuffer[i + 2] && filebuffer[i] == filebuffer[i + 3]) {
                    result[j++] = 0xFE;
                    result[j] = filebuffer[i];
                    k = 4;
                    i += 4;
                    while (k != 0xFF && i < 10003 && result[j] == filebuffer[i]) {
                        i++;
                        k++;
                    }
                    j++;
                    result[j++] = k;
                } else {
                    if (filebuffer[i] == 0xFE) {
                        result[j++] = 0xFE;
                        result[j++] = 0xFE;
                        result[j++] = 0x01;
                        i++;
                    } else {
                        result[j++] = filebuffer[i++];
                    }
                }
            }
            while (i < 10003) {
                result[j++] = filebuffer[i++];
            }
            if (fwrite(result, j, 1, fd) < 1) {
                retval = -1;
            }
        } else {
            if (fwrite(filebuffer, 10003, 1, fd) < 1) {
                retval = -1;
            }
        }
    }

    if (fd != NULL) {
        fclose(fd);
    }

    lib_free(source->colormap);
    lib_free(source);
    lib_free(filename_ext);
    lib_free(filebuffer);
    lib_free(result);

    return retval;
}

static int koala_direct_save(native_data_t *source, int compress, uint8_t bgcolor)
{
    FILE *fd;
    char *filename_ext = NULL;
    uint8_t *filebuffer = NULL;
    uint8_t *result = NULL;
    int i, j, k, l;
    int m = 0;
    int n = 0;
    int retval = 0;
    uint8_t color1 = 255;
    uint8_t color2 = 255;
    uint8_t color3 = 255;
    uint8_t colorbyte;

    /* allocate file buffer */
    filebuffer = lib_malloc(10003);

    /* clear filebuffer */
    memset(filebuffer, 0, 10003);

    /* set load addy */
    filebuffer[0] = 0x00;
    filebuffer[1] = 0x60;

    for (i = 0; i < KOALA_SCREEN_BYTE_HEIGHT; i++) {
        for (j = 0; j < KOALA_SCREEN_BYTE_WIDTH; j++) {
            for (k = 0; k < 8; k++) {
                filebuffer[BITMAP_OFFSET + m] = 0;
                for (l = 0; l < 4; l++) {
                    colorbyte = source->colormap[(i * KOALA_SCREEN_PIXEL_WIDTH * 8) + (j * 8) + (k * KOALA_SCREEN_PIXEL_WIDTH) + (l * 2)];
                    if (k == 0 && l == 0) {
                        color1 = 255;
                        color2 = 255;
                        color3 = 255;
                    }
                    if (colorbyte != bgcolor) {
                        if (color1 == 255) {
                            color1 = colorbyte;
                        } else {
                            if (color1 != colorbyte && color2 == 255) {
                                color2 = colorbyte;
                            } else {
                                if (color2 != colorbyte && color3 == 255) {
                                    color3 = colorbyte;
                                }
                            }
                        }
                    }
                    if (colorbyte != bgcolor) {
                        if (colorbyte == color1) {
                            filebuffer[BITMAP_OFFSET + m] |= (1 << ((3 - l) * 2));
                        }
                        if (colorbyte == color2) {
                            filebuffer[BITMAP_OFFSET + m] |= (2 << ((3 - l) * 2));
                        } else {
                            filebuffer[BITMAP_OFFSET + m] |= (3 << ((3 - l) * 2));
                        }
                    }
                }
                m++;
            }
            filebuffer[SCREENRAM_OFFSET + n] = ((color1 & 0xf) << 4) | (color2 & 0xf);
            filebuffer[VIDEORAM_OFFSET + n++] = color3 & 0xf;
        }
    }
    filebuffer[BGCOLOR_OFFSET] = bgcolor;

    if (compress) {
        filename_ext = util_add_extension_const(source->filename, koala_compressed_drv.default_extension);
    } else {
        filename_ext = util_add_extension_const(source->filename, koala_drv.default_extension);
    }

    fd = fopen(filename_ext, MODE_WRITE);
    if (fd == NULL) {
        retval = -1;
    }

    if (retval != -1) {
        if (compress) {
            result = lib_malloc(10003 * 4);
            j = 0;
            i = 2;
            result[j++] = 0;
            result[j++] = 0x60;
            while (i < 9999) {
                if (filebuffer[i] == filebuffer[i + 1] && filebuffer[i] == filebuffer[i + 2] && filebuffer[i] == filebuffer[i + 3]) {
                    result[j++] = 0xFE;
                    result[j] = filebuffer[i];
                    k = 4;
                    i += 4;
                    while (k != 0xFF && i < 10003 && result[j] == filebuffer[i]) {
                        i++;
                        k++;
                    }
                    j++;
                    result[j++] = k;
                } else {
                    if (filebuffer[i] == 0xFE) {
                        result[j++] = 0xFE;
                        result[j++] = 0xFE;
                        result[j++] = 0x01;
                        i++;
                    } else {
                        result[j++] = filebuffer[i++];
                    }
                }
            }
            while (i < 10003) {
                result[j++] = filebuffer[i++];
            }
            if (fwrite(result, j, 1, fd) < 1) {
                retval = -1;
            }
        } else {
            if (fwrite(filebuffer, 10003, 1, fd) < 1) {
                retval = -1;
            }
        }
    }

    if (fd != NULL) {
        fclose(fd);
    }

    lib_free(source->colormap);
    lib_free(source);
    lib_free(filename_ext);
    lib_free(filebuffer);
    lib_free(result);

    return retval;
}

/* ------------------------------------------------------------------------ */

static int koala_vicii_save(screenshot_t *screenshot, const char *filename, int compress)
{
    uint8_t *regs = screenshot->video_regs;
    uint8_t mc;
    uint8_t eb;
    uint8_t bm;
    uint8_t blank;
    native_data_t *data = NULL;

    mc = (regs[0x16] & 0x10) >> 4;
    eb = (regs[0x11] & 0x40) >> 6;
    bm = (regs[0x11] & 0x20) >> 5;

    blank = (regs[0x11] & 0x10) >> 4;

    if (!blank) {
        ui_error("Screen is blanked, no picture to save");
        return -1;
    }

    switch (mc << 2 | eb << 1 | bm) {
        case 0:    /* normal text mode */
            data = native_vicii_text_mode_render(screenshot, filename);
            return koala_render_and_save(data, compress);
            break;
        case 1:    /* hires bitmap mode */
            data = native_vicii_hires_bitmap_mode_render(screenshot, filename);
            return koala_render_and_save(data, compress);
            break;
        case 2:    /* extended background mode */
            data = native_vicii_extended_background_mode_render(screenshot, filename);
            return koala_render_and_save(data, compress);
            break;
        case 4:    /* multicolor text mode */
            data = native_vicii_multicolor_text_mode_render(screenshot, filename);
            return koala_render_and_save(data, compress);
            break;
        case 5:    /* multicolor bitmap mode */
            data = native_vicii_multicolor_bitmap_mode_render(screenshot, filename);
            return koala_direct_save(data, compress, (uint8_t)(regs[0x21] & 0xf));
            break;
        default:   /* illegal modes (3, 6 and 7) */
            ui_error("Illegal mode, no saving will be done");
            return -1;
            break;
    }
    return 0;
}

/* ------------------------------------------------------------------------ */

static int koala_ted_save(screenshot_t *screenshot, const char *filename, int compress)
{
    uint8_t *regs = screenshot->video_regs;
    uint8_t mc;
    uint8_t eb;
    uint8_t bm;
    native_data_t *data = NULL;

    mc = (regs[0x07] & 0x10) >> 4;
    eb = (regs[0x06] & 0x40) >> 6;
    bm = (regs[0x06] & 0x20) >> 5;

    switch (mc << 2 | eb << 1 | bm) {
        case 0:    /* normal text mode */
            data = native_ted_text_mode_render(screenshot, filename);
            ted_color_to_vicii_color_colormap(data, ted_lum_handling);
            return koala_render_and_save(data, compress);
            break;
        case 1:    /* hires bitmap mode */
            data = native_ted_hires_bitmap_mode_render(screenshot, filename);
            ted_color_to_vicii_color_colormap(data, ted_lum_handling);
            return koala_render_and_save(data, compress);
            break;
        case 2:    /* extended background mode */
            data = native_ted_extended_background_mode_render(screenshot, filename);
            ted_color_to_vicii_color_colormap(data, ted_lum_handling);
            return koala_render_and_save(data, compress);
            break;
        case 4:    /* multicolor text mode */
            ui_error("This screen saver is a WIP, it doesn't support multicolor text mode (yet)");
            return -1;
            break;
        case 5:    /* multicolor bitmap mode */
            data = native_ted_multicolor_bitmap_mode_render(screenshot, filename);
            ted_color_to_vicii_color_colormap(data, ted_lum_handling);
            return koala_render_and_save(data, compress);
            break;
        default:   /* illegal modes (3, 6 and 7) */
            ui_error("Illegal mode, no saving will be done");
            return -1;
            break;
    }
    return 0;
}

/* ------------------------------------------------------------------------ */

static int koala_vic_save(screenshot_t *screenshot, const char *filename, int compress)
{
    uint8_t *regs = screenshot->video_regs;
    native_data_t *data = native_vic_render(screenshot, filename);

    if (data == NULL) {
        return -1;
    }

    vic_color_to_vicii_color_colormap(data);

    if (data->xsize != KOALA_SCREEN_PIXEL_WIDTH || data->ysize != KOALA_SCREEN_PIXEL_HEIGHT) {
        data = native_resize_colormap(data, KOALA_SCREEN_PIXEL_WIDTH, KOALA_SCREEN_PIXEL_HEIGHT, (uint8_t)(regs[0xf] & 7), oversize_handling, undersize_handling);
    }

    return koala_render_and_save(data, compress);
}

/* ------------------------------------------------------------------------ */

static int koala_crtc_save(screenshot_t *screenshot, const char *filename, int compress)
{
    native_data_t *data = native_crtc_render(screenshot, filename, crtc_fgcolor);

    if (data == NULL) {
        return -1;
    }

    if (data->xsize != KOALA_SCREEN_PIXEL_WIDTH || data->ysize != KOALA_SCREEN_PIXEL_HEIGHT) {
        data = native_resize_colormap(data, KOALA_SCREEN_PIXEL_WIDTH, KOALA_SCREEN_PIXEL_HEIGHT, 0, oversize_handling, undersize_handling);
    }
    return koala_render_and_save(data, compress);
}

/* ------------------------------------------------------------------------ */

static int koala_vdc_save(screenshot_t *screenshot, const char *filename, int compress)
{
    uint8_t *regs = screenshot->video_regs;
    native_data_t *data = NULL;

    if (regs[25] & 0x80) {
        ui_error("VDC bitmap mode screenshot saving not implemented yet");
        return -1;
    } else {
        data = native_vdc_text_mode_render(screenshot, filename);
        vdc_color_to_vicii_color_colormap(data);
        return koala_render_and_save(data, compress);
    }
    return -1;
}

/* ------------------------------------------------------------------------ */

static int koaladrv_save(screenshot_t *screenshot, const char *filename)
{
    if (!(strcmp(screenshot->chipid, "VICII"))) {
        return koala_vicii_save(screenshot, filename, 0);
    }
    if (!(strcmp(screenshot->chipid, "VDC"))) {
        return koala_vdc_save(screenshot, filename, 0);
    }
    if (!(strcmp(screenshot->chipid, "CRTC"))) {
        return koala_crtc_save(screenshot, filename, 0);
    }
    if (!(strcmp(screenshot->chipid, "TED"))) {
        return koala_ted_save(screenshot, filename, 0);
    }
    if (!(strcmp(screenshot->chipid, "VIC"))) {
        return koala_vic_save(screenshot, filename, 0);
    }
    ui_error("Unknown graphics chip");
    return -1;
}

static int koaladrv_compressed_save(screenshot_t *screenshot, const char *filename)
{
    if (!(strcmp(screenshot->chipid, "VICII"))) {
        return koala_vicii_save(screenshot, filename, 1);
    }
    if (!(strcmp(screenshot->chipid, "VDC"))) {
        return koala_vdc_save(screenshot, filename, 1);
    }
    if (!(strcmp(screenshot->chipid, "CRTC"))) {
        return koala_crtc_save(screenshot, filename, 1);
    }
    if (!(strcmp(screenshot->chipid, "TED"))) {
        return koala_ted_save(screenshot, filename, 1);
    }
    if (!(strcmp(screenshot->chipid, "VIC"))) {
        return koala_vic_save(screenshot, filename, 1);
    }
    ui_error("Unknown graphics chip");
    return -1;
}

static gfxoutputdrv_t koala_drv =
{
    "KOALA",
    "C64 koala screenshot",
    "koa",
    NULL, /* formatlist */
    NULL,
    NULL,
    NULL,
    NULL,
    koaladrv_save,
    NULL,
    NULL,
    koaladrv_resources_init,
    koaladrv_cmdline_options_init
#ifdef FEATURE_CPUMEMHISTORY
    , NULL
#endif
};

static gfxoutputdrv_t koala_compressed_drv =
{
    "KOALA_COMPRESSED",
    "C64 compressed koala screenshot",
    "gg",
    NULL, /* formatlist */
    NULL,
    NULL,
    NULL,
    NULL,
    koaladrv_compressed_save,
    NULL,
    NULL,
    NULL,
    NULL
#ifdef FEATURE_CPUMEMHISTORY
    , NULL
#endif
};

void gfxoutput_init_koala(int help)
{
    gfxoutput_register(&koala_drv);
    gfxoutput_register(&koala_compressed_drv);
}
