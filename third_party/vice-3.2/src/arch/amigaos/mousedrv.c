/*
 * mousedrv.c
 *
 * Written by
 *  Mathias Roslund <vice.emu@amidog.se>
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

#include "private.h"

#include <stdio.h>
#include "mousedrv.h"
#include "mouse.h"
#include "pointer.h"

#include "lib.h"
#include "vsyncapi.h"

static int g_mx = 0, g_my = 0, g_mb = 0;
static unsigned long mouse_timestamp = 0;

#include <devices/input.h>
#include <devices/inputevent.h>
#include <proto/exec.h>

#if defined(AMIGA_MORPHOS) || defined(AMIGA_M68K)
#include <exec/interrupts.h>
#endif

#define MB_LEFT (1 << 0)
#define MB_RIGHT (1 << 1)
#define MB_MIDDLE (1 << 2)

static mouse_func_t mouse_funcs;

#ifdef AMIGA_MORPHOS
static struct InputEvent *_MyInputHandler(void);
static const struct EmulLibEntry gate_MyInputHandler = {
    TRAP_LIB,
    0,
    (void (*)(void))_MyInputHandler}, *MyInputHandler = &gate_MyInputHandler;

static struct InputEvent *_MyInputHandler(void)
{
    struct InputEvent *event_list = (APTR)REG_A0;
#else
static struct InputEvent *MyInputHandler(struct InputEvent *event_list)
{
#endif
    struct InputEvent *event;

    for (event = event_list; event; event = event->ie_NextEvent) {
        if (event->ie_Class == IECLASS_RAWMOUSE) {
            switch(event->ie_Code) {
                case (IECODE_LBUTTON):
                    Forbid();
                    g_mb |= MB_LEFT;
                    g_mx += event->ie_position.ie_xy.ie_x;
                    g_my -= event->ie_position.ie_xy.ie_y;
                    if (event->ie_position.ie_xy.ie_x || event->ie_position.ie_xy.ie_y) {
                        mouse_timestamp = vsyncarch_gettime();
                    }
                    Permit();
                    event->ie_Class = IECLASS_NULL; /* remove event */
                    break;
                case (IECODE_LBUTTON | IECODE_UP_PREFIX):
                    Forbid();
                    g_mb &= ~MB_LEFT;
                    g_mx += event->ie_position.ie_xy.ie_x;
                    g_my -= event->ie_position.ie_xy.ie_y;
                    if (event->ie_position.ie_xy.ie_x || event->ie_position.ie_xy.ie_y) {
                        mouse_timestamp = vsyncarch_gettime();
                    }
                    Permit();
                    event->ie_Class = IECLASS_NULL; /* remove event */
                    break;
                case (IECODE_RBUTTON):
                    Forbid();
                    g_mb |= MB_RIGHT;
                    g_mx += event->ie_position.ie_xy.ie_x;
                    g_my -= event->ie_position.ie_xy.ie_y;
                    if (event->ie_position.ie_xy.ie_x || event->ie_position.ie_xy.ie_y) {
                        mouse_timestamp = vsyncarch_gettime();
                    }
                    Permit();
                    event->ie_Class = IECLASS_NULL; /* remove event */
                    break;
                case (IECODE_RBUTTON | IECODE_UP_PREFIX):
                    Forbid();
                    g_mb &= ~MB_RIGHT;
                    g_mx += event->ie_position.ie_xy.ie_x;
                    g_my -= event->ie_position.ie_xy.ie_y;
                    if (event->ie_position.ie_xy.ie_x || event->ie_position.ie_xy.ie_y) {
                        mouse_timestamp = vsyncarch_gettime();
                    }
                    Permit();
                    event->ie_Class = IECLASS_NULL; /* remove event */
                    break;
                case (IECODE_MBUTTON):
                    Forbid();
                    g_mb |= MB_MIDDLE;
                    g_mx += event->ie_position.ie_xy.ie_x;
                    g_my -= event->ie_position.ie_xy.ie_y;
                    if (event->ie_position.ie_xy.ie_x || event->ie_position.ie_xy.ie_y) {
                        mouse_timestamp = vsyncarch_gettime();
                    }
                    Permit();
                    event->ie_Class = IECLASS_NULL; /* remove event */
                    break;
                case (IECODE_MBUTTON | IECODE_UP_PREFIX):
                    Forbid();
                    g_mb &= ~MB_MIDDLE;
                    g_mx += event->ie_position.ie_xy.ie_x;
                    g_my -= event->ie_position.ie_xy.ie_y;
                    if (event->ie_position.ie_xy.ie_x || event->ie_position.ie_xy.ie_y) {
                        mouse_timestamp = vsyncarch_gettime();
                    }
                    Permit();
                    event->ie_Class = IECLASS_NULL; /* remove event */
                    break;
                case (IECODE_NOBUTTON):
                    Forbid();
                    g_mx += event->ie_position.ie_xy.ie_x;
                    g_my -= event->ie_position.ie_xy.ie_y;
                    if (event->ie_position.ie_xy.ie_x || event->ie_position.ie_xy.ie_y) {
                        mouse_timestamp = vsyncarch_gettime();
                    }
                    Permit();
                    event->ie_Class = IECLASS_NULL; /* remove event */
                    break;
                default:
                    break;
            }
        }
    }

    return event_list; /* let someone else process the events */
}

