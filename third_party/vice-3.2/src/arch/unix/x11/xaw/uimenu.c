/*
 * uimenu.c - Simple and ugly cascaded pop-up menu implementation.
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

/* Warning: this code sucks.  It does work, but it sucks.  */

#include "vice.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

/* Xaw or Xaw3d */
#ifdef USE_XAW3D
#include <X11/Xaw3d/SimpleMenu.h>
#include <X11/Xaw3d/SmeLine.h>
#include <X11/Xaw3d/SmeBSB.h>
#else
#include <X11/Xaw/SimpleMenu.h>
#include <X11/Xaw/SmeLine.h>
#include <X11/Xaw/SmeBSB.h>
#endif

#ifdef HAVE_X11_INTRINSICI_H
#include <X11/IntrinsicI.h>
#else
#include <X11/IntrinsicP.h>
#endif

#include "checkmark.xbm"
#include "right_arrow.xbm"

#include "fullscreenarch.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "resources.h"
#include "uiapi.h"
#include "uiarch.h"
#include "uimenu.h"
#include "x11menu.h"
#include "util.h"

#define MENU_DEBUG      0

/* Separator item.  */
ui_menu_entry_t ui_menu_separator[] = {
    UI_MENU_ENTRY_SEPERATOR,
    UI_MENU_ENTRY_LIST_END
};

/* Bitmaps for the menus: "tick" and right arrow (for submenus).  */
static Pixmap checkmark_bitmap, right_arrow_bitmap;

static int menu_popup = 0;

#define MAX_SUBMENUS 1024
static struct {
    Widget widget;
    Widget parent;
    int level;
} submenus[MAX_SUBMENUS];

static int num_submenus = 0;

static Widget active_submenu, active_entry;
static Widget active_keyboard_menu;

static int submenu_popped_up = 0;

static Widget top_menu;

static Widget *checkmark_menu_items = NULL;
static int num_checkmark_menu_items_max = 0;
int num_checkmark_menu_items = 0;

static Display *my_display;
static int my_screen;

/* ------------------------------------------------------------------------- */

/* This makes sure the submenu is fully visible.  */
static void position_submenu(Widget w, Widget parent)
{
    Position parent_x, parent_y, my_x, my_y;
    Dimension parent_width, my_width, my_height;
    int foo;
    unsigned int root_width, root_height, ufoo;
    Window foowin;

    /* Make sure the widget is realized--otherwise, we get 0 as width and
       height.  */
    XtRealizeWidget(w);

    XtVaGetValues(parent, XtNx, &parent_x, XtNy, &parent_y, XtNwidth, &parent_width, NULL);
    XtVaGetValues(w, XtNwidth, &my_width, XtNheight, &my_height, NULL);
    XtTranslateCoords(XtParent(parent), parent_x, parent_y, &parent_x, &parent_y);
    my_x = parent_x + parent_width - 2;
    my_y = parent_y + 1;
    XGetGeometry(my_display, RootWindow(my_display, my_screen), &foowin, &foo, &foo, &root_width, &root_height, &ufoo, &ufoo);
    if (my_x + my_width > (int)root_width) {
        my_x -= my_width + parent_width - 2;
    }
    if (my_y + my_height > (int) root_height) {
        my_y = root_height - my_height;
    }
    XtVaSetValues(w, XtNx, my_x, XtNy, my_y, NULL);
    XtPopup(w, XtGrabNonexclusive);
}

static int stay_up_menu_mode;

static UI_CALLBACK(menu_popup_callback)
{
    if (menu_popup == 0) {
        top_menu = w;
        stay_up_menu_mode = -1;         /* unknown yet */
        active_keyboard_menu = w;
    }
    menu_popup++;
}

static UI_CALLBACK(menu_popdown_callback)
{
    if (menu_popup > 0) {
        menu_popup--;
    }
    if (menu_popup == 0) {
        top_menu = NULL;
        active_keyboard_menu = NULL;
    }
}

static UI_CALLBACK(submenu_popup_callback)
{
    submenu_popped_up++;
}

