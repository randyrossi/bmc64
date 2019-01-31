/** \file   uisettings.c
 * \brief   GTK3 main settings dialog
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 */

/*
 * $VICERES SaveResourcesOnExit     all
 * $VICERES ConfirmOnExit           all
 */

/*
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
 */


/* The settings_grid is supposed to become this:
 *
 * +--------------+---------------------------+
 * | treeview     |                           |
 * |  with        |                           |
 * |   settings   |    central widget,        |
 * |  more        |    depending on which     |
 * |   foo        |    item is selected in    |
 * |   bar        |    the treeview           |
 * |    whatever  |                           |
 * | burp         |                           |
 * +--------------+---------------------------+
 *
 * And this is handled by the dialog itself:
 * +------------------------------------------+
 * | load | save | load... | save... | close  |
 * +------------------------------------------+
 */


#include "vice.h"

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "vsync.h"

#include "widgets/base/vice_gtk3.h"

#include "ui.h"
#include "settings_speed.h"
#include "settings_keyboard.h"
#include "settings_sound.h"
#include "settings_autostart.h"
#include "settings_drive.h"
#include "settings_model.h"
#include "settings_misc.h"
#include "settings_ramreset.h"
#include "settings_video.h"
#include "settings_sampler.h"
#include "settings_printer.h"
#include "settings_controlport.h"
#include "settings_joystick.h"
#include "settings_mouse.h"
#include "settings_soundchip.h"
#include "settings_monitor.h"
#include "settings_romset.h"
#include "settings_snapshot.h"
#ifdef HAVE_RAWNET
# include "settings_ethernet.h"
#endif
#include "settings_rs232.h"
#include "scpu64settingswidget.h"
#include "c64dtvflashsettingswidget.h"

/* I/O extension widgets */
#include "settings_io.h"
#include "settings_io_c64_memhacks.h"
#include "settings_io_georam.h"
#include "reuwidget.h"
#include "ramcartwidget.h"
#include "dqbbwidget.h"
#include "expertwidget.h"
#include "isepicwidget.h"
#include "easyflashwidget.h"
#include "gmod2widget.h"
#include "mmcrwidget.h"
#include "mmc64widget.h"
#include "ide64widget.h"
#include "retroreplaywidget.h"

#ifdef HAVE_RAWNET
# include "ethernetcartwidget.h"
# include "rrnetmk3widget.h"
#endif

#include "c128functionromwidget.h"
#include "ieee488widget.h"
#include "digimaxwidget.h"
#include "magicvoicewidget.h"
#include "midiwidget.h"
#include "sfxsoundexpanderwidget.h"
#include "ds12c887widget.h"
#include "userportdeviceswidget.h"
#include "tapeportdeviceswidget.h"
#include "sidcartwidget.h"
#include "v364speechwidget.h"
#include "sfxsoundsamplerwidget.h"
#include "megacartwidget.h"
#include "petreuwidget.h"
#include "petcolourgraphicswidget.h"
#include "petdwwwidget.h"
#include "supersnapshotwidget.h"
#include "cpmwidget.h"
#include "burstmodewidget.h"
#include "c128fullbankswidget.h"
#include "plus4aciawidget.h"
#include "plus4digiblasterwidget.h"
#include "finalexpansionwidget.h"
#include "vicflashwidget.h"
#include "ultimemwidget.h"
#include "vicieee488widget.h"
#include "vicioramwidget.h"
#include "vfliwidget.h"
#include "petdiagpinwidget.h"
#include "pethrewidget.h"

#include "uisettings.h"


/** \brief  Number of columns in the tree model
 */
#define NUM_COLUMNS 3


/** \brief  Column indici for the tree model
 */
enum {
    COLUMN_NAME = 0,    /**< name */
    COLUMN_ID,          /**< id */
    COLUMN_CALLBACK     /**< callback function */
};


/** \brief  Initial dialog width
 *
 * This is not how wide the dialog will actually become, that is determined by
 * the Gtk theme applied. But it's a rough estimate.
 */
#define DIALOG_WIDTH 800


/** \brief  Initial dialog height
 *
 * This is not how tall the dialog will actually become, that is determined by
 * the Gtk theme applied. But it's a rough estimate.
 */
#define DIALOG_HEIGHT 560


/** \brief  Maximum width the UI can be
 *
 * This again is not a really a fixed value, but more of an indicator when the
 * UI might get too large after any decorations are applied. The idea is to
 * have a UI that works on a 1280x768 resolution without requiring scrollbars.
 */
#define DIALOG_WIDTH_MAX 1024


/** \brief  Maximum height the UI can be
 *
 * This again is not a really a fixed value, but more of an indicator when the
 * UI might get too large after any decorations are applied. The idea is to
 * have a UI that works on a 1280x768 resolution without requiring scrollbars.
 */
#define DIALOG_HEIGHT_MAX 640


/** \brief  Enum used for the "response" callback of the settings dialog
 *
 * All values must be positive since Gtk reserves standard responses in its
 * GtkResponse enum as negative values.
 */
enum {
    RESPONSE_LOAD = 1,  /**< "Load" -> load settings from default file */
    RESPONSE_SAVE,      /**< "Save" -> save settings from default file */
    RESPONSE_LOAD_FILE, /**< "Load ..." -> load settings via dialog */
    RESPONSE_SAVE_FILE, /**< "Save ..." -> save settings via dialog */
    RESPONSE_DEFAULT    /**< Restore default settings */
};


/** \brief  List of C64 I/O extensions (x64, x64sc)
 *
 * Every empty line indicates a separator in the Gtk2 UI's menu
 */
static ui_settings_tree_node_t c64_io_extensions[] = {
    { "Memory Expansion Hacks",
       "mem-hacks",
       settings_io_c64_memhacks_widget_create, NULL },

    { "GEO-RAM",
        "geo-ram",
        settings_io_georam_widget_create, NULL },
    { "RAM Expansion Module",
        "reu",
        reu_widget_create, NULL },
    { "RamCart",
        "ramcart",
        ramcart_widget_create, NULL },

    { "Double Quick Brown Box",
        "dqbb",
        dqbb_widget_create, NULL },
    { "Expert Cartridge",
        "expert-cart",
        expert_widget_create, NULL },
    { "ISEPIC",
        "isepic",
        isepic_widget_create, NULL },

    { "EasyFlash",
        "easyflash",
        easyflash_widget_create, NULL },
    { "GMod2",
        "gmod2",
        gmod2_widget_create, NULL },
    { "IDE64",
        "ide64",
        ide64_widget_create, NULL },
    { "MMC64",
        "mmc64",
        mmc64_widget_create, NULL },
    { "MMC Replay",
        "mmcr",
        mmcr_widget_create, NULL },
    { "Retro Replay",
        "retroreplay",
        retroreplay_widget_create, NULL },
    { "Super Snapshot V5",
        "super-snapshot",
        super_snapshot_widget_create, NULL },

#ifdef HAVE_RAWNET
    { "Ethernet Cartridge",
        "ethernet-cart",
        ethernet_cart_widget_create, NULL },
    { "RR-Net Mk3",
        "rrnetmk3",
        rrnetmk3_widget_create, NULL },
#endif

    { "IEEE-448 Interface",
        "ieee-488",
        ieee488_widget_create, NULL },
    { "Burst Mode Modification",
        "burstmode-mode",
        burst_mode_widget_create, NULL },

    { "DigiMAX",
        "digimax",
        digimax_widget_create, NULL },
    { "Magic Voice",
        "magic-voice",
        magic_voice_widget_create, NULL },
    { "MIDI emulation",
        "midi",
        midi_widget_create, NULL },
    { "SFX Sound Expander",
        "sfx-expander",
        sfx_sound_expander_widget_create, NULL },
    { "SFX Sound Sampler",
        "sfx-sampler",
        sfx_sound_sampler_widget_create, NULL },
    { "CP/M Cartridge",
        "cpm-cart",
        cpm_widget_create, NULL },

    { "DS12C887 Real Time Clock",
        "ds12c887-rtc",
        ds12c887_widget_create, NULL },
    { "Userport devices",
        "userport-devices",
        userport_devices_widget_create, NULL },
    { "Tape port devices",
        "tapeport-devices",
        tapeport_devices_widget_create, NULL },

    UI_SETTINGS_TERMINATOR
};


