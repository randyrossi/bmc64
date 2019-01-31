/*
 * ui_pet.cc - PET settings
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
 
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <RadioButton.h>
#include <string.h>
#include <Window.h>

extern "C" {
#include "pets.h"
#include "resources.h"
#include "ui.h"
#include "ui_pet.h"
#include "vsync.h"
}

static int pet_memory[] = { 4, 8, 16, 32, 96, 128, 0 };
static const char *video_text[] = { "Auto", "40 columns", "80 columns", NULL };
static int video_res[] = { 0, 40, 80 };
static const char *iosize_text[] = { "256 Byte", "2 KByte", NULL };
static int iosize_res[] = { 0x100, 0x800 };
static const char *keyboard_text[] = { "Graphics", "Business (UK)", NULL };
static int keyboard_res[] = { 2, 0 };

class PetWindow : public BWindow {
    public:
        PetWindow();
        ~PetWindow();
        virtual void MessageReceived(BMessage *msg);
};

static PetWindow *petwindow = NULL;

PetWindow::PetWindow() 
    : BWindow(BRect(50, 50, 420, 400), "PET settings", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) 
{
    BView *background;
    BRect r;
    BBox *box;
    BButton *button;
    BRadioButton *radiobutton;
    BCheckBox *checkbox;
    BMessage *msg;
    int res, i;
    char str[8];

    r = Bounds();
    background = new BView(r, "backview", B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);

    /* memory */
    r = BRect(10, 5, 140, 170);
    box = new BBox(r, "Memory");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("Memory");
    background->AddChild(box);

    resources_get_int("RamSize", &res);
    for (i = 0; pet_memory[i]; i++) {
        msg = new BMessage(MESSAGE_PET_MEMORY);
        msg->AddInt32("memory", i);
        sprintf(str, "%dKB", pet_memory[i]);
        radiobutton = new BRadioButton(BRect(10, 15 + i * 25, 70, 25 + i * 25), str, str, msg);
        radiobutton->SetValue(res == pet_memory[i]);
        box->AddChild(radiobutton);
    }

    /* video */
    r = BRect(10, 180, 140, 270);
    box = new BBox(r, "Video");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("Video");
    background->AddChild(box);

    resources_get_int("VideoSize", &res);
    for (i = 0; video_text[i]; i++) {
        msg = new BMessage(MESSAGE_PET_VIDEO);
        msg->AddInt32("video", i);
        radiobutton = new BRadioButton(BRect(10, 15 + i * 25, 70, 25 + i * 25), video_text[i], video_text[i], msg);
        radiobutton->SetValue(res == video_res[i]);
        box->AddChild(radiobutton);
    }

    /* io size */
    r = BRect(10, 280, 140, 345);
    box = new BBox(r, "IO Size");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("IO Size");
    background->AddChild(box);

    resources_get_int("IOSize", &res);
    for (i = 0; iosize_text[i]; i++) {
        msg = new BMessage(MESSAGE_PET_IOSIZE);
        msg->AddInt32("iosize", i);
        radiobutton = new BRadioButton(BRect(10, 15 + i * 25, 70, 25 + i * 25), iosize_text[i], iosize_text[i], msg);
        radiobutton->SetValue(res == iosize_res[i]);
        box->AddChild(radiobutton);
    }

    /* keyboard */
    r = BRect(150, 5, 360, 70);
    box = new BBox(r, "Keyboard");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("Keyboard");
    background->AddChild(box);

    resources_get_int("KeymapIndex", &res);
    for (i = 0; keyboard_text[i]; i++) {
        msg = new BMessage(MESSAGE_PET_KEYBOARD);
        msg->AddInt32("keyboard", i);
        radiobutton = new BRadioButton(BRect(10, 15 + i * 25, 140, 25 + i * 25), keyboard_text[i], keyboard_text[i], msg);
        radiobutton->SetValue(res == keyboard_res[i]);
        box->AddChild(radiobutton);
    }

    /* crtc */
    r = BRect(150, 80, 360, 120);
    box = new BBox(r, "CRTC");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("CRTC");
    background->AddChild(box);

    resources_get_int("Crtc", &res);
    checkbox = new BCheckBox(BRect(10, 15, 140, 25), "CRTC chip enabled", "CRTC chip enabled", new BMessage(MESSAGE_PET_CRTC));
    checkbox->SetValue(res);
    box->AddChild(checkbox);

    /* superpet */
    r = BRect(150, 130, 360, 170);
    box = new BBox(r, "SuperPET");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("SuperPET");
    background->AddChild(box);

    resources_get_int("SuperPET", &res);
    checkbox = new BCheckBox(BRect(10, 15, 155, 25), "IO enable (disables 8x96)", "IO enable (disables 8x96)", new BMessage(MESSAGE_PET_SUPERPET));
    checkbox->SetValue(res);
    box->AddChild(checkbox);

    /* 8296 PET */
    r = BRect(150, 180, 360, 245);
    box = new BBox(r, "8296 PET");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("8296 PET");
    background->AddChild(box);

    resources_get_int("Ram9", &res);
    checkbox = new BCheckBox(BRect(10, 15, 155, 25), "$9*** as RAM", "$9*** as RAM", new BMessage(MESSAGE_PET_RAM9));
    checkbox->SetValue(res);
    box->AddChild(checkbox);

    resources_get_int("RamA", &res);
    checkbox = new BCheckBox(BRect(10, 40, 155, 50), "$A*** as RAM", "$A*** as RAM", new BMessage(MESSAGE_PET_RAMA));
    checkbox->SetValue(res);
    box->AddChild(checkbox);

    Show();
}

PetWindow::~PetWindow() 
{
    petwindow = NULL;
}

void PetWindow::MessageReceived(BMessage *msg)
{
    int32 res_value;

    switch (msg->what) {
        case MESSAGE_PET_MEMORY:
            msg->FindInt32("memory", &res_value);
            resources_set_int("RamSize", pet_memory[res_value]);
            break;
        case MESSAGE_PET_VIDEO:
            msg->FindInt32("video", &res_value);
            resources_set_int("VideoSize", video_res[res_value]);
            break;
        case MESSAGE_PET_IOSIZE:
            msg->FindInt32("iosize", &res_value);
            resources_set_int("IOSize", iosize_res[res_value]);
            break;
        case MESSAGE_PET_KEYBOARD:
            msg->FindInt32("keyboard", &res_value);
            resources_set_int("KeymapIndex", keyboard_res[res_value]);
            break;
        case MESSAGE_PET_CRTC:
            resources_toggle("Crtc", (int *)&res_value);
            break;
        case MESSAGE_PET_SUPERPET:
            resources_toggle("SuperPET", (int *)&res_value);
            break;
        case MESSAGE_PET_RAM9:
            resources_toggle("Ram9", (int *) &res_value);
            break;
        case MESSAGE_PET_RAMA:
            resources_toggle("RamA", (int *) &res_value);
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_pet()
{
    thread_id petthread;
    status_t exit_value;

    if (petwindow != NULL) {
        return;
    }

    petwindow = new PetWindow;

    vsync_suspend_speed_eval();

    /* wait until window closed */
    petthread=petwindow->Thread();
    wait_for_thread(petthread, &exit_value);
}
