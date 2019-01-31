/*
 * video.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#include "vice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "videoarch.h"
#include "palette.h"
#include "video.h"
#include "viewport.h"
#include "kbd.h"
#include "keyboard.h"
#include "lib.h"
#include "fullscreenarch.h"
#include "pointer.h"

#define __USE_INLINE__
#define MUIPROG_H

#undef BYTE
#undef WORD
#include <exec/types.h>
#include <exec/nodes.h>
#include <exec/lists.h>
#include <exec/memory.h>

#include <proto/exec.h>
#include <proto/intuition.h>

#ifdef AMIGA_M68K
#define _INLINE_MUIMASTER_H
#endif

#include <proto/muimaster.h>

#ifdef AMIGA_M68K
#include <libraries/mui.h>
#endif

#include <proto/graphics.h>

#ifdef HAVE_PROTO_CYBERGRAPHICS_H
#include <proto/cybergraphics.h>
#include <cybergraphx/cybergraphics.h>

#ifdef AMIGA_MORPHOS
#include <exec/execbase.h>
#endif
#if !defined(AMIGA_MORPHOS) && !defined(AMIGA_AROS)
#include <inline/cybergraphics.h>
#endif
#else
#ifdef HAVE_PROTO_PICASSO96_H
#include <proto/Picasso96.h>
#else
#include <proto/Picasso96API.h>
#endif
#endif

#include "private.h"
#include "statusbar.h"
#include "mui/mui.h"

#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
#include <cybergraphx/cgxvideo.h>
#include <proto/cgxvideo.h>
#include "video/renderyuv.h"
#endif

video_canvas_t *canvaslist = NULL;

int video_arch_cmdline_options_init(void)
{
    return 0;
}

#ifndef AMIGA_AROS
struct RastPort *CreateRastPort(void)
{
    return lib_AllocVec(sizeof(struct RastPort), MEMF_ANY | MEMF_PUBLIC);
}

struct RastPort *CloneRastPort(struct RastPort *friend_rastport)
{
    struct RastPort *tmpRPort = CreateRastPort();

    if (tmpRPort != NULL) {
        CopyMem(friend_rastport, tmpRPort, sizeof(struct RastPort));
        return tmpRPort;
    }
    return NULL;
}
#endif

#ifdef AMIGA_AROS
/* Use these on ALL amiga platforms not just AROS */
UBYTE *unlockable_buffer = NULL;            /* Used to render the vice-buffer so we can WPA it into our backbuffer if we cant lock a bitmap! */

static struct RastPort *renderRPort = NULL; /* Clone of the windows rastport used to visibly output  */
static struct RastPort *backRPort = NULL;   /* RastPort for our backbuffer (canvas->os->window_bitmap) */
#endif

static struct Process *self;
static struct Window *orig_windowptr;

int video_init(void)
{
    self = (APTR)FindTask(NULL);
    orig_windowptr = self->pr_WindowPtr;
    if (mui_init() == 0) {
        return 0;
    }
    return -1;
}

void video_shutdown(void)
{
    struct video_canvas_s *nextcanvas, *canvas;

    mui_exit();

    /* make sure the process window ref won't be bad */
    self->pr_WindowPtr = orig_windowptr;

    /* close any possibly open canvas */
    nextcanvas = canvaslist;
    while ((canvas = nextcanvas)) {
        nextcanvas = canvas->next;

        video_canvas_destroy(canvas);
    }
}

static int IsFullscreenEnabled(void)
{
    int b;
    resources_get_value("FullscreenEnabled", (void *)&b);
    return b;
}

static int IsFullscreenStatusbarEnabled(void)
{
    int b;
    resources_get_value("StatusBarEnabled", (void *)&b);
    return b;
}

#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
static int IsVideoOverlayEnabled(void)
{
    int b;
    resources_get_value("VideoOverlayEnabled", (void *)&b);
    return b;
}
#endif

static void closecanvaswindow(struct video_canvas_s *canvas)
{
    struct Window *window = canvas->os->window;
    if (window != NULL) {
        canvas->os->window = NULL;
        if (canvas->current_fullscreen == 0) {
            canvas->window_left = window->LeftEdge;
            canvas->window_top = window->TopEdge;
        }
        /* make sure the process window ref won't be bad */
        if (self->pr_WindowPtr == window) {
            self->pr_WindowPtr = orig_windowptr;
        }
        CloseWindow(window);
    }
}

static struct video_canvas_s *reopen(struct video_canvas_s *canvas, int width, int height)
{
    int amiga_width, amiga_height, fullscreen, fullscreenstatusbar, overlay;
    ULONG dispid = INVALID_ID; /* stfu compiler */
    int nofullscreen = 0;
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
    int nooverlay = 0;
#endif

    if (canvas == NULL) {
        return NULL;
    }

