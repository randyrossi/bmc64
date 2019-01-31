/*
 * uimenu.c - Simple and ugly cascaded pop-up menu implementation.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Oliver Schaertel GTK+ port
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

/* Warning: this code sucks.  It does work, but it sucks.  */

/* #define DEBUG_MENUS */
/* #define DEBUG_HOTKEYS */ /* undefine to get a list of all hotkeys at startup */

#include "vice.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "lib.h"
#include "log.h"
#include "machine.h"
#include "resources.h"
#include "uiapi.h"
#include "uimenu.h"
#include "util.h"
#include "x11menu.h"

#include "uimenu.h"

#ifdef DEBUG_MENUS
#define DBG(_x_)  log_debug _x_
#else
#define DBG(_x_)
#endif

#ifdef DEBUG_HOTKEYS
#define DBGHK(_x_)  log_debug _x_
#else
#define DBGHK(_x_)
#endif

/* Separator item.  */
ui_menu_entry_t ui_menu_separator[] = {
    UI_MENU_ENTRY_SEPERATOR,
    UI_MENU_ENTRY_LIST_END
};

static int menu_popup = 0;

#if 0
#define MAX_SUBMENUS 1024
static struct {
    GtkWidget* widget;
    GtkWidget* parent;
    int level;
} submenus[MAX_SUBMENUS];
#endif

/* This keeps a list of the menus with a checkmark on the left.  Each time
   some setting is changed, we have to update them. 
   This is also used for submenus which have to be blocked - the condition for this 
   is somewhere coded and not in resources (e.g. drive expansions depending
   on drive types */
#define MAX_UPDATE_MENU_LIST_SIZE 1024
typedef struct {
    char *name;
    GtkWidget *w;
    ui_callback_t cb;
    gint handlerid;
    ui_menu_cb_obj obj;
} checkmark_t;

/** \brief  List of checkmark objects
 */
static GList *checkmark_list = NULL;

/** \brief  List of callback object
 */
static GList *object_list = NULL;


int num_checkmark_menu_items = 0; /* !static because vsidui needs it. ugly! */

/* list of all hotkeys for sanity checking */
#define MAX_HOTKEYS     50
typedef struct {
    char *name;
    guint key;
    ui_hotkey_modifier_t mod;
} hotkey_t;

static int numhotkeys = 0;
static hotkey_t hotkeys[MAX_HOTKEYS];

/* ------------------------------------------------------------------------- */


/** \brief  Module initialization function
 *
 * \note    Initializing the hotkeys array is not needed since ISO C guarantees
 *          static objects to get initialized to 0/NULL, including aggregate
 *          types
 *
 * \return  0 on success (always for now)
 */
int ui_menu_init(void)
{

    return 0;
}

/** \brief  Module shutdown function
 *
 * Frees memory used by hotkey names, checkmarks, menu callback objects. For
 * some reason the "destroy" signal handler of the checkmarks never gets called
 * so we clean up like this.
 *
 * A cleaner way would be the event handler way, but I have no idea why they
 * aren't called (BW)
 *
 * FIXME:   The code setting stuff to NULL/0 is required because VSID uses an
 *          ugly hack to create its 'tunes' submenu: the entire menu is
 *          recreated each time a PSID is loaded.
 */
void ui_menu_shutdown(void)
{
    GList *list;
    int i;

    /* free hotkey names */
    for (i = 0; i < MAX_HOTKEYS; i++) {
        if (hotkeys[i].name != NULL) {
            lib_free(hotkeys[i].name);
            hotkeys[i].name = NULL;
        }
    }
    numhotkeys = 0;

    /* free checkmarks */
    list = checkmark_list;
    while (list != NULL) {
        GList *next = list->next;
        checkmark_t *cm = (checkmark_t *)list->data;
        lib_free(cm->name);
        lib_free(cm);
        list = next;
    }
    g_list_free(checkmark_list);
    checkmark_list = NULL;

    /* free menu objects */
    list = object_list;
    while (list != NULL) {
        GList *next = list->next;
        ui_menu_cb_obj *obj = (ui_menu_cb_obj *)list->data;
        lib_free(obj);
        list = next;
    }
    g_list_free(object_list);
    object_list = NULL;
}

static void delete_checkmark_cb(GtkWidget *w, gpointer data)
{
    checkmark_t *cm;

    /* For some obscure reason `lib_free(cm->name)` causes a segfault when
     * trying to load a PSID file via 'Load' in the VSID UI. When disabling the
     * checkmark free code in ui_shutdown(), we get hundreds of leaks, so for now
     * ui_shutdown() will have to handle the freeing of the checkmarks.
     * Ugly and weird, but since we're moving to Gtk3, this will have to do.
     *
     * -- compyx, 2017-12-26
     */
    return;

    /* printf("delete_checkmark_cb() called\n"); */
    cm = (checkmark_t *)data;
    checkmark_list = g_list_remove(checkmark_list, data);
    lib_free(cm->name);
    lib_free(cm);
}