/** \brief  List of SuperCPU64 extensions (xscpu64)
 *
 * Every empty line indicates a separator in the Gtk2 UI's menu
 */
static ui_settings_tree_node_t scpu64_io_extensions[] = {
    { "GEO-RAM",
        "geo-ram",
        settings_io_georam_widget_create, NULL },
    { "RAM Expansion Module",
        "reu",
        reu_widget_create, NULL },
    { "RamCart",
        "ramcart",
        ramcart_widget_create, NULL },

    { "Double Quick Brown Box",
        "dqbb",
        dqbb_widget_create, NULL },
    { "Expert Cartridge",
        "expert-cart",
        expert_widget_create, NULL },
    { "ISEPIC",
        "isepic",
        isepic_widget_create, NULL },

    { "EasyFlash",
        "easyflash",
        easyflash_widget_create, NULL },
    { "GMod2",
        "gmod2",
        gmod2_widget_create, NULL },
    { "IDE64",
        "ide64",
        ide64_widget_create, NULL },
    { "MMC64",
        "mmc64",
        mmc64_widget_create, NULL },
    { "MMC Replay",
        "mmcr",
        mmcr_widget_create, NULL },
    { "Retro Replay",
        "retroreplay",
        retroreplay_widget_create, NULL },
    { "Super Snapshot V5",
        "super-snapshot",
        super_snapshot_widget_create, NULL },

#ifdef HAVE_RAWNET
    { "Ethernet Cartridge",
        "ethernet-cart",
        ethernet_cart_widget_create, NULL },
    { "RR-Net Mk3",
        "rrnetmk3",
        rrnetmk3_widget_create, NULL },
#endif

    { "IEEE-448 Interface",
        "ieee-488",
        ieee488_widget_create, NULL },
    { "Burst Mode Modification",
        "burstmode-mode",
        burst_mode_widget_create, NULL },

    { "DigiMAX",
        "digimax",
        digimax_widget_create, NULL },
    { "Magic Voice",
        "magic-voice",
        magic_voice_widget_create, NULL },
    { "MIDI emulation",
        "midi",
        midi_widget_create, NULL },
    { "SFX Sound Expander",
        "sfx-expander",
        sfx_sound_expander_widget_create, NULL },
    { "SFX Sound Sampler",
        "sfx-sampler",
        sfx_sound_sampler_widget_create, NULL },
    { "CP/M Cartridge",
        "cpm-cart",
        cpm_widget_create, NULL },

    { "DS12C887 Real Time Clock",
        "ds12c887-rtc",
        ds12c887_widget_create, NULL },
    { "Userport devices",
        "userport-devices",
        userport_devices_widget_create, NULL },

    UI_SETTINGS_TERMINATOR
};


/** \brief  I/O extensions for C128
 */
static ui_settings_tree_node_t c128_io_extensions[] = {
    { "Function ROM",
        "function-rom",
        c128_function_rom_widget_create, NULL },
    { "Banks 2 & 3",
        "banks-23",
        c128_full_banks_widget_create, NULL },

    { "GEO-RAM",
        "geo-ram",
        settings_io_georam_widget_create, NULL },
    { "RAM Expansion Module",
        "reu",
        reu_widget_create, NULL },
    { "RamCart",
        "ramcart",
        ramcart_widget_create, NULL },

    { "Double Quick Brown Box",
        "dqbb",
        dqbb_widget_create, NULL },
    { "Expert Cartridge",
        "expert-cart",
        expert_widget_create, NULL },
    { "ISEPIC",
        "isepic",
        isepic_widget_create, NULL },

    { "EasyFlash",
        "easyflash",
        easyflash_widget_create, NULL },
    { "GMod2",
        "gmod2",
        gmod2_widget_create, NULL },
    { "IDE64",
        "ide64",
        ide64_widget_create, NULL },
    { "MMC64",
        "mmc64",
        mmc64_widget_create, NULL },
    { "MMC Replay",
        "mmcr",
        mmcr_widget_create, NULL },
    { "Retro Replay",
        "retroreplay",
        retroreplay_widget_create, NULL },
    { "Super Snapshot V5",
        "super-snapshot",
        super_snapshot_widget_create, NULL },

#ifdef HAVE_RAWNET
    { "Ethernet Cartridge",
        "ethernet-cart",
        ethernet_cart_widget_create, NULL },
    { "RR-Net Mk3",
        "rrnetmk3",
        rrnetmk3_widget_create, NULL },
#endif

    { "IEEE-448 Interface",
        "ieee-488",
        ieee488_widget_create, NULL },
    { "Burst Mode Modification",
        "burstmode-mode",
        burst_mode_widget_create, NULL },

    { "DigiMAX",
        "digimax",
        digimax_widget_create, NULL },
    { "Magic Voice",
        "magic-voice",
        magic_voice_widget_create, NULL },
    { "MIDI emulation",
        "midi",
        midi_widget_create, NULL },
    { "SFX Sound Expander",
        "sfx-expander",
        sfx_sound_expander_widget_create, NULL },
    { "SFX Sound Sampler",
        "sfx-sampler",
        sfx_sound_sampler_widget_create, NULL },
    { "CP/M Cartridge",
        "cpm-cart",
        cpm_widget_create, NULL },

    { "DS12C887 Real Time Clock",
        "ds12c887-rtc",
        ds12c887_widget_create, NULL },
    { "Userport devices",
        "userport-devices",
        userport_devices_widget_create, NULL },
    { "Tape port devices",
        "tapeport-devices",
        tapeport_devices_widget_create, NULL },

    UI_SETTINGS_TERMINATOR
};


/** \brief  List of VIC-20 I/O extensions
 *
 * Every empty line indicates a separator in the Gtk2 UI's menu
 */
