/*
 * hs-win32-dll.c - Windows (HardSID.dll) specific hardsid driver.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
 *  HardSID Support <support@hardsid.com>
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

/* Tested and confirmed working on:

 - Windows 95C (ISA HardSID)
 - Windows 95C (ISA HardSID Quattro)
 - Windows 98SE (ISA HardSID)
 - Windows 98SE (ISA HardSID Quattro)
 - Windows 98SE (PCI HardSID)
 - Windows 98SE (PCI HardSID Quattro)
 - Windows ME (ISA HardSID)
 - Windows ME (ISA HardSID Quattro)
 - Windows ME (PCI HardSID)
 - Windows ME (PCI HardSID Quattro)
 - Windows NT 4 (ISA HardSID)
 - Windows NT 4 (ISA HardSID Quattro)
 - Windows 2000 (ISA HardSID)
 - Windows 2000 (ISA HardSID Quattro)
 - Windows 2000 (PCI HardSID)
 - Windows 2000 (PCI HardSID Quattro)
 */

#include "vice.h"

#ifdef WIN32_COMPILE

#ifdef HAVE_HARDSID
#include <stdio.h>
#include <windows.h>

#include "archdep.h"
#include "hardsid.h"
#include "log.h"
#include "maincpu.h"
#include "sid-resources.h"
#include "types.h"

#include "hs-win32.h"

#define HARDSID_FLUSH_CYCLES 1000
#define HARDSID_FLUSH_MS 50

static CLOCK lastaccess_clk;
static UINT_PTR ftimer;
static DWORD lastaccess_ms;
static int lastaccess_chipno;
static int chipused = -1;

#define MAXSID 4

static int sids_found = -1;

typedef BYTE (CALLBACK* GetHardSIDCount_t)(void);
typedef void (CALLBACK* InitHardSID_Mapper_t)(void);
typedef void (CALLBACK* MuteHardSID_Line_t)(BOOL);
typedef BYTE (CALLBACK* ReadFromHardSID_t)(BYTE, BYTE);
typedef void (CALLBACK* SetDebug_t)(BOOL);
typedef void (CALLBACK* WriteToHardSID_t)(BYTE, BYTE, BYTE);

/* HardSID USB (HardSID 4U) */
typedef void (CALLBACK* HardSID_Reset_t)(BYTE);
typedef BOOL (CALLBACK* HardSID_Lock_t) (BYTE);
typedef void (CALLBACK* HardSID_AbortPlay_t)(BYTE); //hard flush
typedef void (CALLBACK* HardSID_Flush_t)(BYTE); //soft flush
typedef void (CALLBACK* HardSID_Write_t)(BYTE, WORD, BYTE, BYTE);
typedef void (CALLBACK* HardSID_Delay_t)(BYTE, WORD);

static GetHardSIDCount_t GetHardSIDCount;
static InitHardSID_Mapper_t InitHardSID_Mapper;
static MuteHardSID_Line_t MuteHardSID_Line;
static ReadFromHardSID_t ReadFromHardSID;
static SetDebug_t SetDebug;
static WriteToHardSID_t WriteToHardSID;

/* HardSID USB (HardSID 4U) */
static HardSID_Reset_t HardSID_Reset;
static HardSID_Lock_t HardSID_Lock;
static HardSID_AbortPlay_t HardSID_AbortPlay;
static HardSID_Flush_t HardSID_Flush;
static HardSID_Write_t HardSID_Write;
static HardSID_Delay_t HardSID_Delay;

static HINSTANCE dll = NULL;
static unsigned int device_map[MAXSID] = { 0, 0, 0, 0};

static int has_usb_hardsid = 1;

