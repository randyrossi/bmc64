/*
 * ui.c - Gnome based graphical user interface.  
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
 *  Heavily derived from Oliver Schaertel <orschaer@forwiss.uni-erlangen.de>
 *  GTK+ port.
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

/* #define DEBUG_GNOMEUI */

#include <string.h>
#include <stdlib.h>

#include "ui.h"
#include "uiarch.h"
#include "uicolor.h"

#include "fullscreenarch.h"
#include "log.h"
#include "cmdline.h"
#include "translate.h"
#include "resources.h"
#include "types.h"
#include "videoarch.h"
#include "video.h"
#include "machine.h"
#include "lib.h"
#ifdef USE_UI_THREADS
#include "ui-threads.h"
#endif

#ifdef HAVE_OPENGL_SYNC
#include "openGL_sync.h"
#endif

#define TS_TOUSEC(x) (x.tv_sec * 1000000L + (x.tv_nsec / 1000))

#ifdef DEBUG_GNOMEUI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

static log_t gnomevideo_log = LOG_ERR;

/* FIXME: a resize event should be triggered when any of these two is changed
          ui_trigger_resize() is a kind of ugly hack :)
*/
extern void ui_trigger_resize(void); /* src/arch/unix/x11/gnome/x11ui.c */

static int keepaspect, trueaspect;
static int set_keepaspect(int val, void *param)
{
    keepaspect = val ? 1 : 0;

    ui_trigger_resize();

    return 0;
}

static int set_trueaspect(int val, void *param)
{
    trueaspect = val ? 1 : 0;

    ui_trigger_resize();

    return 0;
}

static const resource_int_t resources_int[] = {
    { "KeepAspectRatio", 1, RES_EVENT_NO, NULL,
      &keepaspect, set_keepaspect, NULL },
    { "TrueAspectRatio", 1, RES_EVENT_NO, NULL,
      &trueaspect, set_trueaspect, NULL },
    RESOURCE_INT_LIST_END
};

int video_arch_resources_init(void)
{
#ifdef HAVE_OPENGL_SYNC
    openGL_register_resources();
#endif
    if (machine_class != VICE_MACHINE_VSID) {
        return resources_register_int(resources_int);
    }
    return 0;
}

void video_arch_resources_shutdown(void)
{
}

int video_init(void)
{
    if (gnomevideo_log == LOG_ERR) {
        gnomevideo_log = log_open("GnomeVideo");
    }
    return 0;
}

void video_shutdown(void)
{
}

