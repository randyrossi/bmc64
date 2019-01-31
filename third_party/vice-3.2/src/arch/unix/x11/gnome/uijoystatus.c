/*
 * uijoystatus.c - GTK only, UI controls for Joystick emulation
 *
 * Written by
 *  groepaz <groepaz@gmx.net>
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

/* #define DEBUG_X11UI */

#include "vice.h"

#include <string.h>

#include "joystick.h"
#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "ui.h"
#include "uiarch.h"
#include "uicontents.h"
#include "uimenu.h"
#include "userport_joystick.h"
#include "util.h"

#include "uijoystatus.h"


#ifdef DEBUG_X11UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

/******************************************************************************/

#define JOY_AREA_WIDTH 16
#define JOY_AREA_HEIGHT 16

#define JOY_LED_WIDTH 4
#define JOY_LED_HEIGHT 4

static int joyledx[5] = {
    (JOY_LED_WIDTH + 1) * 1,    /* up */
    (JOY_LED_WIDTH + 1) * 1,    /* down */
    (JOY_LED_WIDTH + 1) * 0,    /* left */
    (JOY_LED_WIDTH + 1) * 2,    /* right */
    (JOY_LED_WIDTH + 1) * 1,
};

static int joyledy[5] = {
    (JOY_LED_HEIGHT + 1) * 0,
    (JOY_LED_HEIGHT + 1) * 2,
    (JOY_LED_HEIGHT + 1) * 1,
    (JOY_LED_HEIGHT + 1) * 1,
    (JOY_LED_HEIGHT + 1) * 1,
};

/* FIXME: open end, add popup menu with joystick settings */
static gboolean joystick_popup_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
#ifdef DEBUG_X11UI
    int d = vice_ptr_to_int(data);
    DBG(("joystick_popup_cb (%d)", d));
#endif
    if (event->type == GDK_BUTTON_PRESS) {
        GdkEventButton *bevent = (GdkEventButton*) event;
        if (bevent->button == 1) {
            /*ui_menu_update_all_GTK();
             gtk_menu_popup(GTK_MENU(&joystick_settings_c64_menu[0]), NULL, NULL, NULL, NULL, bevent->button, bevent->time);
             */
        } else if (bevent->button == 3) {
        }
    }
    return 0;
}

#if !defined(HAVE_CAIRO)

static void set_joy(GdkColor *colors[5], int i, int joystick_number)
{
    GdkGC *app_gc = get_toplevel();
    joystick_status_widget *ds = &app_shells[i].joystick_status[joystick_number];
    if ((ds) && (ds->led)) {
        GdkColor colbg;
        gdk_color_parse ("#c0c0c0", &colbg);
        gdk_gc_set_rgb_fg_color(app_gc, &colbg);
        gdk_draw_rectangle(ds->led_pixmap, app_gc, TRUE, 0, 0, JOY_AREA_WIDTH + 2, JOY_AREA_HEIGHT + 2);
        gtk_widget_queue_draw(ds->led);
        for (i = 0; i < 5; i++) {
            int tx = joyledx[i] + 1;
            int ty = joyledy[i] + 1;
            gdk_gc_set_rgb_fg_color(app_gc, colors[i]);
            gdk_draw_rectangle(ds->led_pixmap, app_gc, TRUE, tx, ty, JOY_LED_WIDTH, JOY_LED_HEIGHT);
            gtk_widget_queue_draw(ds->led);
        }
    }
}

#else

static void set_joy(GdkColor *colors[5], int i, int joystick_number)
{
    joystick_status_widget *ts = &app_shells[i].joystick_status[joystick_number];
    if ((ts) && (ts->led)) {
        cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(ts->led));
        for (i = 0; i < 5; i++) {
            int tx = joyledx[i];
            int ty = joyledy[i] + JOY_LED_HEIGHT;
            cairo_translate(cr, 0, 0);
            gdk_cairo_set_source_color(cr, colors[i]);
            cairo_rectangle (cr, tx, ty, JOY_LED_WIDTH, JOY_LED_HEIGHT);
            cairo_fill (cr);
        }
        cairo_destroy(cr);
    }
}

