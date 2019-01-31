/*
 * renderxv.h - XVideo rendering.
 *
 * Written by
 *  Dag Lem <resid@nimrod.no>
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

#ifndef VICE_RENDERXV_H
#define VICE_RENDERXV_H

#include "vice.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/XShm.h>

#ifndef __QNX__
#ifndef MINIX_SUPPORT
#include <sys/ipc.h>
#include <sys/shm.h>
#endif
#else
#include "qnxipc.h"
#include "qnxshm.h"
#endif

#include <X11/extensions/Xv.h>
#include <X11/extensions/Xvlib.h>

#include "video/renderyuv.h"

struct xywh_s {
    int x;
    int y;
    unsigned int w;
    unsigned int h;
};

extern int find_yuv_port(Display* display, XvPortID* port, fourcc_t* format, int *overlay);

extern XvImage* create_yuv_image(Display* display, XvPortID port, fourcc_t format, int width, int height, XShmSegmentInfo* shminfo);

extern void destroy_yuv_image(Display* display, XvImage* image, XShmSegmentInfo* shminfo);

extern void display_yuv_image(Display* display, XvPortID port, Drawable d, GC gc, XvImage* image, XShmSegmentInfo* shminfo,
                              int src_x, int src_y, unsigned int src_w, unsigned int src_h, struct xywh_s *dest,
                              double aspect_ratio);

#endif /* _RENDERXV_H */
