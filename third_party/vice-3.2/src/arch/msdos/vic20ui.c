/*
 * vic20ui.c - Definition of the VIC20-specific part of the UI.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cartridge.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "menudefs.h"
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "tuifs.h"
#include "types.h"
#include "ui.h"
#include "uidigimax.h"
#include "uidrive.h"
#include "uids12c887rtc.h"
#ifdef HAVE_RAWNET
#include "uiethernetcart.h"
#endif
#include "uigeoram.h"
#include "uiiocollisions.h"
#include "uisidcart.h"
#include "uisoundexpander.h"
#include "uisoundsampler.h"
#include "uitapeport.h"
#include "uivic20model.h"
#include "uivideo.h"
#include "uiuserport.h"
#include "util.h"
#include "vic20ui.h"

static uint16_t cartridge_type_to_address(int type)
{
    /* We might use a simple AND here, but it's safer to use `switch()' as
       speed does not matter in this case.  */
    switch (type) {
        case CARTRIDGE_VIC20_4KB_2000:
        case CARTRIDGE_VIC20_8KB_2000:
        case CARTRIDGE_VIC20_16KB_2000:
            return 0x2000;
        case CARTRIDGE_VIC20_4KB_4000:
        case CARTRIDGE_VIC20_8KB_4000:
        case CARTRIDGE_VIC20_16KB_4000:
	      return 0x4000;
        case CARTRIDGE_VIC20_4KB_6000:
        case CARTRIDGE_VIC20_8KB_6000:
        case CARTRIDGE_VIC20_16KB_6000:
            return 0x6000;
        case CARTRIDGE_VIC20_4KB_A000:
        case CARTRIDGE_VIC20_8KB_A000:
            return 0xa000;
        case CARTRIDGE_VIC20_4KB_B000:
            return 0xb000;
        case CARTRIDGE_VIC20_DETECT:
        default:
            return 0;               /* bogus */
    }
}

static TUI_MENU_CALLBACK(attach_cartridge_callback)
{
    const char *s;
    int type = (int)param;

    if (been_activated) {
        char *default_item, *directory;
        char *name;

        s = cartridge_get_file_name(cartridge_type_to_address(type));
        if (s == NULL) {
            directory = default_item = NULL;
        } else {
            util_fname_split(s, &directory, &default_item);
        }

        name = tui_file_selector("Attach cartridge image", directory, "*", default_item, NULL, NULL, NULL);
        if (name != NULL && (s == NULL || strcasecmp(name, s) != 0) && cartridge_attach_image(type, name) < 0) {
            tui_error("Invalid cartridge image.");
        }
        ui_update_menus();
        lib_free(name);
    }

    /* This is redundant if `been_activated' is nonzero, but let's stay on
       the safe side.  */
    s = cartridge_get_file_name(cartridge_type_to_address(type));
    if (s == NULL || *s == '\0') {
        return "(none)";
    } else {
        return s;
    }
}