static gboolean joys_draw(GtkWidget *w, GdkEvent *event, gpointer data)
{
    int i;
    int joystick_number = vice_ptr_to_int(data);
    int num_app_shells = get_num_shells();

    for (i = 0; i < num_app_shells; i++) {
        joystick_status_widget *ds = &app_shells[i].joystick_status[joystick_number];
        set_joy(ds->colors, i, joystick_number);
    }
    return 0;
}
#endif

void ui_display_joystick_status_widget(int joystick_number, int status)
{
    int i, n;
    int upjoy = 0, sidjoy = 0, typejoy = USERPORT_JOYSTICK_HUMMER;
    int num_app_shells = get_num_shells();

    DBG(("ui_display_joystick_status_widget (%d, %02x)", joystick_number, status));

    if (machine_class != VICE_MACHINE_CBM5x0) {
        resources_get_int("UserportJoy", &upjoy);
        if (machine_class != VICE_MACHINE_C64DTV) {
            resources_get_int("UserportJoyType", &typejoy);
        }
    }
    if (machine_class == VICE_MACHINE_PLUS4) {
        resources_get_int("SIDCartJoy", &sidjoy);
    }

    if (machine_class != VICE_MACHINE_PLUS4 && machine_class != VICE_MACHINE_C64DTV && joystick_number > 1) {
        switch (typejoy) {
            case USERPORT_JOYSTICK_HUMMER:
            case USERPORT_JOYSTICK_OEM:
                typejoy = 0;
                break;
            default:
                typejoy = 1;
        }
    }

    for (i = 0; i < num_app_shells; i++) {
        joystick_status_widget *ds = &app_shells[i].joystick_status[joystick_number];
        if (ds->led) {
            for (n = 0; n < 4; n++) {
                ds->colors[n] = (status & (1 << n)) ? &drive_led_on_green_pixel : &drive_led_off_pixel;
            }
            ds->colors[4] = (status & (1 << 4)) ? &drive_led_on_red_pixel : &drive_led_off_pixel;
            set_joy(ds->colors, i, joystick_number);

            if (machine_class == VICE_MACHINE_C64DTV) {
                if ((joystick_number == 2) && (upjoy == 0)) {
                    gtk_widget_hide(ds->event_box);
                } else {
                    gtk_widget_show_all(ds->event_box);
                }
            } else {
                if ((joystick_number > 1) && (upjoy == 0)) {
                    gtk_widget_hide(ds->event_box);
                }
                if ((joystick_number == 2) && (upjoy == 1)) {
                    gtk_widget_show_all(ds->event_box);
                }
                if ((joystick_number == 3) && (upjoy == 1) && (typejoy == 0)) {
                    gtk_widget_hide(ds->event_box);
                }
                if ((joystick_number == 3) && (upjoy == 1) && (typejoy == 1)) {
                    gtk_widget_show_all(ds->event_box);
                }
                if ((joystick_number == 4) && (sidjoy == 0)) {
                    gtk_widget_hide(ds->event_box);
                }
                if ((joystick_number == 4) && (sidjoy == 1)) {
                    gtk_widget_show_all(ds->event_box);
                }
            }
        }
    }
}