static ui_settings_tree_node_t vic20_io_extensions[] = {
    { "Mega Cart",
        "mega-cart",
        mega_cart_widget_create, NULL },
    { "Final Expansion",
        "final-expansion",
        final_expansion_widget_create, NULL },
    { "Vic Flash Plugin",
        "vic-flash-plugin",
        vic_flash_widget_create, NULL },
    { "UltiMem",
        "ultimem",
        ultimem_widget_create, NULL },
    { "SID Cartridge",
        "sid-cart",
        sidcart_widget_create, NULL },
    { "VIC-1112 IEEE-488 interface",
        "ieee-488",
        vic_ieee488_widget_create, NULL },
    { "I/O RAM",
        "io-ram",
        vic_ioram_widget_create, NULL },
    { "VFLI modification",
        "vfli",
        vfli_widget_create, NULL },

    { "DigiMAX (MasC=uerade",
        "digimax",
        digimax_widget_create, NULL },
    { "DS12C887 RTC (MasC=uerade)",
        "ds12c887-rtc",
        ds12c887_widget_create, NULL },
    { "GEO-RAM (MasC=uerade)",
       "geo-ram",
       settings_io_georam_widget_create, NULL },
    { "SFX Sound Expander (MasC=uerade)",
        "sfx-expander",
        sfx_sound_expander_widget_create, NULL },
    { "SFX Sound Sampler (MasC=uerade)",
        "sfx-sampler",
        sfx_sound_sampler_widget_create, NULL },

#ifdef HAVE_RAWNET
    { "Ethernet Cartridge (MasC=uerade)",
        "ethernet-cart",
        ethernet_cart_widget_create, NULL },
#endif

    { "MIDI emulation",
        "midi",
        midi_widget_create, NULL },
    { "Userport devices",
        "userport-devices",
        userport_devices_widget_create, NULL },
    { "Tapeport devices",
        "tapeport-devices",
        tapeport_devices_widget_create, NULL },

    UI_SETTINGS_TERMINATOR
};


/** \brief  List of Plus4 I/O extensions
 *
 * Every empty line indicates a separator in the Gtk2 UI's menu
 */
static ui_settings_tree_node_t plus4_io_extensions[] = {
    { "ACIA",
        "acia",
        plus4_acia_widget_create, NULL },
    { "Digiblaster add-on",
        "digiblaster",
        plus4_digiblaster_widget_create, NULL },
    { "SID Cartridge",
        "sid-cart",
        sidcart_widget_create, NULL },
    { "V364 Speech",
        "v364",
        v364_speech_widget_create, NULL },

    { "Userport devices",
        "userport-devices",
        userport_devices_widget_create, NULL },
    { "Tape port devices",
        "tapeport-devices",
        tapeport_devices_widget_create, NULL },

    UI_SETTINGS_TERMINATOR
};


/** \brief  List of PET I/O extensions
 *
 * Every empty line indicates a separator in the Gtk2 UI's menu
 */
static ui_settings_tree_node_t pet_io_extensions[] = {
    { "PET RAM Expansion Unit",
        "pet-reu",
        pet_reu_widget_create, NULL },
    { "PET Colour graphics",
        "pet-colour",
        pet_colour_graphics_widget_create, NULL },
    { "PET DWW hi-res graphics",
        "pet-dww",
        pet_dww_widget_create, NULL },
    { "PET HRE hi-res graphics",
        "pet-hre",
        pet_hre_widget_create, NULL },
    { "SID Cartridge",
        "sid-cart",
        sidcart_widget_create, NULL },
    { "Userport devices",
        "userport-devices",
        userport_devices_widget_create, NULL },
    { "Tape port devices",
        "tapeport-devices",
        tapeport_devices_widget_create, NULL },
    { "PET userport diagnostic pin",
        "pet-diagpin",
        pet_diagpin_widget_create, NULL },
    UI_SETTINGS_TERMINATOR
};


/** \brief  List of CBM 5x0 I/O extensions
 *
 * Every empty line indicates a separator in the Gtk2 UI's menu
 */
static ui_settings_tree_node_t cbm5x0_io_extensions[] = {
    { "Tape port devices",
        "tapeport-devices",
        tapeport_devices_widget_create, NULL },
    UI_SETTINGS_TERMINATOR
};


/** \brief  List of CBM 6x0 I/O extensions
 *
 * Every empty line indicates a separator in the Gtk2 UI's menu
 */
static ui_settings_tree_node_t cbm6x0_io_extensions[] = {
    { "Userport devices",
        "userport-devices",
        userport_devices_widget_create, NULL },
    { "Tape port devices",
        "tapeport_devices",
        tapeport_devices_widget_create, NULL },
    UI_SETTINGS_TERMINATOR
};


#if 0
/** \brief  No I/O extensions (temporary)
 */
static ui_settings_tree_node_t no_io_extensions[] = {
    { "NOT IMPLEMENTED",
       "not-implented",
       NULL, NULL },

    UI_SETTINGS_TERMINATOR
};
#endif


/** \brief  Main tree nodes for VSID
 */
static ui_settings_tree_node_t main_nodes_vsid[] = {
   { "Sound settings",
      "sound",
      settings_sound_create, NULL },
    { "SID settings",
      "sid",
      settings_soundchip_widget_create, NULL },
    { "Speed settings",
      "speed",
       settings_speed_widget_create, NULL },
    /* XXX: basically a selection between 'PAL'/'NTSC' (50/60Hz) */
    { "Model settings",
      "model",
      settings_model_widget_create, NULL },
    /* XXX: do we need this? Sidplay allows ROM selection for Basic, Kernal and
     *      Chargen, perhaps move into model settings, plenty of space there */
    { "ROM settings",
      "rom-settings",
      settings_romset_widget_create, NULL },
    /* XXX: perhaps required for VSID-specific things */
    { "Miscellaneous",
      "misc",
      settings_misc_widget_create, NULL },
    { "Monitor settings",
      "monitor",
      settings_monitor_widget_create, NULL },

    { "HVSC settings",
      "hvsc",
      NULL, NULL },

    UI_SETTINGS_TERMINATOR
};


/** \brief  Main tree nodes for x64/x64sc
 */
