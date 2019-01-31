/*
 * timer.c
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

#ifdef AMIGA_AROS
#define __AROS_OFF_T_DECLARED
#define __AROS_PID_T_DECLARED
#endif

#ifndef AMIGA_OS4

#ifdef AMIGA_MORPHOS

#include <exec/memory.h>
#include <exec/devices.h>
#include <devices/timer.h>
#include <proto/exec.h>
#include <proto/timer.h>

#include "timer.h"
#include "lib.h"

struct timer_s {
    struct MsgPort  TimerMP;
    struct timerequest TimerIO;
    struct Library *TimerBase;
};

void *timer_init(void)
{
    struct timer_s *timer;

    timer = lib_AllocMem(sizeof(*timer), MEMF_PUBLIC);
    if (timer) {
        timer->TimerBase = NULL;
        timer->TimerMP.mp_SigBit = AllocSignal(-1);
        if ((uint8_t)timer->TimerMP.mp_SigBit != -1) {
            timer->TimerMP.mp_Node.ln_Type = NT_MSGPORT;
            timer->TimerMP.mp_Flags = PA_SIGNAL;
            timer->TimerMP.mp_SigTask = FindTask(NULL);
            NEWLIST(&timer->TimerMP.mp_MsgList);
            timer->TimerIO.tr_node.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
            timer->TimerIO.tr_node.io_Message.mn_ReplyPort = &timer->TimerMP;
            timer->TimerIO.tr_node.io_Message.mn_Length = sizeof(timer->TimerIO);
            if (OpenDevice(TIMERNAME, UNIT_MICROHZ, (struct IORequest *)&timer->TimerIO, 0) == 0) {
                timer->TimerBase = &timer->TimerIO.tr_node.io_Device->dd_Library;
                return timer;
            }
        }

        timer_exit(timer);
    }

    return NULL;
}

void timer_exit(void *t)
{
    struct timer_s *timer = t;

    if (timer != NULL) {
        if ((uint8_t)timer->TimerMP.mp_SigBit != -1) {
            if (timer->TimerBase != NULL) {
                CloseDevice((struct IORequest *)&timer->TimerIO);
            }
            FreeSignal(timer->TimerMP.mp_SigBit);
        }
        lib_FreeMem(timer, sizeof(*timer));
    }
}

void timer_gettime(void *t, struct timeval *tv)
{
    struct timer_s *timer = t;

    if (timer != NULL) {
        UQUAD ticks;
        ULONG base;

#define TimerBase timer->TimerBase
        base = ReadCPUClock(&ticks);
#undef TimerBase
        tv->tv_secs  = ticks / base;
        tv->tv_micro = 1000000 * (ticks % base) / base;
    }
}

void timer_subtime(void *t, struct timeval *dt, struct timeval *st)
{
    struct timer_s *timer = t;

    if (timer != NULL) {
#define TimerBase timer->TimerBase
        SubTime(dt, st);
#undef TimerBase
    }
}

void timer_usleep(void *t, int us)
{
    struct timer_s *timer = t;

    if (timer != NULL) {
        /* setup */
        timer->TimerIO.tr_node.io_Command = TR_ADDREQUEST;
        timer->TimerIO.tr_time.tv_secs = us / 1000000;
        timer->TimerIO.tr_time.tv_micro = us % 1000000;

        /* send & wait request */
        DoIO((struct IORequest *)&timer->TimerIO);
    }
}

#else

#include <devices/timer.h>
#include <dos/dos.h>
#include <exec/memory.h>
#include <proto/dos.h>
#include <sys/time.h>
#include <proto/exec.h>

#include "lib.h"

#ifndef NEWLIST
#define NEWLIST(l) ((l)->lh_Head = (struct Node *)&(l)->lh_Tail, \
                    (l)->lh_TailPred = (struct Node *)&(l)->lh_Head)
#endif

void *timer_init(void)
{
    return (void *)1;
}

void timer_exit(void *t)
{
}

void timer_gettime(void *t, struct timeval *tv)
{
    if (tv) {
        struct DateStamp t;

        DateStamp(&t);
        tv->tv_sec = ((t.ds_Days + 2922) * 1440 + t.ds_Minute) * 60 + t.ds_Tick / TICKS_PER_SECOND;
        tv->tv_usec = (t.ds_Tick % TICKS_PER_SECOND) * 1000000 / TICKS_PER_SECOND;
    }
}

void timer_subtime(void *t, struct timeval *dt, struct timeval *st)
{
    int extrasub = 0;

    if (dt->tv_usec < st->tv_usec) {
        extrasub=1;
    }
    dt->tv_usec = (dt->tv_usec * (extrasub ==1 ) ? 10 : 1) - st->tv_usec;
    dt->tv_sec=dt->tv_sec - (st->tv_sec + extrasub);
}

