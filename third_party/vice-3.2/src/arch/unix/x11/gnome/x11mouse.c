/*
 * x11mouse.c - GTK only, Mouse handling
 *
 * Written by
 *  Ettore Perazzoli
 *  Oliver Schaertel
 *  pottendo <pottendo@gmx.net>
 *  groepaz <groepaz@gmx.net>
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

/* #define DEBUG_X11UI */
/* #define DEBUGMOUSECURSOR */  /* dont use a blank mouse cursor */
/* #define DEBUGNOMOUSEGRAB */  /* dont grab mouse */

#include "vice.h"

#include "datasette.h"
#include "fullscreenarch.h"
#include "lib.h"
#include "machine.h"
#include "mouse.h"
#include "mousedrv.h"
#include "ui.h"
#include "uiarch.h"
#include "uicontents.h"
#include "uimenu.h"
#include "util.h"
#include "video.h"
#include "resources.h"
#include "videoarch.h"
#include "lightpen.h"
#include "lightpendrv.h"

#include "x11mouse.h"


#ifdef DEBUG_X11UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif
static GdkCursor *blankCursor;
static gint mouse_dx = 0, mouse_dy = 0;
static gint mouse_lasteventx = 0, mouse_lasteventy = 0;
static gint mouse_warped = 0;
static gint mouse_warpx = 0, mouse_warpy = 0;
#define MOUSE_WRAP_MARGIN  50

static int mouse_grabbed = 0;

/******************************************************************************/

/*
    grab pointer, set mouse pointer shape

    called by: ui_check_mouse_cursor, ui_restore_mouse, x11ui_fullscreen

    TODO: also route lightpen stuff through this function
*/
static void mouse_cursor_grab(int grab, GdkCursor *cursor)
{
#ifdef DEBUGNOMOUSEGRAB
    DBG(("mouse_cursor_grab disabled (%d)", grab));
#else
    GtkWidget *widget;
    GdkWindow *window;

    DBG(("mouse_cursor_grab (%d, was %d)", grab, mouse_grabbed));

    if (mouse_grabbed) {
        gdk_pointer_ungrab(GDK_CURRENT_TIME);
        mouse_grabbed = 0;
    }

    if (grab) {
        /*ui_dispatch_events();
        gdk_flush();*/

        widget = get_active_toplevel();
        window = widget ? gtk_widget_get_window(widget) : NULL;

        if ((widget == NULL) || (window == NULL)) {
            log_error(ui_log, "mouse_cursor_grab: bad params");
            return;
        }
#ifdef DEBUGMOUSECURSOR
        if (cursor == blankCursor) {
            DBG(("mouse_cursor_grab blankCursor disabled"));
            cursor = NULL;
        }
#endif
        gdk_pointer_grab(window, 1, 
                         GDK_POINTER_MOTION_MASK | GDK_BUTTON_PRESS_MASK | GDK_SCROLL_MASK | GDK_BUTTON_RELEASE_MASK, 
                         window, cursor, GDK_CURRENT_TIME);
        mouse_grabbed = 1;
    }
#endif
}

void ui_check_mouse_cursor(void)
{
#ifdef HAVE_FULLSCREEN
    if (machine_class != VICE_MACHINE_VSID) {
        video_canvas_t *canvas;
        if ((canvas = get_active_canvas()) == NULL) {
            DBG(("ui_check_mouse_cursor canvas == NULL"));
            mouse_cursor_grab(0, NULL);
            return;
        }
        if (canvas->fullscreenconfig == NULL) {
            log_error(ui_log, "ui_check_mouse_cursor canvas->fullscreenconfig == NULL");
            mouse_cursor_grab(0, NULL);
            return;
        }

        if (canvas->fullscreenconfig->enable) {
            if (_mouse_enabled) {
                mouse_cursor_grab(1, blankCursor);
            } else {
                /* FIXME: this case seems odd */
                mouse_cursor_grab(1, NULL);
            }
            return;
        }
    }
#endif
    if (_mouse_enabled) {
        mouse_cursor_grab(1, blankCursor);
    } else {
        mouse_cursor_grab(0, NULL);
    }
}

/*
    ungrab mouse and restore mouse pointer shape

    called by uicommands.c:activate_monitor, ui_jam_dialog, ui_popup. ui_exit
*/
void ui_restore_mouse(void)
{
    mouse_cursor_grab(0, NULL);
}

void mouse_init_cursor(void)
{
#if !GTK_CHECK_VERSION(2, 2, 0)
    static char cursor[] = { 0x00 };
    GdkColor fg = { 0, 0, 0, 0 };
    GdkColor bg = { 0, 0, 0, 0 };
    GdkBitmap *source = gdk_bitmap_create_from_data (NULL, cursor, 1, 1);
    GdkBitmap *mask = gdk_bitmap_create_from_data (NULL, cursor, 1, 1);

    blankCursor = gdk_cursor_new_from_pixmap (source, mask, &fg, &bg, 1, 1); 

    g_object_unref (source);
    g_object_unref (mask);
#else
    /* GDK_BLANK_CURSOR exists since 2.16 */
    /* FIXME: to support multiple screens, we must use gdk_cursor_new_for_display */
    blankCursor = gdk_cursor_new(GDK_BLANK_CURSOR);
#endif
}

