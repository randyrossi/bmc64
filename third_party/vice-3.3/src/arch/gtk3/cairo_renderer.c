/**
 * \file    cairo_renderer.c
 * \brief   Cairo-based renderer for the GTK3 backend
 *
 * \author  Michael C. Martin <mcmartin@gmail.com>
 */

/* This file is part of VICE, the Versatile Commodore Emulator.
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

#include "cairo_renderer.h"

#include <string.h>

#include "lib.h"
#include "resources.h"
#include "ui.h"
#include "video.h"

/** \brief Rendering context for the Cairo backend.
 *  \sa video_canvas_s::renderer_context */
typedef struct vice_cairo_renderer_context_s {
    /** \brief The cairo image surface that holds the pixels of the
     *         machine's screen. */
    cairo_surface_t *backing_surface;
    /** \brief The affine transform to scale and translate the backing
     *         surface when it is displayed on the canvas. */
    cairo_matrix_t transform;
} context_t;

/** \brief Rendering callback to display the screen as we understand
 *         it.
 *  \param widget The GtkDrawingArea we are rendering to.
 *  \param cr     The Cairo context that we draw through.
 *  \param data   The video_canvas_t we're rendering from.
 *  \return TRUE if no further processing is needed on this event.
 */
static gboolean
draw_canvas_cairo_cb (GtkWidget *widget, cairo_t *cr, gpointer data)
{
    video_canvas_t *canvas = (video_canvas_t *)data;
    context_t *ctx = canvas ? (context_t *)canvas->renderer_context : NULL;

    /* Half-grey background for those parts of the window that aren't
     * video, or black if it's fullscreen.
     * TODO: configurable? */
    if (ui_is_fullscreen()) {
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    } else {
        cairo_set_source_rgb(cr, 0.5, 0.5, 0.5);
    }
    cairo_paint(cr);

    if (ctx && ctx->backing_surface) {
        cairo_pattern_t *pattern = cairo_pattern_create_for_surface(ctx->backing_surface);
        cairo_pattern_set_matrix(pattern, &ctx->transform);
        cairo_pattern_set_filter(pattern, CAIRO_FILTER_FAST);
        cairo_set_source(cr, pattern);
        cairo_paint(cr);
        cairo_pattern_destroy(pattern);
    }

    return FALSE;
}

/** \brief  Callback to adjust scaling and offset when the window is
 *          resized but the underlying machine screen is not.
 *  \param widget The GtkDrawingArea being resized.
 *  \param event  The GdkEventConfigure that is triggered this callback.
 *  \param data   The video_canvas_t that controls this drawing area.
 *  \return TRUE if no further processing is needed on this event.
 */
static gboolean
resize_canvas_container_cairo_cb (GtkWidget *widget, GdkEventConfigure *event, gpointer data)
{
    /* The GtkDrawingArea that holds the canvas is "widget." */
    video_canvas_t *canvas = (video_canvas_t *)data;
    context_t *ctx = canvas ? (context_t *)canvas->renderer_context : NULL;
    if (ctx && ctx->backing_surface) {
        /* Size of source canvas */
        double source_width = (double)cairo_image_surface_get_width(ctx->backing_surface);
        double source_height = (double)cairo_image_surface_get_height(ctx->backing_surface);
        /* Size of widget */
        double width = (double)gtk_widget_get_allocated_width(widget);
        double height = (double)gtk_widget_get_allocated_height(widget);
        double scale_x = 1.0, scale_y = 1.0;
        double offset_x = 0.0, offset_y = 0.0;
        int keepaspect=1, trueaspect=0;
        resources_get_int("KeepAspectRatio", &keepaspect);
        resources_get_int("TrueAspectRatio", &trueaspect);

        if (keepaspect) {
            double aspect_fix = 1.0;
            if (trueaspect) {
                aspect_fix = canvas->geometry->pixel_aspect_ratio;
            }
            /* Try the Y-fit first */
            double scale = source_height / height;
            if (source_width * aspect_fix / scale > width) {
                /* Need to X-fit instead */
                scale = (double)source_width * aspect_fix / width;
                offset_y = ((source_height / scale) - height) / 2.0;
            } else {
                offset_x = ((source_width * aspect_fix / scale) - width) / 2.0;
            }
            scale_x = scale / aspect_fix;
            scale_y = scale;
        } else {
            scale_x = (double)source_width / width;
            scale_y = (double)source_height / height;
        }
        /* Apply the computed scaling factor to both dimensions */
        cairo_matrix_init_scale(&ctx->transform, scale_x, scale_y);
        /* Center the result in the widget */
        cairo_matrix_translate(&ctx->transform, offset_x, offset_y);
        /* Record the window coordinates of where the result will be */
        canvas->screen_display_w = (double)source_width / scale_x;
        canvas->screen_display_h = (double)source_height / scale_y;
        canvas->screen_origin_x = ((double)width - canvas->screen_display_w) / 2.0;
        canvas->screen_origin_y = ((double)height - canvas->screen_display_h) / 2.0;
    }
    /* No further processing should be needed */
    return FALSE;
}

/** \brief Cairo implementation of create_widget.
 *
 *  \param canvas The canvas to create the widget for.
 *  \return The newly created canvas.
 *  \sa vice_renderer_backend_s::create_widget
 */
static GtkWidget *vice_cairo_create_widget(video_canvas_t *canvas)
{
    GtkWidget *widget = gtk_drawing_area_new();
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_widget_set_vexpand(widget, TRUE);
    canvas->drawing_area = widget;
    g_signal_connect(widget, "draw", G_CALLBACK(draw_canvas_cairo_cb), canvas);
    g_signal_connect(widget, "configure_event", G_CALLBACK(resize_canvas_container_cairo_cb), canvas);
    return widget;
}

