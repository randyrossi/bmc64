/*
 * joyai.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#include "vice.h"

#ifdef AMIGA_OS4

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define __USE_INLINE__
#define MUIPROG_H

#include <exec/memory.h>

#include <devices/input.h>
#include <devices/inputevent.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/muimaster.h>
#include <proto/amigainput.h>

#include "joyai.h"
#include "joystick.h"
#include "private.h"

#include "intl.h"
#include "translate.h"

#include "lib.h"

#include "loadlibs.h"

#ifndef MAKE_ID
#define MAKE_ID(a,b,c,d) ((ULONG) (a)<<24 | (ULONG) (b)<<16 | (ULONG) (c)<<8 | (ULONG) (d))
#endif

#define DoMethod IDoMethod

static const char *rawkeynames[] = {
    "~",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "0",
    "+",
    "=",
    "\\",
    "0x0E",
    "PAD 0",
    /* 0x10 */
    "Q",
    "W",
    "E",
    "R",
    "T",
    "Y",
    "U",
    "I",
    "O",
    "P",
    ":",
    "\"",
    "0x1C",
    "PAD 1",
    "PAD 2",
    "PAD 3",
    /* 0x20 */
    "A",
    "S",
    "D",
    "F",
    "G",
    "H",
    "J",
    "K",
    "L",
    "[",
    "]",
    "*",
    "0x2C",
    "PAD 4",
    "PAD 5",
    "PAD 6",
    /* 0x30 */
    "<",
    "Z",
    "X",
    "C",
    "V",
    "B",
    "N",
    "M",
    ",",
    ".",
    "/",
    "0x3B",
    "PAD .",
    "PAD 7",
    "PAD 8",
    "PAD 9",
    /* 0x40 */
    "SPACE",
    "BACKSPACE",
    "TAB",
    "PAD ENTER",
    "ENTER",
    "ESC",
    "DELETE",
    "INSERT", // NEW
    "PAGE UP", // NEW
    "PAGE DOWN", // NEW
    "PAD -",
    "F11", // NEW
    "UP",
    "DOWN",
    "RIGHT",
    "LEFT",
    /* 0x50 */
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "PAD NUMLOCK",
    "PAD SCRLOCK",
    "PAD /",
    "PAD *",
    "PAD +",
    "HELP",
    /* 0x60 */
    "LSHIFT",
    "RSHIFT",
    "CAPSLOCK",
    "CTRL", // L/R
    "LALT",
    "RALT",
    "LAMIGA",
    "RAMIGA",
    "0x68",
    "0x69",
    "0x6A",
    "MENU", // NEW
    "0x6C",
    "PRINT SCREEN", // NEW
    "PAUSE", // NEW
    "F12", // NEW
    /* 0x70 */
    "HOME", // NEW
    "END", // NEW
    "STOP", // NEW
    "PLAY", // NEW
    "0x74",
    "0x75",
    "PREV", // NEW
    "NEXT", // NEW
    "0x78",
    "NUMLOCK", // NEW
    "0x7A",
    "0x7B",
    "0x7C",
    "0x7D",
    "0x7E",
    "0x7F",
};

#define NUMKEYS (sizeof(rawkeynames) / sizeof(char *))

static const char *rawkey_to_name(unsigned long rawkey)
{
    const char *name;
    if (rawkey >= NUMKEYS) {
        name = "";
    } else {
        name = rawkeynames[rawkey];
    }

    return name;
}

//-----------------------------------------------------------------

#define ONOFF_VALUE (32768/2) /* analog -> digital border value */

#define CLAMP(val, min, max) (((val) < (min)) ? (min) : ((val) > (max)) ? (max) : (val))

#define DIGITAL_UP    (0)
#define DIGITAL_DOWN  (1)
#define DIGITAL_LEFT  (2)
#define DIGITAL_RIGHT (3)
#define DIGITAL_FIRE  (4)

#define TYPE_NONE   (0)
#define TYPE_BUTTON (1)
#define TYPE_AXES   (2)
#define TYPE_HAT    (3)

typedef struct {
    char *label;
    int bitnum;
    int rawkey;
    int type;
    int offset;
    char *resource;
} keysym_type;