static void mouse_handler(GtkWidget *w, GdkEvent *event, gpointer data)
{
    video_canvas_t *canvas = (video_canvas_t *)data;

    if (event->type == GDK_BUTTON_PRESS) {
        GdkEventButton *bevent = (GdkEventButton*)event;
        if (_mouse_enabled || lightpen_enabled) {
            mouse_button(bevent->button-1, TRUE);
            gtk_lightpen_setbutton(bevent->button, TRUE);
        }
    } else if (event->type == GDK_SCROLL) {
        GdkEventScroll *bevent = (GdkEventScroll*)event;
        if (_mouse_enabled) {
            if (bevent->direction == GDK_SCROLL_UP) {
                mouse_button(3, TRUE);
            } else if (bevent->direction == GDK_SCROLL_DOWN) {
                mouse_button(4, TRUE);
            }
        }
    } else if (event->type == GDK_BUTTON_RELEASE && (_mouse_enabled || lightpen_enabled)) {
        GdkEventButton *bevent = (GdkEventButton*)event;
        mouse_button(bevent->button-1, FALSE);
        gtk_lightpen_setbutton(bevent->button, FALSE);
   } else if (event->type == GDK_MOTION_NOTIFY) {
        GdkEventMotion *mevent = (GdkEventMotion*)event;
        if (_mouse_enabled) {
            /* handle pointer motion events for mouse emulation */
            gint x=0, y=0, w=0, h=0, warp=0;
            gint xoff=0, yoff=0;
            gint ptrx, ptry;
            GdkDisplay *display = NULL;
            GdkScreen *screen = NULL;

            /* get default display and screen */
            display = gdk_display_get_default ();
            screen = gdk_display_get_default_screen (display);

            /* get cursor position */
            gdk_display_get_pointer (display, NULL, &x, &y, NULL);

            ptrx = (int)mevent->x;
            ptry = (int)mevent->y;
            xoff = x - ptrx;
            yoff = y - ptry;

            /* w = canvas->draw_buffer->canvas_physical_width; */
            w = gtk_widget_get_allocated_width(canvas->emuwindow);
            h = canvas->draw_buffer->canvas_physical_height;

            /* DBG(("ptrx:%d ptry:%d x:%d y:%d w:%d h:%d", ptrx, ptry, x, y, w, h)); */

            if (mouse_warpx == 1) {
                /* from left to right */
                if ((ptrx > mouse_lasteventx) && (ptrx >= (w - (MOUSE_WRAP_MARGIN * 2))) && (ptrx <= (w - MOUSE_WRAP_MARGIN))) {
                    mouse_warpx = 0;
                    mouse_lasteventx = ptrx;
                }
            } else if (mouse_warpx == 2) {
                /* from right to left */
                if ((ptrx < mouse_lasteventx) && (ptrx <= (MOUSE_WRAP_MARGIN * 2)) && (ptrx >= MOUSE_WRAP_MARGIN)) {
                    mouse_warpx = 0;
                    mouse_lasteventx = ptrx;
                }
            }

            if (mouse_warpy == 1) {
                /* from top to bottom */
                if ((ptry > mouse_lasteventy) && (ptry >= (h - (MOUSE_WRAP_MARGIN * 2))) && (ptry <= (h - MOUSE_WRAP_MARGIN))) {
                    mouse_warpy = 0;
                    mouse_lasteventy = ptry;
                }
            } else if (mouse_warpy == 2) {
                /* from bottom to top */
                if ((ptry < mouse_lasteventy) && (ptry <= (MOUSE_WRAP_MARGIN * 2)) && (ptry >= MOUSE_WRAP_MARGIN)) {
                    mouse_warpy = 0;
                    mouse_lasteventy = ptry;
                }
            }

            if (mouse_warped || mouse_warpx || mouse_warpy) {
                /* ignore this event, its the result of us having moved the pointer */
                /* DBG(("warped!:%d/%d/%d ptrx:%d ptry:%d lastx:%d lasty:%d", mouse_warped, mouse_warpx, mouse_warpy, ptrx, ptry, mouse_lasteventx, mouse_lasteventy)); */
                if (mouse_warped) {
                    --mouse_warped;
                }
            } else {

                if (ptrx < MOUSE_WRAP_MARGIN) {
                    /* from left to right */
                    mouse_lasteventx = ptrx;
                    ptrx = w - (MOUSE_WRAP_MARGIN + 10);
                    mouse_warpx = 1;
                    warp = 1;
                }
                else if (ptrx > (w - MOUSE_WRAP_MARGIN)) {
                    /* from right to left */
                    mouse_lasteventx = ptrx;
                    ptrx = (MOUSE_WRAP_MARGIN + 10);
                    mouse_warpx = 2;
                    warp = 1;
                }

                if (ptry < (MOUSE_WRAP_MARGIN)) {
                    /* from top to bottom */
                    mouse_lasteventy = ptry;
                    ptry = (h - (MOUSE_WRAP_MARGIN + 10));
                    mouse_warpy = 1;
                    warp = 1;
                } else if (ptry > (h - MOUSE_WRAP_MARGIN)) {
                    /* from bottom to top */
                    mouse_lasteventy = ptry;
                    ptry = (MOUSE_WRAP_MARGIN + 10);
                    mouse_warpy = 2;
                    warp = 1;
                }
                /* DBG(("warp:%d ptrx:%d ptry:%d x:%d y:%d w:%d h:%d", warp, ptrx, ptry, x, y, w, h)); */

                if (warp) {
                    /* set new cusor position */
                    ++mouse_warped;
                    /* DBG(("warp to: x:%d y:%d", ptrx, ptry)); */
                    gdk_display_warp_pointer (display, screen, ptrx + xoff, ptry + yoff);
                } else {
                    mouse_dx = (ptrx - mouse_lasteventx) / canvas->videoconfig->scalex;
                    mouse_dy = (ptry - mouse_lasteventy) / canvas->videoconfig->scaley;
                    DBG(("mouse move dx:%8d dy:%8d", mouse_dx, mouse_dy));
                    mouse_move((float)mouse_dx, (float)mouse_dy);
                    mouse_lasteventx = ptrx;
                    mouse_lasteventy = ptry;
                }
            }
        }
#ifdef HAVE_FULLSCREEN
        fullscreen_mouse_moved(canvas, (int)mevent->x, (int)mevent->y, 0);
#endif
   }
}

