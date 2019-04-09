/** \file   kbd.c
 * \brief   Native GTK3 UI keyboard stuff
 *
 * \author  Marco van den Heuvel <blackystardust68@yahoo.com>
 * \author  Michael C. Martin <mcmartin@gmail.com>
 * \author  Oliver Schaertel
 * \author  pottendo <pottendo@gmx.net>
 * \author  Bas Wassink <b.wassink@ziggo.nl>
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

#include "vice.h"

#include <stdio.h>
#include <gtk/gtk.h>
#include "debug_gtk3.h"
#include "lib.h"
#include "log.h"
#include "ui.h"

/* UNIX-specific; for kbd_arch_get_host_mapping */
#include <locale.h>
#include <string.h>


#include "keyboard.h"
#include "kbd.h"


static gboolean kbd_hotkey_handle(GdkEvent *report);


/** \brief  Initial size of the hotkeys array
 */
#define HOTKEYS_SIZE_INIT   64


/** \brief  List of custom hotkeys
 */
static kbd_gtk3_hotkey_t *hotkeys_list = NULL;


/** \brief  Size of the hotkeys array
 *
 * This will be HOTKEYS_SIZE_INIT element after initializing and will grow
 * by doubling its size when the array is full.
 */
static int hotkeys_size = 0;


/** \brief  Number of registered hotkeys
 */
static int hotkeys_count = 0;




int kbd_arch_get_host_mapping(void)
{
    int n;
    char *l;
    int maps[KBD_MAPPING_NUM] = {
        KBD_MAPPING_US, KBD_MAPPING_UK, KBD_MAPPING_DE, KBD_MAPPING_DA,
        KBD_MAPPING_NO, KBD_MAPPING_FI, KBD_MAPPING_IT };
    /* TODO: This is a UNIX-specific version lifted from the SDL
     * implementation. */
    char str[KBD_MAPPING_NUM][6] = {
        "en_US", "en_UK", "de", "da", "no", "fi", "it"};
    setlocale(LC_ALL, "");
    l = setlocale(LC_ALL, NULL);
    if (l && (strlen(l) > 1)) {
        for (n = 1; n < KBD_MAPPING_NUM; n++) {
            if (strncmp(l, str[n], strlen(str[n])) == 0) {
                return maps[n];
            }
        }
    }
    return KBD_MAPPING_US;
}


/** \brief  Initialize keyboard handling
 */
void kbd_arch_init(void)
{
    /* do NOT call kbd_hotkey_init(), keyboard.c calls this function *after*
     * the UI init stuff is called, allocating the hotkeys array again and thus
     * causing a memory leak
     */
}



void kbd_arch_shutdown(void)
{
    /* Also don't call kbd_hotkey_shutdown() here */
}


signed long kbd_arch_keyname_to_keynum(char *keyname)
{
    guint sym = gdk_keyval_from_name(keyname);

    if (sym == GDK_KEY_VoidSymbol) {
        return -1;
    }

    return (signed long)sym;
}

const char *kbd_arch_keynum_to_keyname(signed long keynum)
{
    return gdk_keyval_name((guint)keynum);
}

void kbd_initialize_numpad_joykeys(int *joykeys)
{
    joykeys[0] = GDK_KEY_KP_0;
    joykeys[1] = GDK_KEY_KP_1;
    joykeys[2] = GDK_KEY_KP_2;
    joykeys[3] = GDK_KEY_KP_3;
    joykeys[4] = GDK_KEY_KP_4;
    joykeys[5] = GDK_KEY_KP_6;
    joykeys[6] = GDK_KEY_KP_7;
    joykeys[7] = GDK_KEY_KP_8;
    joykeys[8] = GDK_KEY_KP_9;
}