static ui_settings_tree_node_t main_nodes_c64[] = {
    { "Speed settings",
      "speed",
       settings_speed_widget_create, NULL },
    { "Keyboard settings",
      "keyboard",
      settings_keyboard_widget_create, NULL },
    { "Sound settings",
      "sound",
      settings_sound_create, NULL },
    { "Sampler settings",
      "sampler",
      settings_sampler_widget_create, NULL },
    { "Autostart settings",
      "autostart",
      settings_autostart_widget_create, NULL },
    { "Drive settings",
      "drive",
      settings_drive_widget_create, NULL },
    { "Printer settings",
      "printer",
      settings_printer_widget_create, NULL },
    { "Control port settings",
      "control-port",
      settings_controlport_widget_create, NULL },
    { "Joystick settings",
      "joystick",
      settings_joystick_widget_create, NULL },
    { "Mouse settings",
      "mouse",
      settings_mouse_widget_create, NULL },
    { "Model settings",
      "model",
      settings_model_widget_create, NULL },
    { "RAM reset pattern",
      "ram-reset",
      settings_ramreset_widget_create, NULL },
    { "ROM settings",
      "rom-settings",
      settings_romset_widget_create, NULL },
    { "Miscellaneous",
      "misc",
      settings_misc_widget_create, NULL },
    { "VIC-II settings",
      "vicii",
      settings_video_create, NULL },
    { "SID settings",
      "sid",
      settings_soundchip_widget_create, NULL },

    { "I/O extensions",
      "io-extensions",
      settings_io_widget_create, c64_io_extensions },
#ifdef HAVE_RS232DEV
    { "RS232 settings",
      "rs232",
      settings_rs232_widget_create, NULL },
#endif
#ifdef HAVE_RAWNET
    { "Ethernet settings",
      "ethernet",
      settings_ethernet_widget_create, NULL },
#endif
    { "Snaphot/event/media recording",
      "snapshot",
      settings_snapshot_widget_create, NULL },
    { "Monitor settings",
      "monitor",
      settings_monitor_widget_create, NULL },

    UI_SETTINGS_TERMINATOR
};


/** \brief  Main tree nodes for x64dtv
 */
static ui_settings_tree_node_t main_nodes_c64dtv[] = {
    { "Speed settings",
      "speed",
       settings_speed_widget_create, NULL },
    { "Keyboard settings",
      "keyboard",
      settings_keyboard_widget_create, NULL },
    { "Sound settings",
      "sound",
      settings_sound_create, NULL },
    { "Sampler settings",
      "sampler",
      settings_sampler_widget_create, NULL },
    { "Autostart settings",
      "autostart",
      settings_autostart_widget_create, NULL },
    { "Drive settings",
      "drive",
      settings_drive_widget_create, NULL },
    { "Printer settings",
      "printer",
      settings_printer_widget_create, NULL },
    { "Control port settings",
      "control-port",
      settings_controlport_widget_create, NULL },
    { "Joystick settings",
      "joystick",
      settings_joystick_widget_create, NULL },
    { "Mouse settings",
      "mouse",
      settings_mouse_widget_create, NULL },
    { "Model settings",
      "model",
      settings_model_widget_create, NULL },
    { "RAM reset pattern",
      "ram-reset",
      settings_ramreset_widget_create, NULL },
    { "ROM settings",
      "rom-settings",
      settings_romset_widget_create, NULL },
    { "Miscellaneous",
      "misc",
      settings_misc_widget_create, NULL },
    { "VIC-II settings",
      "vicii",
      settings_video_create, NULL },
    { "SID settings",
      "sid",
      settings_soundchip_widget_create, NULL },
    { "Flash settings",
      "flash",
      c64dtv_flash_settings_widget_create, NULL },
    { "I/O extensions",
      "io-extensions",
      settings_io_widget_create, NULL },

    { "Snaphot/event/media recording",
      "snapshot",
      settings_snapshot_widget_create, NULL },
    { "Monitor settings",
      "monitor",
      settings_monitor_widget_create, NULL },

    UI_SETTINGS_TERMINATOR
};


/** \brief  Main tree nodes for x128
 */
static ui_settings_tree_node_t main_nodes_c128[] = {
    { "Speed settings",
      "speed",
       settings_speed_widget_create, NULL },
    { "Keyboard settings",
      "keyboard",
      settings_keyboard_widget_create, NULL },
    { "Sound settings",
      "sound",
      settings_sound_create, NULL },
    { "Sampler settings",
      "sampler",
      settings_sampler_widget_create, NULL },
    { "Autostart settings",
      "autostart",
      settings_autostart_widget_create, NULL },
    { "Drive settings",
      "drive",
      settings_drive_widget_create, NULL },
    { "Printer settings",
      "printer",
      settings_printer_widget_create, NULL },
    { "Control port settings",
      "control-port",
      settings_controlport_widget_create, NULL },
    { "Joystick settings",
      "joystick",
      settings_joystick_widget_create, NULL },
    { "Mouse settings",
      "mouse",
      settings_mouse_widget_create, NULL },
    { "Model settings",
      "model",
      settings_model_widget_create, NULL },
    { "RAM reset pattern",
      "ram-reset",
      settings_ramreset_widget_create, NULL },
    { "ROM settings",
      "rom-settings",
      settings_romset_widget_create, NULL },
    { "Miscellaneous",
      "misc",
      settings_misc_widget_create, NULL },
    { "VIC-II/VDC settings",
      "vicii-vdc",
      settings_video_create, NULL },
    { "SID settings",
      "sid",
      settings_soundchip_widget_create, NULL },

    { "I/O extensions",
      "io-extensions",
      settings_io_widget_create, c128_io_extensions },
#ifdef HAVE_RS232DEV
    { "RS232 settings",
      "rs232",
      settings_rs232_widget_create, NULL },
#endif
#ifdef HAVE_RAWNET
    { "Ethernet settings",
      "ethernet",
      settings_ethernet_widget_create, NULL },
#endif

    { "Snaphot/event/media recording",
      "snapshot",
      settings_snapshot_widget_create, NULL },
    { "Monitor settings",
      "monitor",
      settings_monitor_widget_create, NULL },

    UI_SETTINGS_TERMINATOR
};


/** \brief  Main tree nodes for xscpu64
 */
static ui_settings_tree_node_t main_nodes_scpu64[] = {
    { "Speed settings",
      "speed",
       settings_speed_widget_create, NULL },
    { "Keyboard settings",
      "keyboard",
      settings_keyboard_widget_create, NULL },
    { "Sound settings",
      "sound",
      settings_sound_create, NULL },
    { "Sampler settings",
      "sampler",
      settings_sampler_widget_create, NULL },
    { "Autostart settings",
      "autostart",
      settings_autostart_widget_create, NULL },
    { "Drive settings",
      "drive",
      settings_drive_widget_create, NULL },
    { "Printer settings",
      "printer",
      settings_printer_widget_create, NULL },
    { "Control port settings",
      "control-port",
      settings_controlport_widget_create, NULL },
    { "Joystick settings",
      "joystick",
      settings_joystick_widget_create, NULL },
    { "Mouse settings",
      "mouse",
      settings_mouse_widget_create, NULL },
    { "Model settings",
      "model",
      settings_model_widget_create, NULL },
    { "SCPU64 settings",
      "scpu64",
      scpu64_settings_widget_create, NULL },
    { "RAM reset pattern",
      "ram-reset",
      settings_ramreset_widget_create, NULL },
    { "ROM settings",
      "rom-settings",
      settings_romset_widget_create, NULL },
    { "Miscellaneous",
      "misc",
      settings_misc_widget_create, NULL },
    { "VIC-II settings",
      "vicii",
      settings_video_create, NULL },
    { "SID settings",
      "sid",
      settings_soundchip_widget_create, NULL },

    { "I/O extensions",
      "io-extensions",
      settings_io_widget_create, scpu64_io_extensions },
#ifdef HAVE_RS232DEV
    { "RS232 settings",
      "rs232",
      settings_rs232_widget_create, NULL },
#endif
#ifdef HAVE_RAWNET
    { "Ethernet settings",
      "ethernet",
      settings_ethernet_widget_create, NULL },
#endif
    { "Snaphot/event/media recording",
      "snapshot",
      settings_snapshot_widget_create, NULL },
    { "Monitor settings",
      "monitor",
      settings_monitor_widget_create, NULL },

    UI_SETTINGS_TERMINATOR
};


