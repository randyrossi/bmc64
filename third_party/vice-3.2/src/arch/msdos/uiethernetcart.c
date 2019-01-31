/*
 * uiethernetcart.c - Ethernet Cart UI interface for MS-DOS.
 *
 * Written by
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

#ifdef HAVE_RAWNET

#include <stdio.h>

#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uiethernetcart.h"

TUI_MENU_DEFINE_TOGGLE(ETHERNETCART_ACTIVE)
TUI_MENU_DEFINE_RADIO(ETHERNETCARTMode)
TUI_MENU_DEFINE_RADIO(ETHERNETCARTBase)

static TUI_MENU_CALLBACK(ethernetcart_mode_submenu_callback)
{
    int value;
    static char *s;

    resources_get_int("ETHERNETCARTMode", &value);
    if (value) {
        s = "RR-Net compatible";
    } else {
        s = "TFE compatible";
    }
    return s;
}

static tui_menu_item_def_t ethernetcart_mode_submenu[] = {
    { "_RR-Net compatible", NULL, radio_ETHERNETCARTMode_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_TFE compatible", NULL, radio_ETHERNETCARTMode_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(ethernetcart_base_submenu_callback)
{
    int value;
    static char s[100];

    resources_get_int("ETHERNETCARTBase", &value);
    sprintf(s, "$%X", value);
    return s;
}

static tui_menu_item_def_t ethernetcart_base64_submenu[] = {
    { "$DE00", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xde00, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE10", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xde10, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE20", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xde20, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE30", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xde30, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE40", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xde40, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE50", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xde50, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE60", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xde60, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE70", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xde70, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE80", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xde80, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DE90", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xde90, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DEA0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdea0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DEB0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdeb0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DEC0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdec0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DED0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xded0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DEE0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdee0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DEF0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdef0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF00", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdf00, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF10", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdf10, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF20", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdf20, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF30", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdf30, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF40", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdf40, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF50", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdf50, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF60", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdf60, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF70", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdf70, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF80", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdf80, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DF90", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdf90, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DFA0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdfa0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DFB0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdfb0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DFC0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdfc0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DFD0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdfd0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DFE0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdfe0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$DFF0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0xdff0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ethernetcart_base20_submenu[] = {
    { "$9800", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9800, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9810", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9810, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9820", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9820, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9830", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9830, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9840", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9840, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9850", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9850, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9860", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9860, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9870", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9870, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9880", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9880, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9890", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9890, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$98A0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x98a0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$98B0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x98b0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$98C0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x98c0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$98D0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x98d0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$98E0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x98e0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$98F0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x98f0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C00", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9c00, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C10", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9c10, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C20", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9c20, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C30", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9c30, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C40", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9c40, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C50", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9c50, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C60", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9c60, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C70", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9c70, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C80", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9c80, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9C90", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9c90, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9CA0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9ca0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9CB0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9cb0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9CC0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9cc0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9CD0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9cd0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9CE0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9ce0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "$9CF0", NULL, radio_ETHERNETCARTBase_callback,
      (void *)0x9cf0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ethernetcart_c64_menu_items[] = {
    { "_Enable Ethernet Cart:", "Emulate Ethernet Cartridge",
      toggle_ETHERNETCART_ACTIVE_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Ethernet Cart _mode:", "Select the mode of the Ethernet Cart",
      ethernetcart_mode_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, ethernetcart_mode_submenu,
      "Ethernet Cart mode" },
    { "Ethernet Cart _base:", "Select the base of the Ethernet Cart",
      ethernetcart_base_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, ethernetcart_base64_submenu,
      "Ethernet Cart base" },
    TUI_MENU_ITEM_DEF_LIST_END
};

static tui_menu_item_def_t ethernetcart_vic20_menu_items[] = {
    { "_Enable Ethernet Cart:", "Emulate Ethernet Cartridge",
      toggle_ETHERNETCART_ACTIVE_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Ethernet Cart _mode:", "Select the mode of the Ethernet Cart",
      ethernetcart_mode_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, ethernetcart_mode_submenu,
      "Ethernet Cart mode" },
    { "Ethernet Cart _base:", "Select the base of the Ethernet Cart",
      ethernetcart_base_submenu_callback, NULL, 7,
      TUI_MENU_BEH_CONTINUE, ethernetcart_base20_submenu,
      "Ethernet Cart base" },
    TUI_MENU_ITEM_DEF_LIST_END
};

void uiethernetcart_c64_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_ethernetcart_submenu;

    ui_ethernetcart_submenu = tui_menu_create("Ethernet Cart settings", 1);

    tui_menu_add(ui_ethernetcart_submenu, ethernetcart_c64_menu_items);

    tui_menu_add_submenu(parent_submenu, "_Ethernet Cart settings...",
                         "Ethernet Cart settings",
                         ui_ethernetcart_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}

void uiethernetcart_vic20_init(struct tui_menu *parent_submenu)
{
    tui_menu_t ui_ethernetcart_submenu;

    ui_ethernetcart_submenu = tui_menu_create("Ethernet Cart settings (MasC=uerade)", 1);

    tui_menu_add(ui_ethernetcart_submenu, ethernetcart_vic20_menu_items);

    tui_menu_add_submenu(parent_submenu, "_Ethernet Cart settings (MasC=uerade)...",
                         "Ethernet Cart settings",
                         ui_ethernetcart_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);
}

#endif
