/*
 * x11video.c - Simple Xaw-based graphical user interface.  It uses widgets
 * from the Free Widget Foundation and Robert W. McMullen.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andre Fachat <fachat@physik.tu-chemnitz.de>
 *
 * Support for multiple visuals and depths by
 *  Teemu Rantanen <tvr@cs.hut.fi>
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


/*** MITSHM-code rewritten by Dirk Farin <farin@ti.uni-mannheim.de>. **

     This is how the MITSHM initialization now works:

       Three variables are used to enable/disable the usage of MITSHM:
       - 'try_mitshm' is set to true by default to specify that
         MITSHM shall be used if possible. If the user sets this
         variable to false MITSHM will be disabled.
       - 'use_mitshm' will be set in video_init() after some quick
         tests if the X11 server supports MITSHM.
       - Every framebuffer structure has a new field named 'using_mitshm'
         that is set to true if MITSHM is used for this buffer.
         Note that it is possible that one buffer is using MITSHM
         while some other buffer is not.

       Detecting if MITSHM usage is possible is now done using a
       minimum of intelligence (only XShmQueryExtension() is checked
       in video_init() ). Then the allocation process is executed
       and the X11 error in case of failure is catched. If an error
       occured the allocation process is reversed and non-MITSHM
       XImages are used instead.
*/

#include "vice.h"


#include <string.h>

#include "types.h"

#undef BYTE

#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>

#include "util.h"
#include "color.h"
#include "cmdline.h"
#include "fullscreenarch.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "resources.h"
#include "translate.h"
#include "ui.h"
#include "uicolor.h"
#include "video.h"
#include "videoarch.h"
#include "viewport.h"
#include "x11ui.h"

#ifdef HAVE_XVIDEO
#include "renderxv.h"
#endif

#include "x11video.h"


static log_t x11video_log = LOG_ERR;

#ifdef HAVE_XVIDEO
#include "video/video-resources.h"
#endif
#ifdef HAVE_OPENGL_SYNC
#include "openGL_sync.h"
#endif

#ifdef USE_MITSHM
#ifdef HAVE_X11_EXTENSIONS_SHMPROTO_H
#include <X11/extensions/shmproto.h>
#endif
/* Define this for additional shared memory verbosity. */
/*  #define MITSHM_DEBUG */

#ifdef MITSHM_DEBUG
#define DEBUG_MITSHM(x) log_debug x
#else
#define DEBUG_MITSHM(x)
#endif
#endif /* USE_MITSHM */

/* Flag: Do we call `XSync()' after blitting?  */
int _video_use_xsync;

/* Flag: Do we try to use the MIT shared memory extensions?  */
static int try_mitshm;

#ifdef HAVE_XVIDEO
static unsigned int fourcc = 0;
static char *fourcc_s = NULL;

static double aspect_ratio;
static char *aspect_ratio_s = NULL;

static int keepaspect, trueaspect;
#endif

static int set_use_xsync(int val, void *param)
{
    _video_use_xsync = val ? 1 : 0;

    return 0;
}

static int set_try_mitshm(int val, void *param)
{
    try_mitshm = val ? 1 : 0;

    return 0;
}

#ifdef HAVE_XVIDEO
static int set_fourcc(const char *val, void *param)
{
    if (util_string_set(&fourcc_s, val)) {
        return 0;
    }

    if (fourcc_s != NULL && strlen(fourcc_s) == 4) {
        memcpy(&fourcc, fourcc_s, 4);
    } else {
        fourcc = 0;
    }
    return 0;
}

static int set_keepaspect(int val, void *param)
{
    keepaspect = val ? 1 : 0;

    return 0;
}

static int set_aspect_ratio(const char *val, void *param)
{
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
        util_string_set(&aspect_ratio_s, "1.0");
        aspect_ratio = 1.0;
    }

    return 0;
}

UI_CALLBACK(set_custom_aspect_ratio)
{
    char *ratio_text = util_concat(_("Aspect ratio"), " (0.5 - 2.0):", NULL);

    uilib_select_string((char *)UI_MENU_CB_PARAM, _("Custom aspect ratio"), ratio_text);
    lib_free(ratio_text);
}

static int set_trueaspect(int val, void *param)
{
    trueaspect = val ? 1 : 0;

    return 0;
}
#endif