/** \brief  Main tree nodes for xvic
 */
static ui_settings_tree_node_t main_nodes_vic20[] = {
    { "Speed settings",
      "speed",
       settings_speed_widget_create, NULL },
    { "Keyboard settings",
      "keyboard",
      settings_keyboard_widget_create, NULL },
    { "Sound settings",
      "sound",
      settings_sound_create, NULL },
    { "Sampler settings",
      "sampler",
      settings_sampler_widget_create, NULL },
    { "Autostart settings",
      "autostart",
      settings_autostart_widget_create, NULL },
    { "Drive settings",
      "drive",
      settings_drive_widget_create, NULL },
    { "Printer settings",
      "printer",
      settings_printer_widget_create, NULL },
    { "Control port settings",
      "control-port",
      settings_controlport_widget_create, NULL },
    { "Joystick settings",
      "joystick",
      settings_joystick_widget_create, NULL },
    { "Mouse settings",
      "mouse",
      settings_mouse_widget_create, NULL },
    { "Model settings",
      "model",
      settings_model_widget_create, NULL },
    { "RAM reset pattern",
      "ram-reset",
      settings_ramreset_widget_create, NULL },
    { "ROM settings",
      "rom-settings",
      settings_romset_widget_create, NULL },
    { "Miscellaneous",
      "misc",
      settings_misc_widget_create, NULL },
    { "VIC settings",
      "vic",
      settings_video_create, NULL },
    { "SID settings",
      "sid",
      settings_soundchip_widget_create, NULL },

    { "I/O extensions",
      "io-extensions",
      settings_io_widget_create, vic20_io_extensions },

#ifdef HAVE_RS232DEV
    { "RS232 settings",
      "rs232",
      settings_rs232_widget_create, NULL },
#endif
#ifdef HAVE_RAWNET
    { "Ethernet settings",
      "ethernet",
      settings_ethernet_widget_create, NULL },
#endif

    { "Snaphot/event/media recording",
      "snapshot",
      settings_snapshot_widget_create, NULL },
    { "Monitor settings",
      "monitor",
      settings_monitor_widget_create, NULL },

    UI_SETTINGS_TERMINATOR
};


/** \brief  Main tree nodes for xplus4
 */
static ui_settings_tree_node_t main_nodes_plus4[] = {
    { "Speed settings",
      "speed",
       settings_speed_widget_create, NULL },
    { "Keyboard settings",
      "keyboard",
      settings_keyboard_widget_create, NULL },
    { "Sound settings",
      "sound",
      settings_sound_create, NULL },
    { "Sampler settings",
      "sampler",
      settings_sampler_widget_create, NULL },
    { "Autostart settings",
      "autostart",
      settings_autostart_widget_create, NULL },
    { "Drive settings",
      "drive",
      settings_drive_widget_create, NULL },
    { "Printer settings",
      "printer",
      settings_printer_widget_create, NULL },
    { "Control port settings",
      "control-port",
      settings_controlport_widget_create, NULL },
    { "Joystick settings",
      "joystick",
      settings_joystick_widget_create, NULL },
    { "Mouse settings",
      "mouse",
      settings_mouse_widget_create, NULL },
    { "Model settings",
      "model",
      settings_model_widget_create, NULL },
    { "RAM reset pattern",
      "ram-reset",
      settings_ramreset_widget_create, NULL },
    { "ROM settings",
      "rom-settings",
      settings_romset_widget_create, NULL },
    { "Miscellaneous",
      "misc",
      settings_misc_widget_create, NULL },
    { "TED settings",
      "ted",
      settings_video_create, NULL },
    { "SID settings",
      "sid",
      settings_soundchip_widget_create, NULL },

    { "I/O extensions",
      "io-extensions",
      settings_io_widget_create, plus4_io_extensions },

#ifdef HAVE_RS232DEV
    { "RS232 settings",
      "rs232",
      settings_rs232_widget_create, NULL },
#endif
#ifdef HAVE_RAWNET
    { "Ethernet settings",
      "ethernet",
      settings_ethernet_widget_create, NULL },
#endif

    { "Snaphot/event/media recording",
      "snapshot",
      settings_snapshot_widget_create, NULL },
    { "Monitor settings",
      "monitor",
      settings_monitor_widget_create, NULL },

    UI_SETTINGS_TERMINATOR
};


/** \brief  Main tree nodes for xpet
 */
static ui_settings_tree_node_t main_nodes_pet[] = {
    { "Speed settings",
      "speed",
       settings_speed_widget_create, NULL },
    { "Keyboard settings",
      "keyboard",
      settings_keyboard_widget_create, NULL },
    { "Sound settings",
      "sound",
      settings_sound_create, NULL },
    { "Sampler settings",
      "sampler",
      settings_sampler_widget_create, NULL },
    { "Autostart settings",
      "autostart",
      settings_autostart_widget_create, NULL },
    { "Drive settings",
      "drive",
      settings_drive_widget_create, NULL },
    { "Printer settings",
      "printer",
      settings_printer_widget_create, NULL },
    { "Control port settings",
      "control-port",
      settings_controlport_widget_create, NULL },
    { "Joystick settings",
      "joystick",
      settings_joystick_widget_create, NULL },
    { "Mouse settings",
      "mouse",
      settings_mouse_widget_create, NULL },
    { "Model settings",
      "model",
      settings_model_widget_create, NULL },
    { "RAM reset pattern",
      "ram-reset",
      settings_ramreset_widget_create, NULL },
    { "ROM settings",
      "rom-settings",
      settings_romset_widget_create, NULL },
    { "Miscellaneous",
      "misc",
      settings_misc_widget_create, NULL },
    { "CRTC settings",
      "crtc",
      settings_video_create, NULL },
    { "SID settings",
      "sid",
      settings_soundchip_widget_create, NULL },

    { "I/O extensions",
      "io-extensions",
      settings_io_widget_create, pet_io_extensions },
#ifdef HAVE_RS232DEV
    { "RS232 settings",
      "rs232",
      settings_rs232_widget_create, NULL },
#endif
#ifdef HAVE_RAWNET
    { "Ethernet settings",
      "ethernet",
      settings_ethernet_widget_create, NULL },
#endif

    { "Snaphot/event/media recording",
      "snapshot",
      settings_snapshot_widget_create, NULL },
    { "Monitor settings",
      "monitor",
      settings_monitor_widget_create, NULL },

    UI_SETTINGS_TERMINATOR
};


/** \brief  Main tree nodes for xcbm5x0
 */