static int init_interface(void)
{
    if (dll == NULL) {
        dll = LoadLibrary("HARDSID.DLL");
        if (dll != NULL) {
            GetHardSIDCount = (GetHardSIDCount_t)GetProcAddress(dll, "GetHardSIDCount");
            InitHardSID_Mapper = (InitHardSID_Mapper_t)GetProcAddress(dll, "InitHardSID_Mapper");
            MuteHardSID_Line = (MuteHardSID_Line_t)GetProcAddress(dll, "MuteHardSID_Line");
            ReadFromHardSID = (ReadFromHardSID_t)GetProcAddress(dll, "ReadFromHardSID");
            SetDebug = (SetDebug_t)GetProcAddress(dll, "SetDebug");
            WriteToHardSID = (WriteToHardSID_t)GetProcAddress(dll, "WriteToHardSID");

/* HardSID USB (HardSID 4U) */
            HardSID_Reset = (HardSID_Reset_t)GetProcAddress(dll, "HardSID_Reset");
            HardSID_Lock = (HardSID_Lock_t)GetProcAddress(dll, "HardSID_Lock");
            HardSID_AbortPlay = (HardSID_AbortPlay_t)GetProcAddress(dll, "HardSID_Flush");
            HardSID_Flush = (HardSID_Flush_t)GetProcAddress(dll, "HardSID_SoftFlush");
            HardSID_Write = (HardSID_Write_t)GetProcAddress(dll, "HardSID_Write");
            HardSID_Delay = (HardSID_Delay_t)GetProcAddress(dll, "HardSID_Delay");

            if (HardSID_Flush == NULL) {
                has_usb_hardsid = 0;
            }
        } else {
            return -1;
        }
    }

    return 0;
}

void hs_dll_reset(void)
{
    BYTE r;

    if (dll != NULL) {
#ifndef SID_TEST_MODE
        if (has_usb_hardsid) {
            if (chipused >= 0) {
                HardSID_AbortPlay((BYTE)device_map[chipused]);
                HardSID_Write((BYTE)device_map[chipused], 4, 0, 0x00);
                Sleep(300);
                for (r = 0; r <= 0x19; r++) {
                    HardSID_Write((BYTE)device_map[chipused], 4, r, 0x00);
                }
                HardSID_Flush((BYTE)device_map[chipused]);
            }
        }
#endif
    }
}

#ifndef SID_TEST_MODE
static VOID CALLBACK ftimerproc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime)
{ 
    if (lastaccess_chipno >= 0 && lastaccess_ms > 0 && (dwTime - lastaccess_ms) >= HARDSID_FLUSH_MS) {
        lastaccess_ms = 0;
        lastaccess_clk = 0;
        HardSID_Flush((BYTE)device_map[lastaccess_chipno]);
        lastaccess_chipno = -1;
    }
}
#endif

int hs_dll_open(void)
{
    int chipno;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting DLL based HardSID boards.");

    init_interface();

    if (!dll) {
        log_message(LOG_DEFAULT, "Cannot open hardsid.dll.");
        return -1;
    }

#ifndef SID_TEST_MODE
    if (dll != NULL && has_usb_hardsid) {
        for (chipno = 0; chipno < 4; chipno++) {
            HardSID_Lock((BYTE)chipno);
        }
        lastaccess_clk = 0;
        lastaccess_ms = 0;
        lastaccess_chipno = -1;
        chipused = -1;
        ftimer = SetTimer(NULL, ftimer, 1, (TIMERPROC) ftimerproc);
    }
#endif
    sids_found = GetHardSIDCount();

    if (!sids_found) {
        log_message(LOG_DEFAULT, "No SIDs found on the HardSID board.");
        return -1;
    }

    log_message(LOG_DEFAULT, "DLL based HardSID: opened, found %d SIDs.", sids_found);

    return 0;
}

static void pcisa_hardsid_close(void)
{
    int chipno;
    WORD addr;

    for (chipno = 0; chipno < 4; chipno++) {
        for (addr = 0; addr < 24; addr++) {
           hardsid_store(addr, 0, chipno);
        }
    }
    log_message(LOG_DEFAULT, "DLL based HardSID: closed.");
}