static UI_CALLBACK(submenu_popdown_callback)
{
    submenu_popped_up--;
#if 0   /* doesn't seem to be needed */
    if (XawSimpleMenuGetActiveEntry(w)) {
#if MENU_DEBUG 
        printf("calling XtPopdown(%p)\n", client_data);
#endif
        XtPopdown((Widget)client_data);
    }
#endif
    active_submenu = 0;
}

/*
 * Some helper functions for choosing between click-move-click,
 * aka stay-up menus, and conventional Xaw press-drag-release menus.
 * The stay-up menu can come in 2 flavours: select-on-press or -on-release.
 * This can be chosen in the translation table.
 *
 * This is all stuff that really belongs in a subclass of SimpleMenu
 * and/or SmeBSB.
 *
 * 1 small weirdness remains:
 * - if you get a submenu to pop up under the mouse cursor, you have 2
 *   highlighted items.
 */

static void fake_button_action(Widget widget, XEvent *event, String *params, Cardinal *num_params)
{
    if (event->type == KeyPress) {
        /*
         * Lie about key presses, since XawSimpleMenus either don't know
         * about them, or handle them annoyingly differently (keyword:
         * spring loaded).
         * Fortunately Button presses are a very similar XEvent.
         */
        event->type = ButtonPress;
        event->xbutton.button = 1;
    } else if (event->type == KeyRelease) {
        event->type = ButtonRelease;
        event->xbutton.button = 1;
    }
}

static void menu_moved_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    /*
     * If we move the mouse first, before a button up,
     * we're in traditional press-drag-release mode.
     */
    if (stay_up_menu_mode == -1) {
        stay_up_menu_mode = 0;
    }
    /*
     * If the user moves the mouse around, adapt the keyboard focus too.
     */
    active_keyboard_menu = w;
}

/*
 * Perform
 * <some_event>: Popdownsubmenus() XtMenuPopdown() notify() unhighlight()\n
 *
 * It would be nicer if we could create a new event, say <MenuSelected>,
 * and have its actions in the translation table.
 */

static void do_menu_select(Widget w, XEvent *event, int keep_menu_up)
{
    String empty = "";

    if (!keep_menu_up) {
        XtCallActionProc(w, "Popdownsubmenus", event, &empty, 0);
        XtCallActionProc(w, "XtMenuPopdown", event, &empty, 0);
    }
    XtCallActionProc(w, "notify", event, &empty, 0);
    if (!keep_menu_up) {
        XtCallActionProc(w, "unhighlight", event, &empty, 0);
    }
}

/*
 * Usually called from a key press, but there is no reason it can't
 * be called from a button event.
 *
 * Accepts 1 numeric parameter: if nonzero, the menu stays up instead
 * of popping down.
 */
static void menu_select_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    int keep_menu_up = 0;

    if (*num_params == 1) {
        keep_menu_up = atoi(*params);
    }

    if (event->type == KeyPress || event->type == KeyRelease) {
        XtWidgetGeometry geometry;
        Widget old_active_entry;

        /*
         * Keyboard events bubble up to the top menu, because the top is
         * "spring loaded", So we may get to see them multiple times for
         * different widgets, if the mouse happens to be in a submenu. To
         * put a stop to that: only listen to the top_menu.
         */
        if (w != top_menu) {
#if MENU_DEBUG 
            printf("menu_select_action: ignoring event, not top_menu\n");
#endif
            return;
        }

        /*
         * Apparently, notify() looks at the event coordinates to know
         * which item should be notify()d, instead of just doing the
         * active one. So fake them.
         * It also looks if the event's window matches the widget,
         * presumably also because of spring-loaded menus.  Fake that too.
         * To look at the coordinates, it checks the event type, and it
         * doesn't know about keys. More faking.
         */
        old_active_entry = XawSimpleMenuGetActiveEntry(active_keyboard_menu);
        if (!old_active_entry) {
            return;
        }

        fake_button_action(w, event, NULL, NULL);       /* notify() cares */

        XtQueryGeometry(old_active_entry, NULL, &geometry);
        event->xbutton.x = geometry.x + 1;
        event->xbutton.y = geometry.y + 1;

        w = active_keyboard_menu;
        event->xany.window = XtWindow(w);       /* notify() cares about this */
    }

    do_menu_select(w, event, keep_menu_up);
}

