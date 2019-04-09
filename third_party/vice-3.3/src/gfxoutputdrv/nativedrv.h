/*
 * nativedrv.h - native screenshot common code header.
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

#ifndef VICE_NATIVEDRV_H
#define VICE_NATIVEDRV_H

#include "screenshot.h"
#include "types.h"

typedef struct native_data_s {
    uint8_t *colormap;
    int xsize;
    int ysize;
    int mc_data_present;
    const char *filename;
} native_data_t;

typedef struct native_color_sort_s {
    uint8_t color;
    int amount;
} native_color_sort_t;

extern void gfxoutput_init_doodle(int help);
extern void gfxoutput_init_koala(int help);

extern void native_smooth_scroll_borderize_colormap(native_data_t *source, uint8_t bordercolor, uint8_t xcover, uint8_t ycover);
extern native_data_t *native_borderize_colormap(native_data_t *source, uint8_t bordercolor, int xsize, int ysize);
extern native_data_t *native_crop_and_borderize_colormap(native_data_t *source, uint8_t bordercolor, int xsize, int ysize, int oversize_handling);
extern native_data_t *native_scale_colormap(native_data_t *source, int xsize, int ysize);
extern native_data_t *native_resize_colormap(native_data_t *source, int xsize, int ysize, uint8_t bordercolor, int oversize_handling, int undersize_handling);
extern native_color_sort_t *native_sort_colors_colormap(native_data_t *source, int color_amount);

extern void vicii_color_to_vicii_bw_colormap(native_data_t *source);
extern void vicii_color_to_vicii_gray_colormap(native_data_t *source);
extern void vicii_color_to_nearest_vicii_color_colormap(native_data_t *source, native_color_sort_t *colors);

extern void ted_color_to_vicii_color_colormap(native_data_t *source, int ted_lum_handling);

extern void vic_color_to_vicii_color_colormap(native_data_t *source);

extern void vdc_color_to_vicii_color_colormap(native_data_t *source);

extern native_data_t *native_vicii_text_mode_render(screenshot_t *screenshot, const char *filename);
extern native_data_t *native_vicii_extended_background_mode_render(screenshot_t *screenshot, const char *filename);
extern native_data_t *native_vicii_multicolor_text_mode_render(screenshot_t *screenshot, const char *filename);
extern native_data_t *native_vicii_hires_bitmap_mode_render(screenshot_t *screenshot, const char *filename);
extern native_data_t *native_vicii_multicolor_bitmap_mode_render(screenshot_t *screenshot, const char *filename);

extern native_data_t *native_ted_text_mode_render(screenshot_t *screenshot, const char *filename);
extern native_data_t *native_ted_extended_background_mode_render(screenshot_t *screenshot, const char *filename);
extern native_data_t *native_ted_hires_bitmap_mode_render(screenshot_t *screenshot, const char *filename);
extern native_data_t *native_ted_multicolor_bitmap_mode_render(screenshot_t *screenshot, const char *filename);

extern native_data_t *native_vic_render(screenshot_t *screenshot, const char *filename);

extern native_data_t *native_crtc_render(screenshot_t *screenshot, const char *filename, int crtc_fgcolor);

extern native_data_t *native_vdc_text_mode_render(screenshot_t *screenshot, const char *filename);

#endif
