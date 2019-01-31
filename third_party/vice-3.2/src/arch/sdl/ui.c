/*
 * ui.c - Common UI routines.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * Based on code by
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

/* #define SDL_DEBUG */

#include "vice.h"

#include "vice_sdl.h"
#include <stdio.h>

#include "autostart.h"
#include "cmdline.h"
#include "color.h"
#include "fullscreenarch.h"
#include "joy.h"
#include "kbd.h"
#include "lib.h"
#include "lightpen.h"
#include "log.h"
#include "machine.h"
#include "mouse.h"
#include "mousedrv.h"
#include "resources.h"
#include "translate.h"
#include "types.h"
#include "ui.h"
#include "uiapi.h"
#include "uicolor.h"
#include "uifilereq.h"
#include "uimenu.h"
#include "uimsgbox.h"
#include "uistatusbar.h"
#include "videoarch.h"
#include "vkbd.h"
#include "vsidui_sdl.h"
#include "vsync.h"

#ifndef SDL_DISABLE
#define SDL_DISABLE SDL_IGNORE
#endif

#ifdef SDL_DEBUG
#define DBG(x)  log_debug x
#else
#define DBG(x)
#endif

#ifdef ANDROID_COMPILE 
extern void keyboard_key_pressed(signed long key);
#endif  


static int sdl_ui_ready = 0;

/* ----------------------------------------------------------------- */
/* ui.h */

/* Misc. SDL event handling */
void ui_handle_misc_sdl_event(SDL_Event e)
{
#ifdef USE_SDLUI2
    if (e.type == SDL_WINDOWEVENT) {
        switch (e.window.event) {
            case SDL_WINDOWEVENT_RESIZED:
                DBG(("ui_handle_misc_sdl_event: SDL_WINDOWEVENT_RESIZED (%d,%d)", (unsigned int)e.window.data1, (unsigned int)e.window.data2));
                sdl_video_resize_event((unsigned int)e.window.data1, (unsigned int)e.window.data2);
                video_canvas_refresh_all(sdl_active_canvas);
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                DBG(("ui_handle_misc_sdl_event: SDL_WINDOWEVENT_FOCUS_GAINED"));
                video_canvas_refresh_all(sdl_active_canvas);
                break;
            case SDL_WINDOWEVENT_EXPOSED:
                DBG(("ui_handle_misc_sdl_event: SDL_WINDOWEVENT_EXPOSED"));
                video_canvas_refresh_all(sdl_active_canvas);
                break;
        }
    }
#endif
    switch (e.type) {
        case SDL_QUIT:
            DBG(("ui_handle_misc_sdl_event: SDL_QUIT"));
            ui_sdl_quit();
            break;
#ifndef USE_SDLUI2
        case SDL_VIDEORESIZE:
            DBG(("ui_handle_misc_sdl_event: SDL_VIDEORESIZE (%d,%d)", (unsigned int)e.resize.w, (unsigned int)e.resize.h));
            sdl_video_resize_event((unsigned int)e.resize.w, (unsigned int)e.resize.h);
            video_canvas_refresh_all(sdl_active_canvas);
            break;
        case SDL_ACTIVEEVENT:
            DBG(("ui_handle_misc_sdl_event: SDL_ACTIVEEVENT"));
            if ((e.active.state & SDL_APPACTIVE) && e.active.gain) {
                video_canvas_refresh_all(sdl_active_canvas);
            }
            break;
        case SDL_VIDEOEXPOSE:
            DBG(("ui_handle_misc_sdl_event: SDL_VIDEOEXPOSE"));
            video_canvas_refresh_all(sdl_active_canvas);
            break;
#else
        case SDL_DROPFILE:
            if (autostart_autodetect(e.drop.file, NULL, 0, AUTOSTART_MODE_RUN) < 0) {
                ui_error("Cannot autostart specified file.");
            }
            break;
#endif
#ifdef SDL_DEBUG
        case SDL_USEREVENT:
            DBG(("ui_handle_misc_sdl_event: SDL_USEREVENT"));
            break;
        case SDL_SYSWMEVENT:
            DBG(("ui_handle_misc_sdl_event: SDL_SYSWMEVENT"));
            break;
#endif
        default:
            DBG(("ui_handle_misc_sdl_event: unhandled"));
            break;
    }
}