    fullscreen = IsFullscreenEnabled();
    fullscreenstatusbar = IsFullscreenStatusbarEnabled();
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
    overlay = IsVideoOverlayEnabled();
#else
    overlay = 0;
#endif

    /* if there is no change, don't bother with anything else */
    if ((canvas->current_fullscreen == fullscreen || width == 0 || height == 0) &&
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
        canvas->current_overlay == overlay &&
#endif
        canvas->os->visible_width == width && canvas->os->visible_height == height) {
            return canvas;
    }

    /* if changing to/from fullscreen, close screen and window */
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
    if ((canvas->current_fullscreen != fullscreen) || fullscreen || canvas->current_overlay != overlay || overlay) {
#else
    if ((canvas->current_fullscreen != fullscreen) || fullscreen) {
#endif
        pointer_show();
        ui_menu_destroy(canvas);
        statusbar_destroy(canvas);

#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
        if (canvas->os->vlayer_handle) {
            DetachVLayer(canvas->os->vlayer_handle);
            DeleteVLayerHandle(canvas->os->vlayer_handle);
            canvas->os->vlayer_handle = NULL;
        }
#endif
        closecanvaswindow(canvas);
        if (canvas->os->screen != NULL) {
            CloseScreen(canvas->os->screen);
            canvas->os->screen = NULL;
        }
    }

    /* free bitmap */
    if (canvas->os->window_bitmap != NULL) {
        FreeBitMap(canvas->os->window_bitmap);
        canvas->os->window_bitmap = NULL;
    }

#ifdef AMIGA_AROS
    if (unlockable_buffer != NULL) {
        lib_free(unlockable_buffer);
        unlockable_buffer = NULL;
    }

    if (renderRPort != NULL) {
        FreeRastPort(renderRPort);
        renderRPort = NULL;
    }
    if (backRPort != NULL) {
        FreeRastPort(backRPort);
        backRPort = NULL;
    }
#endif

    /* try to get screenmode to use */
    if (fullscreen) {
#ifdef HAVE_PROTO_CYBERGRAPHICS_H
        static const UBYTE depths_lowend[] = { 16, 15, 0 };
#ifdef AMIGA_MORPHOS
        static const UBYTE depths_highend[] = { 32, 24, 16, 15, 0 };
#endif
        const UBYTE *depths;
        int i;

        depths = depths_lowend;
#ifdef AMIGA_MORPHOS
        if (SysBase->MaxLocMem == NULL) {
            depths = depths_highend;
        }
#endif

        for (i = 0; depths[i]; i++) {
            dispid = BestCModeIDTags(CYBRBIDTG_Depth, depths[i],
                                     CYBRBIDTG_NominalWidth, width,
                                     CYBRBIDTG_NominalHeight, height + (fullscreenstatusbar ? statusbar_get_status_height() : 0),
                                     TAG_DONE);
            if (dispid != INVALID_ID) {
                break;
            }
        }
#else

        unsigned long cmodels = RGBFF_R5G5B5 | RGBFF_R5G6B5 | RGBFF_R5G5B5PC | RGBFF_R5G6B5PC;
        dispid = p96BestModeIDTags(P96BIDTAG_NominalWidth, width,
        /* FIXME: only ask for statusbar height if it should be shown */
                                   P96BIDTAG_NominalHeight, height + (fullscreenstatusbar ? statusbar_get_status_height() : 0),
                                   P96BIDTAG_FormatsAllowed, cmodels,
                                   TAG_DONE);
#endif

        if (dispid == INVALID_ID) {
            nofullscreen = 1;
        }
    }

    /* if fullscreen, open the screen */
    if (fullscreen && nofullscreen == 0) {
        static const UWORD penarray[1] = { ~0 };
        int amiga_depth;
#ifdef HAVE_PROTO_CYBERGRAPHICS_H
        amiga_width = GetCyberIDAttr(CYBRIDATTR_WIDTH, dispid);
        amiga_height = GetCyberIDAttr(CYBRIDATTR_HEIGHT, dispid);
        amiga_depth = GetCyberIDAttr(CYBRIDATTR_DEPTH, dispid);
#else
        amiga_width = p96GetModeIDAttr(dispid, P96IDA_WIDTH);
        amiga_height = p96GetModeIDAttr(dispid, P96IDA_HEIGHT);
#ifdef AMIGA_OS4
        amiga_depth = p96GetModeIDAttr(dispid, P96IDA_DEPTH);
#else
        amiga_depth = 8;
#endif
#endif

        /* open screen */
        canvas->os->screen = OpenScreenTags(NULL,
                                            SA_Width, amiga_width,
                                            SA_Height, amiga_height,
                                            SA_Depth, amiga_depth,
                                            SA_Quiet, TRUE,
                                            SA_ShowTitle, FALSE,
                                            SA_Type, CUSTOMSCREEN,
                                            SA_DisplayID, dispid,
                                            SA_Title, (ULONG)"VICE",
                                            SA_Pens, (ULONG)penarray,
                                            SA_SharePens, TRUE,
                                            SA_FullPalette, TRUE,
                                            TAG_DONE);

        /* could the screen be opened? */
        if (canvas->os->screen == NULL) {
            return NULL;
        }

        /* open window */
        canvas->os->window = OpenWindowTags(NULL,
                                            WA_CustomScreen, (ULONG)canvas->os->screen,
                                            WA_Width, canvas->os->screen->Width,
                                            WA_Height, canvas->os->screen->Height,
                                            WA_IDCMP, IDCMP_RAWKEY | IDCMP_MENUPICK | IDCMP_MENUVERIFY,
                                            WA_Backdrop, FALSE,
                                            WA_Borderless, TRUE,
                                            WA_Activate, TRUE,
                                            WA_NewLookMenus, TRUE,
                                            TAG_DONE);

        if (canvas->os->window == NULL) {
            CloseScreen(canvas->os->screen);
            canvas->os->screen = NULL;
            return NULL;
        }

#ifdef HAVE_PROTO_CYBERGRAPHICS_H
        FillPixelArray(&canvas->os->screen->RastPort, 0, 0,
                       canvas->os->screen->Width, canvas->os->screen->Height, 0);
#else
        p96RectFill(&canvas->os->screen->RastPort, 0, 0,
                    canvas->os->screen->Width, canvas->os->screen->Height, 0);
#endif

        pointer_set_default(POINTER_HIDE);
        pointer_hide();

        canvas->os->visible_width = canvas->os->screen->Width;
        canvas->os->visible_height = canvas->os->screen->Height;
        if (fullscreenstatusbar) {
            canvas->os->visible_height -= statusbar_get_status_height();
        }
    } else {
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
reopenwindow:
#endif
        /* if window already is open, just resize it, otherwise, open it */
        if (canvas->os->window != NULL) {
            ChangeWindowBox(canvas->os->window,
                            canvas->os->window->LeftEdge,
                            canvas->os->window->TopEdge,
                            canvas->os->window->BorderLeft + width + canvas->os->window->BorderRight,
                            canvas->os->window->BorderTop + height + statusbar_get_status_height() + canvas->os->window->BorderBottom);
            canvas->waiting_for_resize = 1;
        } else {
            int statusheight = statusbar_get_status_height();
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
            int have_resize = overlay && CGXVideoBase && nooverlay == 0;
#else
            const int have_resize = FALSE;
#endif
            canvas->os->window = OpenWindowTags(NULL,
                                                WA_Title, (ULONG)canvas->os->window_name,
                                                WA_Flags, WFLG_NOCAREREFRESH | WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET,
                                                WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_RAWKEY | IDCMP_SIZEVERIFY | IDCMP_CHANGEWINDOW | IDCMP_MENUPICK | IDCMP_MENUVERIFY,
                                                WA_Left, canvas->window_left,
                                                WA_Top, canvas->window_top,
                                                WA_InnerWidth, width,
                                                WA_InnerHeight, height + statusheight,
                                                WA_Activate, TRUE,
                                                WA_NewLookMenus, TRUE,
                                                WA_AutoAdjust, TRUE,
                                                have_resize ? WA_MinWidth : TAG_IGNORE, 64 * 2,
                                                have_resize ? WA_MinHeight : TAG_IGNORE, 48 * 2 + statusheight,
                                                have_resize ? WA_MaxWidth : TAG_IGNORE, ~0,
                                                have_resize ? WA_MaxHeight : TAG_IGNORE, ~0,
                                                have_resize ? WA_SizeGadget : TAG_IGNORE, TRUE,
                                                TAG_DONE);

            if (canvas->os->window == NULL) {
                return NULL;
            }
        }

        pointer_set_default(POINTER_SHOW);
        pointer_show();

        canvas->os->visible_width = width;
        canvas->os->visible_height = height;
    }

#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)

    if (overlay && CGXVideoBase && nooverlay == 0) {
        static const struct {
            int      srcfmt;
            int      pixfmt;
            fourcc_t yuvfmt;
        } vlayer_formats[] = {
            { SRCFMT_YCbCr16, 0, {FOURCC_YUY2} },
            { SRCFMT_RGB16, PIXFMT_RGB16PC, {0} },
            { SRCFMT_RGB15, PIXFMT_RGB15PC, {0} },
            { -1, }
        };

        int i;

        for (i = 0; vlayer_formats[i].srcfmt != -1; i++) {
            canvas->os->vlayer_handle = CreateVLayerHandleTags(canvas->os->window->WScreen,
                                                               VOA_SrcType, vlayer_formats[i].srcfmt,
                                                               VOA_SrcWidth, width,
                                                               VOA_SrcHeight, height,
                                                               VOA_UseColorKey, TRUE,
                                                               TAG_DONE);
            if (canvas->os->vlayer_handle) {
                break;
            }
        }

        if (canvas->os->vlayer_handle != NULL) {
            int statusheight = canvas->os->screen == NULL || fullscreenstatusbar ? statusbar_get_status_height() : 0;

            canvas->os->pixfmt = vlayer_formats[i].pixfmt;
            canvas->vlayer_yuvfmt.id = vlayer_formats[i].yuvfmt.id;

            if (AttachVLayerTags(canvas->os->vlayer_handle, canvas->os->window, VOA_BottomIndent, statusheight, TAG_DONE) == 0) {
                struct Window *window = canvas->os->window;

                canvas->vlayer_image.width = width * 2;
                canvas->vlayer_image.height = height;
                canvas->vlayer_image.data_size = width * 2 * height * 2;
                canvas->vlayer_image.num_planes = 0;
                canvas->vlayer_image.pitches = canvas->vlayer_pitches;
                canvas->vlayer_pitches[0] = width * 2;
                canvas->vlayer_image.offsets = canvas->vlayer_offsets;
                canvas->vlayer_offsets[0] = 0;
                canvas->vlayer_image.data = NULL;

                canvas->os->bpr = GetVLayerAttr(canvas->os->vlayer_handle, VOA_Modulo);
                canvas->os->bpp = 2;

                canvas->os->vlayer_colorkey = GetVLayerAttr(canvas->os->vlayer_handle, VOA_ColorKey);
                if ((LONG)canvas->os->vlayer_colorkey != -1) {
                    FillPixelArray(window->RPort,
                                   window->BorderLeft, window->BorderTop,
                                   window->Width - window->BorderLeft - window->BorderRight,
                                   window->Height - window->BorderTop - window->BorderBottom - statusheight,
                                   canvas->os->vlayer_colorkey);
                }
            } else {
                DeleteVLayerHandle(canvas->os->vlayer_handle);
                canvas->os->vlayer_handle = NULL;
            }
        }

        if (canvas->os->vlayer_handle == NULL && canvas->os->screen == NULL) {
            /* overlay creation failed for some reason, close window and reopen without resize */
            closecanvaswindow(canvas);
            nooverlay = 1;
            goto reopenwindow;
        }
    }
#endif

    if (canvas->os->screen == NULL || fullscreenstatusbar) {
        statusbar_create(canvas);
    }

#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
    if (canvas->os->vlayer_handle == NULL) {
#endif
        canvas->os->window_bitmap = AllocBitMap(width, height,
                                                GetBitMapAttr(canvas->os->window->RPort->BitMap, BMA_DEPTH),
                                                BMF_CLEAR | BMF_INTERLEAVED | BMF_MINPLANES,
                                                canvas->os->window->RPort->BitMap);
        if (canvas->os->window_bitmap == NULL) {
            closecanvaswindow(canvas);
            if (canvas->os->screen != NULL) {
                CloseScreen(canvas->os->screen);
                canvas->os->screen = NULL;
            }
            return NULL;
        }

#ifdef HAVE_PROTO_CYBERGRAPHICS_H
        /* make sure we didn't get some incompatible bitmap */
        if (GetCyberMapAttr(canvas->os->window_bitmap, CYBRMATTR_ISCYBERGFX) == 0) {
            FreeBitMap(canvas->os->window_bitmap);
            canvas->os->window_bitmap = NULL;
            closecanvaswindow(canvas);
            if (canvas->os->screen != NULL) {
                CloseScreen(canvas->os->screen);
                canvas->os->screen = NULL;
            }
            return NULL;
        }

#ifdef AMIGA_AROS
        unlockable_buffer = lib_malloc(width * 2 * height * 2 * 4);

        renderRPort = CloneRastPort(canvas->os->window->RPort);

        backRPort = CreateRastPort();
        backRPort->BitMap = canvas->os->window_bitmap;
#endif

        canvas->os->pixfmt = GetCyberMapAttr(canvas->os->window_bitmap, CYBRMATTR_PIXFMT);
        canvas->os->bpr = GetCyberMapAttr(canvas->os->window_bitmap, CYBRMATTR_XMOD);
        canvas->os->bpp = GetCyberMapAttr(canvas->os->window_bitmap, CYBRMATTR_BPPIX);
#else
        canvas->os->pixfmt = p96GetBitMapAttr(canvas->os->window_bitmap, P96BMA_RGBFORMAT);
        canvas->os->bpr = p96GetBitMapAttr(canvas->os->window_bitmap, P96BMA_BYTESPERROW);
        canvas->os->bpp = p96GetBitMapAttr(canvas->os->window_bitmap, P96BMA_BYTESPERPIXEL);
#endif
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
    }
#endif

    canvas->draw_buffer->canvas_physical_width = width;
    canvas->draw_buffer->canvas_physical_height = height;
    canvas->depth = (canvas->os->bpp * 8);
    canvas->bytes_per_line = canvas->os->bpr;
    canvas->use_triple_buffering = 0;

    video_canvas_set_palette(canvas, canvas->palette);

    /* refresh */
    video_canvas_refresh_all(canvas);

    /* remember previous state */
    canvas->current_fullscreen = fullscreen;
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
    canvas->current_overlay = overlay;
#endif
    /* make sure we get the possible requesters */
    if (self->pr_WindowPtr == orig_windowptr) {
      self->pr_WindowPtr = canvas->os->window;
    }

    return canvas;
}

struct video_canvas_s *video_canvas_create(struct video_canvas_s *canvas, unsigned int *width, unsigned int *height, int mapped)
{
    int i;

