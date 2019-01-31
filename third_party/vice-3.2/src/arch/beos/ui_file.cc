/*
 * ui_file.cc - UI stuff dealing with files
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

#include <Application.h>
#include <Button.h>
#include <CheckBox.h>
#include <FilePanel.h>
#include <Font.h>
#include <ListView.h>
#include <Message.h>
#include <Path.h>
#include <ScrollView.h>
#include <stdio.h>
#include <string.h>
#include <View.h>
#include <Window.h>

#include "ui_file.h"
#include "vicewindow.h"

extern "C" {
#include "attach.h"
#include "autostart.h"
#include "cartridge.h"
#include "constants.h"
#include "diskcontents.h"
#include "imagecontents.h"
#include "interrupt.h"
#include "lib.h"
#include "machine.h"
#include "resources.h"
#include "screenshot.h"
#include "snapshot.h"
#include "tape.h"
#include "tapecontents.h"
#include "ui.h"
#include "uiapi.h"
#include "util.h"
#include "vsync.h"
}

extern ViceWindow *windowlist[];
extern int window_count;

static int last_fileparam[2]; /* 0=filepanel, 1=savepanel */
static int last_filetype[2];
static BCheckBox *cb_readonly;

static image_contents_t *read_disk_or_tape_image_contents(const char *name)
{
    image_contents_t *contents;

    contents = diskcontents_filesystem_read(name);
    if (contents == NULL) {
        contents = tapecontents_read(name);
    }
    return contents;
}

static void create_content_list(BListView *contentlist, image_contents_t *contents)
{
    char *start;
    image_contents_file_list_t *p = contents->file_list;

    while (contentlist->CountItems()) {
        BListItem *item = contentlist->FirstItem();
        contentlist->RemoveItem(item);
        delete item;
    }

    start = image_contents_to_string(contents, IMAGE_CONTENTS_STRING_UTF8);
    contentlist->AddItem(new BStringItem(start));
    lib_free(start);

    if (p == NULL) {
        contentlist->AddItem(new BStringItem("(empty image.)"));
    } else do {
        start = image_contents_file_to_string(p, IMAGE_CONTENTS_STRING_UTF8);
        contentlist->AddItem(new BStringItem(start));
        lib_free(start);
    } while ((p = p->next) != NULL);

    if (contents->blocks_free >= 0) {
        start = lib_msprintf("%d blocks free.", contents->blocks_free);
        contentlist->AddItem(new BStringItem(start));
        lib_free(start);
    }
}

VicePreview::VicePreview(BPoint origin, ViceFilePanel *f)
    : BWindow(BRect(origin.x, origin.y, origin.x + 320, origin.y + 200),
    "Image Contents", B_MODAL_WINDOW_LOOK, B_FLOATING_APP_WINDOW_FEEL, B_NOT_RESIZABLE | B_AVOID_FOCUS)
{
    BView *background;
    BRect r;
    BFont font(be_fixed_font);

    father = f;         

    r = Bounds();
    background = new BView(r, "backview", B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);

    r.InsetBy(10, 10);
    r.right -= 100;
    contentlist = new BListView(r, "contents", B_SINGLE_SELECTION_LIST);
    contentlist->SetInvocationMessage(new BMessage(AUTOSTART_MESSAGE));

    font.SetSize(10.0);
    contentlist->SetFont(&font);

    background->AddChild(new BScrollView("scroll_contents", contentlist, B_FOLLOW_LEFT | B_FOLLOW_TOP, 0, false, true));
        
    background->AddChild(new BButton(BRect(r.right + 20, 10, r.right + 100, 30), "Autostart", "Autostart", new BMessage(AUTOSTART_MESSAGE)));

    Minimize(true);
    Show();
    Hide();
}

void VicePreview::DisplayContent(image_contents_t *content)
{
    Lock();
    if (content) {
        if (IsHidden()) {
            Minimize(false);
            Show();
        }
        create_content_list(contentlist, content);
    } else {
        if (!IsHidden()) {
           Hide();
        }
    }
    Unlock();
}