static keysym_type keysym_1[] = {
    /* Digital */
    { "Up", DIGITAL_UP, -1, TYPE_NONE, 0, NULL },
    { "Down", DIGITAL_DOWN, -1, TYPE_NONE, 0, NULL },
    { "Left", DIGITAL_LEFT,  -1, TYPE_NONE, 0, NULL },
    { "Right", DIGITAL_RIGHT, -1, TYPE_NONE, 0, NULL },
    { "Fire", DIGITAL_FIRE, -1, TYPE_NONE, 0, NULL },
};

static keysym_type keysym_2[] = {
    /* Digital */
    { "Up", DIGITAL_UP, -1, TYPE_NONE, 0, NULL },
    { "Down", DIGITAL_DOWN, -1, TYPE_NONE, 0, NULL },
    { "Left", DIGITAL_LEFT, -1, TYPE_NONE, 0, NULL },
    { "Right",DIGITAL_RIGHT, -1, TYPE_NONE, 0, NULL },
    { "Fire", DIGITAL_FIRE, -1, TYPE_NONE, 0, NULL },
};

static AIN_DeviceID joy_id[2] = {-1, -1};

static char *joy_resource[2] = { NULL, NULL };

#define NUM_KEYSYM ((sizeof(keysym_1)) / (sizeof(keysym_type)))

static AIN_DeviceID default_id = -1, default_count = 0;

static void *CTX = NULL;
static AIN_DeviceHandle *ai_handle[2] = { NULL, NULL };
static struct MsgPort *ai_port = NULL;

#include "resources.h"
#include "util.h"

static int set_JOYAI_resource(const char *name, void *param)
{
    char **ptr = (char **)param;
    if (*ptr != NULL && name != NULL) {
        if (strcmp(name, *ptr) == 0) {
            return 0;
        }
    }
    util_string_set(ptr, name ? name : "");
    return 0;
}

#define MYRES(res, def, name) { name, def, RES_EVENT_NO, NULL, &res, set_JOYAI_resource, (void *)&res },

static const resource_string_t resources_string[] = {
    MYRES(joy_resource[0], "-1", "JOYAI1_ID")
    MYRES(keysym_1[0].resource, "0,0,-1", "JOYAI1_Up")
    MYRES(keysym_1[1].resource, "0,0,-1", "JOYAI1_Down")
    MYRES(keysym_1[2].resource, "0,0,-1", "JOYAI1_Left")
    MYRES(keysym_1[3].resource, "0,0,-1", "JOYAI1_Right")
    MYRES(keysym_1[4].resource, "0,0,-1", "JOYAI1_Fire")
    MYRES(joy_resource[1], "-1", "JOYAI2_ID")
    MYRES(keysym_2[0].resource, "0,0,-1", "JOYAI2_Up")
    MYRES(keysym_2[1].resource, "0,0,-1", "JOYAI2_Down")
    MYRES(keysym_2[2].resource, "0,0,-1", "JOYAI2_Left")
    MYRES(keysym_2[3].resource, "0,0,-1", "JOYAI2_Right")
    MYRES(keysym_2[4].resource, "0,0,-1", "JOYAI2_Fire")
    RESOURCE_STRING_LIST_END
};

int joyai_init_resources(void)
{
    return resources_register_string(resources_string);
}

static void get_cfg(int joy)
{
    keysym_type *keysym = (joy == 2) ? keysym_2 : keysym_1;
    char *tmp, txt[256];
    unsigned int i;

    sprintf(txt, "JOYAI%d_ID", joy);
    tmp = NULL;
    resources_get_value(txt, (void *)&tmp);
    if (tmp != NULL) {
        joy_id[joy - 1] = atoi(tmp);
    }

    for (i = 0; i < NUM_KEYSYM; i++) {
        sprintf(txt, "JOYAI%d_%s", joy, keysym[i].label);
        tmp = NULL;
        resources_get_value(txt, (void *)&tmp);
        if (tmp != NULL) {
            sscanf(tmp, "%d,%d,%d", &keysym[i].type, &keysym[i].offset, &keysym[i].rawkey);
        }
    }
}

