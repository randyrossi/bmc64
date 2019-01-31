/** \file   quartz_renderer.c
 * \brief   Quartz-based renderer for the GTK3 backend
 *
 * \author  Michael C. Martin <mcmartin@gmail.com>
 */

/*
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

#include "quartz_renderer.h"

#ifdef MACOSX_SUPPORT

#include <CoreGraphics/CoreGraphics.h>

#include <cairo/cairo-quartz.h>

#include "lib.h"
#include "resources.h"
#include "ui.h"
#include "video.h"

typedef struct vice_quartz_renderer_context_s {
    CGContextRef bitmap_context;
    CGLayerRef layer;
    double scale_x, scale_y;
} context_t;

/* Note that the ::draw signal receives a ready-to-be-used cairo_t
 * that is already clipped to only draw the exposed areas of the
 * widget */
static gboolean
draw_canvas_quartz_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    video_canvas_t *canvas = (video_canvas_t *)data;
    context_t *ctx = canvas ? (context_t *)canvas->renderer_context : NULL;
    cairo_surface_t *target = cairo_get_target(cr);
    CGContextRef display = NULL;
    CGRect clip_rect;

    cairo_surface_flush(target);
    display = cairo_quartz_surface_get_cg_context(target);
    if (!display) {
        fprintf(stderr, "Rendering impossible as the display area isn't a Quartz surface: %d\n", cairo_surface_get_type(target));
        return FALSE;
    }

    CGContextSaveGState(display);
    CGContextScaleCTM(display, 1.0, -1.0);
    clip_rect = CGContextGetClipBoundingBox(display);

    if (ui_is_fullscreen()) {
        CGContextSetRGBFillColor(display, 0.0f, 0.0f, 0.0f, 1.0f);
    } else {
        CGContextSetRGBFillColor(display, 0.5f, 0.5f, 0.5f, 1.0f);
    }
    CGContextFillRect(display, clip_rect);

    if (ctx && ctx->bitmap_context) {
        CGImageRef frame = CGBitmapContextCreateImage(ctx->bitmap_context);
        if (!ctx->layer) {
            CGSize targetSize = { CGBitmapContextGetWidth(ctx->bitmap_context),
                                  CGBitmapContextGetHeight(ctx->bitmap_context) };
            ctx->layer = CGLayerCreateWithContext(ctx->bitmap_context, targetSize, NULL);
        }
        if (ctx->layer) {
            CGRect layer_rect = { { 0, 0 }, CGLayerGetSize(ctx->layer) };
            CGRect img_rect = clip_rect;
            img_rect.size.width *= ctx->scale_x;
            img_rect.size.height *= ctx->scale_y;

            CGContextDrawImage(CGLayerGetContext(ctx->layer), layer_rect, frame);
            CGContextScaleCTM(display,
                              img_rect.size.width / layer_rect.size.width,
                              img_rect.size.height / layer_rect.size.height);
            clip_rect = CGContextGetClipBoundingBox(display);
            img_rect = clip_rect;
            img_rect.size.width *= ctx->scale_x;
            img_rect.size.height *= ctx->scale_y;
            img_rect.origin.x += (clip_rect.size.width - img_rect.size.width) / 2;
            img_rect.origin.y += (clip_rect.size.height - img_rect.size.height) / 2;
            CGContextSetInterpolationQuality(display, kCGInterpolationNone);
            CGContextDrawLayerAtPoint(display, img_rect.origin, ctx->layer);
        } else {
            CGRect img_rect = clip_rect;
            img_rect.size.width *= ctx->scale_x;
            img_rect.size.height *= ctx->scale_y;
            img_rect.origin.x += (clip_rect.size.width - img_rect.size.width) / 2;
            img_rect.origin.y += (clip_rect.size.height - img_rect.size.height) / 2;
            CGContextDrawImage(display, img_rect, frame);
        }
        CGImageRelease(frame);
    }
    CGContextRestoreGState(display);
    cairo_surface_mark_dirty(target);

    return FALSE;
}

