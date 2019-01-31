/*
 * ui_file.h - UI stuff dealing with files
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#ifndef VICE__UI_FILE_H__
#define VICE__UI_FILE_H__

#include <FilePanel.h>
#include <ListView.h>

extern "C" {
#include "imagecontents.h"
}

const uint32 AUTOSTART_MESSAGE = 'AS01';

enum filetype_t {
    AUTOSTART_FILE,
    AUTOSTART_DISK_IMAGE_FILE,
    DISK_FILE,
    TAPE_FILE,
    SNAPSHOTSAVE_FILE,
    SNAPSHOTLOAD_FILE,
    SNAPSHOT_HISTORY_START,
    SNAPSHOT_HISTORY_END,
    C64_CARTRIDGE_FILE,
    VIC20_CARTRIDGE_FILE,
    PLUS4_CARTRIDGE_FILE,
    CBM2_CARTRIDGE_FILE,
    VSID_FILE,
    REU_FILE,
    GEORAM_FILE,
    RAMCART_FILE,
    MAGICVOICE_FILE,
    DQBB_FILE,
    ISEPIC_FILE,
    PLUS60K_FILE,
    PLUS256K_FILE,
    C64_256K_FILE,
    PETREU_FILE,
    PETDWW_FILE,
    V364SPEECH_FILE,
    MMC64_BIOS_FILE,
    MMC64_IMAGE_FILE,
    MMCR_EEPROM_FILE,
    MMCR_IMAGE_FILE,
    GMOD2_EEPROM_FILE,
    C64DTV_ROM_FILE,
    EXPERT_FILE,
    C128_INT_FUNC_FILE,
    C128_EXT_FUNC_FILE,
    IDE64_FILE1,
    IDE64_FILE2,
    IDE64_FILE3,
    IDE64_FILE4,
    AIFF_FILE,
    IFF_FILE,
    MP3_FILE,
    FLAC_FILE,
    VORBIS_FILE,
    VOC_FILE,
    WAV_FILE,
    PRINTER_OUTPUT_FILE1,
    PRINTER_OUTPUT_FILE2,
    PRINTER_OUTPUT_FILE3,
    VIC20_GENERIC_CART_FILE,
    VIC20_BEHR_BONZ_FILE,
    VIC20_MEGACART_FILE,
    VIC20_MEGACART_NVRAM_FILE,
    VIC20_FINAL_EXPANSION_FILE,
    VIC20_UM_FILE,
    VIC20_FP_FILE,
    VIC20_SMART_CART_ATTACH_FILE,
    COMPUTER_KERNAL_ROM_FILE,
    COMPUTER_BASIC_ROM_FILE,
    COMPUTER_SCPU64_ROM_FILE,
    COMPUTER_KERNAL_INT_ROM_FILE,
    COMPUTER_KERNAL_DE_ROM_FILE,
    COMPUTER_KERNAL_FI_ROM_FILE,
    COMPUTER_KERNAL_FR_ROM_FILE,
    COMPUTER_KERNAL_IT_ROM_FILE,
    COMPUTER_KERNAL_NO_ROM_FILE,
    COMPUTER_KERNAL_SE_ROM_FILE,
    COMPUTER_KERNAL_CH_ROM_FILE,
    COMPUTER_KERNAL_64_ROM_FILE,
    COMPUTER_BASIC_LO_ROM_FILE,
    COMPUTER_BASIC_HI_ROM_FILE,
    COMPUTER_BASIC_64_ROM_FILE,
    COMPUTER_CHARGEN_INT_ROM_FILE,
    COMPUTER_CHARGEN_DE_ROM_FILE,
    COMPUTER_CHARGEN_FR_ROM_FILE,
    COMPUTER_CHARGEN_SE_ROM_FILE,
    COMPUTER_CHARGEN_CH_ROM_FILE,
    COMPUTER_CHARGEN_ROM_FILE,
    COMPUTER_EDITOR_ROM_FILE,
    COMPUTER_ROM_9_FILE,
    COMPUTER_ROM_A_FILE,
    COMPUTER_ROM_B_FILE,
    COMPUTER_FUNCTION_LO_ROM_FILE,
    COMPUTER_FUNCTION_HI_ROM_FILE,
    DRIVE_1540_ROM_FILE,
    DRIVE_1541_ROM_FILE,
    DRIVE_1541II_ROM_FILE,
    DRIVE_1551_ROM_FILE,
    DRIVE_1570_ROM_FILE,
    DRIVE_1571_ROM_FILE,
    DRIVE_1571CR_ROM_FILE,
    DRIVE_1581_ROM_FILE,
    DRIVE_2000_ROM_FILE,
    DRIVE_4000_ROM_FILE,
    DRIVE_2031_ROM_FILE,
    DRIVE_2040_ROM_FILE,
    DRIVE_3040_ROM_FILE,
    DRIVE_4040_ROM_FILE,
    DRIVE_1001_ROM_FILE,
    DRIVE_PROFDOS_ROM_FILE,
    DRIVE_SUPERCARD_ROM_FILE,
    SCREENSHOT_BMP_FILE_SCREEN0,
    SCREENSHOT_BMP_FILE_SCREEN1,
    SCREENSHOT_DOODLE_FILE_SCREEN0,
    SCREENSHOT_DOODLE_FILE_SCREEN1,
    SCREENSHOT_DOODLE_COMPRESSED_FILE_SCREEN0,
    SCREENSHOT_DOODLE_COMPRESSED_FILE_SCREEN1,
#ifdef HAVE_GIF
    SCREENSHOT_GIF_FILE_SCREEN0,
    SCREENSHOT_GIF_FILE_SCREEN1,
#endif
    SCREENSHOT_GODOT_FILE_SCREEN0,
    SCREENSHOT_GODOT_FILE_SCREEN1,
    SCREENSHOT_IFF_FILE_SCREEN0,
    SCREENSHOT_IFF_FILE_SCREEN1,
#ifdef HAVE_JPEG
    SCREENSHOT_JPEG_FILE_SCREEN0,
    SCREENSHOT_JPEG_FILE_SCREEN1,
#endif
    SCREENSHOT_KOALA_FILE_SCREEN0,
    SCREENSHOT_KOALA_FILE_SCREEN1,
    SCREENSHOT_KOALA_COMPRESSED_FILE_SCREEN0,
    SCREENSHOT_KOALA_COMPRESSED_FILE_SCREEN1,
    SCREENSHOT_PCX_FILE_SCREEN0,
    SCREENSHOT_PCX_FILE_SCREEN1,
#ifdef HAVE_PNG
    SCREENSHOT_PNG_FILE_SCREEN0,
    SCREENSHOT_PNG_FILE_SCREEN1,
#endif
    SCREENSHOT_PPM_FILE_SCREEN0,
    SCREENSHOT_PPM_FILE_SCREEN1,
    TAPELOG_FILE,
    SAMPLER_MEDIA_FILE
};

class ViceFilePanel;

class VicePreview : public BWindow {
    public :
        VicePreview(BPoint origin, ViceFilePanel *f);
        void DisplayContent(image_contents_t *content);
        void MessageReceived(BMessage *msg);
        BListView *contentlist;
        char image_name[256];
        ViceFilePanel *father;
};

class ViceFilePanel : public BFilePanel {
    public : 
        ViceFilePanel(file_panel_mode, BMessenger *, entry_ref *, uint32, bool);
        void SelectionChanged(void);
        void WasHidden(void);
        VicePreview *previewwindow;
};

extern "C" {
extern void ui_select_file(file_panel_mode panelmode, filetype_t filetype, void *fileparam);
extern void ui_select_file_action(BMessage *msg);
}

#endif
