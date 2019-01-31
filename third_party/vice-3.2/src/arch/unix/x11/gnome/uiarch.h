/*
 * uiarch.h 
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
 *  Oliver Schaertel
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

#ifndef VICE_UIARCH_H
#define VICE_UIARCH_H

/*
    FIXME: maintaining backwards compatibility in terms of being able to compile
           the code conflicts with updating the code so it can be compiled 
           without the "#undef GSEAL_ENABLE" workaround below.

           however, since we really want to update the code so we can compile it
           cleanly and hopefully some day without using any deprecated stuff at
           all, backwards compatibility would have to be maintained seperately
           on source level (by providing the missing setter/getter functions as
           macros, for example) - see gtk2legacy.c/h
*/

/* undefine the access checks to make the compatibility layer work */
#undef GSEAL_ENABLE

#if 0 /* use this when working on the code */

#define GSEAL_ENABLE

#if 0
/* gtk3 */
#define GDK_VERSION_MIN_REQIRED GDK_VERSION_2_24  /* dont use symbols deprecated in this version */
#define GDK_VERSION_MAX_REQIRED GDK_VERSION_3_00  /* dont use symbols introduced after this version */

/* #define GDK_MULTIHEAD_SAFE 1 */ /* conflicts with manipulating mouse cursor and using clipboard */
#define GTK_MULTIDEVICE_SAFE 1

/* #define GTK_DISABLE_DEPRECATED */
#define GNOME_DISABLE_DEPRECATED
#define GDK_DISABLE_DEPRECATED
#define GDK_PIXBUF_DISABLE_DEPRECATED
#define G_DISABLE_DEPRECATED
/* #define GTK_DISABLE_SINGLE_INCLUDES */ /* FIXME: GL stuff */

#else
/* gtk2 */

/* #define GDK_MULTIHEAD_SAFE 1 */ /* conflicts with manipulating mouse cursor and using clipboard */
#define GTK_MULTIDEVICE_SAFE 1

#define GTK_DISABLE_DEPRECATED
#define GNOME_DISABLE_DEPRECATED
#define GDK_DISABLE_DEPRECATED
#define GDK_PIXBUF_DISABLE_DEPRECATED
#define G_DISABLE_DEPRECATED
/* #define GTK_DISABLE_SINGLE_INCLUDES */ /* FIXME: GL stuff */

#endif

/* FIXME: only use one of these for all GTK stuff */
/* #define DEBUG_X11UI */
/* #define DEBUG_GNOMEUI */

/* #define DEBUGMOUSECURSOR */  /* dont use a blank mouse cursor */
/* #define DEBUGNOMOUSEGRAB */  /* dont grab mouse */
/* #define DEBUG_KBD */
/* #define DEBUGNOKBDGRAB */    /* dont explicitly grab keyboard focus */
/* #define DEBUG_LIGHTPEN */

#endif

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "vice.h"

#include "gtk2legacy.h" /* this must come first here */

/* various options, undefine them here for testing */
/* #undef HAVE_PANGO */
/* #undef HAVE_CAIRO */
/* #undef HAVE_VTE */
/* #undef HAVE_HWSCALE */
/* #undef HAVE_FULLSCREEN */
/* #undef USE_XF86_EXTENSIONS */

#include "joystick.h"
#include "log.h"
#include "ui.h"
#include "uiapi.h"

