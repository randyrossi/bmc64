/*
 * video.c - Common video functions for Win32
 *
 * Written by
 *  Tibor Biczo <crown@matavnet.hu>
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include "cmdline.h"
#include "fullscrn.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "palette.h"
#include "res.h"
#include "resources.h"
#include "translate.h"
#include "ui.h"
#include "uiapi.h"
#include "util.h"
#include "video.h"
#include "videoarch.h"
#include "viewport.h"

static int video_number_of_canvases;
static video_canvas_t *video_canvases[2];
static int dx9_available;

/* ------------------------------------------------------------------------ */
/* Video-related resources.  */

/* Flag: are we in fullscreen mode?  */
int fullscreen_enabled;
int dx_primary_surface_rendering;
int dx9_disable;

static char *fullscreen_device_list = NULL;

static int set_dx_primary_surface_rendering(int val, void *param)
{
    int i;

    dx_primary_surface_rendering = val ? 1 : 0;

    if (video_dx9_enabled()) {
        for (i = 0; i < video_number_of_canvases; i++) {
            video_canvas_reset_dx9(video_canvases[i]);
        }
    }

    return 0;
}

static int set_dx9_disable(int value, void *param)
{
    int i;
    int old_dx9_disable, old_num_of_canvases;
    unsigned int old_width[2], old_height[2];
    int val = value ? 1 : 0;

    if (!dx9_available) {
        return 0;
    }

    old_dx9_disable = dx9_disable;
    old_num_of_canvases = video_number_of_canvases;

    if (old_dx9_disable != val) {
        for (i = 0; i < video_number_of_canvases; i++) {       
            old_width[i] = video_canvases[i]->draw_buffer->canvas_physical_width;
            old_height[i] = video_canvases[i]->draw_buffer->canvas_physical_height;
            if (old_dx9_disable) {
                /* Anything to do here?? */
            } else {
                video_device_release_dx9(video_canvases[i]);
            }
        }

        dx9_disable = val;
        video_number_of_canvases = 0;

        for (i = 0; i < old_num_of_canvases; i++) {
            if (old_dx9_disable) {
                if (video_canvas_create_dx9(video_canvases[i], &old_width[i], &old_height[i]) == NULL) {
                    dx9_disable = old_dx9_disable;
                }
            }
            if (dx9_disable) {
                video_canvas_create_ddraw(video_canvases[i]);
            }
            ui_canvas_child_window(video_canvases[i], old_dx9_disable);
            video_canvas_resize(video_canvases[i], 1);
        }

        fullscreen_getmodes();
    }

    return 0;
}

static const resource_int_t resources_int[] = {
#ifdef HAVE_D3D9_H
    { "DXPrimarySurfaceRendering", 0, RES_EVENT_NO, NULL,
      &dx_primary_surface_rendering, set_dx_primary_surface_rendering, NULL },
    { "DX9Disable", 0, RES_EVENT_NO, NULL,
      &dx9_disable, set_dx9_disable, NULL },
#endif
    RESOURCE_INT_LIST_END
};

int video_arch_resources_init(void)
{
    if (machine_class != VICE_MACHINE_VSID) {
        return resources_register_int(resources_int);
    }
    set_dx9_disable(1, NULL);
    return 0;
}

void video_arch_resources_shutdown(void)
{
    if (fullscreen_device_list) {
        lib_free(fullscreen_device_list);
    }
}

/* ------------------------------------------------------------------------ */

/* Video-related command-line options.  */

