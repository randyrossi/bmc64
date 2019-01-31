/**
 * \file    uimachinewindow.c
 * \brief   Native GTK3 main emulator window code.
 *
 * \author Marcus Sutton <loggedoubt@gmail.com>
 * \author Michael C. Martin <mcmartin@gmail.com>
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

/* \note It should be possible to compile, link and run vsid while
 *       this entire file (amongst others) is contained inside an #if
 *       0 wrapper.
 */
#if 1

#include "vice.h"

#include <gtk/gtk.h>

#include "cairo_renderer.h"
#include "opengl_renderer.h"
#include "quartz_renderer.h"
#include "lightpen.h"
#include "mousedrv.h"
#include "videoarch.h"

#include "ui.h"
#include "uimachinemenu.h"
#include "uimachinewindow.h"

/** \brief Last recoreded X position of the mouse, for computing
 *         relative movement.
 *  \todo  This caching method should be less awful.
 *  \sa    event_box_motion_cb */
static gdouble last_mouse_x = -1;

/** \brief Last recoreded Y position of the mouse, for computing
 *         relative movement.
 *  \todo  This caching method should be less awful.
 *  \sa    event_box_motion_cb */
static gdouble last_mouse_y = -1;

/** \brief If nonzero, the next mouse motion event will ignored by the
 *         mouse driver.
 *  \sa    event_box_motion_cb */
static int warping = 0;

/** \brief Callback for handling mouse motion events over the emulated
 *         screen.
 *
 *  Mouse motion events influence three different subsystems: the
 *  light-pen (if any), the emulated mouse (if any), and the UI-level
 *  routines that hide the mouse pointer if it comes to rest over the
 *  machine's screen.
 *
 *  Moving the mouse pointer resets the number of frames the mouse was
 *  held still.
 *
 *  Light pen position information is computed based on the new mouse
 *  position and what part of the machine window is actually in use
 *  based on current scaling and aspect ratio settings.
 *
 *  Mouse information is computed based on the difference between the
 *  current mouse location and the last recorded mouse location. If
 *  the mouse has been captured by the emulator, this also then warps
 *  the mouse pointer back to the middle of the emulated
 *  screen. (These warps will trigger an additional call to this
 *  function, but an additional flag will prevent them from being
 *  processed as true input.)
 *
 *  Information relevant to these processes is cached in the
 *  video_canvas_s structure for use as needed.
 *
 *  \param widget    The widget that sent the event.
 *  \param event     The GdkEventMotion event itself.
 *  \param user_data The video canvas data structure associated with
 *                   this machine window.
 *  \return TRUE if no further event processing is necessary.
 *
 *  \todo Information involving mouse-warping is not cached with the
 *        canvas yet, and should be for cleaner C128 support.
 *
 *  \todo Pointer warping does not work on Wayland. GTK3 and its GDK
 *        substrate simply do not provide an implementation for
 *        gdk_device_warp(), and Wayland's window model doesn't really
 *        support pointer warping the way GDK envisions. Wayland's
 *        window model envisions using pointer constraints to confine
 *        the mouse pointer within a target window, and then using
 *        relative mouse motion events to capture additional attempts
 *        at motion outside of it. SDL2 implements this and it may
 *        provide a useful starting point for this alternative
 *        implementation.
 *
 * \sa event_box_mouse_button_cb Further light pen and mouse button
 *     handling.
 * \sa event_box_scroll_cb Further mouse button handling.
 * \sa event_box_stillness_tick_cb More of the hide-idle-mouse-pointer
 *     logic.
 * \sa event_box_cross_cb More of the hide-idle-mouse-pointer logic.
 */
