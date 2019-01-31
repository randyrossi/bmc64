/*
 * uitapestatus.c - GTK only, UI controls for Datasette emulation
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

#include "vice.h"

#include "datasette.h"
#include "lib.h"
#include "machine.h"
#include "ui.h"
#include "uiarch.h"
#include "uicontents.h"
#include "uimenu.h"
#include "util.h"
#include "video.h"
#include "resources.h"
#include "videoarch.h"

#include "uitapestatus.h"

#include <math.h>

#ifdef DEBUG_X11UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

/******************************************************************************/

#define CTRL_WIDTH 13
#define CTRL_HEIGHT 11

static int tape_motor_status = -1;        /* tape motor on/off */
static int tape_control_status = -1;      /* tape button/operation status */
static int tape_image_status = -1;        /* tape image present */

char *last_attached_tape; /* FIXME: We want this to be static.  */
static GtkWidget *tape_menu;

static gboolean tape_popup_cb(GtkWidget *w, GdkEvent *event, gpointer data)
{
    if (event->type == GDK_BUTTON_PRESS) {
        GdkEventButton *bevent = (GdkEventButton*)event;
        if (bevent->button == 1) {
            ui_menu_update_all_GTK();
            if (tape_menu) {
                gtk_menu_popup(GTK_MENU(tape_menu), NULL, NULL, NULL, NULL, bevent->button, bevent->time);
            }
        } else if (bevent->button == 3) {
            static char *lasttapemenu;
            static GtkWidget *ltapemenu;

            if (last_attached_tape == NULL) {
                if (lasttapemenu) {
                    if (ltapemenu) {
                        gtk_widget_destroy(ltapemenu);
                    }
                    lib_free(lasttapemenu);
                    lasttapemenu = NULL;
                }
                return 0;
            }

            if (ltapemenu) {
                gtk_widget_destroy(ltapemenu);
            }
            lib_free(lasttapemenu);
            lasttapemenu = lib_stralloc(last_attached_tape);
            ltapemenu = rebuild_contents_menu(1, lasttapemenu);

            if (ltapemenu) {
                gtk_menu_popup(GTK_MENU(ltapemenu), NULL, NULL, NULL, NULL, bevent->button, bevent->time);
            }
        }
    }
    return 0;
}

void ui_set_tape_menu(ui_menu_entry_t *menu)
{
    if (tape_menu) {
        gtk_widget_destroy(tape_menu);
    }
    tape_menu = gtk_menu_new();
    ui_menu_create(tape_menu, NULL, "TapeMenu", menu);
}

 
void ui_display_tape_counter(int counter)
{
    static char label[1024];
    int i;
    int num_app_shells = get_num_shells();

    /* Translators: the string is fixed with 1024 byte! Don't exceed this
       limit !*/
    sprintf(label, _("Tape %03d"), counter % 1000);
    for (i = 0; i < num_app_shells; i++) {
        gtk_label_set_text(GTK_LABEL(app_shells[i].tape_status.label), label);
    }
}

void ui_display_tape_current_image(const char *image)
{
    char *name;
    int i;
    int num_app_shells = get_num_shells();

    lib_free(last_attached_tape);
    last_attached_tape = lib_stralloc(image);
    util_fname_split(image, NULL, &name);

    for (i = 0; i < num_app_shells; i++) {
        gtk_widget_set_tooltip_text(GTK_WIDGET(gtk_widget_get_parent(gtk_widget_get_parent(app_shells[i].tape_status.box))), name);
    }
    lib_free(name);
}

#if defined(HAVE_CAIRO)
static gboolean tape_draw(GtkWidget *w, GdkEvent *event, gpointer data)
{
    /* FIXME: only draw one widget at a time */
    ui_display_tape_control_status(-1);
    return 0;
}
#endif

void build_tape_status_widget(app_shell_type *as, GdkWindow *window)
{
    GtkWidget *frame;

    as->tape_status.event_box = gtk_event_box_new();

    frame = gtk_frame_new(NULL);
    gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_IN);

    as->tape_status.box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(frame), as->tape_status.box);
    gtk_widget_show(as->tape_status.box);

    gtk_container_add(GTK_CONTAINER(as->tape_status.event_box), frame);
    gtk_widget_show(frame);

    gtk_widget_set_tooltip_text(GTK_WIDGET(gtk_widget_get_parent(gtk_widget_get_parent(as->tape_status.box))), "");

    /* Tape Label */
    as->tape_status.label = gtk_label_new(_("Tape 000"));
    gtk_container_add(GTK_CONTAINER(as->tape_status.box), as->tape_status.label);
    /* gtk_misc_set_alignment(GTK_MISC (as->tape_status.label), 0, -1); */
    gtk_widget_show(as->tape_status.label);

    /* Tape control */