static tui_menu_item_def_t attach_cartridge_menu_items[] = {
    { "Generic cartridge:",
      "Attach a generic cartridge image",
      attach_cartridge_callback, (void *)CARTRIDGE_VIC20_GENERIC, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Behr Bonz cartridge:",
      "Attach a Behr Bonz cartridge image",
      attach_cartridge_callback, (void *)CARTRIDGE_VIC20_BEHRBONZ, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Mega-Cart cartridge:",
      "Attach a Mega-Cart cartridge image",
      attach_cartridge_callback, (void *)CARTRIDGE_VIC20_MEGACART, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Final Expansion cartridge:",
      "Attach a Final Expansion cartridge image",
      attach_cartridge_callback, (void *)CARTRIDGE_VIC20_FINAL_EXPANSION, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "UltiMem cartridge:",
      "Attach a UltiMem image",
      attach_cartridge_callback, (void *)CARTRIDGE_VIC20_UM, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Vic Flash Plugin cartridge:",
      "Attach a Vic Flash Plugin image",
      attach_cartridge_callback, (void *)CARTRIDGE_VIC20_FP, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "--" },
    { "Smart attach:",
      "Smart attach a cartridge image",
      attach_cartridge_callback, (void *)CARTRIDGE_VIC20_DETECT, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Cartridge at $_2000:",
      "Attach a cartridge image at address $2000",
      attach_cartridge_callback, (void *)CARTRIDGE_VIC20_16KB_2000, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Cartridge at $_4000:",
      "Attach a cartridge image at address $4000",
      attach_cartridge_callback, (void *)CARTRIDGE_VIC20_16KB_4000, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Cartridge at $_6000:",
      "Attach a cartridge image at address $6000",
       attach_cartridge_callback, (void *)CARTRIDGE_VIC20_16KB_6000, 30,
       TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Cartridge at $_A000:",
      "Attach a cartridge image at address $A000",
      attach_cartridge_callback, (void *)CARTRIDGE_VIC20_8KB_A000, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Cartridge at $_B000:",
      "Attach a cartridge image at address $B000",
      attach_cartridge_callback, (void *)CARTRIDGE_VIC20_4KB_B000, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

static TUI_MENU_CALLBACK(detach_cartridge_callback)
{
    if (been_activated) {
        cartridge_detach_image(-1);
    }

    return NULL;
}

static tui_menu_item_def_t detach_cartridge_menu_items[] = {
    { "--" },
    { "_Cartridges",
      "Detach all attached cartridge images",
      detach_cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

/* ------------------------------------------------------------------------- */

enum {
    MEM_NONE,
    MEM_ALL,
    MEM_3K,
    MEM_8K,
    MEM_16K,
    MEM_24K
};

enum {
    BLOCK_0 = 1,
    BLOCK_1 = 1 << 1,
    BLOCK_2 = 1 << 2,
    BLOCK_3 = 1 << 3,
    BLOCK_5 = 1 << 5
};

static TUI_MENU_CALLBACK(set_common_memory_configuration_callback)
{
    if (been_activated) {
        int blocks;

        switch ((int)param) {
            case MEM_NONE:
                blocks = 0;
                break;
            case MEM_ALL:
                blocks = BLOCK_0 | BLOCK_1 | BLOCK_2 | BLOCK_3 | BLOCK_5;
                break;
            case MEM_3K:
                blocks = BLOCK_0;
                break;
            case MEM_8K:
                blocks = BLOCK_1;
                break;
            case MEM_16K:
                blocks = BLOCK_1 | BLOCK_2;
                break;
            case MEM_24K:
                blocks = BLOCK_1 | BLOCK_2 | BLOCK_3;
                break;
            default:
                /* Shouldn't happen.  */
                log_debug("What?!");
                blocks = 0;         /* Make compiler happy.  */
        }
        resources_set_int("RamBlock0", blocks & BLOCK_0 ? 1 : 0);
        resources_set_int("RamBlock1", blocks & BLOCK_1 ? 1 : 0);
        resources_set_int("RamBlock2", blocks & BLOCK_2 ? 1 : 0);
        resources_set_int("RamBlock3", blocks & BLOCK_3 ? 1 : 0);
        resources_set_int("RamBlock5", blocks & BLOCK_5 ? 1 : 0);
        ui_update_menus();
    }

    /* This way, the "Not Really!" item is always the default one.  */
    *become_default = 0;
    return NULL;
}

static tui_menu_item_def_t common_memory_configurations_items[] = {
    { "_Not Really!",
      "Keep the current settings",
      NULL, NULL, 0, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "--" },
    { "_Unexpanded",
      "Setup a completely unexpanded VIC20",
      set_common_memory_configuration_callback, (void *)MEM_NONE, NULL,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_3K (block 0)",
      "Setup a 3K-expanded VIC20",
      set_common_memory_configuration_callback, (void *)MEM_3K, NULL,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_8K (block 1)",
      "Setup an 8K-expanded VIC20",
      set_common_memory_configuration_callback, (void *)MEM_8K, NULL,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_16K (blocks 1/2)",
      "Setup an 8K-expanded VIC20",
      set_common_memory_configuration_callback, (void *)MEM_16K, NULL,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_24K (blocks 1/2/3)",
      "Setup a 24K-expanded VIC20",
      set_common_memory_configuration_callback, (void *)MEM_24K, NULL,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "_All (blocks 0/1/2/3/5)",
      "Setup a VIC20 with all the possible RAM stuffed in",
      set_common_memory_configuration_callback, (void *)MEM_ALL, NULL,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(RAMBlock0)
TUI_MENU_DEFINE_TOGGLE(RAMBlock1)
TUI_MENU_DEFINE_TOGGLE(RAMBlock2)
TUI_MENU_DEFINE_TOGGLE(RAMBlock3)
TUI_MENU_DEFINE_TOGGLE(RAMBlock5)

static tui_menu_item_def_t special_menu_items[] = {
    { "Choose Common _Memory Configuration...",
      "Choose memory configuration from a set of common ones.",
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, common_memory_configurations_items,
      "Setup Memory Configuration" },
    { "  RAM Block _0 (3K at $0400-$0F00)",
      "Enable RAM expansion block at address $0400-$0F00",
      toggle_RAMBlock0_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "  RAM Block _1 (8K at $2000-$3FFF)",
      "Enable RAM expansion block at address $2000-$3FFF",
      toggle_RAMBlock1_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "  RAM Block _2 (8K at $4000-$5FFF)",
      "Enable RAM expansion block at address $4000-$5FFF",
      toggle_RAMBlock2_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "  RAM Block _3 (8K at $6000-$7FFF)",
      "Enable RAM expansion block at address $6000-$7FFF",
      toggle_RAMBlock3_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "  RAM Block _5 (8K at $A000-$BFFF)",
      "Enable RAM expansion block at address $A000-$BFFF",
      toggle_RAMBlock5_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "--" },
    TUI_MENU_ITEM_DEF_LIST_END
};

/* ------------------------------------------------------------------------- */

static TUI_MENU_CALLBACK(load_rom_file_callback)
{
    if (been_activated) {
        char *name;

        name = tui_file_selector("Load ROM file", NULL, "*", NULL, NULL, NULL, NULL);

        if (name != NULL) {
            if (resources_set_string(param, name) < 0) {
                ui_error("Could not load ROM file '%s'", name);
            }
            lib_free(name);
        }
    }
    return NULL;
}

static tui_menu_item_def_t rom_menu_items[] = {
    { "--" },
    { "Load new Kernal ROM...",
      "Load new Kernal ROM",
      load_rom_file_callback, "KernalName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new BASIC ROM...",
      "Load new BASIC ROM",
      load_rom_file_callback, "BasicName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new Character ROM...",
      "Load new Character ROM",
      load_rom_file_callback, "ChargenName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "--" },
    { "Load new 1540 ROM...",
      "Load new 1540 ROM",
      load_rom_file_callback, "DosName1540", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1541 ROM...",
      "Load new 1541 ROM",
      load_rom_file_callback, "DosName1541", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1541-II ROM...",
      "Load new 1541-II ROM",
      load_rom_file_callback, "DosName1541ii", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1570 ROM...",
      "Load new 1570 ROM",
      load_rom_file_callback, "DosName1570", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1571 ROM...",
      "Load new 1571 ROM",
      load_rom_file_callback, "DosName1571", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1581 ROM...",
      "Load new 1581 ROM",
      load_rom_file_callback, "DosName1581", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 2000 ROM...",
      "Load new 2000 ROM",
      load_rom_file_callback, "DosName2000", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 4000 ROM...",
      "Load new 4000 ROM",
      load_rom_file_callback, "DosName4000", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 2031 ROM...",
      "Load new 2031 ROM",
      load_rom_file_callback, "DosName2031", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 2040 ROM...",
      "Load new 2040 ROM",
      load_rom_file_callback, "DosName2040", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 3040 ROM...",
      "Load new 3040 ROM",
      load_rom_file_callback, "DosName3040", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 4040 ROM...",
      "Load new 4040 ROM",
      load_rom_file_callback, "DosName4040", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1001 ROM...",
      "Load new 1001 ROM",
      load_rom_file_callback, "DosName1001", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(FinalExpansionWriteBack)

static tui_menu_item_def_t final_expansion_menu_items[] = {
    { "_Enable write-back to cart file:", "Enable write-back to cart file",
      toggle_FinalExpansionWriteBack_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(VicFlashPluginWriteBack)

static tui_menu_item_def_t vic_flash_plugin_menu_items[] = {
    { "_Enable write-back to cart file:", "Enable write-back to cart file",
      toggle_VicFlashPluginWriteBack_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(UltiMemWriteBack)

static tui_menu_item_def_t ultimem_menu_items[] = {
    { "_Enable write-back to cart file:", "Enable write-back to cart file",
      toggle_UltiMemWriteBack_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(IO2RAM)
TUI_MENU_DEFINE_TOGGLE(IO3RAM)

static tui_menu_item_def_t io_ram_menu_items[] = {
    { "Enable I/O-2 RAM:", "Enable I/O-2 RAM",
      toggle_IO2RAM_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Enable I/O-3 RAM:", "Enable I/O-3 RAM",
      toggle_IO3RAM_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(MegaCartNvRAMWriteBack)

static TUI_MENU_CALLBACK(megacart_nvram_image_file_callback)
{
    char s[256];
    const char *v;

    if (been_activated) {

        *s = '\0';

        if (tui_input_string("Change Mega-Cart nvram image name", "New image name:", s, 255) == -1) {
            return NULL;
        }

        if (*s == '\0') {
            return NULL;
        }

        resources_set_string("MegaCartNvRAMfilename", s);
    }

    resources_get_string("MegaCartNvRAMfilename", &v);
    return v;
}

static tui_menu_item_def_t megacart_menu_items[] = {
    { "_Enable write-back to Mega-Cart nvram:", "Enable write-back to nvram file",
      toggle_MegaCartNvRAMWriteBack_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Mega-Cart nvram _image file:", "Select the Mega-Cart nvram image file",
      megacart_nvram_image_file_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

TUI_MENU_DEFINE_TOGGLE(VFLImod)

static tui_menu_item_def_t vflimod_menu_items[] = {
    { "_Enable VFLI modification:", "Enable VFLI modification",
      toggle_VFLImod_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

/* ------------------------------------------------------------------------- */

int vic20ui_init(void)
{
    tui_menu_t ui_ioextensions_submenu;
    tui_menu_t ui_final_expansion_submenu;
    tui_menu_t ui_vic_flash_plugin_submenu;
    tui_menu_t ui_ultimem_submenu;
    tui_menu_t ui_megacart_submenu;
    tui_menu_t ui_io_ram_submenu;
    tui_menu_t ui_vflimod_submenu;

    ui_create_main_menu(1, 1, 1, 0x16, 1, drivevic20_settings_submenu);

    tui_menu_add(ui_attach_submenu, attach_cartridge_menu_items);
    tui_menu_add(ui_detach_submenu, detach_cartridge_menu_items);
    tui_menu_add(ui_special_submenu, special_menu_items);

    uivic20model_init(ui_special_submenu);

    ui_ioextensions_submenu = tui_menu_create("I/O Extensions", 1);

    tui_menu_add_submenu(ui_special_submenu, "_I/O Extensions...",
                         "I/O Extensions",
                         ui_ioextensions_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    uiiocollisions_init(ui_ioextensions_submenu);

    uisidcart_init(ui_ioextensions_submenu, "$9800", "$9C00", "VIC20", 0x9800, 0x9c00);

    ui_final_expansion_submenu = tui_menu_create("Final Expansion settings", 1);

    tui_menu_add(ui_final_expansion_submenu, final_expansion_menu_items);

    tui_menu_add_submenu(ui_ioextensions_submenu, "_Final Expansion settings...",
                         "Final Expansion settings",
                         ui_final_expansion_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    ui_vic_flash_plugin_submenu = tui_menu_create("Vic Flash Plugin settings", 1);

    tui_menu_add(ui_vic_flash_plugin_submenu, vic_flash_plugin_menu_items);

    tui_menu_add_submenu(ui_ioextensions_submenu, "_Vic Flash Plugin settings...",
                         "Vic Flash Plugin settings",
                         ui_vic_flash_plugin_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    ui_ultimem_submenu = tui_menu_create("UltiMem settings", 1);

    tui_menu_add(ui_ultimem_submenu, ultimem_menu_items);

    tui_menu_add_submenu(ui_ioextensions_submenu, "_UltiMem settings...",
                         "UltiMem settings",
                         ui_ultimem_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    ui_megacart_submenu = tui_menu_create("Mega-Cart settings", 1);

    tui_menu_add(ui_megacart_submenu, megacart_menu_items);

    tui_menu_add_submenu(ui_ioextensions_submenu, "_Mega-Cart settings...",
                         "Mega-Cart settings",
                         ui_megacart_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    ui_io_ram_submenu = tui_menu_create("I/O RAM settings", 1);

    tui_menu_add(ui_io_ram_submenu, io_ram_menu_items);

    tui_menu_add_submenu(ui_ioextensions_submenu, "_I/O RAM settings...",
                         "I/O RAM settings",
                         ui_io_ram_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    ui_vflimod_submenu = tui_menu_create("VFLI modification settings", 1);

    tui_menu_add(ui_vflimod_submenu, vflimod_menu_items);

    tui_menu_add_submenu(ui_ioextensions_submenu, "_VFLI modification settings...",
                         "VFLI modification  settings",
                         ui_vflimod_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    uidigimax_vic20_init(ui_ioextensions_submenu);
    uids12c887rtc_vic20_init(ui_ioextensions_submenu);
    uigeoram_vic20_init(ui_ioextensions_submenu);
    uisoundexpander_vic20_init(ui_ioextensions_submenu);
    uisoundsampler_init(ui_ioextensions_submenu);
#ifdef HAVE_RAWNET
    uiethernetcart_vic20_init(ui_ioextensions_submenu);
#endif

    uiuserport_pet_vic20_init(ui_ioextensions_submenu);

    uitapeport_init(ui_ioextensions_submenu);

    tui_menu_add(ui_rom_submenu, rom_menu_items);

    uivideo_init(ui_video_submenu, VID_VIC, VID_NONE);

    return 0;
}

void vic20ui_shutdown(void)
{
}