void VicePreview::MessageReceived(BMessage *msg)
{
    int file_num;

    switch (msg->what) {
        case AUTOSTART_MESSAGE:
            file_num = contentlist->CurrentSelection();
            if (file_num >= 0) {
                father->Hide();
                autostart_autodetect(image_name, NULL, file_num, AUTOSTART_MODE_RUN);
            }
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

ViceFilePanel::ViceFilePanel(file_panel_mode mode, BMessenger *target, entry_ref *panel_directory, uint32 node_flavors, bool allow_multiple_selection)
    : BFilePanel(mode, target, panel_directory, node_flavors, allow_multiple_selection)
{
    if (mode == B_OPEN_PANEL) {
        /* readonly checkbox */
        if (Window()->Lock()) {
            BView *back = Window()->ChildAt(0);
            BRect rect = back->Bounds();

            rect.InsetBy(20, 20);
            rect.right = rect.left + 150;
            rect.top = rect.bottom - 10;
            cb_readonly = new BCheckBox(rect, "Attach read only", "Attach read only", NULL);
            cb_readonly->SetTarget(Messenger());
            Window()->Unlock();
        }

        /* create window for the previewlist */
        previewwindow = new VicePreview(BPoint(Window()->Frame().left, Window()->Frame().bottom), this);
    } else {
        previewwindow = NULL;
    }
}

void ViceFilePanel::WasHidden(void)
{
    if (previewwindow) {
        previewwindow->DisplayContent(NULL);
    }
}
        
void ViceFilePanel::SelectionChanged(void)
{
    entry_ref ref;
    BPath *path;

    Rewind();
    if (GetNextSelectedRef(&ref) != B_ENTRY_NOT_FOUND && previewwindow) {
        path = new BPath(&ref);
        if (path->Path() && read_disk_or_tape_image_contents(path->Path())) {
            strncpy(previewwindow->image_name, path->Path(), 255); 
            previewwindow->DisplayContent(read_disk_or_tape_image_contents(path->Path()));
            previewwindow->MoveTo(BPoint(Window()->Frame().left, Window()->Frame().bottom+5));
#ifndef __HAIKU__
            Window()->SendBehind(previewwindow);
#endif
        } else {
            previewwindow->DisplayContent(NULL);
        }
        delete path;
    }

    BFilePanel::SelectionChanged();
}

void ui_select_file(file_panel_mode panelmode, filetype_t filetype, void *fileparam)
{
    ViceFilePanel *filepanel;
    int panelnr; /* = (panelmode == B_OPEN_PANEL ? 0 : 1); */
    char title[40];

    if (panelmode == B_OPEN_PANEL) {
        filepanel = windowlist[0]->filepanel;
        panelnr = 0;
    } else {
        filepanel = windowlist[0]->savepanel;
        panelnr = 1;
    }

    sprintf(title,"VICE filepanel"); /* default */

    /* Modify the panel */
    /* first we may remove the readonly checkbox */
    if (filepanel->Window()->Lock()) {
        filepanel->Window()->ChildAt(0)->RemoveChild(cb_readonly);
        filepanel->Window()->Unlock();
    }
    if (filetype == DISK_FILE) {
        BMessage *msg = new BMessage(MESSAGE_ATTACH_READONLY);
        int n;

        /* attach readonly checkbox */
        if (filepanel->Window()->Lock()) {
            filepanel->Window()->ChildAt(0)->AddChild(cb_readonly);
            msg->MakeEmpty();
            msg->AddInt32("device", *(int*)fileparam);
            cb_readonly->SetMessage(msg);
            resources_get_int_sprintf("AttachDevice%dReadonly", &n, *(int*)fileparam);
            cb_readonly->SetValue(n);
            filepanel->Window()->Unlock();
        }

        sprintf(title, "Attach Disk %d", *(int*)fileparam);
        last_fileparam[panelnr] = *(int*)fileparam;
    }
    if (filetype == TAPE_FILE) {
        sprintf(title, "Attach Tape");
    }
    if (filetype == AUTOSTART_FILE) {
        sprintf(title, "Autostart");
    }
    if (filetype == AUTOSTART_DISK_IMAGE_FILE) {
        sprintf(title, "Autostart Disk Image");
    }
    if (filetype == SNAPSHOTSAVE_FILE) {
        sprintf(title, "Save snapshot");
    }
    if (filetype == SNAPSHOTLOAD_FILE) {
        sprintf(title, "Load snapshot");
    }
    if (filetype == C64_CARTRIDGE_FILE || filetype == VIC20_CARTRIDGE_FILE || filetype == PLUS4_CARTRIDGE_FILE || filetype == CBM2_CARTRIDGE_FILE) {
        sprintf(title," Attach Cartridge (%s)", ((ui_cartridge_t*)fileparam)->cart_name);
        last_fileparam[panelnr] = ((ui_cartridge_t*)fileparam)->cart_type;
    }
    if (filetype == VSID_FILE) {
        sprintf(title, "Load psid file");
    }
    if (filetype == SNAPSHOT_HISTORY_START) {
        sprintf(title, "Select start snapshot");
    }
    if (filetype == SNAPSHOT_HISTORY_END) {
        sprintf(title, "Select end snapshot");
    }
    if (filetype == REU_FILE) {
        sprintf(title, "Select REU file");
    }
    if (filetype == GEORAM_FILE) {
        sprintf(title, "Select GEO-RAM file");
    }
    if (filetype == RAMCART_FILE) {
        sprintf(title, "Select RamCart file");
    }
    if (filetype == MAGICVOICE_FILE) {
        sprintf(title, "Select Magic Voice file");
    }
    if (filetype == DQBB_FILE) {
        sprintf(title, "Select DQBB file");
    }
    if (filetype == ISEPIC_FILE) {
        sprintf(title, "Select ISEPIC file");
    }
    if (filetype == PLUS60K_FILE) {
        sprintf(title, "Select +60K file");
    }
    if (filetype == PLUS256K_FILE) {
        sprintf(title, "Select +256K file");
    }
    if (filetype == C64_256K_FILE) {
        sprintf(title, "Select C64_256K file");
    }
    if (filetype == PETREU_FILE) {
        sprintf(title, "Select PET REU file");
    }
    if (filetype == PETDWW_FILE) {
        sprintf(title, "Select PET DWW file");
    }
    if (filetype == V364SPEECH_FILE) {
        sprintf(title, "Select V364 speech file");
    }
    if (filetype == MMC64_BIOS_FILE) {
        sprintf(title, "Select MMC64 BIOS file");
    }
    if (filetype == MMC64_IMAGE_FILE) {
        sprintf(title, "Select MMC64 image file");
    }
    if (filetype == MMCR_EEPROM_FILE) {
        sprintf(title, "Select MMC Replay EEPROM file");
    }
    if (filetype == MMCR_IMAGE_FILE) {
        sprintf(title, "Select MMC Replay image file");
    }
    if (filetype == GMOD2_EEPROM_FILE) {
        sprintf(title, "Select GMod2 EEPROM file");
    }
    if (filetype == C64DTV_ROM_FILE) {
        sprintf(title, "Select C64DTV ROM file");
    }
    if (filetype == EXPERT_FILE) {
        sprintf(title, "Select Expert Cartridge file");
    }
    if (filetype == SCREENSHOT_BMP_FILE_SCREEN0 || filetype == SCREENSHOT_BMP_FILE_SCREEN1) {
        sprintf(title, "Select bmp file");
    }
    if (filetype == SCREENSHOT_DOODLE_FILE_SCREEN0 || filetype == SCREENSHOT_DOODLE_FILE_SCREEN1) {
        sprintf(title, "Select doodle file");
    }
    if (filetype == SCREENSHOT_DOODLE_COMPRESSED_FILE_SCREEN0 || filetype == SCREENSHOT_DOODLE_COMPRESSED_FILE_SCREEN1) {
        sprintf(title, "Select compressed doodle file");
    }
#ifdef HAVE_GIF
    if (filetype == SCREENSHOT_GIF_FILE_SCREEN0 || filetype == SCREENSHOT_GIF_FILE_SCREEN1) {
        sprintf(title, "Select gif file");
    }
#endif
    if (filetype == SCREENSHOT_GODOT_FILE_SCREEN0 || filetype == SCREENSHOT_GODOT_FILE_SCREEN1) {
        sprintf(title, "Select godot file");
    }
    if (filetype == SCREENSHOT_IFF_FILE_SCREEN0 || filetype == SCREENSHOT_IFF_FILE_SCREEN1) {
        sprintf(title, "Select iff file");
    }
#ifdef HAVE_JPEG
    if (filetype == SCREENSHOT_JPEG_FILE_SCREEN0 || filetype == SCREENSHOT_JPEG_FILE_SCREEN1) {
        sprintf(title, "Select jpeg file");
    }
#endif
    if (filetype == SCREENSHOT_KOALA_FILE_SCREEN0 || filetype == SCREENSHOT_KOALA_FILE_SCREEN1) {
        sprintf(title, "Select koala file");
    }
    if (filetype == SCREENSHOT_KOALA_COMPRESSED_FILE_SCREEN0 || filetype == SCREENSHOT_KOALA_COMPRESSED_FILE_SCREEN1) {
        sprintf(title, "Select compressed koala file");
    }
    if (filetype == SCREENSHOT_PCX_FILE_SCREEN0 || filetype == SCREENSHOT_PCX_FILE_SCREEN1) {
        sprintf(title, "Select pcx file");
    }
#ifdef HAVE_PNG
    if (filetype == SCREENSHOT_PNG_FILE_SCREEN0 || filetype == SCREENSHOT_PNG_FILE_SCREEN1) {
        sprintf(title, "Select png file");
    }
#endif
    if (filetype == SCREENSHOT_PPM_FILE_SCREEN0 || filetype == SCREENSHOT_PPM_FILE_SCREEN1) {
        sprintf(title, "Select ppm file");
    }
    if (filetype == TAPELOG_FILE) {
        sprintf(title, "Select tapelog log file");
    }
    if (filetype == C128_INT_FUNC_FILE) {
        sprintf(title, "Select Internal Function ROM file");
    }
    if (filetype == C128_EXT_FUNC_FILE) {
        sprintf(title, "Select External Function ROM file");
    }
    if (filetype == IDE64_FILE1) {
        sprintf(title, "Select IDE64 HD #1 file");
    }
    if (filetype == IDE64_FILE2) {
        sprintf(title, "Select IDE64 HD #2 file");
    }
    if (filetype == IDE64_FILE3) {
        sprintf(title, "Select IDE64 HD #3 file");
    }
    if (filetype == IDE64_FILE4) {
        sprintf(title, "Select IDE64 HD #4 file");
    }
    if (filetype == PRINTER_OUTPUT_FILE1) {
        sprintf(title, "Select printer output #1");
    }
    if (filetype == PRINTER_OUTPUT_FILE2) {
        sprintf(title, "Select printer output #2");
    }
    if (filetype == PRINTER_OUTPUT_FILE3) {
        sprintf(title, "Select printer output #3");
    }
    if (filetype == AIFF_FILE) {
        sprintf(title, "Select AIFF Sound Recording file");
    }
    if (filetype == IFF_FILE) {
        sprintf(title, "Select IFF Sound Recording file");
    }
#ifdef USE_LAMEMP3
    if (filetype == MP3_FILE) {
        sprintf(title, "Select MP3 Sound Recording file");
    }
#endif
#ifdef USE_FLAC
    if (filetype == FLAC_FILE) {
        sprintf(title, "Select FLAC Sound Recording file");
    }
#endif
#ifdef USE_VORBIS
    if (filetype == FLAC_FILE) {
        sprintf(title, "Select OGG/VORBIS Sound Recording file");
    }
#endif
    if (filetype == VOC_FILE) {
        sprintf(title, "Select VOC Sound Recording file");
    }
    if (filetype == WAV_FILE) {
        sprintf(title, "Select WAV Sound Recording file");
    }
    if (filetype == VIC20_GENERIC_CART_FILE) {
        sprintf(title, "Select generic cartridge file");
    }
    if (filetype == VIC20_UM_FILE) {
        sprintf(title, "Select UltiMem file");
    }
    if (filetype == VIC20_FP_FILE) {
        sprintf(title, "Select Vic Flash Plugin file");
    }
    if (filetype == VIC20_BEHR_BONZ_FILE) {
        sprintf(title, "Select Behr Bonz file");
    }
    if (filetype == VIC20_MEGACART_FILE) {
        sprintf(title, "Select Mega-Cart file");
    }
    if (filetype == VIC20_MEGACART_NVRAM_FILE) {
        sprintf(title, "Select Mega-Cart nvram file");
    }
    if (filetype == VIC20_FINAL_EXPANSION_FILE) {
        sprintf(title, "Select Final Expansion file");
    }
    if (filetype == VIC20_SMART_CART_ATTACH_FILE) {
        sprintf(title, "Select cartridge file");
    }
    if (filetype == COMPUTER_KERNAL_ROM_FILE) {
        sprintf(title, "Select Kernal ROM file");
    }
    if (filetype == COMPUTER_BASIC_ROM_FILE) {
        sprintf(title, "Select Basic ROM file");
    }
    if (filetype == COMPUTER_SCPU64_ROM_FILE) {
        sprintf(title, "Select SCPU64 ROM file");
    }
    if (filetype == COMPUTER_KERNAL_INT_ROM_FILE) {
        sprintf(title, "Select International Kernal ROM file");
    }
    if (filetype == COMPUTER_KERNAL_DE_ROM_FILE) {
        sprintf(title, "Select German Kernal ROM file");
    }
    if (filetype == COMPUTER_KERNAL_FI_ROM_FILE) {
        sprintf(title, "Select Finnish Kernal ROM file");
    }
    if (filetype == COMPUTER_KERNAL_FR_ROM_FILE) {
        sprintf(title, "Select French Kernal ROM file");
    }
    if (filetype == COMPUTER_KERNAL_IT_ROM_FILE) {
        sprintf(title, "Select Italian Kernal ROM file");
    }
    if (filetype == COMPUTER_KERNAL_NO_ROM_FILE) {
        sprintf(title, "Select Norwegian Kernal ROM file");
    }
    if (filetype == COMPUTER_KERNAL_SE_ROM_FILE) {
        sprintf(title, "Select Swedish Kernal ROM file");
    }
    if (filetype == COMPUTER_KERNAL_CH_ROM_FILE) {
        sprintf(title, "Select Swiss Kernal ROM file");
    }
    if (filetype == COMPUTER_KERNAL_64_ROM_FILE) {
        sprintf(title, "Select C64 Kernal ROM file");
    }
    if (filetype == COMPUTER_BASIC_LO_ROM_FILE) {
        sprintf(title, "Select Basic LO ROM file");
    }
    if (filetype == COMPUTER_BASIC_HI_ROM_FILE) {
        sprintf(title, "Select Basic HI ROM file");
    }
    if (filetype == COMPUTER_BASIC_64_ROM_FILE) {
        sprintf(title, "Select C64 Basic ROM file");
    }
    if (filetype == COMPUTER_CHARGEN_INT_ROM_FILE) {
        sprintf(title, "Select International Chargen ROM file");
    }
    if (filetype == COMPUTER_CHARGEN_DE_ROM_FILE) {
        sprintf(title, "Select German Chargen ROM file");
    }
    if (filetype == COMPUTER_CHARGEN_FR_ROM_FILE) {
        sprintf(title, "Select French Chargen ROM file");
    }
    if (filetype == COMPUTER_CHARGEN_SE_ROM_FILE) {
        sprintf(title, "Select Swedish Chargen ROM file");
    }
    if (filetype == COMPUTER_CHARGEN_CH_ROM_FILE) {
        sprintf(title, "Select Swiss Chargen ROM file");
    }
    if (filetype == COMPUTER_CHARGEN_ROM_FILE) {
        sprintf(title, "Select Chargen ROM file");
    }
    if (filetype == COMPUTER_EDITOR_ROM_FILE) {
        sprintf(title, "Select Editor ROM file");
    }
    if (filetype == COMPUTER_ROM_9_FILE) {
        sprintf(title, "Select ROM 9 file");
    }
    if (filetype == COMPUTER_ROM_A_FILE) {
        sprintf(title, "Select ROM A file");
    }
    if (filetype == COMPUTER_ROM_B_FILE) {
        sprintf(title, "Select ROM B file");
    }
    if (filetype == COMPUTER_FUNCTION_LO_ROM_FILE) {
        sprintf(title, "Select Function LO ROM file");
    }
    if (filetype == COMPUTER_FUNCTION_HI_ROM_FILE) {
        sprintf(title, "Select Function HI ROM file");
    }
    if (filetype == DRIVE_1540_ROM_FILE) {
        sprintf(title, "Select 1540 ROM file");
    }
    if (filetype == DRIVE_1541_ROM_FILE) {
        sprintf(title, "Select 1541 ROM file");
    }
    if (filetype == DRIVE_1541II_ROM_FILE) {
        sprintf(title, "Select 1541-II ROM file");
    }
    if (filetype == DRIVE_1551_ROM_FILE) {
        sprintf(title, "Select 1551 ROM file");
    }
    if (filetype == DRIVE_1570_ROM_FILE) {
        sprintf(title, "Select 1570 ROM file");
    }
    if (filetype == DRIVE_1571_ROM_FILE) {
        sprintf(title, "Select 1571 ROM file");
    }
    if (filetype == DRIVE_1571CR_ROM_FILE) {
        sprintf(title, "Select 1571CR ROM file");
    }
    if (filetype == DRIVE_1581_ROM_FILE) {
        sprintf(title, "Select 1581 ROM file");
    }
    if (filetype == DRIVE_2000_ROM_FILE) {
        sprintf(title, "Select FD2000 ROM file");
    }
    if (filetype == DRIVE_4000_ROM_FILE) {
        sprintf(title, "Select FD4000 ROM file");
    }
    if (filetype == DRIVE_2031_ROM_FILE) {
        sprintf(title, "Select 2031 ROM file");
    }
    if (filetype == DRIVE_2040_ROM_FILE) {
        sprintf(title, "Select 2040 ROM file");
    }
    if (filetype == DRIVE_3040_ROM_FILE) {
        sprintf(title, "Select 3040 ROM file");
    }
    if (filetype == DRIVE_4040_ROM_FILE) {
        sprintf(title, "Select 4040 ROM file");
    }
    if (filetype == DRIVE_1001_ROM_FILE) {
        sprintf(title, "Select 1001 ROM file");
    }
    if (filetype == DRIVE_PROFDOS_ROM_FILE) {
        sprintf(title, "Select Professional DOS ROM file");
    }
    if (filetype == DRIVE_SUPERCARD_ROM_FILE) {
        sprintf(title, "Select SuperCard+ ROM file");
    }
    if (filetype == SAMPLER_MEDIA_FILE) {
        sprintf(title, "Select sampler media file");
    }

    filepanel->Window()->SetTitle(title);

    filepanel->Show();

    /* remember for later action */
    last_filetype[panelnr] = filetype;

    /* wait for save panel to disappear */
    if (filepanel->PanelMode() == B_SAVE_PANEL) {
        vsync_suspend_speed_eval();
        while (filepanel->IsShowing()) {
            snooze(1000);
        }
    }
}

static void load_snapshot_trap(uint16_t unused_addr, void *path)
{
    if (machine_read_snapshot((char *)path, 0) < 0) {
        snapshot_display_error();
    }
    lib_free(path);
}

static void save_snapshot_trap(uint16_t unused_addr, void *path)
{
    if (machine_write_snapshot((char *)path, 1, 1, 0) < 0) {
        snapshot_display_error();
    }
    lib_free(path);
}


static void ui_sound_record_action(const char *name, const char *ext)
{
    char *ext_name = util_add_extension_const(name, ext);

    resources_set_string("SoundRecordDeviceArg", ext_name);
    resources_set_string("SoundRecordDeviceName", ext);
    ui_display_statustext("Sound Recording Started...", 1);
    lib_free(ext_name);
}


void ui_select_file_action(BMessage *msg)
{
    entry_ref ref;
    status_t err;
    BPath *path;
    struct video_canvas_s *c0 = (struct video_canvas_s *)windowlist[0]->canvas;
    struct video_canvas_s *c1 = NULL;

    if (window_count == 2) {
        c1 = (struct video_canvas_s *)windowlist[1]->canvas;
    }

    if (msg->what == B_REFS_RECEIVED) {
        /* an open action */            
        /* extract the selected filename from the message */
        if ((err = msg->FindRef("refs", 0, &ref)) != B_OK) {
            ui_error("No File selected ?!");
            return;
        }
        path = new BPath(&ref);

        /* now the ACTION */
        if (last_filetype[0] == DISK_FILE) {
                /* it's a disk-attach */
                if (file_system_attach_disk(last_fileparam[0], path->Path()) < 0) {
                ui_error("Cannot attach specified file");
            }
        } else if (last_filetype[0] == TAPE_FILE) {
            /* it's a tape-attach */
            if (tape_image_attach(1, path->Path()) < 0) {
                ui_error("Cannot attach specified file");
            }
        } else if (last_filetype[0] == AUTOSTART_FILE) {
            if (autostart_autodetect(path->Path(), NULL, 0, AUTOSTART_MODE_RUN) < 0) {
                ui_error("Cannot autostart specified file.");
            }
        } else if (last_filetype[0] == AUTOSTART_DISK_IMAGE_FILE) {
            if (path->Path() != NULL) {
                resources_set_string("AutostartPrgDiskImage", path->Path());
            }
        } else if (last_filetype[0] == SNAPSHOTLOAD_FILE) {
            /* we need a copy of the path that won't be deleted here */
            char *pathname = lib_stralloc(path->Path());

            interrupt_maincpu_trigger_trap(load_snapshot_trap, (void *)pathname);
        } else if (last_filetype[0] == C64_CARTRIDGE_FILE) {
            BMessage *msg = new BMessage(ATTACH_C64_CART);

            msg->AddInt32("type", last_fileparam[0]);
            msg->AddString("filename", path->Path());
            ui_add_event(msg);
        } else if (last_filetype[0] == VIC20_CARTRIDGE_FILE) {
            BMessage *msg = new BMessage(ATTACH_VIC20_CART);

            msg->AddInt32("type", last_fileparam[0]);
            msg->AddString("filename", path->Path());
            ui_add_event(msg);
        } else if (last_filetype[0] == VSID_FILE) {
            BMessage *msg = new BMessage(PLAY_VSID);

            msg->AddString("filename", path->Path());
            ui_add_event(msg);
        } else if (last_filetype[0] == C64DTV_ROM_FILE) {
            resources_set_string("c64dtvromfilename", path->Path());
        } else if (last_filetype[0] == MMC64_BIOS_FILE) {
            resources_set_string("MMC64BIOSfilename", path->Path());
        } else if (last_filetype[0] == MMC64_IMAGE_FILE) {
            resources_set_string("MMC64imagefilename", path->Path());
        } else if (last_filetype[0] == MMCR_EEPROM_FILE) {
            resources_set_string("MMCREEPROMImage", path->Path());
        } else if (last_filetype[0] == MMCR_IMAGE_FILE) {
            resources_set_string("MMCRCardImage", path->Path());
        } else if (last_filetype[0] == GMOD2_EEPROM_FILE) {
            resources_set_string("GMod2EEPROMImage", path->Path());
        }
        delete path;    
    }

    if (msg->what == B_SAVE_REQUESTED) {
        char *fullpath;
        const char *name;

        /* a save action */
        /* first create the full path */
        if ((err = msg->FindRef("directory", &ref)) != B_OK) {
            ui_error("Wrong directory");
            return;
        }
        if ((err = msg->FindString("name", &name)) != B_OK) {
            ui_error("Wrong name");
            return;
        }
        path = new BPath(&ref);
        fullpath = util_concat(path->Path(), "/", name, NULL);

        /* now the action */
        if (last_filetype[1] == SNAPSHOTSAVE_FILE) {
            /* we need a copy of the path that won't be deleted here */
            char *pathname = lib_stralloc(fullpath);

            interrupt_maincpu_trigger_trap(save_snapshot_trap, (void *)pathname);
        } else if (last_filetype[1] == SNAPSHOT_HISTORY_START) {
            resources_set_string("EventStartSnapshot", name);
        } else if (last_filetype[1] == SNAPSHOT_HISTORY_END) {
            resources_set_string("EventEndSnapshot", name);
            resources_set_string("EventSnapshotDir", path->Path());
        } else if (last_filetype[1] == REU_FILE) {
            resources_set_string("REUfilename", fullpath);
        } else if (last_filetype[1] == GEORAM_FILE) {
            resources_set_string("GEORAMfilename", fullpath);
        } else if (last_filetype[1] == RAMCART_FILE) {
            resources_set_string("RAMCARTfilename", fullpath);
        } else if (last_filetype[1] == MAGICVOICE_FILE) {
            resources_set_string("MagicVoiceImage", fullpath);
        } else if (last_filetype[1] == DQBB_FILE) {
            resources_set_string("DQBBfilename", fullpath);
        } else if (last_filetype[1] == ISEPIC_FILE) {
            resources_set_string("Isepicfilename", fullpath);
        } else if (last_filetype[1] == PLUS60K_FILE) {
            resources_set_string("PLUS60Kfilename", fullpath);
        } else if (last_filetype[1] == PLUS256K_FILE) {
            resources_set_string("PLUS256Kfilename", fullpath);
        } else if (last_filetype[1] == C64_256K_FILE) {
            resources_set_string("C64_256Kfilename", fullpath);
        } else if (last_filetype[1] == PETREU_FILE) {
            resources_set_string("PETREUfilename", fullpath);
        } else if (last_filetype[1] == PETDWW_FILE) {
            resources_set_string("PETDWWfilename", fullpath);
        } else if (last_filetype[1] == V364SPEECH_FILE) {
            resources_set_string("SpeechImage", fullpath);
        } else if (last_filetype[1] == EXPERT_FILE) {
            resources_set_string("Expertfilename", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_BMP_FILE_SCREEN0) {
            if (screenshot_save("BMP", fullpath, c0) < 0)
            ui_error("Failed to write bmp screenshot %s (.bmp)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_BMP_FILE_SCREEN1) {
            if (screenshot_save("BMP", fullpath, c1) < 0)
            ui_error("Failed to write bmp screenshot %s (.bmp)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_DOODLE_FILE_SCREEN0) {
            if (screenshot_save("DOODLE", fullpath, c0) < 0)
            ui_error("Failed to write doodle screenshot %s (.dd)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_DOODLE_FILE_SCREEN1) {
            if (screenshot_save("DOODLE", fullpath, c1) < 0)
            ui_error("Failed to write doodle screenshot %s (.dd)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_DOODLE_COMPRESSED_FILE_SCREEN0) {
            if (screenshot_save("DOODLE_COMPRESSED", fullpath, c0) < 0)
            ui_error("Failed to write compressed doodle screenshot %s (.jj)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_DOODLE_COMPRESSED_FILE_SCREEN1) {
            if (screenshot_save("DOODLE_COMPRESSED", fullpath, c1) < 0)
            ui_error("Failed to write compressed doodle screenshot %s (.jj)", fullpath);
#ifdef HAVE_GIF
        } else if (last_filetype[1] == SCREENSHOT_GIF_FILE_SCREEN0) {
            if (screenshot_save("GIF", fullpath, c0) < 0)
            ui_error("Failed to write gif screenshot %s (.gif)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_GIF_FILE_SCREEN1) {
            if (screenshot_save("GIF", fullpath, c1) < 0)
            ui_error("Failed to write gif screenshot %s (.gif)", fullpath);
#endif
        } else if (last_filetype[1] == SCREENSHOT_GODOT_FILE_SCREEN0) {
            if (screenshot_save("4BT", fullpath, c0) < 0)
            ui_error("Failed to write godot screenshot %s (.4bt)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_GODOT_FILE_SCREEN1) {
            if (screenshot_save("4BT", fullpath, c1) < 0)
            ui_error("Failed to write godot screenshot %s (.4bt)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_IFF_FILE_SCREEN0) {
            if (screenshot_save("IFF", fullpath, c0) < 0)
            ui_error("Failed to write iff screenshot %s (.iff)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_IFF_FILE_SCREEN1) {
            if (screenshot_save("IFF", fullpath, c1) < 0)
            ui_error("Failed to write iff screenshot %s (.iff)", fullpath);
#ifdef HAVE_JPEG
        } else if (last_filetype[1] == SCREENSHOT_JPEG_FILE_SCREEN0) {
            if (screenshot_save("JPEG", fullpath, c0) < 0)
            ui_error("Failed to write jpeg screenshot %s (.jpg)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_JPEG_FILE_SCREEN1) {
            if (screenshot_save("JPEG", fullpath, c1) < 0)
            ui_error("Failed to write jpeg screenshot %s (.jpg)", fullpath);
#endif
        } else if (last_filetype[1] == SCREENSHOT_KOALA_FILE_SCREEN0) {
            if (screenshot_save("KOALA", fullpath, c0) < 0)
            ui_error("Failed to write koala screenshot %s (.koa)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_KOALA_FILE_SCREEN1) {
            if (screenshot_save("KOALA", fullpath, c1) < 0)
            ui_error("Failed to write koala screenshot %s (.koa)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_KOALA_COMPRESSED_FILE_SCREEN0) {
            if (screenshot_save("KOALA_COMPRESSED", fullpath, c0) < 0)
            ui_error("Failed to write compressed koala screenshot %s (.gg)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_KOALA_COMPRESSED_FILE_SCREEN1) {
            if (screenshot_save("KOALA_COMPRESSED", fullpath, c1) < 0)
            ui_error("Failed to write compressed koala screenshot %s (.gg)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_PCX_FILE_SCREEN0) {
            if (screenshot_save("PCX", fullpath, c0) < 0)
            ui_error("Failed to write pcx screenshot %s (.pcx)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_PCX_FILE_SCREEN1) {
            if (screenshot_save("PCX", fullpath, c1) < 0)
            ui_error("Failed to write pcx screenshot %s (.pcx)", fullpath);
#ifdef HAVE_PNG
        } else if (last_filetype[1] == SCREENSHOT_PNG_FILE_SCREEN0) {
            if (screenshot_save("PNG", fullpath, c0) < 0)
            ui_error("Failed to write png screenshot %s (.png)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_PNG_FILE_SCREEN1) {
            if (screenshot_save("PNG", fullpath, c1) < 0)
            ui_error("Failed to write png screenshot %s (.png)", fullpath);
#endif
        } else if (last_filetype[1] == SCREENSHOT_PPM_FILE_SCREEN0) {
            if (screenshot_save("PPM", fullpath, c0) < 0)
            ui_error("Failed to write ppm screenshot %s (.ppm)", fullpath);
        } else if (last_filetype[1] == SCREENSHOT_PPM_FILE_SCREEN1) {
            if (screenshot_save("PPM", fullpath, c1) < 0)
            ui_error("Failed to write ppm screenshot %s (.ppm)", fullpath);
        } else if (last_filetype[1] == TAPELOG_FILE) {
            resources_set_string("TapeLogfilename", fullpath);
        } else if (last_filetype[1] == SAMPLER_MEDIA_FILE) {
            resources_set_string("SampleName", fullpath);
        } else if (last_filetype[1] == C128_INT_FUNC_FILE) {
            resources_set_string("InternalFunctionName", fullpath);
        } else if (last_filetype[1] == C128_EXT_FUNC_FILE) {
            resources_set_string("ExternalFunctionName", fullpath);
        } else if (last_filetype[1] == IDE64_FILE1) {
            resources_set_string("IDE64Image1", fullpath);
        } else if (last_filetype[1] == IDE64_FILE2) {
            resources_set_string("IDE64Image2", fullpath);
        } else if (last_filetype[1] == IDE64_FILE3) {
            resources_set_string("IDE64Image3", fullpath);
        } else if (last_filetype[1] == IDE64_FILE4) {
            resources_set_string("IDE64Image4", fullpath);
        } else if (last_filetype[1] == PRINTER_OUTPUT_FILE1) {
            resources_set_string("PrinterTextDevice1", fullpath);
        } else if (last_filetype[1] == PRINTER_OUTPUT_FILE2) {
            resources_set_string("PrinterTextDevice2", fullpath);
        } else if (last_filetype[1] == PRINTER_OUTPUT_FILE3) {
            resources_set_string("PrinterTextDevice3", fullpath);
        } else if (last_filetype[1] == AIFF_FILE) {
            ui_sound_record_action(fullpath, "aiff");
        } else if (last_filetype[1] == IFF_FILE) {
            ui_sound_record_action(fullpath, "iff");
#ifdef USE_LAMEMP3
        } else if (last_filetype[1] == MP3_FILE) {
            ui_sound_record_action(fullpath, "mp3");
#endif
#ifdef USE_FLAC
        } else if (last_filetype[1] == FLAC_FILE) {
            ui_sound_record_action(fullpath, "flac");
#endif
#ifdef USE_VORBIS
        } else if (last_filetype[1] == VORBIS_FILE) {
            ui_sound_record_action(fullpath, "ogg");
#endif
        } else if (last_filetype[1] == VOC_FILE) {
            ui_sound_record_action(fullpath, "voc");
        } else if (last_filetype[1] == WAV_FILE) {
            ui_sound_record_action(fullpath, "wav");
        } else if (last_filetype[1] == WAV_FILE) {
            ui_sound_record_action(fullpath, "wav");
        } else if (last_filetype[1] == VIC20_GENERIC_CART_FILE) {
            if (cartridge_attach_image(CARTRIDGE_VIC20_GENERIC, fullpath) < 0) {
                ui_error("Invalid cartridge image");
            }
        } else if (last_filetype[1] == VIC20_UM_FILE) {
            if (cartridge_attach_image(CARTRIDGE_VIC20_UM, fullpath) < 0) {
                ui_error("Invalid cartridge image");
            }
        } else if (last_filetype[1] == VIC20_FP_FILE) {
            if (cartridge_attach_image(CARTRIDGE_VIC20_FP, fullpath) < 0) {
                ui_error("Invalid cartridge image");
            }
        } else if (last_filetype[1] == VIC20_BEHR_BONZ_FILE) {
            if (cartridge_attach_image(CARTRIDGE_VIC20_BEHRBONZ, fullpath) < 0) {
                ui_error("Invalid cartridge image");
            }
        } else if (last_filetype[1] == VIC20_MEGACART_FILE) {
            if (cartridge_attach_image(CARTRIDGE_VIC20_MEGACART, fullpath) < 0) {
                ui_error("Invalid cartridge image");
            }
        } else if (last_filetype[1] == VIC20_MEGACART_NVRAM_FILE) {
            if (fullpath != NULL) {
                resources_set_string("MegaCartNvRAMfilename", fullpath);
            }
        } else if (last_filetype[1] == VIC20_FINAL_EXPANSION_FILE) {
            if (cartridge_attach_image(CARTRIDGE_VIC20_FINAL_EXPANSION, fullpath) < 0) {
                ui_error("Invalid cartridge image");
            }
        } else if (last_filetype[1] == VIC20_SMART_CART_ATTACH_FILE) {
            if (cartridge_attach_image(CARTRIDGE_VIC20_DETECT, fullpath) < 0) {
                ui_error("Invalid cartridge image");
            }
        } else if (last_filetype[1] == COMPUTER_KERNAL_ROM_FILE) {
            resources_set_string("KernalName", fullpath);
        } else if (last_filetype[1] == COMPUTER_BASIC_ROM_FILE) {
            resources_set_string("BasicName", fullpath);
        } else if (last_filetype[1] == COMPUTER_SCPU64_ROM_FILE) {
            resources_set_string("SCPU64Name", fullpath);
        } else if (last_filetype[1] == COMPUTER_KERNAL_INT_ROM_FILE) {
            resources_set_string("KernalIntName", fullpath);
        } else if (last_filetype[1] == COMPUTER_KERNAL_DE_ROM_FILE) {
            resources_set_string("KernalDEName", fullpath);
        } else if (last_filetype[1] == COMPUTER_KERNAL_FI_ROM_FILE) {
            resources_set_string("KernalFIName", fullpath);
        } else if (last_filetype[1] == COMPUTER_KERNAL_FR_ROM_FILE) {
            resources_set_string("KernalFRName", fullpath);
        } else if (last_filetype[1] == COMPUTER_KERNAL_IT_ROM_FILE) {
            resources_set_string("KernalITName", fullpath);
        } else if (last_filetype[1] == COMPUTER_KERNAL_NO_ROM_FILE) {
            resources_set_string("KernalNOName", fullpath);
        } else if (last_filetype[1] == COMPUTER_KERNAL_SE_ROM_FILE) {
            resources_set_string("KernalSEName", fullpath);
        } else if (last_filetype[1] == COMPUTER_KERNAL_CH_ROM_FILE) {
            resources_set_string("KernalCHName", fullpath);
        } else if (last_filetype[1] == COMPUTER_KERNAL_64_ROM_FILE) {
            resources_set_string("Kernal64Name", fullpath);
        } else if (last_filetype[1] == COMPUTER_BASIC_LO_ROM_FILE) {
            resources_set_string("BasicLoName", fullpath);
        } else if (last_filetype[1] == COMPUTER_BASIC_HI_ROM_FILE) {
            resources_set_string("BasicHiName", fullpath);
        } else if (last_filetype[1] == COMPUTER_BASIC_64_ROM_FILE) {
            resources_set_string("Basic64Name", fullpath);
        } else if (last_filetype[1] == COMPUTER_CHARGEN_INT_ROM_FILE) {
            resources_set_string("ChargenIntName", fullpath);
        } else if (last_filetype[1] == COMPUTER_CHARGEN_DE_ROM_FILE) {
            resources_set_string("ChargenDEName", fullpath);
        } else if (last_filetype[1] == COMPUTER_CHARGEN_FR_ROM_FILE) {
            resources_set_string("ChargenFRName", fullpath);
        } else if (last_filetype[1] == COMPUTER_CHARGEN_SE_ROM_FILE) {
            resources_set_string("ChargenSEName", fullpath);
        } else if (last_filetype[1] == COMPUTER_CHARGEN_CH_ROM_FILE) {
            resources_set_string("ChargenCHName", fullpath);
        } else if (last_filetype[1] == COMPUTER_CHARGEN_ROM_FILE) {
            resources_set_string("ChargenName", fullpath);
        } else if (last_filetype[1] == COMPUTER_EDITOR_ROM_FILE) {
            resources_set_string("EditorName", fullpath);
        } else if (last_filetype[1] == COMPUTER_ROM_9_FILE) {
            resources_set_string("RomModule9Name", fullpath);
        } else if (last_filetype[1] == COMPUTER_ROM_A_FILE) {
            resources_set_string("RomModuleAName", fullpath);
        } else if (last_filetype[1] == COMPUTER_ROM_B_FILE) {
            resources_set_string("RomModuleBName", fullpath);
        } else if (last_filetype[1] == COMPUTER_FUNCTION_LO_ROM_FILE) {
            resources_set_string("FunctionLowName", fullpath);
        } else if (last_filetype[1] == COMPUTER_FUNCTION_HI_ROM_FILE) {
            resources_set_string("FunctionHighName", fullpath);
        } else if (last_filetype[1] == DRIVE_1540_ROM_FILE) {
            resources_set_string("DosName1540", fullpath);
        } else if (last_filetype[1] == DRIVE_1541_ROM_FILE) {
            resources_set_string("DosName1541", fullpath);
        } else if (last_filetype[1] == DRIVE_1541II_ROM_FILE) {
            resources_set_string("DosName1541ii", fullpath);
        } else if (last_filetype[1] == DRIVE_1551_ROM_FILE) {
            resources_set_string("DosName1551", fullpath);
        } else if (last_filetype[1] == DRIVE_1570_ROM_FILE) {
            resources_set_string("DosName1570", fullpath);
        } else if (last_filetype[1] == DRIVE_1571_ROM_FILE) {
            resources_set_string("DosName1571", fullpath);
        } else if (last_filetype[1] == DRIVE_1571CR_ROM_FILE) {
            resources_set_string("DosName1571cr", fullpath);
        } else if (last_filetype[1] == DRIVE_1581_ROM_FILE) {
            resources_set_string("DosName1581", fullpath);
        } else if (last_filetype[1] == DRIVE_2000_ROM_FILE) {
            resources_set_string("DosName2000", fullpath);
        } else if (last_filetype[1] == DRIVE_4000_ROM_FILE) {
            resources_set_string("DosName4000", fullpath);
        } else if (last_filetype[1] == DRIVE_2031_ROM_FILE) {
            resources_set_string("DosName2031", fullpath);
        } else if (last_filetype[1] == DRIVE_2040_ROM_FILE) {
            resources_set_string("DosName2040", fullpath);
        } else if (last_filetype[1] == DRIVE_3040_ROM_FILE) {
            resources_set_string("DosName3040", fullpath);
        } else if (last_filetype[1] == DRIVE_4040_ROM_FILE) {
            resources_set_string("DosName4040", fullpath);
        } else if (last_filetype[1] == DRIVE_1001_ROM_FILE) {
            resources_set_string("DosName1001", fullpath);
        } else if (last_filetype[1] == DRIVE_PROFDOS_ROM_FILE) {
            resources_set_string("DriveProfDOS1571Name", fullpath);
        } else if (last_filetype[1] == DRIVE_SUPERCARD_ROM_FILE) {
            resources_set_string("DriveSuperCardName", fullpath);
        }
        delete path;
        delete fullpath;
    }
}