static gboolean kbd_event_handler(GtkWidget *w, GdkEvent *report, gpointer gp)
{
    gint key;

    key = report->key.keyval;
    switch (report->type) {
        case GDK_KEY_PRESS:
            /* fprintf(stderr, "KeyPress: %d.\n", key); */
            if (gtk_window_activate_key(GTK_WINDOW(w), (GdkEventKey *)report)) {
                return TRUE;
            }
            /* For some reason, the Alt-D of going fullscreen doesn't
             * return true when CAPS LOCK isn't on, but only it does
             * this. */
            if (key == GDK_KEY_d && report->key.state & GDK_MOD1_MASK) {
                return TRUE;
            }

            /* check the custom hotkeys */
            if (kbd_hotkey_handle(report)) {
                return TRUE;
            }

#if 0
            if ((key == GDK_KEY_p || key == GDK_KEY_P)
                    && (report->key.state & GDK_MOD1_MASK)) {
                debug_gtk3("Got Alt+P");
                ui_toggle_pause();
                return TRUE;
            }
#endif

            keyboard_key_pressed((signed long)key);
            return TRUE;
        case GDK_KEY_RELEASE:
            /* fprintf(stderr, "KeyRelease: %d.\n", key); */
            if (key == GDK_KEY_Shift_L || key == GDK_KEY_Shift_R || 
                key == GDK_KEY_ISO_Level3_Shift) {
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

void kbd_connect_handlers(GtkWidget *widget, void *data)
{
    g_signal_connect(G_OBJECT(widget), "key-press-event", G_CALLBACK(kbd_event_handler), data);
    g_signal_connect(G_OBJECT(widget), "key-release-event", G_CALLBACK(kbd_event_handler), data);
    g_signal_connect(G_OBJECT(widget), "enter-notify-event", G_CALLBACK(kbd_event_handler), data);
    g_signal_connect(G_OBJECT(widget), "leave-notify-event", G_CALLBACK(kbd_event_handler), data);
}

/*
 * Hotkeys (keyboard shortcuts not connected to any GtkMenuItem) handling
 */


/** \brief  Initialize the hotkeys
 *
 * This allocates an initial hotkeys array of HOTKEYS_SIZE_INIT elements
 */
void kbd_hotkey_init(void)
{
    debug_gtk3("initializing hotkeys list.");
    hotkeys_list = lib_malloc(HOTKEYS_SIZE_INIT * sizeof *hotkeys_list);
    hotkeys_size = HOTKEYS_SIZE_INIT;
    hotkeys_count = 0;
}



/** \brief  Clean up memory used by the hotkeys array
 */
void kbd_hotkey_shutdown(void)
{
    debug_gtk3("cleaning up memory used by the hotkeys.");
    lib_free(hotkeys_list);
}


/** \brief  Find hotkey index
 *
 * \param[in]   code    key code
 * \param[in]   mask    key mask
 *
 * \return  index in list, -1 when not found
 */
static int kbd_hotkey_get_index(guint code, guint mask)
{
    int i = 0;

    while (i < hotkeys_count) {
        if (hotkeys_list[i].code == code && hotkeys_list[i].mask) {
            return i;
        }
        i++;
    }
    return -1;
}


/** \brief  Look up the requested hotkey and trigger its callback when found
 *
 * \param[in]   report  GDK key press event instance
 *
 * \return  TRUE when the key was found and the callback triggered,
 *          FALSE otherwise
 */
static gboolean kbd_hotkey_handle(GdkEvent *report)
{
    int i = 0;
    gint code = report->key.keyval;

    while (i < hotkeys_count) {
        if ((hotkeys_list[i].code == code)
                && (report->key.state & hotkeys_list[i].mask)) {

            debug_gtk3("triggering callback of hotkey with index %d.", i);
            hotkeys_list[i].callback();
            return TRUE;
        }
        i++;
    }
    return FALSE;
}


/** \brief  Add hotkey to the list
 *
 * \param[in]   code        GDK key code
 * \param[in]   mask        GDK key modifier bitmask
 * \param[in]   callback    function to call when hotkey is triggered
 *
 * \return  bool
 */
gboolean kbd_hotkey_add(guint code, guint mask, void (*callback)(void))
{
    if (callback == NULL) {
        log_error(LOG_ERR, "Error: NULL passed as callback.");
        return FALSE;
    }
    if (kbd_hotkey_get_index(code, mask) >= 0) {
        log_error(LOG_ERR, "Error: hotkey already registered.");
        return FALSE;
    }

    /* resize list? */
    if (hotkeys_count == hotkeys_size) {
        int new_size = hotkeys_size * 2;
        debug_gtk3("Resizing hotkeys list to %d items.", new_size);
        hotkeys_list = lib_realloc(
                hotkeys_list,
                (size_t)new_size * sizeof *hotkeys_list);
        hotkeys_size = new_size;
    }


    /* register hotkey */
    hotkeys_list[hotkeys_count].code = code;
    hotkeys_list[hotkeys_count].mask = mask;
    hotkeys_list[hotkeys_count].callback = callback;
    hotkeys_count++;
    return TRUE;
}


/** \brief  Add multiple hotkeys at once
 *
 * Adds multiple hotkeys from \a list. Terminate the list with NULL for the
 * callback value.
 *
 * \param[in]   list    list of hotkeys
 *
 * \return  TRUE on success, FALSE if the list was exhausted or a hotkey
 *          was already registered
 */
gboolean kbd_hotkey_add_list(kbd_gtk3_hotkey_t *list)
{
    int i = 0;

    while (list[i].callback != NULL) {
        if (!kbd_hotkey_add(list[i].code, list[i].mask, list[i].callback)) {
            return FALSE;
        }
        i++;
    }
    return TRUE;
}