static void set_cfg(int joy)
{
    keysym_type *keysym = (joy == 2) ? keysym_2 : keysym_1;
    char tmp[256], txt[256];
    unsigned int i;

    sprintf(txt, "JOYAI%d_ID", joy);
    sprintf(tmp, "%lu", joy_id[joy - 1]);
    resources_set_value(txt, tmp);

    for (i = 0; i < NUM_KEYSYM; i++) {
        sprintf(txt, "JOYAI%d_%s", joy, keysym[i].label);
        sprintf(tmp, "%d,%d,%d", keysym[i].type,keysym[i].offset,keysym[i].rawkey);
        resources_set_value(txt, tmp);
    }
}

static void ai_exit(void)
{
    if (CTX != NULL) {
        AIN_DeleteContext(CTX);
        CTX = NULL;
    }
    if (ai_port != NULL) {
        DeleteMsgPort(ai_port);
        ai_port = NULL;
    }
    if (IAIN) {
        DropInterface((struct Interface *)IAIN);
        IAIN = NULL;
    }
    if (AIN_Base) {
        CloseLibrary(AIN_Base);
        AIN_Base = NULL;
    }
    amigainput_lib_loaded = 0;
}

static int ai_init(void)
{
    if (!amigainput_lib_loaded) {
        return -1;
    }

    /* load_libs is called by ui_init_finish, which is too late for us */
    if ((AIN_Base = OpenLibrary("AmigaInput.library", 51))) {
        if ((IAIN = GetInterface(AIN_Base, "main", 1, NULL))) {
            if ((ai_port = CreateMsgPort())) {
                struct TagItem tags[] = { { AINCC_Port, (ULONG)ai_port}, { TAG_DONE, TAG_DONE } };

                CTX = AIN_CreateContext(1, tags);
                if (CTX != NULL) {
                    return 0;
                }
            }
        }
    }

    ai_exit();

    return -1;
}

static void ai_release(void)
{
    int i;

    for (i = 0; i < 2; i++) {
        if ((CTX != NULL) && (ai_handle[i] != NULL)) {
            AIN_ReleaseDevice(CTX, ai_handle[i]);
            ai_handle[i] = NULL;
        }
    }
}

static void ai_attach(void)
{
    unsigned int i;

    for (i = 0; i < 2; i++) {
        get_cfg(i + 1);
        if ((CTX != NULL) && (joy_id[i] != -1)) {
            ai_handle[i] = AIN_ObtainDevice(CTX, joy_id[i]);
        }
    }
}

void joyai_close(void)
{
    ai_release();
    ai_exit();
}

int joyai_open(void)
{
    if (ai_init() == -1) {
        return -1;
    }

    /* HACK: try to load the ini file, so we don't fall back to the defaults */
    resources_load(NULL);

    ai_attach();

    return 0;
}