void dotimer(ULONG unit,ULONG timercmd,struct timeval *t)
{
    struct PortIO {
        struct timerequest treq;
        struct MsgPort port;
    } *portio;

    if ((portio = lib_AllocMem(sizeof(*portio), MEMF_CLEAR | MEMF_PUBLIC))) {
        portio->port.mp_Node.ln_Type = NT_MSGPORT;
        if ((uint8_t)(portio->port.mp_SigBit = AllocSignal(-1)) >= 0) {
            portio->port.mp_SigTask = FindTask(NULL);
            NEWLIST(&portio->port.mp_MsgList);
            portio->treq.tr_node.io_Message.mn_Node.ln_Type = NT_REPLYMSG;
            portio->treq.tr_node.io_Message.mn_ReplyPort = &portio->port;
            if (!(OpenDevice(TIMERNAME, unit, &portio->treq.tr_node, 0))) {
                portio->treq.tr_node.io_Command = timercmd;
                portio->treq.tr_time.tv_secs = t->tv_secs;
                portio->treq.tr_time.tv_micro = t->tv_micro;
                if (!DoIO(&portio->treq.tr_node)) {
                    t->tv_secs = portio->treq.tr_time.tv_secs;
                    t->tv_micro = portio->treq.tr_time.tv_micro;
                }
                CloseDevice(&portio->treq.tr_node);
            }
            FreeSignal(portio->port.mp_SigBit);
        }
        lib_FreeMem(portio,sizeof(struct PortIO));
    }
}

void timer_usleep(void *t, int us)
{
    struct timeval tv;

    tv.tv_secs = us / 1000000;
    tv.tv_micro = us % 1000000;

    dotimer(UNIT_VBLANK, TR_ADDREQUEST, &tv);
}
#endif
#else

#ifdef AMIGA_OS4_ALT
#define __USE_OLD_TIMEVAL__
#include <devices/timer.h>
#include <exec/io.h>
#endif

#include <proto/exec.h>
#include <proto/timer.h>

#include "timer.h"

struct timer_s {
    struct MsgPort *TimerMP;
#ifdef AMIGA_OS4_ALT
    struct TimeRequest *TimerIO;
#else
    struct timerequest *TimerIO;
#endif
    struct Device *TimerBase;
    struct TimerIFace *ITimer;
};

timer_t *timer_init(void)
{
    timer_t *timer = IExec->AllocVec(sizeof(timer_t), MEMF_PUBLIC | MEMF_CLEAR);
    if (timer == NULL) {
        return NULL;
    }

    if ((timer->TimerMP = IExec->AllocSysObject(ASOT_PORT, NULL))) {
#ifdef AMIGA_OS4_ALT
        if ((timer->TimerIO = IExec->AllocSysObjectTags(ASOT_IOREQUEST, ASOIOR_Size, sizeof(struct TimeRequest), ASOIOR_ReplyPort,timer->TimerMP, TAG_DONE))) {
#else
        if ((timer->TimerIO = IExec->AllocSysObjectTags(ASOT_IOREQUEST, ASOIOR_Size, sizeof(struct timerequest), ASOIOR_ReplyPort, timer->TimerMP, TAG_DONE))) {
#endif
            if (IExec->OpenDevice(TIMERNAME, UNIT_MICROHZ, (struct IORequest *)timer->TimerIO, 0) == 0) {
#ifdef AMIGA_OS4_ALT
                timer->TimerBase = timer->TimerIO->Request.io_Device;
#else
                timer->TimerBase = timer->TimerIO->tr_node.io_Device;
#endif
                timer->ITimer = (struct TimerIFace *)IExec->GetInterface((struct Library *)timer->TimerBase, "main", 1, NULL);
                if (timer->ITimer != NULL) {
                    return timer;
                }
            }
        }
    }

    timer_exit(timer);

    return NULL;
}

void timer_exit(timer_t *timer)
{
    if (timer != NULL) {
        if (timer->ITimer != NULL) {
            IExec->DropInterface((struct Interface *)timer->ITimer);
        }
        if (timer->TimerBase != NULL) {
            IExec->CloseDevice((struct IORequest *)timer->TimerIO);
        }
        if (timer->TimerIO != NULL) {
            IExec->FreeSysObject(ASOT_IOREQUEST, timer->TimerIO);
        }
        if (timer->TimerMP != NULL) {
          IExec->FreeSysObject(ASOT_PORT, timer->TimerMP);
        }
        IExec->FreeVec(timer);
    }
}

void timer_gettime(timer_t *timer, struct timeval *tv)
{
    if (timer != NULL) {
#ifdef AMIGA_OS4_ALT
        timer->ITimer->GetUpTime((struct TimeVal *)tv);
#else
        timer->ITimer->GetUpTime(tv);
#endif
    }
}

void timer_subtime(timer_t *timer, struct timeval *dt, struct timeval *st)
{
    if (timer != NULL) {
#ifdef AMIGA_OS4_ALT
        timer->ITimer->SubTime((struct TimeVal *)dt, (struct TimeVal *)st);
#else
        timer->ITimer->SubTime(dt, st);
#endif
    }
}

void timer_usleep(timer_t *timer, int us)
{
    if (timer != NULL) {
        /* setup */
#ifdef AMIGA_OS4_ALT
        timer->TimerIO->Request.io_Command = TR_ADDREQUEST;
        timer->TimerIO->Time.Seconds = us / 1000000;
        timer->TimerIO->Time.Microseconds = us % 1000000;
#else
        timer->TimerIO->tr_node.io_Command = TR_ADDREQUEST;
        timer->TimerIO->tr_time.tv_secs = us / 1000000;
        timer->TimerIO->tr_time.tv_micro = us % 1000000;
#endif

        /* send request */
        IExec->SetSignal(0, (1L << timer->TimerMP->mp_SigBit));
        IExec->SendIO((struct IORequest *)timer->TimerIO);
        IExec->Wait((1L << timer->TimerMP->mp_SigBit));
        IExec->WaitIO((struct IORequest *)timer->TimerIO);
    }
}
#endif
