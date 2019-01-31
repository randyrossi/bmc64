/*
 * ui.h
 *
 * Written by
 *  pottendo <pottendo@gmx.net>
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

#ifndef VICE_UI_H_
#define VICE_UI_H_

#include "vice.h"

#include "types.h"
#include "imagecontents.h"

/* If this is #defined, `Alt' is handled the same as `Meta'.  On
   systems which have Meta, it's better to use Meta instead of Alt as
   a shortcut modifier (because Alt is usually used by Window
   Managers), but systems that don't have Meta (eg. GNU/Linux, HP-UX)
   would suffer then.  So it's easier to just handle Meta as Alt in
   such cases.  */
#define ALT_AS_META

/* Number of drives we support in the UI.  */
#define NUM_DRIVES 4

/* Tell menu system to ignore a string for translation
   (e.g. filenames in fliplists) */
#define NO_TRANS "no-trans"

typedef enum {
    UI_BUTTON_NONE,
    UI_BUTTON_CLOSE,
    UI_BUTTON_OK,
    UI_BUTTON_CANCEL,
    UI_BUTTON_YES,
    UI_BUTTON_NO,
    UI_BUTTON_RESET,
    UI_BUTTON_HARDRESET,
    UI_BUTTON_MON,
    UI_BUTTON_DEBUG,
    UI_BUTTON_CONTENTS,
    UI_BUTTON_AUTOSTART
} ui_button_t;

/* ------------------------------------------------------------------------- */
/* Prototypes */

struct video_canvas_s;
struct palette_s;

typedef enum ui_filechooser_s {
    UI_FC_LOAD = 0,
    UI_FC_SAVE,
    UI_FC_DIRECTORY
} ui_filechooser_t;

void ui_display_speed(float percent, float framerate, int warp_flag);
void ui_display_paused(int flag);
void ui_dispatch_events(void);
extern void ui_exit(void);
extern void ui_show_text(const char *title, const char *text, int width, int height);

/* only used in the X11 (XAW/GTK) UI */
#ifndef X_DISPLAY_MISSING
#include "gui/uilib.h"
extern char *ui_select_file(const char *title, read_contents_func_type read_contents_func, unsigned int allow_autostart, const char *default_dir, uilib_file_filter_enum_t* patterns,
                            int num_patterns, ui_button_t *button_return, unsigned int show_preview, int *attach_wp, ui_filechooser_t action);
#else
/* FIXME: this can probably be removed as no other ui uses it (?) */
/* only used in non X11 UIs */
enum uilib_file_filter_enum_s;
extern char *ui_select_file(const char *title, read_contents_func_type read_contents_func, unsigned int allow_autostart, const char *default_dir, enum uilib_file_filter_enum_s* patterns,
                            int num_patterns, ui_button_t *button_return, unsigned int show_preview, int *attach_wp, ui_filechooser_t action);
#endif

extern ui_button_t ui_input_string(const char *title, const char *prompt, char *buf, unsigned int buflen);

extern ui_button_t ui_ask_confirmation(const char *title, const char *text);
extern ui_button_t ui_ask_yesno(const char *title, const char *text);

extern void ui_autorepeat_on(void);
extern void ui_autorepeat_off(void);
extern void ui_pause_emulation(int flag);
extern int ui_emulation_is_paused(void);
extern void ui_check_mouse_cursor(void);
extern void ui_restore_mouse(void);
extern int ui_focus_monitor(void);
extern void ui_restore_focus(void);

extern void ui_set_application_icon(const char *icon_data[]);
extern void ui_set_selected_file(int num);

extern void ui_destroy_drive_menu(int drnr);
extern void ui_update_pal_ctrls(int v);

extern void ui_common_init(void);
extern void ui_common_shutdown(void);

#endif
