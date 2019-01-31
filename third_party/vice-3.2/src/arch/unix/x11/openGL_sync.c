/*
 * openGL_sync.c
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
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

#ifndef HAVE_OPENGL_SYNC
#error "openGL Sync extension (GLX_SGI_video_sync) not available in config.h, check config.log"
#endif

#include <time.h>
#include <stdio.h>
#include <GL/glx.h> 
#include <GL/gl.h> 
#include <string.h>

#include "video.h"
#include "videoarch.h"
#include "x11ui.h"
#include "log.h"
#include "machine.h"
#include "openGL_sync.h"
#include "resources.h"
#include "uiapi.h"
#include "uimenu.h"
#include "lib.h"

static log_t openGL_log = LOG_ERR;
static int no_sync = 0;		/* extension available */
static int openGL_sync = 0;		/* enabled/disable synchronization */
static int openGL_initialized = 0;
static GLXContext cx = (GLXContext) NULL;     

static int set_openGL_sync(int val, void *param);
static int check_openGL(Display *dpy);
static int openGL_dummy_set(int val, void *a);
static video_canvas_t *current_canvas;

static resource_int_t resources_openGL_sync_int[] = {
    { "openGL_sync", 0, RES_EVENT_NO, NULL,
      &openGL_sync, set_openGL_sync, NULL },
    { "openGL_no_sync", 0, RES_EVENT_NO, NULL,
      &no_sync, openGL_dummy_set, NULL },
    RESOURCE_INT_LIST_END
};

int openGL_available(int v)
{
    return !no_sync;
}

void openGL_register_resources(void)
{
    resources_register_int(resources_openGL_sync_int);
}

void openGL_sync_init(video_canvas_t *canvas)
{
    Display *dpy;

    current_canvas = canvas; /* save to have access to refreshrate */

    if (openGL_log == LOG_ERR) {
        openGL_log = log_open("openGL");
    } else {
        return;			/* we've been initializied already */
    }

    dpy = x11ui_get_display_ptr();

    if (check_openGL(dpy)) {
        no_sync = 1;
    } else {
        init_openGL();
    }
}

void openGL_sync_with_raster(void)
{
    extern int glXWaitVideoSyncSGI(int m, int d, unsigned int *c);

    int r;
    unsigned int c;
    if (openGL_sync && !no_sync) {
        if ((r = glXWaitVideoSyncSGI(1, 0, &c))) {
            log_error(openGL_log, "glXWaitVideoSyncSGI() returned %d", r);
        }
    }
}

int openGL_sync_enabled()
{
    return openGL_sync;
}

void openGL_sync_shutdown(void)
{
    if (openGL_sync) {
        set_openGL_sync(0, NULL);
    }
    if (cx) {
        glXDestroyContext(x11ui_get_display_ptr(), cx);
        cx = NULL;
    }
}

void init_openGL(void)
{
    XVisualInfo *vi;     
    Display *dpy;

    /*get the deepest buffer with 1 red bit*/ 
    static int attributeListSgl[] = { GLX_RGBA, GLX_RED_SIZE, 1, GLX_GREEN_SIZE, 1, GLX_BLUE_SIZE, 1, None }; 

    dpy = x11ui_get_display_ptr();
    vi = glXChooseVisual(dpy, DefaultScreen(dpy), attributeListSgl); 
    if (vi == NULL) {
        log_error(openGL_log, "glXChooseVisual() failed");
        no_sync = 1;
        return;
    }

    if (cx) {
        glXDestroyContext(dpy, cx);
    }

    cx = glXCreateContext(dpy, vi, 0, GL_TRUE);
    if (!cx) {
        log_error(openGL_log, "glXCreateContext() failed");
        no_sync = 1;
        return;
    }
    glXMakeCurrent(dpy, x11ui_get_X11_window(), cx);
    openGL_initialized = 1;
}

void openGL_set_canvas_refreshrate(float rr)
{
    if ((rr < 50.0) || (rr > 150.0)) {
        return;
    }
    if (current_canvas) {
        current_canvas->refreshrate = rr;
    }
}

float openGL_get_canvas_refreshrate(void)
{
    return current_canvas->refreshrate;
}

/* ---------------------------------------------------------------------*/
static int check_openGL(Display *dpy)
{
    /* Fixme: add code here to check if extension exists */
    const char *ext_table;
    char *t1, *t2;
    int b, e;

    if (glXQueryExtension(dpy, &b, &e) && (ext_table = glXQueryExtensionsString(dpy, DefaultScreen(dpy)))) {
        t1 = lib_stralloc(ext_table);
        t2 = strtok(t1, " ");
        while (t2) {
            if (strcmp(t2, "GLX_SGI_video_sync") == 0) {
                log_message(openGL_log, "GLX_SGI_video_sync extension is supported");
                lib_free(t1);
                return 0;
            }
            t2 = strtok(NULL, " ");
        }
        lib_free(t1);
    }
    log_message(openGL_log, "GLX_SGI_video_sync extension not supported");

    return 1;
}

static int set_openGL_sync(int val, void *param)
{
    int oldval;
    if ((no_sync) || (machine_class == VICE_MACHINE_VSID)) {
        return 0;
    }
    oldval = openGL_sync;
    openGL_sync = val ? 1 : 0;
    if (openGL_sync && openGL_initialized) {
        init_openGL();
    }
    if (oldval != openGL_sync) {
        log_message(openGL_log, "%s openGL_sync", openGL_sync? "enabling" : "disabling");
    }
    ui_update_menus();
    return 0;
}

static int openGL_dummy_set(int val, void *param)
{
    return 0;
}