static void menu_buttonup_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    /*
     * If we release the button first, before moving,
     * we're in the new click-move-click mode:
     * remember this, and ignore this first ButtonUp.
     */
    if (stay_up_menu_mode == -1) {
        stay_up_menu_mode = 1;
    } else {
        /*
         * Always select on ButtonUp, for stay-up in select-on-release mode.
         * In select-on-click mode we won't even get here.
         */
        do_menu_select(w, event, 0);
    }
}

static void menu_buttondown_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    /*
     * In press-drag-release mode, we can't even get here, so there
     * is no need to check for the mode.
     *
     * We can choose between select-on-press or select-on-release
     * by including/excluding the translation "<BtnDown>: ButtonDown()"
     * or even "<BtnDown>: Select()".
     */
    menu_select_action(w, event, params, num_params);
}

/*
 * Fake a MotionNotify event to select the new menu item by
 * emulating "<Motion>: highlight() PositionSubmenu()".
 */
static void do_fake_motion(Widget w, XEvent *event, int x, int y)
{
    String empty = "";
    XEvent copy;

    copy = *event;
    copy.type = MotionNotify;
    copy.xmotion.x = x;
    copy.xmotion.y = y;

    XtCallActionProc(w, "highlight", &copy, &empty, 0);
    XtCallActionProc(w, "PositionSubmenu", &copy, &empty, 0);
}

static void menu_nextprev_item(Widget w, XEvent *event, int incr)
{
    Widget old_active_entry;
    Widget new_active_entry = NULL;
    WidgetList children;
    Cardinal numChildren;
    int i = 0, found = 0, wrapped = 0;

    if (w != top_menu) {
#if MENU_DEBUG 
        printf("menu_nextprev_item_action: ignoring event, not top_menu\n");
#endif
        return;
    }

    /*
     * Find which smeBSB object is the active one.
     */
    XtVaGetValues(active_keyboard_menu, XtNchildren, &children,
                                        XtNnumChildren, &numChildren, NULL);

    /*
     * If there are not at least 2 children
     * then there is no point in even trying to go up or down.
     * Besides, might go out of the array's bounds.
     */
    if (numChildren < 2) {
        return;
    }

    old_active_entry = XawSimpleMenuGetActiveEntry(active_keyboard_menu);

    if (old_active_entry) {
        for (i = 0; i < numChildren; i++) {
            if (children[i] == old_active_entry) {
                found = 1;
                break;
            }
        }
    }

    /*
     * If no item was active, start at the top or the bottom.
     */
    if (!found) {
        i = (incr > 0) ? -incr : numChildren;
    }

    /*
     * Find the next sensitive item, going either up or down.
     * Note that we made the separator lines non-sensitive, just for this.
     * Do wrap around, but no more than once, that would be an endless loop.
     */
    while (wrapped < 2) {
        Boolean flag;

        i += incr;

        if (i < 0) {
            i = numChildren - 1;
            wrapped++;
        } else if (i >= numChildren) {
            i = 0;
            wrapped++;
        }

        XtVaGetValues(children[i], XtNsensitive, &flag, NULL);
        if (flag) {
            new_active_entry = children[i];
            break;
        }
    }

    if (new_active_entry && new_active_entry != old_active_entry) {
        XtWidgetGeometry geometry;
        int x, y;

        XtQueryGeometry(new_active_entry, NULL, &geometry);
        x = geometry.x + 1;
        y = geometry.y + 1;

        do_fake_motion(active_keyboard_menu, event, x, y);
    }
}

static void menu_prev_item_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    menu_nextprev_item(w, event, -1);
}

static void menu_next_item_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    menu_nextprev_item(w, event, +1);
}

