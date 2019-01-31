/*
 * vicemenu.cc - Implementation of the BeVICE's menubar
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *  Marcus Sutton <loggedoubt@gmail.com>
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

#include <Menu.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <stdio.h>

#include "cartridge.h"
#include "constants.h"
#include "joyport.h"
#include "machine.h"
#include "vicemenu.h"

extern "C" {
#include "lib.h"
}

static BMenu *vsid_tune_menu;
static joyport_desc_t *(*get_devices)(int port) = NULL;
static char *(*get_name)(int port) = NULL;
static int joyport_ports[JOYPORT_MAX_PORTS];

static cartridge_info_t *(*get_carts)(void) = NULL;

void vicemenu_set_cart_func(cartridge_info_t *(*cgil)(void))
{
    get_carts = cgil;
}

void vicemenu_set_joyport_func(joyport_desc_t *(*gd)(int port), char *(*gn)(int port), int port1, int port2, int port3, int port4, int port5)
{
    get_devices = gd;
    get_name = gn;
    joyport_ports[JOYPORT_1] = port1;
    joyport_ports[JOYPORT_2] = port2;
    joyport_ports[JOYPORT_3] = port3;
    joyport_ports[JOYPORT_4] = port4;
    joyport_ports[JOYPORT_5] = port5;
}

void vicemenu_free_tune_menu(void)
{
    BMenuItem *mi;

    while (mi = vsid_tune_menu->FindItem(MENU_VSID_TUNE)) {
        vsid_tune_menu->RemoveItem(mi);
        delete mi;
    }
}

void vicemenu_tune_menu_add(int tune)
{
    char menustr[32];
    char shortcut;
    BMessage *msg;

    if (tune > 0) { 
        sprintf(menustr, "Tune %d", tune);
        shortcut = '0' + tune;
    } else {
        /*the default tune indicator */
        shortcut = '0';
        sprintf(menustr, "Default (%d)", -tune);
        tune = -tune;
    }

    msg = new BMessage(MENU_VSID_TUNE);
    msg->AddInt32("nr", tune);

    if (tune < 10) {
        vsid_tune_menu->AddItem(new BMenuItem(menustr, msg, shortcut));
    } else {
        vsid_tune_menu->AddItem(new BMenuItem(menustr, msg));
    }
}

