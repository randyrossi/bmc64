/*
 * renderxv.c - XVideo rendering.
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

/* The PAL Y/C and PAL Composite emulation is based on work by John
   Selck <graham@cruise.de>. The Xv probing and allocation code is
   loosely based on testxv.c (by Andre Werthmann) and VideoLAN. */

#include "vice.h"

#ifdef HAVE_XVIDEO

#include "renderxv.h"

#include "video.h"

#include <stdio.h>
#include <string.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#if defined(__QNX__) || defined(MINIX_SUPPORT)
Status XShmAttach(Display *display, XShmSegmentInfo *shminfo)
{
    return 0;
}

Status XShmDetach(Display *display, XShmSegmentInfo *shminfo)
{
    return 0;
}
#endif

#ifdef MINIX_SUPPORT
typedef long key_t;

struct ipc_perm {
    key_t key;
    unsigned short uid;
    unsigned short gid;
    unsigned short cuid;
    unsigned short cgid;
    unsigned short mode;
    unsigned short seq;
};

struct shmid_ds {
    struct ipc_perm shm_perm;
    int shm_segsz;
    time_t shm_atime;
    time_t shm_dtime;
    time_t shm_ctime;
    unsigned short shm_cpid;
    unsigned short shm_lpid;
    short shm_nattch;
    unsigned short shm_npages;
    unsigned long *shm_pages;
};

#define IPC_PRIVATE ((key_t)0)
#define IPC_CREAT   0x0200
#define IPC_RMID    0x1000

int shmget(key_t key, int size, int shmflag)
{
    return -1;
}

void *shmat(int shmid, const void *shmaddr, int shmflag)
{
}

int shmctl(int shmid, int cmd, struct shmid_ds *buf)
{
    return -1;
}

int shmdt(const void *shmaddr)
{
    return -1;
}
#endif

/* YUV formats in preferred order. */
fourcc_t fourcc_list[] = {
    /* YUV 4:2:2 formats: */
    { FOURCC_UYVY },
    { FOURCC_YUY2 },
    { FOURCC_YVYU },
    /* YUV 4:1:1 formats: */
    { FOURCC_YV12 },
    { FOURCC_I420 },
    { FOURCC_IYUV }, /* IYUV is a duplicate of I420. */
};

int find_yuv_port(Display* display, XvPortID* port, fourcc_t* format, int *overlay)
{
    int i, j, k, found_one = 0, is_overlay;

    /* XvQueryExtension */
    unsigned int version, release, request_base, event_base, error_base;

    /* XvQueryAdaptors */
    unsigned int num_adaptors;
    XvAdaptorInfo* adaptor_info = NULL;
    XvPortID port_id;

    /* XvListImageFormats */
    int num_formats;
    XvImageFormatValues* format_list = NULL;

    switch (XvQueryExtension(display, &version, &release, &request_base, &event_base, &error_base)) {
        case Success:
            break;
        case XvBadExtension:
            printf("XvQueryExtension returned XvBadExtension.\n");
            return 0;
        case XvBadAlloc:
            printf("XvQueryExtension returned XvBadAlloc.\n");
            return 0;
        default:
            printf("XvQueryExtension returned unknown error.\n");
            return 0;
    }

    switch (XvQueryAdaptors(display, DefaultRootWindow(display), &num_adaptors, &adaptor_info)) {
        case Success:
            break;
        case XvBadExtension:
            printf("XvQueryAdaptors returned XvBadExtension.\n");
            return 0;
        case XvBadAlloc:
            printf("XvQueryAdaptors returned XvBadAlloc.\n");
            return 0;
        default:
            printf("XvQueryAdaptors returned unknown error.\n");
            return 0;
    }

    /* Find YUV capable adaptor. */
    for (i = 0; i < (int)num_adaptors; i++) {
        if (!(adaptor_info[i].type & XvInputMask && adaptor_info[i].type & XvImageMask)) {
            continue;
        }
        /* Textured video is not an overlay */
        is_overlay = !strstr(adaptor_info[i].name, "Textured");

        format_list = XvListImageFormats(display, adaptor_info[i].base_id, &num_formats);

        for (j = 0; j < (int)(sizeof(fourcc_list) / sizeof(*fourcc_list)); j++) {
            if (format->id && fourcc_list[j].id != format->id) {
                continue;
            }
            for (k = 0; k < num_formats; k++) {
                if (format_list[k].id != fourcc_list[j].id) {
                    continue;
                }

                for (port_id = adaptor_info[i].base_id; port_id < adaptor_info[i].base_id + adaptor_info[i].num_ports; port_id++) {
                    if (XvGrabPort(display, port_id, CurrentTime) != Success) {
                        continue;
                    }
                    if (found_one) {
                        if (!*overlay && is_overlay) {
                            XvUngrabPort(display, port_id, CurrentTime);
                            continue; /* Prefer non-overlay as that one can handle more than one windows */
                        }
                        XvUngrabPort(display, *port, CurrentTime);
                    }
                    *port = port_id;
                    *format = fourcc_list[j];
                    *overlay = is_overlay;
                    found_one = 1;
                }
            }
        }
        XFree(format_list);
    }
    XvFreeAdaptorInfo(adaptor_info);
    if (!found_one) {
        printf("No suitable Xv YUV adaptor/port available.\n");
    }
    return found_one;
}