static void menu_exit_submenu_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    int i;

    if (w != top_menu) {
#if MENU_DEBUG 
        printf("menu_exit_submenu_action: ignoring event, not top_menu\n");
#endif
        return;
    }

    /*
     * Try to find the parent menu.
     */
    for (i = 0; i < num_submenus; i++) {
        if (submenus[i].widget == active_keyboard_menu) {
            Widget parent_entry;
            XtWidgetGeometry geometry;
            int x, y;

            /*
             * Call this here, instead of in the translation table, since
             * the widget must be adjusted.
             */
            XtCallActionProc(active_keyboard_menu, "unhighlight", event, params, *num_params);

            parent_entry = submenus[i].parent;
            /* get to its SimpleMenu: gleaned from Xaw's SimpleMenu.c */
            active_keyboard_menu = XtParent(parent_entry);
            XtQueryGeometry(parent_entry, NULL, &geometry);
            /* The parent item is probably at this location. */
            x = geometry.x + 1;
            y = geometry.y + 1;

            do_fake_motion(active_keyboard_menu, event, x, y);
            break;
        }
    }
}

static void menu_enter_submenu_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    /*
     * If there really is a submenu, position_submenu_action()
     * has remembered its Widget.
     */
    if (!active_submenu) {
        return;
    }

    if (w != top_menu) {
#if MENU_DEBUG 
        printf("menu_enter_submenu_action: ignoring event, not top_menu\n");
#endif
        return;
    }
    /*
     * Call this here, instead of in the translation table, since
     * the widget must be adjusted.
     */
    XtCallActionProc(active_keyboard_menu, "unhighlight", event, params, *num_params);

    active_keyboard_menu = active_submenu;

    /*
     * What if the top item in the submenu isn't sensitive...
     * Well, fortunately the current menus don't seem to be like that.
     */
    do_fake_motion(active_keyboard_menu, event, 3, 3);
}

/* ------------------------------------------------------------------------- */

typedef struct {
    ui_hotkey_modifier_t modifier;
    ui_keysym_t keysym;
    ui_callback_t callback;
    ui_callback_data_t client_data;
} registered_hotkey_t;

static registered_hotkey_t *registered_hotkeys = NULL;
static int num_registered_hotkeys;
static int num_allocated_hotkeys;

/* ------------------------------------------------------------------------- */

static void ui_hotkey_register(ui_hotkey_modifier_t modifier, signed long keysym, void *callback, void *client_data)
{
    registered_hotkey_t *p;

    if (registered_hotkeys == 0) {
        num_allocated_hotkeys = 32;
        registered_hotkeys = lib_malloc(num_allocated_hotkeys * sizeof(registered_hotkey_t));
        num_registered_hotkeys = 0;
    } else if (num_registered_hotkeys == num_allocated_hotkeys) {
        num_allocated_hotkeys *= 2;
        registered_hotkeys = lib_realloc(registered_hotkeys, (num_allocated_hotkeys * sizeof(registered_hotkey_t)));
    }

    p = registered_hotkeys + num_registered_hotkeys;

    p->modifier = modifier;
    p->keysym = (ui_keysym_t)keysym;
    p->callback = (ui_callback_t)callback;
    p->client_data = (ui_callback_data_t)client_data;

    num_registered_hotkeys++;
}

/* ------------------------------------------------------------------------- */

int ui_dispatch_hotkeys(int key)
{
    int i, ret = 0;
    registered_hotkey_t *p = registered_hotkeys;

    /* XXX: Notice that we don't actually check the hotkey modifiers
       here.  */
    for (i = 0; i < num_registered_hotkeys; i++, p++) {
        if (p->keysym == key) {
              ((void *(*)(void *, void *, void *))p->callback)(NULL, p->client_data, NULL);
              ret = 1;
              break;
        }
    }
    return ret;
}

/* ------------------------------------------------------------------------- */

/* Yes, this sucks.  Sorry.  */
static void position_submenu_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    Widget new_active_submenu, new_active_entry;

    new_active_entry = XawSimpleMenuGetActiveEntry(w);
    if (new_active_entry != active_entry) {
        int i, level;
        int level_found, active_found;

        new_active_submenu = NULL;

        /* Find the submenu for the current active menu item and the level of
           this submenu.  */
        for (level_found = active_found = 0, level = 0, i = 0; i < num_submenus && !(level_found && active_found); i++) {
            if (!active_found && submenus[i].parent == new_active_entry) {
                new_active_submenu = submenus[i].widget;
                active_found = 1;
            }
            if (!level_found && submenus[i].widget == w) {
                level = submenus[i].level;
                level_found = 1;
            }
        }

        /* Remove all the submenus whose level is higher than this submenu.  */
        for (i = 0; i < num_submenus; i++) {
            if (submenus[i].level > level) {
                XtPopdown(submenus[i].widget);
            }
        }

        /* Position the submenu for this menu item.  */
        if (new_active_submenu != NULL && new_active_entry != NULL) {
            position_submenu(new_active_submenu, new_active_entry);
        }

        active_submenu = new_active_submenu;
        active_entry = new_active_entry;
    }
}