/* Video-related resources.  */
static const resource_string_t resources_string[] = {
#ifdef HAVE_XVIDEO
    { "FOURCC", "", RES_EVENT_NO, NULL,
      &fourcc_s, set_fourcc, NULL },
    { "AspectRatio", "1.0", RES_EVENT_NO, NULL,
      &aspect_ratio_s, set_aspect_ratio, NULL },
#endif
    RESOURCE_STRING_LIST_END
};

static const resource_int_t resources_int[] = {
    { "UseXSync", 1, RES_EVENT_NO, NULL,
      &_video_use_xsync, set_use_xsync, NULL },
      /* turn MITSHM on by default */
    { "MITSHM", 1, RES_EVENT_NO, NULL,
      &try_mitshm, set_try_mitshm, NULL },
#ifdef HAVE_XVIDEO
    { "KeepAspectRatio", 1, RES_EVENT_NO, NULL,
      &keepaspect, set_keepaspect, NULL },
    { "TrueAspectRatio", 1, RES_EVENT_NO, NULL,
      &trueaspect, set_trueaspect, NULL },
#endif
    RESOURCE_INT_LIST_END
};

int video_arch_resources_init(void)
{
#ifdef HAVE_OPENGL_SYNC
    openGL_register_resources();
#endif
    if (machine_class != VICE_MACHINE_VSID) {
        if (resources_register_string(resources_string) < 0) {
            return -1;
        }
        return resources_register_int(resources_int);
    }
    return 0;
}

void video_arch_resources_shutdown(void)
{
#ifdef HAVE_XVIDEO
    lib_free(aspect_ratio_s);
    lib_free(fourcc_s);
#endif
}

/* ------------------------------------------------------------------------- */