static void usb_hardsid_close(void)
{
#ifndef SID_TEST_MODE
    KillTimer(NULL, ftimer);
    if (chipused >= 0) {
        HardSID_AbortPlay((BYTE)device_map[chipused]);
    }
#endif
}

int hs_dll_close(void)
{
    if (dll != NULL) {
        if (has_usb_hardsid) {
            usb_hardsid_close();
        } else {
            pcisa_hardsid_close();
        }
    }
    sids_found = -1;
    return 0;
}

int hs_dll_read(uint16_t addr, int chipno)
{
    if (chipno < MAXSID && addr < 0x20 && dll != NULL && !has_usb_hardsid) {
        return ReadFromHardSID((BYTE)device_map[chipno], (UCHAR)(addr & 0x1f));
    }

    return 0;
}

void hs_dll_store(uint16_t addr, uint8_t val, int chipno)
{
    CLOCK elapsed_cycles;
    BOOL flushneeded = FALSE;

    if (chipno < MAXSID && addr < 0x20 && dll != NULL) {
        if (!has_usb_hardsid) {
            WriteToHardSID((BYTE)device_map[chipno], (UCHAR)(addr & 0x1f), val);
        } else {
#ifndef SID_TEST_MODE
            lastaccess_chipno = chipno;
            chipused = chipno;
            lastaccess_ms = GetTickCount();

            if (lastaccess_clk == 0) {
                elapsed_cycles = 0;
            } else {
                elapsed_cycles = (maincpu_clk - lastaccess_clk) & 0xffff;
                if (elapsed_cycles > HARDSID_FLUSH_CYCLES) {
                    flushneeded = TRUE;
                    elapsed_cycles = 0;
                }
            }
            lastaccess_clk = maincpu_clk;

            if (dll != NULL) {
                while (elapsed_cycles > 0xffff) {
                    HardSID_Delay((BYTE)device_map[chipno], (WORD)0xffff);
                    elapsed_cycles -= 0xffff;
                }
                HardSID_Write((BYTE)device_map[chipno], (WORD)elapsed_cycles, (UCHAR)(addr & 0x1f), val);
                if (flushneeded) {
                    HardSID_Flush((BYTE)device_map[chipno]);
                }
            }
#endif
        }
    }
}

int hs_dll_available(void)
{
    return sids_found;
}

void hs_dll_set_device(unsigned int chipno, unsigned int device)
{
    if (chipno < MAXSID) {
        device_map[chipno] = device;
    }
}

/* ---------------------------------------------------------------------*/

void hs_dll_state_read(int chipno, struct sid_hs_snapshot_state_s *sid_state)
{
    sid_state->hsid_main_clk = 0;
    sid_state->hsid_alarm_clk = 0;
    sid_state->lastaccess_clk = (DWORD)lastaccess_clk;
    sid_state->lastaccess_ms = lastaccess_ms;
    sid_state->lastaccess_chipno = (DWORD)lastaccess_chipno;
    sid_state->chipused = (DWORD)chipused;
    sid_state->device_map[0] = (DWORD)device_map[0];
    sid_state->device_map[1] = (DWORD)device_map[1];
    sid_state->device_map[2] = (DWORD)device_map[2];
    sid_state->device_map[3] = (DWORD)device_map[3];
}

void hs_dll_state_write(int chipno, struct sid_hs_snapshot_state_s *sid_state)
{
    lastaccess_clk = (CLOCK)sid_state->lastaccess_clk;
    lastaccess_ms = sid_state->lastaccess_ms;
    lastaccess_chipno = (int)sid_state->lastaccess_chipno;
    chipused = (int)sid_state->chipused;
    device_map[0] = (unsigned int)sid_state->device_map[0];
    device_map[1] = (unsigned int)sid_state->device_map[1];
    device_map[2] = (unsigned int)sid_state->device_map[2];
    device_map[3] = (unsigned int)sid_state->device_map[3];
}
#endif
#endif