static void popdown_submenus_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    int i;

    if (menu_popup == 0) {
        return;
    }

    /* Pop down all the submenus and the top ones.  */

    for (i = 0; i < num_submenus; i++) {
        XtPopdown(submenus[i].widget);
    }

    XtPopdown(top_menu);
    top_menu = NULL;

#ifdef HAVE_FULLSCREEN
    fullscreen_set_mouse_timeout();
#endif
    menu_popup = 0;
}

static void menu_unhighlight_action(Widget w, XEvent *event, String *params, Cardinal *num_params)
{
    XtCallActionProc(w, "unhighlight", event, params, *num_params);
}

static char *make_menu_label(ui_menu_entry_t *e)
{
    const char *key_string, *tmp = "";
    char *retstr, *trans, *dots = "";

    /* Check wether NO_TRANS prefix is there, if yes don't translate it */
    if (strncmp(e->string, NO_TRANS, strlen(NO_TRANS)) == 0) {
        trans = lib_stralloc(e->string + strlen(NO_TRANS));
    } else {
        trans = lib_stralloc(_(e->string));
    }

    if (e->type == UI_MENU_TYPE_DOTS || e->type == UI_MENU_TYPE_TICKDOTS) {
       dots = "...";
    }

    if (e->hotkey_keysym == KEYSYM_NONE) {
        retstr = util_concat(trans, dots, NULL);
        lib_free(trans);
        return retstr;
    }

    if (e->hotkey_modifier & UI_HOTMOD_CONTROL) {
        tmp = "C-";
    }
    if (e->hotkey_modifier & UI_HOTMOD_META) {
        tmp = "M-";
    }
    if (e->hotkey_modifier & UI_HOTMOD_ALT) {
        tmp = "A-";
    }
    if (e->hotkey_modifier & UI_HOTMOD_SHIFT) {
        tmp = "S-";
    }

    key_string = strchr(XKeysymToString(e->hotkey_keysym), '_');
    if (key_string == NULL) {
        key_string = XKeysymToString(e->hotkey_keysym);
    } else {
        key_string++;
    }

    retstr = util_concat(trans, dots, "    (", tmp, key_string, ")", NULL);

    lib_free(trans);

    return retstr;
}

/* ------------------------------------------------------------------------- */

int ui_menu_init(XtAppContext app_context, Display *d, int s)
{
    static XtActionsRec actions[] = {
        { "PositionSubmenu", position_submenu_action },
        { "Popdownsubmenus", popdown_submenus_action },
        { "Unhighlight", menu_unhighlight_action },
        { "Moved", menu_moved_action },
        { "Select", menu_select_action },
        { "ButtonDown", menu_buttondown_action },
        { "ButtonUp", menu_buttonup_action },
        { "PrevItem", menu_prev_item_action },
        { "NextItem", menu_next_item_action },
        { "ExitSubmenu", menu_exit_submenu_action },
        { "EnterSubmenu", menu_enter_submenu_action },
        { "FakeButton", fake_button_action },
    };

    my_display = d;
    my_screen = s;

    checkmark_bitmap = XCreateBitmapFromData(my_display,
                                             DefaultRootWindow(my_display),
                                             (char *)checkmark_bits,
                                             checkmark_width,
                                             checkmark_height);

    right_arrow_bitmap = XCreateBitmapFromData(my_display,
                                               DefaultRootWindow(my_display),
                                               (char *)right_arrow_bits,
                                               right_arrow_width,
                                               right_arrow_height);

    XtAppAddActions(app_context, actions, XtNumber(actions));
    XawSimpleMenuAddGlobalActions(app_context);

    if (registered_hotkeys != NULL) {
        lib_free(registered_hotkeys);
        registered_hotkeys = NULL;
        num_registered_hotkeys = num_allocated_hotkeys = 0;
    }

    return 0;
}

