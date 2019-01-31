/*
 * ui.c - A (very) simple user interface for MS-DOS.
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

#include "vice.h"

#include <conio.h>
#include <fcntl.h>
#include <go32.h>
#include <io.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/movedata.h>
#include <unistd.h>
#include <string.h>

#include "cmdline.h"
#include "datasette.h"
#include "dos.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "menudefs.h"
#include "monitor.h"
#include "resources.h"
#include "sound.h"
#include "statusbar.h"
#include "translate.h"
#include "tui.h"
#include "tui_backend.h"
#include "tuicharset.h"
#include "tuimenu.h"
#include "types.h"
#include "ui.h"
#include "video.h"
#include "videoarch.h"
#include "version.h"
#include "vsync.h"

/* Status of keyboard LEDs.  */
static int real_kbd_led_status = -1;
static int kbd_led_status;

#define DRIVE0_LED_MSK 4
#define DRIVE1_LED_MSK 4
#define WARP_LED_MSK 1

/* ------------------------------------------------------------------------- */

/* UI-related resources and command-line options.  */

/* Flag: Use keyboard LEDs?  */
static unsigned int use_leds;

static unsigned int statusbar_is_enabled;

static int set_use_leds(int v, void *param)
{
    use_leds = v ? 1 : 0;

    return 0;
}

static int set_statusbar_enabled(int v, void *param) 
{
    switch (v) {
        case STATUSBAR_MODE_OFF:
        case STATUSBAR_MODE_ON:
        case STATUSBAR_MODE_AUTO:
            break;
        default:
            return -1;
    }

    statusbar_is_enabled = v;

    if (statusbar_enabled()) {
        statusbar_prepare();
        statusbar_update();
    } else {
        statusbar_disable();
    }
    return 0;
}

static const resource_int_t resources_int[] = {
    { "UseLeds", 1, RES_EVENT_NO, NULL,
      &use_leds, set_use_leds, NULL },
    { "ShowStatusbar", STATUSBAR_MODE_AUTO, RES_EVENT_NO, NULL,
      &statusbar_is_enabled, set_statusbar_enabled, NULL },
    RESOURCE_INT_LIST_END
};

int ui_resources_init(void)
{
    return resources_register_int(resources_int);
}

void ui_resources_shutdown(void)
{
}

