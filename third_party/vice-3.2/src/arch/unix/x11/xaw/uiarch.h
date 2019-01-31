/*
 * ui.h - Simple Xaw-based graphical user interface.  It uses widgets
 * from the Free Widget Foundation and Robert W. McMullen.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#ifndef VICE_UIARCH_XAW_H
#define VICE_UIARCH_XAW_H

#include "vice.h"

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/keysym.h>

#include "types.h"
#include "joystick.h"
#include "ui.h"
#include "uitapestatus.h"

/* If this is #defined, `Mode_switch' is handled the same as `Meta'.  */
/* #define MODE_SWITCH_AS_META */

typedef Widget ui_window_t;
typedef XtCallbackProc ui_callback_t;
typedef XtPointer ui_callback_data_t;

enum ui_keysym_s {
    KEYSYM_NONE = 0,
    KEYSYM_0 = XK_0,
    KEYSYM_1 = XK_1,
    KEYSYM_2 = XK_2,
    KEYSYM_3 = XK_3,
    KEYSYM_4 = XK_4,
    KEYSYM_5 = XK_5,
    KEYSYM_6 = XK_6,
    KEYSYM_7 = XK_7,
    KEYSYM_8 = XK_8,
    KEYSYM_9 = XK_9,
    KEYSYM_a = XK_a,
    KEYSYM_b = XK_b,
    KEYSYM_c = XK_c,
    KEYSYM_d = XK_d,
    KEYSYM_e = XK_e,
    KEYSYM_f = XK_f,
    KEYSYM_g = XK_g,
    KEYSYM_h = XK_h,
    KEYSYM_i = XK_i,
    KEYSYM_j = XK_j,
    KEYSYM_J = XK_J,
    KEYSYM_k = XK_k,
    KEYSYM_l = XK_l,
    KEYSYM_m = XK_m,
    KEYSYM_n = XK_n,
    KEYSYM_N = XK_N,
    KEYSYM_p = XK_p,
    KEYSYM_q = XK_q,
    KEYSYM_s = XK_s,
    KEYSYM_t = XK_t,
    KEYSYM_u = XK_u,
    KEYSYM_w = XK_w,
    KEYSYM_z = XK_z,
    KEYSYM_F9  = XK_F9 ,
    KEYSYM_F10 = XK_F10,
    KEYSYM_F11 = XK_F11,
    KEYSYM_F12 = XK_F12
};
typedef enum ui_keysym_s ui_keysym_t;

#define UI_CALLBACK(name) void name(Widget w, ui_callback_data_t client_data, ui_callback_data_t call_data)

/* fill callback variables */
#define CHECK_MENUS (call_data != NULL)
#define UI_MENU_CB_PARAM (client_data)

extern Widget _ui_top_level;
extern Visual *visual;
extern Atom wm_delete_window;
extern Atom wm_protocols;

/* ------------------------------------------------------------------------- */

struct video_canvas_s;

#define NUM_TAPES       1
#define MAX_APP_SHELLS 10
typedef struct {
    String title;
    Widget shell;
    Widget canvas;
    Widget speed_label;
    Widget statustext_label;
    struct {
        Widget status;          /* container for the following widgets */
        Widget track_label;
        Widget driveled;
        /* those two replace the single LED widget when SFD1001 is selected */
        Widget driveled1;
        Widget driveled2;
    } drive_widgets[NUM_DRIVES];
    int drive_nleds[NUM_DRIVES];
    tape_widgets_t tape_widgets[NUM_TAPES];
    struct video_canvas_s *video_canvas;
    Widget joystick_status[JOYSTICK_NUM];
} app_shell_type;

extern app_shell_type app_shells[MAX_APP_SHELLS];
extern Pixel drive_led_on_red_pixel, drive_led_on_green_pixel, drive_led_off_pixel;

/* ------------------------------------------------------------------------- */
/* Prototypes */

extern int ui_open_canvas_window(struct video_canvas_s *c, const char *title, int width, int height, int no_autorepeat);
extern Widget ui_create_shell(Widget parent, const char *name, WidgetClass class);
extern Widget ui_create_transient_shell(Widget parent, const char *name);
extern void ui_popdown(Widget w);
extern void ui_popup(Widget w, const char *title, Boolean wait_popdown);
extern void ui_about (Widget w, ui_callback_data_t cd, ui_callback_data_t cl);
extern int ui_fullscreen_statusbar(struct video_canvas_s *canvas, int enable);

extern void ui_set_drop_callback(void *cb);
extern int get_num_shells(void);
extern void ui_delete_menu(Widget w);
extern void ui_update_menus(void);

#endif