static const cmdline_option_t cmdline_options[] = {
#ifdef HAVE_D3D9_H
    { "-fullscreen", SET_RESOURCE, 0,
      NULL, NULL, "FullScreenEnabled", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_START_VICE_FULLSCREEN_MODE,
      NULL, NULL },
    { "+fullscreen", SET_RESOURCE, 0,
      NULL, NULL, "FullScreenEnabled", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_DONT_START_VICE_FULLSCREEN_MODE,
      NULL, NULL },
    { "-dx9disable", SET_RESOURCE, 0,
      NULL, NULL, "DX9Disable", (resource_value_t) 1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_DISABLE_DX9,
      NULL, NULL },
    { "-dx9enable", SET_RESOURCE, 0,
      NULL, NULL, "DX9Disable", (resource_value_t) 0,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_ENABLE_DX9,
      NULL, NULL },
    { "-dxpsrender", SET_RESOURCE, 0,
      NULL, NULL, "DXPrimarySurfaceRendering", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_ENABLE_DXPS_RENDER,
      NULL, NULL },
    { "+dxpsrender", SET_RESOURCE, 0,
      NULL, NULL, "DXPrimarySurfaceRendering", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_DISABLE_DXPS_RENDER,
      NULL, NULL },
#endif
    CMDLINE_LIST_END
};

#ifdef HAVE_D3D9_H
static cmdline_option_t generated_cmdline_options[] = {
    { "-fullscreendevice", SET_RESOURCE, 1,
      NULL, NULL, "FullscreenDevice", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_COMBO,
      IDS_P_NUMBER, IDS_FULLSCREEN_DEVICE,
      NULL, NULL },
    CMDLINE_LIST_END
};
#endif

int video_arch_cmdline_options_init(void)
{
#ifdef HAVE_D3D9_H
    char *temp1, *temp2, *num, *dev;
    int amount, i;

    if (machine_class != VICE_MACHINE_VSID) {
        amount = fullscreen_get_devices_amount();
        if (amount) {
            dev = lib_stralloc(fullscreen_get_device(0));
            util_remove_spaces(dev);
            temp1 = util_concat(". (0: ", dev, NULL);
            lib_free(dev);
            for (i = 1; i < amount; i++) {
                num = lib_msprintf("%d", i);
                dev = lib_stralloc(fullscreen_get_device(i));
                util_remove_spaces(dev);
                temp2 = util_concat(temp1, ", ", num, ":", dev, NULL);
                lib_free(num);
                lib_free(dev);
                lib_free(temp1);
                temp1 = temp2;
            }
            fullscreen_device_list = util_concat(temp1, ")", NULL);
        } else {
            fullscreen_device_list = lib_stralloc(".");
        }

        generated_cmdline_options[0].description = fullscreen_device_list;

        if (cmdline_register_options(generated_cmdline_options) < 0) {
            return -1;
        }
        return cmdline_register_options(cmdline_options);
    }
#else
    if (machine_class != VICE_MACHINE_VSID) {
        return cmdline_register_options(cmdline_options);
    }
#endif
    return 0;
}


/* ------------------------------------------------------------------------ */

/* Initialization.  */
int video_init(void)
{
    return 0;
}

void video_shutdown(void)
{
    video_shutdown_dx9();
}

void video_arch_canvas_init(struct video_canvas_s *canvas)
{
    if (video_setup_dx9() < 0) {
        dx9_available = 0;
    } else {
        dx9_available = 1;
    }

    canvas->video_draw_buffer_callback = NULL;
}

int video_dx9_enabled(void)
{
    if (machine_class == VICE_MACHINE_VSID) {
        return 0;
    }
    return (dx9_available && !dx9_disable);
}

int video_dx9_available(void)
{
    return dx9_available;
}

/* ------------------------------------------------------------------------ */

video_canvas_t *video_canvas_for_hwnd(HWND hwnd)
{
    int i;

    for (i = 0; i < video_number_of_canvases; i++) {
        if (video_canvases[i]->hwnd == hwnd) {
            return video_canvases[i];
        }
    }

    return NULL;
}

int video_canvas_nr_for_hwnd(HWND hwnd)
{
    int i;

    for (i = 0; i < video_number_of_canvases; i++) {
        if (video_canvases[i]->hwnd == hwnd) {
            return i;
        }
    }

    return 0;
}

void video_canvas_add(video_canvas_t *canvas)
{
    video_canvases[video_number_of_canvases++] = canvas;
}