static int get_key(AIN_DeviceID ID, int *keycode, int *offset)
{
    static char *window_text;
    struct Window *window;
    AIN_InputEvent *event;
    int done = 0;

    window_text = translate_text(IDS_PRESS_KEY_BUTTON);
    *keycode = *offset = -1; /* no key */

    if (ID != -1) {
        ai_handle[0] = AIN_ObtainDevice(CTX, ID);
        if (ai_handle[0] != NULL) {
            AIN_SetDeviceParameter(CTX, ai_handle[0], AINDP_EVENT, TRUE);
        }
    }

    window = OpenWindowTags(NULL,
                            WA_Title, (ULONG)"",
                            WA_Flags, WFLG_NOCAREREFRESH|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_CLOSEGADGET|WFLG_RMBTRAP|WFLG_GIMMEZEROZERO,
                            WA_IDCMP, IDCMP_CLOSEWINDOW|WFLG_REPORTMOUSE|IDCMP_RAWKEY|IDCMP_CHANGEWINDOW,
                            WA_Left, 100,
                            WA_Top, 100,
                            WA_Width, 100,
                            WA_Height, 40,
                            WA_Activate, TRUE,
                            TAG_DONE);

    if (window != NULL) {
        struct IntuiMessage *imsg = NULL;
        ULONG imCode, imClass;

        /* Resize window and set pens */
        int length = TextLength(window->RPort, window_text, strlen(window_text));

        ChangeWindowBox(window, 100, 100, window->BorderLeft+length+window->BorderRight+8, window->BorderTop+window->IFont->tf_YSize*2+window->BorderBottom);
        SetAPen(window->RPort, 1);
        SetBPen(window->RPort, 0);

        /* Wait until window has been resized */
        while (done == 0) {
            /* Wait for messages */
            Wait((1L << window->UserPort->mp_SigBit) | (1L << ai_port->mp_SigBit));

            /* Check for IDCMP messages */
            while ((imsg = (struct IntuiMessage *)GetMsg(window->UserPort))) {
                imClass = imsg->Class;
                imCode = imsg->Code;

                ReplyMsg((struct Message *)imsg);

                if (imClass == IDCMP_CHANGEWINDOW) {
                    Move(window->RPort, 4, window->IFont->tf_YSize);
                    Text(window->RPort, window_text, strlen(window_text));
                } else if (imClass == IDCMP_RAWKEY) {
                    imCode &= 0x7f;
                    if (imCode != 69) {
                        *keycode = imCode;
                    }
                    done = 1; /* KEY ok */
                } else if (imClass == IDCMP_CLOSEWINDOW) {
                    done = -1; /* cancel */
                }
            }

            /* Check for AI messages */
            if (ai_handle[0] != NULL) {
                while ((event = AIN_GetEvent(CTX))) {
                    switch(event->Type) {
                        case AINET_AXIS:
                            if ((event->Value >= (ONOFF_VALUE)) || (event->Value <= (-(ONOFF_VALUE)))) {
                                *offset = event->Index;
                                done = 2; /* AI ok */
                            }
                            break;
                        case AINET_BUTTON:
                            *offset = event->Index;
                            done = 2; /* AI ok */
                            break;
                        case AINET_HAT:
                            *offset = event->Index;
                            done = 2; /* AI ok */
                            break;
                        default:
                            break;
                    }

                    AIN_FreeEvent(CTX, event);
                }
            }
        }
        CloseWindow(window);
    }

    if (ai_handle[0] != NULL) {
        AIN_SetDeviceParameter(CTX, ai_handle[0], AINDP_EVENT, FALSE);
        /* Remove pending AI messages */
        while ((event = AIN_GetEvent(CTX))) {
            AIN_FreeEvent(CTX, event);
        }
        AIN_ReleaseDevice(CTX, ai_handle[0]);
        ai_handle[0] = NULL;
    }

    return done;
}

static struct {
    ULONG count;
    AIN_DeviceID ids[256];
    char *names[256];
} devices;

static struct {
    ULONG count;
    int offsets[256];
    char *names[256];
    int types[256];
} inputs;

static void update_inputs(ULONG ID)
{
    char name[256];
    unsigned int i;
    int num, offset;

    for (i = 0; i < inputs.count; i++) {
        lib_free(inputs.names[i]);
    }
    memset(&inputs, 0, sizeof(inputs));

    inputs.offsets[0] = 0;
    inputs.names[0] = lib_stralloc("-");
    inputs.types[0] = TYPE_NONE;
    inputs.count++;

    if (ID == -1) {
        return;
    }

    /* BUTTON */
    AIN_Query(CTX, ID, AINQ_NUMBUTTONS, 0, &num, sizeof(num));
    AIN_Query(CTX, ID, AINQ_BUTTON_OFFSET, 0, &offset, sizeof(offset));
    for (i = 0; i < num; i++) {
        name[0] = '\0';
        AIN_Query(CTX, ID, AINQ_BUTTONNAME, i, name, sizeof(name));
        if (name[0] != '\0') {
            inputs.offsets[inputs.count] = offset + i;
            inputs.names[inputs.count] = lib_stralloc(name);
            inputs.types[inputs.count] = TYPE_BUTTON;
            inputs.count++;
        }
    }

    /* AXES */
    AIN_Query(CTX, ID, AINQ_NUMAXES, 0, &num, sizeof(num));
    AIN_Query(CTX, ID, AINQ_AXIS_OFFSET, 0, &offset, sizeof(offset));
    for (i = 0; i < num; i++) {
        name[0] = '\0';
        AIN_Query(CTX, ID, AINQ_AXISNAME, i, name, sizeof(name));
        if (name[0] != '\0') {
            inputs.offsets[inputs.count] = offset + i;
            inputs.names[inputs.count] = lib_stralloc(name);
            inputs.types[inputs.count] = TYPE_AXES;
            inputs.count++;
        }
    }

    /* HATS */
    AIN_Query(CTX, ID, AINQ_NUMHATS, 0, &num, sizeof(num));
    AIN_Query(CTX, ID, AINQ_HAT_OFFSET, 0, &offset, sizeof(offset));
    for (i = 0; i < num; i++) {
        name[0] = '\0';
        AIN_Query(CTX, ID, AINQ_HATNAME, i, name, sizeof(name));
        if (name[0] != '\0') {
            inputs.offsets[inputs.count] = offset + i;
            inputs.names[inputs.count] = lib_stralloc(name);
            inputs.types[inputs.count] = TYPE_HAT;
            inputs.count++;
        }
    }
}