static const cmdline_option_t cmdline_options[] = {
    { "-trueaspect", SET_RESOURCE, 0,
      NULL, NULL, "TrueAspectRatio", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Enable true aspect ratio") },
    { "+trueaspect", SET_RESOURCE, 0,
      NULL, NULL, "TrueAspectRatio", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Disable true aspect ratio") },
    { "-keepaspect", SET_RESOURCE, 0,
      NULL, NULL, "KeepAspectRatio", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Keep aspect ratio when scaling") },
    { "+keepaspect", SET_RESOURCE, 0,
      NULL, NULL, "KeepAspectRatio", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Do not keep aspect ratio when scaling (freescale)") },
    CMDLINE_LIST_END
};

int video_arch_cmdline_options_init(void)
{
    if (machine_class != VICE_MACHINE_VSID) {
        return cmdline_register_options(cmdline_options);
    }
    return 0;
}

/* called from video/video-canvas.c:video_canvas_init */
void video_arch_canvas_init(struct video_canvas_s *canvas)
{
    DBG(("video_arch_canvas_init %p", canvas));
    canvas->video_draw_buffer_callback = NULL;

#ifdef HAVE_FULLSCREEN
    if (machine_class != VICE_MACHINE_VSID) {
        canvas->fullscreenconfig = lib_calloc(1, sizeof(fullscreenconfig_t));
        fullscreen_init_alloc_hooks(canvas);
    }
#endif
}

/* called from raster/raster.c:realize_canvas */
video_canvas_t *video_canvas_create(video_canvas_t *canvas, unsigned int *width, unsigned int *height, int mapped)
{
    int res;
    DBG(("video_canvas_create %p", canvas));

#if !defined(HAVE_CAIRO)
    canvas->gdk_image = NULL;
#else
    canvas->gdk_pixbuf = NULL;
    /* canvas->cairo_ctx = gdk_cairo_create(gtk_widget_get_window(canvas->emuwindow)); */
#endif

#ifdef HAVE_HWSCALE
    canvas->hwscale_image = NULL;
#endif

    res = ui_open_canvas_window(canvas, canvas->viewport->title, *width, *height, 1);
    if (res < 0) {
        return NULL;
    }

#ifdef HAVE_OPENGL_SYNC
    openGL_sync_init(canvas);
#endif
    return canvas;
}

void video_canvas_destroy(video_canvas_t *canvas)
{
    DBG(("video_canvas_destroy %p", canvas));

    if (canvas) {
#ifdef HAVE_FULLSCREEN
        fullscreen_shutdown_alloc_hooks(canvas);
        lib_free(canvas->fullscreenconfig);
#endif

#if !defined(HAVE_CAIRO)
        if (canvas->gdk_image != NULL) {
            g_object_unref(canvas->gdk_image);
        }
#else
    /* FIXME */
#endif

#if defined(HAVE_HWSCALE) && !defined(USE_UI_THREADS)
        lib_free(canvas->hwscale_image);
#endif
    }
}

/* set it, update if we know the endianness required by the image */
int video_canvas_set_palette(video_canvas_t *canvas, struct palette_s *palette)
{
    if (palette == NULL) {
        return 0; /* no palette, nothing to do */
    }
    canvas->palette = palette;
    return uicolor_set_palette(canvas, canvas->palette);
}

/* Change the size of the canvas. */
void video_canvas_resize(video_canvas_t *canvas, char resize_canvas)
{
    int imgw, imgh;

    if (console_mode || video_disabled_mode) {
        return;
    }
    canvas->videoconfig->readable = 1; /* it's not direct rendering */

    imgw = canvas->draw_buffer->canvas_physical_width;
    imgh = canvas->draw_buffer->canvas_physical_height;
#if !defined(HAVE_CAIRO)
    if (canvas->gdk_image != NULL) {
        g_object_unref(canvas->gdk_image);
    }
    canvas->gdk_image = gdk_image_new(GDK_IMAGE_FASTEST, gtk_widget_get_visual(canvas->emuwindow), imgw, imgh);
#else
/*
    if (canvas->cairo_ctx != NULL) {
        cairo_destroy(canvas->cairo_ctx);
    }
    canvas->cairo_ctx = gdk_cairo_create(gtk_widget_get_window(canvas->emuwindow));
*/
    if (canvas->gdk_pixbuf != NULL) {
        g_object_unref(canvas->gdk_pixbuf);
    }
    canvas->gdk_pixbuf = gdk_pixbuf_new(GDK_COLORSPACE_RGB, 0, 8, imgw, imgh);
#endif

#ifdef HAVE_HWSCALE
#ifdef USE_UI_THREADS
    mbuffer_init(canvas, imgw, imgh, 4, canvas->app_shell);
#else
    lib_free(canvas->hwscale_image);
    /* canvas->hwscale_image = lib_malloc(gdk_image_get_width(canvas->gdk_image) * gdk_image_get_height(canvas->gdk_image) * 4); */
    canvas->hwscale_image = lib_malloc(imgw * imgh * 4);
#endif	/* USE_UI_THREADS */
#endif

    if (video_canvas_set_palette(canvas, canvas->palette) < 0) {
        log_debug("Setting palette for this mode failed. (Try 16/24/32 bpp.)");
        exit(-1);
    }

    ui_resize_canvas_window(canvas);
}

/* Make the canvas visible. */
void video_canvas_map(video_canvas_t *s)
{
    fprintf(stderr, "**Function `canvas_map' not implemented.\n");
}

/* Make the canvas not visible. */
void video_canvas_unmap(video_canvas_t *s)
{
    fprintf(stderr, "**Function `canvas_unmap' not implemented.\n");
}

/* Refresh a canvas.  */
void video_canvas_refresh(video_canvas_t *canvas, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h)
{
    int imgw, imgh;
#if 0
    log_debug("XS%i YS%i XI%i YI%i W%i H%i PS%i", xs, ys, xi, yi, w, h, canvas->draw_buffer->draw_buffer_width);
#endif

    if (console_mode || video_disabled_mode) {
        return;
    }

    xi *= canvas->videoconfig->scalex;
    w *= canvas->videoconfig->scalex;

    yi *= canvas->videoconfig->scaley;
    h *= canvas->videoconfig->scaley;

#ifdef HAVE_FULLSCREEN
    if (canvas->video_fullscreen_refresh_func) {
        canvas->video_fullscreen_refresh_func(canvas, xs, ys, xi, yi, w, h);
        return;
    }
#endif

#if !defined(HAVE_CAIRO)
    imgw = gdk_image_get_width(canvas->gdk_image);
    imgh = gdk_image_get_height(canvas->gdk_image);
#else
    /* FIXME */
    imgw = canvas->draw_buffer->canvas_physical_width;
    imgh = canvas->draw_buffer->canvas_physical_height;
#endif

    if (((xi + w) > imgw) || ((yi + h) > imgh)) {
#ifdef DEBUG	
        log_debug("Attempt to draw outside canvas!\nXI%i YI%i W%i H%i CW%i CH%i\n", xi, yi, w, h, imgw, imgh);
#endif
	return;
    }

#ifdef HAVE_HWSCALE
    if (canvas->videoconfig->hwscale) {
#ifdef USE_UI_THREADS
	struct timespec t1[MAX_APP_SHELLS];
	clock_gettime(CLOCK_REALTIME, &t1[canvas->app_shell]);
	canvas->hwscale_image = 
	    mbuffer_get_buffer(&t1[canvas->app_shell], canvas->app_shell);
#endif
        video_canvas_render(canvas, canvas->hwscale_image, 
			    w, h, xs, ys, xi, yi, 
			    canvas->draw_buffer->canvas_physical_width * 4, 32);
        gtk_widget_queue_draw(canvas->emuwindow);
#if 0
	/* timing probe */
	{
	    static struct timespec t0[MAX_APP_SHELLS], t2[MAX_APP_SHELLS];
	    
	    clock_gettime(CLOCK_REALTIME, &t2[canvas->app_shell]);
	    DBG(("shell %d - emulation rate: %ldus, rendertime: %ldus", 
		 canvas->app_shell,
		 (TS_TOUSEC(t1[canvas->app_shell]) - 
		  TS_TOUSEC(t0[canvas->app_shell])), 
		 (TS_TOUSEC(t2[canvas->app_shell]) - 
		  TS_TOUSEC(t1[canvas->app_shell])))); 
	    memcpy(&t0[canvas->app_shell], &t1[canvas->app_shell], 
		   sizeof(struct timespec));
	}
#endif
    } else
#endif
    {
#if !defined(HAVE_CAIRO)
        video_canvas_render(canvas, gdk_image_get_pixels(canvas->gdk_image), w, h, xs, ys, xi, yi, gdk_image_get_bytes_per_line(canvas->gdk_image), gdk_image_get_bits_per_pixel(canvas->gdk_image));
#else
        /* FIXME */
        video_canvas_render(canvas, gdk_pixbuf_get_pixels(canvas->gdk_pixbuf), w, h, xs, ys, xi, yi, gdk_pixbuf_get_rowstride(canvas->gdk_pixbuf), gdk_pixbuf_get_bits_per_sample(canvas->gdk_pixbuf) * gdk_pixbuf_get_n_channels(canvas->gdk_pixbuf));
#endif
        gtk_widget_queue_draw_area(canvas->emuwindow, xi, yi, w, h);
    }
}

/* FIXME: this one should really return an int instead */
char video_canvas_can_resize(video_canvas_t *canvas)
{
    GtkWidget *t = get_active_toplevel();
    if (t) {
        return (gdk_window_get_state(gtk_widget_get_window(t)) & GDK_WINDOW_STATE_MAXIMIZED) ? 0 : 1;
    }
    return 0;
}
