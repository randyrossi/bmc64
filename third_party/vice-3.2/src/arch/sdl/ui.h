/*
 * ui.h
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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

#ifndef VICE_UI_H
#define VICE_UI_H

#include "vice.h"

#include "vice_sdl.h"

#include "types.h"
#include "uiapi.h"
#include "uimenu.h"

/* Allow native monitor code for various platforms. */
#if defined(UNIX_COMPILE)
#define ALLOW_NATIVE_MONITOR
#endif

/* Number of drives we support in the UI.  */
#define NUM_DRIVES 4

/* Tell menu system to ignore a string for translation
   (e.g. filenames in fliplists) */
#define NO_TRANS "no-trans"

typedef enum {
    UI_BUTTON_NONE, UI_BUTTON_CLOSE, UI_BUTTON_OK, UI_BUTTON_CANCEL,
    UI_BUTTON_YES, UI_BUTTON_NO, UI_BUTTON_RESET, UI_BUTTON_HARDRESET,
    UI_BUTTON_MON, UI_BUTTON_DEBUG, UI_BUTTON_CONTENTS, UI_BUTTON_AUTOSTART
} ui_button_t;

/* ------------------------------------------------------------------------- */
/* Prototypes */

struct video_canvas_s;
struct palette_s;

extern void ui_display_speed(float percent, float framerate, int warp_flag);
extern void ui_display_paused(int flag);
extern void ui_handle_misc_sdl_event(SDL_Event e);
extern ui_menu_action_t ui_dispatch_events(void);
extern void ui_exit(void);
extern void ui_message(const char *format, ...);
extern void ui_show_text(const char *title, const char *text, int width, int height);
extern char *ui_select_file(const char *title, char *(*read_contents_func)(const char *, unsigned int unit), unsigned int unit,
                            unsigned int allow_autostart, const char *default_dir, const char *default_pattern,
                            ui_button_t *button_return, unsigned int show_preview, int *attach_wp);
extern ui_button_t ui_input_string(const char *title, const char *prompt, char *buf, unsigned int buflen);
extern ui_button_t ui_ask_confirmation(const char *title, const char *text);
extern void ui_autorepeat_on(void);
extern void ui_autorepeat_off(void);
extern void ui_pause_emulation(int flag);
extern int ui_emulation_is_paused(void);
extern void ui_check_mouse_cursor(void);
extern void ui_restore_mouse(void);

extern void ui_set_application_icon(const char *icon_data[]);
extern void ui_set_selected_file(int num);

extern void ui_destroy_drive_menu(int drnr);
extern void ui_destroy_drive8_menu(void);
extern void ui_destroy_drive9_menu(void);
extern void ui_update_pal_ctrls(int v);

extern void ui_common_init(void);
extern void ui_common_shutdown(void);
extern void ui_sdl_quit(void);

extern int native_monitor;

#ifdef BEOS_COMPILE
extern int CheckForHaiku(void);
#endif

#endif
