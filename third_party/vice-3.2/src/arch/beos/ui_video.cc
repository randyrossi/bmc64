/*
 * ui_video.cc - Video settings
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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
 
#include <Box.h>
#include <CheckBox.h>
#include <List.h>
#include <ListItem.h>
#include <ListView.h>
#include <RadioButton.h>
#include <ScrollView.h>
#include <Slider.h>
#include <String.h>
#include <TabView.h>
#include <Window.h>

#include <stdlib.h>
#include <string.h>

extern "C" { 
#include "constants.h"
#include "lib.h"
#include "machine.h"
#include "palette.h"
#include "resources.h"
#include "ui.h"
#include "ui_video.h"
#include "util.h"
#include "vsync.h"
}

typedef struct control_res_s {
    const char *name;
    const char *res_name;
    int multiplier;
} control_res_t;

static control_res_t color_controls[] = {
    { "Gamma", "ColorGamma", 4000 },
    { "Tint", "ColorTint", 2000 },
    { "Saturation", "ColorSaturation", 2000 },
    { "Contrast", "ColorContrast", 2000 },
    { "Brightness", "ColorBrightness", 2000 },
    { NULL, NULL, 0 }
};

static control_res_t crt_controls[] = {
    { "Scanline Shade", "PALScanLineShade", 1000 },
    { "Blur", "PALBlur", 1000 },
    { "Odd Lines Phase", "PALOddLinePhase", 2000 },
    { "Odd Lines Offset", "PALOddLineOffset", 2000 },
    { NULL, NULL, 0 }
};

static const char *chip_display_names[] = { "VIC-II", "VIC", "CRTC", "VDC", "TED" };

static const char *res_prefixes[] = { "VICII", "VIC", "Crtc", "VDC", "TED" };

static palette_info_t *palettelist;

class VideoView : public BView {
        BBox *color_ctrlsbox;
        BBox *crt_ctrlsbox;
        BListView *palettelistview;
        BList *palettelistfiles;
        const char *res_prefix;

        void CreateSliders(BBox *parent, control_res_t *ctrls);
        //~ void EnableSliders(BBox *parent);
    public:
        VideoView(BRect r, int chiptype);
        ~VideoView();
};

class VideoWindow : public BWindow {
    public:
        VideoWindow(int chip1type, int chip2type);
        ~VideoWindow();
        virtual void MessageReceived(BMessage *msg);
    private:
        //~ VideoView *vv[2]; /* pointers to the VideoViews */
};

static VideoWindow *videowindow = NULL;

void VideoView::CreateSliders(BBox *parent, control_res_t *ctrls)
{
    int res_val;
    char *resname;
    BMessage *msg;
    BSlider *slider;
    BRect r;

    r = parent->Bounds();
    r.bottom = 50;
    r.InsetBy(5, 15);
    for (int i = 0; ctrls[i].name; i++) {
        resname = util_concat(res_prefix, ctrls[i].res_name, NULL);
        if (resources_get_int(resname, &res_val) == 0) {
            slider = new BSlider(r.OffsetByCopy(0, i * 45), resname, ctrls[i].name, NULL, 0, ctrls[i].multiplier, B_TRIANGLE_THUMB);
            msg = new BMessage(MESSAGE_VIDEO_COLOR);
            msg->AddString("resname", resname);
            slider->SetMessage(msg);
            slider->SetValue(res_val);
            slider->SetHashMarkCount(11);
            slider->SetHashMarks(B_HASH_MARKS_BOTTOM);
            parent->AddChild(slider);
        }
        lib_free(resname);
    }
}

VideoView::VideoView(BRect r, int chiptype) : BView(r, "video_view", B_FOLLOW_NONE, B_WILL_DRAW)
{
    BMessage *msg;
    BCheckBox *checkbox;
    char *resname;
    const char *palettefile_const;
    char *palettefile;
    //~ BRadioButton *rb_mode;
    int res_val;

    BView::SetViewColor(220, 220, 220, 0);

    res_prefix = res_prefixes[chiptype];

    /* Sliders for color control */
    color_ctrlsbox = new BBox(BRect(10, 10, 120, 255), "Color controls");
    color_ctrlsbox->SetLabel("Color controls");
    CreateSliders(color_ctrlsbox, color_controls);
    AddChild(color_ctrlsbox);
    //~ color_ctrlsbox->SetViewColor(216, 216, 216, 0);

    /* Sliders for CRT Emulation control */
    crt_ctrlsbox = new BBox(BRect(130, 10, 240, 210), "CRT emulation");
    crt_ctrlsbox->SetLabel("CRT emulation");
    CreateSliders(crt_ctrlsbox, crt_controls);
    AddChild(crt_ctrlsbox);

    /* Audio Leak check box */
    resname = util_concat(res_prefix, "AudioLeak", NULL);
    msg = new BMessage(MESSAGE_VIDEO_AUDIO_LEAK);
    msg->AddString("resname", resname);
    checkbox = new BCheckBox(BRect(250, 210, 380, 225), "AudioLeak", "Audio Leak", msg);
    AddChild(checkbox);
    resources_get_int(resname, &res_val);
    checkbox->SetValue(res_val);
    lib_free(resname);

    /* External Palette check box */
    resname = util_concat(res_prefix, "ExternalPalette", NULL);
    msg = new BMessage(MESSAGE_VIDEO_EXTERNALPALETTE);
    msg->AddString("resname", resname);
    checkbox = new BCheckBox(BRect(250, 10, 380, 25), "ExternalPalette", "External Palette", msg);
    AddChild(checkbox);
    resources_get_int(resname, &res_val);
    checkbox->SetValue(res_val);
    lib_free(resname);

    /* External Palette File list box */
    resname = util_concat(res_prefix, "PaletteFile", NULL);
    palettelistfiles = new BList();
    msg = new BMessage(MESSAGE_VIDEO_PALETTEFILE);
    msg->AddString("resname", resname);
    msg->AddPointer("filelist", palettelistfiles);
    palettelistview = new BListView(BRect(250, 35, 360, 125), "PaletteFile");
    palettelistview->SetSelectionMessage(msg);
    AddChild(new BScrollView("scroll", palettelistview, B_FOLLOW_LEFT | B_FOLLOW_TOP, 0, false, true));

    /* Fill External Palette File list box */
    resources_get_string(resname, &palettefile_const);
    palettefile = lib_stralloc(palettefile_const);
    util_add_extension(&palettefile, "vpl");
    for (int i = 0; palettelist[i].name; i++) {
        BListItem *item;
        BString *bstr;

        if (palettelist[i].chip && !strcmp(palettelist[i].chip, res_prefix)) {
            palettelistview->AddItem(item = new BStringItem(palettelist[i].name));
            palettelistfiles->AddItem(bstr = new BString(palettelist[i].file));
            if (strncmp(bstr->String(), palettefile, bstr->Length()) == 0) {
                palettelistview->Select(palettelistview->IndexOf(item));
            }
        }
    }
    lib_free(palettefile);
    lib_free(resname);
}