/** \brief  Callback to handle cases where the window is resized but
 *          the canvas is not
 */
static gboolean
resize_canvas_container_quartz_cb (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
    /* The GtkDrawingArea that holds the canvas is "widget." */
    video_canvas_t *canvas = (video_canvas_t *)data;
    context_t *ctx = canvas ? (context_t *)canvas->renderer_context : NULL;
    if (ctx && ctx->bitmap_context) {
        /* Size of source canvas */
        double source_width = (double)CGBitmapContextGetWidth(ctx->bitmap_context);
        double source_height = (double)CGBitmapContextGetHeight(ctx->bitmap_context);
        /* Size of widget */
        double width = (double)gtk_widget_get_allocated_width(widget);
        double height = (double)gtk_widget_get_allocated_height(widget);
        double scale_x = 1.0, scale_y = 1.0;
        int keepaspect=1, trueaspect=0;
        resources_get_int("KeepAspectRatio", &keepaspect);
        resources_get_int("TrueAspectRatio", &trueaspect);

        if (source_width < 1 || source_height < 1) {
            return FALSE;
        }

        if (keepaspect) {
            double canvas_aspect, viewport_aspect;

            viewport_aspect = width / height;
            canvas_aspect = source_width / source_height;
            if (trueaspect) {
                canvas_aspect *= canvas->geometry->pixel_aspect_ratio;
            }
            if (canvas_aspect < viewport_aspect) {
                scale_x = canvas_aspect / viewport_aspect;
                scale_y = 1.0f;
            } else {
                scale_x = 1.0f;
                scale_y = viewport_aspect / canvas_aspect;
            }
        } else {
            scale_x = 1.0;
            scale_y = 1.0;
        }
        ctx->scale_x = scale_x;
        ctx->scale_y = scale_y;
        canvas->screen_display_w = (double)width * ctx->scale_x;
        canvas->screen_display_h = (double)height * ctx->scale_y;
        canvas->screen_origin_x = ((double)width - canvas->screen_display_w) / 2.0;
        canvas->screen_origin_y = ((double)height - canvas->screen_display_h) / 2.0;
    }
    /* No further processing should be needed */
    return FALSE;
}

static GtkWidget *vice_quartz_create_widget(video_canvas_t *canvas)
{
    GtkWidget *widget = gtk_drawing_area_new();
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_widget_set_vexpand(widget, TRUE);
    canvas->drawing_area = widget;
    g_signal_connect(widget, "draw", G_CALLBACK(draw_canvas_quartz_cb), canvas);
    g_signal_connect(widget, "configure_event", G_CALLBACK(resize_canvas_container_quartz_cb), canvas);
    return widget;
}

static void vice_quartz_destroy_context(video_canvas_t *canvas)
{
    if (canvas) {
        context_t *ctx = (context_t *)canvas->renderer_context;
        if (!ctx) {
            return;
        }
        if (ctx->bitmap_context) {
            CGContextRelease(ctx->bitmap_context);
        }
        if (ctx->layer) {
            CGLayerRelease(ctx->layer);
        }
        lib_free(ctx);
        canvas->renderer_context = NULL;
    }
}