/* Video-related command-line options.  */
static const cmdline_option_t cmdline_options[] = {
    { "-xsync", SET_RESOURCE, 0,
      NULL, NULL, "UseXSync", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Call `XSync()' after updating the emulation window") },
    { "+xsync", SET_RESOURCE, 0,
      NULL, NULL, "UseXSync", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Do not call `XSync()' after updating the emulation window") },
    { "-mitshm", SET_RESOURCE, 0,
      NULL, NULL, "MITSHM", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Use shared memory") },
    { "+mitshm", SET_RESOURCE, 0,
      NULL, NULL, "MITSHM", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, N_("Never use shared memory (slower)") },
#ifdef HAVE_XVIDEO
    { "-fourcc", SET_RESOURCE, -1,
      NULL, NULL, "FOURCC", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      N_("<fourcc>"), N_("Request YUV FOURCC format") },
    { "-aspect", SET_RESOURCE, -1,
      NULL, NULL, "AspectRatio", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      N_("<Aspect ratio>"), N_("Set aspect ratio (0.5 - 2.0)") },
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
#endif
    CMDLINE_LIST_END
};

int video_arch_cmdline_options_init(void)
{
    if (machine_class != VICE_MACHINE_VSID) {
        return cmdline_register_options(cmdline_options);
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

static GC _video_gc;
static void (*_refresh_func)() = NULL;

/* This is set to 1 if the Shared Memory Extensions can actually be used. */
int use_mitshm = 0;

/* ------------------------------------------------------------------------- */

void video_convert_color_table(unsigned int i, uint8_t *data, long col, video_canvas_t *canvas)
{
#ifdef HAVE_XVIDEO
    if (canvas->videoconfig->hwscale && canvas->xv_image) {
        return;
    }
#endif

    switch (canvas->depth) {
        case 8:
            video_render_setphysicalcolor(canvas->videoconfig, i, (uint32_t)(*data), 8);
            break;
        case 16:
        case 24:
        case 32:
        default:
            video_render_setphysicalcolor(canvas->videoconfig, i, (uint32_t)(col), canvas->depth);
            break;
    }
}

static GC video_get_gc(XGCValues *gc_values);

int video_init(void)
{
    XGCValues gc_values;
    Display *display;

    _video_gc = video_get_gc(&gc_values);
    display = x11ui_get_display_ptr();

    x11video_log = log_open("X11Video");

    color_init();

#ifdef USE_MITSHM
    if (!try_mitshm) {
        use_mitshm = 0;
    } else {
        /* This checks if the server has MITSHM extensions available
           If try_mitshm is true and we are on a different machine,
           frame_buffer_alloc will fall back to non shared memory calls. */
        int major_version, minor_version, pixmap_flag;

        /* Check whether the server supports the Shared Memory Extension. */
        if (!XShmQueryVersion(display, &major_version, &minor_version, &pixmap_flag)) {
            log_warning(x11video_log, "The MITSHM extension is not supported on this display.");
            use_mitshm = 0;
        } else {
            DEBUG_MITSHM((_("MITSHM extensions version %d.%d detected."), major_version, minor_version));
	    if (!pixmap_flag) {
		DEBUG_MITSHM(("The MITSHM extension is supported on this display, but shared pixmaps are not available."));
	    }
            use_mitshm = 1;
        }
    }

#else
    use_mitshm = 0;
#endif

    return 0;
}

void video_shutdown(void)
{
    color_shutdown();
}

#ifdef USE_MITSHM
static int mitshm_failed = 0; /* will be set to true if XShmAttach() failed */

/* Catch XShmAttach()-failure. */
static int shm_attach_handler(Display *display, XErrorEvent *err)
{
    mitshm_failed = 1;

    return 0;
}
#endif

/* Free an allocated frame buffer. */
static void video_arch_frame_buffer_free(video_canvas_t *canvas)
{
    Display *display;

    if (canvas == NULL) {
        return;
    }

#ifdef HAVE_XVIDEO
    if (canvas->xv_image) {
#if defined(__QNX__) || defined(MINIX_SUPPORT)
        XShmSegmentInfo* shminfo = NULL;
#else
        XShmSegmentInfo* shminfo = canvas->using_mitshm ? &canvas->xshm_info : NULL;
#endif

        display = x11ui_get_display_ptr();
        destroy_yuv_image(display, canvas->xv_image, shminfo);
        return;
    }
#endif

#ifdef HAVE_FULLSCREEN
    if (canvas->fullscreenconfig->enable) {
        return;
    }
#endif

    display = x11ui_get_display_ptr();

#ifdef USE_MITSHM
    if (canvas->using_mitshm) {
        XShmDetach(display, &(canvas->xshm_info));
        if (canvas->x_image) {
            XDestroyImage(canvas->x_image);
        }
        if (shmdt(canvas->xshm_info.shmaddr)) {
            log_error(x11video_log, "Cannot release shared memory!");
        }
    }
    else if (canvas->x_image) {
        XDestroyImage(canvas->x_image);
    }
#else
    if (canvas->x_image) {
        XDestroyImage(canvas->x_image);
    }
#endif
}

/* ------------------------------------------------------------------------- */

void video_arch_canvas_init(struct video_canvas_s *canvas)
{
    canvas->video_draw_buffer_callback = NULL;

#ifdef HAVE_FULLSCREEN
    canvas->fullscreenconfig = lib_calloc(1, sizeof(fullscreenconfig_t));
    fullscreen_init_alloc_hooks(canvas);
#endif
}

static void video_refresh_func(void (*rfunc)(void))
{
    _refresh_func = rfunc;
}

static int video_arch_frame_buffer_alloc(video_canvas_t *canvas, unsigned int width, unsigned int height)
{
    int sizeofpixel = sizeof(uint8_t);
    Display *display;
#ifdef USE_MITSHM
    int (*olderrorhandler)(Display *, XErrorEvent *);
#endif

#ifdef USE_MITSHM
    canvas->using_mitshm = use_mitshm;
#endif

    display = x11ui_get_display_ptr();

    /* sizeof(PIXEL) is not always what we are using. I guess this should
       be checked from the XImage but I'm lazy... */
    if (canvas->depth > 8) {
        sizeofpixel *= 2;
    }
    if (canvas->depth > 16) {
        sizeofpixel *= 2;
    }
    canvas->videoconfig->readable = 1; /* it's not direct rendering */

#ifdef HAVE_XVIDEO
    canvas->xv_image = NULL;

    if (canvas->videoconfig->hwscale) {
#if defined(__QNX__) || defined(MINIX_SUPPORT)
        XShmSegmentInfo* shminfo = NULL;
#else
        XShmSegmentInfo* shminfo = canvas->using_mitshm ? &canvas->xshm_info : NULL;
#endif
	for (;;) {
	    canvas->xv_image = create_yuv_image(display, canvas->xv_port, canvas->xv_format, width, height, shminfo);
	    if (canvas->xv_image) {
		break;
	    }

	    if (shminfo) {
#if !defined(__QNX__) && !defined(MINIX_SUPPORT)
		canvas->using_mitshm = 0;
#endif
		shminfo = NULL;
		continue;
	    }
	    return -1;
        }
        XSync(display, False);

        /* Copy data for architecture independent rendering. */
        canvas->yuv_image.width = canvas->xv_image->width;
        canvas->yuv_image.height = canvas->xv_image->height;
        canvas->yuv_image.data_size = canvas->xv_image->data_size;
        canvas->yuv_image.num_planes = canvas->xv_image->num_planes;
        canvas->yuv_image.pitches = canvas->xv_image->pitches;
        canvas->yuv_image.offsets = canvas->xv_image->offsets;
        canvas->yuv_image.data = (unsigned char *)canvas->xv_image->data;

        log_message(x11video_log, "Successfully initialized using XVideo (%dx%d %.4s)%s shared memory.",
		width, height, canvas->xv_format.label,
#if !defined(__QNX__) && !defined(MINIX_SUPPORT)
		canvas->using_mitshm ? ", using" : " without");
#else
                " without");
#endif

        return 0;
    }
#endif /* HAVE_XVIDEO */

    /* Round up to 32-bit boundary (used in XCreateImage). */
    width = (width + 3) & ~0x3;

#ifdef USE_MITSHM
tryagain:
    if (canvas->using_mitshm) {
        DEBUG_MITSHM(("frame_buffer_alloc(): allocating XImage with MITSHM, %d x %d pixels...", width, height));
        canvas->x_image = XShmCreateImage(display, visual, canvas->depth, ZPixmap, NULL, &(canvas->xshm_info), width, height);
        if (!canvas->x_image) {
            log_warning(x11video_log, "Cannot allocate XImage with XShm; falling back to non MITSHM extension mode.");
            canvas->using_mitshm = 0;
            goto tryagain;
        }
        DEBUG_MITSHM(("Done."));
        DEBUG_MITSHM(("frame_buffer_alloc(): shmgetting %ld bytes...", (long)canvas->x_image->bytes_per_line * canvas->x_image->height));
        canvas->xshm_info.shmid = shmget(IPC_PRIVATE, canvas->x_image->bytes_per_line * canvas->x_image->height, IPC_CREAT | 0604);
        if (canvas->xshm_info.shmid == -1) {
            log_warning(x11video_log, "Cannot get shared memory; falling back to non MITSHM extension mode.");
            XDestroyImage(canvas->x_image);
            canvas->using_mitshm = 0;
            goto tryagain;
        }
        DEBUG_MITSHM(("Done, id = 0x%x.", canvas->xshm_info.shmid));
        DEBUG_MITSHM(("frame_buffer_alloc(): getting address... "));
        canvas->xshm_info.shmaddr = shmat(canvas->xshm_info.shmid, 0, 0);
        canvas->x_image->data = canvas->xshm_info.shmaddr;
        if (canvas->xshm_info.shmaddr == (char *)-1) {
            log_warning(x11video_log, "Cannot get shared memory address; falling back to non MITSHM extension mode.");
            shmctl(canvas->xshm_info.shmid,IPC_RMID,0);
            XDestroyImage(canvas->x_image);
            canvas->using_mitshm = 0;
            goto tryagain;
        }
        DEBUG_MITSHM(("0x%lx OK.", (unsigned long) canvas->xshm_info.shmaddr));
        canvas->xshm_info.readOnly = True;
        mitshm_failed = 0;

        olderrorhandler = XSetErrorHandler(shm_attach_handler);

        if (!XShmAttach(display, &(canvas->xshm_info))) {
            log_warning(x11video_log, "Cannot attach shared memory; falling back to non MITSHM extension mode.");
            shmdt(canvas->xshm_info.shmaddr);
            shmctl(canvas->xshm_info.shmid,IPC_RMID,0);
            XDestroyImage(canvas->x_image);
            canvas->using_mitshm = 0;
            goto tryagain;
        }

        /* Wait for XShmAttach to fail or to succeed. */
        XSync(display,False);
        XSetErrorHandler(olderrorhandler);

        /* Mark memory segment for automatic deletion. */
        shmctl(canvas->xshm_info.shmid, IPC_RMID, 0);

        if (mitshm_failed) {
            log_warning(x11video_log, "Cannot attach shared memory; falling back to non MITSHM extension mode.");
            shmdt(canvas->xshm_info.shmaddr);
            XDestroyImage(canvas->x_image);
            canvas->using_mitshm = 0;
            goto tryagain;
        }

        DEBUG_MITSHM((_("MITSHM initialization succeed.\n")));
        video_refresh_func((void (*)(void))XShmPutImage);
    } else
#endif
    {                           /* !i->using_mitshm */
        char *data;

        data = lib_malloc(width * height * sizeofpixel);

        if (data == NULL) {
            return -1;
        }

        canvas->x_image = XCreateImage(display, visual, canvas->depth, ZPixmap, 0, data, width, height, 32, 0);
        if (!canvas->x_image) {
            return -1;
        }

        video_refresh_func((void (*)(void))XPutImage);
    }

#ifdef USE_MITSHM
    log_message(x11video_log, "Successfully initialized (%dx%d)%s shared memory.",
            width, height,
            (canvas->using_mitshm) ? ", using" : " without");

    if (!(canvas->using_mitshm)) {
        log_warning(x11video_log, "Performance will be poor.");
    }
#else
    log_message(x11video_log, "Successfully initialized (%dx%d) without shared memory.",
            width, height);
#endif

    return 0;
}

static GC video_get_gc(XGCValues *gc_values)
{
    Display *display;

    display = x11ui_get_display_ptr();

    return XCreateGC(display, XtWindow(_ui_top_level), 0, gc_values);
}

/* Make the canvas visible. */
void video_canvas_map(video_canvas_t *s)
{
    Display *display;

    display = x11ui_get_display_ptr();

    XMapWindow(display, s->drawable);
    XFlush(display);
}

/* Make the canvas not visible. */
void video_canvas_unmap(video_canvas_t *s)
{
    Display *display;

    display = x11ui_get_display_ptr();

    XUnmapWindow(display, s->drawable);
    XFlush(display);
}

static void ui_finish_canvas(video_canvas_t *c)
{
    c->drawable = XtWindow(c->emuwindow);
}

video_canvas_t *video_canvas_create(video_canvas_t *canvas, unsigned int *width, unsigned int *height, int mapped)
{
    int res;
    XGCValues gc_values;
    int win_width = canvas->draw_buffer->visible_width * canvas->videoconfig->scalex;
    int win_height = canvas->draw_buffer->visible_height * canvas->videoconfig->scaley;

    canvas->depth = x11ui_get_display_depth();

#ifdef HAVE_XVIDEO
    /* Request specified video format. */
    canvas->xv_format.id = fourcc;

    if (!find_yuv_port(x11ui_get_display_ptr(), &canvas->xv_port, &canvas->xv_format, &canvas->is_overlay)) {
        if (canvas->videoconfig->hwscale) {
            log_message(x11video_log, "HW scaling not available");
            canvas->videoconfig->hwscale = 0;
        }
        resources_set_int("HwScalePossible", 0);
    }
    if (canvas->videoconfig->hwscale) {
        double local_aspect_ratio = 1.0;
        if (trueaspect) {
            local_aspect_ratio = canvas->geometry->pixel_aspect_ratio;
        } else if (keepaspect) {
            local_aspect_ratio = aspect_ratio;
        }
        win_width = (int)((double)win_width * local_aspect_ratio + 0.5);
    }
#else
    resources_set_int("HwScalePossible", 0);
#endif

    res = ui_open_canvas_window(canvas, canvas->viewport->title, win_width, win_height, 1);
    if (res < 0) {
        return NULL;
    }

    if (!_video_gc) {
        _video_gc = video_get_gc(&gc_values);
    }

    ui_finish_canvas(canvas);

    if (canvas->depth > 8) {
        uicolor_init_video_colors(canvas);
    }

#ifdef HAVE_OPENGL_SYNC
    openGL_sync_init(canvas);
#endif

    return canvas;
}

void video_canvas_destroy(video_canvas_t *canvas)
{
    video_arch_frame_buffer_free(canvas);
#ifdef HAVE_FULLSCREEN
    if (canvas != NULL) {
        fullscreen_shutdown_alloc_hooks(canvas);
        lib_free(canvas->fullscreenconfig);
    }
#endif
}

int video_canvas_set_palette(video_canvas_t *c, struct palette_s *palette)
{
    if (palette == NULL) {
        return 0; /* no palette, nothing to do */
    }

    c->palette = palette;

    return uicolor_set_palette(c, palette);
}

/* Change the size of the canvas. */
void video_canvas_resize(video_canvas_t *canvas, char resize_canvas)
{
    if (console_mode || video_disabled_mode) {
        return;
    }

    video_arch_frame_buffer_free(canvas);
    video_arch_frame_buffer_alloc(canvas, canvas->draw_buffer->canvas_physical_width, canvas->draw_buffer->canvas_physical_height);

    if (resize_canvas) {
        int width = canvas->draw_buffer->canvas_physical_width;
        int height = canvas->draw_buffer->canvas_physical_height;
#ifdef HAVE_XVIDEO
        double local_aspect_ratio = 1.0;
        if (canvas->videoconfig->hwscale) {
            if (trueaspect) {
                local_aspect_ratio = canvas->geometry->pixel_aspect_ratio;
            } else if (keepaspect) {
                local_aspect_ratio = aspect_ratio;
            }
        }
        width = (int)((double)width * local_aspect_ratio + 0.5);
#endif
        x11ui_resize_canvas_window(canvas->emuwindow, width, height);
    }

    ui_finish_canvas(canvas);
}

/* ------------------------------------------------------------------------- */

/* Refresh a canvas.  */
void video_canvas_refresh(video_canvas_t *canvas, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h)
{
    Display *display;

    if (console_mode || video_disabled_mode) {
        return;
    }

#ifdef HAVE_XVIDEO
    if (canvas->videoconfig->hwscale && canvas->xv_image) {
#if defined(__QNX__) || defined(MINIX_SUPPORT)
        XShmSegmentInfo* shminfo = NULL;
#else
        XShmSegmentInfo* shminfo = canvas->using_mitshm ? &canvas->xshm_info : NULL;
#endif
        Window root;
        int x, y;
        unsigned int border_width, depth;
        double local_aspect_ratio;

        display = x11ui_get_display_ptr();

        if (!canvas->videoconfig->color_tables.updated) { /* update colors as necessary */
            video_render_update_palette(canvas);
        }
        render_yuv_image(canvas->viewport,
                         (canvas->videoconfig->filter == VIDEO_FILTER_CRT),
                         canvas->videoconfig->video_resources.pal_blur * 64 / 1000,
                         canvas->videoconfig->video_resources.pal_scanlineshade * 1024 / 1000,
                         canvas->xv_format,
                         &canvas->yuv_image,
                         canvas->draw_buffer->draw_buffer,
                         canvas->draw_buffer->draw_buffer_width,
                         canvas->videoconfig,
                         xs, ys, w, h,
                         xi, yi);

        if (trueaspect) {
            local_aspect_ratio = canvas->geometry->pixel_aspect_ratio;
        } else if (keepaspect) {
            local_aspect_ratio = aspect_ratio;
        } else {
            local_aspect_ratio = 0.0;
        }

        XGetGeometry(display, canvas->drawable, &root, &x, &y, &canvas->xv_geometry.w, &canvas->xv_geometry.h, &border_width, &depth);

        /* Xv does subpixel scaling. Since coordinates are in integers we
           refresh the entire image to get it right. */
        display_yuv_image(display, canvas->xv_port, canvas->drawable,
                _video_gc, canvas->xv_image, shminfo, 0, 0,
                canvas->draw_buffer->canvas_physical_width,
                canvas->draw_buffer->canvas_physical_height,
                &canvas->xv_geometry, local_aspect_ratio);

        if (_video_use_xsync) {
            XSync(display, False);
        }

        return;
    }
#endif

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

    if (xi + w > canvas->draw_buffer->canvas_physical_width || yi + h > canvas->draw_buffer->canvas_physical_height) {
        log_debug("Attempt to draw outside canvas!\nXI%i YI%i W%i H%i CW%i CH%i\n", xi, yi, w, h, canvas->draw_buffer->canvas_physical_width, canvas->draw_buffer->canvas_physical_height);
        return; /* this makes `-fullscreen -80col' work
                   XXX fix me some day */
    }

    if ((int)xs >= 0) {
        /* some render routines don't like negative xs */
	video_canvas_render(canvas, (uint8_t *)canvas->x_image->data, w, h, xs, ys, xi, yi, canvas->x_image->bytes_per_line, canvas->x_image->bits_per_pixel);
    }

    /* This could be optimized away.  */
    display = x11ui_get_display_ptr();

    _refresh_func(display, canvas->drawable, _video_gc, canvas->x_image, xi, yi, xi, yi, w, h, False, NULL, canvas);

    if (_video_use_xsync) {
        XSync(display, False);
    }
}

/* FIXME: This should return 0 if the window is maximized
   (and therefore cannot change size).
   What is a pure X11 method to detect that? */
char video_canvas_can_resize(video_canvas_t *canvas)
{
    return 1;
}