static int offset_to_index(int offset)
{
    unsigned int i;

    for (i = 0; i < inputs.count; i++) {
        if ((inputs.types[i] != TYPE_NONE) && (inputs.offsets[i] == offset)) {
            return i;
        }
    }
    return -1;
}

BOOL enumfunc(AIN_Device *device, void *UserData)
{
    devices.ids[devices.count] = device->DeviceID;
    devices.names[devices.count] = lib_stralloc(device->DeviceName);

    if (default_id != -1) {
        if (default_id == device->DeviceID) {
            default_count = devices.count;
        }
    }

    devices.count++;

    return FALSE;
}

static int offset_to_active(int type, int offset)
{
    unsigned int i;

    for (i = 0; i < inputs.count; i++) {
        if ((inputs.types[i] == type) && (inputs.offsets[i] == offset)) {
            return i;
        }
    }
    return 0;
}

#define BTN_OK (32)
#define DEV_CHANGE (33)
#define KEYNAME "VICE"

int joyai_config(int joy)
{
    BOOL running = TRUE;
    keysym_type *keysym = (joy == 2) ? keysym_2 : keysym_1;
    ULONG signals;

    if (!amigainput_lib_loaded) {
        return -1;
    }

    if (CTX == NULL) {
        return -1;
    }

    get_cfg(joy);
    ai_release();

    memset(&devices, 0, sizeof(devices));
    memset(&inputs, 0, sizeof(inputs));

    if (joy_id[joy - 1] != -1) {
        ai_handle[0] = AIN_ObtainDevice(CTX, joy_id[joy - 1]);
        if (ai_handle[0] != NULL) {
            AIN_ReleaseDevice(CTX, ai_handle[0]);
            ai_handle[0] = NULL;
        } else {
            joy_id[joy - 1] = -1;
        }
    }

    devices.ids[devices.count] = -1;
    devices.names[devices.count] = lib_stralloc("-");
    devices.count++;

    default_id = joy_id[joy - 1];
    default_count = 0;

    AIN_EnumDevices(CTX, enumfunc, &devices);

    update_inputs(joy_id[joy - 1]);

    {
        APTR label, text[NUM_KEYSYM], button, app, main_group, window, ok, cancel, Odevice, Oinput[NUM_KEYSYM];

        app = ApplicationObject,
                MUIA_Application_Author, "Mathias Roslund & Marco van den Heuvel",
                MUIA_Application_Base, KEYNAME,
                MUIA_Application_Title, KEYNAME,
                MUIA_Application_Version, "$VER: " KEYNAME " v1.0",
                MUIA_Application_Copyright, "Mathias Roslund & Marco van den Heuvel",
                MUIA_Application_Description, "Versatile Commodore Emulator",
                SubWindow, window = WindowObject,
                MUIA_Window_Title, KEYNAME,
                MUIA_Window_ID, MAKE_ID('0', 'W', 'I', 'N'),
                MUIA_Window_Screen, canvaslist->os->screen,
                WindowContents, VGroup,
                Child, HGroup,
                Child, TextObject,
                MUIA_Text_PreParse, "\033r",
                MUIA_Text_Contents, "AI Device",
                MUIA_Weight, 0,
                MUIA_InnerLeft, 0,
                MUIA_InnerRight, 0,
                End,
                Child, Odevice = CycleObject,
                MUIA_Cycle_Entries, devices.names,
	          MUIA_Cycle_Active, default_count,
                End,
                End,
                Child, main_group = ColGroup(4),
                End,
                Child, HGroup,
                Child, ok = TextObject,
                ButtonFrame,
                MUIA_Background, MUII_ButtonBack,
                MUIA_Text_Contents, "Ok",
                MUIA_Text_PreParse, "\033c",
                MUIA_InputMode, MUIV_InputMode_RelVerify,
                End,
                Child, cancel = TextObject,
                ButtonFrame,
                MUIA_Background, MUII_ButtonBack,
                MUIA_Text_Contents, "Cancel",
                MUIA_Text_PreParse, "\033c",
                MUIA_InputMode, MUIV_InputMode_RelVerify,
                End,
                End,
                End,
                End,
                End;

        if (app) {
            unsigned int i;

            DoMethod(window, MUIM_Notify, MUIA_Window_CloseRequest, TRUE,
                     app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

            DoMethod(Odevice, MUIM_Notify, MUIA_Cycle_Active, MUIV_EveryTime,
                     app, 2, MUIM_Application_ReturnID, DEV_CHANGE);

            DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                     app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

            DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                     app, 2, MUIM_Application_ReturnID, BTN_OK);

            for (i = 0; i < NUM_KEYSYM; i++) {

                /* label */
                label = TextObject,
                          MUIA_Text_PreParse, "\033r",
                          MUIA_Text_Contents, keysym[i].label,
                          MUIA_InnerLeft, 0,
                          MUIA_InnerRight, 0,
                        End;

                Oinput[i] = CycleObject,
                              MUIA_Cycle_Entries, inputs.names,
                              MUIA_Cycle_Active, offset_to_active(keysym[i].type, keysym[i].offset),
                            End,

                text[i] = TextObject,
                            MUIA_Background, MUII_TextBack,
                            MUIA_Frame, MUIV_Frame_Text,
                            MUIA_Text_Contents, rawkey_to_name(keysym[i].rawkey),
                          End;

                button = TextObject,
                           ButtonFrame,
                           MUIA_Background, MUII_ButtonBack,
                           MUIA_Text_Contents, "Change",
                           MUIA_Text_PreParse, "\033c",
                           MUIA_InputMode, MUIV_InputMode_RelVerify,
                         End;

                DoMethod(button, MUIM_Notify, MUIA_Pressed, FALSE,
                         app, 2, MUIM_Application_ReturnID, (i + 1));

                /* add to window */

                DoMethod(main_group, OM_ADDMEMBER, label);
                DoMethod(main_group, OM_ADDMEMBER, Oinput[i]);
                DoMethod(main_group, OM_ADDMEMBER, text[i]);
                DoMethod(main_group, OM_ADDMEMBER, button);

            }
        }

        if (app) {
            unsigned int i;

            set(window, MUIA_Window_Open, TRUE);
            while (running) {
                unsigned long retval = DoMethod(app, MUIM_Application_Input, &signals);

                switch (retval) {
                    case MUIV_Application_ReturnID_Quit:
                        running = FALSE;
                        break;
                    case BTN_OK:
                        for (i = 0; i < NUM_KEYSYM; i++) {
                            ULONG active;

                            get(Oinput[i], MUIA_Cycle_Active, &active);
                            keysym[i].type = inputs.types[active];
                            keysym[i].offset = inputs.offsets[active];
                        }
                        set_cfg(joy);
                        running = FALSE;
                        break;
                    case DEV_CHANGE:
                        {
                            ULONG active;

                            get(Odevice, MUIA_Cycle_Active, &active);
                            joy_id[joy - 1] = devices.ids[active];
                            update_inputs(joy_id[joy - 1]);
                            for (i = 0; i < NUM_KEYSYM; i++) {
                                set(Oinput[i], MUIA_Cycle_Entries, inputs.names);
                                set(Oinput[i], MUIA_Cycle_Active, 0);
                            }
                        }
                        break;
                    default:
                        if ((retval >= 1) && (retval <= (1 + NUM_KEYSYM))) {
                            int keycode, index, result;
                            result = get_key(joy_id[joy - 1], &keycode, &index);
                            if (result == 1) { /* KEY */
                                keysym[retval - 1].rawkey = keycode;
                                set(text[retval - 1], MUIA_Text_Contents, rawkey_to_name(keycode));
                            } else if (result == 2) { /* AI */
                                index = offset_to_index(index);
                                if (index >= 0) {
                                    set(Oinput[retval - 1], MUIA_Cycle_Active, index);
                                }
                            }
                        }
                        break;
                }
                if (running && signals) {
                    Wait(signals);
                }
            }
            MUI_DisposeObject(app);
        }  
    }

#undef DEV_CHANGE
#undef BTN_OK

    ai_attach();

    return 0;
}

