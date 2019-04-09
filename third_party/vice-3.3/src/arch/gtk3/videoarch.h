/**
 * \file   videoarch.h
 * \brief  Native GTK3 graphics routines
 *
 * \author Ettore Perazzoli
 * \author Teemu Rantanen <tvr@cs.hut.fi>
 * \author Andreas Boose <viceteam@t-online.de>
 * \author Michael C. Martin <mcmartin@gmail.com>
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
#ifndef VICE_VIDEOARCH_H
#define VICE_VIDEOARCH_H

#include "vice.h"

#include "viewport.h"
#include "video.h"

#include <gtk/gtk.h>

struct vice_renderer_backend_s;

/**
 * \brief Master data structure for a machine window's primary display.
 */
typedef struct video_canvas_s {
    /** \brief Nonzero if it is safe to access other members of the
     *         structure. */
    unsigned int initialized;
    /** \brief Nonzero if the structure has been fully realized. */
    unsigned int created;

    /** \brief Top-level widget that contains the full contents of the
     *         machine window. */
    GtkWidget *grid;
    /** \brief Child widget to which the emulated screen is drawn. 
     *
     *  Depending on what renderer backend is in use this will be
     *  either a GtkDrawingArea or a GtkGLArea. */
    GtkWidget *drawing_area;
    /** \brief The renderer backend selected for use this run. */
    struct vice_renderer_backend_s *renderer_backend;
    /** \brief Data unique to the renderer backend. This value is
     *         passed to all renderer methods. and is managed by
     *         them. */
    void *renderer_context;
    /** \brief Special "blank" cursor for cases where the mouse
     *         pointer should disappear. */
    GdkCursor *blank_ptr;
    /** \brief Special "target" cursor for active light pens. */
    GdkCursor *pen_ptr;
    /** \brief Number of frames the mouse hasn't moved while still on
     *         the canvas. */
    unsigned int still_frames;
    /** \brief Handle to the timer callback that will make the mouse
     *         disappear if it's hovered for too long over the screen
     *         display. */
    guint still_frame_callback_id;
    /** \brief Light pen X coordinate, in window coordinates. */
    int pen_x;
    /** \brief Light pen Y coordinate, in window coordinates. */
    int pen_y;
    /** \brief Light pen button status. */
    int pen_buttons;
    /** \brief Leftmost X coordinate of the actual machine's screen,
     *         in window coordinates. */
    double screen_origin_x;
    /** \brief Topmost Y coordinate of the actual machine's screen, in
     *         window coordinates. */
    double screen_origin_y;
    /** \brief Width of the actual machine's screen, in window
     *         coordinates. */
    double screen_display_w;
    /** \brief Height of the actual machine's screen, in window
     *         coordinates. */
    double screen_display_h;

    /** \brief Rendering configuration as seen by the emulator
     *         core. */
    struct video_render_config_s *videoconfig;
    /** \brief Drawing buffer as seen by the emulator core. */
    struct draw_buffer_s *draw_buffer;
    /** \brief Display window as seen by the emulator core. */
    struct viewport_s *viewport;
    /** \brief Machine screen geometry as seen by the emulator
     *         core. */
    struct geometry_s *geometry;
    /** \brief Color palette for translating display results into
     *         window colors. */
    struct palette_s *palette;
    /** \brief Methods for managing the draw buffer when the core
     *         rasterizer handles it. */
    struct video_draw_buffer_callback_s *video_draw_buffer_callback;

    /** \brief Which window contains this canvas.
     *  \sa ui_resources_s::canvas The array this value indexes */
    int window_index;
} video_canvas_t;

/** \brief Rescale and reposition the screen inside the canvas if the
 *         screen's size has been programatically changed. 
 *  \param canvas The canvas to adjust.
 */
void video_canvas_adjust_aspect_ratio(struct video_canvas_s *canvas);

/** \brief A collection of methods that abstract away the underlying
 *         display API.
 *
 *  GTK3's default software rendering (Cairo) and its accelerated one
 *  (OpenGL) use very different mechanisms for displaying (possibly
 *  scaled) pixel content or incrementally updating it. These routines
 *  let us keep those differences contained. */
typedef struct vice_renderer_backend_s {
    /** \brief Creates a widget suitable for this renderer to target.
     *
     *  Also initializes the opaque video_canvas_s::renderer_context
     *  field if needed, and sets other necessary fields.
     *
     *  \param canvas The canvas to create the widget for.
     *  \return The newly created canvas.
     */
    GtkWidget *(*create_widget)(video_canvas_t *canvas);
    /** \brief Creates or resizes the pixel buffer that this renderer
     *         backend is using for the screen.
     *
     * This is an expensive operation if the width and height have
     * changed since the last call.
     *
     * \param canvas The canvas being resized or initially created.
     * \param width The new width for the machine's screen.
     * \param height The new height for the machine's screen.
     */
    void (*update_context)(video_canvas_t *canvas,
                           unsigned int width, unsigned int height);
    /** \brief Clean up any resources used by this renderer backend,
     *         in preparation for destruction or recreation.
     * 
     *  \param canvas The canvas whose renderer_context is to be
     *                deleted
     */
    void (*destroy_context)(video_canvas_t *canvas);
    /** \brief Render pixels in the specified rectangle.
     *
     * This both asks the emulator core to update the renderer context
     * and asks the UI to display the changed results.
     *
     * \param canvas The canvas being rendered to
     * \param xs     A parameter to forward to video_canvas_render()
     * \param ys     A parameter to forward to video_canvas_render()
     * \param xi     X coordinate of the leftmost pixel to update
     * \param yi     Y coordinate of the topmost pixel to update
     * \param w      Width of the rectangle to update
     * \param h      Height of the rectangle to update
     */
    void (*refresh_rect)(video_canvas_t *canvas,
                         unsigned int xs, unsigned int ys,
                         unsigned int xi, unsigned int yi,
                         unsigned int w, unsigned int h);
    /** \brief Initialize the palette for this renderer.
     *
     * \param canvas The canvas being initialized
     */
    void (*set_palette)(video_canvas_t *canvas);
} vice_renderer_backend_t;

#endif
