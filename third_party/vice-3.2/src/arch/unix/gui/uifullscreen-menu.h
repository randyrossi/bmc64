/*
 * fullscreen-uimenu.h
 *
 * Written by
 *   pottendo (pottendo@utanet.at)
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

#if defined (HAVE_FULLSCREEN) && defined (USE_XF86_EXTENSIONS)

#ifdef USE_XF86_VIDMODE_EXT
#define __VIDMODE__                                          \
/* Translators: 'VidMode' must remain in the beginning       \
   of the translation e.g. German: "VidMode Aufloesungen" */ \
    { N_("VidMode Resolutions"), UI_MENU_TYPE_NORMAL, (ui_callback_t)NULL, NULL, NULL, \
        (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },

#define VIDMODE_RADIO(__CHIP__) UI_MENU_DEFINE_RADIO(__CHIP__##VidmodeFullscreenMode);

#define VIDMODE_DEVICE_SUBMENU(__CHIP__)                                                  \
{ "Vidmode", UI_MENU_TYPE_TICK,                                                           \
  (ui_callback_t)radio_##__CHIP__##FullscreenDevice, (ui_callback_data_t)"Vidmode", NULL, \
  (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },

#define VIDMODE_FULLSCREEN_MENU_CREATE(__CHIP__) fullscreen_mode_callback("Vidmode", (void *)radio_##__CHIP__##VidmodeFullscreenMode);

#else

#define __VIDMODE__
#define VIDMODE_RADIO(__CHIP__)
#define VIDMODE_DEVICE_SUBMENU(__CHIP__)
#define VIDMODE_FULLSCREEN_MENU_CREATE(__CHIP__)

#endif  /* USE_XF86_VIDMODE_EXT */

#ifdef HAVE_XRANDR 
#define __XRANDR__                                                            \
    /* Translators: `XRandR' must remain in the beginning of the translation: \
       e.g. German: "XRandR Aufloesungen" */                                  \
    { N_("XRandR Resolutions"), UI_MENU_TYPE_NORMAL, (ui_callback_t)NULL, NULL, NULL, \
    (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },

#define XRANDR_RADIO(__CHIP__) UI_MENU_DEFINE_RADIO(__CHIP__##XRANDRFullscreenMode);

#define XRANDR_DEVICE_SUBMENU(__CHIP__)                                                  \
{ "XRandR", UI_MENU_TYPE_TICK,                                                           \
  (ui_callback_t)radio_##__CHIP__##FullscreenDevice, (ui_callback_data_t)"XRANDR", NULL, \
  (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },

#define XRANDR_FULLSCREEN_MENU_CREATE(__CHIP__) fullscreen_mode_callback("XRANDR", (void *)radio_##__CHIP__##XRANDRFullscreenMode);

#else

#define __XRANDR__
#define XRANDR_RADIO(__CHIP__)
#define XRANDR_DEVICE_SUBMENU(__CHIP__)
#define XRANDR_FULLSCREEN_MENU_CREATE(__CHIP__)

#endif  /* HAVE_XRANDR */

#define FULLSCREENDEVICE_SUBMENU(__CHIP__)                           \
static ui_menu_entry_t set_fullscreen_device_submenu##__CHIP__[] = { \
  VIDMODE_DEVICE_SUBMENU(__CHIP__)                                   \
  XRANDR_DEVICE_SUBMENU(__CHIP__)                                    \
  UI_MENU_ENTRY_LIST_END                                             \
};

#define UI_FULLSCREEN(__CHIP__, __HOTKEY__)                              \
UI_MENU_DEFINE_TOGGLE(__CHIP__##Fullscreen)                              \
UI_MENU_DEFINE_TOGGLE(__CHIP__##FullscreenStatusbar)                     \
UI_MENU_DEFINE_STRING_RADIO(__CHIP__##FullscreenDevice)                  \
VIDMODE_RADIO(__CHIP__)                                                  \
XRANDR_RADIO(__CHIP__)                                                   \
FULLSCREENDEVICE_SUBMENU(__CHIP__)                                       \
                                                                         \
ui_menu_entry_t fullscreen_menu##__CHIP__[] =                            \
{                                                                        \
    { N_("Enable fullscreen"), UI_MENU_TYPE_TICK,                        \
      (ui_callback_t)toggle_##__CHIP__##Fullscreen, NULL, NULL,          \
      __HOTKEY__, UI_HOTMOD_META },                                      \
    { N_("Show Statusbar/Menu"), UI_MENU_TYPE_TICK,                      \
      (ui_callback_t)toggle_##__CHIP__##FullscreenStatusbar, NULL, NULL, \
      KEYSYM_b, UI_HOTMOD_META },                                        \
    { N_("Fullscreen device"), UI_MENU_TYPE_NORMAL,                      \
      NULL, NULL, set_fullscreen_device_submenu##__CHIP__,               \
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },                         \
      __VIDMODE__                                                        \
      __XRANDR__                                                         \
    UI_MENU_ENTRY_LIST_END                                               \
};

#define UI_FULLSCREEN_MENU_CREATE(__CHIP__)  \
    VIDMODE_FULLSCREEN_MENU_CREATE(__CHIP__) \
    XRANDR_FULLSCREEN_MENU_CREATE(__CHIP__)  \
    fullscreen_menu_create(fullscreen_menu##__CHIP__);

#define UI_FULLSCREEN_MENU_SHUTDOWN(__CHIP__) fullscreen_menu_shutdown(fullscreen_menu##__CHIP__);

#else

#define UI_FULLSCREEN(__CHIP__,__HOTKEY__)
#define UI_FULLSCREEN_MENU_CREATE(__CHIP__)
#define UI_FULLSCREEN_MENU_SHUTDOWN(__CHIP__)

#endif  /* USE_XF86_EXTENSIONS && HAVE_FULLSCREEN */
