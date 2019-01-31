/** \file   src/arch/unix/gui/uiethernetcart.c
 * \brief   Ethernet cartridge UI items
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  Bas Wassink <b.wassink@ziggo.nl>
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

#ifdef HAVE_RAWNET

#include <stdio.h>

#include "lib.h"
#include "machine.h"
#include "uilib.h"
#include "uimenu.h"
#include "util.h"

#include "uiethernetcart.h"

UI_MENU_DEFINE_TOGGLE(ETHERNETCART_ACTIVE)
UI_MENU_DEFINE_RADIO(ETHERNETCARTMode)
UI_MENU_DEFINE_RADIO(ETHERNETCARTBase)


#if 0
/** \brief  Callback to set the ethernet interface name
 */
static UI_CALLBACK(set_interface_name)
{
    char *name = util_concat(_("Name"), ":", NULL);

    /* FIXME: might actually make some sense to use the file browser on *nix systems which
              actually DO have /dev/eth0. linux doesn't however */
    /* uilib_select_dev((char *)UI_MENU_CB_PARAM, _("Ethernet interface"), UILIB_FILTER_ETH); */
    uilib_select_string((char *)UI_MENU_CB_PARAM, _("Ethernet interface"), name);
    lib_free(name);
}
#endif


/** \brief  Submenu to select the emulated ethernet cartridge
 */
static ui_menu_entry_t ethernetcart_mode_submenu[] = {
    { "TFE", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_ETHERNETCARTMode, (ui_callback_data_t)0, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "RR-Net", UI_MENU_TYPE_TICK,
      (ui_callback_t)radio_ETHERNETCARTMode, (ui_callback_data_t)1, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};


/** \brief  Submenu to select the I/O area the cart uses on C64
 */
static ui_menu_entry_t ethernetcart_c64_base_submenu[] = {
    { "$DExx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$DFxx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};


/** \brief  Submenu to select the I/O area the cart uses on VIC20
 */
static ui_menu_entry_t ethernetcart_vic20_base_submenu[] = {
    { "$98xx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { "$9cxx", UI_MENU_TYPE_NORMAL,
      NULL, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};


/** \brief  Ethernet emulation/cartridge submenu for C64
 */
ui_menu_entry_t ethernetcart_c64_submenu[] = {
    { N_("Cartridge enabled"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_ETHERNETCART_ACTIVE, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Cartridge mode"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ethernetcart_mode_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Cartridge I/O area"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ethernetcart_c64_base_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};

ui_menu_entry_t ethernetcart_vic20_submenu[] = {
    { N_("Cartridge enabled"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_ETHERNETCART_ACTIVE, NULL, NULL,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Cartridge mode"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ethernetcart_mode_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    { N_("Cartridge I/O area"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, ethernetcart_vic20_base_submenu,
      (ui_keysym_t)0, (ui_hotkey_modifier_t)0 },
    UI_MENU_ENTRY_LIST_END
};


/** \brief  Generate a submenu for selection of 16-bit integers
 *
 * \param[in]   callback    UI callback that sets the resource
 * \param[in]   base        starting value
 * \param[in]   step        step value
 * \param[in]   count       number of values to generate
 *
 * \return  heap-allocated submenu, free with uiethernetcart_io_submenu_shutdown()
 */
static ui_menu_entry_t *uiethernetcart_io_submenu_create(ui_callback_t callback,
                                                         unsigned int base,
                                                         unsigned int step,
                                                         unsigned int count)
{
    ui_menu_entry_t *menu;
    unsigned int i;

    menu = lib_calloc((size_t)(count + 1), sizeof *menu);
    for (i = 0; i < count; i++) {
        unsigned int addr = base + (step * i);
        menu[i].string = lib_msprintf("$%04X", addr, NULL);
        menu[i].type = UI_MENU_TYPE_TICK;
        menu[i].callback = callback;
        menu[i].callback_data = (ui_callback_data_t)(uint_to_void_ptr(addr));
    }
    return menu;
}


/** \brief  Clean up memory used by \a menu and its items
 *
 * \param[in,out]   menu    dynamically generated sub menu
 */
static void uiethernetcart_io_submenu_shutdown(ui_menu_entry_t *menu)
{
    if (menu != NULL) {
        int i = 0;

        while (menu[i].string != NULL) {
            lib_free(menu[i].string);
            i++;
        }
        lib_free(menu);
    }
}


/** \brief  Generate dynamic portions of the ethernetcart menu
 */
void uiethernetcart_menu_create(void)
{
    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fallthrough */
        case VICE_MACHINE_C64SC:    /* fallthrough */
        case VICE_MACHINE_C128:     /* fallthrough */
        case VICE_MACHINE_SCPU64:
            ethernetcart_c64_base_submenu[0].sub_menu =
                uiethernetcart_io_submenu_create(
                        (ui_callback_t)radio_ETHERNETCARTBase, 0xde00, 0x10, 16);
            ethernetcart_c64_base_submenu[1].sub_menu =
                uiethernetcart_io_submenu_create(
                        (ui_callback_t)radio_ETHERNETCARTBase, 0xdf00, 0x10, 16);
            break;
        case VICE_MACHINE_VIC20:
            ethernetcart_vic20_base_submenu[0].sub_menu =
                uiethernetcart_io_submenu_create(
                        (ui_callback_t)radio_ETHERNETCARTBase, 0x9800, 0x10, 16);
            ethernetcart_vic20_base_submenu[1].sub_menu =
                uiethernetcart_io_submenu_create(
                        (ui_callback_t)radio_ETHERNETCARTBase, 0x9c00, 0x10, 16);
            break;
        default:
            break;
    }
}

/** \brief  Clean up dynamically generated portions of the ethernet menu
 */
void uiethernetcart_menu_shutdown(void)
{
    switch (machine_class) {
        case VICE_MACHINE_C64:      /* fallthrough */
        case VICE_MACHINE_C64SC:    /* fallthrough */
        case VICE_MACHINE_C128:     /* fallthrough */
        case VICE_MACHINE_SCPU64:
            uiethernetcart_io_submenu_shutdown(
                    ethernetcart_c64_base_submenu[0].sub_menu);
            uiethernetcart_io_submenu_shutdown(
                    ethernetcart_c64_base_submenu[1].sub_menu);
            break;
        case VICE_MACHINE_VIC20:
            uiethernetcart_io_submenu_shutdown(
                    ethernetcart_vic20_base_submenu[0].sub_menu);
            uiethernetcart_io_submenu_shutdown(
                    ethernetcart_vic20_base_submenu[1].sub_menu);
            break;
        default:
            break;

    }
}
#endif