/** \brief Cairo implementation of destroy_context.
 * 
 *  \param canvas The canvas whose renderer_context is to be
 *                deleted
 *  \sa vice_renderer_backend_s::destroy_context
 */
static void vice_cairo_destroy_context(video_canvas_t *canvas)
{
    if (canvas) {
        context_t *ctx = (context_t *)canvas->renderer_context;
        if (!ctx) {
            return;
        }
        if (ctx->backing_surface) {
            cairo_surface_finish(ctx->backing_surface);
            cairo_surface_destroy(ctx->backing_surface);
        }
        lib_free(ctx);
        canvas->renderer_context = NULL;
    }
}

/** \brief Cairo implementation of update_context.
 * \param canvas The canvas being resized or initially created.
 * \param width The new width for the machine's screen.
 * \param height The new height for the machine's screen.
 * \sa vice_renderer_backend_s::update_context
 */
static void vice_cairo_update_context(video_canvas_t *canvas, unsigned int width, unsigned int height)
{
    if (canvas) {
        context_t *ctx = (context_t *)canvas->renderer_context;

        if (ctx && ctx->backing_surface) {
            unsigned int source_width, source_height;
            source_width = cairo_image_surface_get_width(ctx->backing_surface);
            source_height = cairo_image_surface_get_height(ctx->backing_surface);
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
        vice_cairo_destroy_context(canvas);
        ctx = lib_malloc(sizeof(context_t));
        if (ctx) {
            ctx->backing_surface = NULL;
            cairo_matrix_init_identity(&ctx->transform);
        }
        canvas->renderer_context = ctx;
        if (width != 0 && height != 0) {
            /* Actually create the backing surface */
            int keepaspect=1, trueaspect=0;
            double aspect = 1.0;
            resources_get_int("KeepAspectRatio", &keepaspect);
            resources_get_int("TrueAspectRatio", &trueaspect);
            if (keepaspect && trueaspect) {
                aspect = canvas->geometry->pixel_aspect_ratio;
            }

            ctx->backing_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);

            /* Configure the matrix to fit it in the widget as it exists */
            resize_canvas_container_cairo_cb (canvas->drawing_area, NULL, canvas);

            /* Fix the widget's size request */
            gtk_widget_set_size_request(canvas->drawing_area, width * aspect, height);
        }
    }
}

/** \brief Cairo implementation of refresh_rect.
 * \param canvas The canvas being rendered to
 * \param xs     A parameter to forward to video_canvas_render()
 * \param ys     A parameter to forward to video_canvas_render()
 * \param xi     X coordinate of the leftmost pixel to update
 * \param yi     Y coordinate of the topmost pixel to update
 * \param w      Width of the rectangle to update
 * \param h      Height of the rectangle to update
 * \sa vice_renderer_backend_s::refresh_rect */
static void vice_cairo_refresh_rect(video_canvas_t *canvas,
                                    unsigned int xs, unsigned int ys,
                                    unsigned int xi, unsigned int yi,
                                    unsigned int w, unsigned int h)
{
    context_t *ctx = canvas ? (context_t *)canvas->renderer_context : NULL;
    unsigned char *backbuffer;
    if (!ctx || !ctx->backing_surface) {
        return;
    }

    if (((xi + w) > cairo_image_surface_get_width(ctx->backing_surface)) ||
        ((yi+h) > cairo_image_surface_get_height(ctx->backing_surface))) {
        /* Trying to draw outside canvas? */
        fprintf(stderr, "Attempt to draw outside canvas!\nXI%u YI%u W%u H%u CW%u CH%u\n", xi, yi, w, h, cairo_image_surface_get_width(ctx->backing_surface), cairo_image_surface_get_height(ctx->backing_surface));
        return;
    }

    cairo_surface_flush(ctx->backing_surface);
    backbuffer = cairo_image_surface_get_data(ctx->backing_surface);
    if (!backbuffer) {
        return;
    }
    video_canvas_render(canvas, backbuffer, w, h, xs, ys, xi, yi, cairo_image_surface_get_stride(ctx->backing_surface), 32);
    cairo_surface_mark_dirty_rectangle(ctx->backing_surface, xi, yi, w, h);
    gtk_widget_queue_draw(canvas->drawing_area);
}

/** \brief Cairo implementation of set_palette.
 * \param canvas The canvas being initialized
 * \sa vice_renderer_backend_s::set_palette */
static void vice_cairo_set_palette(video_canvas_t *canvas)
{
    int i;
    struct palette_s *palette = canvas ? canvas->palette : NULL;
    if (!palette) {
        return;
    }
    /* If we get this far we know canvas is also non-NULL */

    /* We use CAIRO_FORMAT_RGB24, which is defined as follows: "Each
     * pixel is a 32-bit quantity, with the upper 8 bits unused. Red,
     * Green, and Blue are stored in the remaining 24 bits in that
     * order." */
    for (i = 0; i < palette->num_entries; i++) {
        palette_entry_t color = palette->entries[i];
        uint32_t cairo_color = (color.red << 16) | (color.green << 8) | color.blue;
        video_render_setphysicalcolor(canvas->videoconfig, i, cairo_color, 32);
    }

    for (i = 0; i < 256; i++) {
        video_render_setrawrgb(i, i << 16, i << 8, i);
    }
    video_render_initraw(canvas->videoconfig);
}

vice_renderer_backend_t vice_cairo_backend = {
    vice_cairo_create_widget,
    vice_cairo_update_context,
    vice_cairo_destroy_context,
    vice_cairo_refresh_rect,
    vice_cairo_set_palette
};
