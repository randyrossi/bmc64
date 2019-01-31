/*
 * ui_sidcart.cc - SID cart settings
 *
 * Written by
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
#include <OptionPopUp.h>
#include <RadioButton.h>
#include <Slider.h>
#include <string.h>
#include <Window.h>

#include <stdlib.h>

extern "C" {
#include "constants.h"
#include "machine.h"
#include "resources.h"
#include "sid.h"
#include "ui.h"
#include "ui_sidcart.h"
#include "util.h"
#include "vsync.h"
}

static const char *samplingmode[] = {
    "fast sampling",
    "interpolating",
    "resampling",
    "fast resampling",
    NULL
};

typedef struct slider_res_s {
    const char *name;
    const char *res_name;
    int min_val;
    int max_val;
} slider_res_t;

static slider_res_t sliders[] = {
    { "Passband", "SidResidPassband", 0, 90 },
    { "Gain", "SidResidGain", 90, 100 },
    { "Bias", "SidResidFilterBias", -5000, 5000 },
    { NULL, NULL, 0, 0 }
};

static sid_engine_model_t **sid_engine_model_list;

static const char **sidaddresstextpair;
static const char **sidclockpair;
static const int *sidaddressintpair;

class SidCartWindow : public BWindow {
        BOptionPopUp *engine_model_popup;
        BBox *addressbox;
        BBox *clockbox;
        BBox *residbox;

        void EnableBoxedControls(BBox *box, int enable);
        void EnableReSidControls(int engine);
        void EnableControls();
    public:
        SidCartWindow();
        ~SidCartWindow();
        virtual void MessageReceived(BMessage *msg);
};

static SidCartWindow *sidcartwindow = NULL;

void SidCartWindow::EnableBoxedControls(BBox *box, int enable)
{
    int32 children, i;

    children = box->CountChildren();
    for (i = 0; i < children; i++) {
        ((BControl *)box->ChildAt(i))->SetEnabled(enable);
    }

}

void SidCartWindow::EnableReSidControls(int engine)
{
    int enable;

    enable = ((BControl *)FindView("SidCart"))->Value();
    EnableBoxedControls(residbox, enable && (engine == SID_ENGINE_RESID));
}

void SidCartWindow::EnableControls()
{
    int enable, engine;

    enable = ((BControl *)FindView("SidCart"))->Value();
    engine_model_popup->SetEnabled(enable);
    ((BControl *)FindView("SidFilters"))->SetEnabled(enable);
    EnableBoxedControls(addressbox, enable);
    EnableBoxedControls(clockbox, enable);
    if (machine_class == VICE_MACHINE_PLUS4) {
        ((BControl *)FindView("DIGIBLASTER"))->SetEnabled(enable);
    }

    resources_get_int("SidEngine", &engine);
    EnableReSidControls(engine);
}

SidCartWindow::SidCartWindow() 
    : BWindow(BRect(250, 50, 580, 450), "SID cartridge settings", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) 
{
    BMessage *msg;
    BCheckBox *checkbox;
    BRect r;
    BRadioButton *radiobutton;
    BSlider *slider;
    BView *background;
    char st[12], st2[12];
    int engine, res_val, i;

    r = Bounds();
    background = new BView(r, NULL,  B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);

    /* SID cart emulation */
    resources_get_int("SidCart", &res_val);
    checkbox = new BCheckBox(BRect(10, 10, 240, 25), "SidCart", "Enable SID cartridge", new BMessage(MESSAGE_SIDCART_ENABLE));
    checkbox->SetValue(res_val);
    background->AddChild(checkbox);

    /* SID model */
    sid_engine_model_list = sid_get_engine_model_list();
    resources_get_int("SidModel", &i);
    resources_get_int("SidEngine", &engine);
    res_val = engine << 8;
    res_val |= i;
    engine_model_popup = new BOptionPopUp(BRect(10, 35, 330, 59), "SID Engine/Model", "SID Engine/Model", new BMessage(MESSAGE_SIDCART_MODEL));
    for (i = 0; sid_engine_model_list[i] != NULL; i++) {
        engine_model_popup->AddOption(sid_engine_model_list[i]->name, sid_engine_model_list[i]->value);
    }
    engine_model_popup->SelectOptionFor(res_val);
    background->AddChild(engine_model_popup);

    /* SID filter */
    resources_get_int("SidFilters", &res_val);
    checkbox = new BCheckBox(BRect(10, 60, 240, 75), "SidFilters", "SID filters", new BMessage(MESSAGE_SIDCART_FILTERS));
    checkbox->SetValue(res_val);
    background->AddChild(checkbox);

    /* SID address */
    addressbox = new BBox(BRect(10, 85, 160, 145), "SID address");
    addressbox->SetViewColor(220, 220, 220, 0);
    addressbox->SetLabel("SID address");
    background->AddChild(addressbox);

    resources_get_int("SidAddress", &res_val);
    for (i = 0; i < 2; i++) {
        msg = new BMessage(MESSAGE_SIDCART_ADDRESS);
        msg->AddInt32("address", i);
        radiobutton = new BRadioButton(BRect(10, 15 + i * 20, 140, 30 + i * 20), sidaddresstextpair[i], sidaddresstextpair[i], msg);
        radiobutton->SetValue(res_val == sidaddressintpair[i]);
        addressbox->AddChild(radiobutton);
    }

    /* SID clock */
    clockbox = new BBox(BRect(170, 85, 320, 145), "SID clock");
    clockbox->SetViewColor(220, 220, 220, 0);
    clockbox->SetLabel("SID clock");
    background->AddChild(clockbox);

    resources_get_int("SidClock", &res_val);
    for (i = 0; i < 2; i++) {
        msg = new BMessage(MESSAGE_SIDCART_CLOCK);
        msg->AddInt32("clock", i);
        radiobutton = new BRadioButton(BRect(10, 15 + i * 20, 140, 30 + i * 20), sidclockpair[i], sidclockpair[i], msg);
        radiobutton->SetValue(res_val == i);
        clockbox->AddChild(radiobutton);
    }

    /* DIGIBLASTER */
    if (machine_class == VICE_MACHINE_PLUS4) {
        resources_get_int("DIGIBLASTER", &res_val);
        checkbox = new BCheckBox(BRect(10, 155, 240, 170), "DIGIBLASTER", "Enable digiblaster add-on", new BMessage(MESSAGE_SIDCART_DIGIBLASTER));
        checkbox->SetValue(res_val);
        background->AddChild(checkbox);
    } else {
        ResizeTo(330, 375);
    }

    /* reSID settings */
    r = Bounds();
    r.InsetBy(10, 10);
    r.top = r.bottom - 200;
    residbox = new BBox(r, "reSID settings");
    //~ residbox->SetViewColor(220, 220, 220, 0);
    residbox->SetLabel("reSID settings");
    background->AddChild(residbox);
    r = residbox->Bounds();

    /* sampling method */
    resources_get_int("SidResidSampling", &res_val);
    for (i = 0; samplingmode[i] != NULL; i++) {
        msg = new BMessage(MESSAGE_SIDCART_RESIDSAMPLING);
        msg->AddInt32("mode", i);
        radiobutton = new BRadioButton(BRect(10, 15 + i * 25, r.Width() / 2, 30 + i * 25), samplingmode[i], samplingmode[i], msg);
        radiobutton->SetValue(res_val == i);
        residbox->AddChild(radiobutton);
    }

    r.left = r.Width() / 2 - 10;
    r.bottom = 80;
    r.InsetBy(10, 15);
    for (i = 0; sliders[i].name; i++) {
        resources_get_int(sliders[i].res_name, &res_val);
        msg = new BMessage(MESSAGE_SIDCART_RESIDSLIDER);
        slider = new BSlider(r, sliders[i].res_name, sliders[i].name, msg, sliders[i].min_val, sliders[i].max_val, B_TRIANGLE_THUMB);
        slider->SetValue(res_val);
        slider->SetHashMarkCount(11);
        slider->SetHashMarks(B_HASH_MARKS_BOTTOM);
        sprintf(st, "%d", sliders[i].min_val);
        sprintf(st2, "%d", sliders[i].max_val);
        slider->SetLimitLabels(st, st2);
        residbox->AddChild(slider);
        //~ DBG_RECT((sliders[i].name, slider->Frame()));
        r.OffsetBy(0, 50);
    }

    EnableControls();

    Show();
}

