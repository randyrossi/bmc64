/*
 * ui_keymap.cc - Keymap settings dialog
 *
 * Written by
 *   Bas Wassink <b.wassink@ziggo.nl>
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

#include <stdlib.h>
#include <string.h>

#include <Box.h>
#include <Button.h>
#include <CheckBox.h>
#include <FilePanel.h>
#include <OptionPopUp.h>
#include <Path.h>
#include <RadioButton.h>
#include <TextControl.h>
#include <Window.h>


extern "C" {
#include "c128.h"
#include "keyboard.h"
#include "machine.h"
#include "pet-resources.h"
#include "resources.h"
#include "ui.h"
#include "ui_keymap.h"
#include "vsync.h"
}

#include "ui_file.h"

#define BOX_LEFT    10
#define BOX_RIGHT   490


//! \brief  Struct holding information on a C128 machine type
struct c128_machine_type_s {
    const char *name;   /**< machine name */
    int id;             /**< machine ID, see c128.h for details */
};


class KeymapWindow : public BWindow
{
    public:
                        KeymapWindow();
                        ~KeymapWindow();
        virtual void    MessageReceived(BMessage *msg);

    private:
        void            get_resources(void);
        void            revert_resources(void);
        void            browse_open(int keymap_type);

        int             keymap_index;
        const char *    keymap_sym_file;
        const char *    keymap_pos_file;
        const char *    keymap_user_sym_file;
        const char *    keymap_user_pos_file;
        int             keyboard_type;
        int             keyboard_mapping;

        BRadioButton *  keymap_radios[4];
        BTextControl *  text_sym_file;
        BTextControl *  text_pos_file;
        BFilePanel *    browser;
        BOptionPopUp *  emu_keyboard;
        BOptionPopUp *  host_keyboard;
};


static KeymapWindow *keymap_window = NULL;

/** \brief  Names for the radio buttons in the 'keymapping' section
 */
static const char *index_names[] = {
    "Symbolic (VICE)",
    "Positional (VICE)",
    "Symbolic (User):",
    "Positional (User):"
};


static struct c128_machine_type_s c128_machines[] = {
    { "International", C128_MACHINE_INT },
    { "Finnish", C128_MACHINE_FINNISH },
    { "French", C128_MACHINE_FRENCH },
    { "German", C128_MACHINE_GERMAN },
    { "Italian", C128_MACHINE_ITALIAN },
    { "Norwegian", C128_MACHINE_NORWEGIAN },
    { "Swedish", C128_MACHINE_SWEDISH },
    { "Swiss", C128_MACHINE_SWISS },
    { NULL, -1 }
};



/** \brief  UI constructor
 */