#if !defined(HAVE_CAIRO)
    as->tape_status.control_pixmap = gdk_pixmap_new(window, CTRL_WIDTH, CTRL_HEIGHT, -1);
    as->tape_status.control = gtk_image_new_from_pixmap(as->tape_status.control_pixmap, NULL);
#else
    as->tape_status.control = gtk_drawing_area_new();
#endif
    gtk_widget_set_size_request(as->tape_status.control, CTRL_WIDTH, CTRL_HEIGHT);
    gtk_box_pack_start(GTK_BOX(as->tape_status.box), as->tape_status.control, FALSE, FALSE, 4);
    gtk_widget_show(as->tape_status.control);

    gtk_widget_set_events(as->tape_status.event_box, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_ENTER_NOTIFY_MASK);
    g_signal_connect(G_OBJECT(as->tape_status.event_box), "button-press-event", G_CALLBACK(tape_popup_cb), (gpointer)NULL);
#if defined(HAVE_CAIRO)
#if GTK_CHECK_VERSION(3, 0, 0)
    g_signal_connect(G_OBJECT(as->tape_status.control), "draw", G_CALLBACK(tape_draw), (gpointer)as->canvas);
#else
    g_signal_connect(G_OBJECT(as->tape_status.control), "expose-event", G_CALLBACK(tape_draw), (gpointer)as->canvas);
#endif
#endif
}

void ui_set_tape_status(int tape_status)
{
    DBG(("ui_set_tape_status (%d)\n", tape_status));
    if (tape_image_status == tape_status) {
        return;
    }
    tape_image_status = tape_status;
    ui_display_tape_control_status(-1);
}

void ui_display_tape_motor_status(int motor)
{
    DBG(("ui_display_tape_motor_status (%d)\n", motor));
    if (tape_motor_status == motor) {
        return;
    }
    tape_motor_status = motor;
    ui_display_tape_control_status(-1);
}


static GdkPoint ffw[] = {
    { 0, 0 },
    { CTRL_WIDTH / 2, CTRL_HEIGHT / 2 },
    { CTRL_WIDTH / 2, 0 },
    { CTRL_WIDTH - 1, CTRL_HEIGHT / 2 },
    { CTRL_WIDTH / 2, CTRL_HEIGHT - 1 }, 
    { CTRL_WIDTH / 2, CTRL_HEIGHT / 2 },
    { 0, CTRL_HEIGHT - 1 }
};

static GdkPoint rew[] = {
    { 0, CTRL_HEIGHT / 2 },
    { CTRL_WIDTH / 2, 0 },
    { CTRL_WIDTH / 2, CTRL_HEIGHT / 2 },
    { CTRL_WIDTH - 1, 0 },
    { CTRL_WIDTH - 1, CTRL_HEIGHT - 1 },
    { CTRL_WIDTH / 2, CTRL_HEIGHT / 2 },
    { CTRL_WIDTH / 2, CTRL_HEIGHT - 1 }
};

static GdkPoint play[] = {
    { 3, 0 },
    { CTRL_WIDTH - 3, CTRL_HEIGHT / 2},
    { 3, CTRL_HEIGHT - 1 }
};

static GdkPoint stop[] = {
    { 3, 2 },
    { CTRL_WIDTH - 3, 2 },
    { CTRL_WIDTH - 3, CTRL_HEIGHT - 2 },
    { 3, CTRL_HEIGHT - 2 }
};

static int numpoints[5] = {
    4, 3, 7, 7, 0
};
static GdkPoint *polyptr[5] = {
    stop, play, ffw, rew, NULL
};

#if !defined(HAVE_CAIRO)
static void set_background(GdkColor *color)
{
    int i;
    int num_app_shells = get_num_shells();
    GdkGC *app_gc = get_toplevel();

    gdk_gc_set_rgb_fg_color(app_gc, color);
    for (i = 0; i < num_app_shells; i++) {
        tape_status_widget *ts = &app_shells[i].tape_status;
        if (ts) {
            gdk_draw_rectangle(ts->control_pixmap, app_gc, TRUE, 0, 0, CTRL_WIDTH, CTRL_HEIGHT);
        }
    }
}

