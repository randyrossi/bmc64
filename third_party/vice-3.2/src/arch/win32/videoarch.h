/*
 * video.h - Video implementation for Win32, using DirectDraw.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef VICE_VIDEOARCH_H
#define VICE_VIDEOARCH_H

/*  Nasty patch to avoid defining BYTE, WORD, DWORD more than once.  */

#include <windows.h>

#include "types.h"

#if defined(HAVE_D3D9_H) && !defined(RC_INVOKED)
#include <d3d9.h>
#endif

#define CANVAS_USES_TRIPLE_BUFFERING(c) 0

struct palette_s;
struct video_draw_buffer_callback_s;

typedef struct video_canvas_s {
    unsigned int initialized;
    unsigned int created;
    char *title;
    int mapped;
    int depth;
    float refreshrate; /* currently displayed refresh rate */
    struct video_render_config_s *videoconfig;
    struct draw_buffer_s *draw_buffer;
    struct viewport_s *viewport;
    struct geometry_s *geometry;
    struct palette_s *palette;
    BITMAPINFO bmp_info;
    BYTE *pixels;
    HWND hwnd;
    HWND render_hwnd;
    HWND client_hwnd;
#ifdef HAVE_D3D9_H
    LPDIRECT3DDEVICE9 d3ddev;
    LPDIRECT3DSURFACE9 d3dsurface;
    D3DPRESENT_PARAMETERS d3dpp;
    RECT *dest_rect_ptr;
    RECT dest_rect;
#endif
    struct video_draw_buffer_callback_s *video_draw_buffer_callback;
} video_canvas_t;

/* ------------------------------------------------------------------------ */

const char *dd_error(HRESULT ddrval);

extern int video_set_palette(video_canvas_t *c);
extern int video_set_physical_colors(video_canvas_t *c);

extern int video_create_triple_surface(struct video_canvas_s *canvas, int width, int height);
extern int video_create_single_surface(struct video_canvas_s *canvas, int width, int height);

extern video_canvas_t *video_canvas_for_hwnd(HWND hwnd);
extern int video_canvas_nr_for_hwnd(HWND hwnd);
extern void video_canvas_add(video_canvas_t *canvas);

extern void video_canvas_update(HWND hwnd, HDC hdc, int xclient, int yclient, int w, int h);

extern int video_dx9_enabled(void);
extern int video_dx9_available(void);

/* DDraw functions */
extern video_canvas_t *video_canvas_create_ddraw(video_canvas_t *canvas);
extern void video_canvas_reset_ddraw(video_canvas_t *canvas);
extern void video_canvas_destroy_ddraw(video_canvas_t *canvas);
extern void video_canvas_refresh_ddraw(video_canvas_t *canvas, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h);
extern void video_canvas_set_palette_ddraw_8bit(video_canvas_t *canvas);
extern int video_set_physical_colors_ddraw(video_canvas_t *c);
extern DWORD video_get_color_from_palette_ddraw(video_canvas_t *c, unsigned int i);
extern void video_canvas_update_ddraw(HWND hwnd, HDC hdc, int xclient, int yclient, int w, int h);

/* DX9 functions */
extern int video_setup_dx9(void);
extern void video_shutdown_dx9(void);
extern int video_device_create_dx9(video_canvas_t *canvas, int fullscreen);
extern video_canvas_t *video_canvas_create_dx9(video_canvas_t *canvas, unsigned int *width, unsigned int *height);
extern void video_device_release_dx9(video_canvas_t *canvas);
extern HRESULT video_canvas_reset_dx9(video_canvas_t *canvas);
extern int video_canvas_refresh_dx9(video_canvas_t *canvas, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h);
extern void video_canvas_update_dx9(HWND hwnd, HDC hdc, int xclient, int yclient, int w, int h);

/* FIXME: ugly */
extern int fullscreen_enabled;
extern int dx_primary_surface_rendering;
#ifdef HAVE_D3D9_H
extern LPDIRECT3D9 d3d;
#endif

#if _MSC_VER > 1400 && _WIN32_WINNT >= 0x0501
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#endif
