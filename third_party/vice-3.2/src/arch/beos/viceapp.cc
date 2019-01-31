/*
 * viceapp.cc - The main entry for BEOS
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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
#include <Message.h>
#include <string.h>
#include <stdio.h>

extern "C" {
#include "constants.h"
#include "machine.h"
#include "main.h"
#include "maincpu.h"
#include "viceapp.h"
}

static int viceargc;
static char** viceargv;

char APP_SIGNATURE[256];

ViceApp::ViceApp() : BApplication(APP_SIGNATURE)
{
}

int32 vice_start_main(void* data)
{
    main_program(viceargc,viceargv);
	
    /* if vice thread returns, it's time to close the application */
    BMessenger messenger(APP_SIGNATURE);
    BMessage message(WINDOW_CLOSED);
    messenger.SendMessage(&message, be_app);
    return 0;
}

void ViceApp::ReadyToRun()
{
    vicethread = spawn_thread(vice_start_main, "vicethread", B_NORMAL_PRIORITY, NULL);
    resume_thread(vicethread);
}

void ViceApp::MessageReceived(BMessage *message)
{
    status_t exit_value;

    switch (message->what) {
        case WINDOW_CLOSED:
            /* Finish the Application after emulation thread has finished */
            wait_for_thread(vicethread, &exit_value);
            Quit();
            break;
        default:
            BApplication::MessageReceived(message);
            break;
    }
}

int main(int argc, char **argv)
{
    strcpy(APP_SIGNATURE, "application/x-vnd.Be");
    strcat(APP_SIGNATURE, machine_name);
    viceargc = argc;
    viceargv = argv;
    ViceApp theApp;		// The application object
    theApp.Run();
    return 0;
}

void main_exit(void)
{
}
