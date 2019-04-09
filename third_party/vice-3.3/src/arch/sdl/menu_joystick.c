/*
 * menu_joystick.c - Joystick menu for SDL UI.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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
#include "types.h"

#include "vice_sdl.h"

#include "joy.h"
#include "joystick.h"
#include "kbd.h"
#include "lib.h"
#include "menu_common.h"
#include "menu_joystick.h"
#include "resources.h"
#include "uimenu.h"
#include "uipoll.h"
#include "userport_joystick.h"

UI_MENU_DEFINE_RADIO(JoyDevice1)
UI_MENU_DEFINE_RADIO(JoyDevice2)
UI_MENU_DEFINE_RADIO(JoyDevice3)
UI_MENU_DEFINE_RADIO(JoyDevice4)
UI_MENU_DEFINE_RADIO(JoyDevice5)

#define VICE_SDL_JOYSTICK_DEVICE_MENU(port)                              \
    static const ui_menu_entry_t joystick_port##port##_device_menu[] = { \
        { "None",                                                        \
          MENU_ENTRY_RESOURCE_RADIO,                                     \
          radio_JoyDevice##port##_callback,                              \
          (ui_callback_data_t)JOYDEV_NONE },                             \
        { "Numpad",                                                      \
          MENU_ENTRY_RESOURCE_RADIO,                                     \
          radio_JoyDevice##port##_callback,                              \
          (ui_callback_data_t)JOYDEV_NUMPAD },                           \
        { "Keyset 1",                                                    \
          MENU_ENTRY_RESOURCE_RADIO,                                     \
          radio_JoyDevice##port##_callback,                              \
          (ui_callback_data_t)JOYDEV_KEYSET1 },                          \
        { "Keyset 2",                                                    \
          MENU_ENTRY_RESOURCE_RADIO,                                     \
          radio_JoyDevice##port##_callback,                              \
          (ui_callback_data_t)JOYDEV_KEYSET2 },                          \
        { "Joystick",                                                    \
          MENU_ENTRY_RESOURCE_RADIO,                                     \
          radio_JoyDevice##port##_callback,                              \
          (ui_callback_data_t)JOYDEV_JOYSTICK },                         \
        SDL_MENU_LIST_END                                                \
    };

VICE_SDL_JOYSTICK_DEVICE_MENU(1)
VICE_SDL_JOYSTICK_DEVICE_MENU(2)
VICE_SDL_JOYSTICK_DEVICE_MENU(3)
VICE_SDL_JOYSTICK_DEVICE_MENU(4)
VICE_SDL_JOYSTICK_DEVICE_MENU(5)

UI_MENU_DEFINE_TOGGLE(KeySetEnable)
UI_MENU_DEFINE_TOGGLE(JoyOpposite)

static UI_MENU_CALLBACK(custom_swap_ports_callback)
{
    if (activated) {
        sdljoy_swap_ports();
    }
    return sdljoy_get_swap_ports() ? MENU_CHECKMARK_CHECKED_STRING : NULL;
}

static UI_MENU_CALLBACK(custom_swap_userport_joystick_ports)
{
    if (activated) {
        sdljoy_swap_userport_ports();
    }
    return sdljoy_get_swap_userport_ports() ? MENU_CHECKMARK_CHECKED_STRING : NULL;
}

static UI_MENU_CALLBACK(custom_keyset_callback)
{
    SDL_Event e;
    int previous;

    if (resources_get_int((const char *)param, &previous)) {
        return sdl_menu_text_unknown;
    }

    if (activated) {
        e = sdl_ui_poll_event("key", (const char *)param, SDL_POLL_KEYBOARD | SDL_POLL_MODIFIER, 5);

        if (e.type == SDL_KEYDOWN) {
            resources_set_int((const char *)param, (int)SDL2x_to_SDL1x_Keys(e.key.keysym.sym));
        }
    } else {
        return SDL_GetKeyName(SDL1x_to_SDL2x_Keys(previous));
    }
    return NULL;
}

static const ui_menu_entry_t define_keyset_menu[] = {
    { "Keyset 1 Up",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet1North" },
    { "Keyset 1 Down",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet1South" },
    { "Keyset 1 Left",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet1West" },
    { "Keyset 1 Right",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet1East" },
    { "Keyset 1 Fire",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet1Fire" },
    SDL_MENU_ITEM_SEPARATOR,
    { "Keyset 2 Up",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet2North" },
    { "Keyset 2 Down",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet2South" },
    { "Keyset 2 Left",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet2West" },
    { "Keyset 2 Right",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet2East" },
    { "Keyset 2 Fire",
      MENU_ENTRY_DIALOG,
      custom_keyset_callback,
      (ui_callback_data_t)"KeySet2Fire" },
    SDL_MENU_LIST_END
};

#ifdef HAVE_SDL_NUMJOYSTICKS
static const char *joy_pin[] = {
    "Up",
    "Down",
    "Left",
    "Right",
    "Fire"
};

static UI_MENU_CALLBACK(custom_joymap_callback)
{
    char *target = NULL;
    SDL_Event e;
    int pin, port;

    if (activated) {
        pin = (vice_ptr_to_int(param)) & 7;
        port = (vice_ptr_to_int(param)) >> 4;

        target = lib_msprintf("Port %i %s", port + 1, joy_pin[pin]);
        e = sdl_ui_poll_event("joystick", target, SDL_POLL_JOYSTICK, 5);
        lib_free(target);

        switch (e.type) {
            case SDL_JOYAXISMOTION:
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYHATMOTION:
                sdljoy_set_joystick(e, port, (1 << pin));
                break;
            default:
                break;
        }
    }

    return NULL;
}

#define VICE_SDL_JOYSTICK_MAPPING_MENU(port)                       \
    static const ui_menu_entry_t define_joy ## port ## _menu[] = { \
        { "Up",                                                    \
          MENU_ENTRY_DIALOG,                                       \
          custom_joymap_callback,                                  \
          (ui_callback_data_t)(0 | ((port - 1) << 4)) },           \
        { "Down",                                                  \
          MENU_ENTRY_DIALOG,                                       \
          custom_joymap_callback,                                  \
          (ui_callback_data_t)(1 | ((port - 1) << 4)) },           \
        { "Left",                                                  \
          MENU_ENTRY_DIALOG,                                       \
          custom_joymap_callback,                                  \
          (ui_callback_data_t)(2 | ((port - 1) << 4)) },           \
        { "Right",                                                 \
          MENU_ENTRY_DIALOG,                                       \
          custom_joymap_callback,                                  \
          (ui_callback_data_t)(3 | ((port - 1) << 4)) },           \
        { "Fire",                                                  \
          MENU_ENTRY_DIALOG,                                       \
          custom_joymap_callback,                                  \
          (ui_callback_data_t)(4 | ((port - 1) << 4)) },           \
        SDL_MENU_LIST_END                                          \
    };

VICE_SDL_JOYSTICK_MAPPING_MENU(1)
VICE_SDL_JOYSTICK_MAPPING_MENU(2)
VICE_SDL_JOYSTICK_MAPPING_MENU(3)
VICE_SDL_JOYSTICK_MAPPING_MENU(4)
VICE_SDL_JOYSTICK_MAPPING_MENU(5)

static UI_MENU_CALLBACK(custom_joy_misc_callback)
{
    char *target = NULL;
    SDL_Event e;

    if (activated) {
        e = sdl_ui_poll_event("joystick", (vice_ptr_to_int(param)) ? "Map" : "Menu activate", SDL_POLL_JOYSTICK, 5);
        lib_free(target);

        switch (e.type) {
            case SDL_JOYAXISMOTION:
            case SDL_JOYBUTTONDOWN:
            case SDL_JOYHATMOTION:
                sdljoy_set_extra(e, vice_ptr_to_int(param));
                break;
            default:
                break;
        }
    }

    return NULL;
}

UI_MENU_DEFINE_SLIDER(JoyThreshold, 0, 32767)
UI_MENU_DEFINE_SLIDER(JoyFuzz, 0, 32767)

static const ui_menu_entry_t define_joy_misc_menu[] = {
    { "Menu activate",
      MENU_ENTRY_DIALOG,
      custom_joy_misc_callback,
      (ui_callback_data_t)0 },
    { "Map",
      MENU_ENTRY_DIALOG,
      custom_joy_misc_callback,
      (ui_callback_data_t)1 },
    SDL_MENU_ITEM_SEPARATOR,
    { "Threshold",
      MENU_ENTRY_RESOURCE_INT,
      slider_JoyThreshold_callback,
      (ui_callback_data_t)"Set joystick threshold (0 - 32767)" },
    { "Fuzz",
      MENU_ENTRY_RESOURCE_INT,
      slider_JoyFuzz_callback,
      (ui_callback_data_t)"Set joystick fuzz (0 - 32767)" },
    SDL_MENU_LIST_END
};
#endif

const ui_menu_entry_t joystick_menu[] = {
    { "Joystick device 1",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port1_device_menu },
    { "Joystick device 2",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port2_device_menu },
    { "Swap joystick ports",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_swap_ports_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Allow keyset joystick",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_KeySetEnable_callback,
      NULL },
    { "Define keysets",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_keyset_menu },
#ifdef HAVE_SDL_NUMJOYSTICKS
    SDL_MENU_ITEM_SEPARATOR,
    { "Joystick 1 mapping", /* TODO better name */
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy1_menu },
    { "Joystick 2 mapping", /* TODO better name */
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy2_menu },
    { "Joystick extra options", /* TODO better name */
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy_misc_menu },
#endif
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_TOGGLE(UserportJoy)
UI_MENU_DEFINE_RADIO(UserportJoyType)

