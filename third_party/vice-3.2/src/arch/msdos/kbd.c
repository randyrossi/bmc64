/*
 * kbd.c - MS-DOS keyboard driver.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#include <ctype.h>
#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <pc.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/farptr.h>
#include <time.h>

#include "cmdline.h"
#include "datasette.h"
#include "fliplist.h"
#include "interrupt.h"
#include "joy.h"
#include "kbd.h"
#include "keyboard.h"
#include "log.h"
#include "machine.h"
#include "mem.h"
#include "resources.h"
#include "statusbar.h"
#include "types.h"
#include "ui.h"
#include "vsync.h"

/* #define DEBUG_KBD */

/* ------------------------------------------------------------------------- */

/* Segment info for the standard keyboard handler.  */
static _go32_dpmi_seginfo std_kbd_handler_seginfo;

/* This takes account of the status of the modifier keys on the real PC
   keyboard.  */
static struct {
    unsigned int left_ctrl:1;
    unsigned int right_ctrl:1;
    unsigned int left_shift:1;
    unsigned int right_shift:1;
    unsigned int left_alt:1;
    unsigned int right_alt:1;
} modifiers;

/* Function for triggering cartridge (e.g. AR) freezing.  */
static void (*freeze_function)(void);

uint8_t _kbd_extended_key_tab[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, K_KPENTER, K_RIGHTCTRL, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, K_KPDIV, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, K_HOME, K_UP, K_PGUP, 0, K_LEFT, 0, K_RIGHT, 0, K_END,
    K_DOWN, K_PGDOWN, K_INS, K_DEL, 0, 0, 0, 0, 0, 0, 0, K_LEFTW95,
    K_RIGHTW95, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* ------------------------------------------------------------------------- */

/* These are the keyboard commands that cannot be handled within a keyboard
   interrupt.  They are dispatched via `kbd_flush_commands()'.  */

typedef enum {
    KCMD_MENU,
    KCMD_RESET,
    KCMD_HARD_RESET,
    KCMD_RESTORE_PRESSED,
    KCMD_RESTORE_RELEASED,
    KCMD_TOGGLE_WARP,
    KCMD_FREEZE,
    KCMD_FLIP_NEXT,
    KCMD_FLIP_PREVIOUS,
    KCMD_FLIP_ADD,
    KCMD_FLIP_REMOVE,
    KCMD_TOGGLE_STATUSBAR,
    KCMD_DATASETTE_START,
    KCMD_DATASETTE_STOP,
    KCMD_DATASETTE_FORWARD,
    KCMD_DATASETTE_REWIND,
    KCMD_DATASETTE_RECORD
} kbd_command_type_t;

typedef uint32_t kbd_command_data_t;

typedef struct {
    kbd_command_type_t type;
    kbd_command_data_t data;
} kbd_command_t;

#define MAX_COMMAND_QUEUE_SIZE 256
static kbd_command_t command_queue[MAX_COMMAND_QUEUE_SIZE];
static int num_queued_commands;

/* Add a command to the queue.  */
static void queue_command(kbd_command_type_t type, kbd_command_data_t data)
{
    if (num_queued_commands < MAX_COMMAND_QUEUE_SIZE) {
        int i = num_queued_commands++;

        command_queue[i].type = type;
        command_queue[i].data = data;
    }
}

static void queue_command_end(void)
{
}

/* CPU trap to enter the main menu.  */
static void menu_trap(uint16_t addr, void *data)
{
    ui_main((char)(unsigned long)data);
}

/* Dispatch all the pending keyboard commands.  */
void kbd_flush_commands(void)
{
    int i;

    if (num_queued_commands == 0) {
        return;
    }

    for (i = 0; i < num_queued_commands; i++) {
        switch (command_queue[i].type) {
            case KCMD_HARD_RESET:
                vsync_suspend_speed_eval();
                machine_trigger_reset(MACHINE_RESET_MODE_HARD);
                break;
            case KCMD_RESET:
                vsync_suspend_speed_eval();
                machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
                break;
            case KCMD_RESTORE_PRESSED:
                machine_set_restore_key(1);
                break;
            case KCMD_RESTORE_RELEASED:
                machine_set_restore_key(0);
                break;
            case KCMD_FREEZE:
                if (freeze_function != NULL) {
                    freeze_function();
                }
                break;
            case KCMD_FLIP_NEXT:
                fliplist_attach_head(8, 1);
                break;
            case KCMD_FLIP_PREVIOUS:
                fliplist_attach_head(8, 0);
                break;
            case KCMD_FLIP_ADD:
                fliplist_add_image(8);
                break;
            case KCMD_FLIP_REMOVE:
                fliplist_remove(-1, NULL);
                break;
            case KCMD_TOGGLE_WARP:
                resources_toggle("WarpMode", NULL);
                break;
            case KCMD_MENU:
                interrupt_maincpu_trigger_trap(menu_trap, (void *)command_queue[i].data);
                break;
            case KCMD_TOGGLE_STATUSBAR:
                if (statusbar_enabled()) {
                    resources_set_int("ShowStatusbar", STATUSBAR_MODE_OFF);
                } else {
                    resources_set_int("ShowStatusbar", STATUSBAR_MODE_ON);
                }
                break;
            case KCMD_DATASETTE_START:
                datasette_control(DATASETTE_CONTROL_START);
                break;
            case KCMD_DATASETTE_STOP:
                datasette_control(DATASETTE_CONTROL_STOP);
                break;
            case KCMD_DATASETTE_FORWARD:
                datasette_control(DATASETTE_CONTROL_FORWARD);
                break;
            case KCMD_DATASETTE_REWIND:
                datasette_control(DATASETTE_CONTROL_REWIND);
                break;
            case KCMD_DATASETTE_RECORD:
                datasette_control(DATASETTE_CONTROL_RECORD);
                break;
            default:
                log_error(LOG_DEFAULT, "Unknown keyboard command %d.", (int)command_queue[i].type);
        }
    }
    num_queued_commands = 0;
}

/* ------------------------------------------------------------------------- */

/* Convert a kcode to a printable ASCII character.  If not possible, return
   0.  Warning: this only works for the US layout, and does not handle the
   keypad correctly.  But for our current needs, this is more than enough.  */
static char kcode_to_ascii(unsigned int kcode)
{
    char conv_table[256] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,
        0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\n',
        0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0,
        '\\', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/', 0, '*', 0,
        ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0,
        0, 0, '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, '\n', 0, 0, 0, 0, 0, 0, 0, 0
    };

    return conv_table[kcode];
}