    canvas->next = NULL;
    canvas->os = lib_malloc(sizeof(struct os_s));
    if (canvas->os == NULL) {
      return NULL;
    }
    memset(canvas->os, 0, sizeof(struct os_s));
    for (i = 0; i < 16; i++) {
        canvas->os->pens[i] = -1;
    }

    canvas->os->window_name = lib_stralloc(canvas->viewport->title);
    if (canvas->os->window_name == NULL) {
        lib_free(canvas->os);
        canvas->os = NULL;
        return NULL;
    }

    if (reopen(canvas, *width, *height) == NULL) {
        lib_free(canvas->os->window_name);
        lib_free(canvas->os);
        canvas->os = NULL;
        return NULL;
    }

    if (canvaslist == NULL) {
        canvaslist = canvas;
    } else {
        video_canvas_t *node = canvaslist;
        while (node->next != NULL) {
            node = node->next;
        }
        node->next = canvas;
    }

    return canvas;
}

void video_arch_canvas_init(struct video_canvas_s *canvas)
{
    canvas->os = NULL;
    canvas->video_draw_buffer_callback = NULL;
    canvas->window_left = 100;
    canvas->window_top = 100;
    canvas->current_fullscreen = 0;
    canvas->waiting_for_resize = 0;
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
    canvas->current_overlay = 0;
#endif
#ifdef AMIGA_AROS
    canvas->videoconfig->readable = 1; /* it's not direct rendering */
#endif
}

void video_canvas_refresh(struct video_canvas_s *canvas, unsigned int xs, unsigned int ys, unsigned int xi, unsigned int yi, unsigned int w, unsigned int h)
{
    int dx, dy, sx, sy;
    ULONG lock;
#ifdef HAVE_PROTO_CYBERGRAPHICS_H
    ULONG cgx_base_addy;
#else
    struct RenderInfo ri;
#endif
#ifdef AMIGA_AROS
    int fullscreen;

    fullscreen = IsFullscreenEnabled();
#endif

#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)

    if (canvas->os->vlayer_handle) {
        if (LockVLayer(canvas->os->vlayer_handle)) {

            cgx_base_addy = GetVLayerAttr(canvas->os->vlayer_handle, VOA_BaseAddress);

            if (canvas->vlayer_yuvfmt.id == 0) {
                /* it's RGB overlay, render to it directly */
                video_canvas_render(canvas, (UBYTE *)cgx_base_addy,
                                    w, h,
                                    xs, ys,
                                    xi, yi,
                                    canvas->bytes_per_line, canvas->depth);
            } else {
                /* everything else is preset at creation time */
                canvas->vlayer_image.data = (APTR)cgx_base_addy;

                if (!canvas->videoconfig->color_tables.updated) { /* update colors as necessary */
                    video_render_update_palette(canvas);
                }
                render_yuv_image(canvas->videoconfig->doublescan, (canvas->videoconfig->filter == VIDEO_FILTER_CRT),
                                 canvas->videoconfig->video_resources.pal_blur * 64 / 1000, canvas->videoconfig->video_resources.pal_scanlineshade * 1024 / 1000,
                                 canvas->vlayer_yuvfmt, &canvas->vlayer_image, canvas->draw_buffer->draw_buffer,
                                 canvas->draw_buffer->draw_buffer_width, canvas->videoconfig,
                                 xs, ys,
                                 w, h,
                                 xi, yi);
            }

            UnlockVLayer(canvas->os->vlayer_handle);
        }
        return;
    }

#endif

    xi *= canvas->videoconfig->scalex;
    w *= canvas->videoconfig->scalex;

    yi *= canvas->videoconfig->scaley;
    h *= canvas->videoconfig->scaley;

#ifdef AMIGA_OS4
    /* this has to be done before locking the bitmap to avoid disk accesses when the lock is held */
    if (!canvas->videoconfig->color_tables.updated) {
        video_color_update_palette(canvas);
    }
#endif

#ifdef HAVE_PROTO_CYBERGRAPHICS_H
    if ((lock = (ULONG)LockBitMapTags(canvas->os->window_bitmap, LBMI_BASEADDRESS, (ULONG)&cgx_base_addy, TAG_DONE))) {
#else
    if ((lock = p96LockBitMap(canvas->os->window_bitmap, (UBYTE *)&ri, sizeof(ri)))) {
#endif
        video_canvas_render(canvas,
#ifdef HAVE_PROTO_CYBERGRAPHICS_H
                            (UBYTE *)cgx_base_addy,
#else
                            (UBYTE *)ri.Memory,
#endif
                            w, h,
                            xs, ys,
                            xi, yi,
                            canvas->bytes_per_line, canvas->depth);
#ifdef HAVE_PROTO_CYBERGRAPHICS_H
        UnLockBitMap((APTR)lock);
#else
        p96UnlockBitMap(canvas->os->window_bitmap, lock);
#endif
    }
#ifdef AMIGA_AROS
    else {
        /* We failed to lock the bitmap - so render into our flatbuffer then WPA that to the backbuffer .. */
        struct Window *window = canvas->os->window;

        video_canvas_render(canvas,
                            (UBYTE *)unlockable_buffer,
                            w, h,
                            xs, ys,
                            xi, yi,
                            canvas->bytes_per_line, canvas->depth);

        WritePixelArray((UBYTE *)unlockable_buffer,
                        0, 0,
                        canvas->bytes_per_line, backRPort,
                        0, 0,
                        window->Width - window->BorderLeft - window->BorderRight,
                        window->Height - window->BorderTop - window->BorderBottom - statusbar_get_status_height(),
                        RECTFMT_RAW);
    }
#endif

    sx = xi;
    sy = yi;
    dx = xi + ((canvas->os->visible_width - (int)canvas->draw_buffer->canvas_physical_width) / 2);
    dy = yi + ((canvas->os->visible_height - (int)canvas->draw_buffer->canvas_physical_height) / 2);
    if (dx < 0) {
        sx += -dx;
        w += dx;
        dx = 0;
    }
    if (dy < 0) {
        sy += -dy;
        h += dy;
        dy = 0;
    }
    if (w > (unsigned int)canvas->os->visible_width) {
        w = canvas->os->visible_width;
    }
    if (h > (unsigned int)canvas->os->visible_height) {
        h = canvas->os->visible_height;
    }

    if (canvas->waiting_for_resize == 0 && w > 0 && h > 0) {
        struct Window *window = canvas->os->window;
#ifdef AMIGA_AROS
        if (fullscreen == 0) {
            int blit_width = canvas->draw_buffer->canvas_physical_width;
            int blit_height = canvas->draw_buffer->canvas_physical_height;
   
            if (blit_width > (window->RPort->Layer->bounds.MaxX - window->RPort->Layer->bounds.MinX)) {
                blit_width = (window->RPort->Layer->bounds.MaxX - window->RPort->Layer->bounds.MinX);
            }
            if (blit_height > (window->RPort->Layer->bounds.MaxY - window->RPort->Layer->bounds.MinY)) {
                blit_height = (window->RPort->Layer->bounds.MaxY - window->RPort->Layer->bounds.MinY);
            }
            ClipBlit(backRPort, 0, 0, renderRPort,
                     window->BorderLeft, window->BorderTop,
                     blit_width, blit_height,
                     0xc0);
        } else
#endif
            BltBitMapRastPort(canvas->os->window_bitmap,
                              sx, sy,
                              window->RPort,
                              window->BorderLeft + dx, window->BorderTop + dy,
                              w, h,
                              0xc0);
    }
}

/* dummy */

static int makecol_dummy(int r, int g, int b)
{
    return 0;
}

/* 16bit - BE */

static int makecol_RGB565BE(int r, int g, int b)
{
    int c = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3);
    return c;
}

#ifdef HAVE_PROTO_CYBERGRAPHICS_H
static int makecol_BGR565BE(int r, int g, int b)
{
    int c = ((b & 0xf8) << 8) | ((g & 0xfc) << 3) | ((r & 0xf8) >> 3);
    return c;
}
#endif

static int makecol_RGB555BE(int r, int g, int b)
{
    int c = ((r & 0xf8) << 7) | ((g & 0xf8) << 2) | ((b & 0xf8) >> 3);
    return c;
}

#ifdef HAVE_PROTO_CYBERGRAPHICS_H
static int makecol_BGR555BE(int r, int g, int b)
{
    int c = ((b & 0xf8) << 7) | ((g & 0xf8) << 2) | ((r & 0xf8) >> 3);
    return c;
}
#endif

/* 16bit - LE */

static int makecol_RGB565LE(int r, int g, int b)
{
    int c = ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3);
    c = ((c << 8) & 0xff00) | ((c >> 8) & 0x00ff);
    return c;
}