static const cmdline_option_t cmdline_options[] = {
    { "-leds", SET_RESOURCE, 0,
      NULL, NULL, "UseLeds", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Enable usage of PC keyboard LEDs" },
    { "+leds", SET_RESOURCE, 0,
      NULL, NULL, "UseLeds", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Disable usage of PC keyboard LEDs" },
    { "-statusbar", SET_RESOURCE, 1,
      NULL, NULL, "ShowStatusbar", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      "<Mode>", "Set the Statusbar mode (0: Off, 1: On, 2: Auto)" },
    CMDLINE_LIST_END
};

int ui_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

inline static void set_kbd_leds(int value)
{
    /* FIXME: Is this the 100% correct way to do it?  */
    if (use_leds) {
        outportb(0x60, 0xed);
        delay(1);
        outportb(0x60, value);
        delay(1);
    }
}

/* ------------------------------------------------------------------------- */

static void ui_exit(void)
{
    statusbar_exit();
    _setcursortype(_NORMALCURSOR);
    normvideo();
    _set_screen_lines(25);

#ifdef UNSTABLE
    cprintf("VICE version %s (unstable).\n\r", VERSION);
#else
    cprintf("VICE version %s.\n\r", VERSION);
#endif

    cprintf("\nOfficial VICE homepage: http://vice-emu.sourceforge.net/\n\n\r");
}

int ui_init(int *argc, char **argv)
{
    /* This forces Ctrl-C and Ctrl-Break to be treated as normal key
       sequences.  */
    _go32_want_ctrl_break(1);

    statusbar_init();

    return 0;
}

void ui_shutdown(void)
{
}

int ui_init_finish(void)
{
    log_message(LOG_DEFAULT, "Initializing TUI.");
    tui_init();
    log_message(LOG_DEFAULT, "Initialized TUI.");
    atexit(ui_exit);
    return 0;
}

int ui_init_finalize(void)
{
    return 0;
}

void ui_main(char hotkey)
{
    char *str;
    double speed_index, frame_rate;
    int old_stdin_mode = setmode(STDIN_FILENO, O_BINARY);

    sound_suspend();

    speed_index = vsync_get_avg_speed_index();
    frame_rate = vsync_get_avg_frame_rate();

    /* Get the BIOS LED status and restore it.  */
    {
        uint8_t bios_leds;
        int leds = 0;

        _dosmemgetb(0x417, 1, &bios_leds);
        if (bios_leds & 16) {
            leds |= 1;          /* Scroll Lock */
        }
        if (bios_leds & 32) {
            leds |= 2;          /* Num Lock */
        }
        if (bios_leds & 64) {
            leds |= 4;          /* Caps Lock */
        }
        set_kbd_leds(leds);
    }

    enable_text();
    tui_charset_set(TUI_CHARSET_CBM_2);

    tui_clear_screen();
    tui_set_attr(FIRST_LINE_FORE, FIRST_LINE_BACK, 0);

    if (speed_index > 0.0 && frame_rate > 0.0) {
        str = lib_msprintf("%s emulator at %d%% speed, %d fps", machine_name, (int)floor(speed_index), (int)floor(frame_rate));
    } else {
        str = lib_msprintf("%s emulator", machine_name);
    }
    tui_display(tui_num_cols() - strlen(str), 0, 0, "%s", str);

    /* FIXME: This should not be necessary.  */
    tui_menu_update(ui_main_menu);

    tui_menu_handle(ui_main_menu, hotkey);

    disable_text();
    vsync_suspend_speed_eval();

    set_kbd_leds(real_kbd_led_status);

    setmode(STDIN_FILENO, old_stdin_mode);
    lib_free(str);
}

void ui_error(const char *format,...)
{
    char *tmp;
    va_list ap;

    enable_text();
    tui_clear_screen();

    va_start(ap, format);
    tmp = lib_mvsprintf(format, ap);
    va_end(ap);
    tui_error(tmp);

    disable_text();
    lib_free(tmp);
}

void ui_message(const char *format,...)
{
    char *tmp;
    va_list ap;

    enable_text();
    tui_clear_screen();

    va_start(ap, format);
    tmp = lib_mvsprintf(format, ap);
    va_end(ap);
    tui_message(tmp);

    disable_text();
    lib_free(tmp);
}

ui_jam_action_t ui_jam_dialog(const char *format,...)
{
    char *tmp;
    va_list ap;

    enable_text();
    tui_clear_screen();

    va_start(ap, format);
    tmp = lib_mvsprintf(format, ap);
    va_end(ap);
    tui_error(tmp);

    disable_text();
    lib_free(tmp);

    /* Always hard reset.  */
    return UI_JAM_HARD_RESET;
}

void ui_show_text(const char *title, const char *text)
{
}

void ui_update_menus(void)
{
    if (ui_main_menu != NULL) {
        tui_menu_update(ui_main_menu);
    }
}

static ui_drive_enable_t ui_drive_enabled;
static int ui_status_led[4];
static double ui_status_track[4];
static int *ui_drive_active_led;

static void ui_draw_drive_status(int drive_bar)
{
    BITMAP *drive_bitmap;
    int current_led_color;

    if (status_bitmap == NULL) {
        return;
    }

    drive_bitmap = create_sub_bitmap(status_bitmap, 180 + drive_bar * 70, 2, 60, 8);

    if (((drive_bar == 0) && (ui_drive_enabled & UI_DRIVE_ENABLE_0)) || ((drive_bar == 1) && (ui_drive_enabled & UI_DRIVE_ENABLE_1))) {
        int white = statusbar_get_color(STATUSBAR_COLOR_WHITE);

        textprintf(drive_bitmap, font, 2, 0, statusbar_get_color(STATUSBAR_COLOR_BLUE), "%d", drive_bar + 8);

        /* track */
        textprintf(drive_bitmap, font, 14, 0, white, "%2d", (int)ui_status_track[drive_bar]);
        textprintf(drive_bitmap, font, 30, 0, white, ".");
        textprintf(drive_bitmap, font, 36, 0, white, "%01d", (int)(ui_status_track[drive_bar] * 10) % 10);

        /* drive-led */
        if (!ui_status_led[drive_bar]) {
            current_led_color = statusbar_get_color(STATUSBAR_COLOR_DARKGREY);
        } else {
            current_led_color = ui_drive_active_led[drive_bar] ? statusbar_get_color(STATUSBAR_COLOR_GREEN) : statusbar_get_color(STATUSBAR_COLOR_RED);
        }
        rectfill(drive_bitmap, 48, 2, 55, 4, current_led_color);
    } else {
        clear(drive_bitmap);
    }
    destroy_bitmap(drive_bitmap);
    statusbar_update();
}


void ui_enable_drive_status(ui_drive_enable_t state, int *drive_led_color)
{
    if (!(state & UI_DRIVE_ENABLE_0)) {
        ui_display_drive_led(0, 0, 0);
    }
    if (!(state & UI_DRIVE_ENABLE_1)) {
        ui_display_drive_led(1, 0, 0);
    }
    ui_drive_enabled = state;
    ui_drive_active_led = drive_led_color;
    ui_draw_drive_status(0);
    ui_draw_drive_status(1);
}

/* drive_base is either 8 or 0 depending on unit or drive display.
   Dual drives display drive 0: and 1: instead of unit 8: and 9: */
void ui_display_drive_track(unsigned int drive_number, unsigned int drive_base, unsigned int half_track_number)
{
    double track_number = (double)half_track_number / 2.0;

    ui_status_track[drive_number] = track_number;
    ui_draw_drive_status(drive_number);
}

void ui_display_drive_led(int drive_number, unsigned int led_pwm1, unsigned int led_pwm2)
{
    int status = 0;

    if (led_pwm1 > 100) {
        status |= 1;
    }
    if (led_pwm2 > 100) {
        status |= 2;
    }

    switch (drive_number) {
        case 0:
            if (status) {
                kbd_led_status |= DRIVE0_LED_MSK;
            } else {
                kbd_led_status &= ~DRIVE0_LED_MSK;
            }
            break;
        case 1:
            if (status) {
                kbd_led_status |= DRIVE1_LED_MSK;
            } else {
                kbd_led_status &= ~DRIVE1_LED_MSK;
            }
            break;
        default:
            break;
    }
    ui_status_led[drive_number]=status;
    ui_draw_drive_status(drive_number);
}

/* display current image */
void ui_display_drive_current_image(unsigned int drivenum, const char *image)
{
    /* just a dummy so far */
}

/* tape-related ui */

static int ui_tape_enabled = 0;
static int ui_tape_counter = -1;
static int ui_tape_motor = -1;
static int ui_tape_control = -1;

static void ui_draw_tape_status()
{
    BITMAP *tape_bitmap;
    int motor_color;
    int record_led;
    int black = statusbar_get_color(STATUSBAR_COLOR_BLACK);

    if (status_bitmap == NULL) {
        return;
    }
    tape_bitmap = create_sub_bitmap(status_bitmap,112,2,54,8);
    if (ui_tape_enabled == 0) {
        clear(tape_bitmap);
    } else {
        textprintf(tape_bitmap, font, 0, 0, statusbar_get_color(STATUSBAR_COLOR_BLUE), "T");

        /* motor */
        if (ui_tape_motor) {
            motor_color = statusbar_get_color(STATUSBAR_COLOR_YELLOW);
        } else {
            motor_color = statusbar_get_color(STATUSBAR_COLOR_GREY);
        }
        rectfill(tape_bitmap, 10, 0, 17, 6, motor_color);

        /* control */
        record_led = black;
        switch (ui_tape_control) {
            case DATASETTE_CONTROL_STOP:
                rectfill(tape_bitmap, 12, 2, 15, 4, black); 
                break;
            case DATASETTE_CONTROL_RECORD:
                record_led = statusbar_get_color(STATUSBAR_COLOR_RED);
            case DATASETTE_CONTROL_START:
                triangle(tape_bitmap, 12, 1, 12, 5, 14, 3, black);
                line(tape_bitmap, 12, 1, 12, 5, black);
                break;
            case DATASETTE_CONTROL_REWIND:
                line(tape_bitmap, 13, 1, 11, 3, black);
                line(tape_bitmap, 11, 3, 13, 5, black);
                line(tape_bitmap, 16, 1, 14, 3, black);
                line(tape_bitmap, 14, 3, 16, 5, black);
                break;
            case DATASETTE_CONTROL_FORWARD:
                line(tape_bitmap, 11, 1, 13, 3, black);
                line(tape_bitmap, 13, 3, 11, 5, black);
                line(tape_bitmap, 14, 1, 16, 3, black);
                line(tape_bitmap, 16, 3, 14, 5, black);
                break;
        }
        rectfill(tape_bitmap, 20,2, 22,4, record_led);

        /* counter */
        textprintf(tape_bitmap, font, 26, 0, statusbar_get_color(STATUSBAR_COLOR_WHITE), "%03d", ui_tape_counter);
    }
    destroy_bitmap(tape_bitmap);
    statusbar_update();
}

void ui_set_tape_status(int tape_status)
{
    ui_tape_enabled = tape_status;
    ui_draw_tape_status();
}

void ui_display_tape_motor_status(int motor)
{
    ui_tape_motor = motor;
    ui_draw_tape_status();
}

void ui_display_tape_control_status(int control)
{
    ui_tape_control = control;
    ui_draw_tape_status();
}


void ui_display_tape_counter(int counter)
{   
    if (counter == ui_tape_counter) {
        return;
    }

    ui_tape_counter = counter;
    ui_draw_tape_status();
}


void ui_display_tape_current_image(const char *image)
{
}

void ui_display_recording(int recording_status)
{
}

void ui_display_playback(int playback_status, char *version)
{
}

void ui_display_event_time(unsigned int current, unsigned int total)
{
}

void ui_display_joyport(uint8_t *joyport)
{
}

void ui_set_warp_status(int status)
{
    if (status) {
        kbd_led_status |= WARP_LED_MSK;
    
    } else {
        kbd_led_status &= ~WARP_LED_MSK;
    }
}

int ui_extend_image_dialog(void)
{
    int ret;

    enable_text();
    tui_clear_screen();

    ret = tui_ask_confirmation("Do you want to extend the disk image in drive 8?  (Y/N)");

    disable_text();

    return ret;
}

void ui_display_speed(float percent, float framerate, int warp_flag)
{
    BITMAP *speed_bitmap;
    int white = statusbar_get_color(STATUSBAR_COLOR_WHITE);

    if (status_bitmap == NULL) {
        return;
    }

    speed_bitmap = create_sub_bitmap(status_bitmap, 2, 2, 96, 8);

    if ((percent < 0) || (framerate < 0)) { 
        textout(speed_bitmap, font, "suspended", 0, 0, white);
    } else {
        textprintf(speed_bitmap, font, 0, 0, white, "%4d%%", (int)(percent + .5));
        textprintf(speed_bitmap, font, 44, 0, white, "%2dfps", (int)(framerate + .5));
        textprintf(speed_bitmap, font, 88, 0, white, "%s", warp_flag ? "W " : "  ");
    }
    destroy_bitmap(speed_bitmap);
    statusbar_update();
}

/* ------------------------------------------------------------------------- */

void ui_dispatch_events(void)
{
    /* Update keyboard LED status.  */
    if (kbd_led_status != real_kbd_led_status) {
        set_kbd_leds(kbd_led_status);
        real_kbd_led_status = kbd_led_status;
    }
}

void ui_dispatch_next_event(void)
{
}

void ui_display_statustext(const char *text, int fade_out)
{
}

void ui_display_volume(int vol)
{
}

char* ui_get_file(const char *format,...)
{
    return NULL;
}

void ui_pause_emulation(int flag)
{
}

int ui_emulation_is_paused(void)
{
    return 0;
}