static void my_kbd_interrupt_handler(void)
{
    static int extended = 0;    /* Extended key count.  */
    static int skip_count = 0;
    unsigned int kcode;

    kcode = inportb(0x60);

    if (skip_count > 0) {
        skip_count--;
        outportb(0x20, 0x20);
        return;
    } else if (kcode == 0xe0) {
        /* Extended key: at the next interrupt we'll get its extended keycode
           or 0xe0 again.  */
        extended++;
        outportb(0x20, 0x20);
        return;
    } else if (kcode == 0xe1) {
        /* Damn Pause key.  It sends 0xe1 0x1d 0x52 0xe1 0x9d 0xd2.  This is
           awesome, but at least we know it's the only sequence starting by
           0xe1, so we can just skip the next 5 codes.  Btw, there is no
           release code.  */
        skip_count = 5;
        kcode = K_PAUSE;
    }

    if (!(kcode & 0x80)) {      /* Key pressed.  */
        /* Derive the extended keycode.  */
        if (extended == 1) {
            kcode = _kbd_extended_key_tab[kcode];
        }

        /* Handle modifiers.  */
        switch (kcode) {
            case K_LEFTCTRL:
                modifiers.left_ctrl = 1;
                break;
            case K_RIGHTCTRL:
                modifiers.right_ctrl = 1;
                break;
            case K_LEFTSHIFT:
                modifiers.left_shift = 1;
                break;
            case K_RIGHTSHIFT:
                modifiers.right_shift = 1;
                break;
            case K_LEFTALT:
                modifiers.left_alt = 1;
                break;
            case K_RIGHTALT:
                modifiers.right_alt = 1;
                break;
        }
        switch (kcode) {
            case K_ESC:           /* Menu */
                queue_command(KCMD_MENU, (kbd_command_data_t)0);
                break;
            case K_SCROLLOCK:     /* Warp mode on/off */
                queue_command(KCMD_TOGGLE_WARP, (kbd_command_data_t)0);
                break;
                /* Fall through */
            default:
                if (modifiers.left_alt && modifiers.left_ctrl) {
                    switch (kcode) {
                    case K_F12:
                        /* Ctrl-Alt-F12 does a hard reset. */
                        queue_command(KCMD_HARD_RESET, (kbd_command_data_t)0);
                        break;
                    case K_F4:
                        /* Ctrl-Alt-F4 is RECORD on the datasette. */
                        queue_command(KCMD_DATASETTE_RECORD, (kbd_command_data_t)0);
                        break;
                    case K_F5:
                        /* Ctrl-Alt-F5 is PLAY on the datasette. */
                        queue_command(KCMD_DATASETTE_START, (kbd_command_data_t)0);
                        break;
                    case K_F6:
                        /* Ctrl-Alt-F6 is REWIND on the datasette. */
                        queue_command(KCMD_DATASETTE_REWIND, (kbd_command_data_t)0);
                        break;
                    case K_F7:
                        /* Ctrl-Alt-F7 is FF on the datasette. */
                        queue_command(KCMD_DATASETTE_FORWARD, (kbd_command_data_t)0);
                        break;
                    case K_F8:
                        /* Ctrl-Alt-F8 is STOP on the datasette. */
                        queue_command(KCMD_DATASETTE_STOP, (kbd_command_data_t)0);
                        break;
                }
            } else if (modifiers.left_alt || modifiers.right_alt) {
                /* Handle Alt-... hotkeys.  */
                switch (kcode) {
                    case K_F12:
                        /* Alt-F12 does a reset.  */
                        queue_command(KCMD_RESET, (kbd_command_data_t)0);
                        break;
                    case K_F1:
                        /* Alt-F1 Next image in flip list.  */
                        queue_command(KCMD_FLIP_NEXT, (kbd_command_data_t)0);
                        break;
                    case K_F2:
                        /* Alt-F2 Previous image in flip list.  */
                        queue_command(KCMD_FLIP_PREVIOUS, (kbd_command_data_t)0);
                        break;
                    case K_F3:
                        /* Alt-F3 Add image to flip list.  */
                        queue_command(KCMD_FLIP_ADD, (kbd_command_data_t)0);
                        break;
                    case K_F4:
                        /* Alt-F4 remove image from flip list.  */
                        queue_command(KCMD_FLIP_REMOVE, (kbd_command_data_t)0);
                        break;
                    case K_F5:
                        /* Alt-F5 toggles statusbar.  */
                        queue_command(KCMD_TOGGLE_STATUSBAR, (kbd_command_data_t)0);
                        break;
                    case K_PAUSE:
                        /* Alt-Pause enables cartridge freezing.  */
                        queue_command(KCMD_FREEZE, (kbd_command_data_t)0);
                        break;
                    default:
                        /* Alt-{letter,number} enters the main menu.  */
                        if (isalnum((int)kcode_to_ascii(kcode))) {
                            queue_command(KCMD_MENU, (kbd_command_data_t)kcode_to_ascii(kcode));
                        }
                }
            } else {
                /* "Normal" key.  */
                keyboard_key_pressed((signed long)kcode);
            }
        }

    } else {                    /* Key released.  */
        /* Remove release bit.  */
        kcode &= 0x7f;

        /* Derive the extended keycode.  */
        if (extended == 1) {
            kcode = _kbd_extended_key_tab[kcode];
        }

        /* Handle modifiers.  */
        switch (kcode) {
            case K_LEFTCTRL:
                modifiers.left_ctrl = 0;
                break;
            case K_RIGHTCTRL:
                modifiers.right_ctrl = 0;
                break;
            case K_LEFTSHIFT:
                modifiers.left_shift = 0;
                break;
            case K_RIGHTSHIFT:
                modifiers.right_shift = 0;
                break;
            case K_LEFTALT:
                modifiers.left_alt = 0;
                break;
            case K_RIGHTALT:
                modifiers.right_alt = 0;
                break;
        }

        if (!modifiers.left_alt && !modifiers.right_alt) {
            /* "Normal" key.  */
            keyboard_key_released((signed long)kcode);
        }
    }

    extended = 0;
    outportb(0x20, 0x20);
}