static void add_accelerator(const char *name, GtkWidget *w, GtkAccelGroup *accel, guint accel_key, ui_hotkey_modifier_t mod)
{
    GdkModifierType flags = 0;
    int i, f;

    /* first do sanity checks and warn about hotkeys that are redefined (which
       seems to be a common error :)) */
    f = 0;
    for (i = 0; i < numhotkeys; ++i) {
        if ((hotkeys[i].key == accel_key) && (hotkeys[i].mod == mod)) {
            f = 1;
            if (strcmp(name, hotkeys[i].name) != 0) {
                log_error(LOG_DEFAULT, "add_accelerator: hotkey idx:%2d key:%04x '%c' mod:%04x redefined from '%s' to '%s'.", i, accel_key, accel_key, mod, hotkeys[i].name, name);
                f = 2;
            }
        }
    }
    if (f == 0) {
        if (numhotkeys == MAX_HOTKEYS) {
            log_error(LOG_DEFAULT, "add_accelerator: too many hotkeys, increase MAX_HOTKEYS");
        } else {
            hotkeys[numhotkeys].key = accel_key;
            hotkeys[numhotkeys].mod = mod;
            hotkeys[numhotkeys].name = lib_stralloc(name);
            DBGHK(("add_accelerator: new hotkey idx:%2d key:%04x '%c' mod:%04x (%s)", numhotkeys, accel_key, accel_key, mod, name));
            ++numhotkeys;
        }
    }

    /* actually add the hotkey as accelerator */
    if (mod & UI_HOTMOD_CONTROL) {
        flags |= GDK_CONTROL_MASK;
    }
    if (mod & UI_HOTMOD_META) {
        flags |= GDK_MOD1_MASK;
    }
    if (mod & UI_HOTMOD_SHIFT) {
        flags |= GDK_SHIFT_MASK;
    }

    gtk_widget_add_accelerator(w, "activate", accel, accel_key, flags, GTK_ACCEL_VISIBLE);
}

static char *make_menu_label(ui_menu_entry_t *e)
{
    char *trans;
    char *dots = "";
    char *retstr;

    /* Check wether NO_TRANS prefix is there, if yes don't translate it */
    if (strncmp(e->string, NO_TRANS, strlen(NO_TRANS)) == 0) {
        trans = lib_stralloc(e->string + strlen(NO_TRANS));
    } else {
        trans = lib_stralloc(_(e->string));
    }

    if (e->type == UI_MENU_TYPE_DOTS || e->type == UI_MENU_TYPE_TICKDOTS) {
        dots = "...";
    }

    retstr = util_concat(trans, dots, NULL);
    lib_free(trans);

    return retstr;
}

void ui_menu_create(GtkWidget *w, GtkAccelGroup *accel, const char *menu_name, ui_menu_entry_t *list)
{
    static int level = 0;
    unsigned int i;
    level++;

    DBG(("ui_menu_create: allocate new: %s\t(%p)\t%s", gtk_type_name(GTK_WIDGET_TYPE(w)), w, menu_name));

    for (i = 0; list[i].string; i++) {
        GtkWidget *new_item = NULL;
        int do_right_justify = 0;
        int update_item = 0;

        switch (list[i].type) {
            case UI_MENU_TYPE_SEPARATOR:    /* line */
                new_item  = gtk_menu_item_new();
                break;
            case UI_MENU_TYPE_NONE:
                break;
            case UI_MENU_TYPE_TICK:         /* toggle */
            case UI_MENU_TYPE_TICKDOTS:     /* toggle */
                {
                    char *label = make_menu_label(&list[i]);

                    if (list[i].callback) {
                        new_item = gtk_check_menu_item_new_with_label(label);
                        update_item = 1;
                    } else {
                        log_error(LOG_DEFAULT, "checkbox without callback: %s", label);
                    }
                    lib_free(label);
                    break;
                }
            case UI_MENU_TYPE_BL_SUB: /* callback to block/unblock menu needed */
                update_item = 1;
                /* fall through */
            default:
                {
                    char *item, *itemp;

                    item = itemp = make_menu_label(&list[i]);
                    if (strncmp(item, "RJ", 2) == 0) {
                        do_right_justify = 1;
                        item += 2;
                    }
                    new_item = gtk_menu_item_new_with_label(item);
                    lib_free(itemp);
                }
        }

        if (new_item) {
            if (list[i].callback) {
                if (update_item) {
                    checkmark_t *cmt;
                    cmt = lib_malloc(sizeof(checkmark_t));
                    cmt->name = lib_stralloc(list[i].string);
                    cmt->w = new_item;
                    cmt->cb = list[i].callback;
                    cmt->obj.value = (void*)list[i].callback_data;
                    cmt->obj.status = CB_NORMAL;
                    cmt->handlerid = g_signal_connect(
                            G_OBJECT(new_item),
                            "activate",
                            G_CALLBACK(list[i].callback),
                            (gpointer)&(cmt->obj));
                    /* never got triggered for some reason, now recent gtk it gets triggered */
                    g_signal_connect(
                            G_OBJECT(new_item),
                            "destroy",
                            G_CALLBACK(delete_checkmark_cb),
                            (gpointer)cmt);
                    /* Add this item to the list of calls to perform to update the
                    menu status. e.g. checkmarks or submenus */
                    checkmark_list = g_list_prepend(checkmark_list, cmt);
                } else {
                    ui_menu_cb_obj *obj;
                    obj = lib_malloc(sizeof(ui_menu_cb_obj));
                    obj->value = (void*)list[i].callback_data;
                    obj->status = CB_NORMAL;
                    g_signal_connect(
                            G_OBJECT(new_item),
                            "activate",
                            G_CALLBACK(list[i].callback),
                            (gpointer)obj);
                    object_list = g_list_prepend(object_list, obj);
                }
            }

            gtk_menu_shell_append(GTK_MENU_SHELL(w), new_item);
            gtk_widget_show(new_item);
            if (do_right_justify) {
                gtk_menu_item_set_right_justified(GTK_MENU_ITEM(new_item), TRUE);
            }

            DBG(("ui_menu_create: allocate new: %s\t(%p)\t%s", gtk_type_name(GTK_WIDGET_TYPE(new_item)), new_item, list[i].string));
        }

        if (list[i].sub_menu) {
            GtkWidget *sub;
            if (new_item && (list[i].type != UI_MENU_TYPE_SEPARATOR)) {
                sub = gtk_menu_new();
                gtk_menu_item_set_submenu(GTK_MENU_ITEM(new_item), sub);
            } else {
                sub = w;
            }
            ui_menu_create(sub, accel, list[i].string, list[i].sub_menu);
        } else {            /* no submenu */
            if (accel && list[i].hotkey_keysym != KEYSYM_NONE && list[i].callback != NULL && new_item != NULL) {
                add_accelerator(list[i].string, new_item, accel, list[i].hotkey_keysym, list[i].hotkey_modifier);
            }
        }
    }

    level--;
}