SidCartWindow::~SidCartWindow() 
{
    sidcartwindow = NULL;
}

void SidCartWindow::MessageReceived(BMessage *msg)
{
    BSlider *slider;
    int32 engine, val;

    switch (msg->what) {
        case MESSAGE_SIDCART_ENABLE:
            resources_toggle("SidCart", (int *)&val);
            EnableControls();
            break;
        case MESSAGE_SIDCART_MODEL:
            val = engine_model_popup->Value();
            engine = val >> 8;
            val &= 0xff;
            sid_set_engine_model(engine, val);
            EnableReSidControls(engine);
            break;
        case MESSAGE_SIDCART_FILTERS:
            resources_toggle("SidFilters", (int *)&val);
            break;
        case MESSAGE_SIDCART_ADDRESS:
            val = msg->FindInt32("address");
            resources_set_int("SidAddress",  sidaddressintpair[val]);
            break;
        case MESSAGE_SIDCART_CLOCK:
            val = msg->FindInt32("clock");
            resources_set_int("SidClock", (int)val);
            break;
        case MESSAGE_SIDCART_DIGIBLASTER:
            resources_toggle("DIGIBLASTER", (int *)&val);
            break;
        case MESSAGE_SIDCART_RESIDSAMPLING:
            val = msg->FindInt32("mode");
            resources_set_int("SidResidSampling", val);
            break;
        case MESSAGE_SIDCART_RESIDSLIDER:
            msg->FindPointer("source", (void **)&slider);
            if (slider) {
                resources_set_int(slider->Name(), slider->Value());
            }
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_sidcart(const char **cartaddresstextpair, const char **cartclockpair, const int *cartaddressintpair)
{
    thread_id sidcartthread;
    status_t exit_value;

    if (sidcartwindow != NULL) {
        return;
    }

    sidaddresstextpair = cartaddresstextpair;
    sidclockpair = cartclockpair;
    sidaddressintpair = cartaddressintpair;

    sidcartwindow = new SidCartWindow;

    vsync_suspend_speed_eval();

    /* wait until window closed */
    sidcartthread = sidcartwindow->Thread();
    wait_for_thread(sidcartthread, &exit_value);
}
