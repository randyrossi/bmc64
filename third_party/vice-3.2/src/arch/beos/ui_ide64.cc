/*
 * ui_ide64.cc - IDE64 settings
 *
 * Written by
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
 
#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <stdlib.h>
#include <string.h>
#include <TextControl.h>
#include <Window.h>

extern "C" {
#include "resources.h"
#include "ui.h"
#include "ui_ide64.h"
#include "vsync.h"
}

class IDE64Window : public BWindow {
    public:
        IDE64Window(int img_num);
        ~IDE64Window();
        virtual void MessageReceived(BMessage *msg);
    private:
        BBox *customsizebox;
        BTextControl *cylinderstextcontrol;
        BTextControl *headstextcontrol;
        BTextControl *sectorstextcontrol;

        void EnableSizeControls(int enable);
};

static IDE64Window *ide64window = NULL;

void IDE64Window::EnableSizeControls(int enable)
{
    int32 children, i;

    children = customsizebox->CountChildren();
    for (i = 0; i < children; i++) {
        ((BControl *)customsizebox->ChildAt(i))->SetEnabled(enable);
    }

}

IDE64Window::IDE64Window(int img_num) 
    : BWindow(BRect(50, 50, 340, 235), "IDE64 size settings", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) 
{
    int cylinders, heads, sectors, autodetect;
    char temp_str[40];
    const char *img_str = NULL;
    BView *background;
    BCheckBox *checkbox;
    BMessage *msg;
    BRect r;

    switch (img_num) {
        case 1:
            img_str = "primary master";
            break;
        case 2:
            img_str = "primary slave";
            break;
        case 3:
            img_str = "secondary master";
            break;
        case 4:
            img_str = "secondary slave";
            break;
    }
    sprintf(temp_str, "IDE64 %s HD size settings", img_str);
    SetTitle(temp_str);

    r = Bounds();
    background = new BView(r, "backview", B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);

    r.bottom = 30;
    r.InsetBy(10, 5);
    msg = new BMessage(MESSAGE_IDE64_AUTODETECT);
    msg->AddInt32("img_num", img_num);
    checkbox = new BCheckBox(r, "autodetect", "Autodetect image size", msg);
    background->AddChild(checkbox);
    resources_get_int_sprintf("IDE64AutodetectSize%d", &autodetect, img_num);
    checkbox->SetValue(autodetect);

    r = Bounds();
    r.top = 20;
    r.right -= 110;
    r.InsetBy(10, 10);
    customsizebox = new BBox(r, "Custom size");
    customsizebox->SetLabel("Custom size");
    background->AddChild(customsizebox);

    resources_get_int_sprintf("IDE64Cylinders%d", &cylinders, img_num);
    resources_get_int_sprintf("IDE64Heads%d", &heads, img_num);
    resources_get_int_sprintf("IDE64Sectors%d", &sectors, img_num);

    r = customsizebox->Bounds();
    r.bottom = 60;
    r.InsetBy(10, 20);
    sprintf(temp_str, "%d", cylinders);
    cylinderstextcontrol = new BTextControl(r, "Cylinders", "Cylinders", temp_str, NULL);
    cylinderstextcontrol->SetDivider(60);
    customsizebox->AddChild(cylinderstextcontrol);

    r.OffsetBy(0, 30);
    sprintf(temp_str, "%d", heads);
    headstextcontrol = new BTextControl(r, "Heads", "Heads", temp_str, NULL);
    headstextcontrol->SetDivider(60);
    customsizebox->AddChild(headstextcontrol);

    r.OffsetBy(0, 30);
    sprintf(temp_str, "%d", sectors);
    sectorstextcontrol = new BTextControl(r, "Sectors", "Sectors", temp_str, NULL);
    sectorstextcontrol->SetDivider(60);
    customsizebox->AddChild(sectorstextcontrol);

    msg = new BMessage(MESSAGE_IDE64_APPLY);
    msg->AddInt32("img_num", img_num);
    customsizebox->AddChild(new BButton(BRect(10, 110, 150, 135), "Apply", "Apply custom size", msg));

    EnableSizeControls(!autodetect);
    Show();
}

IDE64Window::~IDE64Window() 
{
    ide64window = NULL;       
}

void IDE64Window::MessageReceived(BMessage *msg)
{
    int cylinders, heads, sectors;
    int temp;
    int32 img_num;

    switch (msg->what) {
        case MESSAGE_IDE64_AUTODETECT:
            msg->FindInt32("img_num", &img_num);
            resources_get_int_sprintf("IDE64AutodetectSize%d", &temp, img_num);
            resources_set_int_sprintf("IDE64AutodetectSize%d", !temp, img_num);
            EnableSizeControls(temp);
            break;
        case MESSAGE_IDE64_APPLY:
            msg->FindInt32("img_num", &img_num);
            temp = atoi(cylinderstextcontrol->Text());
            if (temp < 1) {
                cylinders = 1;
            } else if (temp > 65535) {
                cylinders = 65535;
            } else {
                cylinders = temp;
            }

            if (temp != cylinders) {
                ui_error("Value for IDE64 cylinders was invalid, using %d instead.", cylinders);
            }
            resources_set_int_sprintf("IDE64Cylinders%d", cylinders, img_num);

            temp = atoi(headstextcontrol->Text());
            if (temp < 1) {
                heads = 1;
            } else if (temp > 16) {
                heads = 16;
            } else {
                heads = temp;
            }

            if (temp != heads) {
                ui_error("Value for IDE64 heads was invalid, using %d instead.", heads);
            }
            resources_set_int_sprintf("IDE64Heads%d", heads, img_num);

            temp = atoi(sectorstextcontrol->Text());
            if (temp < 1) {
                sectors = 1;
            } else if (temp > 63) {
                sectors = 63;
            } else {
                sectors = temp;
            }

            if (temp != sectors) {
                ui_error("Value for IDE64 sectors was invalid, using %d instead.", sectors);
            }
            resources_set_int_sprintf("IDE64Sectors%d", sectors, img_num);
            BWindow::Quit();
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_ide64(int img_num)
{
    thread_id ide64thread;
    status_t exit_value;

    if (ide64window != NULL) {
        return;
    }

    ide64window = new IDE64Window(img_num);

    vsync_suspend_speed_eval();

    /* wait until window closed */
    ide64thread = ide64window->Thread();
    wait_for_thread(ide64thread, &exit_value);
}