static void my_kbd_interrupt_handler_end()
{
}

/* ------------------------------------------------------------------------- */

/* Install our custom keyboard interrupt.  */
void kbd_install(void)
{
    int r;
    static _go32_dpmi_seginfo my_kbd_handler_seginfo;

    my_kbd_handler_seginfo.pm_offset = (int)my_kbd_interrupt_handler;
    my_kbd_handler_seginfo.pm_selector = _go32_my_cs();
    r = _go32_dpmi_allocate_iret_wrapper(&my_kbd_handler_seginfo);
    if (r) {
        log_error(LOG_DEFAULT, "Cannot allocate IRET wrapper for the keyboard interrupt.");
        exit(-1);
    }

    r = _go32_dpmi_set_protected_mode_interrupt_vector(9, &my_kbd_handler_seginfo);
    if (r) {
        log_error(LOG_DEFAULT, "Cannot install the keyboard interrupt handler.");
        exit(-1);
    }

    /* Initialize the keyboard matrix.  */
    keyboard_clear_keymatrix();
    /* Reset modifier status.  */
    memset(&modifiers, 0, sizeof(modifiers));
}

/* Restore the standard keyboard interrupt.  */
void kbd_uninstall(void)
{
    int r;

    r = _go32_dpmi_set_protected_mode_interrupt_vector(9, &std_kbd_handler_seginfo);
    if (r) {
        log_error(LOG_DEFAULT, "Couldn't restore the standard kbd interrupt vector!");
    }
}