/*
 * Hack the popup_list from the Core part of the parent Widget from a
 * FIFO to a LIFO list.
 *
 * Reason: if a Widget (a menu in our case) is destroyed, this happens
 * in 2 phases. Only in phase 2 is the Widget removed from this
 * popup_list.  Phase 2 is only called if app->dispatch_level == 0. So,
 * if we destroy a menu in a callback or Action, this does not happen
 * (yet).  If (before the dispatch ends) a new replacement menu is
 * created with the same name, it will normally be put at the end of the
 * list. If the old (half-destroyed one) is still there, it will be
 * found first. Chaos will ensue.
 *
 * Changing the order of the list seems the least disruptive way to fix
 * this.
 */

#ifndef DARWIN_COMPILE
static void do_popuplist_hack(Widget widget)
{
    Widget parent = XtParent(widget);
    Cardinal i, n;

    n = parent->core.num_popups - 1;
    if (parent->core.popup_list[n] == widget) {
        for (i = n; i > 0; i--) {
            parent->core.popup_list[i] = parent->core.popup_list[i-1];
        }
        parent->core.popup_list[0] = widget;
    } else {
        static int warned;
        
        if (!warned) {
            log_warning(LOG_DEFAULT, "popup widget list not FIFO");
            warned = 1;
        }
    }
}
#endif

static void ui_add_items_to_shell(Widget w, int menulevel, ui_menu_entry_t *list);
static XtTranslations menu_translations;

Widget ui_menu_create(const char *menu_name, ...)
{
    Widget w;
    ui_menu_entry_t *list;
    va_list ap;

    w = ui_create_shell(_ui_top_level, menu_name, simpleMenuWidgetClass);
#ifndef DARWIN_COMPILE
    do_popuplist_hack(w);
#endif
    XtAddCallback(w, XtNpopupCallback, menu_popup_callback, NULL);
    XtAddCallback(w, XtNpopdownCallback, menu_popdown_callback, NULL);

    /*
     * We want to be able to have either kind of menu:
     * stay-up when we release the mouse button before moving
     * (it starts on the edge of the menu).
     */
    if (!menu_translations) {
        menu_translations = XtParseTranslationTable(
            "<Motion>: Moved() highlight() PositionSubmenu()\n"
            "<LeaveWindow>: Unhighlight()\n"
            "<BtnUp>: ButtonUp()\n"     /* may do Select() depending on mode */
            "<KeyDown>Down: NextItem()\n"
            "<KeyDown>Up: PrevItem()\n"
            "<KeyDown>Left: ExitSubmenu()\n"
            "<KeyDown>Right: EnterSubmenu()\n"
            "<KeyDown>Return: Select()\n"
            "<KeyDown>space: Select(1)\n"
            "<KeyDown>Escape: Popdownsubmenus() XtMenuPopdown() unhighlight()\n"
          );
    }
    XtOverrideTranslations(w, menu_translations);

    va_start(ap, menu_name);
    while ((list = va_arg(ap, ui_menu_entry_t *)) != NULL) {
        ui_add_items_to_shell(w, 1, list);
    }
    va_end(ap);

    return w;
}

void ui_menu_delete(Widget widget)
{
    if (widget) {
        /* pop down the menu if it is still up */
        XtPopdown(widget);
        XtDestroyWidget(widget);
    }
}

static void tick_destroy(Widget w, XtPointer client_data, XtPointer call_data)
{
    int index = (int)(long)client_data;

    if (index >= num_checkmark_menu_items) {
        index = num_checkmark_menu_items - 1;
    }

    /*
     * Invariant: the callback receives the position that was originally
     * assigned to the menu item. This is an upper bound to the actual
     * position, because this function may move a menu item to a lower
     * position in the list, but not to a higher one.
     *
     * This is also efficient because the dynamic menus tend to be at
     * the end of the list, certainly after the first destroy/create
     * iteration.
     */
    while (index >= 0) {
        if (checkmark_menu_items[index] == w) {
            if (index == num_checkmark_menu_items - 1) {
                /* The last item in the list: just forget it. */
            } else {
                /*
                 * Put the last item from the list in the destroyed
                 * location.
                 */
                checkmark_menu_items[index] =
                    checkmark_menu_items[num_checkmark_menu_items - 1];
            }
            num_checkmark_menu_items--;
            checkmark_menu_items[num_checkmark_menu_items] = 0;
            break;
        }
        index--;
    }
}