KeymapWindow::KeymapWindow()
    : BWindow(BRect(50, 50, 550, 330), "Keymap and keyboard settings",
            B_TITLED_WINDOW_LOOK, B_MODAL_APP_WINDOW_FEEL,
            B_NOT_ZOOMABLE | B_NOT_RESIZABLE)
{
    BView *background;
    BRect rect;
    BBox *box;
    BMessage *msg;
    BMessage *browse_msg;
    BMessage *text_msg;
    BRadioButton *radio;
    BTextControl *text;
    BButton *button;
    size_t i;
    mapping_info_t *mapping;

    // get resources
    get_resources();

    // set background color
    rect = Bounds();
    background = new BView(rect, "backview", B_FOLLOW_NONE, B_WILL_DRAW);
    background->SetViewColor(220, 220, 220, 0);
    AddChild(background);

    // generate key mapping section
    box = new BBox(BRect(BOX_LEFT, 10, BOX_RIGHT, 160), "Set Keymap");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("Keyboard mapping");

    for (i = 0; i < sizeof index_names / sizeof index_names[0]; i++) {
        msg = new BMessage(MESSAGE_KEYMAP_INDEX_CHANGED);
        msg->AddInt32("index", (int)i);

        radio = new BRadioButton(BRect(10, 20 + 30 * i, 150, 35 + 30 * i),
                index_names[i], index_names[i], msg);
        radio->SetValue(keymap_index == i);
        box->AddChild(radio);

        switch (i) {
            case KBD_INDEX_USERSYM: // 2
                // add textbox & browse button for user symbolic file
                browse_msg = new BMessage(MESSAGE_KEYMAP_BROWSE);
                browse_msg->AddInt32("keymap_type", KBD_INDEX_USERSYM);
                text_msg = new BMessage(MESSAGE_KEYMAP_TEXT_CHANGED);
                text_msg->AddInt32("keymap_type", KBD_INDEX_USERSYM);
                text_sym_file = new BTextControl(BRect(150, 20 + 30 * i,
                            350, 35 + 30 * i),
                        "usersymfile", NULL, keymap_sym_file, text_msg);
                box->AddChild(text_sym_file);
                button = new BButton(BRect(360, 20 + 30 * i, 450, 35 + 30 * i),
                        "browsesymfile", "Browse ...", browse_msg);
                box->AddChild(button);
                break;

            case KBD_INDEX_USERPOS: // 3
                // add textbox & browse button for user positional file
                browse_msg = new BMessage(MESSAGE_KEYMAP_BROWSE);
                browse_msg->AddInt32("keymap_type", KBD_INDEX_USERPOS);
                text_msg = new BMessage(MESSAGE_KEYMAP_TEXT_CHANGED);
                text_msg->AddInt32("keymap_type", KBD_INDEX_USERPOS);
                text_pos_file = new BTextControl(BRect(150, 20 + 30 * i,
                            350, 35 + 30 * i),
                        "userposfile", NULL, keymap_pos_file, text_msg);
                box->AddChild(text_pos_file);
                button = new BButton(BRect(360, 20 + 30 * i, 450, 35 + 30 * i),
                        "browseposfile", "Browse ...", browse_msg);
                box->AddChild(button);
                break;
            default:
                break;
        }
        // add radio to array for later access
        keymap_radios[i] = radio;
    }
    background->AddChild(box);

    // generate keyboard type and keyboard mapping section
    // C128 or PET only, the other machines have only one type of keyboard
    box = new BBox(BRect(BOX_LEFT, 165, BOX_RIGHT, 270), "keyboardtype");
    box->SetViewColor(220, 220, 220, 0);
    box->SetLabel("Emulated and host keyboard layout");

    // emulated keyboard (resource "KeyboardType")
    emu_keyboard = new BOptionPopUp(BRect(10, 30, 420, 60),
            "KeyboardType", "Emulated keyboard type:",
            new BMessage(MESSAGE_KEYBOARD_TYPE_CHANGED));
    // emu_keyboard->SetDivider(120);
    switch (machine_class) {
        case VICE_MACHINE_C128:
            {
                int i = 0;
                while (c128_machines[i].name != NULL) {
                    emu_keyboard->AddOption(c128_machines[i].name,
                            c128_machines[i].id);
                    i++;
                }
            }
           break;
        case VICE_MACHINE_PET:
            emu_keyboard->AddOption(KBD_TYPE_STR_BUSINESS_US,
                    KBD_TYPE_BUSINESS_US);
            emu_keyboard->AddOption(KBD_TYPE_STR_BUSINESS_UK,
                    KBD_TYPE_BUSINESS_UK);
            emu_keyboard->AddOption(KBD_TYPE_STR_BUSINESS_DE,
                    KBD_TYPE_BUSINESS_DE);
            emu_keyboard->AddOption(KBD_TYPE_STR_BUSINESS_JP,
                    KBD_TYPE_BUSINESS_JP);
            emu_keyboard->AddOption(KBD_TYPE_STR_GRAPHICS_US,
                    KBD_TYPE_GRAPHICS_US);
            break;
        default:
            emu_keyboard->AddOption("default", 0);
            emu_keyboard->SetEnabled(0);
            break;
    }
    box->AddChild(emu_keyboard);

    // host keyboard
    host_keyboard = new BOptionPopUp(BRect(10, 60, 420, 90),
            "KeyboardMapping", "Host keyboard type:",
            new BMessage(MESSAGE_KEYBOARD_MAPPING_CHANGED));
    // host_keyboard->SetDivider(120);
    mapping = keyboard_get_info_list();
    while (mapping->name != NULL) {
        host_keyboard->AddOption(mapping->name, mapping->mapping);
        mapping++;
    }
    box->AddChild(host_keyboard);

    background->AddChild(box);



    //  CenterOnScreen();   // Since Haiku R1, not in BeOS
    Show();
}



// Destructor
KeymapWindow::~KeymapWindow()
{
    keymap_window = NULL;
}



//! \brief  Get resources for the UI
void KeymapWindow::get_resources(void)
{
    resources_get_int("KeymapIndex", &keymap_index);
    resources_get_string("KeymapSymFile", &keymap_sym_file);
    resources_get_string("KeymapPosFile", &keymap_pos_file);
    resources_get_string("KeymapUserSymFile", &keymap_user_sym_file);
    resources_get_string("KeymapUserPosFile", &keymap_user_pos_file);
    resources_get_int("KeyboardType", &keyboard_type);
    resources_get_int("KeyboardMapping", &keyboard_mapping);
}