static gboolean event_box_motion_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    video_canvas_t *canvas = (video_canvas_t *)user_data;

    canvas->still_frames = 0;

    if (event->type == GDK_MOTION_NOTIFY) {
        GdkEventMotion *motion = (GdkEventMotion *)event;
        double render_w = canvas->geometry->screen_size.width;
        double render_h = canvas->geometry->last_displayed_line - canvas->geometry->first_displayed_line + 1;
        int pen_x = (motion->x - canvas->screen_origin_x) * render_w / canvas->screen_display_w;
        int pen_y = (motion->y - canvas->screen_origin_y) * render_h / canvas->screen_display_h;
        if (pen_x < 0 || pen_y < 0 || pen_x >= render_w || pen_y >= render_h) {
            /* Mouse pointer is offscreen, so the light pen is disabled. */
            canvas->pen_x = -1;
            canvas->pen_y = -1;
            canvas->pen_buttons = 0;
        } else {
            canvas->pen_x = pen_x;
            canvas->pen_y = pen_y;
        }
        if (warping) {
            warping = 0;
        } else {
            if (last_mouse_x > 0 && last_mouse_y > 0) {
                mouse_move((pen_x-last_mouse_x) * canvas->videoconfig->scalex,
                           (pen_y-last_mouse_y) * canvas->videoconfig->scaley);
            }
            if (_mouse_enabled) {
                GdkWindow *window = gtk_widget_get_window(gtk_widget_get_toplevel(widget));
                GdkScreen *screen = gdk_window_get_screen(window);
                int window_w = gdk_window_get_width(window);
                int window_h = gdk_window_get_height(window);
                gdk_device_warp(motion->device, screen,
                                (window_w / 2) + motion->x_root - motion->x,
                                (window_h / 2) + motion->y_root - motion->y);
                warping = 1;
            }
        }
        last_mouse_x = pen_x;
        last_mouse_y = pen_y;
    }
    return FALSE;
}

/** \brief Callback for handling mouse button events over the emulated
 *         screen.
 *
 *  This forwards any button press or release events on to the light
 *  pen and mouse subsystems.
 *
 *  \param widget    The widget that sent the event.
 *  \param event     The GdkEventButton event itself.
 *  \param user_data The video canvas data structure associated with
 *                   this machine window.
 *  \return TRUE if no further event processing is necessary.
 *
 *  \sa event_box_mouse_motion_cb Further handling of light pen and
 *      mouse events.
 *  \sa event_box_scroll_cb Further handling of mouse button events.
 */
static gboolean event_box_mouse_button_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    video_canvas_t *canvas = (video_canvas_t *)user_data;

    if (event->type == GDK_BUTTON_PRESS) {
        int button = ((GdkEventButton *)event)->button;
        if (button == 1) {
            /* Left mouse button */
            canvas->pen_buttons |= LP_HOST_BUTTON_1;
        } else if (button == 3) {
            /* Right mouse button */
            canvas->pen_buttons |= LP_HOST_BUTTON_2;
        }
        mouse_button(button-1, 1);
    } else if (event->type == GDK_BUTTON_RELEASE) {
        int button = ((GdkEventButton *)event)->button;
        if (button == 1) {
            /* Left mouse button */
            canvas->pen_buttons &= ~LP_HOST_BUTTON_1;
        } else if (button == 3) {
            /* Right mouse button */
            canvas->pen_buttons &= ~LP_HOST_BUTTON_2;
        }        
        mouse_button(button-1, 0);
    }
    /* Ignore all other mouse button events, though we'll be sent
     * things like double- and triple-click. */
    return FALSE;
}

/** \brief Callback for handling mouse scroll wheel events over the
 *         emulated screen.
 *
 *  GTK generates these by translating button presses on buttons 4 and
 *  5 into scroll events; we convert them back and forward them on to
 *  the mouse subsystem.
 *
 *  "Smooth scroll" events are also processed, interpreted as "up
 *  scroll" or "down scroll" based on the vertical component of the
 *  smooth-scroll event.
 *
 *  \param widget    The widget that sent the event.
 *  \param event     The GdkEventScroll event itself.
 *  \param user_data The video canvas data structure associated with
 *                   this machine window.
 *  \return TRUE if no further event processing is necessary.
 *
 *  \sa event_box_scroll_cb Further handling of mouse button events.
 */
static gboolean event_box_scroll_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    GdkScrollDirection dir = ((GdkEventScroll *)event)->direction;
    gdouble smooth_x = 0.0, smooth_y = 0.0;
    switch (dir) {
    case GDK_SCROLL_UP:
        mouse_button(3, 1);
        break;
    case GDK_SCROLL_DOWN:
        mouse_button(4, 1);
        break;
    case GDK_SCROLL_SMOOTH:
        /* Isolate the Y component of a smooth scroll */
        if (gdk_event_get_scroll_deltas(event, &smooth_x, &smooth_y)) {
            if (smooth_y < 0) {
                mouse_button(3, 1);
            } else if (smooth_y > 0) {
                mouse_button(4, 1);
            }
        }
        break;
    default:
        /* Ignore left and right scroll */
        break;
    }
    return FALSE;
}    