int ui_menu_any_open(void)
{
    return menu_popup;
}

static void menu_handle_block(gpointer data, gpointer user_data)
{
    checkmark_t *cm = (checkmark_t *)data;

    if (user_data) {
        g_signal_handler_block(G_OBJECT(cm->w), cm->handlerid);
    } else {
        g_signal_handler_unblock(G_OBJECT(cm->w), cm->handlerid);
    }
}

static void menu_update_checkmarks(gpointer data, gpointer user_data)
{
    checkmark_t *cm = (checkmark_t *)data;

    cm->obj.status = CB_REFRESH;
    ((void*(*)(GtkWidget*, ui_callback_data_t))cm->cb)(cm->w, (ui_callback_data_t) &cm->obj);
    cm->obj.status = CB_NORMAL;
}

void ui_menu_update_all_GTK(void)
{
    g_list_foreach(checkmark_list, menu_handle_block, (gpointer) 1);
    g_list_foreach(checkmark_list, menu_update_checkmarks, NULL);
    ui_dispatch_events();
    g_list_foreach(checkmark_list, menu_handle_block, (gpointer) 0);
}

void ui_menu_update_all(void)
{
}

void ui_menu_set_tick(GtkWidget *w, int flag) {
    if (GTK_IS_CHECK_MENU_ITEM(w)) {
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(w), flag != 0);
    }
}

void ui_menu_set_sensitive(GtkWidget *w, int flag)
{
    gtk_widget_set_sensitive(w, flag);
}

/* ------------------------------------------------------------------------- */

/* These functions are called by radio and toggle menu items if the callback
   functions are defined through `UI_MENU_DEFINE_TOGGLE()',
   `UI_MENU_DEFINE_RADIO()' or `UI_MENU_DEFINE_STRING_RADIO()'.  */

void _ui_menu_toggle_helper(GtkWidget *w, ui_callback_data_t event_data, const char *resource_name)
{
    int current_value;

    if (resources_get_int(resource_name, &current_value) < 0) {
        return;
    }

    if (!CHECK_MENUS) {
        resources_set_int(resource_name, !current_value);
        ui_update_menus();
    } else {
        ui_menu_set_tick(w, current_value);
    }
}

void _ui_menu_radio_helper(GtkWidget *w, ui_callback_data_t event_data, const char *resource_name)
{
    int current_value;

    resources_get_int(resource_name, &current_value);

    if (!CHECK_MENUS) {
        if (current_value != vice_ptr_to_int(UI_MENU_CB_PARAM)) {
            resources_set_int(resource_name, vice_ptr_to_int(UI_MENU_CB_PARAM));
            ui_update_menus();
        }
    } else {
        ui_menu_set_tick(w, current_value == vice_ptr_to_int(UI_MENU_CB_PARAM));
    }
}

void _ui_menu_string_radio_helper(GtkWidget *w, ui_callback_data_t event_data, const char *resource_name)
{
    const char *current_value;

    resources_get_string(resource_name, &current_value);

    if (current_value == NULL) {
        return;
    }

    if (!CHECK_MENUS) {
        if (strcmp(current_value, (const char *)UI_MENU_CB_PARAM) != 0) {
            resources_set_string(resource_name,(const char *)UI_MENU_CB_PARAM);
            ui_update_menus();
        }
    } else {
        ui_menu_set_tick(w, strcmp(current_value, (const char *)UI_MENU_CB_PARAM) == 0);
    }
}
