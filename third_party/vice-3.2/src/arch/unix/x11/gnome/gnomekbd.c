/*
 * gnomekbd.c - GTK Keyboard driver
 *
 * Written by
 *  Oliver Schaertel
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
 * GTK Keyboard driver
 */

/* #define DEBUG_X11UI */
/* #define DEBUG_KBD */
/* #define DEBUGNOKBDGRAB */    /* dont explicitly grab keyboard focus */

#include "vice.h"

#include <string.h>             /* memset() */
#include <X11/XKBlib.h>

#include "uiarch.h"

#include "kbd.h"
#include "keyboard.h"
#include "machine.h"

#include "gnomekbd.h"


#ifdef DEBUG_X11UI
#define DBG(_x_) log_debug _x_
#else
#define DBG(_x_)
#endif

void kbd_arch_init(void)
{
}

signed long kbd_arch_keyname_to_keynum(char *keyname)
{
    guint sym = gdk_keyval_from_name(keyname);

    if (sym == KEYSYM_VoidSymbol) {
        return -1;
    }

    return (signed long)sym;
}

const char *kbd_arch_keynum_to_keyname(signed long keynum)
{
    return gdk_keyval_name((guint)keynum);
}

static gboolean kbd_event_handler(GtkWidget *w, GdkEvent *report, gpointer gp)
{
    gint key;

    key = report->key.keyval;
    switch (report->type) {
        case GDK_KEY_PRESS:
#ifdef DEBUG_KBD
            log_debug("KeyPress`%d'.", key);
#endif
            keyboard_key_pressed((signed long)key);
            return TRUE;
        case GDK_KEY_RELEASE:
#ifdef DEBUG_KBD
            log_debug("KeyRelese`%d'.", key);
#endif
            if (key == KEYSYM_Shift_L || key == KEYSYM_Shift_R || 
                key == KEYSYM_ISO_Level3_Shift ||
                /* the following checks are an ugly workaround for bug #549
                    for some reasond gdk returns different keycodes
                    for press and release events of the shift keys.
                    any explanation would be helpful
                    */
                key == KEYSYM_Shift_Lrel || key == KEYSYM_Shift_Rrel) {
                keyboard_key_clear();
            }
            keyboard_key_released(key);
            break;
        case GDK_ENTER_NOTIFY:
        case GDK_LEAVE_NOTIFY:
        case GDK_FOCUS_CHANGE:
            keyboard_key_clear();
            break;
        default:
            break;
    }                           /* switch */
    return FALSE;
}

void kbd_connect_handler(GtkWidget *widget, void *data)
{
    g_signal_connect(G_OBJECT(widget), "key-press-event", G_CALLBACK(kbd_event_handler), data);
    g_signal_connect(G_OBJECT(widget), "key-release-event", G_CALLBACK(kbd_event_handler), data);
}

void kbd_connect_enterleave_handler(GtkWidget *widget, void *data)
{
    g_signal_connect(G_OBJECT(widget), "enter-notify-event", G_CALLBACK(kbd_event_handler), data);
    g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(kbd_event_handler), data);
}

void kbd_initialize_numpad_joykeys(int* joykeys)
{
    joykeys[0] = KEYSYM_KP_0;
    joykeys[1] = KEYSYM_KP_1;
    joykeys[2] = KEYSYM_KP_2;
    joykeys[3] = KEYSYM_KP_3;
    joykeys[4] = KEYSYM_KP_4;
    joykeys[5] = KEYSYM_KP_6;
    joykeys[6] = KEYSYM_KP_7;
    joykeys[7] = KEYSYM_KP_8;
    joykeys[8] = KEYSYM_KP_9;
}

/* returns host keyboard mapping. used to initialize the keyboard map when
   starting with a black (default) config */

/* FIXME: add more languages, then copy to x11kbd.c */
int kbd_arch_get_host_mapping(void)
{

    int n;
    int maps[KBD_MAPPING_NUM] = {
        KBD_MAPPING_US, KBD_MAPPING_UK, KBD_MAPPING_DE, KBD_MAPPING_DA,
        KBD_MAPPING_NO, KBD_MAPPING_FI, KBD_MAPPING_IT };
    char str[KBD_MAPPING_NUM][3] = {
        "us", "uk", "de", "da", "no", "fi", "it"};

    Display* _display;
    char* displayName = "";
    _display = XOpenDisplay(displayName);

    XkbDescRec* _kbdDescPtr = XkbAllocKeyboard();
    XkbGetNames(_display, XkbSymbolsNameMask, _kbdDescPtr);
    Atom symName = _kbdDescPtr -> names -> symbols;
    char* layoutString = XGetAtomName(_display, symName);

    XCloseDisplay(_display);
    char *p = layoutString;

    if (memcmp(p, "pc+", 3) == 0) {
        p += 3;
        if (p && (strlen(p) > 1)) {
            for (n = 1; n < KBD_MAPPING_NUM; n++) {
                if (memcmp(p, str[n], 2) == 0) {
                    return maps[n];
                }
            }
        }
    }
    return KBD_MAPPING_US;
}

#if 0
/*
    grab keyboard focus

    called by: ui_init_finalize
 */
static int keyboard_grabbed = 0;
static void keyboard_grab(int grab)
{
#ifdef DEBUGNOKBDGRAB
    DBG(("keyboard_grab disabled (%d)", grab));
#else
    GtkWidget *widget;
    GdkWindow *window;

    DBG(("keyboard_grab (%d, was %d)", grab, keyboard_grabbed));

    if (grab == keyboard_grabbed) {
        return;
    }

    /*ui_dispatch_events();
    gdk_flush();*/

    if (grab) {
        widget = get_active_toplevel();
        window = widget ? widget->window : NULL;

        if ((widget == NULL) || (window == NULL)) {
            log_error(ui_log, "keyboard_grab: bad params");
            return;
        }

        gdk_keyboard_grab(window, 1, GDK_CURRENT_TIME);
        keyboard_grabbed = 1;
    } else {
        gdk_keyboard_ungrab(GDK_CURRENT_TIME);
        keyboard_grabbed = 0;
    }
#endif
}
#endif
