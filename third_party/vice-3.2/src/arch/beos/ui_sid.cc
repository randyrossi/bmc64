/*
 * ui_sid.cc - SID settings
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
#include <OptionPopUp.h>
#include <RadioButton.h>
#include <Slider.h>
#include <string.h>
#include <Window.h>

#include <stdlib.h>

extern "C" {
#include "constants.h"
#include "log.h"
#include "machine.h"
#include "resources.h"
#include "sid.h"
#include "ui.h"
#include "ui_sid.h"
#include "util.h"
#include "vsync.h"
}

/* #define DEBUG_UI */

#ifdef DEBUG_UI
void print_rect(const char *view, BRect r)
{
    log_debug("%s slider (Width: %.1f, Height: %.1f) (Top: %.1f, Bottom: %.1f)", view, r.Width(), r.Height(), r.top, r.bottom);
}
#define DBG_RECT(_x_) print_rect _x_
#define DBG_MSG(_x_) log_debug _x_
#else
#define DBG_RECT(_x_)
#define DBG_MSG(_x_)
#endif

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

static int *sidaddressbase;

class SidWindow : public BWindow {
        BOptionPopUp *engine_model_popup;
        BOptionPopUp *second_sid_popup;
        BOptionPopUp *third_sid_popup;
        BOptionPopUp *fourth_sid_popup;
        BBox *extrasidbox;
        BBox *residbox;

        BOptionPopUp *CreateAddressPopUp(BRect r, const char *label, const char *resource);
        void EnableReSidControls(int engine);
    public:
        SidWindow();
        ~SidWindow();
        virtual void MessageReceived(BMessage *msg);
};

static SidWindow *sidwindow = NULL;

BOptionPopUp *SidWindow::CreateAddressPopUp(BRect r, const char *label, const char *resource)
{
    BOptionPopUp *address_popup;
    char st[12];
    int adr, ladr, hi;
    int *hadr = sidaddressbase;

    address_popup = new BOptionPopUp(r, resource, label, new BMessage(MESSAGE_SID_ADDRESS));

    for (hi = 0; hadr[hi] >= 0; hi++) {
        for (ladr = (hi == 0 ? 0x20 : 0x0); ladr < 0x100; ladr += 0x20) {
            adr = hadr[hi] * 0x100 + ladr;
            sprintf(st, "$%04X", adr);

            address_popup->AddOption(st, adr);
        }
    }

    resources_get_int(resource, &adr);
    address_popup->SelectOptionFor(adr);
    extrasidbox->AddChild(address_popup);

    return address_popup;
}

void SidWindow::EnableReSidControls(int engine)
{
    int32 children, i;

    children = residbox->CountChildren();
    for (i = 0; i < children; i++) {
        ((BControl *)residbox->ChildAt(i))->SetEnabled(engine == SID_ENGINE_RESID);
    }

}

