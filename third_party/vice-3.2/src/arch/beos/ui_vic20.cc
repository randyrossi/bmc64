/*
 * ui_vic20.cc - VIC20 settings
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
#include <CheckBox.h>
#include <RadioButton.h>
#include <string.h>
#include <Window.h>

extern "C" { 
#include "resources.h"
#include "ui.h"
#include "ui_vic20.h"
#include "vsync.h"
}

enum {
    BLOCK_0 = 1,
    BLOCK_1 = 1 << 1,
    BLOCK_2 = 1 << 2,
    BLOCK_3 = 1 << 3,
    BLOCK_5 = 1 << 5
};

static int config_number[] = {
    0,
    BLOCK_0,
    BLOCK_1,
    BLOCK_1 | BLOCK_2,
    BLOCK_1 | BLOCK_2 | BLOCK_3,
    BLOCK_0 | BLOCK_1 | BLOCK_2 | BLOCK_3 | BLOCK_5,
    -1
};

static const char *config_text[] = {
    "no expansion",
    "3K (block 0)",
    "8K (block 1)",
    "16K (block 1+2)",
    "24K (block 1+2+3)",
    "full (block 0+1+2+3+5)",
    "custom"
};

static const char *block_text[] = {
    "Block 0",
    "Block 1",
    "Block 2",
    "Block 3",
    "Block 5"
};

static const char *block_resource[] = {
    "RAMBlock0",
    "RAMBlock1",
    "RAMBlock2",
    "RAMBlock3",
    "RAMBlock5"
};

class Vic20Window : public BWindow {
    public:
        Vic20Window();
        ~Vic20Window();
        virtual void MessageReceived(BMessage *msg);
        void UpdateConfig(void);
        void UpdateBlocks(int config_nr);
    private:
        BCheckBox *CBblock[5];
        BRadioButton *RBconfig[7];
};

static int block[5];
static Vic20Window *vic20window = NULL;

Vic20Window::Vic20Window() 
    : BWindow(BRect(50, 50, 350, 250), "VIC20 settings", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) 
{
    BView *background;
    BRect r;
    BBox *box;
    BMessage *msg;
    int i;

    r = Bounds();
    background = new BView(r, "backview", B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);

    for (i = 0; i < 5; i++) {
        resources_get_int(block_resource[i], &block[i]);
    }

    r = Bounds();
    r.InsetBy(10, 5);
    box = new BBox(r, "Memory configuration");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("Memory configuration");

    /* Common memory settings */
    for (i = 0; i < 7; i++) {
        msg = new BMessage(MESSAGE_VIC20_MEMCONFIG);
        msg->AddInt32("config", config_number[i]);
        RBconfig[i] = new BRadioButton(BRect(10, 20 + i * 20, 180, 35 + i * 20), config_text[i], config_text[i], msg);
        box->AddChild(RBconfig[i]);
    }

    /* memory blocks */
    for (i = 0; i < 5; i++) {
        msg = new BMessage(MESSAGE_VIC20_MEMBLOCK);
        msg->AddInt32("block", i);
        CBblock[i] = new BCheckBox(BRect(180, 20 + i * 20, 250, 35 + i * 20), block_text[i], block_text[i], msg);
        CBblock[i]->SetValue(block[i]);
        box->AddChild(CBblock[i]);
    }

    background->AddChild(box);				    
    UpdateConfig();

    Show();
}

Vic20Window::~Vic20Window() 
{
    vic20window = NULL;	
}

void Vic20Window::MessageReceived(BMessage *msg)
{
    int32 res_value, block_nr;

    switch (msg->what) {
        case MESSAGE_VIC20_MEMCONFIG:
            msg->FindInt32("config", &res_value);
            UpdateBlocks(res_value);
            break;
        case MESSAGE_VIC20_MEMBLOCK:
            msg->FindInt32("block", &block_nr);
            resources_toggle(block_resource[block_nr], (int *)&res_value);
            UpdateConfig();
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void Vic20Window::UpdateConfig(void)
{
    int current_config;
    int config_not_found;
    int i;

    current_config = (CBblock[0]->Value() ? BLOCK_0 : 0) |
                     (CBblock[1]->Value() ? BLOCK_1 : 0) |
                     (CBblock[2]->Value() ? BLOCK_2 : 0) |
                     (CBblock[3]->Value() ? BLOCK_3 : 0) |
                     (CBblock[4]->Value() ? BLOCK_5 : 0);
    
    config_not_found = 1;
    i = 0;
    while (config_not_found && i < 6) {
        if (current_config == config_number[i]) {
            RBconfig[i]->SetValue(1);
            config_not_found = 0;
        }
        i++;
    }
    if (config_not_found) {
        RBconfig[6]->SetValue(1);
    }
}

void Vic20Window::UpdateBlocks(int config_nr)
{
    int i;

    if (config_nr < 0) {
        return;
    }

    i = (config_nr & BLOCK_0) ? 1 : 0;
    CBblock[0]->SetValue(i);
    resources_set_int(block_resource[0], i);
    i = (config_nr & BLOCK_1) ? 1 : 0;
    CBblock[1]->SetValue(i);
    resources_set_int(block_resource[1], i);
    i = (config_nr & BLOCK_2) ? 1 : 0;
    CBblock[2]->SetValue(i);
    resources_set_int(block_resource[2], i);
    i = (config_nr & BLOCK_3) ? 1 : 0;
    CBblock[3]->SetValue(i);
    resources_set_int(block_resource[3], i);
    i = (config_nr & BLOCK_5) ? 1 : 0;
    CBblock[4]->SetValue(i);
    resources_set_int(block_resource[4], i);
}

void ui_vic20()
{
    thread_id vic20thread;
    status_t exit_value;

    if (vic20window != NULL) {
        return;
    }

    vic20window = new Vic20Window;

    vsync_suspend_speed_eval();

    /* wait until window closed */
    vic20thread = vic20window->Thread();
    wait_for_thread(vic20thread, &exit_value);
}