video_canvas_t *video_canvas_create(video_canvas_t *canvas, unsigned int *width, unsigned int *height, int mapped)
{
    video_canvas_t *canvas_temp;

    canvas->title = lib_stralloc(canvas->viewport->title);

    ui_open_canvas_window(canvas);
    ui_canvas_child_window(canvas, video_dx9_enabled());

    if (video_dx9_enabled()) {
        canvas_temp = video_canvas_create_dx9(canvas, width, height);
        if (canvas_temp == NULL) {
            log_debug("video: Falling back to DirectDraw canvas!");
            dx9_available = 0;
            ui_canvas_child_window(canvas, 0);
        } else {
            return canvas_temp;
        }
    }
    return video_canvas_create_ddraw(canvas);
}

void video_canvas_destroy(video_canvas_t *canvas)
{
    if (video_dx9_enabled()) {
        video_device_release_dx9(canvas);
    }

    if (canvas != NULL) {
        if (canvas->hwnd !=0) {
            DestroyWindow(canvas->hwnd);
        }
        lib_free(canvas->title);
        lib_free(canvas->pixels);
        canvas->title = NULL;
    }
}

int video_canvas_set_palette(video_canvas_t *canvas, palette_t *p)
{
    if (p == NULL) {
        return 0; /* no palette, nothing to do */
    }
    canvas->palette = p;

    video_set_physical_colors(canvas);
    return 0;
}

int video_set_physical_colors(video_canvas_t *c)
{
    unsigned int i;
    int rshift;
    int rbits;
    int gshift;
    int gbits;
    int bshift;
    int bbits;
    DWORD rmask;
    DWORD gmask;
    DWORD bmask;

    /* Use hard coded D3DFMT_X8R8G8B8 format, driver does conversion */
    rshift = 16;
    rmask = 0xff;
    rbits = 0;

    gshift = 8;
    gmask = 0xff;
    gbits = 0;

    bshift = 0;
    bmask = 0xff;
    bbits = 0;

    if (c->depth > 8) {
        for (i = 0; i < 256; i++) {
            video_render_setrawrgb(i, ((i & (rmask << rbits)) >> rbits) << rshift, ((i & (gmask << gbits)) >> gbits) << gshift, ((i & (bmask << bbits)) >> bbits) << bshift);
        }
        video_render_initraw(c->videoconfig);
    }

    if (c->palette) {
        for (i = 0; i < c->palette->num_entries; i++) {
            DWORD p = (((c->palette->entries[i].red&(rmask << rbits)) >> rbits) << rshift) +
                    (((c->palette->entries[i].green&(gmask << gbits)) >> gbits) << gshift) +
                    (((c->palette->entries[i].blue&(bmask << bbits)) >> bbits) << bshift);
            video_render_setphysicalcolor(c->videoconfig, i, p, c->depth);
        }
    }
    return 0;
}

/* Change the size of `s' to `width' * `height' pixels.  */
void video_canvas_resize(video_canvas_t *canvas, char resize_canvas)
{
    int device;
    int fullscreen_width;
    int fullscreen_height;
    int bitdepth;
    int refreshrate;

    if (IsFullscreenEnabled()) {
        GetCurrentModeParameters(&device, &fullscreen_width, &fullscreen_height, &bitdepth, &refreshrate);
    } else {
        ui_resize_canvas_window(canvas);
    }

    if (video_dx9_enabled()) {
        video_canvas_reset_dx9(canvas);
    }
    else {
        video_canvas_reset_ddraw(canvas);
    }
}


/* Raster code has updated display */
void video_canvas_refresh(video_canvas_t *canvas, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h)
{
    if (video_dx9_enabled()) {
        video_canvas_refresh_dx9(canvas, xs, ys, xi, yi, w, h);
    } else {
        video_canvas_refresh_ddraw(canvas, xs, ys, xi, yi, w, h);
    }
}

/* Window got a WM_PAINT and needs a refresh */
void video_canvas_update(HWND hwnd, HDC hdc, int xclient, int yclient, int w, int h)
{
    if (video_dx9_enabled()) {
        video_canvas_update_dx9(hwnd, hdc, xclient, yclient, w, h);
    } else {
        video_canvas_update_ddraw(hwnd, hdc, xclient, yclient, w, h);
    }
}

char video_canvas_can_resize(video_canvas_t *canvas)
{
    return !IsZoomed(canvas->hwnd);
}