static int makecol_BGR565LE(int r, int g, int b)
{
    int c = ((b & 0xf8) << 8) | ((g & 0xfc) << 3) | ((r & 0xf8) >> 3);
    c = ((c << 8) & 0xff00) | ((c >> 8) & 0x00ff);
    return c;
}

static int makecol_RGB555LE(int r, int g, int b)
{
    int c = ((r & 0xf8) << 7) | ((g & 0xf8) << 2) | ((b & 0xf8) >> 3);
    c = ((c << 8) & 0xff00) | ((c >> 8) & 0x00ff);
    return c;
}

static int makecol_BGR555LE(int r, int g, int b)
{
    int c = ((b & 0xf8) << 7) | ((g & 0xf8) << 2) | ((r & 0xf8) >> 3);
    c = ((c << 8) & 0xff00) | ((c >> 8) & 0x00ff);
    return c;
}

/* 24bit (swapped these as VICE read from LSB, *NOT* MSB when rendering) */

static int makecol_RGB24(int r, int g, int b)
{
    int c = (b << 16) | (g << 8) | r;
    return c;
}

static int makecol_BGR24(int r, int g, int b)
{
    int c = (r << 16) | (g << 8) | b;
    return c;
}

/* 32bit */

static int makecol_ARGB32(int r, int g, int b)
{
    int c = (r << 16) | (g << 8) | b;
    return c;
}