static ui_settings_tree_node_t main_nodes_cbm5x0[] = {
    { "Speed settings",
      "speed",
       settings_speed_widget_create, NULL },
    { "Keyboard settings",
      "keyboard",
      settings_keyboard_widget_create, NULL },
    { "Sound settings",
      "sound",
      settings_sound_create, NULL },
    { "Sampler settings",
      "sampler",
      settings_sampler_widget_create, NULL },
    { "Autostart settings",
      "autostart",
      settings_autostart_widget_create, NULL },
    { "Drive settings",
      "drive",
      settings_drive_widget_create, NULL },
    { "Printer settings",
      "printer",
      settings_printer_widget_create, NULL },
    { "Control port settings",
      "control-port",
      settings_controlport_widget_create, NULL },
    { "Joystick settings",
      "joystick",
      settings_joystick_widget_create, NULL },
    { "Mouse settings",
      "mouse",
      settings_mouse_widget_create, NULL },
    { "Model settings",
      "model",
      settings_model_widget_create, NULL },
    { "RAM reset pattern",
      "ram-reset",
      settings_ramreset_widget_create, NULL },
    { "ROM settings",
      "rom-settings",
      settings_romset_widget_create, NULL },
    { "Miscellaneous",
      "misc",
      settings_misc_widget_create, NULL },
    { "VIC-II settings",
      "vicii",
      settings_video_create, NULL },
    { "SID settings",
      "sid",
      settings_soundchip_widget_create, NULL },

    { "I/O extensions",
      "io-extensions",
      settings_io_widget_create, cbm5x0_io_extensions },

    { "Snaphot/event/media recording",
      "snapshot",
      settings_snapshot_widget_create, NULL },
    { "Monitor settings",
      "monitor",
      settings_monitor_widget_create, NULL },

    UI_SETTINGS_TERMINATOR
};


/** \brief  Main tree nodes for xcbm6x0
 */
static ui_settings_tree_node_t main_nodes_cbm6x0[] = {
    { "Speed settings",
      "speed",
       settings_speed_widget_create, NULL },
    { "Keyboard settings",
      "keyboard",
      settings_keyboard_widget_create, NULL },
    { "Sound settings",
      "sound",
      settings_sound_create, NULL },
    { "Sampler settings",
      "sampler",
      settings_sampler_widget_create, NULL },
    { "Autostart settings",
      "autostart",
      settings_autostart_widget_create, NULL },
    { "Drive settings",
      "drive",
      settings_drive_widget_create, NULL },
    { "Printer settings",
      "printer",
      settings_printer_widget_create, NULL },
    { "Control port settings",
      "control-port",
      settings_controlport_widget_create, NULL },
    { "Joystick settings",
      "joystick",
      settings_joystick_widget_create, NULL },
    { "Mouse settings",
      "mouse",
      settings_mouse_widget_create, NULL },
    { "Model settings",
      "model",
      settings_model_widget_create, NULL },
    { "RAM reset pattern",
      "ram-reset",
      settings_ramreset_widget_create, NULL },
    { "ROM settings",
      "rom-settings",
      settings_romset_widget_create, NULL },
    { "Miscellaneous",
      "misc",
      settings_misc_widget_create, NULL },
    { "CRTC settings",
      "crtc",
      settings_video_create, NULL },
    { "SID settings",
      "sid",
      settings_soundchip_widget_create, NULL },

    { "I/O extensions",
      "io-extensions",
      settings_io_widget_create, cbm6x0_io_extensions },

    { "Snaphot/event/media recording",
      "snapshot",
      settings_snapshot_widget_create, NULL },
    { "Monitor settings",
      "monitor",
      settings_monitor_widget_create, NULL },

    UI_SETTINGS_TERMINATOR
};



/** \brief  Reference to the current 'central' widget in the settings dialog
 */
static void ui_settings_set_central_widget(GtkWidget *widget);


/** \brief  Reference to the settings dialog
 *
 * Used to show/hide the widget without rebuilding it each time. Clean up
 * with ui_settings_dialog_shutdown()
 */
static GtkWidget *settings_window = NULL;


/** \brief  Reference to the 'content area' widget of the settings dialog
 */
static GtkWidget *settings_grid = NULL;


/** \brief  Reference to the tree model for the settings tree
 */
static GtkTreeStore *settings_model = NULL;


/** \brief  Reference to the tree view for the settings tree
 */
static GtkWidget *settings_tree = NULL;

#if 0
/** \brief  Paused state when popping up the UI
 */
static int old_pause_state;
#endif

/** \brief  Handler for the "changed" event of the tree view
 *
 * \param[in]   selection   GtkTreeSelection associated with the tree model
 * \param[in]   user_data   data for the event (unused for now)
 *
 */
static void on_tree_selection_changed(
        GtkTreeSelection *selection,
        gpointer user_data)
{
    GtkTreeIter iter;
    GtkTreeModel *model;

    if (gtk_tree_selection_get_selected(selection, &model, &iter))
    {
        gchar *name;
        GtkWidget *(*callback)(void *) = NULL;

        gtk_tree_model_get(model, &iter, COLUMN_NAME, &name, -1);
#if 0
        debug_gtk3("item '%s' clicked\n", name);
#endif
        gtk_tree_model_get(model, &iter, COLUMN_CALLBACK, &callback, -1);
        if (callback != NULL) {
            char *title = lib_msprintf("%s settings :: %s", machine_name, name);
            gtk_window_set_title(GTK_WINDOW(settings_window), title);
            lib_free(title);
            /* create new central widget, using settings_window (this dialog)
             * as its parent, this will allow for proper blocking in modal
             * dialogs, while ui_get_active_window() breaks that. */
            ui_settings_set_central_widget(callback(settings_window));
        }
        g_free(name);
    }
}


/** \brief  Create the 'Save on exit' checkbox
 *
 * The current position/display of the checkbox is a little lame at the moment
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_save_on_exit_checkbox(void)
{
    return vice_gtk3_resource_check_button_new("SaveResourcesOnExit",
            "Save settings on exit");
}


/** \brief  Create the 'Confirm on exit' checkbox
 *
 * The current position/display of the checkbox is a little lame at the moment
 *
 * \return  GtkCheckButton
 */
static GtkWidget *create_confirm_on_exit_checkbox(void)
{
    return vice_gtk3_resource_check_button_new("ConfirmOnExit",
            "Confirm on exit");
}


/** \brief  Create empty tree model for the settings tree
 */
static void create_tree_model(void)
{
    settings_model = gtk_tree_store_new(NUM_COLUMNS,
            G_TYPE_STRING, G_TYPE_STRING, G_TYPE_POINTER);
}


/** \brief  Get iterator into the tree model by \a path, setting \a iter
 *
 * Sets \a iter to the position in the settings tree model as requested by
 * \a path. If \a path is `NULL` or "" an iter to the very first element in
 * the tree model will be set (no idea if this useful).
 *
 * \param[in]   path    xpath-like expression ("foo/bar/huppel" for now)
 * \param[out]  iter    tree model iterator target
 *
 * \return  boolean (probably best not to touch \a iter when `false`)
 */