typedef GtkWidget *ui_window_t;
typedef GCallback ui_callback_t;
typedef gpointer ui_callback_data_t;
enum ui_keysym_s {
    KEYSYM_NONE = 0,
#if GTK_CHECK_VERSION(2, 24, 0)
    KEYSYM_0 = GDK_KEY_0,
    KEYSYM_1 = GDK_KEY_1,
    KEYSYM_2 = GDK_KEY_2,
    KEYSYM_3 = GDK_KEY_3,
    KEYSYM_4 = GDK_KEY_4,
    KEYSYM_5 = GDK_KEY_5,
    KEYSYM_6 = GDK_KEY_6,
    KEYSYM_7 = GDK_KEY_7,
    KEYSYM_8 = GDK_KEY_8,
    KEYSYM_9 = GDK_KEY_9,
    KEYSYM_a = GDK_KEY_a,
    KEYSYM_b = GDK_KEY_b,
    KEYSYM_c = GDK_KEY_c,
    KEYSYM_d = GDK_KEY_d,
    KEYSYM_e = GDK_KEY_e,
    KEYSYM_f = GDK_KEY_f,
    KEYSYM_g = GDK_KEY_g,
    KEYSYM_h = GDK_KEY_h,
    KEYSYM_i = GDK_KEY_i,
    KEYSYM_j = GDK_KEY_j,
    KEYSYM_J = GDK_KEY_J,
    KEYSYM_k = GDK_KEY_k,
    KEYSYM_l = GDK_KEY_l,
    KEYSYM_m = GDK_KEY_m,
    KEYSYM_n = GDK_KEY_n,
    KEYSYM_N = GDK_KEY_N,
    KEYSYM_p = GDK_KEY_p,
    KEYSYM_q = GDK_KEY_q,
    KEYSYM_s = GDK_KEY_s,
    KEYSYM_t = GDK_KEY_t,
    KEYSYM_u = GDK_KEY_u,
    KEYSYM_v = GDK_KEY_v,
    KEYSYM_w = GDK_KEY_w,
    KEYSYM_z = GDK_KEY_z,
    KEYSYM_F9  = GDK_KEY_F9 ,
    KEYSYM_F10 = GDK_KEY_F10,
    KEYSYM_F11 = GDK_KEY_F11,
    KEYSYM_F12 = GDK_KEY_F12,
    KEYSYM_Alt_L = GDK_KEY_Alt_L,
    KEYSYM_Alt_R = GDK_KEY_Alt_R,
    KEYSYM_VoidSymbol = GDK_KEY_VoidSymbol,
    KEYSYM_Shift_L = GDK_KEY_Shift_L,
    KEYSYM_Shift_R = GDK_KEY_Shift_R,
    KEYSYM_Shift_Lrel = GDK_KEY_ISO_Next_Group,
    KEYSYM_Shift_Rrel = GDK_KEY_ISO_Prev_Group,
    KEYSYM_ISO_Level3_Shift = GDK_KEY_ISO_Level3_Shift,
    KEYSYM_KP_0 = GDK_KEY_KP_0,
    KEYSYM_KP_1 = GDK_KEY_KP_1,
    KEYSYM_KP_2 = GDK_KEY_KP_2,
    KEYSYM_KP_3 = GDK_KEY_KP_3,
    KEYSYM_KP_4 = GDK_KEY_KP_4,
    KEYSYM_KP_5 = GDK_KEY_KP_5,
    KEYSYM_KP_6 = GDK_KEY_KP_6,
    KEYSYM_KP_7 = GDK_KEY_KP_7,
    KEYSYM_KP_8 = GDK_KEY_KP_8,
    KEYSYM_KP_9 = GDK_KEY_KP_9,
    KEYSYM_Escape = GDK_KEY_Escape
#else
    KEYSYM_0 = GDK_0,
    KEYSYM_1 = GDK_1,
    KEYSYM_2 = GDK_2,
    KEYSYM_3 = GDK_3,
    KEYSYM_4 = GDK_4,
    KEYSYM_5 = GDK_5,
    KEYSYM_6 = GDK_6,
    KEYSYM_7 = GDK_7,
    KEYSYM_8 = GDK_8,
    KEYSYM_9 = GDK_9,
    KEYSYM_a = GDK_a,
    KEYSYM_b = GDK_b,
    KEYSYM_c = GDK_c,
    KEYSYM_d = GDK_d,
    KEYSYM_e = GDK_e,
    KEYSYM_f = GDK_f,
    KEYSYM_g = GDK_g,
    KEYSYM_h = GDK_h,
    KEYSYM_i = GDK_i,
    KEYSYM_j = GDK_j,
    KEYSYM_J = GDK_J,
    KEYSYM_k = GDK_k,
    KEYSYM_l = GDK_l,
    KEYSYM_m = GDK_m,
    KEYSYM_n = GDK_n,
    KEYSYM_N = GDK_N,
    KEYSYM_p = GDK_p,
    KEYSYM_q = GDK_q,
    KEYSYM_s = GDK_s,
    KEYSYM_t = GDK_t,
    KEYSYM_u = GDK_u,
    KEYSYM_v = GDK_v,
    KEYSYM_w = GDK_w,
    KEYSYM_z = GDK_z,
    KEYSYM_F9  = GDK_F9 ,
    KEYSYM_F10 = GDK_F10,
    KEYSYM_F11 = GDK_F11,
    KEYSYM_F12 = GDK_F12,
    KEYSYM_Alt_L = GDK_Alt_L,
    KEYSYM_Alt_R = GDK_Alt_R,
    KEYSYM_VoidSymbol = GDK_VoidSymbol,
    KEYSYM_Shift_L = GDK_Shift_L,
    KEYSYM_Shift_R = GDK_Shift_R,
    KEYSYM_Shift_Lrel = GDK_ISO_Next_Group,
    KEYSYM_Shift_Rrel = GDK_ISO_Prev_Group,
    KEYSYM_ISO_Level3_Shift = GDK_ISO_Level3_Shift,
    KEYSYM_KP_0 = GDK_KP_0,
    KEYSYM_KP_1 = GDK_KP_1,
    KEYSYM_KP_2 = GDK_KP_2,
    KEYSYM_KP_3 = GDK_KP_3,
    KEYSYM_KP_4 = GDK_KP_4,
    KEYSYM_KP_5 = GDK_KP_5,
    KEYSYM_KP_6 = GDK_KP_6,
    KEYSYM_KP_7 = GDK_KP_7,
    KEYSYM_KP_8 = GDK_KP_8,
    KEYSYM_KP_9 = GDK_KP_9,
    KEYSYM_Escape = GDK_Escape
#endif
};
typedef enum ui_keysym_s ui_keysym_t;

