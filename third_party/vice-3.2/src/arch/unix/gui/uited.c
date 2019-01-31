/*
 * uited.c
 *
 * Written by
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

#include <stdio.h>
#include <string.h>

#include "fullscreenarch.h"
#include "lib.h"
#include "machine.h"
#include "palette.h"
#include "resources.h"
#include "ted.h"
#include "uiapi.h"
#include "uimenu.h"
#include "uipalette.h"
#include "uited.h"
#include "uifullscreen-menu.h"
#include "util.h"
#include "video.h"

#ifdef HAVE_OPENGL_SYNC
#include <stdlib.h>             /* strtol() */
#include "openGL_sync.h"
#endif

UI_FULLSCREEN(TED, KEYSYM_d)

static UI_CALLBACK(radio_TEDPaletteFile)
{
    ui_select_palette(w, CHECK_MENUS, UI_MENU_CB_PARAM, "TED");
}

static ui_menu_entry_t *attach_palette_submenu;

static ui_menu_entry_t palette_submenu[] = {
    { N_("Internal"), UI_MENU_TYPE_TICK,
     (ui_callback_t)radio_TEDPaletteFile, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { "", UI_MENU_TYPE_NONE,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Load custom"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)ui_load_palette, (ui_callback_data_t)"TED", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

static ui_menu_entry_t ui_palette_entry = {
    NULL, UI_MENU_TYPE_TICK,
    (ui_callback_t)radio_TEDPaletteFile, (ui_callback_data_t)0, NULL,
    (ui_keysym_t)0, (ui_hotkey_modifier_t)0
};

static int countgroup(palette_info_t *palettelist, char *chip)
{
    int num = 0;
    while(palettelist->name) {
        /* printf("name:%s file:%s chip:%s\n",palettelist->name,palettelist->file,palettelist->chip); */
        if (palettelist->chip && !strcmp(palettelist->chip, chip)) {
            num++;
        }
        palettelist++;
    }
    return num;
}

static void makegroup(palette_info_t *palettelist, ui_menu_entry_t *entry, char *chip)
{
    while(palettelist->name) {
        if (palettelist->chip && !strcmp(palettelist->chip, chip)) {
            ui_palette_entry.string = palettelist->name;
            ui_palette_entry.callback_data = (ui_callback_data_t)palettelist->file;
            memcpy(entry, &ui_palette_entry, sizeof(ui_menu_entry_t));
            entry++;
        }
        palettelist++;
    }
    memset(entry, 0, sizeof(ui_menu_entry_t));
}

static void uipalette_menu_create(void)
{
    int num;
    palette_info_t *palettelist = palette_get_info_list();

    num = countgroup(palettelist, "TED");
    /* printf("num:%d\n",num); */
    attach_palette_submenu = lib_malloc(sizeof(ui_menu_entry_t) * (num + 1));
    makegroup(palettelist, attach_palette_submenu, "TED");
    palette_submenu[2].sub_menu = attach_palette_submenu;
}

static void uipalette_menu_shutdown(void)
{
    if (attach_palette_submenu) {
        lib_free(attach_palette_submenu);
        attach_palette_submenu = NULL;
    }
}

UI_MENU_DEFINE_RADIO(TEDBorderMode)

static ui_menu_entry_t bordermode_submenu[] = {
    { N_("Normal"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_TEDBorderMode, (ui_callback_data_t)TED_NORMAL_BORDERS, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Full"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_TEDBorderMode, (ui_callback_data_t)TED_FULL_BORDERS, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Debug"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_TEDBorderMode, (ui_callback_data_t)TED_DEBUG_BORDERS, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("None"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_TEDBorderMode, (ui_callback_data_t)TED_NO_BORDERS, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

UI_MENU_DEFINE_RADIO(TEDFilter)

static ui_menu_entry_t renderer_submenu[] = {
    { N_("Unfiltered"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_TEDFilter, (ui_callback_data_t)VIDEO_FILTER_NONE, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("CRT emulation"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_TEDFilter, (ui_callback_data_t)VIDEO_FILTER_CRT, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Scale2x"), UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_TEDFilter, (ui_callback_data_t)VIDEO_FILTER_SCALE2X, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

#define NOTHING(x) x

UI_MENU_DEFINE_TOGGLE(TEDDoubleSize)
UI_MENU_DEFINE_TOGGLE(TEDDoubleScan)
UI_MENU_DEFINE_TOGGLE(TEDVideoCache)
UI_MENU_DEFINE_TOGGLE(TEDAudioLeak)

#ifdef HAVE_HWSCALE
UI_MENU_DEFINE_TOGGLE_COND(TEDHwScale, HwScalePossible, NOTHING)
#endif

#ifndef USE_GNOMEUI
UI_MENU_DEFINE_TOGGLE(UseXSync)
#endif

#ifdef HAVE_HWSCALE
static int get_aspect_enabled(int m)
{
    int n;
    resources_get_int("TEDHwScale", &n);
    return n && m;
}
UI_MENU_DEFINE_TOGGLE_COND(KeepAspectRatio, TEDHwScale, NOTHING)
UI_MENU_DEFINE_TOGGLE_COND(TrueAspectRatio, KeepAspectRatio, get_aspect_enabled)
#ifndef USE_GNOMEUI
#ifdef HAVE_XVIDEO
extern UI_CALLBACK(set_custom_aspect_ratio);
#endif
#endif /* USE_GNOMEUI */
#endif

#ifdef USE_UI_THREADS
static int get_hw_scale(int m)
{
    int n;
    resources_get_int("TEDHwScale", &n);
    return n && m;
}
UI_MENU_DEFINE_TOGGLE_COND(AlphaBlending, TEDHwScale, get_hw_scale)
#endif

#ifdef HAVE_OPENGL_SYNC
UI_MENU_DEFINE_TOGGLE_COND(openGL_sync, openGL_no_sync, openGL_available)

static UI_CALLBACK(openGL_set_desktoprefresh)
{
    char *enter_refresh_rate = util_concat(_("Enter refresh rate"), " (Hz): ", NULL);
    char *refresh_rate = util_concat(_("Refresh rate"), ": ", NULL);

    if (!CHECK_MENUS) {
        float f;
        char *buf = lib_calloc(sizeof(char), 10);

        sprintf(buf, "%.0f", openGL_get_canvas_refreshrate());
        ui_input_string(refresh_rate, enter_refresh_rate, buf, 10);
        f = (float) strtol(buf, NULL, 10);
        openGL_set_canvas_refreshrate(f);
        lib_free(buf);
        lib_free(enter_refresh_rate);
        lib_free(refresh_rate);
    } else {
        if (openGL_available(0) && openGL_sync_enabled()) {
            ui_menu_set_sensitive(w, 1);
        } else {
            ui_menu_set_sensitive(w, 0);
        }
    }
}
#endif

ui_menu_entry_t ted_submenu[] = {
    { N_("Double size"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_TEDDoubleSize, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Double scan"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_TEDDoubleScan, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Video cache"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_TEDVideoCache, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Colors"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, palette_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Render filter"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, renderer_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Border mode"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, bordermode_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Audio leak emulation"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_TEDAudioLeak, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifdef HAVE_HWSCALE
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Hardware scaling"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_TEDHwScale, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Keep aspect ratio"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_KeepAspectRatio, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("True aspect ratio"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_TrueAspectRatio, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#ifndef USE_GNOMEUI
#ifdef HAVE_XVIDEO
    { N_("Set custom aspect ratio"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)set_custom_aspect_ratio, (ui_callback_data_t)"AspectRatio", NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif /* HAVE_XVIDEO */
#endif /* USE_GNOMEUI */
#endif /* HAVE_HWSCALE */
#ifdef USE_UI_THREADS
    { N_("Alpha Blending"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_AlphaBlending, NULL, NULL,
      KEYSYM_v, UI_HOTMOD_META },
#endif
#ifdef HAVE_OPENGL_SYNC
    UI_MENU_ENTRY_SEPERATOR,
    { N_("OpenGL Rastersynchronization"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_openGL_sync, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Desktop Refreshrate"), UI_MENU_TYPE_DOTS,
      (ui_callback_t)openGL_set_desktoprefresh, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
#ifdef HAVE_FULLSCREEN
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Fullscreen settings"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, fullscreen_menuTED,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
#ifndef USE_GNOMEUI
    UI_MENU_ENTRY_SEPERATOR,
    { N_("Use XSync()"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_UseXSync, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
#endif
    UI_MENU_ENTRY_LIST_END
};

void uited_menu_create(void)
{
    uipalette_menu_create();
    UI_FULLSCREEN_MENU_CREATE(TED)
}

void uited_menu_shutdown(void)
{
    uipalette_menu_shutdown();
    UI_FULLSCREEN_MENU_SHUTDOWN(TED);
}