VideoView::~VideoView()
{
    BStringItem *item;
    BString *bstr;
    int32 count;

    count = palettelistview->CountItems();
    while (count > 0) {
        item = (BStringItem *)palettelistview->RemoveItem(--count);
        delete item;
    }

    count = palettelistfiles->CountItems();
    while (count > 0) {
        bstr = (BString *)palettelistfiles->RemoveItem(--count);
        delete bstr;
    }

    delete palettelistfiles;
}

VideoWindow::VideoWindow(int chip1type, int chip2type)
    : BWindow(BRect(250, 50, 640, 375), "Video settings", B_TITLED_WINDOW, B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
{
    BRect frame;
    BTabView *tabview;
    BTab *tab;
    char str[24];
    int chip_type[2];
    int chip_no;

    frame = Bounds();
    tabview = new BTabView(frame, "tab_view");
    tabview->SetViewColor(220, 220, 220, 0);

    frame = tabview->Bounds();
    frame.InsetBy(5, 5);
    //~ frame.OffsetTo(3, 3);
    frame.bottom -= tabview->TabHeight();

    /* the video chips 1 & 2 */
    chip_type[0] = chip1type;
    chip_type[1] = chip2type;
    for (chip_no = 0; (chip_no < 2) && (chip_type[chip_no] >= 0); chip_no++) {
        tab = new BTab();
        tabview->AddTab(new VideoView(frame, chip_type[chip_no]), tab);
        //~ tabview->AddTab(vv[chip_no] = new VideoView(frame, chip_type[chip_no]), tab);
        sprintf(str, "%s Colors", chip_display_names[chip_type[chip_no]]);
        tab->SetLabel(str);
    }

    AddChild(tabview);
    tabview->SetTabWidth(B_WIDTH_FROM_WIDEST);
    Show();
}

VideoWindow::~VideoWindow()
{
    videowindow = NULL;
}

void VideoWindow::MessageReceived(BMessage *msg)
{
    const char *resname;
    BMessage *msr;
    BSlider *slider;
    BListView *listview;
    BList *filelist;
    BString *bstr;
    int32 val;

    msg->FindString("resname", &resname);

    switch (msg->what) {
        case MESSAGE_VIDEO_COLOR:
            msg->FindPointer("source", (void **)&slider);
            val = slider->Value();
            msr = new BMessage(MESSAGE_SET_RESOURCE);
            msr->AddString("resname", resname);
            msr->AddInt32("resval", val);
            ui_add_event((void*)msr);
            delete msr;
            break;
        case MESSAGE_VIDEO_EXTERNALPALETTE:
            resources_get_int(resname, (int *)&val);
            msr = new BMessage(MESSAGE_SET_RESOURCE);
            msr->AddString("resname", resname);
            msr->AddInt32("resval", 1 - val);
            ui_add_event((void*)msr);
            delete msr;
            break;
        case MESSAGE_VIDEO_AUDIO_LEAK:
            resources_get_int(resname, (int *)&val);
            msr = new BMessage(MESSAGE_SET_RESOURCE);
            msr->AddString("resname", resname);
            msr->AddInt32("resval", 1 - val);
            ui_add_event((void*)msr);
            delete msr;
            break;
        case MESSAGE_VIDEO_PALETTEFILE:
            msg->FindPointer("source", (void **)&listview);
            msg->FindPointer("filelist", (void **)&filelist);
            val = listview->CurrentSelection();
            bstr = (BString *)filelist->ItemAt(val);
            if (bstr) {
                msr = new BMessage(MESSAGE_SET_RESOURCE);
                msr->AddString("resname", resname);
                msr->AddString("resvalstr", bstr->String());
                ui_add_event((void*)msr);
                delete msr;
            }
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_video_two_chip(int chip1_type, int chip2_type)
{
    if (videowindow != NULL) {
        videowindow->Activate();
        return;
    }

    palettelist = palette_get_info_list();

    videowindow = new VideoWindow(chip1_type, chip2_type);
}

void ui_video(int chip_type)
{
    if (videowindow != NULL) {
        videowindow->Activate();
        return;
    }

    palettelist = palette_get_info_list();

    videowindow = new VideoWindow(chip_type, UI_VIDEO_CHIP_NONE);
}