#define UI_CALLBACK(name) void name(GtkWidget *w, ui_callback_data_t event_data)

#define CHECK_MENUS      (((ui_menu_cb_obj*)event_data)->status != CB_NORMAL)
#define UI_MENU_CB_PARAM (((ui_menu_cb_obj*)event_data)->value) 

/* Joystick status widget */
typedef struct {
    GtkWidget *box;                     /* contains all the widgets */
    GtkWidget *pixmap;
    GtkWidget *event_box;
    GtkWidget *led;
#if !defined(HAVE_CAIRO)
    GdkPixmap *led_pixmap;
#endif
    GdkColor *colors[5]; /* FIXME */
} joystick_status_widget;

/* Drive status widget */
typedef struct {
    GtkWidget *box;                     /* contains all the widgets */
    char *label;
    GtkWidget *pixmap;
    GtkWidget *event_box;
    GtkWidget *track_label;
    GtkWidget *led;
    GtkWidget *led1;
    GtkWidget *led2;
#if !defined(HAVE_CAIRO)
    GdkPixmap *led_pixmap;
    GdkPixmap *led1_pixmap;
    GdkPixmap *led2_pixmap;
#endif
    GdkColor *color1, *color2;
} drive_status_widget;

/* Tape status widget */
typedef struct {
    GtkWidget *box;
    GtkWidget *event_box;
    GtkWidget *label;
    GtkWidget *control;
#if !defined(HAVE_CAIRO)
    GdkPixmap *control_pixmap;
#endif
} tape_status_widget;

#define MAX_APP_SHELLS 10
typedef struct {
    gchar *title;
    GtkWidget *shell;
    GtkWidget *topmenu;
    GtkWidget *status_bar;
    GtkWidget *pal_ctrl;
    void *pal_ctrl_data;
    GtkLabel *speed_label;
    GtkLabel *statustext;
    GtkAccelGroup *accel;
    joystick_status_widget joystick_status[JOYSTICK_NUM];
    drive_status_widget drive_status[NUM_DRIVES];
    tape_status_widget tape_status;
    GdkGeometry geo;
    struct video_canvas_s *canvas;
} app_shell_type;
extern app_shell_type app_shells[MAX_APP_SHELLS];
extern int get_num_shells(void);

#if !defined(HAVE_CAIRO)
extern GdkGC *get_toplevel(void);
extern GdkVisual *visual; /* FIXME: also wrap into a function call */
#endif

extern GtkWidget *get_active_toplevel(void);
extern int get_active_shell(void);
extern struct video_canvas_s *get_active_canvas(void);

extern void ui_trigger_resize(void);
extern void ui_trigger_window_resize(struct video_canvas_s *c);

extern int ui_open_canvas_window(struct video_canvas_s *c, const char *title, int width, int heigth, int no_autorepeat);
extern void ui_resize_canvas_window(struct video_canvas_s *c);
extern GtkWidget *ui_create_transient_shell(GtkWidget *parent, const char *name);
extern void ui_popdown(GtkWidget *w);
extern void ui_popup(GtkWidget *w, const char *title, gboolean wait_popdown);
extern void ui_make_window_transient(GtkWidget *parent,GtkWidget *window);
extern void ui_about(gpointer data);
extern gint ui_hotkey_event_handler(GtkWidget *w, GdkEvent *report, gpointer gp);
extern void ui_block_shells(void);
extern void ui_unblock_shells(void);
extern int ui_fullscreen_statusbar(struct video_canvas_s *canvas, int enable);

extern void ui_set_drop_callback(void *cb);

extern unsigned char *convert_utf8(unsigned char *s);

extern int ui_open_manual(const char *path);

/* UI logging goes here.  */
extern log_t ui_log;

/* color constants used by the GUI */
extern GdkColor drive_led_on_red_pixel, drive_led_on_green_pixel;
extern GdkColor drive_led_off_pixel, motor_running_pixel, tape_control_pixel;
extern GdkColor drive_led_on_red_pixels[16];
extern GdkColor drive_led_on_green_pixels[16];

#endif /* !defined (_UIARCH_H) */