bool ui_settings_iter_by_xpath(const char *path, GtkTreeIter *iter)
{
    GtkTreeModel *model = GTK_TREE_MODEL(settings_model);
    gchar **elements;
    gchar **curr;

    gtk_tree_model_get_iter_first(GTK_TREE_MODEL(settings_model), iter);
    if (path == NULL || *path == '\0') {
        return true;
    }

    /* split the path into elements */
    debug_gtk3("splitting '%s' into elements ...\n", path);
    elements = g_strsplit(path, "/", 0);
    for (curr = elements; *curr != NULL; curr++) {
        debug_gtk3("element '%s'\n", *curr);
        do {
            const char *id;

            gtk_tree_model_get(model, iter, COLUMN_ID, &id, -1);
            debug_gtk3("checking ID string '%s'=='%s'\n", id, *curr);
            if (strcmp(id, *curr) == 0) {
                const char *name;
                gtk_tree_model_get(model, iter, COLUMN_NAME, &name, -1);
                debug_gtk3("got the bastard! '%s'\n", name);
                /* clean up */
                g_strfreev(elements);
                return true;
            }

        } while (gtk_tree_model_iter_next(model, iter));
    }
    /* TODO: figure out if Gtk supports setting an iter to an invalid state
     *       that avoids weird behaviour */
    g_strfreev(elements);
    return false;
}


bool ui_settings_append_by_xpath(const char *path,
                                 ui_settings_tree_node_t *nodes)
{
#if 0
    GtkTreeModel *model = GTK_TREE_MODEL(settings_model);
    GtkTreeIter iter;

    if (ui_settings_iter_by_xpath(path, &iter)) {
        /* found the proper node, add node */
        return true;
    }
#endif
    return false;
}


/** \brief  Create tree store containing settings items and children
 *
 * \return  GtkTreeStore
 */
static GtkTreeStore *populate_tree_model(void)
{
    GtkTreeStore *model;
    GtkTreeIter iter;
    GtkTreeIter child;
    ui_settings_tree_node_t *nodes = NULL;
    int i;

    model = settings_model;

    switch (machine_class) {
        case VICE_MACHINE_C64:  /* fall through */
        case VICE_MACHINE_C64SC:
            nodes = main_nodes_c64;
            break;
        case VICE_MACHINE_C64DTV:
            nodes = main_nodes_c64dtv;
            break;
        case VICE_MACHINE_C128:
            nodes = main_nodes_c128;
            break;
        case VICE_MACHINE_SCPU64:
            nodes = main_nodes_scpu64;
            break;
        case VICE_MACHINE_VIC20:
            nodes = main_nodes_vic20;
            break;
        case VICE_MACHINE_PLUS4:
            nodes = main_nodes_plus4;
            break;
        case VICE_MACHINE_PET:
            nodes = main_nodes_pet;
            break;
        case VICE_MACHINE_CBM5x0:
            nodes = main_nodes_cbm5x0;
            break;
        case VICE_MACHINE_CBM6x0:
            nodes = main_nodes_cbm6x0;
            break;
        case VICE_MACHINE_VSID:
            nodes = main_nodes_vsid;
            break;
        default:
            fprintf(stderr,
                    "Error: %s:%d:%s(): unsupported machine_class %d\n",
                    __FILE__, __LINE__, __func__, machine_class);
            exit(1);
            break;
    }

    for (i = 0; nodes[i].name != NULL; i++) {
        char *name;

        if (nodes[i].callback == NULL) {
            name = lib_msprintf("[TODO] %s", nodes[i].name);
        } else {
            name = lib_stralloc(nodes[i].name);
        }

        gtk_tree_store_append(model, &iter, NULL);
        gtk_tree_store_set(model, &iter,
                COLUMN_NAME, name,
                COLUMN_ID, nodes[i].id,
                COLUMN_CALLBACK, nodes[i].callback,
                -1);
        lib_free(name);

        /* this bit will need proper recursion if we need more than two
         * levels of subitems */
        if (nodes[i].children != NULL) {
            int c;
            ui_settings_tree_node_t *list = nodes[i].children;

            for (c = 0; list[c].name != NULL; c++) {
                char buffer[256];

                /* mark items without callback with 'TODO' */
                if (list[c].callback != NULL) {
                    g_snprintf(buffer, 256, "%s", list[c].name);
                } else {
                    g_snprintf(buffer, 256, "TODO: %s", list[c].name);
                }

                gtk_tree_store_append(model, &child, &iter);
                gtk_tree_store_set(model, &child,
                        COLUMN_NAME, buffer,
                        COLUMN_ID, list[c].id,
                        COLUMN_CALLBACK, list[c].callback,
                        -1);
            }
        }
    }
    return model;
}


/** \brief  Create treeview for settings side-menu
 *
 * Reads items from `main_nodes` and adds them to the tree view.
 *
 * \return  GtkTreeView
 *
 * TODO:    Handle nested items, and write up somewhere how the hell I finally
 *          got the callbacks working.
 *          Split into a function creating the tree view and functions adding,
 *          altering or removing nodes.
 */
static GtkWidget *create_treeview(void)
{
    GtkWidget *tree;
    GtkCellRenderer *text_renderer;
    GtkTreeViewColumn *text_column;

    create_tree_model();
    tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(populate_tree_model()));
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), FALSE);

    text_renderer = gtk_cell_renderer_text_new();
    text_column = gtk_tree_view_column_new_with_attributes(
            "item-name",
            text_renderer,
            "text", 0,
            NULL);
    /*    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), obj_column); */
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), text_column);

    return tree;
}


/** \brief  Set the 'central'/action widget for the settings dialog
 *
 * Destroys the old 'central' widget and sets the new one.
 *
 *  \param[in,out]  widget  widget to use as the new 'central' widget
 */
static void ui_settings_set_central_widget(GtkWidget *widget)
{
    GtkWidget *child;

    child = gtk_grid_get_child_at(GTK_GRID(settings_grid), 1, 0);
    if (child != NULL) {
        gtk_widget_destroy(child);
    }
    gtk_grid_attach(GTK_GRID(settings_grid), widget, 1, 0, 1, 1);
    /* add a little space around the widget */
    g_object_set(widget, "margin", 16, NULL);
}


/** \brief  Create the 'content widget' of the settings dialog
 *
 * This creates the widget in the dialog used to display the treeview and room
 * for the widget connected to that tree's currently selected item.
 *
 * \param[in]   widget  parent widget
 *
 * \return  GtkGrid (as a GtkWidget)
 */