static const ui_menu_entry_t joystick_extra_joy_type_menu[] = {
    { "CGA/Protovision userport adapter",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_UserportJoyType_callback,
      (ui_callback_data_t)USERPORT_JOYSTICK_CGA },
    { "DXS/HIT userport adapter",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_UserportJoyType_callback,
      (ui_callback_data_t)USERPORT_JOYSTICK_HIT },
    { "Kingsoft userport adapter",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_UserportJoyType_callback,
      (ui_callback_data_t)USERPORT_JOYSTICK_KINGSOFT },
    { "Starbyte userport adapter",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_UserportJoyType_callback,
      (ui_callback_data_t)USERPORT_JOYSTICK_STARBYTE },
    { "PET userport adapter",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_UserportJoyType_callback,
      (ui_callback_data_t)USERPORT_JOYSTICK_PET },
    { "Hummer userport adapter",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_UserportJoyType_callback,
      (ui_callback_data_t)USERPORT_JOYSTICK_HUMMER },
    { "OEM userport adapter",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_UserportJoyType_callback,
      (ui_callback_data_t)USERPORT_JOYSTICK_OEM },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t joystick_extra_joy_type_no_hit_menu[] = {
    { "CGA/Protovision userport adapter",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_UserportJoyType_callback,
      (ui_callback_data_t)USERPORT_JOYSTICK_CGA },
    { "PET userport adapter",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_UserportJoyType_callback,
      (ui_callback_data_t)USERPORT_JOYSTICK_PET },
    { "HUMMER userport adapter",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_UserportJoyType_callback,
      (ui_callback_data_t)USERPORT_JOYSTICK_HUMMER },
    { "OEM userport adapter",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_UserportJoyType_callback,
      (ui_callback_data_t)USERPORT_JOYSTICK_OEM },
    SDL_MENU_LIST_END
};

static const ui_menu_entry_t joystick_extra_joy_type_no_hit_cga_menu[] = {
    { "PET userport adapter",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_UserportJoyType_callback,
      (ui_callback_data_t)USERPORT_JOYSTICK_PET },
    { "HUMMER userport adapter",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_UserportJoyType_callback,
      (ui_callback_data_t)USERPORT_JOYSTICK_HUMMER },
    { "OEM userport adapter",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_UserportJoyType_callback,
      (ui_callback_data_t)USERPORT_JOYSTICK_OEM },
    SDL_MENU_LIST_END
};

const ui_menu_entry_t joystick_c64_menu[] = {
    { "Joystick device 1",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port1_device_menu },
    { "Joystick device 2",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port2_device_menu },
    { "Userport joystick adapter port 1",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port3_device_menu },
    { "Userport joystick adapter port 2",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port4_device_menu },
    { "Swap joystick ports",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_swap_ports_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Allow opposite directions",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_JoyOpposite_callback,
      NULL },
    { "Allow keyset joystick",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_KeySetEnable_callback,
      NULL },
    { "Define keysets",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_keyset_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Userport joystick adapter",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UserportJoy_callback,
      NULL },
    { "Userport joystick adapter type",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_extra_joy_type_menu },
    { "Swap userport joystick ports",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_swap_userport_joystick_ports,
      NULL },
#ifdef HAVE_SDL_NUMJOYSTICKS
    SDL_MENU_ITEM_SEPARATOR,
    { "Joystick 1 mapping", /* TODO better name */
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy1_menu },
    { "Joystick 2 mapping", /* TODO better name */
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy2_menu },
    { "Userport joystick 1 mapping",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy3_menu },
    { "Userport joystick 2 mapping",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy4_menu },
    { "Extra joystick options",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy_misc_menu },
#endif
    SDL_MENU_LIST_END
};

const ui_menu_entry_t joystick_c64dtv_menu[] = {
    { "Joystick device 1",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port1_device_menu },
    { "Joystick device 2",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port2_device_menu },
    { "Userport joystick adapter port",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port3_device_menu },
    { "Swap joystick ports",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_swap_ports_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Allow opposite directions",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_JoyOpposite_callback,
      NULL },
    { "Allow keyset joystick",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_KeySetEnable_callback,
      NULL },
    { "Define keysets",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_keyset_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Userport joystick adapter",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UserportJoy_callback,
      NULL },
#ifdef HAVE_SDL_NUMJOYSTICKS
    SDL_MENU_ITEM_SEPARATOR,
    { "Joystick 1 mapping", /* TODO better name */
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy1_menu },
    { "Joystick 2 mapping", /* TODO better name */
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy2_menu },
    { "Userport joystick mapping",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy3_menu },
    { "Extra joystick options",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy_misc_menu },
#endif
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_TOGGLE(SIDCartJoy)

const ui_menu_entry_t joystick_plus4_menu[] = {
    { "Joystick device 1",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port1_device_menu },
    { "Joystick device 2",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port2_device_menu },
    { "Userport joystick adapter port 1",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port3_device_menu },
    { "Userport joystick adapter port 2",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port4_device_menu },
    { "Joystick device in SIDcart joytick port",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port5_device_menu },
    { "Swap joystick ports",
      MENU_ENTRY_OTHER_TOGGLE,
      custom_swap_ports_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Allow opposite directions",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_JoyOpposite_callback,
      NULL },
    { "Allow keyset joystick",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_KeySetEnable_callback,
      NULL },
    { "Define keysets",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_keyset_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Userport joystick adapter",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UserportJoy_callback,
      NULL },
    { "Userport joystick adapter type",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_extra_joy_type_no_hit_cga_menu },
    { "SID Cart Joystick",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SIDCartJoy_callback,
      NULL },
#ifdef HAVE_SDL_NUMJOYSTICKS
    SDL_MENU_ITEM_SEPARATOR,
    { "Joystick 1 mapping", /* TODO better name */
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy1_menu },
    { "Joystick 2 mapping", /* TODO better name */
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy2_menu },
    { "Userport joystick 1 mapping",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy3_menu },
    { "Userport joystick 2 mapping",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy4_menu },
    { "SID Cart joystick mapping",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy5_menu },
    { "Extra joystick options",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy_misc_menu },
#endif
    SDL_MENU_LIST_END
};

const ui_menu_entry_t joystick_vic20_menu[] = {
    { "Joystick device",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port1_device_menu },
    { "Userport joystick adapter port 1",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port3_device_menu },
    { "Userport joystick adapter port 2",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port4_device_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Allow opposite directions",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_JoyOpposite_callback,
      NULL },
    { "Allow keyset joystick",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_KeySetEnable_callback,
      NULL },
    { "Define keysets",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_keyset_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Userport joystick adapter",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UserportJoy_callback,
      NULL },
    { "Userport joystick adapter type",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_extra_joy_type_no_hit_menu },
#ifdef HAVE_SDL_NUMJOYSTICKS
    SDL_MENU_ITEM_SEPARATOR,
    { "Joystick mapping",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy1_menu },
    { "Userport joystick 1 mapping",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy3_menu },
    { "Userport joystick 2 mapping",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy4_menu },
    { "Extra joystick options",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy_misc_menu },
#endif
    SDL_MENU_LIST_END
};

const ui_menu_entry_t joystick_userport_only_menu[] = {
    { "Userport joystick adapter port 1",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port3_device_menu },
    { "Userport joystick adapter port 2",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_port4_device_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Allow opposite directions",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_JoyOpposite_callback,
      NULL },
    { "Allow keyset joystick",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_KeySetEnable_callback,
      NULL },
    { "Define keysets",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_keyset_menu },
    SDL_MENU_ITEM_SEPARATOR,
    { "Userport joystick adapter",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_UserportJoy_callback,
      NULL },
    { "Userport joystick adapter type",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)joystick_extra_joy_type_no_hit_menu },
#ifdef HAVE_SDL_NUMJOYSTICKS
    SDL_MENU_ITEM_SEPARATOR,
    { "Userport joystick 1 mapping",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy3_menu },
    { "Userport joystick 2 mapping",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy4_menu },
    { "Extra joystick options",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)define_joy_misc_menu },
#endif
    SDL_MENU_LIST_END
};