static int makecol_ABGR32(int r, int g, int b)
{
    int c = (b << 16) | (g << 8) | r;
    return c;
}

static int makecol_RGBA32(int r, int g, int b)
{
    int c = (r << 24) | (g << 16) | (b << 8);
    return c;
}

static int makecol_BGRA32(int r, int g, int b)
{
    int c = (b << 24) | (g << 16) | (r << 8);
    return c;
}

#ifdef HAVE_PROTO_CYBERGRAPHICS_H
#if defined(AMIGA_AROS) && !defined(WORDS_BIGENDIAN)
static const struct {
    unsigned long color_format;
    int (*makecol)(int r, int g, int b);
} color_formats[] = {
    { PIXFMT_RGB15, makecol_RGB555LE },
    { PIXFMT_BGR15, makecol_BGR555LE },
    { PIXFMT_RGB15PC, makecol_RGB555BE },
    { PIXFMT_BGR15PC, makecol_BGR555BE },
    { PIXFMT_RGB16, makecol_RGB565LE },
    { PIXFMT_BGR16, makecol_BGR565LE },
    { PIXFMT_RGB16PC, makecol_RGB565BE },
    { PIXFMT_BGR16PC, makecol_BGR565BE },
    { PIXFMT_RGB24, makecol_BGR24 },
    { PIXFMT_BGR24, makecol_RGB24 },
    { PIXFMT_ARGB32, makecol_BGRA32 },
    { PIXFMT_BGRA32, makecol_ARGB32 },
    { PIXFMT_RGBA32, makecol_ABGR32 },
    { PIXFMT_ABGR32, makecol_RGBA32 },
    { 0, NULL }
};
#else
static const struct {
    unsigned long color_format;
    int (*makecol)(int r, int g, int b);
} color_formats[] = {
    { PIXFMT_RGB15, makecol_RGB555BE },
    { PIXFMT_BGR15, makecol_BGR555BE },
    { PIXFMT_RGB15PC, makecol_RGB555LE },
    { PIXFMT_BGR15PC, makecol_BGR555LE },
    { PIXFMT_RGB16, makecol_RGB565BE },
    { PIXFMT_BGR16, makecol_BGR565BE },
    { PIXFMT_RGB16PC, makecol_RGB565LE },
    { PIXFMT_BGR16PC, makecol_BGR565LE },
    { PIXFMT_RGB24, makecol_RGB24 },
    { PIXFMT_BGR24, makecol_BGR24 },
    { PIXFMT_ARGB32, makecol_ARGB32 },
    { PIXFMT_BGRA32, makecol_BGRA32 },
    { PIXFMT_RGBA32, makecol_RGBA32 },
    { 0, NULL }
};
#endif
#else
static const struct {
    unsigned long color_format;
    int (*makecol)(int r, int g, int b);
} color_formats[] = {
    { RGBFB_R8G8B8, makecol_RGB24 }, /* TrueColor RGB (8 bit each) */
    { RGBFB_B8G8R8, makecol_BGR24 }, /* TrueColor BGR (8 bit each) */
    { RGBFB_R5G6B5PC, makecol_RGB565LE }, /* HiColor16 (5 bit R, 6 bit G, 5 bit B), format: gggbbbbbrrrrrggg */
    { RGBFB_R5G5B5PC, makecol_RGB555LE }, /* HiColor15 (5 bit each), format: gggbbbbb0rrrrrgg */
    { RGBFB_A8R8G8B8, makecol_ARGB32 }, /* 4 Byte TrueColor ARGB (A unused alpha channel) */
    { RGBFB_A8B8G8R8, makecol_ABGR32 }, /* 4 Byte TrueColor ABGR (A unused alpha channel) */
    { RGBFB_R8G8B8A8, makecol_RGBA32 }, /* 4 Byte TrueColor RGBA (A unused alpha channel) */
    { RGBFB_B8G8R8A8, makecol_BGRA32 }, /* 4 Byte TrueColor BGRA (A unused alpha channel) */
    { RGBFB_R5G6B5, makecol_RGB565BE }, /* HiColor16 (5 bit R, 6 bit G, 5 bit B), format: rrrrrggggggbbbbb */
    { RGBFB_R5G5B5, makecol_RGB555BE }, /* HiColor15 (5 bit each), format: 0rrrrrgggggbbbbb */
    { RGBFB_B5G6R5PC, makecol_BGR565LE }, /* HiColor16 (5 bit R, 6 bit G, 5 bit B), format: gggrrrrrbbbbbggg */
    { RGBFB_B5G5R5PC, makecol_BGR555LE }, /* HiColor15 (5 bit each), format: gggrrrrr0bbbbbbgg */
    /* END */
    { 0, NULL },
};
#endif