static void kbd_exit(void)
{
    kbd_uninstall();
}

static void kbd_init_common(void)
{
    _go32_dpmi_get_protected_mode_interrupt_vector(9, &std_kbd_handler_seginfo);
    atexit(kbd_exit);

    _go32_dpmi_lock_code(my_kbd_interrupt_handler, (unsigned long)my_kbd_interrupt_handler_end - (unsigned long)my_kbd_interrupt_handler);
    _go32_dpmi_lock_code(queue_command, (unsigned long)queue_command_end - (unsigned long)queue_command);

    _go32_dpmi_lock_data(keyarr, sizeof(keyarr));
    _go32_dpmi_lock_data(rev_keyarr, sizeof(rev_keyarr));
    _go32_dpmi_lock_data(&modifiers, sizeof(modifiers));
    _go32_dpmi_lock_data(&num_queued_commands, sizeof(num_queued_commands));
    _go32_dpmi_lock_data(&command_queue, sizeof(command_queue));

    num_queued_commands = 0;

    kbd_install();
}

/* ------------------------------------------------------------------------- */

void kbd_set_freeze_function(void (*f)())
{
    freeze_function = f;
}

/* ------------------------------------------------------------------------- */

const char *kbd_code_to_string(kbd_code_t kcode)
{
    static char *tab[256] = {
        "None", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-",
        "=", "Backspace", "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O",
        "P", "{", "}", "Enter", "Left Ctrl", "A", "S", "D", "F", "G", "H", "J",
        "K", "L", ";", "'", "`", "Left Shift", "\\", "Z", "X", "C", "V", "B",
        "N", "M", ",", ".", "/", "Right Shift", "Numpad *", "Left Alt",
        "Space", "Caps Lock", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
        "F9", "F10", "Num Lock", "Scroll Lock", "Numpad 7", "Numpad ",
        "Numpad 9", "Numpad -", "Numpad 4", "Numpad 5", "Numpad 6",
        "Numpad +", "Numpad 1", "Numpad 2", "Numpad 3", "Numpad 0",
        "Numpad .", "SysReq", "85", "86", "F11", "F12", "Home",
        "Up", "PgUp", "Left", "Right", "End", "Down", "PgDown", "Ins", "Del",
        "Numpad Enter", "Right Ctrl", "Pause", "PrtScr", "Numpad /",
        "Right Alt", "Break", "Left Win95", "Right Win95"
    };

    return tab[(int) kcode];
}

/* ------------------------------------------------------------------------ */

void kbd_arch_init(void)
{
    kbd_init_common();
}

signed long kbd_arch_keyname_to_keynum(char *keyname)
{
    return (signed long)atoi(keyname);
}

const char *kbd_arch_keynum_to_keyname(signed long keynum)
{
    static char keyname[20];

    memset(keyname, 0, 20);

    sprintf(keyname, "%li", keynum);

    return keyname;
}

void kbd_initialize_numpad_joykeys(int* joykeys)
{
    joykeys[0] = K_RIGHTCTRL;
    joykeys[1] = K_KP1;
    joykeys[2] = K_KP2;
    joykeys[3] = K_KP3;
    joykeys[4] = K_KP4;
    joykeys[5] = K_KP6;
    joykeys[6] = K_KP7;
    joykeys[7] = K_KP8;
    joykeys[8] = K_KP9;
}

/* returns host keyboard mapping. used to initialize the keyboard map when
   starting with a black (default) config, so an educated guess works good
   enough most of the time :)

   FIXME: add more languages/actual detection
*/
int kbd_arch_get_host_mapping(void)
{
    return KBD_MAPPING_US;
}