static void ui_add_items_to_shell(Widget w, int menulevel, ui_menu_entry_t *list)
{
    unsigned int i, j;

    for (i = j = 0; list[i].string; i++) {
        Widget new_item = NULL;
        char *name;
        int update_item = 0;

        name = lib_msprintf("MenuItem%d", j);
        switch (list[i].type) {
            case UI_MENU_TYPE_SEPARATOR:    /* line */
                new_item = XtVaCreateManagedWidget("separator",
                                                   smeLineObjectClass, w,
                                                   XtNsensitive, 0,
                                                   NULL);
                break;
            case UI_MENU_TYPE_TICK:         /* toggle */
            case UI_MENU_TYPE_TICKDOTS:     /* toggle */
                {
                    char *label = make_menu_label(&list[i]);

                    new_item = XtVaCreateManagedWidget(name,
                                                       smeBSBObjectClass, w,
                                                       XtNrightMargin, 20,
                                                       XtNleftMargin, 20,
                                                       XtNlabel, label,
                                                       NULL);
                    /* Add this item to the list of calls to perform to update
                       the menu status. */
                    if (list[i].callback) {
                        update_item = 1;
                    } else {
			log_error(LOG_DEFAULT, "checkbox menu item without callback: %s",
				  label);
                    }
                    j++;

                    lib_free(label);
                }
                break;
            case UI_MENU_TYPE_NONE:
                break;
            case UI_MENU_TYPE_BL_SUB:
                update_item = 1;
                /* fall through */
            default:
                {
                    char *label = make_menu_label(&list[i]);

                    if (update_item && !list[i].callback) {
                        log_error(LOG_DEFAULT, "callback menu item without callback: %s",
                                  label);
                    }
                    new_item = XtVaCreateManagedWidget(name,
                                                       smeBSBObjectClass, w,
                                                       XtNleftMargin, 20,
                                                       XtNrightMargin, 20,
                                                       XtNlabel, label,
                                                       NULL);
                    lib_free(label);
                    j++;
                }
        }

        if (update_item) {
            /*
             * This is a menu item that may be updated on the fly:
             * either checked/unchecked or sensitive/insensitive (grayed out).
             */
            if (num_checkmark_menu_items >= num_checkmark_menu_items_max) {
                num_checkmark_menu_items_max += 100;
                checkmark_menu_items = lib_realloc(checkmark_menu_items, num_checkmark_menu_items_max * sizeof(Widget));
            }
            /* cast num_checkmark_menu_items to long and then to void *, so
             * casting to XtPointer (void*) is safe. (BW) */
            XtAddCallback(new_item, XtNdestroyCallback, tick_destroy,
                    (XtPointer)int_to_void_ptr(num_checkmark_menu_items));
            checkmark_menu_items[num_checkmark_menu_items++] = new_item;
        }

        lib_free(name);

        if (list[i].callback) {
            XtAddCallback(new_item, XtNcallback, (XtCallbackProc)list[i].callback, list[i].callback_data);
        }
        if (list[i].sub_menu) {
            /*
             * Apparently, the submenu support of Xaw is not used
             * (it isn't documented in the specs document but exists since
             * patch "#2716 24 Apr 1999"; see its old-doc/ChangeLog;
             * see also the man page Xaw(3) which mentions it.)
             * If the "menuName" resource (XtNmenuName) of a SmeBSB (item)
             * contains a menu name, that is the submenu and it will
             * pop up automatically.
            */
            if (num_submenus > MAX_SUBMENUS) {
                fprintf(stderr, "Maximum number of sub menus reached! Please fix the code.\n");
                exit(-1);
            }
            if (new_item != NULL && (list[i].type != UI_MENU_TYPE_SEPARATOR)) {
                Widget subw;

                XtVaSetValues(new_item, XtNrightBitmap, right_arrow_bitmap, NULL);
                subw = ui_create_shell(_ui_top_level, "SUB", simpleMenuWidgetClass);
                XtAddCallback(subw, XtNpopupCallback, submenu_popup_callback, submenus + num_submenus);
                XtAddCallback(subw, XtNpopdownCallback, submenu_popdown_callback, (XtPointer)w);
                XtOverrideTranslations(subw, menu_translations);

                ui_add_items_to_shell(subw, menulevel + 1, list[i].sub_menu);
                submenus[num_submenus].widget = subw;
                submenus[num_submenus].parent = new_item;
                submenus[num_submenus].level = menulevel;
                num_submenus++;
            } else {
                ui_add_items_to_shell(w, menulevel, list[i].sub_menu);
            }
        } else {            /* no submenu */
            if (list[i].hotkey_keysym != (KeySym)0 && list[i].callback != NULL) {
                ui_hotkey_register(list[i].hotkey_modifier, (signed long)list[i].hotkey_keysym, (ui_callback_t)list[i].callback, (ui_callback_data_t)list[i].callback_data);
            }
        }
    }

#ifdef UI_MENU_DEBUG
    fprintf(stderr, "num_checkmark_menu_items: %d\tnum_submenus = %d.\n", num_checkmark_menu_items, num_submenus);
#endif
}

