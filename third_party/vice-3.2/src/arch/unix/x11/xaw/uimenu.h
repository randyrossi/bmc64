/*
 * uimenu.h - Simple and ugly cascaded pop-up menu implementation.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *
 * Support for multiple visuals and depths by
 *  Teemu Rantanen <tvr@cs.hut.fi>
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

#ifndef VICE_UIMENU_H
#define VICE_UIMENU_H

#include "ui.h"
#include "uiarch.h"

#ifdef VMS
#include "x11menu.h"
#else

/* The ".." will go away once the UI stuff is separated.  */
#include "../x11menu.h"
#endif

extern struct ui_menu_entry_s ui_menu_separator[];

extern int ui_menu_init(XtAppContext app_context, Display *display, int screen);
extern void ui_menu_set_sensitive(Widget w, int flag);
extern void ui_menu_set_tick(Widget w, int flag);
extern int ui_menu_any_open(void);
extern void ui_menu_update_all(void);
extern Widget ui_menu_create(const char *name, ...);
extern void uimenu_shutdown(void);
extern int ui_dispatch_hotkeys(int key);
extern void ui_menu_delete(Widget widget);

/* ------------------------------------------------------------------------- */

/* These are the callbacks for the toggle and radio menus (the ones with a
   checkmark on the left).  If call_data is NULL, they simply set/unset the
   checkmark according to the value of the corrisponding resource.  If not
   NULL, they set the value of the corresponding resource before doing so.
   For this reason, to update the checkmarks, we simply have to call all the
   callbacks with a NULL `call_data' parameter.  */

#define UI_MENU_DEFINE_TOGGLE(resource)                               \
    static UI_CALLBACK(toggle_##resource)                             \
    {                                                                 \
        _ui_menu_toggle_helper(w, client_data, call_data, #resource); \
    }

#define UI_MENU_DEFINE_RADIO(resource)                               \
    static UI_CALLBACK(radio_##resource)                             \
    {                                                                \
        _ui_menu_radio_helper(w, client_data, call_data, #resource); \
    }

#define UI_MENU_DEFINE_STRING_RADIO(resource)                               \
    static UI_CALLBACK(radio_##resource)                                    \
    {                                                                       \
        _ui_menu_string_radio_helper(w, client_data, call_data, #resource); \
    }

#define UI_MENU_DEFINE_TOGGLE_COND(name, res, comp)               \
    static UI_CALLBACK(toggle_##name)                             \
    {                                                             \
        int val;                                                  \
                                                                  \
        resources_get_int(#res, &val);                            \
                                                                  \
        if (comp(val)) {                                          \
            ui_menu_set_sensitive(w, True);                       \
        } else {                                                  \
            ui_menu_set_sensitive(w, False);                      \
        }                                                         \
                                                                  \
        _ui_menu_toggle_helper(w, client_data, call_data, #name); \
    }

/* Private helper functions for toggle and radio menu items.  */
extern void _ui_menu_toggle_helper(Widget w, ui_callback_data_t client_data, ui_callback_data_t call_data, const char *resource_name);
extern void _ui_menu_radio_helper(Widget w, ui_callback_data_t client_data, ui_callback_data_t call_data, const char *resource_name);
extern void _ui_menu_string_radio_helper(Widget w, ui_callback_data_t client_data, ui_callback_data_t call_data, const char *resource_name);

extern void ui_about_shutdown(void);

#endif /* _UIMENU_H */