static void vice_quartz_update_context(video_canvas_t *canvas, unsigned int width, unsigned int height)
{
    if (canvas) {
        context_t *ctx = (context_t *)canvas->renderer_context;

        if (ctx && ctx->bitmap_context) {
            unsigned int source_width, source_height;
            source_width = CGBitmapContextGetWidth(ctx->bitmap_context);
            source_height = CGBitmapContextGetHeight(ctx->bitmap_context);
            if (source_width == width && source_height == height) {
                /* Canvas already exists and is the proper size */
                return;
            }
        } else {
            if (width == 0 || height == 0) {
                /* We have no surface and were asked to create a
                 * surface of area zero, so we're done */
                return;
            }
        }
        /* If we get this far, we have new dimensions for our canvas */
        vice_quartz_destroy_context(canvas);
        ctx = lib_malloc(sizeof(context_t));
        if (ctx) {
            ctx->bitmap_context = NULL;
            ctx->layer = NULL;
            ctx->scale_x = 1.0;
            ctx->scale_y = 1.0;
        }
        canvas->renderer_context = ctx;
        if (width != 0 && height != 0 && ctx) {
            /* Actually create the bitmap context */
            int keepaspect=1, trueaspect=0;
            double aspect = 1.0;
            CGColorSpaceRef colorspace = NULL;
            resources_get_int("KeepAspectRatio", &keepaspect);
            resources_get_int("TrueAspectRatio", &trueaspect);
            if (keepaspect && trueaspect) {
                aspect = canvas->geometry->pixel_aspect_ratio;
            }

            /* Possible TODO: If someone wants to get real fancy they
             * could they could, at least on Macs, define reference
             * whites and blacks and use a calibrated RGB system. As a
             * rule we seem to address this by tuning our palettes in
             * VICE itself, though. */
            colorspace = CGColorSpaceCreateDeviceRGB();
            ctx->bitmap_context = CGBitmapContextCreate(NULL, width, height, 8, 0, colorspace, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little);
            CGColorSpaceRelease(colorspace);

            /* Configure the matrix to fit it in the widget as it exists */
            resize_canvas_container_quartz_cb (canvas->drawing_area, NULL, canvas);

            /* Fix the widget's size request */
            gtk_widget_set_size_request(canvas->drawing_area, width * aspect, height);
        }
    }
}

static void vice_quartz_refresh_rect(video_canvas_t *canvas,
                                    unsigned int xs, unsigned int ys,
                                    unsigned int xi, unsigned int yi,
                                    unsigned int w, unsigned int h)
{
    context_t *ctx = canvas ? (context_t *)canvas->renderer_context : NULL;
    unsigned char *backbuffer;
    if (!ctx || !ctx->bitmap_context) {
        return;
    }

    if (((xi + w) > CGBitmapContextGetWidth(ctx->bitmap_context)) ||
        ((yi+h) > CGBitmapContextGetHeight(ctx->bitmap_context))) {
        /* Trying to draw outside canvas? */
        fprintf(stderr, "Attempt to draw outside canvas!\nXI%u YI%u W%u H%u CW%lu CH%lu\n", xi, yi, w, h, CGBitmapContextGetWidth(ctx->bitmap_context), CGBitmapContextGetHeight(ctx->bitmap_context));
        return;
    }

    backbuffer = CGBitmapContextGetData(ctx->bitmap_context);
    if (!backbuffer) {
        return;
    }
    video_canvas_render(canvas, backbuffer, w, h, xs, ys, xi, yi,  CGBitmapContextGetBytesPerRow(ctx->bitmap_context), 32);
    gtk_widget_queue_draw(canvas->drawing_area);
}

static void vice_quartz_set_palette(video_canvas_t *canvas)
{
    int i;
    struct palette_s *palette = canvas ? canvas->palette : NULL;
    if (!palette) {
        return;
    }
    /* If we get this far we know canvas is also non-NULL */

    /* We use use the Device RGB color space with "no alpha, skip first", which leaves the most significant bits unused and should have R, G, B in decreasing significance in machine byte order. */
    for (i = 0; i < palette->num_entries; i++) {
        palette_entry_t color = palette->entries[i];
        uint32_t rgb_color = (color.red << 16) | (color.green << 8) | color.blue;
        video_render_setphysicalcolor(canvas->videoconfig, i, rgb_color, 32);
    }

    for (i = 0; i < 256; i++) {
        video_render_setrawrgb(i, i << 16, i << 8, i);
    }
    video_render_initraw(canvas->videoconfig);
}

vice_renderer_backend_t vice_quartz_backend = {
    vice_quartz_create_widget,
    vice_quartz_update_context,
    vice_quartz_destroy_context,
    vice_quartz_refresh_rect,
    vice_quartz_set_palette
};

#endif