static void set_foreground(GdkColor *color, int status)
{
    int i, num;
    GdkPoint *p;
    int num_app_shells = get_num_shells();
    GdkGC *app_gc = get_toplevel();

    num = numpoints[status];
    p = polyptr[status];

    if (num) {
        gdk_gc_set_rgb_fg_color(app_gc, color);
        for (i = 0; i < num_app_shells; i++) {
            tape_status_widget *ts = &app_shells[i].tape_status;
            if (ts) {
                gdk_draw_polygon(ts->control_pixmap, app_gc, TRUE, p, num);
                gtk_widget_queue_draw(ts->control);
            }
        }
    } else {
        gdk_gc_set_rgb_fg_color(app_gc, &drive_led_on_red_pixel);
        for (i = 0; i < num_app_shells; i++) {
            tape_status_widget *ts = &app_shells[i].tape_status;
            if (ts) {
                gdk_draw_arc(ts->control_pixmap, app_gc, TRUE, 3, 1, CTRL_WIDTH - 6, CTRL_HEIGHT - 2, 0, 360 * 64);
                gtk_widget_queue_draw(ts->control);
            }
        }
    }
}

#else

static void draw_polygon(cairo_t *cr, GdkPoint *p, int num)
{
    cairo_move_to(cr, p->x, p->y);
    ++p;--num;
    while(num) {
        cairo_line_to(cr, p->x, p->y);
        ++p;--num;
    }
}

static void set_background(GdkColor *color)
{
    int num_app_shells = get_num_shells();
    int i;
    for (i = 0; i < num_app_shells; i++) {
        tape_status_widget *ts = &app_shells[i].tape_status;
        if (ts) {
            cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(ts->control));
            gdk_cairo_set_source_color(cr, color);
            cairo_translate(cr, 0, (gtk_widget_get_allocated_height(ts->control) - CTRL_HEIGHT) / 2);
            cairo_rectangle (cr, 0, 0, CTRL_WIDTH, CTRL_HEIGHT);
            cairo_fill (cr);
            cairo_destroy(cr);
        }
    }
}

static void set_foreground(GdkColor *color, int status)
{
    int num_app_shells = get_num_shells();
    int i, num;
    GdkPoint *p;

    num = numpoints[status];
    p = polyptr[status];

    for (i = 0; i < num_app_shells; i++) {
        tape_status_widget *ts = &app_shells[i].tape_status;
        if (ts) {
            cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(ts->control));
            if (cr) {
                gdk_cairo_set_source_color(cr, color);
                cairo_translate(cr, 0, (gtk_widget_get_allocated_height(ts->control) - CTRL_HEIGHT) / 2);
                if (num) {
                    draw_polygon(cr, p, num);
                } else {
                    cairo_arc(cr, CTRL_WIDTH / 2, CTRL_HEIGHT / 2, (CTRL_HEIGHT / 2) - 1, 0, 360.0f * (M_PI / 180.0f));
                }
                cairo_fill (cr);
                cairo_destroy(cr);
            }
        }
    }
}
#endif

void ui_display_tape_control_status(int control)
{
    GdkColor *color;
    int status = 0;
#if !defined(HAVE_CAIRO)
    GdkGC *app_gc = get_toplevel();
#endif

    DBG(("ui_display_tape_control_status (%d) motor:%d image:%d\n", control, tape_motor_status, tape_image_status));

    if (control < 0) {
        control = tape_control_status;
    } else {
        tape_control_status = control;
    }

#if !defined(HAVE_CAIRO)
    if (app_gc == NULL) {
        DBG(("ui_display_tape_control_status skipped\n"));
        return;
    }
#endif

    /* Set background color */
    color = (tape_image_status == 1) ? &drive_led_on_green_pixel : &drive_led_on_red_pixel;
    set_background(color);

    /* set foreground color for tape-button */
    color = (tape_motor_status == 1) ? &motor_running_pixel : &tape_control_pixel;
    switch (control) {
        case DATASETTE_CONTROL_START:
            status = 1;
            break;
        case DATASETTE_CONTROL_FORWARD:
            status = 2;
            break;
        case DATASETTE_CONTROL_REWIND:
            status = 3;
            break;
        case DATASETTE_CONTROL_RECORD:
            status = 4;
            break;
    }
    set_foreground(color, status);
    gdk_flush();
}