static struct IOStdReq *inputReqBlk = NULL;
static struct MsgPort *inputPort = NULL;
static struct Interrupt *inputHandler = NULL;
static const UBYTE HandlerName[] = __FILE__ " input handler";
static int input_error = -1;

void rem_inputhandler(void)
{
    if (!input_error) {
        inputReqBlk->io_Data = (APTR)inputHandler;
        inputReqBlk->io_Command = IND_REMHANDLER;
        DoIO((struct IORequest *)inputReqBlk);
        CloseDevice((struct IORequest *)inputReqBlk);
        input_error = -1;
    }

    if (inputReqBlk) {
        DeleteIORequest((struct IORequest *)inputReqBlk);
        inputReqBlk = NULL;
    }

    if (inputHandler) {
        lib_FreeMem(inputHandler, sizeof(struct Interrupt));
        inputHandler = NULL;
    }

    if (inputPort) {
        DeleteMsgPort(inputPort);
        inputPort = NULL;
    }
}

int add_inputhandler(void)
{
    if ((inputPort = CreateMsgPort())) {
        if ((inputHandler = lib_AllocMem(sizeof(struct Interrupt), MEMF_PUBLIC|MEMF_CLEAR))) {
            if ((inputReqBlk = (struct IOStdReq *)CreateIORequest(inputPort, sizeof(struct IOStdReq)))) {
                if (!(input_error = OpenDevice("input.device", 0, (struct IORequest *)inputReqBlk, 0))) {
                    inputHandler->is_Code = (void *)MyInputHandler;
                    inputHandler->is_Data = NULL;
                    inputHandler->is_Node.ln_Pri = 100;
                    inputHandler->is_Node.ln_Name = (STRPTR)HandlerName;
                    inputReqBlk->io_Data = (APTR)inputHandler;
                    inputReqBlk->io_Command = IND_ADDHANDLER;
                    DoIO((struct IORequest *)inputReqBlk);
                }
            }
        }
    }

    if (!input_error) {
      return 0;
    } else {
      rem_inputhandler();
      return -1;
    }
}

static int mouse_acquired = 0;

int mousedrv_resources_init(mouse_func_t *funcs)
{
    mouse_funcs.mbl = funcs->mbl;
    mouse_funcs.mbr = funcs->mbr;
    mouse_funcs.mbm = funcs->mbm;
    mouse_funcs.mbu = funcs->mbu;
    mouse_funcs.mbd = funcs->mbd;
    return 0;
}

int mousedrv_cmdline_options_init(void)
{
    return 0;
}

void mousedrv_init(void)
{
}

void mousedrv_mouse_changed(void)
{
    if (_mouse_enabled) {
        if (add_inputhandler() == 0) {
            pointer_hide();
            mouse_acquired = 1;
        }
    } else {
        if (mouse_acquired) {
            pointer_to_default();
            mouse_acquired = 0;
            rem_inputhandler();
        }
    }
}

int mousedrv_get_x(void)
{
    static int mx;

    if (_mouse_enabled) {
        Forbid();
        mx = g_mx;
        Permit();
    }
    return mx >> 1;
}

int mousedrv_get_y(void)
{
    static int my;

    if (_mouse_enabled) {
        Forbid();
        my = g_my;
        Permit();
    }
    return my >> 1;
}

void mousedrv_sync(void)
{
    if (mouse_acquired) {
        int mb;

        Forbid();
        mb = g_mb;
        Permit();
        mouse_funcs.mbl(mb & MB_LEFT);
        mouse_funcs.mbr(mb & MB_RIGHT);
        mouse_funcs.mbm(mb & MB_MIDDLE);
    }
}

unsigned long mousedrv_get_timestamp(void)
{
    return mouse_timestamp;
}

void mousedrv_button_left(int pressed)
{
    mouse_funcs.mbl(pressed);
}

void mousedrv_button_right(int pressed)
{
    mouse_funcs.mbr(pressed);
}

void mousedrv_button_middle(int pressed)
{
    mouse_funcs.mbm(pressed);
}