static GtkWidget *create_content_widget(GtkWidget *widget)
{
    GtkTreeSelection *selection;
    GtkWidget *scroll;
    GtkWidget *extra;

    settings_grid = gtk_grid_new();
    settings_tree = create_treeview();

    /* pack the tree in a scrolled window to allow scrolling of the tree when
     * it gets too large for the dialog
     */
    scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), settings_tree);

    gtk_grid_attach(GTK_GRID(settings_grid), scroll, 0, 0, 1, 1);

    /* TODO: remember the previously selected setting/widget and set it here */
    ui_settings_set_central_widget(settings_speed_widget_create(widget));

    /* create container for generic settings */
    extra = gtk_grid_new();
    gtk_grid_set_column_spacing(GTK_GRID(extra), 8);
    gtk_grid_set_row_spacing(GTK_GRID(extra), 8);
    g_object_set(extra, "margin", 16, NULL);

    gtk_grid_attach(GTK_GRID(extra), create_save_on_exit_checkbox(),
            0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(extra), create_confirm_on_exit_checkbox(),
            1, 0, 1, 1);

    /* add to main layout */
    gtk_grid_attach(GTK_GRID(settings_grid), extra, 0, 2, 2, 1);

    gtk_widget_show(settings_grid);
    gtk_widget_show(settings_tree);

    gtk_widget_set_size_request(scroll, 250, 500);
    gtk_widget_set_size_request(settings_grid, DIALOG_WIDTH, DIALOG_HEIGHT);

    selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(settings_tree));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
    g_signal_connect(G_OBJECT(selection), "changed",
            G_CALLBACK(on_tree_selection_changed), NULL);


    return settings_grid;
}


/** \brief  Properly destroy the settings window if required
 */
void ui_settings_dialog_shutdown(void)
{
    if (settings_window != NULL && GTK_IS_WIDGET(settings_window)) {
        gtk_widget_destroy(settings_window);
    }
}



/** \brief  Handler for the "response" event of the settings dialog
 *
 * This determines what to do based on the 'reponse ID' emitted by the dialog.
 *
 * \param[in]   widget      widget triggering the event (button pushed)
 * \param[in]   response_id response ID
 * \param[in]   user_data   extra data (unused)
 */
static void response_callback(GtkWidget *widget, gint response_id,
                              gpointer user_data)
{
    gchar *filename;

    switch (response_id) {

        /* close dialog */
        case GTK_RESPONSE_DELETE_EVENT:
            gtk_widget_destroy(widget);
#if 0
            /* restore old pause state */
            if (!old_pause_state) {
                ui_pause_emulation(0);
            }
#endif
            break;

        /* load vicerc from default location */
        case RESPONSE_LOAD:
#if 0
            debug_gtk3("loading resources from default file\n");
#endif
            if(resources_load(NULL) != 0) {
                vice_gtk3_message_error("VICE core error",
                        "Failed to load default settings file");
            }
            break;

        /* load vicerc from a user-specified location */
        case RESPONSE_LOAD_FILE:
            filename = vice_gtk3_open_file_dialog("Load settings file",
                    NULL, NULL, NULL);
            if (filename!= NULL) {
#if 0
                debug_gtk3("loading settings from '%s'\n", filename);
#endif
                if (resources_load(filename) != 0) {
                    vice_gtk3_message_error("VICE core error",
                            "Failed to load settings from '%s'", filename);
                }
            }
            break;

        /* save settings to default location */
        case RESPONSE_SAVE:
#if 0
            debug_gtk3("saving vicerc to default location\n");
#endif
            if (resources_save(NULL) != 0) {
                vice_gtk3_message_error("VICE core error",
                        "Failed to save settings to default file");
            }
            break;

        /* save settings to a user-specified location */
        case RESPONSE_SAVE_FILE:
            filename = vice_gtk3_save_file_dialog("Save settings as ...",
                    NULL, TRUE, NULL);
            if (filename != NULL) {
#if 0
                debug_gtk3("saving setting as '%s'\n",
                        filename ? filename : "NULL");
#endif
                if (resources_save(filename) != 0) {
                    vice_gtk3_message_error("VICE core error",
                            "Failed to save setting as '%s'.", filename);
                }
                g_free(filename);
            }
            break;

        case RESPONSE_DEFAULT:
            if (vice_gtk3_message_confirm("Reset to default setting",
                        "Do you wish to reset to default settings?")) {
                resources_set_defaults();
                gtk_widget_destroy(widget);
                /* this one really behaves a little odd: */
#if 0
                machine_reset();
#endif
            }
            break;


        default:
            break;
    }
}


/** \brief  Respond to window size changes
 *
 * This allows for quickly seeing if specific dialog is getting too large. The
 * DIALOG_WIDTH_MAX and DIALOG_HEIGHT_MAX I sucked out of my thumb, since due
 * to window managers using different themes, we can't use 'proper' values, so
 * I had to use approximate values.
 *
 * \param[in]   widget  a GtkWindow
 * \param[in]   event   the GDK event
 * \param[in]   data    extra event data (unused)
 *
 * \return  boolean
 */
static gboolean on_dialog_configure_event(
        GtkWidget *widget,
        GdkEvent *event,
        gpointer data)
{
    if (event->type == GDK_CONFIGURE) {
        int width = ((GdkEventConfigure*)event)->width;
        int height = ((GdkEventConfigure*)event)->height;

        /* debug_gtk3("width %d, height %d\n", width, height); */
        if (width > DIALOG_WIDTH_MAX || height > DIALOG_HEIGHT_MAX) {
            /* uncomment the following to get some 'help' while building
             * new dialogs: */
#if 0
            gtk_window_set_title(GTK_WINDOW(widget),
                    "HELP! --- DIALOG IS TOO BLOODY LARGE -- ERROR!");
#endif
            debug_gtk3("Dialog is too large: %dx%d (max: %dx%d)\n",
                    width, height, DIALOG_WIDTH_MAX, DIALOG_HEIGHT_MAX);
        }
    }
    return FALSE;
}



/** \brief  Callback to create the main settings dialog from the menu
 *
 * \param[in]   widget      (direct) parent widget, the menu item
 * \param[in]   user_data   data for the event (unused)
 *
 * \note    The appearance of minimize/maximize buttons seems to depend on
 *          which Window Manager is active:
 *
 *          On MATE (marco, a Metacity fork) both buttons are hidden.
 *          On KDE (KWin) the maximize button is still visible but inactive
 *          On OpenBox both min/max are visible with only minimize working
 */
void ui_settings_dialog_create(GtkWidget *widget, gpointer user_data)
{
    GtkWidget *dialog;
    GtkWidget *content;
    char title[256];
#if 0
    GtkTreeIter iter;
#endif

#if 0
    /* remember pause setting */
    old_pause_state = ui_emulation_is_paused();

    /* pause emulation (required for some settings) */
    if (!old_pause_state) {
        ui_pause_emulation(1);
    }
#endif

    vsync_suspend_speed_eval();

    g_snprintf(title, 256, "%s Settings", machine_name);

    dialog = gtk_dialog_new_with_buttons(
            title,
            ui_get_active_window(),
            GTK_DIALOG_MODAL,
            "Restore defaults", RESPONSE_DEFAULT,
            "Load", RESPONSE_LOAD,
            "Save", RESPONSE_SAVE,
            "Load file ...", RESPONSE_LOAD_FILE,
            "Save file ...", RESPONSE_SAVE_FILE,
            "Close", GTK_RESPONSE_DELETE_EVENT,
            NULL);

    content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content), create_content_widget(dialog));

    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    g_signal_connect(dialog, "response", G_CALLBACK(response_callback), NULL);
    g_signal_connect(dialog, "configure-event",
            G_CALLBACK(on_dialog_configure_event), NULL);

    settings_window = dialog;
    gtk_widget_show_all(dialog);
}