int ui_menu_any_open(void)
{
    return menu_popup;
}

void ui_menu_update_all(void)
{
    int i;

    for (i = 0; i < num_checkmark_menu_items; i++) {
        XtCallCallbacks(checkmark_menu_items[i], XtNcallback, (XtPointer)!NULL);
    }
}

void ui_menu_set_tick(Widget w, int flag)
{
    XtVaSetValues(w, XtNleftBitmap, flag ? checkmark_bitmap : 0, NULL);
}

void ui_menu_set_sensitive(Widget w, int flag)
{
    XtVaSetValues(w, XtNsensitive, flag, NULL);
}

/* ------------------------------------------------------------------------- */

/* These functions are called by radio and toggle menu items if the callback
   functions are defined through `UI_MENU_DEFINE_TOGGLE()',
   `UI_MENU_DEFINE_RADIO()' or `UI_MENU_DEFINE_STRING_RADIO()'.  */

void _ui_menu_toggle_helper(Widget w, ui_callback_data_t client_data, ui_callback_data_t call_data, const char *resource_name)
{
    int current_value;

    if (resources_get_int(resource_name, &current_value) < 0) {
        return;
    }

    if (!call_data) {
        resources_set_int(resource_name, !current_value);
        ui_update_menus();
    } else {
        ui_menu_set_tick(w, current_value);
    }
}

void _ui_menu_radio_helper(Widget w, ui_callback_data_t client_data, ui_callback_data_t call_data, const char *resource_name)
{
    int current_value;

    resources_get_int(resource_name, &current_value);

    if (!call_data) {
        if (current_value != vice_ptr_to_int(client_data)) {
            resources_set_int(resource_name, vice_ptr_to_int(client_data));
            ui_update_menus();
        }
    } else {
        ui_menu_set_tick(w, current_value == vice_ptr_to_int(client_data));
    }
}

void _ui_menu_string_radio_helper(Widget w, ui_callback_data_t client_data, ui_callback_data_t call_data, const char *resource_name)
{
    const char *current_value;

    resources_get_string(resource_name, &current_value);

    if (current_value == 0) {
        return;
    }

    if (!call_data) {
        if (strcmp(current_value, (const char *)client_data) != 0) {
            resources_set_string(resource_name, (const char *)client_data);
            ui_update_menus();
        }
    } else {
        ui_menu_set_tick(w, strcmp(current_value, (const char *)client_data) == 0);
    }
}

void uimenu_shutdown(void)
{
    lib_free(registered_hotkeys);
    registered_hotkeys = NULL;
    lib_free(checkmark_menu_items);
    checkmark_menu_items = NULL;
    ui_about_shutdown();
}