GtkWidget *build_joystick_status_widget(app_shell_type *as, GdkWindow *window)
{
    GtkWidget *joystick_box, *frame;
    int i = 0;
    char *empty = util_concat("<", _("empty"), ">", NULL);

    joystick_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    for (i = 0; i < JOYSTICK_NUM; i++)
    {
        /* skip port 1 for machines that have no internal joystick ports */
        if (((machine_class == VICE_MACHINE_CBM6x0) ||
             (machine_class == VICE_MACHINE_PET)
            ) && (i == 0)) {
            as->joystick_status[i].led = NULL;
            continue;
        }

        /* skip port 2 for machines that only have one joystick port or no internal joystick ports */
        if (((machine_class == VICE_MACHINE_VIC20) ||
             (machine_class == VICE_MACHINE_CBM6x0) ||
             (machine_class == VICE_MACHINE_PET)
            ) && (i == 1)) {
            as->joystick_status[i].led = NULL;
            continue;
        }

        /* skip port 3 for machines with no user port and no other joystick adapter type */
        if (((machine_class == VICE_MACHINE_CBM5x0)
            ) && (i == 2)) {
            as->joystick_status[i].led = NULL;
            continue;
        }

        /* skip port 4 for machines with no user port, or not enough userport lines for 2 port userport adapters */
        if (((machine_class == VICE_MACHINE_CBM5x0) ||
             (machine_class == VICE_MACHINE_C64DTV)
            ) && (i == 3)) {
            as->joystick_status[i].led = NULL;
            continue;
        }

        /* skip port 5 for machines with no 5th control port */
        if (((machine_class != VICE_MACHINE_PLUS4)) && (i == 4)) {
            as->joystick_status[i].led = NULL;
            continue;
        }

        as->joystick_status[i].event_box = gtk_event_box_new();

        frame = gtk_frame_new(NULL);
        gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);

        as->joystick_status[i].box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

        gtk_container_add(GTK_CONTAINER(frame), as->joystick_status[i].box);
        gtk_widget_show(as->joystick_status[i].box);

        gtk_container_add(GTK_CONTAINER(as->joystick_status[i].event_box), frame);
        gtk_widget_show(frame);
#if 0   /* FIXME */
        gtk_widget_set_tooltip_text(GTK_WIDGET(gtk_widget_get_parent(gtk_widget_get_parent(as->joystick_status[i].box))), empty);
#endif
        /* Single Led */
#if !defined(HAVE_CAIRO)
        as->joystick_status[i].led_pixmap = gdk_pixmap_new(window, JOY_AREA_WIDTH, JOY_AREA_HEIGHT, -1);
        as->joystick_status[i].led = gtk_image_new_from_pixmap(as->joystick_status[i].led_pixmap, NULL);
#else
        as->joystick_status[i].led = gtk_drawing_area_new();
#endif
        gtk_widget_set_size_request(as->joystick_status[i].led, JOY_AREA_WIDTH, JOY_AREA_HEIGHT);
        gtk_box_pack_start(GTK_BOX(as->joystick_status[i].box), (GtkWidget *)as->joystick_status[i].led, FALSE, FALSE, 4);
        gtk_widget_show(as->joystick_status[i].led);

        /* Pack everything together */
        gtk_box_pack_start(GTK_BOX(joystick_box), as->joystick_status[i].event_box, FALSE, FALSE, 0);

        gtk_widget_set_events(as->joystick_status[i].event_box, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK);
        g_signal_connect(G_OBJECT(as->joystick_status[i].event_box), "button-press-event", G_CALLBACK(joystick_popup_cb), (gpointer)(int_to_void_ptr(i)));
        gtk_widget_show(as->joystick_status[i].event_box);

#if defined(HAVE_CAIRO)
#if GTK_CHECK_VERSION(3, 0, 0)
        g_signal_connect(G_OBJECT(as->joystick_status[i].led), "draw", G_CALLBACK(joys_draw), int_to_void_ptr(i));
#else
        g_signal_connect(G_OBJECT(as->joystick_status[i].led), "expose-event", G_CALLBACK(joys_draw), int_to_void_ptr(i));
#endif
#endif
    }
    lib_free(empty);
    return joystick_box;
}

void ui_init_joystick_status_widget(void)
{
    int i, n, c;
    int num_app_shells = get_num_shells();

    for (i = 0; i < num_app_shells; i++) {
        for (n = 0; n < JOYSTICK_NUM; n++) {
            joystick_status_widget *ds = &app_shells[i].joystick_status[n];
            for (c = 0; c < 5; c++) {
                ds->colors[c] = &drive_led_off_pixel;
            }
        }
    }
}