/** \brief Create a reusable cursor that may be used as part of this
 *         widget.
 *
 *  GDK cursors are tied to specific displays, so they need to be
 *  created for each machine window individually.
 *
 *  \param widget The widget that will be using this cursor.
 *  \param name   The name of the cursor to create.
 *  \return A new, non-floating, GdkCursor reference, or NULL on
 *          failure.
 *
 *  \note Users coming to this code from the more X11-centric GTK2
 *        will notice that the array of guaranteed-available cursors
 *        is much smaller. Please continue to only use the cursors
 *        listed in the documentation for gdk_cursor_new_from_name()
 *        here.
 */
static GdkCursor *make_cursor(GtkWidget *widget, const char *name)
{
    GdkDisplay *display = gtk_widget_get_display(widget);
    GdkCursor *result = NULL;

    if (display) {
        result = gdk_cursor_new_from_name(display, name);
        if (result != NULL) {
            g_object_ref_sink(G_OBJECT(result));
        }
    }
    return result;
}

/** \brief Frame-advance callback for the hide-mouse-when-idle logic.
 *
 *  This function is called as the "tick callback" whenever the mouse
 *  is hovering over the machine's screen. Its job is primarily to
 *  manage the mouse cursor:
 *
 *  - If the light pen is active, the cursor is always visible and is
 *    shaped like a crosshair.
 *  - If the mouse is grabbed, the cursor is never visible.
 *  - Otherwise, the cursor is visible as a normal mouse pointer as
 *    long as it's been 60 or fewer ticks since the last time the
 *    mouse moved.
 *
 *  \param widget    The widget that sent the event.
 *  \param clock     The GdkFrameClock that's managing our ticks.
 *  \param user_data The video canvas data structure associated with
 *                   this machine window.
 *  \return TRUE if no further event processing is necessary.
 *
 *  \sa event_box_cross_cb  Manages the lifecycle of this tick
 *      callback.
 *  \sa event_box_motion_cb Manages the "ticks since the last time the
 *      mouse moved" counter.
 */
static gboolean event_box_stillness_tick_cb(GtkWidget *widget, GdkFrameClock *clock, gpointer user_data)
{
    video_canvas_t *canvas = (video_canvas_t *)user_data;

    ++canvas->still_frames;
    if (_mouse_enabled || (!lightpen_enabled && canvas->still_frames > 60)) {
        if (canvas->blank_ptr == NULL) {
            canvas->blank_ptr = make_cursor(widget, "none");
        }
        if (canvas->blank_ptr != NULL) {
            GdkWindow *window = gtk_widget_get_window(widget);

            if (window) {
                gdk_window_set_cursor(window, canvas->blank_ptr);
            }
        }
    } else {
        GdkWindow *window = gtk_widget_get_window(widget);
        if (canvas->pen_ptr == NULL) {
            canvas->pen_ptr = make_cursor(widget, "crosshair");
        }
        if (window) {
            if (lightpen_enabled && canvas->pen_ptr) {
                gdk_window_set_cursor(window, canvas->pen_ptr);
            } else {
                gdk_window_set_cursor(window, NULL);
            }
        }
    }
    return G_SOURCE_CONTINUE;
}

/** \brief Callback for managing the hide-pointer-on-idle timings.
 *
 *  This callback fires whenever the machine window's canvas gains or
 *  loses focus over the mouse pointer. It manages the logic that
 *  hides the mouse pointer after inactivity. Entering the window will
 *  start the timer, and leaving it will stop it.
 *
 *  Leaving the window entirely will also be interpreted as removing
 *  the light pen from the screen.
 *
 *  \param widget    The widget that sent the event.
 *  \param event     The GdkEventCrossing event itself.
 *  \param user_data The video canvas data structure associated with
 *                   this machine window.
 *  \return TRUE if no further event processing is necessary.
 *
 *  \sa event_box_stillness_tick_cb The timer managed by this function.
 */