static int mitshm_failed = 0; /* will be set to true if XShmAttach() failed */

/* Catch XShmAttach()-failure. */
static int shm_attach_handler(Display *display, XErrorEvent *err)
{
    mitshm_failed = 1;

    return 0;
}

XvImage* create_yuv_image(Display* display, XvPortID port, fourcc_t format, int width, int height, XShmSegmentInfo* shminfo)
{
    XvImage* image;

    if (shminfo) {
	int (*olderrorhandler)(Display *, XErrorEvent *);

        if (!(image = XvShmCreateImage(display, port, format.id, NULL, width, height, shminfo))) {
            printf("Unable to create shm XvImage\n");
            return NULL;
        }

        if ((shminfo->shmid = shmget(IPC_PRIVATE, image->data_size, IPC_CREAT | 0777)) == -1) {
            printf("Unable to allocate shared memory\n");
            XFree(image);
            return NULL;
        }

        if (!(shminfo->shmaddr = shmat(shminfo->shmid, 0, 0))) {
            printf("Unable to attach shared memory\n");
            shmctl(shminfo->shmid, IPC_RMID, 0);
            XFree(image);
            return NULL;
        }
        shminfo->readOnly = False;

        image->data = shminfo->shmaddr;

	mitshm_failed = 0;
        olderrorhandler = XSetErrorHandler(shm_attach_handler);

        if (!XShmAttach(display, shminfo) || mitshm_failed) {
            printf("XShmAttach failed\n");
	    XSetErrorHandler(olderrorhandler);
            shmdt(shminfo->shmaddr);
            shmctl(shminfo->shmid, IPC_RMID, 0);
            XFree(image);
            return NULL;
        }

        /* Send image to X server. This instruction is required, since having
         * built a Shm XImage and not using it causes an error on XCloseDisplay. */
        XSync(display, False);
        XSetErrorHandler(olderrorhandler);

	if (mitshm_failed) {
            shmdt(shminfo->shmaddr);
            shmctl(shminfo->shmid, IPC_RMID, 0);
            XFree(image);
            return NULL;
	}

        /* Mark the segment to be automatically removed when the last
           attachment is broken (i.e. on shmdt or process exit). */
        shmctl(shminfo->shmid, IPC_RMID, 0);
    } else {
        if (!(image = XvCreateImage(display, port, format.id, NULL, width, height))) {
            printf("Unable to create XvImage\n");
            return NULL;
        }
        image->data = malloc(image->data_size);
    }

    return image;
}

void destroy_yuv_image(Display* display, XvImage* image, XShmSegmentInfo* shminfo)
{
    if (shminfo) {
        XShmDetach(display, shminfo);
        XFree(image);
        shmdt(shminfo->shmaddr);
    } else {
        XFree(image);
    }
}

void display_yuv_image(Display* display, XvPortID port, Drawable d, GC gc, XvImage* image, XShmSegmentInfo* shminfo,
                       int src_x, int src_y, unsigned int src_w, unsigned int src_h, struct xywh_s *dest,
                       double aspect_ratio)
{
    int dest_x = 0, dest_y = 0;
    int dest_w = dest->w;
    int dest_h = dest->h;

    /* Maybe keep aspect ratio of src image. */
    if (aspect_ratio == 0.0) {
        /*
         * Don't bother correcting any aspect ratio, just scale
         * to size as given.
         */
    } else if (dest_w * src_h < src_w * aspect_ratio * dest_h) {
        dest_y = dest_h;
        dest_h = dest_w * src_h / (src_w * aspect_ratio);
        dest_y = (dest_y - dest_h) / 2;
    } else {
        dest_x = dest_w;
        dest_w = dest_h * src_w * aspect_ratio / src_h;
        dest_x = (dest_x - dest_w) / 2;
    }

    /* Record (for the lightpen code) where the scaled screen ended up */
    dest->x = dest_x;
    dest->y = dest_y;
    dest->w = dest_w;
    dest->h = dest_h;

    if (shminfo) {
        XvShmPutImage(display, port, d, gc, image, src_x, src_y, src_w, src_h, dest_x, dest_y, dest_w, dest_h, False);
    } else {
        XvPutImage(display, port, d, gc, image, src_x, src_y, src_w, src_h, dest_x, dest_y, dest_w, dest_h);
    }
}

#endif /* HAVE_XVIDEO */