/*
    connected to the "leave-notify-event" of the pane, generates an extra mouse
    event to make sure the cursor can not escape the wraparound area. needed
    because the area handled by mouse grab includes the menu and statusbar.
*/
static gboolean mouse_handler_wrap(GtkWidget *w, GdkEventCrossing *e, gpointer p)
{
    if (_mouse_enabled) {
        GdkEventMotion mevent;

        DBG(("mouse_handler_wrap"));

        mevent.x = e->x;
        mevent.y = e->y;
        mevent.type = GDK_MOTION_NOTIFY;

        mouse_warped = 0;
        mouse_warpx = 0;
        mouse_warpy = 0;

        mouse_handler(w, (GdkEvent*)&mevent, p);
    }
    return 0;
}
 

/* dummy menus for hotkeys (AccelGroups) are needed - this seems ugly here, but it works
   a better way could be to make the toplevel menu accelgroup active/inactive depending 
   on fullscreen on/off and not showing/hiding the toplevel menu in fullscreen/window mode
   -> then the code doesn't belong here in ..mouse.c!
   
   popup menus when clicking into the windows are unusual for
   GTK apps and they cause more problems than they solve (eg when clicking into
   the window to switch focus, so popup is not done */
void ui_set_left_menu(ui_menu_entry_t *menu)
{
    int i;
    static GtkAccelGroup *accel;
    static GtkWidget *left_menu;
    int num_app_shells = get_num_shells();

    DBG(("ui_set_left_menu"));

    ui_block_shells();

    if (accel) {
        g_object_unref(accel);
    }

    accel = gtk_accel_group_new();
    for (i = 0; i < num_app_shells; i++) {
        gtk_window_add_accel_group (GTK_WINDOW (app_shells[i].shell), accel);
    }

    if (left_menu != NULL) {
        gtk_widget_destroy(left_menu);
    }
    left_menu = gtk_menu_new();
    ui_menu_create(left_menu, accel, "LeftMenu", menu);

    ui_unblock_shells();
}

void ui_set_right_menu(ui_menu_entry_t *menu)
{
    int i;
    static GtkAccelGroup *accel;
    static GtkWidget *right_menu;
    int num_app_shells = get_num_shells();

    DBG(("ui_set_right_menu"));

    ui_block_shells();

    if (accel) {
        g_object_unref(accel);
    }

    accel = gtk_accel_group_new();
    for (i = 0; i < num_app_shells; i++) {
        gtk_window_add_accel_group (GTK_WINDOW (app_shells[i].shell), accel);
    }

    if (right_menu != NULL) {
        gtk_widget_destroy(right_menu);
    }
    right_menu = gtk_menu_new();
    ui_menu_create(right_menu, accel, "RightMenu", menu);

    ui_unblock_shells();    
}

void mouse_connect_handler(GtkWidget *widget, void *data)
{
    g_signal_connect(G_OBJECT(widget), "button-press-event", G_CALLBACK(mouse_handler), data);
    g_signal_connect(G_OBJECT(widget), "button-release-event", G_CALLBACK(mouse_handler), data);
    g_signal_connect(G_OBJECT(widget), "motion-notify-event", G_CALLBACK(mouse_handler), data);
    g_signal_connect(G_OBJECT(widget), "scroll-event", G_CALLBACK(mouse_handler), data);
}

void mouse_connect_wrap_handler(GtkWidget *widget, void *data)
{
    g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(mouse_handler_wrap), data);
}