SidWindow::SidWindow() 
    : BWindow(BRect(250, 50, 580, 420), "SID settings", B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL, B_NOT_ZOOMABLE | B_NOT_RESIZABLE) 
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

    /* SID model */
    sid_engine_model_list = sid_get_engine_model_list();
    resources_get_int("SidModel", &i);
    resources_get_int("SidEngine", &engine);
    res_val = engine << 8;
    res_val |= i;
    engine_model_popup = new BOptionPopUp(BRect(10, 10, 330, 34), "SID Engine/Model", "SID Engine/Model", new BMessage(MESSAGE_SID_MODEL));
    for (i = 0; sid_engine_model_list[i] != NULL; i++) {
        engine_model_popup->AddOption(sid_engine_model_list[i]->name, sid_engine_model_list[i]->value);
    }
    engine_model_popup->SelectOptionFor(res_val);
    background->AddChild(engine_model_popup);

    /* SID filter */
    resources_get_int("SidFilters", &res_val);
    checkbox = new BCheckBox(BRect(10, 35, 120, 50), "SID Filters", "SID Filters", new BMessage(MESSAGE_SID_FILTERS));
    checkbox->SetValue(res_val);
    background->AddChild(checkbox);

    if (sidaddressbase) {
        extrasidbox = new BBox(BRect(10, 60, 320, 140), "Extra SID chips");
        extrasidbox->SetViewColor(220, 220, 220, 0);
        extrasidbox->SetLabel("Extra SID chips");
        background->AddChild(extrasidbox);

        /* Extra SIDs */
        resources_get_int("SidStereo", &res_val);
        for (i = 0; i < 4; i++) {
            msg = new BMessage(MESSAGE_SID_STEREO);
            msg->AddInt32("sids", i);
            sprintf(st, "%d", i);
            radiobutton = new BRadioButton(BRect(10, 15 + i * 20, 50, 30 + i * 20), st, st, msg);
            radiobutton->SetValue(res_val == i);
            extrasidbox->AddChild(radiobutton);
        }

        /* Extra SID addresses */
        r = extrasidbox->Bounds();
        r.InsetBy(10, 0);
        r.left = 60;
        r.bottom = 25;
        second_sid_popup = CreateAddressPopUp(BRect(60, 15, 220, 40), "Second SID at", "SidStereoAddressStart");
        second_sid_popup->SetEnabled(res_val >= 1);
        third_sid_popup = CreateAddressPopUp(BRect(60, 45, 220, 70), "Third SID at", "SidTripleAddressStart");
        third_sid_popup->SetEnabled(res_val >= 2);
        fourth_sid_popup = CreateAddressPopUp(BRect(60, 45, 220, 70), "Fourth SID at", "SidQuadAddressStart");
        fourth_sid_popup->SetEnabled(res_val >= 2);
    } else {
        extrasidbox = NULL;
        second_sid_popup = NULL;
        third_sid_popup = NULL;
        fourth_sid_popup = NULL;
        ResizeTo(330, 270);
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
        msg = new BMessage(MESSAGE_SID_RESIDSAMPLING);
        msg->AddInt32("mode", i);
        radiobutton = new BRadioButton(BRect(10, 15 + i * 25, r.Width() / 2, 30 + i * 25), samplingmode[i], samplingmode[i], msg);
        radiobutton->SetValue(res_val == i);
        residbox->AddChild(radiobutton);
    }

    //~ r = residbox->Bounds();
    //~ r = BRect(r.Width() / 2, 15, r.Width() - 10, 65);
    r.left = r.Width() / 2 - 10;
    r.bottom = 80;
    r.InsetBy(10, 15);
    for (i = 0; sliders[i].name; i++) {
        resources_get_int(sliders[i].res_name, &res_val);
        msg = new BMessage(MESSAGE_SID_RESIDSLIDER);
        slider = new BSlider(r, sliders[i].res_name, sliders[i].name, msg, sliders[i].min_val, sliders[i].max_val, B_TRIANGLE_THUMB);
        slider->SetValue(res_val);
        slider->SetHashMarkCount(11);
        slider->SetHashMarks(B_HASH_MARKS_BOTTOM);
        sprintf(st, "%d", sliders[i].min_val);
        sprintf(st2, "%d", sliders[i].max_val);
        slider->SetLimitLabels(st, st2);
        residbox->AddChild(slider);
        DBG_RECT((sliders[i].name, slider->Frame()));
        r.OffsetBy(0, 60);
    }

    EnableReSidControls(engine);

    Show();
}

SidWindow::~SidWindow() 
{
    sidwindow = NULL;
}

void SidWindow::MessageReceived(BMessage *msg)
{
    BControl *control;
    int32 engine, val;

    switch (msg->what) {
        case MESSAGE_SID_MODEL:
            /* engine_model_popup->SelectedOption((char **)&dummy, &val); */
            val = engine_model_popup->Value();
            engine = val >> 8;
            val &= 0xff;
            sid_set_engine_model(engine, val);
            EnableReSidControls(engine);
            break;
        case MESSAGE_SID_FILTERS:
            resources_toggle("SidFilters", (int *)&val);
            break;
        case MESSAGE_SID_STEREO:
            if (extrasidbox) {
                val = msg->FindInt32("sids");
                resources_set_int("SidStereo", (int)val);
                second_sid_popup->SetEnabled(val >= 1);
                third_sid_popup->SetEnabled(val >= 2);
                fourth_sid_popup->SetEnabled(val >= 3);
            }
            break;
        case MESSAGE_SID_ADDRESS:
        case MESSAGE_SID_RESIDSLIDER:
            msg->FindPointer("source", (void **)&control);
            if (control) {
                DBG_MSG(("Setting resource %s to %d", control->Name(), control->Value()));
                resources_set_int(control->Name(), control->Value());
#ifdef DEBUG_UI
                resources_get_int(control->Name(), (int *)&val);
                DBG_MSG(("Resource %s set to %d", control->Name(), val));
#endif
            }
            break;
        case MESSAGE_SID_RESIDSAMPLING:
            val = msg->FindInt32("mode");
            resources_set_int("SidResidSampling", val);
            break;
        default:
            BWindow::MessageReceived(msg);
    }
}

void ui_sid(int *stereoaddressbase)
{
    thread_id sidthread;
    status_t exit_value;

    if (sidwindow != NULL) {
        return;
    }

    sidaddressbase = stereoaddressbase;

    sidwindow = new SidWindow;

    vsync_suspend_speed_eval();

    /* wait until window closed */
    sidthread = sidwindow->Thread();
    wait_for_thread(sidthread, &exit_value);
}
