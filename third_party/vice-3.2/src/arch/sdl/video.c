/*
 * video.c - SDL video
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * Based on code by
 *  Ettore Perazzoli
 *  Andre Fachat
 *  Oliver Schaertel
 *  Martin Pottendorfer
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

/* #define SDL_DEBUG */

#include "vice.h"

#include <stdio.h>
#include "vice_sdl.h"

#include "archdep.h"
#include "cmdline.h"
#include "fullscreen.h"
#include "fullscreenarch.h"
#include "joy.h"
#include "joystick.h"
#include "lib.h"
#include "lightpendrv.h"
#include "log.h"
#include "machine.h"
#include "palette.h"
#include "raster.h"
#include "resources.h"
#include "translate.h"
#include "uimenu.h"
#include "uistatusbar.h"
#include "util.h"
#include "videoarch.h"
#include "vkbd.h"
#include "vsidui_sdl.h"
#include "vsync.h"

#ifdef SDL_DEBUG
#define DBG(x)  log_debug x
#else
#define DBG(x)
#endif

#ifdef USE_SDLUI2
typedef GLubyte* (APIENTRY * glGetString_Func)(unsigned int);
glGetString_Func glGetStringAPI = NULL;
#endif

static log_t sdlvideo_log = LOG_ERR;

static int sdl_bitdepth;

static int sdl_limit_mode;
static int sdl_ui_finalized;

/* Custom w/h, used for fullscreen and limiting*/
static int sdl_custom_width = 0;
static int sdl_custom_height = 0;

/* window size, used for free scaling */
static int sdl_window_width = 0;
static int sdl_window_height = 0;

int sdl_active_canvas_num = 0;
static int sdl_num_screens = 0;
static video_canvas_t *sdl_canvaslist[MAX_CANVAS_NUM];
video_canvas_t *sdl_active_canvas = NULL;

#if defined(HAVE_HWSCALE) || defined(USE_SDLUI2)
#ifndef USE_SDLUI2
static int sdl_gl_mode;
static GLint screen_texture;
static int sdl_gl_vertex_base = 0;

static const float sdl_gl_vertex_coord[4 * 4] = {
    /* Normal */
    -1.0f, +1.0f, -1.0f, +1.0f,
    /* Flip X */
    +1.0f, +1.0f, -1.0f, -1.0f,
    /* Flip Y */
    -1.0f, -1.0f, +1.0f, +1.0f,
    /* Flip X&Y */
    +1.0f, -1.0f, +1.0f, -1.0f
};
#endif

static int sdl_gl_aspect_mode;
static char *aspect_ratio_s = NULL;
static double aspect_ratio;

static int sdl_gl_flipx;
static int sdl_gl_flipy;

static int sdl_gl_filter_res;
static int sdl_gl_filter;
#endif

#ifdef USE_SDLUI2
static char *sdl2_renderer_name = NULL;
SDL_RendererFlip flip;
SDL_Window *new_window = NULL;
SDL_Renderer *new_renderer = NULL;
SDL_Texture *new_texture = NULL;
SDL_Surface *new_screen = NULL;
static Uint32 rmask = 0, gmask = 0, bmask = 0, amask = 0;
static int texformat = 0;
#endif

uint8_t *draw_buffer_vsid = NULL;
/* ------------------------------------------------------------------------- */
/* Video-related resources.  */

