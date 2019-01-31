/** \file   ui.h
 * \brief   Main Gtk3 UI code - header
 *
 * \author  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#ifndef VICE_UI_H
#define VICE_UI_H

#include "vice.h"

#include <gtk/gtk.h>
#include <stdbool.h>

#include "videoarch.h"
#include "palette.h"


/* Number of drives we support in the UI.  */
#define NUM_DRIVES 4


/** \brief  Number of GtkWindow's in the ui_resources
 */
#define NUM_WINDOWS 3


/** \brief  Window indices
 */
enum {
    PRIMARY_WINDOW,     /**< primary window, all emulators */
    SECONDARY_WINDOW,   /**< secondary window, C128's VDC */
    MONITOR_WINDOW      /**< optional monitor window/terminal */
};


/* ------------------------------------------------------------------------- */
/* Prototypes */

void ui_set_create_window_func(void (*func)(video_canvas_t *));
void ui_set_identify_canvas_func(int (*func)(video_canvas_t *));
void ui_set_create_controls_widget_func(GtkWidget *(*func)(int));

void ui_create_main_window(video_canvas_t *canvas);
void ui_display_main_window(int index);
void ui_destroy_main_window(int index);

void ui_display_speed(float percent, float framerate, int warp_flag);
void ui_display_paused(int flag);
void ui_dispatch_events(void);
void ui_exit(void);
void ui_show_text(const char *title, const char *text, int width, int height);

void ui_autorepeat_on(void);
void ui_autorepeat_off(void);

void ui_display_paused(int flag);
void ui_pause_emulation(int flag);
int  ui_emulation_is_paused(void);
int  ui_is_fullscreen(void);
void ui_trigger_resize(void);
void ui_fullscreen_callback(GtkWidget *widget, gpointer user_data);
void ui_fullscreen_decorations_callback(GtkWidget *widget, gpointer user_data);

void ui_check_mouse_cursor(void);
void ui_restore_mouse(void);

void ui_set_application_icon(const char *icon_data[]);
void ui_set_selected_file(int num);

void ui_common_init(void);
void ui_common_shutdown(void);

GtkWindow *ui_get_active_window(void);
video_canvas_t *ui_get_active_canvas(void);

gboolean ui_toggle_pause(void);
gboolean ui_advance_frame(void);

void ui_update_lightpen(void);

void ui_enable_crt_controls(bool enabled);

#endif