int video_canvas_set_palette(struct video_canvas_s *canvas, struct palette_s *palette)
{
    int (*makecol)(int r, int g, int b);
    unsigned int i;
    int col;

    if (palette == NULL) {
        return 0; /* no palette, nothing to do */
    }

    canvas->palette = palette;

    i = 0;
    makecol = makecol_dummy;

    while (color_formats[i].makecol != NULL) {
        if (color_formats[i].color_format == canvas->os->pixfmt) {
            makecol = color_formats[i].makecol;
           break;
        }
        i++;
    }

    for (i = 0; i < canvas->palette->num_entries; i++) {
        if (canvas->depth == 8) {
            col = 0;
        } else {
            col = makecol(canvas->palette->entries[i].red,
                          canvas->palette->entries[i].green,
                          canvas->palette->entries[i].blue);
        }

        video_render_setphysicalcolor(canvas->videoconfig, i, col, canvas->depth);
    }

    if (canvas->depth > 8) {
        for (i = 0; i < 256; i++) {
            video_render_setrawrgb(i, makecol(i, 0, 0), makecol(0, i, 0), makecol(0, 0, i));
        }
        video_render_initraw(canvas->videoconfig);
    }

    return 0;
}

void video_canvas_destroy(struct video_canvas_s *canvas)
{
    if ((canvas != NULL) && (canvas->os != NULL)) {
        ui_menu_destroy(canvas);
        statusbar_destroy(canvas);
        lib_free(canvas->os->window_name);

#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
        if (canvas->os->vlayer_handle) {
            DetachVLayer(canvas->os->vlayer_handle);
            DeleteVLayerHandle(canvas->os->vlayer_handle);
            canvas->os->vlayer_handle = NULL;
        }
#endif

        closecanvaswindow(canvas);
        if (canvas->os->screen != NULL) {
            CloseScreen(canvas->os->screen);
            canvas->os->screen = NULL;
        }
        if (canvas->os->window_bitmap != NULL) {
            FreeBitMap(canvas->os->window_bitmap);
            canvas->os->window_bitmap = NULL;
        }

#ifdef AMIGA_AROS
        if (unlockable_buffer != NULL) {
            lib_free(unlockable_buffer);
            unlockable_buffer = NULL;
        }

        if (renderRPort != NULL) {
            FreeRastPort(renderRPort);
            renderRPort=NULL;
        }

        if (backRPort != NULL) {
            FreeRastPort(backRPort);
            backRPort = NULL;
        }
#endif

        if (canvaslist == canvas) {
            canvaslist = canvas->next;
        } else {
            video_canvas_t *node = canvaslist;
            while (node->next != canvas) {
                node = node->next;
            }
            node->next = canvas->next;
        }

        lib_free(canvas->os);
        canvas->os = NULL;
    }
}

void video_canvas_resize(struct video_canvas_s *canvas, char resize_canvas)
{
    if (reopen(canvas, canvas->draw_buffer->canvas_physical_width, canvas->draw_buffer->canvas_physical_height) == NULL) {
        exit(20);
    }
}

int video_arch_resources_init(void)
{
    return 0;
}

void video_arch_resources_shutdown(void)
{
}

static int fullscreen_update_needed = 0;

void video_arch_fullscreen_toggle(void)
{
    fullscreen_update_needed = 1; /* just remember the toggle */
}

void video_arch_fullscreen_update(void)
{
    if (fullscreen_update_needed == 1) {
        if (canvaslist != NULL) {
            if (reopen(canvaslist, canvaslist->draw_buffer->canvas_physical_width, canvaslist->draw_buffer->canvas_physical_height) == NULL) {
                exit(20);
            }
        }
        fullscreen_update_needed = 0;
    }
}

char video_canvas_can_resize(struct video_canvas_s *canvas)
{
    return 1;
}