//! \brief  Revert all resources to their previous values
//
void KeymapWindow::revert_resources(void)
{
    resources_set_int("KeymapIndex", keymap_index);
    resources_set_string("KeymapSymFile", keymap_sym_file);
    resources_set_string("KeymapPosFile", keymap_pos_file);
    resources_set_string("KeymapUserSymFile", keymap_user_sym_file);
    resources_set_string("KeymapUserPosFile", keymap_user_pos_file);
    resources_set_int("KeyboardType", keyboard_type);
    resources_set_int("KeyboardMapping", keyboard_mapping);
}


/** \brief  Pop up a file browser to select a keymap file
 *
 * \param[in]   keymap_type type of keymap (symbolic or positional)
 */
void KeymapWindow::browse_open(int keymap_type)
{
    char title[256];
    BMessage *msg;

    msg = new BMessage(MESSAGE_KEYMAP_BROWSE_DONE);
    msg->AddInt32("keymap_type", keymap_type);

    browser = new BFilePanel(B_OPEN_PANEL, new BMessenger(this), NULL,
            B_FILE_NODE, false, msg, NULL, true);
    sprintf(title, "Select a %s keymap",
            keymap_type == KBD_INDEX_USERSYM ? "symbolic" : "positional");
    browser->Window()->SetTitle(title);
    browser->Show();
    return;
}




void KeymapWindow::MessageReceived(BMessage *msg)
{
    char str[256];
    entry_ref ref;
    BPath *path;
    int index;

    switch (msg->what) {
        case MESSAGE_KEYMAP_SETTINGS_APPLY:
            break;

        case MESSAGE_KEYMAP_INDEX_CHANGED:
            index = msg->FindInt32("index");
            // printf("keymap index = %d\n", index);
            resources_set_int("KeymapIndex", index);
            break;

        case MESSAGE_KEYMAP_TEXT_CHANGED:
            index = msg->FindInt32("keymap_type");
            // update proper resource
            switch (index) {
                case KBD_INDEX_USERSYM:
                    resources_set_string("KeymapUserSymFile",
                            text_sym_file->Text());
                    break;
                case KBD_INDEX_USERPOS:
                    resources_set_string("KeymapUserPosFile",
                            text_pos_file->Text());
                    break;
                default:
                    break;
            }
            // update radio buttons
            if (index >= 0 && index <= KBD_INDEX_LAST) {
                keymap_radios[index]->SetValue(1);
                keymap_radios[index]->Invoke(); // propagate message
            }
            break;

        case MESSAGE_KEYMAP_BROWSE:
            browse_open(msg->FindInt32("keymap_type"));
            break;

        case MESSAGE_KEYMAP_BROWSE_DONE:
            msg->FindRef("refs", 0, &ref);
            path = new BPath(&ref);
            index = msg->FindInt32("keymap_type");
            switch (index) {
                case KBD_INDEX_USERSYM:
                    text_sym_file->SetText(path->Path());
                    text_sym_file->Invoke();    // propagate message
                    break;
                case KBD_INDEX_USERPOS:
                    text_pos_file->SetText(path->Path());
                    text_pos_file->Invoke();    // propagate message
                    break;
                default:
                    break;
            }
            delete path;
            break;

        case MESSAGE_KEYBOARD_TYPE_CHANGED:
            {
                const char *name;
                int32 value;

                emu_keyboard->SelectedOption(&name, &value);
                // printf("%s: %d\n", name, value);
                resources_set_int("KeyboardType", value);
            }
            break;

        case MESSAGE_KEYBOARD_MAPPING_CHANGED:
            {
                const char *name;
                int32 value;

                host_keyboard->SelectedOption(&name, &value);
                resources_set_int("KeyboardMapping", value);
            }
            break;

        default:
            BWindow::MessageReceived(msg);
            break;
    }
}

// execute the dialog
void ui_keymap(void)
{
    thread_id keymap_thread;
    status_t exit_value;

    if (keymap_window != NULL) {
        return;
    }

    keymap_window = new KeymapWindow;
    vsync_suspend_speed_eval();

    keymap_thread = keymap_window->Thread();
    wait_for_thread(keymap_thread, &exit_value);
}