#ifdef ANDROID_COMPILE
#include "loader.h"
#include "keyboard.h"

extern int loader_loadstate;
extern int loader_savestate;
extern int loader_turbo;
extern int loader_showinfo;
extern int loader_true_drive;
extern char savestate_filename[256];
extern void loader_save_snapshot(char *name);
extern void loader_load_snapshot(char *name);
extern void loader_set_warpmode(int on);
extern void loader_set_statusbar(int val);
extern void loader_set_drive_true_emulation(int val);
static int oldx = 0, oldy = 0, down_x, down_y;
int old_joy_direction = 0;
extern int mouse_x, mouse_y;
#endif

#if 0
/* unused ? */
void ui_dispatch_next_event(void)
{
#ifdef ANDROID_COMPILE
    struct locnet_al_event event;

    if (Android_PollEvent(&event)) {
#else
    SDL_Event e;

    if (SDL_PollEvent(&e)) {
        ui_handle_misc_sdl_event(e);
#endif
    } else {
        /* Add a small delay to not hog the host CPU when remote
           monitor is being used. */
        SDL_Delay(10);
    }
}
#endif

/* Main event handler */
ui_menu_action_t ui_dispatch_events(void)
{
    SDL_Event e;
    ui_menu_action_t retval = MENU_ACTION_NONE;

#ifdef ANDROID_COMPILE
    struct locnet_al_event event1;

    if (loader_showinfo) {
        int value = loader_showinfo;

        loader_showinfo = 0;
        loader_set_statusbar((value == 1) ? 1 : 0);
    }
    if (loader_true_drive) {
        int value = loader_true_drive;

        loader_true_drive = 0;
        loader_set_drive_true_emulation((value == 1) ? 1 : 0);
    }
    if (loader_turbo) {
        int value = loader_turbo;

        loader_turbo = 0;
        loader_set_warpmode((value == 1) ? 1 : 0);
    }
    if (loadf->frameskip) {
        int value = loadf->frameskip;

        loadf->frameskip = 0;
        resources_set_int("RefreshRate", ((value > 0) && (value <= 10)) ? value : 1);
    }
    if (loadf->abort) {
        loadf->abort = 0;
        ui_pause_emulation(1);
        ui_sdl_quit();
        ui_pause_emulation(0);
        return MENU_ACTION_NONE;
    } else if (loader_loadstate) {
        loader_loadstate = 0;
        loader_load_snapshot(savestate_filename);
        ui_pause_emulation(0);
        return MENU_ACTION_NONE;
    } else if (loader_savestate) {
        loader_savestate = 0;
        loader_save_snapshot(savestate_filename);
        ui_pause_emulation(0);
        return MENU_ACTION_NONE;
    }

    int stopPoll = 0;

    while ((!stopPoll) && Android_PollEvent(&event1)) {
        struct locnet_al_event *event = &event1;

        switch (event->eventType) {
            case SDL_MOUSEMOTION:
                {
                    /* locnet, 2011-06-16, detect auto calibrate */
                    if ((event->x == -2048) && (event->y == -2048)) {
                        down_x = -1;
                        down_y = -1;
                        oldx = 0;
                        oldy = 0;
                        stopPoll = 1;
                    /* locnet, 2011-07-01, detect pure relative move */
                    } else if ((event->down_x == -1024) && (event->down_y == -1024)) {
                        down_x = 0;
                        down_y = 0;
                        oldx = 0;
                        oldy = 0;
                    } else if ((down_x != event->down_x) || (down_y != event->down_y)) {
                        down_x = event->down_x;
                        down_y = event->down_y;
                        oldx = down_x;
                        oldy = down_y;
                    }
                    mouse_move((int)(event->x - oldx), (int)(event->y - oldy));
                    oldx = event->x;
                    oldy = event->y;
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
                {
                    if ((event->down_x >= 0) && (event->down_y >= 0)) {
                        mouse_x = 640 * event->down_x / 1000.0f - 64;
                        mouse_y = 400 * (1000 - event->down_y) / 1000.0f - 200;
                    }
                    if (event->keycode >= 0) {
                        mouse_button((int)(event->keycode) ? SDL_BUTTON_RIGHT : SDL_BUTTON_LEFT, 1);
                    }
                    stopPoll = 1;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                {
                    if (event->keycode >= 0) {
                        mouse_button((int)(event->keycode) ? SDL_BUTTON_RIGHT : SDL_BUTTON_LEFT, 0);
                    }
                    stopPoll = 1;
                }
                break;
            case SDL_JOYAXISMOTION:
                {
                    float x = event->x / 256.0f;
                    float y = event->y / 256.0f;
                    int left = 0, top = 0, right = 0, bot = 0;
                    int value;

                    if (y < -DEAD_ZONE) {
                        top = 1;
                    }
                    if (y > DEAD_ZONE) {
                        bot = 1;
                    }
                    if (x < -DEAD_ZONE) {
                        left = 1;
                    }
                    if (x > DEAD_ZONE) {
                        right = 1;
                    }

                    value = 0;

                    if (left) {
                        value |= 4;
                    }
                    if (right) {
                        value |= 8;
                    }
                    if (top) {
                        value |= 1;
                    }
                    if (bot) {
                        value |= 2;
                    }
                    retval = sdljoy_axis_event(0, 0, event->x / 256.0f * 32767);
                    ui_menu_action_t retval2 = sdljoy_axis_event(0, 1, event->y / 256.0f * 32767);
                    if (retval == MENU_ACTION_NONE) {
                        retval = retval2;
                    }
                    old_joy_direction = value;
                    stopPoll = 1;
                }
                break;
            case SDL_JOYBUTTONDOWN:
                {
                    retval = sdljoy_button_event(0, event->keycode, 1);

                    /* 2011-09-20, buffer overflow when autofire if stopPoll */
                    if (!Android_HasRepeatEvent(SDL_JOYBUTTONDOWN, event->keycode)) {
                        stopPoll = 1;
                    }
                }
                break;
            case SDL_JOYBUTTONUP:
                {
                    retval = sdljoy_button_event(0, event->keycode, 0);

                    /* 2011-09-20, buffer overflow when autofire if stopPoll */
                    if (!Android_HasRepeatEvent(SDL_JOYBUTTONUP, event->keycode)) {
                        stopPoll = 1;
                    }
                }
                break;
            case SDL_KEYUP:
            case SDL_KEYDOWN:
                {
                    static int ctrl_down = 0;
                    static int alt_down = 0;
                    static int shift_down = 0;

                    int down = (event->eventType == SDL_KEYDOWN);
                    unsigned long modifier = event->modifier;
                    int ctrl = ((modifier & KEYBOARD_CTRL_FLAG) != 0);
                    int alt = ((modifier & KEYBOARD_ALT_FLAG) != 0);
                    int shift = ((modifier & KEYBOARD_SHIFT_FLAG) != 0);
                    unsigned long kcode = (unsigned long)event->keycode;

                    int kmod = 0;

                    if (ctrl) {
                        kmod |= KMOD_LCTRL;
                    }
                    if (alt) {
                        kmod |= KMOD_LALT;
                    }
                    if (shift) {
                        kmod |= KMOD_LSHIFT;
                    }
                    if (down) {
                        if (ctrl || (kcode == SDLK_TAB)) {
                            if (!ctrl_down) {
                                keyboard_key_pressed((unsigned long)SDLK_TAB);
                            }
                            ctrl_down++;
                        }
                        if (alt || (kcode == SDLK_LCTRL)) {
                            if (!alt_down) {
                                keyboard_key_pressed((unsigned long)SDLK_LCTRL);
                            }
                            alt_down++;
                        }
                        if (shift || (kcode == SDLK_LSHIFT)) {
                            if (!shift_down) {
                                keyboard_key_pressed((unsigned long)SDLK_LSHIFT);
                            }
                            shift_down++;
                        }
                        ui_display_kbd_status(event);
                        retval = sdlkbd_press(kcode, 0);
                    } else {
                        ui_display_kbd_status(event);
                        retval = sdlkbd_release(kcode, 0);

                        if (ctrl || (kcode == SDLK_TAB)) {
                            if (kcode == SDLK_TAB) {
                                ctrl_down = 0;
                            }
                            if (ctrl_down) {
                                ctrl_down--;
                            }
                            if (!ctrl_down) {
                                keyboard_key_released((unsigned long)SDLK_TAB);
                            }
                        }
                        if (alt || (kcode == SDLK_LCTRL)) {
                            if (kcode == SDLK_LCTRL) {
                                alt_down = 0;
                            }
                            if (alt_down) {
                                alt_down--;
                            }
                            if (!alt_down) {
                                keyboard_key_released((unsigned long)SDLK_LCTRL);
                            }
                        }
                        if (shift || (kcode == SDLK_LSHIFT)) {
                            if (kcode == SDLK_LSHIFT) {
                                shift_down = 0;
                            }
                            if (shift_down) {
                                shift_down--;
                            }
                            if (!shift_down) {
                                keyboard_key_released((unsigned long)SDLK_LSHIFT);
                            }
                        }
                    }
                    stopPoll = 1;
                }
                break;
        }
    }
#else
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_KEYDOWN:
                ui_display_kbd_status(&e);
                retval = sdlkbd_press(SDL2x_to_SDL1x_Keys(e.key.keysym.sym), e.key.keysym.mod);
                break;
            case SDL_KEYUP:
                ui_display_kbd_status(&e);
                retval = sdlkbd_release(SDL2x_to_SDL1x_Keys(e.key.keysym.sym), e.key.keysym.mod);
                break;
#ifdef HAVE_SDL_NUMJOYSTICKS
            case SDL_JOYAXISMOTION:
                retval = sdljoy_axis_event(e.jaxis.which, e.jaxis.axis, e.jaxis.value);
                break;
            case SDL_JOYBUTTONDOWN:
                retval = sdljoy_button_event(e.jbutton.which, e.jbutton.button, 1);
                break;
            case SDL_JOYBUTTONUP:
                retval = sdljoy_button_event(e.jbutton.which, e.jbutton.button, 0);
                break;
            case SDL_JOYHATMOTION:
                retval = sdljoy_hat_event(e.jhat.which, e.jhat.hat, e.jhat.value);
                break;
#endif
            case SDL_MOUSEMOTION:
                if (_mouse_enabled) {
                    mouse_move((int)(e.motion.xrel), (int)(e.motion.yrel));
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                if (_mouse_enabled) {
                    mouse_button((int)(e.button.button), (e.button.state == SDL_PRESSED));
                }
                break;
            default:
                /* SDL_EventState(SDL_VIDEORESIZE, SDL_IGNORE); */
                ui_handle_misc_sdl_event(e);
                /* SDL_EventState(SDL_VIDEORESIZE, SDL_ENABLE); */
                break;
        }
        /* When using the menu or vkbd, pass every meaningful event to the caller */
        if (((sdl_menu_state) || (sdl_vkbd_state & SDL_VKBD_ACTIVE)) && (retval != MENU_ACTION_NONE) && (retval != MENU_ACTION_NONE_RELEASE)) {
            break;
        }
    }
#endif
    return retval;
}

/* note: we need to be a bit more "radical" about disabling the (mouse) pointer.
 * in practise, we really only need it for the lightpen emulation.
 *
 * TODO: and perhaps in windowed mode enable it when the mouse is moved.
 */

void ui_check_mouse_cursor(void)
{
    if (_mouse_enabled && !lightpen_enabled && !sdl_menu_state) {
        /* mouse grabbed, not in menu. grab input but do not show a pointer */
#ifndef USE_SDLUI2
        SDL_ShowCursor(SDL_DISABLE);
        SDL_WM_GrabInput(SDL_GRAB_ON);
#else
        SDL_SetRelativeMouseMode(SDL_TRUE);
#endif
    } else if (lightpen_enabled && !sdl_menu_state) {
        /* lightpen active, not in menu. show a pointer for the lightpen emulation */
#ifndef USE_SDLUI2
        SDL_ShowCursor(SDL_ENABLE);
        SDL_WM_GrabInput(SDL_GRAB_OFF);
#else
        SDL_SetRelativeMouseMode(SDL_FALSE);
#endif
    } else {
        if (sdl_active_canvas->fullscreenconfig->enable) {
            /* fullscreen, never show pointer (we really never need it) */
#ifndef USE_SDLUI2
            SDL_ShowCursor(SDL_DISABLE);
            SDL_WM_GrabInput(SDL_GRAB_OFF);
#else
            SDL_ShowCursor(SDL_DISABLE);
            SDL_SetRelativeMouseMode(SDL_FALSE);
#endif
        } else {
            /* windowed, TODO: disable pointer after time-out */
#ifndef USE_SDLUI2
            SDL_ShowCursor(SDL_DISABLE);
            SDL_WM_GrabInput(SDL_GRAB_OFF);
#else
            SDL_ShowCursor(SDL_DISABLE);
            SDL_SetRelativeMouseMode(SDL_FALSE);
#endif
        }
    }
}

void ui_message(const char* format, ...)
{
    va_list ap;
    char *tmp;

    va_start(ap, format);
    tmp = lib_mvsprintf(format, ap);
    va_end(ap);

    if (sdl_ui_ready) {
        message_box("VICE MESSAGE", tmp, MESSAGE_OK);
    } else {
        fprintf(stderr, "%s\n", tmp);
    }
    lib_free(tmp);
}

/* ----------------------------------------------------------------- */
/* uiapi.h */

static int save_resources_on_exit;
static int confirm_on_exit;

static int set_ui_menukey(int val, void *param)
{
    sdl_ui_menukeys[vice_ptr_to_int(param)] = SDL2x_to_SDL1x_Keys(val);
    return 0;
}

static int set_save_resources_on_exit(int val, void *param)
{
    save_resources_on_exit = val ? 1 : 0;

    return 0;
}

static int set_confirm_on_exit(int val, void *param)
{
    confirm_on_exit = val ? 1 : 0;

    return 0;
}

#ifdef ALLOW_NATIVE_MONITOR
int native_monitor;

static int set_native_monitor(int val, void *param)
{
    native_monitor = val;
    return 0;
}
#endif

#ifdef __sortix__
#define DEFAULT_MENU_KEY SDLK_END
#endif

#ifndef DEFAULT_MENU_KEY
# ifdef MACOSX_SUPPORT
#  define DEFAULT_MENU_KEY SDLK_F10
# else
#  define DEFAULT_MENU_KEY SDLK_F12
# endif
#endif

static const resource_int_t resources_int[] = {
    { "MenuKey", DEFAULT_MENU_KEY, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[0], set_ui_menukey, (void *)MENU_ACTION_NONE },
    { "MenuKeyUp", SDLK_UP, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[1], set_ui_menukey, (void *)MENU_ACTION_UP },
    { "MenuKeyDown", SDLK_DOWN, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[2], set_ui_menukey, (void *)MENU_ACTION_DOWN },
    { "MenuKeyLeft", SDLK_LEFT, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[3], set_ui_menukey, (void *)MENU_ACTION_LEFT },
    { "MenuKeyRight", SDLK_RIGHT, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[4], set_ui_menukey, (void *)MENU_ACTION_RIGHT },
    { "MenuKeySelect", SDLK_RETURN, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[5], set_ui_menukey, (void *)MENU_ACTION_SELECT },
    { "MenuKeyCancel", SDLK_BACKSPACE, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[6], set_ui_menukey, (void *)MENU_ACTION_CANCEL },
    { "MenuKeyExit", SDLK_ESCAPE, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[7], set_ui_menukey, (void *)MENU_ACTION_EXIT },
    { "MenuKeyMap", SDLK_m, RES_EVENT_NO, NULL,
      &sdl_ui_menukeys[8], set_ui_menukey, (void *)MENU_ACTION_MAP },
    { "SaveResourcesOnExit", 0, RES_EVENT_NO, NULL,
      &save_resources_on_exit, set_save_resources_on_exit, NULL },
    { "ConfirmOnExit", 0, RES_EVENT_NO, NULL,
      &confirm_on_exit, set_confirm_on_exit, NULL },
#ifdef ALLOW_NATIVE_MONITOR
    { "NativeMonitor", 0, RES_EVENT_NO, NULL,
      &native_monitor, set_native_monitor, NULL },
#endif
    RESOURCE_INT_LIST_END
};

void ui_sdl_quit(void)
{
    if (confirm_on_exit) {
        if (message_box("VICE QUESTION", "Do you really want to exit?", MESSAGE_YESNO) != 0) {
            return;
        }
    }

    if (save_resources_on_exit) {
        if (resources_save(NULL) < 0) {
            ui_error("Cannot save current settings.");
        }
    }
    exit(0);
}

/* Initialization  */
int ui_resources_init(void)
{
    DBG(("%s", __func__));
    if (resources_register_int(resources_int) < 0) {
        return -1;
    }

    if (machine_class != VICE_MACHINE_VSID) {
        return uistatusbar_init_resources();
    }
    return 0;
}

void ui_resources_shutdown(void)
{
    DBG(("%s", __func__));
    joy_arch_resources_shutdown();
    sdlkbd_resources_shutdown();
}

static const cmdline_option_t cmdline_options[] = {
    { "-menukey", SET_RESOURCE, 1, NULL, NULL, "MenuKey", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<key>", "Keycode of the menu activate key" },
    { "-menukeyup", SET_RESOURCE, 1, NULL, NULL, "MenuKeyUp", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<key>", "Keycode of the menu up key" },
    { "-menukeydown", SET_RESOURCE, 1, NULL, NULL, "MenuKeyDown", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<key>", "Keycode of the menu down key" },
    { "-menukeyleft", SET_RESOURCE, 1, NULL, NULL, "MenuKeyLeft", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<key>", "Keycode of the menu left key" },
    { "-menukeyright", SET_RESOURCE, 1, NULL, NULL, "MenuKeyRight", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<key>", "Keycode of the menu right key" },
    { "-menukeyselect", SET_RESOURCE, 1, NULL, NULL, "MenuKeySelect", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<key>", "Keycode of the menu select key" },
    { "-menukeycancel", SET_RESOURCE, 1, NULL, NULL, "MenuKeyCancel", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<key>", "Keycode of the menu cancel key" },
    { "-menukeyexit", SET_RESOURCE, 1, NULL, NULL, "MenuKeyExit", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<key>", "Keycode of the menu exit key" },
    { "-menukeymap", SET_RESOURCE, 1, NULL, NULL, "MenuKeyMap", NULL,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      "<key>", "Keycode of the menu map key" },
    { "-saveresourcesonexit", SET_RESOURCE, 0, NULL, NULL, "SaveResourcesOnExit", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Enable saving of the resources on exit" },
    { "+saveresourcesonexit", SET_RESOURCE, 0, NULL, NULL, "SaveResourcesOnExit", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Disable saving of the resources on exit" },
    { "-confirmonexit", SET_RESOURCE, 0, NULL, NULL, "ConfirmOnExit", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Enable confirm on exit" },
    { "+confirmonexit", SET_RESOURCE, 0, NULL, NULL, "ConfirmOnExit", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Disable confirm on exit" },
#ifdef ALLOW_NATIVE_MONITOR
    { "-nativemonitor", SET_RESOURCE, 0, NULL, NULL, "NativeMonitor", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Enable native monitor" },
    { "+nativemonitor", SET_RESOURCE, 0, NULL, NULL, "NativeMonitor", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Disable native monitor" },
#endif
    CMDLINE_LIST_END
};

static const cmdline_option_t statusbar_cmdline_options[] = {
    { "-statusbar", SET_RESOURCE, 0, NULL, NULL, "SDLStatusbar", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Enable statusbar" },
    { "+statusbar", SET_RESOURCE, 0, NULL, NULL, "SDLStatusbar", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Disable statusbar" },
    { "-kbdstatusbar", SET_RESOURCE, 0, NULL, NULL, "SDLKbdStatusbar", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Enable keyboard-status bar" },
    { "+kbdstatusbar", SET_RESOURCE, 0, NULL, NULL, "SDLKbdStatusbar", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING, IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Disable keyboard-status bar" },
    CMDLINE_LIST_END
};

int ui_cmdline_options_init(void)
{
    DBG(("%s", __func__));

    if (machine_class != VICE_MACHINE_VSID) {
        if (cmdline_register_options(statusbar_cmdline_options) < 0) {
            return -1;
        }
    }

    return cmdline_register_options(cmdline_options);
}

int ui_init(int *argc, char **argv)
{
    DBG(("%s", __func__));
    return 0;
}

int ui_init_finish(void)
{
    DBG(("%s", __func__));
    return 0;
}

int ui_init_finalize(void)
{
    DBG(("%s", __func__));

    if (!console_mode) {
        sdl_ui_init_finalize();
#ifndef USE_SDLUI2
        SDL_WM_SetCaption(sdl_active_canvas->viewport->title, "VICE");
#endif
        sdl_ui_ready = 1;
    }
    return 0;
}

void ui_shutdown(void)
{
    DBG(("%s", __func__));
    sdl_ui_file_selection_dialog_shutdown();
}

/* Print an error message.  */
void ui_error(const char *format, ...)
{
    va_list ap;
    char *tmp;

    va_start(ap, format);
    tmp = lib_mvsprintf(format, ap);
    va_end(ap);

    if (sdl_ui_ready) {
        message_box("VICE ERROR", tmp, MESSAGE_OK);
    } else {
        fprintf(stderr, "%s\n", tmp);
    }
    lib_free(tmp);
}

/* Let the user browse for a filename; display format as a titel */
char* ui_get_file(const char *format, ...)
{
    return NULL;
}

/* Drive related UI.  */
int ui_extend_image_dialog(void)
{
    if (message_box("VICE QUESTION", "Do you want to extend the disk image?", MESSAGE_YESNO) == 0) {
        return 1;
    }
    return 0;
}

/* Show a CPU JAM dialog.  */
ui_jam_action_t ui_jam_dialog(const char *format, ...)
{
    int retval;

    retval = message_box("VICE CPU JAM", "a CPU JAM has occured, choose the action to take", MESSAGE_CPUJAM);
    if (retval == 0) {
        return UI_JAM_HARD_RESET;
    }
    if (retval == 1) {
        return UI_JAM_MONITOR;
    }
    return UI_JAM_NONE;
}

/* Update all menu entries.  */
void ui_update_menus(void)
{
}

/* ----------------------------------------------------------------- */
/* uicolor.h */

int uicolor_alloc_color(unsigned int red, unsigned int green, unsigned int blue, unsigned long *color_pixel, uint8_t *pixel_return)
{
    DBG(("%s", __func__));
    return 0;
}

void uicolor_free_color(unsigned int red, unsigned int green, unsigned int blue, unsigned long color_pixel)
{
    DBG(("%s", __func__));
}

void uicolor_convert_color_table(unsigned int colnr, uint8_t *data, long color_pixel, void *c)
{
    DBG(("%s", __func__));
}

int uicolor_set_palette(struct video_canvas_s *c, const struct palette_s *palette)
{
    DBG(("%s", __func__));
    return 0;
}

/* ---------------------------------------------------------------------*/
/* vsidui_sdl.h */

/* These items are here so they can be used in generic UI
   code without vsidui.c being linked into all emulators. */
int sdl_vsid_state = 0;

static ui_draw_func_t vsid_draw_func = NULL;

void sdl_vsid_activate(void)
{
    sdl_vsid_state = SDL_VSID_ACTIVE | SDL_VSID_REPAINT;
}

void sdl_vsid_close(void)
{
    sdl_vsid_state = 0;
}

void sdl_vsid_draw_init(ui_draw_func_t func)
{
    vsid_draw_func = func;
}

void sdl_vsid_draw(void)
{
    if (vsid_draw_func) {
        vsid_draw_func();
    }
}