BMenuBar *menu_create(int machine_class, int window_nr)
{
    BMenuBar *menubar;
    BMenu *uppermenu, *menu, *submenu, *extsubmenu;
    BMenuItem *item;
    uint32 i;
    joyport_desc_t *devices_port_1 = NULL;
    joyport_desc_t *devices_port_2 = NULL;
    joyport_desc_t *devices_port_3 = NULL;
    joyport_desc_t *devices_port_4 = NULL;
    joyport_desc_t *devices_port_5 = NULL;
    cartridge_info_t *cartlist = NULL;

    char *tmp_text = NULL;

    menubar = new BMenuBar(BRect(0, 0, 10, 10), "Menubar");

    /* create the FILE menu */
    uppermenu = new BMenu("File");
    menubar->AddItem(uppermenu);

    if (machine_class != VICE_MACHINE_VSID) {
        uppermenu->AddItem(new BMenuItem("Autostart", new BMessage(MENU_AUTOSTART), 'A'));
        uppermenu->AddItem(menu = new BMenu("Autostart Settings"));
            menu->AddItem(new BMenuItem("Autostart warp", new BMessage(MENU_AUTOSTART_WARP)));
            menu->AddItem(new BMenuItem("Use ':' with run", new BMessage(MENU_USE_COLON_WITH_RUN)));
            menu->AddItem(new BMenuItem("Load to BASIC start (,8)", new BMessage(MENU_LOAD_TO_BASIC_START)));
            menu->AddItem(new BMenuItem("Autostart delay", new BMessage(MENU_AUTOSTART_DELAY)));
            menu->AddItem(new BMenuItem("Random Delay", new BMessage(MENU_AUTOSTART_DELAY_RANDOM)));
            menu->AddItem(submenu = new BMenu("PRG autostart mode"));
                submenu->SetRadioMode(true);
                submenu->AddItem(new BMenuItem("Virtual FS", new BMessage(MENU_AUTOSTART_PRG_VIRTUAL_FS)));
                submenu->AddItem(new BMenuItem("Inject", new BMessage(MENU_AUTOSTART_PRG_INJECT)));
                submenu->AddItem(new BMenuItem("Disk image", new BMessage(MENU_AUTOSTART_PRG_DISK_IMAGE)));
            menu->AddItem(new BMenuItem("Select file for PRG autostart disk", new BMessage(MENU_AUTOSTART_PRG_DISK_IMAGE_SELECT)));
        uppermenu->AddSeparatorItem();
        uppermenu->AddItem(menu = new BMenu("Attach Disk"));
            menu->AddItem(new BMenuItem("Drive 8", new BMessage(MENU_ATTACH_DISK8), '8'));
            menu->AddItem(new BMenuItem("Drive 9", new BMessage(MENU_ATTACH_DISK9), '9'));
            menu->AddItem(new BMenuItem("Drive 10", new BMessage(MENU_ATTACH_DISK10)));
            menu->AddItem(new BMenuItem("Drive 11", new BMessage(MENU_ATTACH_DISK11)));
        uppermenu->AddItem(menu = new BMenu("Detach Disk"));
            menu->AddItem(new BMenuItem("Drive 8", new BMessage(MENU_DETACH_DISK8)));
            menu->AddItem(new BMenuItem("Drive 9", new BMessage(MENU_DETACH_DISK9)));
            menu->AddItem(new BMenuItem("Drive 10", new BMessage(MENU_DETACH_DISK10)));
            menu->AddItem(new BMenuItem("Drive 11", new BMessage(MENU_DETACH_DISK11)));
        uppermenu->AddItem(menu = new BMenu("Flip List"));
            menu->AddItem(new BMenuItem("Add current image", new BMessage(MENU_FLIP_ADD), 'I'));
            menu->AddItem(new BMenuItem("Remove Current Image", new BMessage(MENU_FLIP_REMOVE), 'K'));
            menu->AddItem(new BMenuItem("Attach next image", new BMessage(MENU_FLIP_NEXT), 'N'));
            menu->AddItem(new BMenuItem("Attach previous image", new BMessage(MENU_FLIP_PREVIOUS), 'N', B_CONTROL_KEY));
        uppermenu->AddSeparatorItem();
    }

    if (machine_class != VICE_MACHINE_C64DTV && machine_class != VICE_MACHINE_VSID && machine_class != VICE_MACHINE_SCPU64) {
        uppermenu->AddItem(new BMenuItem("Attach Tape", new BMessage(MENU_ATTACH_TAPE), 'T'));
        uppermenu->AddItem(new BMenuItem("Detach Tape", new BMessage(MENU_DETACH_TAPE)));
        uppermenu->AddItem(menu = new BMenu("Datasette Control"));
            menu->AddItem(new BMenuItem("Start", new BMessage(MENU_DATASETTE_START)));
            menu->AddItem(new BMenuItem("Stop", new BMessage(MENU_DATASETTE_STOP)));
            menu->AddItem(new BMenuItem("Forward", new BMessage(MENU_DATASETTE_FORWARD)));
            menu->AddItem(new BMenuItem("Rewind", new BMessage(MENU_DATASETTE_REWIND)));
            menu->AddItem(new BMenuItem("Record", new BMessage(MENU_DATASETTE_RECORD)));
            menu->AddItem(new BMenuItem("Reset", new BMessage(MENU_DATASETTE_RESET)));
            menu->AddItem(new BMenuItem("Reset Counter", new BMessage(MENU_DATASETTE_COUNTER)));
        uppermenu->AddSeparatorItem();
    }

    if (get_carts != NULL) {
        uppermenu->AddItem(menu = new BMenu("Attach cartridge image"));
            menu->AddItem(new BMenuItem("CRT", new BMessage(MENU_CART_ATTACH_CRT)));

            cartlist = get_carts();

            menu->AddItem(submenu = new BMenu("Attach generic cartridge images"));
            for (i = 0; cartlist[i].name; ++i) {
                if (cartlist[i].flags & CARTRIDGE_GROUP_GENERIC) {
                    submenu->AddItem(new BMenuItem(cartlist[i].name, new BMessage(MENU_GENERIC_CARTS + cartlist[i].crtid + 256)));
                }
            }

            menu->AddItem(submenu = new BMenu("Attach RAM expansion cartridge images"));
            for (i = 0; cartlist[i].name; ++i) {
                if (cartlist[i].flags & CARTRIDGE_GROUP_RAMEX) {
                    submenu->AddItem(new BMenuItem(cartlist[i].name, new BMessage(MENU_RAMEX_CARTS + cartlist[i].crtid + 256)));
                }
            }

            if (machine_class != VICE_MACHINE_SCPU64) {
                menu->AddItem(submenu = new BMenu("Attach freezer cartridge images"));
                for (i = 0; cartlist[i].name; ++i) {
                    if (cartlist[i].flags & CARTRIDGE_GROUP_FREEZER) {
                        submenu->AddItem(new BMenuItem(cartlist[i].name, new BMessage(MENU_FREEZER_CARTS + cartlist[i].crtid + 256)));
                    }
                }
            }

            menu->AddItem(submenu = new BMenu("Attach game cartridge images"));
            for (i = 0; cartlist[i].name; ++i) {
                if (cartlist[i].flags & CARTRIDGE_GROUP_GAME) {
                    submenu->AddItem(new BMenuItem(cartlist[i].name, new BMessage(MENU_GAME_CARTS + cartlist[i].crtid + 256)));
                }
            }

            menu->AddItem(submenu = new BMenu("Attach utility cartridge images"));
            for (i = 0; cartlist[i].name; ++i) {
                if (cartlist[i].flags & CARTRIDGE_GROUP_UTIL) {
                    submenu->AddItem(new BMenuItem(cartlist[i].name, new BMessage(MENU_UTIL_CARTS + cartlist[i].crtid + 256)));
                }
            }

            menu->AddSeparatorItem();
            menu->AddItem(new BMenuItem("Set cartridge as default", new BMessage(MENU_CART_SET_DEFAULT)));
        uppermenu->AddItem(new BMenuItem("Detach cartridge image", new BMessage(MENU_CART_DETACH)));
        if (machine_class != VICE_MACHINE_SCPU64) {
            uppermenu->AddItem(new BMenuItem("Cartridge freeze", new BMessage(MENU_CART_FREEZE), 'Z'));
        }
        uppermenu->AddSeparatorItem();
    }

    if (machine_class == VICE_MACHINE_VIC20) {
        uppermenu->AddItem(menu = new BMenu("Attach cartridge image"));
            menu->AddItem(new BMenuItem("Generic cartridge image", new BMessage(MENU_CART_VIC20_GENERIC)));
            menu->AddItem(new BMenuItem("Behr Bonz image", new BMessage(MENU_CART_VIC20_BEHR_BONZ)));
            menu->AddItem(new BMenuItem("Mega-Cart image", new BMessage(MENU_CART_VIC20_MEGACART)));
            menu->AddItem(new BMenuItem("Final Expansion image", new BMessage(MENU_CART_VIC20_FINAL_EXPANSION)));
            menu->AddItem(new BMenuItem("UltiMem image", new BMessage(MENU_CART_VIC20_UM)));
            menu->AddItem(new BMenuItem("Vic Flash Plugin image", new BMessage(MENU_CART_VIC20_FP)));
            menu->AddItem(submenu = new BMenu("Add to generic"));
                submenu->AddItem(new BMenuItem("Smart Attach", new BMessage(MENU_CART_VIC20_SMART_ATTACH)));
                submenu->AddItem(new BMenuItem("4/8/16KB image at $2000", new BMessage(MENU_CART_VIC20_16KB_2000)));
                submenu->AddItem(new BMenuItem("4/8/16KB image at $4000", new BMessage(MENU_CART_VIC20_16KB_4000)));
                submenu->AddItem(new BMenuItem("4/8/16KB image at $6000", new BMessage(MENU_CART_VIC20_16KB_6000)));
                submenu->AddItem(new BMenuItem("4/8KB image at $A000", new BMessage(MENU_CART_VIC20_8KB_A000)));
                submenu->AddItem(new BMenuItem("4KB image at $B000", new BMessage(MENU_CART_VIC20_4KB_B000)));
        uppermenu->AddItem(new BMenuItem("Detach cartridge image", new BMessage(MENU_CART_DETACH)));
        uppermenu->AddSeparatorItem();
    }

    if (machine_class == VICE_MACHINE_PLUS4) {
        uppermenu->AddItem(menu = new BMenu("Attach cartridge image"));
            menu->AddItem(new BMenuItem("Smart attach", new BMessage(MENU_CART_PLUS4_SMART)));
            menu->AddItem(new BMenuItem("C0 low image attach", new BMessage(MENU_CART_PLUS4_C0_LOW)));
            menu->AddItem(new BMenuItem("C0 high image attach", new BMessage(MENU_CART_PLUS4_C0_HIGH)));
            menu->AddItem(new BMenuItem("C1 low image attach", new BMessage(MENU_CART_PLUS4_C1_LOW)));
            menu->AddItem(new BMenuItem("C1 high image attach", new BMessage(MENU_CART_PLUS4_C1_HIGH)));
            menu->AddItem(new BMenuItem("C2 low image attach", new BMessage(MENU_CART_PLUS4_C2_LOW)));
            menu->AddItem(new BMenuItem("C2 high image attach", new BMessage(MENU_CART_PLUS4_C2_HIGH)));
            menu->AddSeparatorItem();
        uppermenu->AddItem(new BMenuItem("Reset on cart change", new BMessage(MENU_CART_PLUS4_RESET_ON_CHANGE)));
        uppermenu->AddItem(new BMenuItem("Detach cartridge image", new BMessage(MENU_CART_PLUS4_DETACH)));
        uppermenu->AddSeparatorItem();
    }

    if (machine_class == VICE_MACHINE_CBM5x0 || machine_class == VICE_MACHINE_CBM6x0) {
        uppermenu->AddItem(menu = new BMenu("Cartridge image"));
            menu->AddItem(new BMenuItem("Load new Cart $1000", new BMessage(MENU_CART_CBM2_LOAD_1000)));
            menu->AddItem(new BMenuItem("Unload Cart $1000", new BMessage(MENU_CART_CBM2_UNLOAD_1000)));
            menu->AddItem(new BMenuItem("Load new Cart $2000-$3000", new BMessage(MENU_CART_CBM2_LOAD_2000)));
            menu->AddItem(new BMenuItem("Unload Cart $2000-$3000", new BMessage(MENU_CART_CBM2_UNLOAD_2000)));
            menu->AddItem(new BMenuItem("Load new Cart $4000-$5000", new BMessage(MENU_CART_CBM2_LOAD_4000)));
            menu->AddItem(new BMenuItem("Unload Cart $4000-$5000", new BMessage(MENU_CART_CBM2_UNLOAD_4000)));
            menu->AddItem(new BMenuItem("Load new Cart $6000-$7000", new BMessage(MENU_CART_CBM2_LOAD_6000)));
            menu->AddItem(new BMenuItem("Unload Cart $6000-$7000", new BMessage(MENU_CART_CBM2_UNLOAD_6000)));
            menu->AddSeparatorItem();
        uppermenu->AddItem(new BMenuItem("Reset on cart change", new BMessage(MENU_CART_CBM2_RESET_ON_CHANGE)));
        uppermenu->AddSeparatorItem();
    }

    if (machine_class != VICE_MACHINE_VSID && machine_class != VICE_MACHINE_C64DTV) {
        uppermenu->AddItem(menu = new BMenu("I/O collision handling"));
            menu->SetRadioMode(true);
            menu->AddItem(new BMenuItem("Detach all involved carts", new BMessage(MENU_IO_COLLISION_DETACH_ALL)));
            menu->AddItem(new BMenuItem("Detach last inserted cart", new BMessage(MENU_IO_COLLISION_DETACH_LAST)));
            menu->AddItem(new BMenuItem("'AND' wires", new BMessage(MENU_IO_COLLISION_AND_WIRES)));
    }

    if (machine_class != VICE_MACHINE_VSID) {
        uppermenu->AddItem(menu = new BMenu("Snapshot"));
            menu->AddItem(new BMenuItem("Load snapshot", new BMessage(MENU_SNAPSHOT_LOAD)));
            menu->AddItem(new BMenuItem("Save snapshot", new BMessage(MENU_SNAPSHOT_SAVE)));
            menu->AddItem(new BMenuItem("Load quicksnapshot", new BMessage(MENU_LOADQUICK), 'L', B_CONTROL_KEY));
            menu->AddItem(new BMenuItem("Save quicksnapshot", new BMessage(MENU_SAVEQUICK), 'S', B_CONTROL_KEY));
        uppermenu->AddItem(menu = new BMenu("Netplay"));
            menu->AddItem(new BMenuItem("Start Server", new BMessage(MENU_NETPLAY_SERVER)));
            menu->AddItem(new BMenuItem("Connect Client", new BMessage(MENU_NETPLAY_CLIENT)));
            menu->AddItem(new BMenuItem("Disconnect", new BMessage(MENU_NETPLAY_DISCONNECT)));
            menu->AddItem(new BMenuItem("Settings...", new BMessage(MENU_NETPLAY_SETTINGS)));

        uppermenu->AddItem(menu = new BMenu("Event History"));
            menu->AddItem(new BMenuItem("Start/Stop recording", new BMessage(MENU_EVENT_TOGGLE_RECORD)));
            menu->AddItem(new BMenuItem("Start/Stop playback", new BMessage(MENU_EVENT_TOGGLE_PLAYBACK)));
            menu->AddItem(new BMenuItem("Set Milestone", new BMessage(MENU_EVENT_SETMILESTONE), 'G'));
            menu->AddItem(new BMenuItem("Return to Milestone", new BMessage(MENU_EVENT_RESETMILESTONE), 'H'));
            menu->AddItem(new BMenuItem("Select start snapshot", new BMessage(MENU_EVENT_SNAPSHOT_START)));
            menu->AddItem(new BMenuItem("Select end snapshot", new BMessage(MENU_EVENT_SNAPSHOT_END)));
        uppermenu->AddItem(menu = new BMenu("Recording start mode"));
            menu->SetRadioMode(true);
            menu->AddItem(new BMenuItem("Save new snapshot", new BMessage(MENU_EVENT_START_MODE_SAVE)));
            menu->AddItem(new BMenuItem("Load existing snapshot", new BMessage(MENU_EVENT_START_MODE_LOAD)));
            menu->AddItem(new BMenuItem("Start with Reset", new BMessage(MENU_EVENT_START_MODE_RESET)));
            menu->AddItem(new BMenuItem("Overwrite Playback", new BMessage(MENU_EVENT_START_MODE_PLAYBACK)));
    }

    if (machine_class != VICE_MACHINE_VSID) {
        uppermenu->AddSeparatorItem();
        uppermenu->AddItem(menu = new BMenu("Screenshot"));
            menu->AddItem(submenu = new BMenu("Doodle screenshot settings"));
                submenu->AddItem(extsubmenu = new BMenu("Oversize handling"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Scale", new BMessage(MENU_SCREENSHOT_DOODLE_OVERSIZE_SCALE)));
                    extsubmenu->AddItem(new BMenuItem("Crop left top", new BMessage(MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_LEFT_TOP)));
                    extsubmenu->AddItem(new BMenuItem("Crop middle top", new BMessage(MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_MIDDLE_TOP)));
                    extsubmenu->AddItem(new BMenuItem("Crop right top", new BMessage(MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_RIGHT_TOP)));
                    extsubmenu->AddItem(new BMenuItem("Crop left center", new BMessage(MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_LEFT_CENTER)));
                    extsubmenu->AddItem(new BMenuItem("Crop middle center", new BMessage(MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_MIDDLE_CENTER)));
                    extsubmenu->AddItem(new BMenuItem("Crop right center", new BMessage(MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_RIGHT_CENTER)));
                    extsubmenu->AddItem(new BMenuItem("Crop left bottom", new BMessage(MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_LEFT_BOTTOM)));
                    extsubmenu->AddItem(new BMenuItem("Crop middle bottom", new BMessage(MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_MIDDLE_BOTTOM)));
                    extsubmenu->AddItem(new BMenuItem("Crop right bottom", new BMessage(MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_RIGHT_BOTTOM)));
                submenu->AddItem(extsubmenu = new BMenu("Undersize handling"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Scale", new BMessage(MENU_SCREENSHOT_DOODLE_UNDERSIZE_SCALE)));
                    extsubmenu->AddItem(new BMenuItem("Borderize", new BMessage(MENU_SCREENSHOT_DOODLE_UNDERSIZE_BORDERIZE)));
            if (machine_class != VICE_MACHINE_PET && machine_class != VICE_MACHINE_CBM6x0) {
                submenu->AddItem(extsubmenu = new BMenu("Multicolor handling"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Black & white", new BMessage(MENU_SCREENSHOT_DOODLE_MULTICOLOR_BLACK_WHITE)));
                    extsubmenu->AddItem(new BMenuItem("2 colors", new BMessage(MENU_SCREENSHOT_DOODLE_MULTICOLOR_2_COLORS)));
                    extsubmenu->AddItem(new BMenuItem("4 colors", new BMessage(MENU_SCREENSHOT_DOODLE_MULTICOLOR_4_COLORS)));
                    extsubmenu->AddItem(new BMenuItem("Gray scale", new BMessage(MENU_SCREENSHOT_DOODLE_MULTICOLOR_GRAY_SCALE)));
                    extsubmenu->AddItem(new BMenuItem("Gray scale", new BMessage(MENU_SCREENSHOT_DOODLE_MULTICOLOR_DITHER)));
            }
            if (machine_class == VICE_MACHINE_PLUS4) {
                submenu->AddItem(extsubmenu = new BMenu("TED luminosity handling"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Ignore", new BMessage(MENU_SCREENSHOT_DOODLE_TED_LUM_IGNORE)));
                    extsubmenu->AddItem(new BMenuItem("Dither", new BMessage(MENU_SCREENSHOT_DOODLE_TED_LUM_DITHER)));
            }
            if (machine_class == VICE_MACHINE_PET || machine_class == VICE_MACHINE_CBM6x0) {
                submenu->AddItem(extsubmenu = new BMenu("CRTC text color"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("White", new BMessage(MENU_SCREENSHOT_DOODLE_CRTC_TEXT_COLOR_WHITE)));
                    extsubmenu->AddItem(new BMenuItem("Amber", new BMessage(MENU_SCREENSHOT_DOODLE_CRTC_TEXT_COLOR_AMBER)));
                    extsubmenu->AddItem(new BMenuItem("Green", new BMessage(MENU_SCREENSHOT_DOODLE_CRTC_TEXT_COLOR_GREEN)));
            }

            menu->AddItem(submenu = new BMenu("Koala screenshot settings"));
                submenu->AddItem(extsubmenu = new BMenu("Oversize handling"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Scale", new BMessage(MENU_SCREENSHOT_KOALA_OVERSIZE_SCALE)));
                    extsubmenu->AddItem(new BMenuItem("Crop left top", new BMessage(MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_LEFT_TOP)));
                    extsubmenu->AddItem(new BMenuItem("Crop middle top", new BMessage(MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_MIDDLE_TOP)));
                    extsubmenu->AddItem(new BMenuItem("Crop right top", new BMessage(MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_RIGHT_TOP)));
                    extsubmenu->AddItem(new BMenuItem("Crop left center", new BMessage(MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_LEFT_CENTER)));
                    extsubmenu->AddItem(new BMenuItem("Crop middle center", new BMessage(MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_MIDDLE_CENTER)));
                    extsubmenu->AddItem(new BMenuItem("Crop right center", new BMessage(MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_RIGHT_CENTER)));
                    extsubmenu->AddItem(new BMenuItem("Crop left bottom", new BMessage(MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_LEFT_BOTTOM)));
                    extsubmenu->AddItem(new BMenuItem("Crop middle bottom", new BMessage(MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_MIDDLE_BOTTOM)));
                    extsubmenu->AddItem(new BMenuItem("Crop right bottom", new BMessage(MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_RIGHT_BOTTOM)));
                submenu->AddItem(extsubmenu = new BMenu("Undersize handling"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Scale", new BMessage(MENU_SCREENSHOT_KOALA_UNDERSIZE_SCALE)));
                    extsubmenu->AddItem(new BMenuItem("Borderize", new BMessage(MENU_SCREENSHOT_KOALA_UNDERSIZE_BORDERIZE)));
            if (machine_class == VICE_MACHINE_PLUS4) {
                submenu->AddItem(extsubmenu = new BMenu("TED luminosity handling"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Ignore", new BMessage(MENU_SCREENSHOT_KOALA_TED_LUM_IGNORE)));
                    extsubmenu->AddItem(new BMenuItem("Dither", new BMessage(MENU_SCREENSHOT_KOALA_TED_LUM_DITHER)));
            }
            if (machine_class == VICE_MACHINE_PET || machine_class == VICE_MACHINE_CBM6x0) {
                submenu->AddItem(extsubmenu = new BMenu("CRTC text color"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("White", new BMessage(MENU_SCREENSHOT_KOALA_CRTC_TEXT_COLOR_WHITE)));
                    extsubmenu->AddItem(new BMenuItem("Amber", new BMessage(MENU_SCREENSHOT_KOALA_CRTC_TEXT_COLOR_AMBER)));
                    extsubmenu->AddItem(new BMenuItem("Green", new BMessage(MENU_SCREENSHOT_KOALA_CRTC_TEXT_COLOR_GREEN)));
            }

            if (window_nr) {
                menu->AddItem(new BMenuItem("Save bmp screenshot", new BMessage(MENU_SCREENSHOT_BMP_SCREEN1)));
                menu->AddItem(new BMenuItem("Save doodle screenshot", new BMessage(MENU_SCREENSHOT_DOODLE_SCREEN1)));
                menu->AddItem(new BMenuItem("Save compressed doodle screenshot", new BMessage(MENU_SCREENSHOT_DOODLE_COMPRESSED_SCREEN1)));
#ifdef HAVE_GIF
                menu->AddItem(new BMenuItem("Save gif screenshot", new BMessage(MENU_SCREENSHOT_GIF_SCREEN1)));
#endif
                menu->AddItem(new BMenuItem("Save godot screenshot", new BMessage(MENU_SCREENSHOT_GODOT_SCREEN1)));
                menu->AddItem(new BMenuItem("Save iff screenshot", new BMessage(MENU_SCREENSHOT_IFF_SCREEN1)));
#ifdef HAVE_JPEG
                menu->AddItem(new BMenuItem("Save jpeg screenshot", new BMessage(MENU_SCREENSHOT_JPEG_SCREEN1)));
#endif
                menu->AddItem(new BMenuItem("Save koala screenshot", new BMessage(MENU_SCREENSHOT_KOALA_SCREEN1)));
                menu->AddItem(new BMenuItem("Save compressed koala screenshot", new BMessage(MENU_SCREENSHOT_KOALA_COMPRESSED_SCREEN1)));
                menu->AddItem(new BMenuItem("Save pcx screenshot", new BMessage(MENU_SCREENSHOT_PCX_SCREEN1)));
#ifdef HAVE_PNG
                menu->AddItem(new BMenuItem("Save png screenshot", new BMessage(MENU_SCREENSHOT_PNG_SCREEN1)));
#endif
                menu->AddItem(new BMenuItem("Save ppm screenshot", new BMessage(MENU_SCREENSHOT_PPM_SCREEN1)));
            } else {
                menu->AddItem(new BMenuItem("Save bmp screenshot", new BMessage(MENU_SCREENSHOT_BMP_SCREEN0)));
                menu->AddItem(new BMenuItem("Save doodle screenshot", new BMessage(MENU_SCREENSHOT_DOODLE_SCREEN0)));
                menu->AddItem(new BMenuItem("Save compressed doodle screenshot", new BMessage(MENU_SCREENSHOT_DOODLE_COMPRESSED_SCREEN0)));
#ifdef HAVE_GIF
                menu->AddItem(new BMenuItem("Save gif screenshot", new BMessage(MENU_SCREENSHOT_GIF_SCREEN0)));
#endif
                menu->AddItem(new BMenuItem("Save godot screenshot", new BMessage(MENU_SCREENSHOT_GODOT_SCREEN0)));
                menu->AddItem(new BMenuItem("Save iff screenshot", new BMessage(MENU_SCREENSHOT_IFF_SCREEN0)));
#ifdef HAVE_JPEG
                menu->AddItem(new BMenuItem("Save jpeg screenshot", new BMessage(MENU_SCREENSHOT_JPEG_SCREEN0)));
#endif
                menu->AddItem(new BMenuItem("Save koala screenshot", new BMessage(MENU_SCREENSHOT_KOALA_SCREEN0)));
                menu->AddItem(new BMenuItem("Save compressed koala screenshot", new BMessage(MENU_SCREENSHOT_KOALA_COMPRESSED_SCREEN0)));
                menu->AddItem(new BMenuItem("Save pcx screenshot", new BMessage(MENU_SCREENSHOT_PCX_SCREEN0)));
#ifdef HAVE_PNG
                menu->AddItem(new BMenuItem("Save png screenshot", new BMessage(MENU_SCREENSHOT_PNG_SCREEN0)));
#endif
                menu->AddItem(new BMenuItem("Save ppm screenshot", new BMessage(MENU_SCREENSHOT_PPM_SCREEN0)));
            }
    }

    if (machine_class == VICE_MACHINE_VSID) {
        /* vsid */
        uppermenu->AddItem(new BMenuItem("Load PSID file", new BMessage(MENU_VSID_LOAD)));
        uppermenu->AddItem(vsid_tune_menu = new BMenu("Tune"));
    }

    uppermenu->AddSeparatorItem();
    uppermenu->AddItem(menu = new BMenu("Default CPU JAM action"));
        menu->SetRadioMode(true);
        menu->AddItem(new BMenuItem("Ask", new BMessage(MENU_JAM_ACTION_ASK)));
        menu->AddItem(new BMenuItem("Continue", new BMessage(MENU_JAM_ACTION_CONTINUE)));
        menu->AddItem(new BMenuItem("Start monitor", new BMessage(MENU_JAM_ACTION_START_MONITOR)));
        menu->AddItem(new BMenuItem("Reset", new BMessage(MENU_JAM_ACTION_RESET)));
        menu->AddItem(new BMenuItem("Hard reset", new BMessage(MENU_JAM_ACTION_HARD_RESET)));
        menu->AddItem(new BMenuItem("Quit emulator", new BMessage(MENU_JAM_ACTION_QUIT_EMULATOR)));

    uppermenu->AddItem(item = new BMenuItem("Pause", new BMessage(MENU_PAUSE), 'P'));
    uppermenu->AddItem(item = new BMenuItem("Single frame advance", new BMessage(MENU_SINGLE_FRAME_ADVANCE)));
    uppermenu->AddItem(item = new BMenuItem("Monitor", new BMessage(MENU_MONITOR), 'M'));
    uppermenu->AddItem(item = new BMenuItem("Soft Reset", new BMessage(MENU_RESET_SOFT), 'R'));
    uppermenu->AddItem(new BMenuItem("Hard Reset", new BMessage(MENU_RESET_HARD), 'R', B_CONTROL_KEY));
    uppermenu->AddSeparatorItem();
    uppermenu->AddItem(new BMenuItem("Quit", new BMessage(MENU_EXIT_REQUESTED), 'Q'));

    /* create the EDIT menu */
    if (machine_class != VICE_MACHINE_VSID) {
        uppermenu = new BMenu("Edit");
        menubar->AddItem(uppermenu);

        uppermenu->AddItem(new BMenuItem("Copy", new BMessage(MENU_COPY)));
        uppermenu->AddItem(new BMenuItem("Paste", new BMessage(MENU_PASTE)));
    }

    /* create the OPTIONS menu */
    uppermenu = new BMenu("Options");
    menubar->AddItem(uppermenu);

    if (machine_class != VICE_MACHINE_VSID) {
        /* refresh rate */
        uppermenu->AddItem(menu = new BMenu("Refresh Rate"));
            menu->SetRadioMode(true);
            menu->AddItem(new BMenuItem("Auto", new BMessage(MENU_REFRESH_RATE_AUTO)));
            menu->AddItem(new BMenuItem("1/1", new BMessage(MENU_REFRESH_RATE_1)));
            menu->AddItem(new BMenuItem("1/2", new BMessage(MENU_REFRESH_RATE_2)));
            menu->AddItem(new BMenuItem("1/3", new BMessage(MENU_REFRESH_RATE_3)));
            menu->AddItem(new BMenuItem("1/4", new BMessage(MENU_REFRESH_RATE_4)));
            menu->AddItem(new BMenuItem("1/5", new BMessage(MENU_REFRESH_RATE_5)));
            menu->AddItem(new BMenuItem("1/6", new BMessage(MENU_REFRESH_RATE_6)));
            menu->AddItem(new BMenuItem("1/7", new BMessage(MENU_REFRESH_RATE_7)));
            menu->AddItem(new BMenuItem("1/8", new BMessage(MENU_REFRESH_RATE_8)));
            menu->AddItem(new BMenuItem("1/9", new BMessage(MENU_REFRESH_RATE_9)));
            menu->AddItem(new BMenuItem("1/10", new BMessage(MENU_REFRESH_RATE_10)));
    }

    /* maximum speed */
    uppermenu->AddItem(menu = new BMenu("Maximum Speed"));
        menu->SetRadioMode(true);
        menu->AddItem(new BMenuItem("No Limit", new BMessage(MENU_MAXIMUM_SPEED_NO_LIMIT)));
        menu->AddItem(new BMenuItem("200%", new BMessage(MENU_MAXIMUM_SPEED_200)));
        menu->AddItem(new BMenuItem("100%", new BMessage(MENU_MAXIMUM_SPEED_100)));
        menu->AddItem(new BMenuItem("50%", new BMessage(MENU_MAXIMUM_SPEED_50)));
        menu->AddItem(new BMenuItem("20%", new BMessage(MENU_MAXIMUM_SPEED_20)));
        menu->AddItem(new BMenuItem("10%", new BMessage(MENU_MAXIMUM_SPEED_10)));
    uppermenu->AddItem(new BMenuItem("Warp Mode", new BMessage(MENU_TOGGLE_WARP_MODE),'W'));

    if (machine_class != VICE_MACHINE_VSID) {
        uppermenu->AddSeparatorItem();

        /* video options */
        uppermenu->AddItem(new BMenuItem("DirectWindow", new BMessage(MENU_TOGGLE_DIRECTWINDOW)));
        uppermenu->AddItem(new BMenuItem("Video Cache", new BMessage(MENU_TOGGLE_VIDEOCACHE)));
        uppermenu->AddItem(new BMenuItem("Double Size", new BMessage(MENU_TOGGLE_DOUBLESIZE),'D'));
        if (machine_class == VICE_MACHINE_PET || machine_class == VICE_MACHINE_CBM6x0) {
            uppermenu->AddItem(new BMenuItem("Stretch Vertically", new BMessage(MENU_TOGGLE_STRETCHVERTICAL)));
        }
        uppermenu->AddItem(new BMenuItem("Double Scan", new BMessage(MENU_TOGGLE_DOUBLESCAN)));

        uppermenu->AddItem(menu = new BMenu("Render filter"));
            menu->SetRadioMode(true);
            menu->AddItem(new BMenuItem("None", new BMessage(MENU_RENDER_FILTER_NONE)));
            menu->AddItem(new BMenuItem("CRT emulation", new BMessage(MENU_RENDER_FILTER_CRT_EMULATION)));
            if (machine_class != VICE_MACHINE_PET && machine_class != VICE_MACHINE_CBM6x0) {
                menu->AddItem(new BMenuItem("Scale2x", new BMessage(MENU_RENDER_FILTER_SCALE2X)));
            }
    }

    if (machine_class == VICE_MACHINE_C128) {
        /* VDC options */
        uppermenu->AddItem(menu = new BMenu("VDC"));
            menu->AddItem(submenu = new BMenu("Render filter"));
                submenu->SetRadioMode(true);
                submenu->AddItem(new BMenuItem("None", new BMessage(MENU_VDC_RENDER_FILTER_NONE)));
                submenu->AddItem(new BMenuItem("CRT emulation", new BMessage(MENU_VDC_RENDER_FILTER_CRT_EMULATION)));
            menu->AddItem(new BMenuItem("Video Cache", new BMessage(MENU_TOGGLE_VDC_VIDEOCACHE)));
            menu->AddItem(new BMenuItem("Double Size", new BMessage(MENU_TOGGLE_VDC_DOUBLESIZE)));
            menu->AddItem(new BMenuItem("Stretch Vertically", new BMessage(MENU_TOGGLE_STRETCHVERTICAL)));
            menu->AddItem(new BMenuItem("Double Scan", new BMessage(MENU_TOGGLE_VDC_DOUBLESCAN)));
            menu->AddSeparatorItem();
            menu->AddItem(new BMenuItem("64KB video memory", new BMessage(MENU_TOGGLE_VDC64KB)));
            menu->AddItem(submenu = new BMenu("VDC revision"));
                submenu->SetRadioMode(true);
                submenu->AddItem(new BMenuItem("Rev 0", new BMessage(MENU_VDC_REV_0)));
                submenu->AddItem(new BMenuItem("Rev 1", new BMessage(MENU_VDC_REV_1)));
                submenu->AddItem(new BMenuItem("Rev 2", new BMessage(MENU_VDC_REV_2)));
    }

    if (machine_class != VICE_MACHINE_VSID) {
        uppermenu->AddSeparatorItem();
    }

    /* sound options */
    uppermenu->AddItem(new BMenuItem("Sound", new BMessage(MENU_TOGGLE_SOUND)));
    uppermenu->AddItem(menu = new BMenu("Sound Recording"));
        menu->AddItem(new BMenuItem("Stop Sound Record", new BMessage(MENU_SOUND_RECORD_STOP)));
        menu->AddSeparatorItem();
        menu->AddItem(new BMenuItem("Sound Record AIFF", new BMessage(MENU_SOUND_RECORD_AIFF)));
        menu->AddItem(new BMenuItem("Sound Record IFF", new BMessage(MENU_SOUND_RECORD_IFF)));
#ifdef USE_LAMEMP3
        menu->AddItem(new BMenuItem("Sound Record MP3", new BMessage(MENU_SOUND_RECORD_MP3)));
#endif
#ifdef USE_FLAC
        menu->AddItem(new BMenuItem("Sound Record FLAC", new BMessage(MENU_SOUND_RECORD_FLAC)));
#endif
#ifdef USE_VORBIS
        menu->AddItem(new BMenuItem("Sound Record OGG/VORBIS", new BMessage(MENU_SOUND_RECORD_VORBIS)));
#endif
        menu->AddItem(new BMenuItem("Sound Record VOC", new BMessage(MENU_SOUND_RECORD_VOC)));
        menu->AddItem(new BMenuItem("Sound Record WAV", new BMessage(MENU_SOUND_RECORD_WAV)));
    uppermenu->AddSeparatorItem();

    if (machine_class != VICE_MACHINE_VSID) {
        uppermenu->AddItem(new BMenuItem("True Drive Emulation", new BMessage(MENU_TOGGLE_DRIVE_TRUE_EMULATION)));
        uppermenu->AddItem(new BMenuItem("Drive Sound Emulation", new BMessage(MENU_TOGGLE_DRIVE_SOUND_EMULATION)));
        uppermenu->AddItem(new BMenuItem("Handle TDE for autostart", new BMessage(MENU_TOGGLE_HANDLE_TDE_AUTOSTART)));
        uppermenu->AddItem(new BMenuItem("Virtual Devices", new BMessage(MENU_TOGGLE_VIRTUAL_DEVICES)));
        uppermenu->AddSeparatorItem();
    }

    if (machine_class == VICE_MACHINE_VSID) {
        uppermenu->AddItem(new BMenuItem("Override PSID settings", new BMessage(MENU_TOGGLE_KEEP_ENV)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64DTV ||
        machine_class == VICE_MACHINE_C128 || machine_class == VICE_MACHINE_VIC20 ||
        machine_class == VICE_MACHINE_VSID) {
        uppermenu->AddItem(menu = new BMenu("Video Standard"));
            menu->SetRadioMode(true);
            menu->AddItem(new BMenuItem("PAL", new BMessage(MENU_SYNC_FACTOR_PAL)));
            menu->AddItem(new BMenuItem("NTSC", new BMessage(MENU_SYNC_FACTOR_NTSC)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_VSID) {
            menu->AddItem(new BMenuItem("Old NTSC", new BMessage(MENU_SYNC_FACTOR_NTSCOLD)));
            menu->AddItem(new BMenuItem("PAL-N", new BMessage(MENU_SYNC_FACTOR_PALN)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC ||
        machine_class == VICE_MACHINE_SCPU64) {
        uppermenu->AddItem(menu = new BMenu("C64 model"));
            menu->SetRadioMode(true);
            menu->AddItem(new BMenuItem("C64 PAL", new BMessage(MENU_C64_MODEL_C64_PAL)));
            menu->AddItem(new BMenuItem("C64C PAL", new BMessage(MENU_C64_MODEL_C64C_PAL)));
            menu->AddItem(new BMenuItem("C64 old PAL", new BMessage(MENU_C64_MODEL_C64_OLD_PAL)));
            menu->AddItem(new BMenuItem("C64 NTSC", new BMessage(MENU_C64_MODEL_C64_NTSC)));
            menu->AddItem(new BMenuItem("C64C NTSC", new BMessage(MENU_C64_MODEL_C64C_NTSC)));
            menu->AddItem(new BMenuItem("C64 old NTSC", new BMessage(MENU_C64_MODEL_C64_OLD_NTSC)));
            menu->AddItem(new BMenuItem("Drean", new BMessage(MENU_C64_MODEL_DREAN)));
            menu->AddItem(new BMenuItem("C64 SX PAL", new BMessage(MENU_C64_MODEL_C64SX_PAL)));
            menu->AddItem(new BMenuItem("C64 SX NTSC", new BMessage(MENU_C64_MODEL_C64SX_NTSC)));
            menu->AddItem(new BMenuItem("Japanese", new BMessage(MENU_C64_MODEL_C64_JAP)));
            menu->AddItem(new BMenuItem("C64 GS", new BMessage(MENU_C64_MODEL_C64_GS)));
            if (machine_class != VICE_MACHINE_SCPU64) {
                menu->AddItem(new BMenuItem("PET64 PAL", new BMessage(MENU_C64_MODEL_PET64_PAL)));
                menu->AddItem(new BMenuItem("PET64 NTSC", new BMessage(MENU_C64_MODEL_PET64_NTSC)));
                menu->AddItem(new BMenuItem("MAX Machine", new BMessage(MENU_C64_MODEL_ULTIMAX)));
            }
            menu->AddItem(submenu = new BMenu("Custom"));
            if (machine_class != VICE_MACHINE_C64) {
                submenu->AddItem(extsubmenu = new BMenu("VICII model"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("6569 (PAL)", new BMessage(MENU_VICII_MODEL_6569_PAL)));
                    extsubmenu->AddItem(new BMenuItem("8565 (PAL)", new BMessage(MENU_VICII_MODEL_8565_PAL)));
                    extsubmenu->AddItem(new BMenuItem("6569R1 (old PAL)", new BMessage(MENU_VICII_MODEL_6569R1_OLD_PAL)));
                    extsubmenu->AddItem(new BMenuItem("6567 (NTSC)", new BMessage(MENU_VICII_MODEL_6567_NTSC)));
                    extsubmenu->AddItem(new BMenuItem("8562 (NTSC)", new BMessage(MENU_VICII_MODEL_8562_NTSC)));
                    extsubmenu->AddItem(new BMenuItem("6567R56A (old NTSC)", new BMessage(MENU_VICII_MODEL_6567R56A_OLD_NTSC)));
                    extsubmenu->AddItem(new BMenuItem("6572 (PAL-N)", new BMessage(MENU_VICII_MODEL_6572_PAL_N)));

                submenu->AddItem(extsubmenu = new BMenu("Glue logic"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Discrete", new BMessage(MENU_GLUE_LOGIC_DISCRETE)));
                    extsubmenu->AddItem(new BMenuItem("Custom IC", new BMessage(MENU_GLUE_LOGIC_CUSTOM_IC)));
            } else {
                submenu->AddItem(extsubmenu = new BMenu("VICII model"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("PAL-G", new BMessage(MENU_VICII_MODEL_PALG)));
                    extsubmenu->AddItem(new BMenuItem("Old PAL-G", new BMessage(MENU_VICII_MODEL_OLD_PALG)));
                    extsubmenu->AddItem(new BMenuItem("NTSC-M", new BMessage(MENU_VICII_MODEL_NTSCM)));
                    extsubmenu->AddItem(new BMenuItem("Old NTSC-M", new BMessage(MENU_VICII_MODEL_OLD_NTSCM)));
                    extsubmenu->AddItem(new BMenuItem("PAL-N", new BMessage(MENU_VICII_MODEL_PALN)));
            }
                submenu->AddItem(new BMenuItem("Reset IEC bus with computer", new BMessage(MENU_IEC_RESET)));
            if (machine_class != VICE_MACHINE_SCPU64) {
                submenu->AddItem(extsubmenu = new BMenu("Kernal revision"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Rev 1", new BMessage(MENU_KERNAL_REV_1)));
                    extsubmenu->AddItem(new BMenuItem("Rev 2", new BMessage(MENU_KERNAL_REV_2)));
                    extsubmenu->AddItem(new BMenuItem("Rev 3", new BMessage(MENU_KERNAL_REV_3)));
                    extsubmenu->AddItem(new BMenuItem("SX-64", new BMessage(MENU_KERNAL_REV_SX64)));
                    extsubmenu->AddItem(new BMenuItem("4064", new BMessage(MENU_KERNAL_REV_4064)));
            }
    }

    if (machine_class == VICE_MACHINE_PLUS4) {
        uppermenu->AddItem(menu = new BMenu("Plus4 model"));
            menu->SetRadioMode(true);
            menu->AddItem(new BMenuItem("C16 PAL", new BMessage(MENU_PLUS4_MODEL_C16_PAL)));
            menu->AddItem(new BMenuItem("C16 NTSC", new BMessage(MENU_PLUS4_MODEL_C16_NTSC)));
            menu->AddItem(new BMenuItem("Plus4 PAL", new BMessage(MENU_PLUS4_MODEL_PLUS4_PAL)));
            menu->AddItem(new BMenuItem("Plus4 NTSC", new BMessage(MENU_PLUS4_MODEL_PLUS4_NTSC)));
            menu->AddItem(new BMenuItem("V364 NTSC", new BMessage(MENU_PLUS4_MODEL_V364_NTSC)));
            menu->AddItem(new BMenuItem("C232 NTSC", new BMessage(MENU_PLUS4_MODEL_C232_NTSC)));
    }

    if (machine_class == VICE_MACHINE_C64DTV) {
        uppermenu->AddItem(menu = new BMenu("C64DTV model"));
            menu->SetRadioMode(true);
            menu->AddItem(new BMenuItem("C64DTV v2 (PAL)", new BMessage(MENU_C64DTV_MODEL_V2_PAL)));
            menu->AddItem(new BMenuItem("C64DTV v2 (NTSC)", new BMessage(MENU_C64DTV_MODEL_V2_NTSC)));
            menu->AddItem(new BMenuItem("C64DTV v3 (PAL)", new BMessage(MENU_C64DTV_MODEL_V3_PAL)));
            menu->AddItem(new BMenuItem("C64DTV v3 (NTSC)", new BMessage(MENU_C64DTV_MODEL_V3_NTSC)));
            menu->AddItem(new BMenuItem("Hummer (NTSC)", new BMessage(MENU_C64DTV_MODEL_HUMMER_NTSC)));
    }

    if (machine_class == VICE_MACHINE_CBM5x0) {
        uppermenu->AddItem(menu = new BMenu("CBM5x0 model"));
            menu->SetRadioMode(true);
            menu->AddItem(new BMenuItem("510 (PAL)", new BMessage(MENU_CBM5X0_MODEL_510_PAL)));
            menu->AddItem(new BMenuItem("510 (NTSC)", new BMessage(MENU_CBM5X0_MODEL_510_NTSC)));
    }

    if (machine_class == VICE_MACHINE_CBM6x0) {
        uppermenu->AddItem(menu = new BMenu("CBM2 model"));
            menu->SetRadioMode(true);
            menu->AddItem(new BMenuItem("610 (PAL)", new BMessage(MENU_CBM2_MODEL_610_PAL)));
            menu->AddItem(new BMenuItem("610 (NTSC)", new BMessage(MENU_CBM2_MODEL_610_NTSC)));
            menu->AddItem(new BMenuItem("620 (PAL)", new BMessage(MENU_CBM2_MODEL_620_PAL)));
            menu->AddItem(new BMenuItem("620 (NTSC)", new BMessage(MENU_CBM2_MODEL_620_NTSC)));
            menu->AddItem(new BMenuItem("620+ (PAL)", new BMessage(MENU_CBM2_MODEL_620PLUS_PAL)));
            menu->AddItem(new BMenuItem("620+ (NTSC)", new BMessage(MENU_CBM2_MODEL_620PLUS_NTSC)));
            menu->AddItem(new BMenuItem("710 (NTSC)", new BMessage(MENU_CBM2_MODEL_710_NTSC)));
            menu->AddItem(new BMenuItem("720 (NTSC)", new BMessage(MENU_CBM2_MODEL_720_NTSC)));
            menu->AddItem(new BMenuItem("720+ (NTSC)", new BMessage(MENU_CBM2_MODEL_720PLUS_NTSC)));
    }

    if (machine_class == VICE_MACHINE_C128) {
        uppermenu->AddItem(menu = new BMenu("C128 model"));
            menu->SetRadioMode(true);
            menu->AddItem(new BMenuItem("C128 PAL", new BMessage(MENU_C128_MODEL_C128_PAL)));
            menu->AddItem(new BMenuItem("C128DCR PAL", new BMessage(MENU_C128_MODEL_C128DCR_PAL)));
            menu->AddItem(new BMenuItem("C128 NTSC", new BMessage(MENU_C128_MODEL_C128_NTSC)));
            menu->AddItem(new BMenuItem("C128DCR NTSC", new BMessage(MENU_C128_MODEL_C128DCR_NTSC)));
    }

    if (machine_class == VICE_MACHINE_PET) {
        uppermenu->AddItem(menu = new BMenu("PET model"));
            menu->SetRadioMode(true);
            menu->AddItem(new BMenuItem("PET 2001", new BMessage(MENU_PET_MODEL_2001)));
            menu->AddItem(new BMenuItem("PET 3008", new BMessage(MENU_PET_MODEL_3008)));
            menu->AddItem(new BMenuItem("PET 3016", new BMessage(MENU_PET_MODEL_3016)));
            menu->AddItem(new BMenuItem("PET 3032", new BMessage(MENU_PET_MODEL_3032)));
            menu->AddItem(new BMenuItem("PET 3032B", new BMessage(MENU_PET_MODEL_3032B)));
            menu->AddItem(new BMenuItem("PET 4016", new BMessage(MENU_PET_MODEL_4016)));
            menu->AddItem(new BMenuItem("PET 4032", new BMessage(MENU_PET_MODEL_4032)));
            menu->AddItem(new BMenuItem("PET 4032B", new BMessage(MENU_PET_MODEL_4032B)));
            menu->AddItem(new BMenuItem("PET 8032", new BMessage(MENU_PET_MODEL_8032)));
            menu->AddItem(new BMenuItem("PET 8096", new BMessage(MENU_PET_MODEL_8096)));
            menu->AddItem(new BMenuItem("PET 8296", new BMessage(MENU_PET_MODEL_8296)));
            menu->AddItem(new BMenuItem("SuperPET", new BMessage(MENU_PET_MODEL_SUPERPET)));
    }

    if (machine_class == VICE_MACHINE_VIC20) {
        uppermenu->AddItem(menu = new BMenu("VIC20 model"));
            menu->SetRadioMode(true);
            menu->AddItem(new BMenuItem("VIC20 PAL", new BMessage(MENU_VIC20_MODEL_VIC20_PAL)));
            menu->AddItem(new BMenuItem("VIC20 NTSC", new BMessage(MENU_VIC20_MODEL_VIC20_NTSC)));
            menu->AddItem(new BMenuItem("VIC21/SuperVIC", new BMessage(MENU_VIC20_MODEL_VIC21)));
    }

    if (machine_class == VICE_MACHINE_C128) {
        uppermenu->AddItem(menu = new BMenu("Internal Function ROM options"));
            menu->AddItem(submenu = new BMenu("Internal Function ROM type"));
                submenu->SetRadioMode(true);
                submenu->AddItem(new BMenuItem("None", new BMessage(MENU_C128_INTERNAL_FUNCTION_NONE)));
                submenu->AddItem(new BMenuItem("ROM", new BMessage(MENU_C128_INTERNAL_FUNCTION_ROM)));
                submenu->AddItem(new BMenuItem("RAM", new BMessage(MENU_C128_INTERNAL_FUNCTION_RAM)));
                submenu->AddItem(new BMenuItem("RAM+RTC", new BMessage(MENU_C128_INTERNAL_FUNCTION_RTC)));
            menu->AddItem(new BMenuItem("Select ROM file", new BMessage(MENU_C128_INTERNAL_FUNCTION_FILE)));
            menu->AddItem(new BMenuItem("Save Internal Function RTC data when changed", new BMessage(MENU_TOGGLE_INT_FUNCTION_RTC_SAVE)));

        uppermenu->AddItem(menu = new BMenu("External Function ROM options"));
            menu->AddItem(submenu = new BMenu("External Function ROM type"));
                submenu->SetRadioMode(true);
                submenu->AddItem(new BMenuItem("None", new BMessage(MENU_C128_EXTERNAL_FUNCTION_NONE)));
                submenu->AddItem(new BMenuItem("ROM", new BMessage(MENU_C128_EXTERNAL_FUNCTION_ROM)));
                submenu->AddItem(new BMenuItem("RAM", new BMessage(MENU_C128_EXTERNAL_FUNCTION_RAM)));
                submenu->AddItem(new BMenuItem("RAM+RTC", new BMessage(MENU_C128_EXTERNAL_FUNCTION_RTC)));
            menu->AddItem(new BMenuItem("Select ROM file", new BMessage(MENU_C128_EXTERNAL_FUNCTION_FILE)));
            menu->AddItem(new BMenuItem("Save External Function RTC data when changed", new BMessage(MENU_TOGGLE_EXT_FUNCTION_RTC_SAVE)));
    }

    if (machine_class == VICE_MACHINE_PET || machine_class == VICE_MACHINE_PLUS4 ||
        machine_class == VICE_MACHINE_CBM5x0 || machine_class == VICE_MACHINE_CBM6x0) {
        uppermenu->AddItem(menu = new BMenu("Drive sync factor"));
            menu->SetRadioMode(true);
            menu->AddItem(new BMenuItem("PAL", new BMessage(MENU_SYNC_FACTOR_PAL)));
            menu->AddItem(new BMenuItem("NTSC", new BMessage(MENU_SYNC_FACTOR_NTSC)));
    }

    if (machine_class == VICE_MACHINE_C64DTV) {
        uppermenu->AddItem(new BMenuItem("PS/2 mouse", new BMessage(MENU_TOGGLE_PS2MOUSE)));
        uppermenu->AddSeparatorItem();
        uppermenu->AddItem(menu = new BMenu("C64DTV Options"));
            menu->AddItem(new BMenuItem("C64DTV ROM File", new BMessage(MENU_C64DTV_ROM_FILE)));
            menu->AddItem(submenu = new BMenu("C64DTV revision"));
                submenu->SetRadioMode(true);
                submenu->AddItem(new BMenuItem("DTV2", new BMessage(MENU_C64DTV_REVISION_2)));
                submenu->AddItem(new BMenuItem("DTV3", new BMessage(MENU_C64DTV_REVISION_3)));
            menu->AddItem(new BMenuItem("C64DTV ROM writable", new BMessage(MENU_TOGGLE_C64DTV_WRITE_ENABLE)));
            menu->AddItem(new BMenuItem("Enable Hummer ADC", new BMessage(MENU_HUMMER_USERPORT_ADC)));
    }

    if (machine_class == VICE_MACHINE_SCPU64) {
        uppermenu->AddItem(menu = new BMenu("SuperCPU64 Options"));
            menu->AddItem(submenu = new BMenu("SIMM size"));
                submenu->SetRadioMode(true);
                submenu->AddItem(new BMenuItem("0 MB", new BMessage(MENU_SCPU64_SIMM_SIZE_0)));
                submenu->AddItem(new BMenuItem("1 MB", new BMessage(MENU_SCPU64_SIMM_SIZE_1)));
                submenu->AddItem(new BMenuItem("4 MB", new BMessage(MENU_SCPU64_SIMM_SIZE_4)));
                submenu->AddItem(new BMenuItem("8 MB", new BMessage(MENU_SCPU64_SIMM_SIZE_8)));
                submenu->AddItem(new BMenuItem("16 MB", new BMessage(MENU_SCPU64_SIMM_SIZE_16)));
            menu->AddItem(new BMenuItem("Enable jiffy switch", new BMessage(MENU_TOGGLE_SCPU64_JIFFY_ENABLE)));
            menu->AddItem(new BMenuItem("Enable speed switch", new BMessage(MENU_TOGGLE_SCPU64_SPEED_ENABLE)));
    }

    if (get_devices != NULL) {
        uppermenu->AddSeparatorItem();
        uppermenu->AddItem(new BMenuItem("Grab mouse events", new BMessage(MENU_TOGGLE_MOUSE)));
        uppermenu->AddSeparatorItem();
    }

#if defined(HAVE_RS232DEV) || defined(HAVE_RS232NET)
    if (machine_class != VICE_MACHINE_C64DTV && machine_class != VICE_MACHINE_VSID) {
        uppermenu->AddItem(menu = new BMenu("Expansion Carts"));
        if (machine_class == VICE_MACHINE_VIC20) {
            menu->AddItem(submenu = new BMenu("ACIA Options (MasC=uerade)"));
        } else {
            menu->AddItem(submenu = new BMenu("ACIA Options"));
        }
        if (machine_class != VICE_MACHINE_PET && machine_class != VICE_MACHINE_CBM5x0 && machine_class != VICE_MACHINE_CBM6x0) {
            submenu->AddItem(new BMenuItem("ACIA emulation", new BMessage(MENU_TOGGLE_ACIA)));
        }
        submenu->AddItem(extsubmenu = new BMenu("ACIA device"));
            extsubmenu->SetRadioMode(true);
            extsubmenu->AddItem(new BMenuItem("RS232 device 1", new BMessage(MENU_ACIA_RS323_DEVICE_1)));
            extsubmenu->AddItem(new BMenuItem("RS232 device 2", new BMessage(MENU_ACIA_RS323_DEVICE_2)));
            extsubmenu->AddItem(new BMenuItem("RS232 device 3", new BMessage(MENU_ACIA_RS323_DEVICE_3)));
            extsubmenu->AddItem(new BMenuItem("RS232 device 4", new BMessage(MENU_ACIA_RS323_DEVICE_4)));
        if (machine_class != VICE_MACHINE_PLUS4 && machine_class != VICE_MACHINE_PET && 
            machine_class != VICE_MACHINE_CBM5x0 && machine_class != VICE_MACHINE_CBM6x0) {
            submenu->AddItem(extsubmenu = new BMenu("ACIA base"));
                extsubmenu->SetRadioMode(true);
                if (machine_class == VICE_MACHINE_VIC20) {
                    extsubmenu->AddItem(new BMenuItem("$9800", new BMessage(MENU_ACIA_BASE_9800)));
                    extsubmenu->AddItem(new BMenuItem("$9C00", new BMessage(MENU_ACIA_BASE_9C00)));
                } else if (machine_class == VICE_MACHINE_C128) {
                    extsubmenu->AddItem(new BMenuItem("$D700", new BMessage(MENU_ACIA_BASE_D700)));
                    extsubmenu->AddItem(new BMenuItem("$DE00", new BMessage(MENU_ACIA_BASE_DE00)));
                    extsubmenu->AddItem(new BMenuItem("$DF00", new BMessage(MENU_ACIA_BASE_DF00)));
                } else {
                    extsubmenu->AddItem(new BMenuItem("$DE00", new BMessage(MENU_ACIA_BASE_DE00)));
                    extsubmenu->AddItem(new BMenuItem("$DF00", new BMessage(MENU_ACIA_BASE_DF00)));
                }
            submenu->AddItem(extsubmenu = new BMenu("ACIA interrupt"));
                extsubmenu->SetRadioMode(true);
                extsubmenu->AddItem(new BMenuItem("None", new BMessage(MENU_ACIA_INT_NONE)));
                extsubmenu->AddItem(new BMenuItem("IRQ", new BMessage(MENU_ACIA_INT_IRQ)));
                extsubmenu->AddItem(new BMenuItem("NMI", new BMessage(MENU_ACIA_INT_NMI)));
            submenu->AddItem(extsubmenu = new BMenu("ACIA mode"));
                extsubmenu->SetRadioMode(true);
                extsubmenu->AddItem(new BMenuItem("Normal", new BMessage(MENU_ACIA_MODE_NORMAL)));
                extsubmenu->AddItem(new BMenuItem("Swiftlink", new BMessage(MENU_ACIA_MODE_SWIFTLINK)));
                extsubmenu->AddItem(new BMenuItem("Turbo232", new BMessage(MENU_ACIA_MODE_TURBO232)));
        }
    }
#else
    if (machine_class != VICE_MACHINE_C64DTV && machine_class != VICE_MACHINE_VSID &&
        machine_class != VICE_MACHINE_CBM5x0 && machine_class != VICE_MACHINE_CBM6x0) {
        uppermenu->AddItem(menu = new BMenu("Expansion Carts"));
    }
#endif

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC ||
        machine_class == VICE_MACHINE_C128 || machine_class == VICE_MACHINE_SCPU64) {
            menu->AddItem(submenu = new BMenu("REU Options"));
                submenu->AddItem(new BMenuItem("REU emulation", new BMessage(MENU_TOGGLE_REU)));
                submenu->AddItem(extsubmenu = new BMenu("REU size"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("128 kB", new BMessage(MENU_REU_SIZE_128)));
                    extsubmenu->AddItem(new BMenuItem("256 kB", new BMessage(MENU_REU_SIZE_256)));
                    extsubmenu->AddItem(new BMenuItem("512 kB", new BMessage(MENU_REU_SIZE_512)));
                    extsubmenu->AddItem(new BMenuItem("1024 kB", new BMessage(MENU_REU_SIZE_1024)));
                    extsubmenu->AddItem(new BMenuItem("2048 kB", new BMessage(MENU_REU_SIZE_2048)));
                    extsubmenu->AddItem(new BMenuItem("4096 kB", new BMessage(MENU_REU_SIZE_4096)));
                    extsubmenu->AddItem(new BMenuItem("8192 kB", new BMessage(MENU_REU_SIZE_8192)));
                    extsubmenu->AddItem(new BMenuItem("16384 kB", new BMessage(MENU_REU_SIZE_16384)));
                submenu->AddItem(new BMenuItem("Save to REU image when changed", new BMessage(MENU_TOGGLE_REU_SWC)));
                submenu->AddItem(new BMenuItem("REU File", new BMessage(MENU_REU_FILE)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC ||
        machine_class == VICE_MACHINE_C128 || machine_class == VICE_MACHINE_SCPU64 ||
        machine_class == VICE_MACHINE_VIC20) {
                if (machine_class == VICE_MACHINE_VIC20) {
                    menu->AddItem(submenu = new BMenu("GEO-RAM Options (MasC=uerade)"));
                } else {
                    menu->AddItem(submenu = new BMenu("GEO-RAM Options"));
                }
                submenu->AddItem(new BMenuItem("GEO-RAM emulation", new BMessage(MENU_TOGGLE_GEORAM)));
                submenu->AddItem(extsubmenu = new BMenu("GEO-RAM size"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("64 kB", new BMessage(MENU_GEORAM_SIZE_64)));
                    extsubmenu->AddItem(new BMenuItem("128 kB", new BMessage(MENU_GEORAM_SIZE_128)));
                    extsubmenu->AddItem(new BMenuItem("256 kB", new BMessage(MENU_GEORAM_SIZE_256)));
                    extsubmenu->AddItem(new BMenuItem("512 kB", new BMessage(MENU_GEORAM_SIZE_512)));
                    extsubmenu->AddItem(new BMenuItem("1024 kB", new BMessage(MENU_GEORAM_SIZE_1024)));
                    extsubmenu->AddItem(new BMenuItem("2048 kB", new BMessage(MENU_GEORAM_SIZE_2048)));
                    extsubmenu->AddItem(new BMenuItem("4096 kB", new BMessage(MENU_GEORAM_SIZE_4096)));
                submenu->AddItem(new BMenuItem("Save to GEO-RAM image when changed", new BMessage(MENU_TOGGLE_GEORAM_SWC)));
                if (machine_class == VICE_MACHINE_VIC20) {
                    submenu->AddItem(new BMenuItem("I/O Swap", new BMessage(MENU_TOGGLE_GEORAM_IO_SWAP)));
                }
                submenu->AddItem(new BMenuItem("GEO-RAM File", new BMessage(MENU_GEORAM_FILE)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC ||
        machine_class == VICE_MACHINE_C128 || machine_class == VICE_MACHINE_SCPU64) {
            menu->AddItem(submenu = new BMenu("RamCart Options"));
                submenu->AddItem(new BMenuItem("RamCart emulation", new BMessage(MENU_TOGGLE_RAMCART)));
                submenu->AddItem(extsubmenu = new BMenu("RamCart size"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("64 kB", new BMessage(MENU_RAMCART_SIZE_64)));
                    extsubmenu->AddItem(new BMenuItem("128 kB", new BMessage(MENU_RAMCART_SIZE_128)));
                submenu->AddItem(new BMenuItem("Save to RamCart image when changed", new BMessage(MENU_TOGGLE_RAMCART_SWC)));
                submenu->AddItem(new BMenuItem("RamCart File", new BMessage(MENU_RAMCART_FILE)));

            menu->AddItem(submenu = new BMenu("IDE64 Options"));
                submenu->AddItem(extsubmenu = new BMenu("IDE64 version"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("V3", new BMessage(MENU_IDE64_VERSION_V3)));
                    extsubmenu->AddItem(new BMenuItem("V4.1", new BMessage(MENU_IDE64_VERSION_V4_1)));
                    extsubmenu->AddItem(new BMenuItem("V4.2", new BMessage(MENU_IDE64_VERSION_V4_2)));
                submenu->AddItem(new BMenuItem("Save IDE64 RTC data when changed", new BMessage(MENU_TOGGLE_IDE64_RTC_SAVE)));
                submenu->AddItem(new BMenuItem("IDE64 device #1 File", new BMessage(MENU_IDE64_FILE1)));
                submenu->AddItem(new BMenuItem("IDE64 device #1 image size ...", new BMessage(MENU_IDE64_SIZE1)));
                submenu->AddItem(new BMenuItem("IDE64 device #2 File", new BMessage(MENU_IDE64_FILE2)));
                submenu->AddItem(new BMenuItem("IDE64 device #2 image size ...", new BMessage(MENU_IDE64_SIZE2)));
                submenu->AddItem(new BMenuItem("IDE64 device #3 File", new BMessage(MENU_IDE64_FILE3)));
                submenu->AddItem(new BMenuItem("IDE64 device #3 image size ...", new BMessage(MENU_IDE64_SIZE3)));
                submenu->AddItem(new BMenuItem("IDE64 device #4 File", new BMessage(MENU_IDE64_FILE4)));
                submenu->AddItem(new BMenuItem("IDE64 device #4 image size ...", new BMessage(MENU_IDE64_SIZE4)));
                submenu->AddItem(new BMenuItem("IDE64 shortbus DigiMAX emulation", new BMessage(MENU_TOGGLE_IDE64_SB_DIGIMAX)));
                submenu->AddItem(extsubmenu = new BMenu("IDE64 shortbus DigiMAX base"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("$DE40", new BMessage(MENU_IDE64_SB_DIGIMAX_BASE_DE40)));
                    extsubmenu->AddItem(new BMenuItem("$DE48", new BMessage(MENU_IDE64_SB_DIGIMAX_BASE_DE48)));
#ifdef HAVE_RAWNET
                submenu->AddItem(new BMenuItem("IDE64 shortbus ETFE device emulation", new BMessage(MENU_TOGGLE_IDE64_SB_ETFE)));
                submenu->AddItem(extsubmenu = new BMenu("IDE64 shortbus ETFE device base"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("$DE00", new BMessage(MENU_IDE64_SB_ETFE_BASE_DE00)));
                    extsubmenu->AddItem(new BMenuItem("$DE10", new BMessage(MENU_IDE64_SB_ETFE_BASE_DE10)));
                    extsubmenu->AddItem(new BMenuItem("$DF00", new BMessage(MENU_IDE64_SB_ETFE_BASE_DF00)));
#endif
                submenu->AddItem(extsubmenu = new BMenu("IDE64 ClockPort device"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("None", new BMessage(MENU_IDE64_CLOCKPORT_NONE)));
#ifdef HAVE_RAWNET
                    extsubmenu->AddItem(new BMenuItem("RR-Net", new BMessage(MENU_IDE64_CLOCKPORT_RRNET)));
#endif
#ifdef USE_MPG123
                    extsubmenu->AddItem(new BMenuItem("MP3@64", new BMessage(MENU_IDE64_CLOCKPORT_MP3AT64)));
#endif
                submenu->AddItem(new BMenuItem("IDE64 USB server settings ...", new BMessage(MENU_IDE64_USB_SERVER)));

            menu->AddItem(submenu = new BMenu("Magic Voice Options"));
                submenu->AddItem(new BMenuItem("Magic Voice emulation", new BMessage(MENU_TOGGLE_MAGICVOICE)));
                submenu->AddItem(new BMenuItem("Magic Voice File", new BMessage(MENU_MAGICVOICE_FILE)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC ||
        machine_class == VICE_MACHINE_C128 || machine_class == VICE_MACHINE_SCPU64 ||
        machine_class == VICE_MACHINE_VIC20) {
                if (machine_class == VICE_MACHINE_VIC20) {
                    menu->AddItem(submenu = new BMenu("DigiMAX Options (MasC=uerade)"));
                } else {
                    menu->AddItem(submenu = new BMenu("DigiMAX Options"));
                }
                submenu->AddItem(new BMenuItem("DigiMAX emulation", new BMessage(MENU_TOGGLE_DIGIMAX)));
                submenu->AddItem(extsubmenu = new BMenu("DigiMAX base"));
                    extsubmenu->SetRadioMode(true);
                    if (machine_class == VICE_MACHINE_VIC20) {
                        extsubmenu->AddItem(new BMenuItem("$9800", new BMessage(MENU_DIGIMAX_BASE_9800)));
                        extsubmenu->AddItem(new BMenuItem("$9820", new BMessage(MENU_DIGIMAX_BASE_9820)));
                        extsubmenu->AddItem(new BMenuItem("$9840", new BMessage(MENU_DIGIMAX_BASE_9840)));
                        extsubmenu->AddItem(new BMenuItem("$9860", new BMessage(MENU_DIGIMAX_BASE_9860)));
                        extsubmenu->AddItem(new BMenuItem("$9880", new BMessage(MENU_DIGIMAX_BASE_9880)));
                        extsubmenu->AddItem(new BMenuItem("$98A0", new BMessage(MENU_DIGIMAX_BASE_98A0)));
                        extsubmenu->AddItem(new BMenuItem("$98C0", new BMessage(MENU_DIGIMAX_BASE_98C0)));
                        extsubmenu->AddItem(new BMenuItem("$98E0", new BMessage(MENU_DIGIMAX_BASE_98E0)));
                        extsubmenu->AddItem(new BMenuItem("$9C00", new BMessage(MENU_DIGIMAX_BASE_9C00)));
                        extsubmenu->AddItem(new BMenuItem("$9C20", new BMessage(MENU_DIGIMAX_BASE_9C20)));
                        extsubmenu->AddItem(new BMenuItem("$9C40", new BMessage(MENU_DIGIMAX_BASE_9C40)));
                        extsubmenu->AddItem(new BMenuItem("$9C60", new BMessage(MENU_DIGIMAX_BASE_9C60)));
                        extsubmenu->AddItem(new BMenuItem("$9C80", new BMessage(MENU_DIGIMAX_BASE_9C80)));
                        extsubmenu->AddItem(new BMenuItem("$9CA0", new BMessage(MENU_DIGIMAX_BASE_9CA0)));
                        extsubmenu->AddItem(new BMenuItem("$9CC0", new BMessage(MENU_DIGIMAX_BASE_9CC0)));
                        extsubmenu->AddItem(new BMenuItem("$9CE0", new BMessage(MENU_DIGIMAX_BASE_9CE0)));
                    } else {
                        extsubmenu->AddItem(new BMenuItem("$DE00", new BMessage(MENU_DIGIMAX_BASE_DE00)));
                        extsubmenu->AddItem(new BMenuItem("$DE20", new BMessage(MENU_DIGIMAX_BASE_DE20)));
                        extsubmenu->AddItem(new BMenuItem("$DE40", new BMessage(MENU_DIGIMAX_BASE_DE40)));
                        extsubmenu->AddItem(new BMenuItem("$DE60", new BMessage(MENU_DIGIMAX_BASE_DE60)));
                        extsubmenu->AddItem(new BMenuItem("$DE80", new BMessage(MENU_DIGIMAX_BASE_DE80)));
                        extsubmenu->AddItem(new BMenuItem("$DEA0", new BMessage(MENU_DIGIMAX_BASE_DEA0)));
                        extsubmenu->AddItem(new BMenuItem("$DEC0", new BMessage(MENU_DIGIMAX_BASE_DEC0)));
                        extsubmenu->AddItem(new BMenuItem("$DEE0", new BMessage(MENU_DIGIMAX_BASE_DEE0)));
                        extsubmenu->AddItem(new BMenuItem("$DF00", new BMessage(MENU_DIGIMAX_BASE_DF00)));
                        extsubmenu->AddItem(new BMenuItem("$DF20", new BMessage(MENU_DIGIMAX_BASE_DF20)));
                        extsubmenu->AddItem(new BMenuItem("$DF40", new BMessage(MENU_DIGIMAX_BASE_DF40)));
                        extsubmenu->AddItem(new BMenuItem("$DF60", new BMessage(MENU_DIGIMAX_BASE_DF60)));
                        extsubmenu->AddItem(new BMenuItem("$DF80", new BMessage(MENU_DIGIMAX_BASE_DF80)));
                        extsubmenu->AddItem(new BMenuItem("$DFA0", new BMessage(MENU_DIGIMAX_BASE_DFA0)));
                        extsubmenu->AddItem(new BMenuItem("$DFC0", new BMessage(MENU_DIGIMAX_BASE_DFC0)));
                        extsubmenu->AddItem(new BMenuItem("$DFE0", new BMessage(MENU_DIGIMAX_BASE_DFE0)));
                    }
            if (machine_class == VICE_MACHINE_VIC20) {
                menu->AddItem(submenu = new BMenu("SFX Sound Expander Options (MasC=uerade)"));
            } else {
                menu->AddItem(submenu = new BMenu("SFX Sound Expander Options"));
            }
                submenu->AddItem(new BMenuItem("SFX Sound Expander emulation", new BMessage(MENU_TOGGLE_SFX_SE)));
                submenu->AddItem(extsubmenu = new BMenu("SFX Sound Expander YM chip"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("3526", new BMessage(MENU_SFX_SE_3526)));
                    extsubmenu->AddItem(new BMenuItem("3812", new BMessage(MENU_SFX_SE_3812)));
                if (machine_class == VICE_MACHINE_VIC20) {
                    submenu->AddItem(new BMenuItem("I/O Swap", new BMessage(MENU_TOGGLE_SFX_SE_IO_SWAP)));
                }
            if (machine_class == VICE_MACHINE_VIC20) {
                menu->AddItem(submenu = new BMenu("SFX Sound Sampler Options (MasC=uerade)"));
            } else {
                menu->AddItem(submenu = new BMenu("SFX Sound Sampler Options"));
            }
                submenu->AddItem(new BMenuItem("SFX Sound Sampler emulation", new BMessage(MENU_TOGGLE_SFX_SS)));
                if (machine_class == VICE_MACHINE_VIC20) {
                    submenu->AddItem(new BMenuItem("I/O Swap", new BMessage(MENU_TOGGLE_SFX_SS_IO_SWAP)));
                }
            if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC) {
                menu->AddItem(submenu = new BMenu("CP/M Cartridge Options"));
                submenu->AddItem(new BMenuItem("CP/M Cartridge emulation", new BMessage(MENU_TOGGLE_CPM_CART)));
            }
            if (machine_class == VICE_MACHINE_VIC20) {
                menu->AddItem(submenu = new BMenu("DS12C887 RTC Options (MasC=uerade)"));
            } else {
                menu->AddItem(submenu = new BMenu("DS12C887 RTC Options"));
            }
                submenu->AddItem(new BMenuItem("DS12C887 RTC emulation", new BMessage(MENU_TOGGLE_DS12C887_RTC)));
                submenu->AddItem(new BMenuItem("DS12C887 RTC running mode (running)", new BMessage(MENU_TOGGLE_DS12C887_RTC_RUNNING_MODE)));
                submenu->AddItem(extsubmenu = new BMenu("DS12C887 base"));
                    extsubmenu->SetRadioMode(true);
                    if (machine_class == VICE_MACHINE_VIC20) {
                        extsubmenu->AddItem(new BMenuItem("$9800", new BMessage(MENU_DS12C887_RTC_BASE_9800)));
                        extsubmenu->AddItem(new BMenuItem("$9C00", new BMessage(MENU_DS12C887_RTC_BASE_9C00)));
                    } else if (machine_class == VICE_MACHINE_C128) {
                        extsubmenu->AddItem(new BMenuItem("$D700", new BMessage(MENU_DS12C887_RTC_BASE_D700)));
                        extsubmenu->AddItem(new BMenuItem("$DE00", new BMessage(MENU_DS12C887_RTC_BASE_DE00)));
                        extsubmenu->AddItem(new BMenuItem("$DF00", new BMessage(MENU_DS12C887_RTC_BASE_DF00)));
                    } else {
                        extsubmenu->AddItem(new BMenuItem("$D500", new BMessage(MENU_DS12C887_RTC_BASE_D500)));
                        extsubmenu->AddItem(new BMenuItem("$D600", new BMessage(MENU_DS12C887_RTC_BASE_D600)));
                        extsubmenu->AddItem(new BMenuItem("$D700", new BMessage(MENU_DS12C887_RTC_BASE_D700)));
                        extsubmenu->AddItem(new BMenuItem("$DE00", new BMessage(MENU_DS12C887_RTC_BASE_DE00)));
                        extsubmenu->AddItem(new BMenuItem("$DF00", new BMessage(MENU_DS12C887_RTC_BASE_DF00)));
                    }
                submenu->AddItem(new BMenuItem("Save DS12C887 RTC data when changed", new BMessage(MENU_TOGGLE_DS12C887_RTC_SAVE)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC ||
        machine_class == VICE_MACHINE_C128 || machine_class == VICE_MACHINE_SCPU64) {
            menu->AddItem(submenu = new BMenu("EasyFlash Options"));
                submenu->AddItem(new BMenuItem("Jumper", new BMessage(MENU_TOGGLE_EASYFLASH_JUMPER)));
                submenu->AddItem(new BMenuItem("Save to .crt file on detach", new BMessage(MENU_TOGGLE_EASYFLASH_AUTOSAVE)));
                submenu->AddItem(new BMenuItem("Optimize .crt file", new BMessage(MENU_TOGGLE_EASYFLASH_OPTIMIZE)));
                submenu->AddItem(new BMenuItem("Save .crt file now", new BMessage(MENU_EASYFLASH_SAVE_NOW)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC || machine_class == VICE_MACHINE_SCPU64) {
            menu->AddItem(submenu = new BMenu("Double Quick Brown Box Options"));
                submenu->AddItem(new BMenuItem("DQBB emulation", new BMessage(MENU_TOGGLE_DQBB)));
                submenu->AddItem(new BMenuItem("Save to DQBB image when changed", new BMessage(MENU_TOGGLE_DQBB_SWC)));
                submenu->AddItem(new BMenuItem("DQBB File", new BMessage(MENU_DQBB_FILE)));
            menu->AddItem(submenu = new BMenu("ISEPIC Options"));
                submenu->AddItem(new BMenuItem("ISEPIC emulation", new BMessage(MENU_TOGGLE_ISEPIC)));
                submenu->AddItem(new BMenuItem("ISEPIC switch", new BMessage(MENU_TOGGLE_ISEPIC_SWITCH)));
                submenu->AddItem(new BMenuItem("Save to ISEPIC image when changed", new BMessage(MENU_TOGGLE_ISEPIC_SWC)));
                submenu->AddItem(new BMenuItem("ISEPIC File", new BMessage(MENU_ISEPIC_FILE)));
            menu->AddItem(submenu = new BMenu("Expert Cartridge Options"));
                submenu->AddItem(new BMenuItem("Expert Cartridge emulation", new BMessage(MENU_TOGGLE_EXPERT)));
                submenu->AddItem(extsubmenu = new BMenu("Expert Cartridge mode"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Off", new BMessage(MENU_EXPERT_MODE_OFF)));
                    extsubmenu->AddItem(new BMenuItem("Prg", new BMessage(MENU_EXPERT_MODE_PRG)));
                    extsubmenu->AddItem(new BMenuItem("On", new BMessage(MENU_EXPERT_MODE_ON)));
                submenu->AddItem(new BMenuItem("Save to Expert Cartridge image when changed", new BMessage(MENU_TOGGLE_EXPERT_SWC)));
                submenu->AddItem(new BMenuItem("Expert Cartridge File", new BMessage(MENU_EXPERT_FILE)));
            menu->AddItem(submenu = new BMenu("Burst Mode Modification Options"));
                submenu->SetRadioMode(true);
                submenu->AddItem(new BMenuItem("None", new BMessage(MENU_BURSTMOD_NONE)));
                submenu->AddItem(new BMenuItem("CIA-1", new BMessage(MENU_BURSTMOD_CIA1)));
                submenu->AddItem(new BMenuItem("CIA-2", new BMessage(MENU_BURSTMOD_CIA2)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC) {
            menu->AddItem(submenu = new BMenu("Memory Expansion Hacks Options"));
                submenu->AddItem(extsubmenu = new BMenu("Memory Expansion Hack Device"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("None", new BMessage(MENU_C64_MEMORY_HACKS_NONE)));
                    extsubmenu->AddItem(new BMenuItem("C64 256K", new BMessage(MENU_C64_MEMORY_HACKS_256K)));
                    extsubmenu->AddItem(new BMenuItem("+60K", new BMessage(MENU_C64_MEMORY_HACKS_PLUS60K)));
                    extsubmenu->AddItem(new BMenuItem("+256K", new BMessage(MENU_C64_MEMORY_HACKS_PLUS256K)));
                submenu->AddItem(extsubmenu = new BMenu("C64_256K base"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("$DE00-$DE7F", new BMessage(MENU_C64_256K_BASE_DE00)));
                    extsubmenu->AddItem(new BMenuItem("$DE80-$DEFF", new BMessage(MENU_C64_256K_BASE_DE80)));
                    extsubmenu->AddItem(new BMenuItem("$DF00-$DF7F", new BMessage(MENU_C64_256K_BASE_DF00)));
                    extsubmenu->AddItem(new BMenuItem("$DF80-$DFFF", new BMessage(MENU_C64_256K_BASE_DF80)));
                submenu->AddItem(new BMenuItem("C64_256K File", new BMessage(MENU_C64_256K_FILE)));
                submenu->AddItem(extsubmenu = new BMenu("PLUS60K base"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("$D040", new BMessage(MENU_PLUS60K_BASE_D040)));
                    extsubmenu->AddItem(new BMenuItem("$D100", new BMessage(MENU_PLUS60K_BASE_D100)));
                submenu->AddItem(new BMenuItem("PLUS60K File", new BMessage(MENU_PLUS60K_FILE)));
                submenu->AddItem(new BMenuItem("PLUS256K File", new BMessage(MENU_PLUS256K_FILE)));
    }

    if (machine_class == VICE_MACHINE_PLUS4) {
            menu->AddItem(submenu = new BMenu("Memory Expansion Hacks Options"));
                submenu->AddItem(extsubmenu = new BMenu("Memory Expansion Hack Device"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("None", new BMessage(MENU_PLUS4_MEMORY_HACK_NONE)));
                    extsubmenu->AddItem(new BMenuItem("CSORY 256K", new BMessage(MENU_PLUS4_MEMORY_HACK_C256K)));
                    extsubmenu->AddItem(new BMenuItem("HANNES 256K", new BMessage(MENU_PLUS4_MEMORY_HACK_H256K)));
                    extsubmenu->AddItem(new BMenuItem("HANNES 1024K", new BMessage(MENU_PLUS4_MEMORY_HACK_H1024K)));
                    extsubmenu->AddItem(new BMenuItem("HANNES 4096K", new BMessage(MENU_PLUS4_MEMORY_HACK_H4096K)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC || machine_class == VICE_MACHINE_SCPU64 || machine_class == VICE_MACHINE_C128) {
            menu->AddItem(submenu = new BMenu("MMC64 Options"));
                submenu->AddItem(new BMenuItem("MMC64 emulation", new BMessage(MENU_TOGGLE_MMC64)));
                submenu->AddItem(extsubmenu = new BMenu("MMC64 revision"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Revision A", new BMessage(MENU_MMC64_REVISION_A)));
                    extsubmenu->AddItem(new BMenuItem("Revision B", new BMessage(MENU_MMC64_REVISION_B)));
                submenu->AddItem(new BMenuItem("MMC64 BIOS jumper", new BMessage(MENU_TOGGLE_MMC64_FLASHJUMPER)));
                submenu->AddItem(new BMenuItem("MMC64 BIOS save when changed", new BMessage(MENU_TOGGLE_MMC64_SAVE)));
                submenu->AddItem(new BMenuItem("MMC64 BIOS File", new BMessage(MENU_MMC64_BIOS_FILE)));
                submenu->AddItem(extsubmenu = new BMenu("MMC64 card type"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Auto", new BMessage(MENU_MMC64_CARD_TYPE_AUTO)));
                    extsubmenu->AddItem(new BMenuItem("MMC", new BMessage(MENU_MMC64_CARD_TYPE_MMC)));
                    extsubmenu->AddItem(new BMenuItem("SD", new BMessage(MENU_MMC64_CARD_TYPE_SD)));
                    extsubmenu->AddItem(new BMenuItem("SDHC", new BMessage(MENU_MMC64_CARD_TYPE_SDHC)));
                submenu->AddItem(new BMenuItem("MMC64 Image read-only", new BMessage(MENU_TOGGLE_MMC64_READ_ONLY)));
                submenu->AddItem(new BMenuItem("MMC64 Image File", new BMessage(MENU_MMC64_IMAGE_FILE)));
                submenu->AddItem(extsubmenu = new BMenu("MMC64 ClockPort device"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("None", new BMessage(MENU_MMC64_CLOCKPORT_NONE)));
#ifdef HAVE_RAWNET
                    extsubmenu->AddItem(new BMenuItem("RR-Net", new BMessage(MENU_MMC64_CLOCKPORT_RRNET)));
#endif
#ifdef USE_MPG123
                    extsubmenu->AddItem(new BMenuItem("MP3@64", new BMessage(MENU_MMC64_CLOCKPORT_MP3AT64)));
#endif

            menu->AddItem(submenu = new BMenu("MMC Replay Options"));
                submenu->AddItem(new BMenuItem("EEPROM read/write", new BMessage(MENU_TOGGLE_MMCR_EEPROM_READ_WRITE)));
                submenu->AddItem(new BMenuItem("Save EEPROM image when changed", new BMessage(MENU_TOGGLE_MMCR_EEPROM_SWC)));
                submenu->AddItem(new BMenuItem("EEPROM File", new BMessage(MENU_MMCR_EEPROM_FILE)));
                submenu->AddItem(new BMenuItem("Rescue mode", new BMessage(MENU_TOGGLE_MMCR_RESCUE_MODE)));
                submenu->AddItem(extsubmenu = new BMenu("MMC Replay card type"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Auto", new BMessage(MENU_MMCR_CARD_TYPE_AUTO)));
                    extsubmenu->AddItem(new BMenuItem("MMC", new BMessage(MENU_MMCR_CARD_TYPE_MMC)));
                    extsubmenu->AddItem(new BMenuItem("SD", new BMessage(MENU_MMCR_CARD_TYPE_SD)));
                    extsubmenu->AddItem(new BMenuItem("SDHC", new BMessage(MENU_MMCR_CARD_TYPE_SDHC)));
                submenu->AddItem(new BMenuItem("MMC Replay Image read/write", new BMessage(MENU_TOGGLE_MMCR_READ_WRITE)));
                submenu->AddItem(new BMenuItem("MMC Replay Image File", new BMessage(MENU_MMCR_IMAGE_FILE)));
                submenu->AddItem(extsubmenu = new BMenu("MMC Replay ClockPort device"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("None", new BMessage(MENU_MMCR_CLOCKPORT_NONE)));
#ifdef HAVE_RAWNET
                    extsubmenu->AddItem(new BMenuItem("RR-Net", new BMessage(MENU_MMCR_CLOCKPORT_RRNET)));
#endif
#ifdef USE_MPG123
                    extsubmenu->AddItem(new BMenuItem("MP3@64", new BMessage(MENU_MMCR_CLOCKPORT_MP3AT64)));
#endif

            menu->AddItem(submenu = new BMenu("Retro Replay Options"));
                submenu->AddItem(new BMenuItem("Enable RR flash jumper", new BMessage(MENU_TOGGLE_RR_FLASH_JUMPER)));
                submenu->AddItem(new BMenuItem("Enable RR bank jumper", new BMessage(MENU_TOGGLE_RR_BANK_JUMPER)));
                submenu->AddItem(new BMenuItem("Save RR flash when changed", new BMessage(MENU_TOGGLE_SAVE_RR_FLASH)));
                submenu->AddItem(extsubmenu = new BMenu("RR Revision"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Retro Replay", new BMessage(MENU_RR_REV_RETRO)));
                    extsubmenu->AddItem(new BMenuItem("Nordic Replay", new BMessage(MENU_RR_REV_NORDIC)));
                submenu->AddItem(extsubmenu = new BMenu("Retro Replay ClockPort device"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("None", new BMessage(MENU_RR_CLOCKPORT_NONE)));
#ifdef HAVE_RAWNET
                    extsubmenu->AddItem(new BMenuItem("RR-Net", new BMessage(MENU_RR_CLOCKPORT_RRNET)));
#endif
#ifdef USE_MPG123
                    extsubmenu->AddItem(new BMenuItem("MP3@64", new BMessage(MENU_RR_CLOCKPORT_MP3AT64)));
#endif

            menu->AddItem(submenu = new BMenu("GMod2 Options"));
                submenu->AddItem(new BMenuItem("FLASH read/write", new BMessage(MENU_TOGGLE_GMOD2_FLASH_READ_WRITE)));
                submenu->AddItem(new BMenuItem("Save EEPROM image when changed", new BMessage(MENU_TOGGLE_GMOD2_EEPROM_SWC)));
                submenu->AddItem(new BMenuItem("EEPROM File", new BMessage(MENU_GMOD2_EEPROM_FILE)));

#ifdef HAVE_RAWNET
            menu->AddItem(submenu = new BMenu("RR-NET MK3 Options"));
                submenu->AddItem(new BMenuItem("Enable RR-NET MK3 flash jumper", new BMessage(MENU_TOGGLE_RRNET_MK3_FLASH_JUMPER)));
                submenu->AddItem(new BMenuItem("Save RR-NET MK3 flash when changed", new BMessage(MENU_TOGGLE_SAVE_RRNET_MK3_FLASH)));
#endif

            menu->AddItem(submenu = new BMenu("Super Snapshot 5 Options"));
                submenu->AddItem(new BMenuItem("Enable 32KB RAM addon", new BMessage(MENU_TOGGLE_SS5_32K)));
    }

    if (machine_class == VICE_MACHINE_PET) {
            menu->AddItem(submenu = new BMenu("PET REU Options"));
                submenu->AddItem(new BMenuItem("PET REU emulation", new BMessage(MENU_TOGGLE_PETREU)));
                submenu->AddItem(extsubmenu = new BMenu("PET REU size"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("128 kB", new BMessage(MENU_PETREU_SIZE_128)));
                    extsubmenu->AddItem(new BMenuItem("512 kB", new BMessage(MENU_PETREU_SIZE_512)));
                    extsubmenu->AddItem(new BMenuItem("1024 kB", new BMessage(MENU_PETREU_SIZE_1024)));
                    extsubmenu->AddItem(new BMenuItem("2048 kB", new BMessage(MENU_PETREU_SIZE_2048)));
                submenu->AddItem(new BMenuItem("PET REU File", new BMessage(MENU_PETREU_FILE)));
            menu->AddItem(submenu = new BMenu("PET DWW Options"));
                submenu->AddItem(new BMenuItem("PET DWW emulation", new BMessage(MENU_TOGGLE_PETDWW)));
                submenu->AddItem(new BMenuItem("PET DWW File", new BMessage(MENU_PETDWW_FILE)));
            menu->AddItem(new BMenuItem("PET High Res Emulator board emulation", new BMessage(MENU_TOGGLE_PETHRE)));
    }

    if (machine_class == VICE_MACHINE_PLUS4) {
            menu->AddItem(submenu = new BMenu("V364 speech Options"));
                submenu->AddItem(new BMenuItem("V364 speech emulation", new BMessage(MENU_TOGGLE_V364SPEECH)));
                submenu->AddItem(new BMenuItem("V364 speech File", new BMessage(MENU_V364SPEECH_FILE)));
    }

    if (machine_class == VICE_MACHINE_VIC20) {
            menu->AddItem(submenu = new BMenu("Final Expansion Options"));
                submenu->AddItem(new BMenuItem("Write back to cart image", new BMessage(MENU_TOGGLE_FE_WRITE_BACK)));
            menu->AddItem(submenu = new BMenu("Mega-Cart Options"));
                submenu->AddItem(new BMenuItem("Write back to nvram file", new BMessage(MENU_TOGGLE_MC_NVRAM_WRITE_BACK)));
                submenu->AddItem(new BMenuItem("Select nvram file", new BMessage(MENU_MC_NVRAM_FILE)));
            menu->AddItem(submenu = new BMenu("Vic Flash Plugin Options"));
                submenu->AddItem(new BMenuItem("Write back to cart image", new BMessage(MENU_TOGGLE_VFP_WRITE_BACK)));
            menu->AddItem(submenu = new BMenu("UltiMem Options"));
                submenu->AddItem(new BMenuItem("Write back to cart image", new BMessage(MENU_TOGGLE_UM_WRITE_BACK)));
            menu->AddItem(submenu = new BMenu("I/O RAM Options"));
                submenu->AddItem(new BMenuItem("Enable I/O-2 RAM", new BMessage(MENU_TOGGLE_IO2_RAM)));
                submenu->AddItem(new BMenuItem("Enable I/O-3 RAM", new BMessage(MENU_TOGGLE_IO3_RAM)));
    }

    if (machine_class == VICE_MACHINE_VIC20 || machine_class == VICE_MACHINE_C128) {
        uppermenu->AddItem(new BMenuItem("IEEE488 Interface", new BMessage(MENU_TOGGLE_IEEE488)));
    }

    if (machine_class == VICE_MACHINE_VIC20) {
        uppermenu->AddItem(new BMenuItem("VFLI modification", new BMessage(MENU_TOGGLE_VFLI)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC ||
        machine_class == VICE_MACHINE_SCPU64 || machine_class == VICE_MACHINE_C128 ||
        machine_class == VICE_MACHINE_CBM6x0 || machine_class == VICE_MACHINE_PET ||
        machine_class == VICE_MACHINE_VIC20 || machine_class == VICE_MACHINE_PLUS4) {
        uppermenu->AddItem(menu = new BMenu("Userport devices"));
            menu->AddItem(new BMenuItem("Enable userport DAC", new BMessage(MENU_TOGGLE_USERPORT_DAC)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC ||
        machine_class == VICE_MACHINE_SCPU64 || machine_class == VICE_MACHINE_C128 ||
        machine_class == VICE_MACHINE_CBM6x0 || machine_class == VICE_MACHINE_PET ||
        machine_class == VICE_MACHINE_VIC20) {
            menu->AddItem(new BMenuItem("Enable userport RTC (58321A)", new BMessage(MENU_TOGGLE_USERPORT_58321A)));
            menu->AddItem(new BMenuItem("Save userport RTC (58321A) data when changed", new BMessage(MENU_TOGGLE_USERPORT_58321A_SAVE)));
            menu->AddItem(new BMenuItem("Enable userport RTC (DS1307)", new BMessage(MENU_TOGGLE_USERPORT_DS1307)));
            menu->AddItem(new BMenuItem("Save userport RTC (DS1307) data when changed", new BMessage(MENU_TOGGLE_USERPORT_DS1307_SAVE)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC ||
        machine_class == VICE_MACHINE_SCPU64 || machine_class == VICE_MACHINE_C128 ||
        machine_class == VICE_MACHINE_CBM6x0) {
            menu->AddItem(new BMenuItem("Enable userport DigiMAX", new BMessage(MENU_TOGGLE_USERPORT_DIGIMAX)));
            menu->AddItem(new BMenuItem("Enable userport 4bit sampler", new BMessage(MENU_TOGGLE_USERPORT_4BIT_SAMPLER)));
            menu->AddItem(new BMenuItem("Enable userport 8bit stereo sampler", new BMessage(MENU_TOGGLE_USERPORT_8BSS)));
    }

    if (machine_class != VICE_MACHINE_C64DTV && machine_class != VICE_MACHINE_VSID && machine_class != VICE_MACHINE_SCPU64) {
        uppermenu->AddItem(menu = new BMenu("Tape port devices"));
            menu->AddItem(new BMenuItem("Enable datasette device", new BMessage(MENU_TOGGLE_TAPEPORT_DATASETTE)));
            menu->AddItem(new BMenuItem("Enable tape sense dongle", new BMessage(MENU_TOGGLE_TAPEPORT_TAPE_SENSE_DONGLE)));
            menu->AddItem(new BMenuItem("Enable DTL Basic dongle", new BMessage(MENU_TOGGLE_TAPEPORT_DTL_BASIC_DONGLE)));
            menu->AddItem(new BMenuItem("Enable CP Clock F83 device", new BMessage(MENU_TOGGLE_TAPEPORT_CP_CLOCK_F83)));
            menu->AddItem(new BMenuItem("Save CP Clock F83 RTC data when changed", new BMessage(MENU_TOGGLE_TAPEPORT_CP_CLOCK_F83_SAVE)));
            menu->AddItem(new BMenuItem("Enable tape log device", new BMessage(MENU_TOGGLE_TAPEPORT_TAPELOG)));
            menu->AddItem(submenu = new BMenu("Tape log destination"));
                submenu->SetRadioMode(true);
                submenu->AddItem(new BMenuItem("Log messages to emulator log file", new BMessage(MENU_TAPEPORT_TAPELOG_DEFAULT_LOGFILE)));
                submenu->AddItem(new BMenuItem("Log messages to user specified file", new BMessage(MENU_TAPEPORT_TAPELOG_USER_LOGFILE)));
            menu->AddItem(new BMenuItem("Tape log filename", new BMessage(MENU_TAPEPORT_TAPLOG_FILENAME)));
    }

    if (machine_class != VICE_MACHINE_VSID) {
        uppermenu->AddItem(menu = new BMenu("Sampler settings"));
            menu->AddItem(submenu = new BMenu("Sampler device"));
                submenu->AddItem(new BMenuItem("Media file device", new BMessage(MENU_SAMPLER_DEVICE_MEDIA_FILE)));
#ifdef USE_PORTAUDIO
                submenu->AddItem(new BMenuItem("Media file device", new BMessage(MENU_SAMPLER_DEVICE_PORTAUDIO)));
#endif
            menu->AddItem(submenu = new BMenu("Sampler gain"));
                submenu->AddItem(new BMenuItem("10%", new BMessage(MENU_SAMPLER_GAIN_10)));
                submenu->AddItem(new BMenuItem("25%", new BMessage(MENU_SAMPLER_GAIN_25)));
                submenu->AddItem(new BMenuItem("50%", new BMessage(MENU_SAMPLER_GAIN_50)));
                submenu->AddItem(new BMenuItem("75%", new BMessage(MENU_SAMPLER_GAIN_75)));
                submenu->AddItem(new BMenuItem("100%", new BMessage(MENU_SAMPLER_GAIN_100)));
                submenu->AddItem(new BMenuItem("110%", new BMessage(MENU_SAMPLER_GAIN_110)));
                submenu->AddItem(new BMenuItem("125%", new BMessage(MENU_SAMPLER_GAIN_125)));
                submenu->AddItem(new BMenuItem("150%", new BMessage(MENU_SAMPLER_GAIN_150)));
                submenu->AddItem(new BMenuItem("175%", new BMessage(MENU_SAMPLER_GAIN_175)));
                submenu->AddItem(new BMenuItem("200%", new BMessage(MENU_SAMPLER_GAIN_200)));
            menu->AddItem(new BMenuItem("Sampler media filename", new BMessage(MENU_SAMPLER_FILENAME)));
    }

    /* create the SETTINGS menu */
    uppermenu = new BMenu("Settings");
    menubar->AddItem(uppermenu);

    if (machine_class == VICE_MACHINE_VIC20) {
        uppermenu->AddItem(new BMenuItem("VIC20 ...", new BMessage(MENU_VIC20_SETTINGS)));
    }

    if (machine_class == VICE_MACHINE_PET) {
        uppermenu->AddItem(new BMenuItem("PET ...", new BMessage(MENU_PET_SETTINGS)));
    }

    if (machine_class == VICE_MACHINE_CBM6x0) {
        uppermenu->AddItem(new BMenuItem("CBM 2 ...", new BMessage(MENU_CBM2_SETTINGS)));
    }

    if (machine_class == VICE_MACHINE_CBM5x0) {
        uppermenu->AddItem(new BMenuItem("CBM 5x0 ...", new BMessage(MENU_CBM5X0_SETTINGS)));
    }

    if (machine_class != VICE_MACHINE_VSID) {
        uppermenu->AddItem(new BMenuItem("Video ...", new BMessage(MENU_VIDEO_SETTINGS)));
    }

    if (machine_class != VICE_MACHINE_C64DTV && machine_class != VICE_MACHINE_VSID && machine_class != VICE_MACHINE_SCPU64) {
        uppermenu->AddItem(new BMenuItem("Datasette ...", new BMessage(MENU_DATASETTE_SETTINGS)));
    }

    if (machine_class != VICE_MACHINE_VSID) {
        uppermenu->AddItem(new BMenuItem("Drive ...", new BMessage(MENU_DRIVE_SETTINGS)));
        uppermenu->AddItem(new BMenuItem("Device ...", new BMessage(MENU_DEVICE_SETTINGS)));
        // keymap settings menu
        uppermenu->AddItem(new BMenuItem("Keyboard ...",
                    new BMessage(MENU_KEYMAP_SETTINGS)));


        uppermenu->AddItem(menu = new BMenu("Printer"));
            menu->AddItem(new BMenuItem("Printer settings ...", new BMessage(MENU_PRINTER_SETTINGS)));
            menu->AddItem(submenu = new BMenu("Printer 4 ..."));
                submenu->AddItem(extsubmenu = new BMenu("Printer 4 emulation"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("None", new BMessage(MENU_PRINTER_4_EMULATION_NONE)));
                    extsubmenu->AddItem(new BMenuItem("Filesystem", new BMessage(MENU_PRINTER_4_EMULATION_FS)));
                submenu->AddItem(extsubmenu = new BMenu("Printer 4 driver"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("ASCII", new BMessage(MENU_PRINTER_4_DRIVER_ASCII)));
                    extsubmenu->AddItem(new BMenuItem("MPS803", new BMessage(MENU_PRINTER_4_DRIVER_MPS803)));
                    extsubmenu->AddItem(new BMenuItem("NL10", new BMessage(MENU_PRINTER_4_DRIVER_NL10)));
                    extsubmenu->AddItem(new BMenuItem("Raw", new BMessage(MENU_PRINTER_4_DRIVER_RAW)));
                submenu->AddItem(extsubmenu = new BMenu("Printer 4 output type"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("Text", new BMessage(MENU_PRINTER_4_TYPE_TEXT)));
                    extsubmenu->AddItem(new BMenuItem("Graphics", new BMessage(MENU_PRINTER_4_TYPE_GFX)));
                submenu->AddItem(extsubmenu = new BMenu("Printer 4 output device"));
                    extsubmenu->SetRadioMode(true);
                    extsubmenu->AddItem(new BMenuItem("1", new BMessage(MENU_PRINTER_4_DEV_1)));
                    extsubmenu->AddItem(new BMenuItem("2", new BMessage(MENU_PRINTER_4_DEV_2)));
                    extsubmenu->AddItem(new BMenuItem("3", new BMessage(MENU_PRINTER_4_DEV_3)));
                submenu->AddItem(new BMenuItem("Send formfeed to printer 4", new BMessage(MENU_PRINTER_4_SEND_FF)));
                if (machine_class == VICE_MACHINE_C64 ||
                    machine_class == VICE_MACHINE_C64SC ||
                    machine_class == VICE_MACHINE_C64DTV ||
                    machine_class == VICE_MACHINE_C128 ||
                    machine_class == VICE_MACHINE_PLUS4 ||
                    machine_class == VICE_MACHINE_VIC20 ||
                    machine_class == VICE_MACHINE_SCPU64) {
                    submenu->AddItem(new BMenuItem("Enable IEC emulation", new BMessage(MENU_PRINTER_4_IEC)));
                }
                
            menu->AddItem(new BMenuItem("Output device 1", new BMessage(MENU_OUTPUT_DEVICE_1)));
            menu->AddItem(new BMenuItem("Output device 2", new BMessage(MENU_OUTPUT_DEVICE_2)));
            menu->AddItem(new BMenuItem("Output device 3", new BMessage(MENU_OUTPUT_DEVICE_3)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC ||
        machine_class == VICE_MACHINE_C64DTV || machine_class == VICE_MACHINE_CBM5x0 ||
        machine_class == VICE_MACHINE_C128 || machine_class == VICE_MACHINE_SCPU64) {
        uppermenu->AddItem(new BMenuItem("VIC-II ...", new BMessage(MENU_VICII_SETTINGS)));
    }

    if (machine_class == VICE_MACHINE_VIC20) {
        uppermenu->AddItem(new BMenuItem("VIC ...", new BMessage(MENU_VIC_SETTINGS)));
    }

    if (machine_class == VICE_MACHINE_PLUS4) {
        uppermenu->AddItem(new BMenuItem("TED ...", new BMessage(MENU_TED_SETTINGS)));
    }

    if (get_devices != NULL) {
        uppermenu->AddItem(menu = new BMenu("Joyport"));
            if (joyport_ports[JOYPORT_1]) {
                devices_port_1 = get_devices(JOYPORT_1);
                tmp_text = lib_msprintf("%s device", get_name(JOYPORT_1));
                menu->AddItem(submenu = new BMenu(tmp_text));
                submenu->SetRadioMode(true);
                for (i = 0; devices_port_1[i].name; ++i) {
                    submenu->AddItem(new BMenuItem(devices_port_1[i].name, new BMessage(MENU_JOYPORT1 + devices_port_1[i].id)));
                }
                lib_free(tmp_text);
            }
            if (joyport_ports[JOYPORT_2]) {
                devices_port_2 = get_devices(JOYPORT_2);
                tmp_text = lib_msprintf("%s device", get_name(JOYPORT_2));
                menu->AddItem(submenu = new BMenu(tmp_text));
                submenu->SetRadioMode(true);
                for (i = 0; devices_port_2[i].name; ++i) {
                    submenu->AddItem(new BMenuItem(devices_port_2[i].name, new BMessage(MENU_JOYPORT2 + devices_port_2[i].id)));
                }
                lib_free(tmp_text);
            }
            if (joyport_ports[JOYPORT_3]) {
                devices_port_3 = get_devices(JOYPORT_3);
                tmp_text = lib_msprintf("%s device", get_name(JOYPORT_3));
                menu->AddItem(submenu = new BMenu(tmp_text));
                submenu->SetRadioMode(true);
                for (i = 0; devices_port_3[i].name; ++i) {
                    submenu->AddItem(new BMenuItem(devices_port_3[i].name, new BMessage(MENU_JOYPORT3 + devices_port_3[i].id)));
                }
                lib_free(tmp_text);
            }
            if (joyport_ports[JOYPORT_4]) {
                devices_port_4 = get_devices(JOYPORT_4);
                tmp_text = lib_msprintf("%s device", get_name(JOYPORT_4));
                menu->AddItem(submenu = new BMenu(tmp_text));
                submenu->SetRadioMode(true);
                for (i = 0; devices_port_4[i].name; ++i) {
                    submenu->AddItem(new BMenuItem(devices_port_4[i].name, new BMessage(MENU_JOYPORT4 + devices_port_4[i].id)));
                }
                lib_free(tmp_text);
            }
            if (joyport_ports[JOYPORT_5]) {
                devices_port_5 = get_devices(JOYPORT_5);
                tmp_text = lib_msprintf("%s device", get_name(JOYPORT_5));
                menu->AddItem(submenu = new BMenu(tmp_text));
                submenu->SetRadioMode(true);
                for (i = 0; devices_port_5[i].name; ++i) {
                    submenu->AddItem(new BMenuItem(devices_port_5[i].name, new BMessage(MENU_JOYPORT5 + devices_port_5[i].id)));
                }
                lib_free(tmp_text);
            }
            menu->AddSeparatorItem();
            menu->AddItem(new BMenuItem("Joystick/Keyset settings ...", new BMessage(MENU_JOYSTICK_SETTINGS)));

        if (machine_class != VICE_MACHINE_CBM5x0) {
            menu->AddItem(new BMenuItem("Userport joystick emulation", new BMessage(MENU_TOGGLE_USERPORT_JOY)));
            menu->AddItem(new BMenuItem("Userport joystick settings ...", new BMessage(MENU_USERPORT_JOY_SETTINGS)));
            if (machine_class != VICE_MACHINE_C64DTV) {
                menu->AddItem(submenu = new BMenu("Userport joystick type"));
                    submenu->SetRadioMode(true);
                    submenu->AddItem(new BMenuItem("CGA", new BMessage(MENU_USERPORT_JOY_CGA)));
                    submenu->AddItem(new BMenuItem("PET", new BMessage(MENU_USERPORT_JOY_PET)));
                    submenu->AddItem(new BMenuItem("Hummer", new BMessage(MENU_USERPORT_JOY_HUMMER)));
                    submenu->AddItem(new BMenuItem("OEM", new BMessage(MENU_USERPORT_JOY_OEM)));
                if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC ||
                    machine_class == VICE_MACHINE_SCPU64 || machine_class == VICE_MACHINE_C128) {
                    submenu->AddItem(new BMenuItem("HIT", new BMessage(MENU_USERPORT_JOY_HIT)));
                    submenu->AddItem(new BMenuItem("Kingsoft", new BMessage(MENU_USERPORT_JOY_KINGSOFT)));
                    submenu->AddItem(new BMenuItem("Starbyte", new BMessage(MENU_USERPORT_JOY_STARBYTE)));
                }
            }
        }
        if (machine_class == VICE_MACHINE_PLUS4) {
            menu->AddItem(new BMenuItem("SID cart joystick emulation", new BMessage(MENU_TOGGLE_SIDCART_JOY)));
            menu->AddItem(new BMenuItem("SID cart joystick settings ...", new BMessage(MENU_SIDCART_JOY_SETTINGS)));
        }
            menu->AddItem(new BMenuItem("Allow opposite joystick directions", new BMessage(MENU_ALLOW_OPPOSITE_JOY)));

            menu->AddSeparatorItem();
            menu->AddItem(new BMenuItem("Save BBRTC data when changed", new BMessage(MENU_TOGGLE_BBRTC_DATA_SAVE)));
        if (machine_class != VICE_MACHINE_CBM6x0 && machine_class != VICE_MACHINE_PET) {
                menu->AddItem(new BMenuItem("Save Smart Mouse RTC data when changed", new BMessage(MENU_TOGGLE_SMART_MOUSE_RTC_SAVE)));
        }

        if (devices_port_1) {
            lib_free(devices_port_1);
        }
        if (devices_port_2) {
            lib_free(devices_port_2);
        }
        if (devices_port_3) {
            lib_free(devices_port_3);
        }
        if (devices_port_4) {
            lib_free(devices_port_4);
        }
        if (devices_port_5) {
            lib_free(devices_port_5);
        }
    }

    uppermenu->AddItem(new BMenuItem("Sound ...", new BMessage(MENU_SOUND_SETTINGS)));

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC ||
        machine_class == VICE_MACHINE_C64DTV || machine_class == VICE_MACHINE_C128 ||
        machine_class == VICE_MACHINE_CBM5x0 || machine_class == VICE_MACHINE_CBM6x0 ||
        machine_class == VICE_MACHINE_VSID || machine_class == VICE_MACHINE_SCPU64) {
        uppermenu->AddItem(new BMenuItem("SID ...", new BMessage(MENU_SID_SETTINGS)));
    }

    if (machine_class == VICE_MACHINE_VIC20 || machine_class == VICE_MACHINE_PLUS4 ||
        machine_class == VICE_MACHINE_PET) {
        uppermenu->AddItem(new BMenuItem("SID cartridge ...", new BMessage(MENU_SIDCART_SETTINGS)));
    }

    if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC ||
        machine_class == VICE_MACHINE_SCPU64 || machine_class == VICE_MACHINE_C128 ||
        machine_class == VICE_MACHINE_CBM5x0 || machine_class == VICE_MACHINE_CBM6x0) {
        uppermenu->AddItem(new BMenuItem("CIA ...", new BMessage(MENU_CIA_SETTINGS)));
    }

    uppermenu->AddItem(menu = new BMenu("Computer ROM ..."));
        if (machine_class != VICE_MACHINE_SCPU64 && machine_class != VICE_MACHINE_C128) {
            menu->AddItem(new BMenuItem("Select Kernal ROM file", new BMessage(MENU_COMPUTER_KERNAL_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select Basic ROM file", new BMessage(MENU_COMPUTER_BASIC_ROM_FILE)));
        } else if (machine_class == VICE_MACHINE_SCPU64) {
            menu->AddItem(new BMenuItem("Select SCPU64 ROM file", new BMessage(MENU_COMPUTER_SCPU64_ROM_FILE)));
        } else {
            menu->AddItem(new BMenuItem("Select International Kernal ROM file", new BMessage(MENU_COMPUTER_KERNAL_INT_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select German Kernal ROM file", new BMessage(MENU_COMPUTER_KERNAL_DE_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select Finnish Kernal ROM file", new BMessage(MENU_COMPUTER_KERNAL_FI_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select French Kernal ROM file", new BMessage(MENU_COMPUTER_KERNAL_FR_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select Italian Kernal ROM file", new BMessage(MENU_COMPUTER_KERNAL_IT_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select Norwegian Kernal ROM file", new BMessage(MENU_COMPUTER_KERNAL_NO_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select Swedish Kernal ROM file", new BMessage(MENU_COMPUTER_KERNAL_SE_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select Swiss Kernal ROM file", new BMessage(MENU_COMPUTER_KERNAL_CH_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select C64 Kernal ROM file", new BMessage(MENU_COMPUTER_KERNAL_64_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select Basic LO ROM file", new BMessage(MENU_COMPUTER_BASIC_LO_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select Basic HI ROM file", new BMessage(MENU_COMPUTER_BASIC_HI_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select C64 Basic ROM file", new BMessage(MENU_COMPUTER_BASIC_64_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select International Chargen ROM file", new BMessage(MENU_COMPUTER_CHARGEN_INT_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select German Chargen ROM file", new BMessage(MENU_COMPUTER_CHARGEN_DE_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select French Chargen ROM file", new BMessage(MENU_COMPUTER_CHARGEN_FR_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select Swedish Chargen ROM file", new BMessage(MENU_COMPUTER_CHARGEN_SE_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select Swiss Chargen ROM file", new BMessage(MENU_COMPUTER_CHARGEN_CH_ROM_FILE)));
        }
        if (machine_class != VICE_MACHINE_C128 && machine_class != VICE_MACHINE_PLUS4) {
            menu->AddItem(new BMenuItem("Select Chargen ROM file", new BMessage(MENU_COMPUTER_CHARGEN_ROM_FILE)));
        }
        if (machine_class == VICE_MACHINE_PET) {
            menu->AddItem(new BMenuItem("Select Editor ROM file", new BMessage(MENU_COMPUTER_EDITOR_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select ROM 9 file", new BMessage(MENU_COMPUTER_ROM_9_FILE)));
            menu->AddItem(new BMenuItem("Select ROM A file", new BMessage(MENU_COMPUTER_ROM_A_FILE)));
            menu->AddItem(new BMenuItem("Select ROM B file", new BMessage(MENU_COMPUTER_ROM_B_FILE)));
        }
        if (machine_class == VICE_MACHINE_PLUS4) {
            menu->AddItem(new BMenuItem("Select Function LO ROM file", new BMessage(MENU_COMPUTER_FUNCTION_LO_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select Function HI ROM file", new BMessage(MENU_COMPUTER_FUNCTION_HI_ROM_FILE)));
        }

    if (machine_class != VICE_MACHINE_VSID) {
        uppermenu->AddItem(menu = new BMenu("Drive ROM ..."));
        if (machine_class != VICE_MACHINE_CBM5x0 && machine_class != VICE_MACHINE_CBM6x0 && machine_class != VICE_MACHINE_PET) {
            menu->AddItem(new BMenuItem("Select 1540 ROM file", new BMessage(MENU_DRIVE_1540_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select 1541 ROM file", new BMessage(MENU_DRIVE_1541_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select 1541-II ROM file", new BMessage(MENU_DRIVE_1541II_ROM_FILE)));
            if (machine_class == VICE_MACHINE_PLUS4) {
                menu->AddItem(new BMenuItem("Select 1551 ROM file", new BMessage(MENU_DRIVE_1551_ROM_FILE)));
            }
            menu->AddItem(new BMenuItem("Select 1570 ROM file", new BMessage(MENU_DRIVE_1570_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select 1571 ROM file", new BMessage(MENU_DRIVE_1571_ROM_FILE)));
            if (machine_class == VICE_MACHINE_C128) {
                menu->AddItem(new BMenuItem("Select 1571CR ROM file", new BMessage(MENU_DRIVE_1571CR_ROM_FILE)));
            }
            menu->AddItem(new BMenuItem("Select 1581 ROM file", new BMessage(MENU_DRIVE_1581_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select FD2000 ROM file", new BMessage(MENU_DRIVE_2000_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select FD4000 ROM file", new BMessage(MENU_DRIVE_4000_ROM_FILE)));
        }
        if (machine_class != VICE_MACHINE_C64DTV && machine_class != VICE_MACHINE_PLUS4) {
            menu->AddItem(new BMenuItem("Select 2031 ROM file", new BMessage(MENU_DRIVE_2031_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select 2040 ROM file", new BMessage(MENU_DRIVE_2040_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select 3040 ROM file", new BMessage(MENU_DRIVE_3040_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select 4040 ROM file", new BMessage(MENU_DRIVE_4040_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select 1001 ROM file", new BMessage(MENU_DRIVE_1001_ROM_FILE)));
        }
        if (machine_class == VICE_MACHINE_C64 || machine_class == VICE_MACHINE_C64SC ||
            machine_class == VICE_MACHINE_SCPU64 || machine_class == VICE_MACHINE_C128) {
            menu->AddItem(new BMenuItem("Select Professional DOS ROM file", new BMessage(MENU_DRIVE_PROFDOS_ROM_FILE)));
            menu->AddItem(new BMenuItem("Select SuperCard+ ROM file", new BMessage(MENU_DRIVE_SUPERCARD_ROM_FILE)));
        }
    }

    uppermenu->AddItem(new BMenuItem("RAM ...", new BMessage(MENU_RAM_SETTINGS)));

    if (machine_class == VICE_MACHINE_C128) {
        uppermenu->AddItem(new BMenuItem("RAM banks 2 & 3", new BMessage(MENU_TOGGLE_C128FULLBANKS)));
    }

    uppermenu->AddSeparatorItem();
    uppermenu->AddItem(new BMenuItem("Load Settings", new BMessage(MENU_SETTINGS_LOAD)));
    uppermenu->AddItem(new BMenuItem("Save Settings", new BMessage(MENU_SETTINGS_SAVE)));
    uppermenu->AddItem(new BMenuItem("Default Settings", new BMessage(MENU_SETTINGS_DEFAULT)));
    uppermenu->AddSeparatorItem();
    uppermenu->AddItem(new BMenuItem("Save settings on exit", new BMessage(MENU_TOGGLE_SAVE_SETTINGS_ON_EXIT)));
    uppermenu->AddItem(new BMenuItem("Confirm on exit", new BMessage(MENU_TOGGLE_CONFIRM_ON_EXIT)));

    /* create the HELP menu */
    uppermenu = new BMenu("Help");
    menubar->AddItem(uppermenu);

    uppermenu->AddItem(new BMenuItem("About BeVICE...", new BMessage(MENU_ABOUT)));
    uppermenu->AddSeparatorItem();
    uppermenu->AddItem(new BMenuItem("Commandline Options", new BMessage(MENU_CMDLINE)));
    uppermenu->AddItem(new BMenuItem("Compile Time Features", new BMessage(MENU_COMPILE_TIME_FEATURES)));
    uppermenu->AddSeparatorItem();
    uppermenu->AddItem(new BMenuItem("Contributors", new BMessage(MENU_CONTRIBUTORS)));
    uppermenu->AddItem(new BMenuItem("License", new BMessage(MENU_LICENSE)));
    uppermenu->AddItem(new BMenuItem("No Warranty", new BMessage(MENU_WARRANTY)));

    return menubar;
}