static int set_sdl_bitdepth(int d, void *param)
{
#ifndef USE_SDLUI2
    switch (d) {
        case 0:
        case 8:
        case 15:
        case 16:
        case 24:
        case 32:
            break;
        default:
            return -1;
#else
    switch (d) {
        case 8:
            texformat = SDL_PIXELFORMAT_RGB332;
            rmask = 0x000000e0, gmask = 0x0000001c, bmask = 0x00000003, amask = 0x00000000;
            break;
        case 15:
            /* Fixme: add render support for that format */
            return -1;
        case 16:
            texformat = SDL_PIXELFORMAT_RGB565;
            rmask = 0x0000f800, gmask = 0x000007e0, bmask = 0x0000001f, amask = 0x00000000;
            break;
        case 24:
            texformat = SDL_PIXELFORMAT_RGB24;
            rmask = 0x000000ff, gmask = 0x0000ff00, bmask = 0x00ff0000, amask = 0x00000000;
            break;
        case 32:
            texformat = SDL_PIXELFORMAT_ARGB8888;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            rmask = 0x0000ff00, gmask = 0x00ff0000, bmask = 0xff000000, amask = 0x000000ff;
#else
            rmask = 0x00ff0000, gmask = 0x0000ff00, bmask = 0x000000ff, amask = 0xff000000;
#endif
            break;
        default:
            return -1;
#endif
    }

    if (sdl_bitdepth == d) {
        return 0;
    }
    sdl_bitdepth = d;
#if defined(HAVE_HWSCALE) && !defined(USE_SDLUI2)
    if (!((d == 0) || (d == 24) || (d == 32))) {
        resources_set_int("HwScalePossible", 0);
    }
#endif
    /* update */
    return 0;
}

static int set_sdl_limit_mode(int v, void *param)
{
    switch (v) {
        case SDL_LIMIT_MODE_OFF:
        case SDL_LIMIT_MODE_MAX:
        case SDL_LIMIT_MODE_FIXED:
            break;
        default:
            return -1;
    }

    if (sdl_limit_mode != v) {
        sdl_limit_mode = v;
        video_viewport_resize(sdl_active_canvas, 1);
    }
    return 0;
}

static int set_sdl_custom_width(int w, void *param)
{
    if (w <= 0) {
        return -1;
    }

    if (sdl_custom_width != w) {
        sdl_custom_width = w;
        if (sdl_active_canvas && sdl_active_canvas->fullscreenconfig->enable
            && sdl_active_canvas->fullscreenconfig->mode == FULLSCREEN_MODE_CUSTOM) {
            video_viewport_resize(sdl_active_canvas, 1);
        }
    }
    return 0;
}

static int set_sdl_custom_height(int h, void *param)
{
    if (h <= 0) {
        return -1;
    }

    if (sdl_custom_height != h) {
        sdl_custom_height = h;
        if (sdl_active_canvas && sdl_active_canvas->fullscreenconfig->enable
            && sdl_active_canvas->fullscreenconfig->mode == FULLSCREEN_MODE_CUSTOM) {
            video_viewport_resize(sdl_active_canvas, 1);
        }
    }
    return 0;
}

static int set_sdl_window_width(int w, void *param)
{
    if (w < 0) {
        return -1;
    }

    sdl_window_width = w;
    return 0;
}

static int set_sdl_window_height(int h, void *param)
{
    if (h < 0) {
        return -1;
    }

    sdl_window_height = h;
    return 0;
}

#if defined(HAVE_HWSCALE) || defined(USE_SDLUI2)
static int set_sdl_gl_aspect_mode(int v, void *param)
{
    int old_v = sdl_gl_aspect_mode;

    switch (v) {
        case SDL_ASPECT_MODE_OFF:
        case SDL_ASPECT_MODE_CUSTOM:
        case SDL_ASPECT_MODE_TRUE:
            break;
        default:
            return -1;
    }

    sdl_gl_aspect_mode = v;

    if (old_v != v) {
        if (sdl_active_canvas && sdl_active_canvas->videoconfig->hwscale) {
            video_viewport_resize(sdl_active_canvas, 1);
#ifdef USE_SDLUI2
            sdl_video_resize_event(sdl_active_canvas->actual_width, sdl_active_canvas->actual_height);
#endif
        }
    }

    return 0;
}

static int set_aspect_ratio(const char *val, void *param)
{
    double old_aspect = aspect_ratio;
    char buf[20];

    if (val) {
        char *endptr;

        util_string_set(&aspect_ratio_s, val);

        aspect_ratio = strtod(val, &endptr);
        if (val == endptr) {
            aspect_ratio = 1.0;
        } else if (aspect_ratio < 0.5) {
            aspect_ratio = 0.5;
        } else if (aspect_ratio > 2.0) {
            aspect_ratio = 2.0;
        }
    } else {
        aspect_ratio = 1.0;
    }

    sprintf(buf, "%f", aspect_ratio);
    util_string_set(&aspect_ratio_s, buf);

    if (old_aspect != aspect_ratio) {
        if (sdl_active_canvas && sdl_active_canvas->videoconfig->hwscale) {
            video_viewport_resize(sdl_active_canvas, 1);
#ifdef USE_SDLUI2
            sdl_video_resize_event(sdl_active_canvas->actual_width, sdl_active_canvas->actual_height);
#endif
        }
    }

    return 0;
}

#ifndef USE_SDLUI2
static void update_vertex_base(void)
{
    sdl_gl_vertex_base = (sdl_gl_flipx << 2) | (sdl_gl_flipy << 3);
}
#endif

static int set_sdl_gl_flipx(int v, void *param)
{
    sdl_gl_flipx = v ? 1 : 0;

#ifdef USE_SDLUI2
    if (sdl_gl_flipx) {
        flip |= SDL_FLIP_HORIZONTAL;
    } else {
        flip &= ~SDL_FLIP_HORIZONTAL;
    }
#else
    update_vertex_base();
#endif

    return 0;
}

static int set_sdl_gl_flipy(int v, void *param)
{
    sdl_gl_flipy = v ? 1 : 0;

#ifdef USE_SDLUI2
    if (sdl_gl_flipy) {
        flip |= SDL_FLIP_VERTICAL;
    } else {
        flip &= ~SDL_FLIP_VERTICAL;
    }
#else
    update_vertex_base();
#endif

    return 0;
}

static int set_sdl_gl_filter(int v, void *param)
{
    switch (v) {
        case SDL_FILTER_NEAREST:
            sdl_gl_filter = GL_NEAREST;
            break;

        case SDL_FILTER_LINEAR:
            sdl_gl_filter = GL_LINEAR;
            break;

        default:
            return -1;
    }

    sdl_gl_filter_res = v;
    return 0;
}
#endif /* HAVE_HWSCALE */

#ifdef USE_SDLUI2
static int set_sdl2_renderer_name(const char *val, void *param)
{
    if (!val || val[0] == '\0') {
        util_string_set(&sdl2_renderer_name, "");
    } else {
        util_string_set(&sdl2_renderer_name, val);
    }
    return 0;
}
#endif

static const resource_string_t resources_string[] = {
#if defined(HAVE_HWSCALE) || defined(USE_SDLUI2)
    { "AspectRatio", "1.0", RES_EVENT_NO, NULL,
      &aspect_ratio_s, set_aspect_ratio, NULL },
#endif
#ifdef USE_SDLUI2
    { "SDL2Renderer", "", RES_EVENT_NO, NULL,
      &sdl2_renderer_name, set_sdl2_renderer_name, NULL },
#endif
    RESOURCE_STRING_LIST_END
};

#if defined(WATCOM_COMPILE) || defined (USE_SDLUI2)
#define VICE_DEFAULT_BITDEPTH 32
#else
#define VICE_DEFAULT_BITDEPTH 0
#endif

#ifdef ANDROID_COMPILE
#define SDLLIMITMODE_DEFAULT     SDL_LIMIT_MODE_MAX
#define SDLCUSTOMWIDTH_DEFAULT   320
#define SDLCUSTOMHEIGHT_DEFAULT  200
#else
#define SDLLIMITMODE_DEFAULT     SDL_LIMIT_MODE_OFF
#define SDLCUSTOMWIDTH_DEFAULT   800
#define SDLCUSTOMHEIGHT_DEFAULT  600
#endif

static const resource_int_t resources_int[] = {
    { "SDLBitdepth", VICE_DEFAULT_BITDEPTH, RES_EVENT_NO, NULL,
      &sdl_bitdepth, set_sdl_bitdepth, NULL },
    { "SDLLimitMode", SDLLIMITMODE_DEFAULT, RES_EVENT_NO, NULL,
      &sdl_limit_mode, set_sdl_limit_mode, NULL },
    { "SDLCustomWidth", SDLCUSTOMWIDTH_DEFAULT, RES_EVENT_NO, NULL,
      &sdl_custom_width, set_sdl_custom_width, NULL },
    { "SDLCustomHeight", SDLCUSTOMHEIGHT_DEFAULT, RES_EVENT_NO, NULL,
      &sdl_custom_height, set_sdl_custom_height, NULL },
    { "SDLWindowWidth", 0, RES_EVENT_NO, NULL,
      &sdl_window_width, set_sdl_window_width, NULL },
    { "SDLWindowHeight", 0, RES_EVENT_NO, NULL,
      &sdl_window_height, set_sdl_window_height, NULL },
#if defined(HAVE_HWSCALE) || defined(USE_SDLUI2)
    { "SDLGLAspectMode", SDL_ASPECT_MODE_TRUE, RES_EVENT_NO, NULL,
      &sdl_gl_aspect_mode, set_sdl_gl_aspect_mode, NULL },
    { "SDLGLFlipX", 0, RES_EVENT_NO, NULL,
      &sdl_gl_flipx, set_sdl_gl_flipx, NULL },
    { "SDLGLFlipY", 0, RES_EVENT_NO, NULL,
      &sdl_gl_flipy, set_sdl_gl_flipy, NULL },
    { "SDLGLFilter", SDL_FILTER_LINEAR, RES_EVENT_NO, NULL,
      &sdl_gl_filter_res, set_sdl_gl_filter, NULL },
#endif
    RESOURCE_INT_LIST_END
};

int video_arch_resources_init(void)
{
    DBG(("%s", __func__));

    if (machine_class == VICE_MACHINE_VSID) {
        if (joy_arch_resources_init() < 0) {
            return -1;
        }
    }

    if (resources_register_string(resources_string) < 0) {
        return -1;
    }

    return resources_register_int(resources_int);
}

void video_arch_resources_shutdown(void)
{
    DBG(("%s", __func__));

    if (machine_class == VICE_MACHINE_VSID) {
        joy_arch_resources_shutdown();
    }

#if defined(HAVE_HWSCALE) || defined(USE_SDLUI2)
    lib_free(aspect_ratio_s);
#endif


#ifdef USE_SDLUI2
    lib_free(sdl2_renderer_name);
#endif
}

/* ------------------------------------------------------------------------- */
/* Video-related command-line options.  */

static const cmdline_option_t cmdline_options[] = {
    { "-sdlbitdepth", SET_RESOURCE, 1, NULL, NULL, "SDLBitdepth", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<bpp>", "Set bitdepth (0 = current, 8, 15, 16, 24, 32)" },
    { "-sdllimitmode", SET_RESOURCE, 1, NULL, NULL, "SDLLimitMode", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<mode>", "Set resolution limiting mode (0 = off, 1 = max, 2 = fixed)" },
    { "-sdlcustomw", SET_RESOURCE, 1, NULL, NULL, "SDLCustomWidth", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<width>", "Set custom resolution width" },
    { "-sdlcustomh", SET_RESOURCE, 1, NULL, NULL, "SDLCustomHeight", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<height>", "Set custom resolution height" },
#if defined(HAVE_HWSCALE) || defined(USE_SDLUI2)
    { "-sdlaspectmode", SET_RESOURCE, 1, NULL, NULL, "SDLGLAspectMode", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<mode>", "Set aspect ratio mode (0 = off, 1 = custom, 2 = true)" },
    { "-aspect", SET_RESOURCE, 1, NULL, NULL, "AspectRatio", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<aspect ratio>", "Set custom aspect ratio (0.5 - 2.0)" },
    { "-sdlflipx", SET_RESOURCE, 0, NULL, NULL, "SDLGLFlipX", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Enable X flip" },
    { "+sdlflipx", SET_RESOURCE, 0, NULL, NULL, "SDLGLFlipX", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Disable X flip" },
    { "-sdlflipy", SET_RESOURCE, 0, NULL, NULL, "SDLGLFlipY", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Enable Y flip" },
    { "+sdlflipy", SET_RESOURCE, 0, NULL, NULL, "SDLGLFlipY", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Disable Y flip" },
    { "-sdlglfilter", SET_RESOURCE, 1, NULL, NULL, "SDLGLFilter", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<mode>", "Set OpenGL filtering mode (0 = nearest, 1 = linear)" },
#endif
#ifdef USE_SDLUI2
    { "-sdl2renderer", SET_RESOURCE, 1, NULL, NULL, "SDL2Renderer", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<renderer name>", "Set the preferred SDL2 renderer" },
#endif
    CMDLINE_LIST_END
};

int video_arch_cmdline_options_init(void)
{
    DBG(("%s", __func__));

    if (machine_class == VICE_MACHINE_VSID) {
        if (joystick_cmdline_options_init() < 0) {
            return -1;
        }
    }

    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

int video_init(void)
{
    sdlvideo_log = log_open("SDLVideo");
    return 0;
}

void video_shutdown(void)
{
    DBG(("%s", __func__));

    if (draw_buffer_vsid) {
        lib_free(draw_buffer_vsid);
    }

    sdl_active_canvas = NULL;
}

/* ------------------------------------------------------------------------- */
/* static helper functions */

static int sdl_video_canvas_limit(unsigned int limit_w, unsigned int limit_h, unsigned int *w, unsigned int *h, int mode)
{
    DBG(("%s", __func__));
    switch (mode & 3) {
        case SDL_LIMIT_MODE_MAX:
            if ((*w > limit_w) || (*h > limit_h)) {
                *w = MIN(*w, limit_w);
                *h = MIN(*h, limit_h);
                return 1;
            }
            break;
        case SDL_LIMIT_MODE_FIXED:
            if ((*w != limit_w) || (*h != limit_h)) {
                *w = limit_w;
                *h = limit_h;
                return 1;
            }
            break;
        case SDL_LIMIT_MODE_OFF:
        default:
            break;
    }
    return 0;
}

#if defined(HAVE_HWSCALE) || defined(USE_SDLUI2)
static void sdl_gl_set_viewport(unsigned int src_w, unsigned int src_h, unsigned int dest_w, unsigned int dest_h)
{
    int dest_x = 0, dest_y = 0;

    if (sdl_gl_aspect_mode != SDL_ASPECT_MODE_OFF) {
        double aspect = aspect_ratio;

        /* Get "true" aspect ratio */
        if (sdl_gl_aspect_mode == SDL_ASPECT_MODE_TRUE) {
            aspect = sdl_active_canvas->geometry->pixel_aspect_ratio;
        }

        /* Keep aspect ratio of src image. */
        if (dest_w * src_h < src_w * aspect * dest_h) {
            dest_y = dest_h;
            dest_h = (unsigned int)(dest_w * src_h / (src_w * aspect) + 0.5);
            dest_y = (dest_y - dest_h) / 2;
        } else {
            dest_x = dest_w;
            dest_w = (unsigned int)(dest_h * src_w * aspect / src_h + 0.5);
            dest_x = (dest_x - dest_w) / 2;
        }
    }

    /* Update lightpen adjustment parameters */
    sdl_lightpen_adjust.offset_x = dest_x;
    sdl_lightpen_adjust.offset_y = dest_y;

    sdl_lightpen_adjust.max_x = dest_w;
    sdl_lightpen_adjust.max_y = dest_h;

    sdl_lightpen_adjust.scale_x = (double)(src_w) / (double)(dest_w);
    sdl_lightpen_adjust.scale_y = (double)(src_h) / (double)(dest_h);

#ifndef USE_SDLUI2
    glViewport(dest_x, dest_y, dest_w, dest_h);
#else
    SDL_RenderSetLogicalSize(sdl_active_canvas->renderer, dest_w, dest_h);
#endif
}
#endif

#ifndef USE_SDLUI2
static video_canvas_t *sdl_canvas_create(video_canvas_t *canvas, unsigned int *width, unsigned int *height)
{
    SDL_Surface *new_screen;
    unsigned int new_width, new_height;
    unsigned int actual_width, actual_height;
    int flags;
    int fullscreen = 0;
    int limit = sdl_limit_mode;
    unsigned int limit_w = (unsigned int)sdl_custom_width;
    unsigned int limit_h = (unsigned int)sdl_custom_height;
    int hwscale = 0;
    int lightpen_updated = 0;
#ifdef HAVE_HWSCALE
    int rbits = 0, gbits = 0, bbits = 0;
    const Uint32

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000, gmask = 0x00ff0000, bmask = 0x0000ff00, amask = 0x000000ff;
#else
    rmask = 0x000000ff, gmask = 0x0000ff00, bmask = 0x00ff0000, amask = 0xff000000;
#endif
#endif

    DBG(("%s: %i,%i (%i)", __func__, *width, *height, canvas->index));

    flags = SDL_SWSURFACE | SDL_RESIZABLE;

    new_width = *width;
    new_height = *height;

    new_width *= canvas->videoconfig->scalex;
    new_height *= canvas->videoconfig->scaley;

    if ((canvas == sdl_active_canvas) && (canvas->fullscreenconfig->enable)) {
        fullscreen = 1;
    }

#ifdef HAVE_HWSCALE
    if ((canvas == sdl_active_canvas) && (canvas->videoconfig->hwscale)) {
        hwscale = 1;
    }
#endif

    if (fullscreen) {
        flags = SDL_FULLSCREEN | SDL_SWSURFACE;

        if (canvas->fullscreenconfig->mode == FULLSCREEN_MODE_CUSTOM) {
            limit = SDL_LIMIT_MODE_FIXED;
        }
    }

    if (!sdl_ui_finalized) { /* remember first size */
        double aspect = 1.0;
#ifdef HAVE_HWSCALE
        aspect = aspect_ratio;

        if (sdl_gl_aspect_mode == SDL_ASPECT_MODE_TRUE) {
            aspect = sdl_active_canvas->geometry->pixel_aspect_ratio;
        }
#endif
        sdl_active_canvas->real_width = (unsigned int)((double)new_width * aspect + 0.5);
        sdl_active_canvas->real_height = new_height;
        DBG(("first: %d:%d\n", sdl_active_canvas->real_width, sdl_active_canvas->real_height));
    }

#ifdef HAVE_HWSCALE
    if (hwscale) {
        flags |= SDL_OPENGL;

        if (fullscreen) {
            limit = SDL_LIMIT_MODE_OFF;
        } else {
            double aspect = aspect_ratio;

            if (sdl_gl_aspect_mode == SDL_ASPECT_MODE_TRUE) {
                aspect = sdl_active_canvas->geometry->pixel_aspect_ratio;
            }

            /* if no window geometry given then create one. */
            if (!sdl_window_width || !sdl_window_height) {
                limit_w = (unsigned int)((double)new_width * aspect + 0.5);
                limit_h = new_height;
            } else { /* full window size remembering when aspect ratio is not important */
                if (sdl_gl_aspect_mode == SDL_ASPECT_MODE_OFF) {
                    limit_w = (unsigned int)sdl_window_width;
                    limit_h = (unsigned int)sdl_window_height;
                } else { /* only remember height, set width according to that and the aspect ratio */
                    limit_h = (unsigned int)sdl_window_height;
                    limit_w = (unsigned int)((double)new_width * (double)sdl_window_height * aspect / (double)new_height + 0.5);
                }
            }
            limit = SDL_LIMIT_MODE_FIXED;
        }

        switch (sdl_bitdepth) {
            case 0:
                log_warning(sdlvideo_log, "bitdepth not set for OpenGL, trying 32...");
                sdl_bitdepth = 32;
            /* fall through */
            case 32:
                rbits = gbits = bbits = 8;
                sdl_gl_mode = GL_RGBA;
                break;
            case 24:
                rbits = gbits = bbits = 8;
                sdl_gl_mode = GL_RGB;
                break;
            default:
                log_error(sdlvideo_log, "%i bpp not supported in OpenGL.", sdl_bitdepth);
                resources_set_int("HwScalePossible", 0);
                hwscale = 0;
                canvas->videoconfig->hwscale = 0;
                flags = SDL_SWSURFACE;
                break;
        }

        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, rbits);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, gbits);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, bbits);
    }
#endif

    actual_width = new_width;
    actual_height = new_height;

    if (canvas == sdl_active_canvas) {
#ifdef HAVE_HWSCALE
        if (hwscale) {
            double aspect = aspect_ratio;

            if (sdl_gl_aspect_mode == SDL_ASPECT_MODE_TRUE) {
                aspect = sdl_active_canvas->geometry->pixel_aspect_ratio;
            }

            actual_width = (unsigned int)((double)actual_width * aspect + 0.5);
        }
#endif
        if (sdl_video_canvas_limit(limit_w, limit_h, &actual_width, &actual_height, limit)) {
            if (!hwscale) {
                canvas->draw_buffer->canvas_physical_width = actual_width;
                canvas->draw_buffer->canvas_physical_height = actual_height;
                video_viewport_resize(sdl_active_canvas, 0);
                if (sdl_ui_finalized) {
                    return canvas; /* exit here as video_viewport_resize will recall */
                }
            }
        }
    }

    if (canvas == sdl_active_canvas) {
#ifndef ANDROID_COMPILE
        SDL_EventState(SDL_VIDEORESIZE, SDL_IGNORE);
#endif
#ifndef HAVE_HWSCALE
        new_screen = SDL_SetVideoMode(actual_width, actual_height, sdl_bitdepth, flags);
        new_width = new_screen->w;
        new_height = new_screen->h;
#else
        if (hwscale) {
            /* To get fullscreen resolution, SetVideoMode must be called with the
               desired fullscreen resolution. If it is called with a smaller resolution,
               it will display the undesirable black borders around the emulator display. */
            if ((fullscreen) && (canvas->fullscreenconfig->mode == FULLSCREEN_MODE_CUSTOM)) {
                new_screen = SDL_SetVideoMode(limit_w, limit_h, sdl_bitdepth, flags);
            } else {
                new_screen = SDL_SetVideoMode(actual_width, actual_height, sdl_bitdepth, flags); 
            }
            if (!new_screen) { /* Did not work out quite well. Let's try without hwscale */
                resources_set_int("HwScalePossible", 0);
                canvas->videoconfig->hwscale = 0;
                return sdl_canvas_create(canvas, width, height);
            }
            actual_width = new_screen->w;
            actual_height = new_screen->h;

            /* free the old rendering surface when staying in hwscale mode */
            if ((canvas->hwscale_screen) && (canvas->screen)) {
                SDL_FreeSurface(canvas->screen);
            }

            canvas->hwscale_screen = new_screen;
            new_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, new_width, new_height, sdl_bitdepth, rmask, gmask, bmask, amask);
            sdl_gl_set_viewport(new_width, new_height, actual_width, actual_height);
            lightpen_updated = 1;
        } else {
            new_screen = SDL_SetVideoMode(actual_width, actual_height, sdl_bitdepth, flags);
            new_width = new_screen->w;
            new_height = new_screen->h;

            /* free the old rendering surface when leaving hwscale mode */
            if ((canvas->hwscale_screen) && (canvas->screen)) {
                SDL_FreeSurface(canvas->screen);
                SDL_FreeSurface(canvas->hwscale_screen);
                canvas->hwscale_screen = NULL;
            }
        }
#endif
#ifndef ANDROID_COMPILE
        SDL_EventState(SDL_VIDEORESIZE, SDL_ENABLE);
#endif
    } else {
#ifdef HAVE_HWSCALE
        /* free the old hwscale screen when hwscaled screen is switched away */
        if (canvas->hwscale_screen) {
            SDL_FreeSurface(canvas->hwscale_screen);
            canvas->hwscale_screen = NULL;
        }
        if (!hwscale) {
            new_width = actual_width;
            new_height = actual_height;
        }
#else
        new_width = actual_width;
        new_height = actual_height;
#endif
        if (canvas->screen) {
            SDL_FreeSurface(canvas->screen);
        }
        new_screen = SDL_CreateRGBSurface(SDL_SWSURFACE, new_width, new_height, sdl_bitdepth, 0, 0, 0, 0);
    }

    if (!new_screen) {
        log_error(sdlvideo_log, "SDL_SetVideoMode failed!");
        return NULL;
    }
    sdl_bitdepth = new_screen->format->BitsPerPixel;

    canvas->depth = sdl_bitdepth;
    canvas->width = new_width;
    canvas->height = new_height;
    canvas->screen = new_screen;
    canvas->actual_width = actual_width;
    canvas->actual_height = actual_height;

    if (canvas == sdl_active_canvas) {
        if (!fullscreen) {
            resources_set_int("SDLWindowWidth", actual_width);
            resources_set_int("SDLWindowHeight", actual_height);
        }
    }

    log_message(sdlvideo_log, "%s (%s) %ix%i %ibpp %s%s", canvas->videoconfig->chip_name, (canvas == sdl_active_canvas) ? "active" : "inactive", actual_width, actual_height, sdl_bitdepth, hwscale ? "OpenGL " : "", (canvas->fullscreenconfig->enable) ? "(fullscreen)" : "");
#ifdef SDL_DEBUG
    log_message(sdlvideo_log, "Canvas %ix%i, real %ix%i", new_width, new_height, canvas->real_width, canvas->real_height);
#endif

    /* Update lightpen adjustment parameters */
    if (canvas == sdl_active_canvas && !lightpen_updated) {
        sdl_lightpen_adjust.max_x = actual_width;
        sdl_lightpen_adjust.max_y = actual_height;

        sdl_lightpen_adjust.scale_x = (double)*width / (double)actual_width;
        sdl_lightpen_adjust.scale_y = (double)*height / (double)actual_height;
    }

    video_canvas_set_palette(canvas, canvas->palette);

    return canvas;
}
#else
static video_canvas_t *sdl_canvas_create(video_canvas_t *canvas, unsigned int *width, unsigned int *height)
{
    unsigned int new_width, new_height;
    unsigned int actual_width, actual_height;
    int flags;
    int fullscreen = 0;
    int limit = sdl_limit_mode;
    unsigned int limit_w = (unsigned int)sdl_custom_width;
    unsigned int limit_h = (unsigned int)sdl_custom_height;
    int lightpen_updated = 0;
    int it;
    int l;
    int drv_index = -1;
    double aspect = 1.0;
    char rendername[256] = { 0 };
    char **renderlist = NULL;
    char *gl_string;
    int renderamount = SDL_GetNumRenderDrivers();
    unsigned int window_h = 0;
    unsigned int window_w = 0;
    int temp_h = 0;
    int temp_w = 0;
    SDL_GLContext ctx;
    SDL_RendererInfo info;

    DBG(("%s: %i,%i (%i)", __func__, *width, *height, canvas->index));

    aspect = aspect_ratio;

    new_width = *width;
    new_height = *height;

    new_width *= canvas->videoconfig->scalex;
    new_height *= canvas->videoconfig->scaley;

    if ((canvas == sdl_active_canvas) && (canvas->fullscreenconfig->enable)) {
        fullscreen = 1;
    }

    if (fullscreen) {
        if (canvas->fullscreenconfig->mode == FULLSCREEN_MODE_CUSTOM) {
            flags = SDL_WINDOW_FULLSCREEN;
        } else {
            flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
    } else {
        flags = SDL_WINDOW_RESIZABLE;
    }

    if (sdl_gl_aspect_mode == SDL_ASPECT_MODE_TRUE && !fullscreen) {
        aspect = sdl_active_canvas->geometry->pixel_aspect_ratio;
    }

    if (sdl_video_canvas_limit(limit_w, limit_h, &new_width, &new_height, limit)) {
        canvas->draw_buffer->canvas_physical_width = new_width;
        canvas->draw_buffer->canvas_physical_height = new_height;
        video_viewport_resize(sdl_active_canvas, 0);
        if (sdl_ui_finalized) {
            return canvas; /* exit here as video_viewport_resize will recall */
        }
    }

    if (!sdl_ui_finalized) { /* remember first size */
        sdl_active_canvas->real_width = (unsigned int)((double)new_width * aspect + 0.5);
        sdl_active_canvas->real_height = new_height;
        DBG(("first: %d:%d\n", sdl_active_canvas->real_width, sdl_active_canvas->real_height));
    }

    actual_width = new_width;
    actual_height = new_height;

    if (!fullscreen) {
        /* if no window geometry given then create one. */
        if (!sdl_window_width || !sdl_window_height) {
            window_w = sdl_window_width = (unsigned int)((double)new_width * aspect + 0.5);
            window_h = sdl_window_height = new_height;
        } else { /* full window size remembering when aspect ratio is not important */
            window_w = (unsigned int)sdl_window_width;
            window_h = (unsigned int)sdl_window_height;
        }
    }

    if (new_window) {
        if (new_screen) {
            SDL_FreeSurface(new_screen);
            new_screen = NULL;
        }
        if (new_texture) {
            SDL_DestroyTexture(new_texture);
            new_texture = NULL;
        }
        if (new_renderer) {
            SDL_DestroyRenderer(new_renderer);
            new_renderer = NULL;
        }
        SDL_DestroyWindow(new_window);
        new_window = NULL;
    }

    /* Obtain the Window with the corresponding size and behavior based on the flags */
    new_window = SDL_CreateWindow(canvas->viewport->title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, SDL_WINDOW_OPENGL | flags);
    if (new_window == NULL) {
        log_error(sdlvideo_log, "SDL_CreateWindow() failed: %s\n", SDL_GetError());
        return NULL;
    }

    ctx = SDL_GL_CreateContext(new_window);
    SDL_GL_MakeCurrent(new_window, ctx);

    /* Allocate renderlist strings */
    renderlist = lib_malloc((renderamount + 1) * sizeof(char *));

    /* Fill in the renderlist and render info string */
    for (it = 0; it < renderamount; ++it) {
        SDL_GetRenderDriverInfo(it, &info);

        strcat(rendername, info.name);
        strcat(rendername, " ");
        renderlist[it] = lib_stralloc(info.name);
    }
    renderlist[it] = NULL;
        
    /* Check for resource preferred renderer */
    if (sdl2_renderer_name != NULL && *sdl2_renderer_name != '\0') {
        for (it = 0; it < renderamount; ++it) {
            if (!strcmp(sdl2_renderer_name, renderlist[it])) {
                drv_index = it;
            }
        }
        if (drv_index == -1) {
            log_warning(sdlvideo_log, "Resource preferred renderer %s not available, trying arch default renderer(s)", sdl2_renderer_name);
        }
    }

    /* Try arch default renderer(s) if the resource preferred renderer was not available */
    for (l = 0; drv_index == -1 && archdep_sdl2_default_renderers[l]; ++l) {
        for (it = 0; it < renderamount; ++it) {
            if (!strcmp(archdep_sdl2_default_renderers[l], renderlist[it])) {
                drv_index = it;
            }
        }
    }

    for (l = 0; l < renderamount; ++l) {
        lib_free(renderlist[l]);
    }
    lib_free(renderlist);
    renderlist = NULL;

    log_message(sdlvideo_log, "Available Renderers: %s", rendername);

    if (new_window) {
        new_renderer = SDL_CreateRenderer(new_window, drv_index, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (new_renderer) {
            SDL_SetRenderDrawColor(new_renderer, 0, 0, 0, 255);
            SDL_RenderClear(new_renderer);
            SDL_RenderPresent(new_renderer);
            new_screen = SDL_CreateRGBSurface(0, actual_width, actual_height, sdl_bitdepth, rmask, gmask, bmask, amask);
            if (fullscreen) {
                SDL_RenderSetLogicalSize(new_renderer, actual_width, actual_height);
            }
            if (new_screen) {
                SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
                new_texture = SDL_CreateTexture(new_renderer, texformat, SDL_TEXTUREACCESS_STREAMING, actual_width, actual_height);
                if (!new_texture) {
                    SDL_FreeSurface(new_screen);
                    new_screen = NULL;
                    SDL_DestroyRenderer(new_renderer);
                    new_renderer = NULL;
                    SDL_DestroyWindow(new_window);
                    new_window = NULL;
                }
            } else {
                SDL_DestroyRenderer(new_renderer);
                new_renderer = NULL;
                SDL_DestroyWindow(new_window);
                new_window = NULL;
            }
        } else {
            SDL_DestroyWindow(new_window);
            new_window = NULL;
            new_screen = NULL;
        }
    } else {
        log_error(sdlvideo_log, "SDL_CreateWindow failed!");
        return NULL;
    }

    /* here SDL2 knows about what driver is used and has that loaded via dlopen (default behavior), 
       NOW we can get the proc adress from opengl/es/1/2 functions in there */
    glGetStringAPI = (glGetString_Func)SDL_GL_GetProcAddress("glGetString");

    gl_string = (char *)glGetStringAPI(GL_VENDOR);
    log_message(sdlvideo_log, "Vendor     : %s", gl_string != NULL ? gl_string : "Unknown");
    gl_string = (char *)glGetStringAPI(GL_RENDERER);
    log_message(sdlvideo_log, "Renderer   : %s", gl_string != NULL ? gl_string : "Unknown");
    gl_string = (char *)glGetStringAPI(GL_VERSION);
    log_message(sdlvideo_log, "Version    : %s", gl_string != NULL ? gl_string : "Unknown");
#ifdef SDL_DEBUG
    gl_string = (char *)glGetStringAPI(GL_EXTENSIONS);
    log_message(sdlvideo_log, "Extensions : %s", gl_string != NULL ? gl_string : "Unknown");
#endif

    /* some devices, OS do not provide a windowing system they have always a fixed width/height, 
       check if our desired window size is different from real size, needed by apect ratio */
    SDL_GetWindowSize(new_window, &temp_w, &temp_h);
    if (temp_w != window_w && temp_h != window_h && !fullscreen) {
        sdl_window_width = (unsigned int)temp_w;
        sdl_window_height = (unsigned int)temp_h;
    }

    sdl_bitdepth = new_screen->format->BitsPerPixel;
    actual_width = new_screen->w;
    actual_height = new_screen->h;

    canvas->depth = sdl_bitdepth;
    canvas->width = new_width;
    canvas->height = new_height;
    canvas->screen = new_screen;
    canvas->window = new_window;
    canvas->renderer = new_renderer;
    canvas->texture = new_texture;
    canvas->actual_width = actual_width;
    canvas->actual_height = actual_height;
    canvas->videoconfig->hwscale = 1;

    SDL_GetRenderDriverInfo(drv_index, &info);

    log_message(sdlvideo_log, "%s (%s) %ix%i %ibpp using %s%s", canvas->videoconfig->chip_name, (canvas == sdl_active_canvas) ? "active" : "inactive", actual_width, actual_height, sdl_bitdepth, info.name, (canvas->fullscreenconfig->enable) ? " (fullscreen)" : "");
#ifdef SDL_DEBUG
    log_message(sdlvideo_log, "Canvas %ix%i, real %ix%i", new_width, new_height, canvas->real_width, canvas->real_height);
#endif

    /* Update lightpen adjustment parameters */
    if (!lightpen_updated) {
        sdl_lightpen_adjust.max_x = actual_width;
        sdl_lightpen_adjust.max_y = actual_height;

        sdl_lightpen_adjust.scale_x = (double)*width / (double)actual_width;
        sdl_lightpen_adjust.scale_y = (double)*height / (double)actual_height;
    }

    video_canvas_set_palette(canvas, canvas->palette);

    if ((sdl_window_width || sdl_window_height) && !fullscreen) {
        SDL_Event sdlevent;
        sdlevent.type = SDL_WINDOWEVENT;
        sdlevent.window.event = SDL_WINDOWEVENT_RESIZED;
        sdlevent.window.data1 = sdl_window_width;
        sdlevent.window.data2 = sdl_window_height;

        SDL_PushEvent(&sdlevent);
    }

    /* Enable file/text drag and drop support */
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

    return canvas;
}
#endif

/* ------------------------------------------------------------------------- */
/* Main API */

/* called from raster/raster.c:realize_canvas */
video_canvas_t *video_canvas_create(video_canvas_t *canvas, unsigned int *width, unsigned int *height, int mapped)
{
    /* nothing to do here, the real work is done in sdl_ui_init_finalize */
    return canvas;
}

void video_canvas_refresh(struct video_canvas_s *canvas, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h)
{
    uint8_t *backup;

    if ((canvas == NULL) || (canvas->screen == NULL) || (canvas != sdl_active_canvas)) {
        return;
    }

    if (sdl_vsid_state & SDL_VSID_ACTIVE) {
        sdl_vsid_draw();
    }

    if (sdl_vkbd_state & SDL_VKBD_ACTIVE) {
        sdl_vkbd_draw();
    }

    if (uistatusbar_state & UISTATUSBAR_ACTIVE) {
        uistatusbar_draw();
    }

    xi *= canvas->videoconfig->scalex;
    w *= canvas->videoconfig->scalex;

    yi *= canvas->videoconfig->scaley;
    h *= canvas->videoconfig->scaley;

    w = MIN(w, canvas->width);
    h = MIN(h, canvas->height);

    /* FIXME attempt to draw outside canvas */
    if ((xi + w > canvas->width) || (yi + h > canvas->height)) {
        return;
    }

#ifndef USE_SDLUI2
    if (SDL_MUSTLOCK(canvas->screen)) {
        canvas->videoconfig->readable = 0;
        if (SDL_LockSurface(canvas->screen) < 0) {
            return;
        }
    } else { /* no direct rendering, safe to read */
        canvas->videoconfig->readable = !(canvas->screen->flags & SDL_HWSURFACE);
    }
#endif

    if (machine_class == VICE_MACHINE_VSID) {
        canvas->draw_buffer_vsid->draw_buffer_width = canvas->draw_buffer->draw_buffer_width;
        canvas->draw_buffer_vsid->draw_buffer_height = canvas->draw_buffer->draw_buffer_height;
        canvas->draw_buffer_vsid->draw_buffer_pitch = canvas->draw_buffer->draw_buffer_pitch;
        canvas->draw_buffer_vsid->canvas_physical_width = canvas->draw_buffer->canvas_physical_width;
        canvas->draw_buffer_vsid->canvas_physical_height = canvas->draw_buffer->canvas_physical_height;
        canvas->draw_buffer_vsid->canvas_width = canvas->draw_buffer->canvas_width;
        canvas->draw_buffer_vsid->canvas_height = canvas->draw_buffer->canvas_height;
        canvas->draw_buffer_vsid->visible_width = canvas->draw_buffer->visible_width;
        canvas->draw_buffer_vsid->visible_height = canvas->draw_buffer->visible_height;

        backup = canvas->draw_buffer->draw_buffer;
        canvas->draw_buffer->draw_buffer = canvas->draw_buffer_vsid->draw_buffer;
        video_canvas_render(canvas, (uint8_t *)canvas->screen->pixels, w, h, xs, ys, xi, yi, canvas->screen->pitch, canvas->screen->format->BitsPerPixel);
        canvas->draw_buffer->draw_buffer = backup;
    } else {
        video_canvas_render(canvas, (uint8_t *)canvas->screen->pixels, w, h, xs, ys, xi, yi, canvas->screen->pitch, canvas->screen->format->BitsPerPixel);
    }

#ifndef USE_SDLUI2
    if (SDL_MUSTLOCK(canvas->screen)) {
        SDL_UnlockSurface(canvas->screen);
    }
#else
    SDL_UpdateTexture(canvas->texture, NULL, canvas->screen->pixels, canvas->screen->pitch);
    SDL_RenderClear(canvas->renderer);
    SDL_RenderCopyEx(canvas->renderer, canvas->texture, NULL, NULL, 0, NULL, flip);
    SDL_RenderPresent(canvas->renderer);
#endif

#if defined(HAVE_HWSCALE) && !defined(USE_SDLUI2)
    if (canvas->videoconfig->hwscale) {
        const float *v = &(sdl_gl_vertex_coord[sdl_gl_vertex_base]);

        if (canvas != sdl_active_canvas) {
            DBG(("%s: not active SDL canvas, ignoring", __func__));
            return;
        }

        if (!(canvas->hwscale_screen)) {
            DBG(("%s: hwscale refresh without hwscale screen, ignoring", __func__));
            return;
        }

/* XXX make use of glXBindTexImageEXT aka texture from pixmap extension */

        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);

/* GL_TEXTURE_RECTANGLE is standardised as _EXT in OpenGL 1.4. Here's some
 * aliases in the meantime. */
#ifndef GL_TEXTURE_RECTANGLE_EXT
    #if defined(GL_TEXTURE_RECTANGLE_NV)
        #define GL_TEXTURE_RECTANGLE_EXT GL_TEXTURE_RECTANGLE_NV
    #elif defined(GL_TEXTURE_RECTANGLE_ARB)
        #define GL_TEXTURE_RECTANGLE_EXT GL_TEXTURE_RECTANGLE_ARB
    #else
        #error "Your headers do not supply GL_TEXTURE_RECTANGLE. Disable HWSCALE and try again."
    #endif
#endif

        glEnable(GL_TEXTURE_RECTANGLE_EXT);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, screen_texture);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, sdl_gl_filter);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, sdl_gl_filter);
        glTexImage2D (GL_TEXTURE_RECTANGLE_EXT, 0, sdl_gl_mode, canvas->width, canvas->height, 0, sdl_gl_mode, GL_UNSIGNED_BYTE, canvas->screen->pixels);

        glBegin(GL_QUADS);

        /* Lower Right Of Texture */
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(v[0], v[1]);

        /* Upper Right Of Texture */
        glTexCoord2f(0.0f, (float)(canvas->height));
        glVertex2f(v[0], v[2]);

        /* Upper Left Of Texture */
        glTexCoord2f((float)(canvas->width), (float)(canvas->height));
        glVertex2f(v[3], v[2]);

        /* Lower Left Of Texture */
        glTexCoord2f((float)(canvas->width), 0.0f);
        glVertex2f(v[3], v[1]);

        glEnd();

        SDL_GL_SwapBuffers();
    } else
#endif

#ifndef USE_SDLUI2
    SDL_UpdateRect(canvas->screen, xi, yi, w, h);
#endif
}

int video_canvas_set_palette(struct video_canvas_s *canvas, struct palette_s *palette)
{
    unsigned int i, col = 0;
    SDL_PixelFormat *fmt;
#ifndef USE_SDLUI2
    SDL_Color colors[256];
#endif

    DBG(("video_canvas_set_palette canvas: %p", canvas));

    if (palette == NULL) {
        return 0; /* no palette, nothing to do */
    }

    canvas->palette = palette;

    fmt = canvas->screen->format;

    /* Fixme: needs further investigation how it can reach here without being fully initialized */
    if (canvas != sdl_active_canvas || canvas->width != canvas->screen->w) {
        DBG(("video_canvas_set_palette not active canvas or window not created, don't update hw palette"));
        return 0;
    }

    for (i = 0; i < palette->num_entries; i++) {
#ifndef USE_SDLUI2
        if (canvas->depth == 8) {
            colors[i].r = palette->entries[i].red;
            colors[i].b = palette->entries[i].blue;
            colors[i].g = palette->entries[i].green;
            col = i;
        } else {
#else
        if (canvas->depth % 8 == 0) {
#endif
            col = SDL_MapRGB(fmt, palette->entries[i].red, palette->entries[i].green, palette->entries[i].blue);
        }
        video_render_setphysicalcolor(canvas->videoconfig, i, col, canvas->depth);
    }

#ifndef USE_SDLUI2
    if (canvas->depth == 8) {
        SDL_SetColors(canvas->screen, colors, 0, palette->num_entries);
    } else {
#else
    if (canvas->depth % 8 == 0) {
#endif
        for (i = 0; i < 256; i++) {
            video_render_setrawrgb(i, SDL_MapRGB(fmt, (Uint8)i, 0, 0), SDL_MapRGB(fmt, 0, (Uint8)i, 0), SDL_MapRGB(fmt, 0, 0, (Uint8)i));
        }
        video_render_initraw(canvas->videoconfig);
    }

    return 0;
}

/* called from video_viewport_resize */
void video_canvas_resize(struct video_canvas_s *canvas, char resize_canvas)
{
    unsigned int width = canvas->draw_buffer->canvas_width;
    unsigned int height = canvas->draw_buffer->canvas_height;
    DBG(("%s: %ix%i (%i)", __func__, width, height, canvas->index));
    /* Check if canvas needs to be resized to real size first */
    if (sdl_ui_finalized) {
        /* NOTE: setting the resources to zero like this here would actually
                 not only force a recalculation of the resources, but also
                 result in the window size being recalculated from the default
                 dimensions instead of the (saved and supposed to be persistant)
                 values in the resources. what goes wrong when this is done can
                 be observed when x128 starts up.
            FIXME: remove this note and code below after some testing. hopefully
                   nothing else relies on the broken behavior...
         */
#if 0
        sdl_window_width = 0; /* force recalculate */
        sdl_window_height = 0;
#endif
        sdl_canvas_create(canvas, &width, &height); /* set the real canvas size */

        if (resize_canvas) {
            DBG(("%s: set and resize to real size (%ix%i)", __func__, width, height));
            canvas->real_width = canvas->actual_width;
            canvas->real_height = canvas->actual_height;
        }
    }
}

/* Resize window to w/h. */
static void sdl_video_resize(unsigned int w, unsigned int h)
{
    DBG(("%s: %ix%i", __func__, w, h));

    if ((w == 0) || (h == 0)) {
        DBG(("%s: ERROR, ignored!", __func__));
        return;
    }

    vsync_suspend_speed_eval();

#if defined(HAVE_HWSCALE) && !defined(USE_SDLUI2)
    if (sdl_active_canvas->videoconfig->hwscale && sdl_active_canvas->hwscale_screen) {
        int flags;

        if (sdl_active_canvas->fullscreenconfig->enable) {
            flags = SDL_OPENGL | SDL_SWSURFACE | SDL_FULLSCREEN;
        } else {
            flags = SDL_OPENGL | SDL_SWSURFACE | SDL_RESIZABLE;
        }

#ifndef ANDROID_COMPILE
        SDL_EventState(SDL_VIDEORESIZE, SDL_IGNORE);
#endif
        sdl_active_canvas->hwscale_screen = SDL_SetVideoMode((int)w, (int)h, sdl_bitdepth, flags);
#ifndef ANDROID_COMPILE
        SDL_EventState(SDL_VIDEORESIZE, SDL_ENABLE);
#endif

#ifdef SDL_DEBUG
        if (!sdl_active_canvas->hwscale_screen) {
            DBG(("%s: setting video mode failed", __func__));
        }
#endif
        sdl_gl_set_viewport(sdl_active_canvas->width, sdl_active_canvas->height, w, h);
        sdl_active_canvas->actual_width = w;
        sdl_active_canvas->actual_height = h;
    } else
#endif /*  HAVE_HWSCALE */
    {
#ifdef USE_SDLUI2
        sdl_gl_set_viewport(sdl_active_canvas->width, sdl_active_canvas->height, w, h);
        sdl_active_canvas->actual_width = w;
        sdl_active_canvas->actual_height = h;
#else
        sdl_active_canvas->draw_buffer->canvas_physical_width = w;
        sdl_active_canvas->draw_buffer->canvas_physical_height = h;
        video_viewport_resize(sdl_active_canvas, 0);
#endif
    }
}

/* Resize window to stored real size */
void sdl_video_restore_size(void)
{
    unsigned int w, h;

    w = sdl_active_canvas->real_width;
    h = sdl_active_canvas->real_height;

    DBG(("%s: %ix%i->%ix%i", __func__, sdl_active_canvas->real_width, sdl_active_canvas->real_height, w, h));
    sdl_video_resize(w, h);
}

/* special case handling for the SDL window resize event */
void sdl_video_resize_event(unsigned int w, unsigned int h)
{
#if defined(HAVE_HWSCALE) || defined(USE_SDLUI2)

    DBG(("%s: %ix%i", __func__, w, h));
    if ((w == 0) || (h == 0)) {
        DBG(("%s: ERROR, ignored!", __func__));
        return;
    }
    sdl_video_resize(w, h);
    if (!sdl_active_canvas->fullscreenconfig->enable) {
        resources_set_int("SDLWindowWidth", sdl_active_canvas->actual_width);
        resources_set_int("SDLWindowHeight", sdl_active_canvas->actual_height);
    }

#endif /*  HAVE_HWSCALE */
}

void sdl_video_canvas_switch(int index)
{
    struct video_canvas_s *canvas;

    DBG(("%s: %i->%i", __func__, sdl_active_canvas_num, index));

    if (sdl_active_canvas_num == index) {
        return;
    }

    if (index >= sdl_num_screens) {
        return;
    }

#ifndef USE_SDLUI2
    if (sdl_canvaslist[index]->screen != NULL) {
        SDL_FreeSurface(sdl_canvaslist[index]->screen);
        sdl_canvaslist[index]->screen = NULL;
    }
#endif

    sdl_active_canvas_num = index;

    canvas = sdl_canvaslist[sdl_active_canvas_num];
    sdl_active_canvas = canvas;

    video_viewport_resize(canvas, 1);
}

void video_arch_canvas_init(struct video_canvas_s *canvas)
{
    DBG(("%s: (%p, %i)", __func__, canvas, sdl_num_screens));

    if (sdl_num_screens == MAX_CANVAS_NUM) {
        log_error(sdlvideo_log, "Too many canvases!");
        exit(-1);
    }

    canvas->video_draw_buffer_callback = NULL;

    canvas->fullscreenconfig = lib_calloc(1, sizeof(fullscreenconfig_t));

    if (sdl_active_canvas_num == sdl_num_screens) {
        sdl_active_canvas = canvas;
    }

    canvas->index = sdl_num_screens;

    sdl_canvaslist[sdl_num_screens++] = canvas;

    canvas->screen = NULL;
#if defined(HAVE_HWSCALE) && !defined(USE_SDLUI2)
    canvas->hwscale_screen = NULL;
#endif
    canvas->real_width = 0;
    canvas->real_height = 0;
}

void video_canvas_destroy(struct video_canvas_s *canvas)
{
    int i;

    DBG(("%s: (%p, %i)", __func__, canvas, canvas->index));

    for (i = 0; i < sdl_num_screens; ++i) {
        if ((sdl_canvaslist[i] == canvas) && (canvas == sdl_active_canvas)) {
            SDL_FreeSurface(sdl_canvaslist[i]->screen);
            sdl_canvaslist[i]->screen = NULL;
#ifdef USE_SDLUI2
            SDL_DestroyTexture(sdl_canvaslist[i]->texture);
            sdl_canvaslist[i]->texture = NULL;
            SDL_DestroyRenderer(sdl_canvaslist[i]->renderer);
            sdl_canvaslist[i]->renderer = NULL;
            SDL_DestroyWindow(sdl_canvaslist[i]->window);
            sdl_canvaslist[i]->window = NULL;
#endif
        }
    }

    lib_free(canvas->fullscreenconfig);
}

char video_canvas_can_resize(video_canvas_t *canvas)
{
    return 1;
}

void sdl_ui_init_finalize(void)
{
    unsigned int width = sdl_active_canvas->draw_buffer->canvas_width;
    unsigned int height = sdl_active_canvas->draw_buffer->canvas_height;

    sdl_canvas_create(sdl_active_canvas, &width, &height); /* set the real canvas size */
    sdl_ui_finalized = 1;
}