int joyai_update(int joy, int dst)
{
    void *ptr;
    keysym_type *keysym = (joy == 2) ? keysym_2 : keysym_1;
    uint8_t value = 0;

    if (!amigainput_lib_loaded) {
        return -1;
    }

    if ((CTX == NULL) || (ai_handle[joy - 1] == NULL)) {
        return -1;
    }

    if (AIN_ReadDevice(CTX, ai_handle[joy - 1], &ptr) == TRUE) {
        unsigned int i;
        int32 *data = ptr;

        for (i = 0; i < NUM_KEYSYM; i++) {
            switch (keysym[i].type) {

                /* Axis and hats use values between -32768 and 32767. Digital Buttons use
                 * values between 0 and 1 (1 is "pressed"), while analog buttons use
                 * values between 0 and 32767.
                 */
                case TYPE_BUTTON:
                    if (data[keysym[i].offset]) {
                        value |= (1 << keysym[i].bitnum);
                    }
                    break;
                case TYPE_AXES:
                    switch (keysym[i].bitnum) {
                        case DIGITAL_UP: /* neg value */
                        case DIGITAL_LEFT: /* neg value */
                            if (data[keysym[i].offset] <= (-(ONOFF_VALUE))) {
                                value |= (1 << keysym[i].bitnum);
                            }
                            break;
                        case DIGITAL_DOWN: /* pos value */
                        case DIGITAL_RIGHT: /* pos value */
                            if (data[keysym[i].offset] >= (ONOFF_VALUE)) {
                                value |= (1 << keysym[i].bitnum);
                            }
                            break;
                        default:
                            break;
                    }
                    break;
                case TYPE_HAT:
                    switch (data[keysym[i].offset]) {
                        case 1: /* N */
                            value |= (1 << DIGITAL_UP);
                            break;
                        case 2: /* NE */
                            value |= ((1 << DIGITAL_UP) | (1 << DIGITAL_RIGHT));
                            break;
                        case 3: /* E */
                            value |= (1 << DIGITAL_RIGHT);
                            break;
                        case 4: /* SE */
                            value |= ((1 << DIGITAL_DOWN) | (1 << DIGITAL_RIGHT));
                            break;
                        case 5: /* S */
                            value |= (1 << DIGITAL_DOWN);
                            break;
                        case 6: /* SW */
                            value |= ((1 << DIGITAL_DOWN) | (1 << DIGITAL_LEFT));
                            break;
                        case 7: /* W */
                            value |= (1 << DIGITAL_LEFT);
                            break;
                        case 8: /* NW */
                            value |= ((1 << DIGITAL_UP) | (1 << DIGITAL_LEFT));
                            break;
                        default: /* none */
                            break;
                    }
                    break;
                default:
                    break;
            }
        }

        joystick_set_value_absolute(dst, value);

        return 0;
    }

    return -1;
}

int joyai_key(int joy, int dst, unsigned long kcode, int pressed)
{
    uint8_t value = 0;
    keysym_type *keysym = (joy == 2) ? keysym_2 : keysym_1;
    unsigned int i;

    for (i = 0; i < NUM_KEYSYM; i++) {
        if ((keysym[i].type == TYPE_NONE) && (keysym[i].rawkey == kcode)) {
            value = (1 << keysym[i].bitnum);
            if (pressed) {
                joystick_set_value_or(dst, value);
            } else {
                joystick_set_value_and(dst, (uint8_t) ~value);
            }
        }
    }

    return value;
}
#endif