static gboolean event_box_cross_cb(GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
    video_canvas_t *canvas = (video_canvas_t *)user_data;
    GdkEventCrossing *crossing = (GdkEventCrossing *)event;

    if (!canvas || !event ||
        (event->type != GDK_ENTER_NOTIFY && event->type != GDK_LEAVE_NOTIFY) ||
        crossing->mode != GDK_CROSSING_NORMAL) {
        /* Spurious event. Most likely, this is an event fired because
         * clicking the canvas altered grab status. */
        return FALSE;
    }
    
    if (crossing->type == GDK_ENTER_NOTIFY) {
        canvas->still_frames = 0;
        if (canvas->still_frame_callback_id == 0) {
            canvas->still_frame_callback_id = gtk_widget_add_tick_callback(canvas->drawing_area,
                                                                           event_box_stillness_tick_cb,
                                                                           canvas, NULL);
        }
    } else {
        GdkWindow *window = gtk_widget_get_window(canvas->drawing_area);

        if (window) {
            gdk_window_set_cursor(window, NULL);
        }
        if (canvas->still_frame_callback_id != 0) {
            gtk_widget_remove_tick_callback(canvas->drawing_area, canvas->still_frame_callback_id);
            canvas->still_frame_callback_id = 0;
        }
        canvas->pen_x = -1;
        canvas->pen_y = -1;
        canvas->pen_buttons = 0;
    }
    return FALSE;
}

/** \brief Create a new machine window.
 *
 *  A machine window is a GtkGrid that has a menu bar on top, a status
 *  bar on the bottom, and a renderer-backend specific drawing area in
 *  the middle. The canvas argument has its relevant fields populated
 *  by this process.
 *
 *  \param canvas The video canvas to populate.
 *
 *  \todo At the moment, the renderer backend is selected at compile
 *        time and cannot be changed. It would be nice to be able to
 *        fall back to simpler backends if more specialized ones
 *        fail. This is difficult at present because we cannot know if
 *        OpenGL is available until long after the window is created
 *        and realized.
 */
static void machine_window_create(video_canvas_t *canvas)
{
    GtkWidget *new_drawing_area, *new_event_box;
    GtkWidget *menu_bar;

    /* TODO: Make the rendering process transparent enough that this can be selected and altered as-needed */
#ifdef HAVE_GTK3_OPENGL
    canvas->renderer_backend = &vice_opengl_backend;
#else
    canvas->renderer_backend = &vice_cairo_backend;
#endif

    new_drawing_area = canvas->renderer_backend->create_widget(canvas);
    canvas->drawing_area = new_drawing_area;

    new_event_box = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(new_event_box), new_drawing_area);

    gtk_widget_add_events(new_event_box, GDK_POINTER_MOTION_MASK);
    gtk_widget_add_events(new_event_box, GDK_BUTTON_PRESS_MASK);
    gtk_widget_add_events(new_event_box, GDK_BUTTON_RELEASE_MASK);
    gtk_widget_add_events(new_event_box, GDK_SCROLL_MASK);
    g_signal_connect(new_event_box, "enter-notify-event", G_CALLBACK(event_box_cross_cb), canvas);
    g_signal_connect(new_event_box, "leave-notify-event", G_CALLBACK(event_box_cross_cb), canvas);
    g_signal_connect(new_event_box, "motion-notify-event", G_CALLBACK(event_box_motion_cb), canvas);
    g_signal_connect(new_event_box, "button-press-event", G_CALLBACK(event_box_mouse_button_cb), canvas);
    g_signal_connect(new_event_box, "button-release-event", G_CALLBACK(event_box_mouse_button_cb), canvas);
    g_signal_connect(new_event_box, "scroll-event", G_CALLBACK(event_box_scroll_cb), canvas);

    /* I'm pretty sure when running x128 we get two menu instances, so this
     * should go somewhere else: call ui_menu_bar_create() once and attach the
     * result menu to each GtkWindow instance
     */
    menu_bar = ui_machine_menu_bar_create();

    gtk_container_add(GTK_CONTAINER(canvas->grid), menu_bar);
    gtk_container_add(GTK_CONTAINER(canvas->grid), new_event_box);

    return;
}

void ui_machine_window_init(void)
{
    ui_set_create_window_func(machine_window_create);
    return;
}

#endif
