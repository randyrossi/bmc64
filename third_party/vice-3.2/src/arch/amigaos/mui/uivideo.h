/*
 * uivideo.h
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

#ifndef VICE_UIVIDEO_H
#define VICE_UIVIDEO_H

extern void ui_video_palette_settings_dialog(video_canvas_t *canvas,
                                             char *palette_enable_res,
                                             char *palette_filename_res,
                                             char *palette_filename_text);

extern void ui_video_color_settings_dialog(video_canvas_t *canvas,
                                           char *gamma_res,
                                           char *tint_res,
                                           char *saturation_res,
                                           char *contrast_res,
                                           char *brightness_res);

extern void ui_video_crt_settings_dialog(video_canvas_t *canvas,
                                         char *scanline_shade_res,
                                         char *blur_res,
                                         char *oddline_phase_res,
                                         char *oddline_offset_res);

extern void ui_video_render_filter_settings_dialog(video_canvas_t *canvas, char *render_filter_res);
#endif
