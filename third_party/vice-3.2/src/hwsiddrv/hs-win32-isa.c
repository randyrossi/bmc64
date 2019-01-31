/*
 * hs-win32-isa.c - HardSID ISA support for WIN32.
 *
 * Written by
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

 - Windows 95C (ISA HardSID, Direct ISA I/O, inpout32.dll is incompatible with windows 95)
 - Windows 95C (ISA HardSID Quattro, Direct ISA I/O, inpout32.dll is incompatible with windows 95)
 - Windows 98SE (ISA HardSID, winio.dll ISA I/O)
 - Windows 98SE (ISA HardSID Quattro, winio.dll ISA I/O)
 - Windows 98SE (ISA HardSID, inpout32.dll ISA I/O)
 - Windows 98SE (ISA HardSID Quattro, inpout32.dll ISA I/O)
 - Windows 98SE (ISA HardSID, Direct ISA I/O)
 - Windows 98SE (ISA HardSID Quattro, Direct ISA I/O)
 - Windows ME (ISA HardSID, winio.dll ISA I/O)
 - Windows ME (ISA HardSID Quattro, winio.dll ISA I/O)
 - Windows ME (ISA HardSID, inpout32.dll ISA I/O)
 - Windows ME (ISA HardSID Quattro, inpout32.dll ISA I/O)
 - Windows ME (ISA HardSID, Direct ISA I/O)
 - Windows ME (ISA HardSID Quattro, Direct ISA I/O)
 - Windows NT 3.51 (ISA HardSID, inpout32.dll ISA I/O)
 - Windows NT 3.51 (ISA HardSID Quattro, inpout32.dll ISA I/O)
 - Windows NT 4 (ISA HardSID, winio32.dll ISA I/O)
 - Windows NT 4 (ISA HardSID Quattro, winio32.dll ISA I/O)
 - Windows NT 4 (ISA HardSID, inpout32.dll ISA I/O)
 - Windows NT 4 (ISA HardSID Quattro, inpout32.dll ISA I/O)
 - Windows 2000 (ISA HardSID, winio32.dll ISA I/O)
 - Windows 2000 (ISA HardSID Quattro, winio32.dll ISA I/O)
 - Windows 2000 (ISA HardSID, inpout32.dll ISA I/O)
 - Windows 2000 (ISA HardSID Quattro, inpout32.dll ISA I/O)
 - Windows XP (ISA HardSID, winio32.dll ISA I/O)
 - Windows XP (ISA HardSID Quattro, winio32.dll ISA I/O)
 - Windows XP (ISA HardSID, inpout32.dll ISA I/O)
 - Windows XP (ISA HardSID Quattro, inpout32.dll ISA I/O)
 - Windows 2003 Server (ISA HardSID, winio32.dll ISA I/O)
 - Windows 2003 Server (ISA HardSID Quattro, winio32.dll ISA I/O)
 - Windows 2003 Server (ISA HardSID, inpout32.dll ISA I/O)
 - Windows 2003 Server (ISA HardSID Quattro, inpout32.dll ISA I/O)
 */


#include "vice.h"

#ifdef WIN32_COMPILE

#ifdef HAVE_HARDSID
#include <windows.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "alarm.h"
#include "archdep.h"
#include "hardsid.h"
#include "hs-win32.h"
#include "log.h"
#include "sid-resources.h"
#include "types.h"
#include "wininpoutp.h"

#define HARDSID_BASE 0x300

#define MAXSID 4

static int sids_found = -1;
static int hssids[MAXSID] = {-1, -1, -1, -1};

static int hardsid_use_lib = 0;
static int hardsid_use_inpout_dll = 0;
static int hardsid_use_winio_dll = 0;

#ifndef MSVC_RC
typedef short _stdcall (*inpout_inpfuncPtr)(short portaddr);
typedef void _stdcall (*inpout_oupfuncPtr)(short portaddr, short datum);

typedef int _stdcall (*initfuncPtr)(void);
typedef void _stdcall (*shutdownfuncPtr)(void);
typedef int _stdcall (*winio_inpfuncPtr)(WORD port, PDWORD value, BYTE size);
typedef int _stdcall (*winio_oupfuncPtr)(WORD port, DWORD value, BYTE size);
#else
typedef short (CALLBACK* inpout_inpfuncPtr)(short);
typedef void (CALLBACK* inpout_oupfuncPtr)(short, short);

typedef int (CALLBACK* initfuncPtr)(void);
typedef void (CALLBACK* shutdownfuncPtr)(void);
typedef int (CALLBACK* winio_inpfuncPtr)(WORD, PDWORD, BYTE);
typedef int (CALLBACK* winio_oupfuncPtr)(WORD, DWORD, BYTE);
#endif

static inpout_inpfuncPtr inpout_inp32fp;
static inpout_oupfuncPtr inpout_oup32fp;

static initfuncPtr init32fp;
static shutdownfuncPtr shutdown32fp;
static winio_inpfuncPtr winio_inp32fp;
static winio_oupfuncPtr winio_oup32fp;

/* input/output functions */
static void hardsid_outb(unsigned int addrint, BYTE value)
{
    WORD addr = (WORD)addrint;

    /* make sure the above conversion did not loose any details */
    assert(addr == addrint);

    if (hardsid_use_lib) {
        if (hardsid_use_winio_dll) {
            winio_oup32fp(addr, (DWORD)value, 1);
        } else {
            inpout_oup32fp(addr, (WORD)value);
        }
    } else {
#ifdef  _M_IX86
#ifdef WATCOM_COMPILE
        outp(addr, value);
#else
        _outp(addr, value);
#endif
#endif
    }
}

static BYTE hardsid_inb(unsigned int addrint)
{
    WORD addr = (WORD)addrint;
    DWORD tmp;
    BYTE retval = 0;

    /* make sure the above conversion did not loose any details */
    assert(addr == addrint);

    if (hardsid_use_lib) {
        if (hardsid_use_winio_dll) {
            winio_inp32fp(addr, &tmp, 1);
            retval = (BYTE)tmp;
        } else {
            retval = (BYTE)inpout_inp32fp(addr);
        }
    } else {
#ifdef  _M_IX86
#ifdef WATCOM_COMPILE
        retval = inp(addr);
#else
        retval = _inp(addr);
#endif
#endif
    }
    return retval;
}

int hs_isa_read(uint16_t addr, int chipno)
{
    if (chipno < MAXSID && hssids[chipno] != -1 && addr < 0x20) {
        hardsid_outb(HARDSID_BASE + 1, (BYTE)((chipno << 6) | (addr & 0x1f) | 0x20));
        vice_usleep(2);
        return hardsid_inb(HARDSID_BASE);
    }
    return 0;
}

void hs_isa_store(uint16_t addr, uint8_t outval, int chipno)
{
    if (chipno < MAXSID && hssids[chipno] != -1 && addr < 0x20) {
        hardsid_outb(HARDSID_BASE, outval);
        hardsid_outb(HARDSID_BASE + 1, (BYTE)((chipno << 6) | (addr & 0x1f)));
        vice_usleep(2);
    }
}

/*----------------------------------------------------------------------*/

static HINSTANCE hLib = NULL;

#ifdef _MSC_VER
#  ifdef _WIN64
#    define INPOUTDLLNAME "inpoutx64.dll"
#    define WINIODLLNAME  "winio64.dll"
#  else
#    define INPOUTDLLNAME "inpout32.dll"
#    define WINIODLLNAME  "winio32.dll"
#    define WINIOOLDNAME  "winio.dll"
#  endif
#else
#  if defined(__amd64__) || defined(__x86_64__)
#    define INPOUTDLLNAME "inpoutx64.dll"
#    define WINIODLLNAME  "winio64.dll"
#  else
#    define INPOUTDLLNAME "inpout32.dll"
#    define WINIODLLNAME  "winio32.dll"
#    define WINIOOLDNAME  "winio.dll"
#  endif
#endif

static int detect_sid_uno(void)
{
    int i;
    int j;

    for (j = 0; j < 4; ++j) {
        for (i = 0x18; i >= 0; --i) {
            hs_isa_store((WORD)i, 0, j);
        }
    }

    hs_isa_store(0x12, 0xff, 0);

    for (i = 0; i < 100; ++i) {
        if (hs_isa_read(0x1b, 3)) {
            return 0;
        }
    }

    hs_isa_store(0x0e, 0xff, 0);
    hs_isa_store(0x0f, 0xff, 0);
    hs_isa_store(0x12, 0x20, 0);

    for (i = 0; i < 100; ++i) {
        if (hs_isa_read(0x1b, 3)) {
            return 1;
        }
    }
    return 0;
}

static int detect_sid(int chipno)
{
    int i;

    for (i = 0x18; i >= 0; --i) {
        hs_isa_store((WORD)i, 0, chipno);
    }

    hs_isa_store(0x12, 0xff, chipno);

    for (i = 0; i < 100; ++i) {
        if (hs_isa_read(0x1b, chipno)) {
            return 0;
        }
    }

    hs_isa_store(0x0e, 0xff, chipno);
    hs_isa_store(0x0f, 0xff, chipno);
    hs_isa_store(0x12, 0x20, chipno);

    for (i = 0; i < 100; ++i) {
        if (hs_isa_read(0x1b, chipno)) {
            return 1;
        }
    }
    return 0;
}

static void close_device(void)
{
    if (hardsid_use_lib) {
        if (hardsid_use_winio_dll) {
            shutdown32fp();
        }
        FreeLibrary(hLib);
        hLib = NULL;
        hardsid_use_winio_dll = 0;
        hardsid_use_inpout_dll = 0;
    }
}

int hs_isa_open(void)
{
    int i;
    char *libname = NULL;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting ISA HardSID boards.");

#ifdef WINIOOLDNAME
    if (hLib == NULL) {
        libname = WINIOOLDNAME;
        hLib = LoadLibrary(libname);
        hardsid_use_inpout_dll = 0;
        hardsid_use_winio_dll = 1;
    }
#endif

    if (hLib == NULL) {
        libname = WINIODLLNAME;
        hLib = LoadLibrary(libname);
        hardsid_use_inpout_dll = 0;
        hardsid_use_winio_dll = 1;
    }

    if (hLib == NULL) {
        libname = INPOUTDLLNAME;
        hLib = LoadLibrary(libname);
        hardsid_use_inpout_dll = 1;
        hardsid_use_winio_dll = 0;
    }

    hardsid_use_lib = 0;

    if (hLib != NULL) {
        log_message(LOG_DEFAULT, "Opened %s.", libname);

        if (hardsid_use_inpout_dll) {
            inpout_inp32fp = (inpout_inpfuncPtr)GetProcAddress(hLib, "Inp32");
            if (inpout_inp32fp != NULL) {
                inpout_oup32fp = (inpout_oupfuncPtr)GetProcAddress(hLib, "Out32");
                if (inpout_oup32fp != NULL) {
                    log_message(LOG_DEFAULT, "Using %s for ISA I/O access.", libname);
                    hardsid_use_lib = 1;
                }
            }
        } else {
            winio_inp32fp = (winio_inpfuncPtr)GetProcAddress(hLib, "GetPortVal");
            if (winio_inp32fp != NULL) {
                winio_oup32fp = (winio_oupfuncPtr)GetProcAddress(hLib, "SetPortVal");
                if (winio_oup32fp != NULL) {
                    init32fp = (initfuncPtr)GetProcAddress(hLib, "InitializeWinIo");
                    if (init32fp != NULL) {
                        shutdown32fp = (shutdownfuncPtr)GetProcAddress(hLib, "ShutdownWinIo");
                        if (shutdown32fp != NULL) {
                            if (init32fp()) {
                                log_message(LOG_DEFAULT, "Using %s for ISA I/O access.", libname);
                                hardsid_use_lib = 1;
                            } else {
                                log_message(LOG_DEFAULT, "Cannot init %s.", libname);
                            }
                        }
                    }
                }
            }
        }
        if (!hardsid_use_lib) {
            log_message(LOG_DEFAULT, "Cannot get I/O functions in %s, using direct I/O access.", libname);
        }
    } else {
        log_message(LOG_DEFAULT, "Cannot open %s, trying direct ISA I/O access.", libname);
    }

    if (!(GetVersion() & 0x80000000) && hardsid_use_lib == 0) {
        log_message(LOG_DEFAULT, "Cannot use direct I/O access on Windows NT/2000/Server/XP/Vista/7/8/10.");
        return -1;
    }

    for (i = 0; i < MAXSID; ++i) {
        hssids[sids_found] = i;
        if (detect_sid(i)) {
            sids_found++;
        }
    }

    if (!sids_found) {
        log_message(LOG_DEFAULT, "No ISA HardSID found.");
        close_device();
        return -1;
    }

    /* Check for classic HardSID if 4 SIDs were found. */
    if (sids_found == 4) {
        if (detect_sid_uno()) {
            sids_found = 1;
        }
    }

    log_message(LOG_DEFAULT, "ISA HardSID: opened, found %d SIDs.", sids_found);

    return 0;
}

int hs_isa_close(void)
{
    int i;

    close_device();

    for (i = 0; i < MAXSID; ++i) {
        if (hssids[i] != -1) {
            hssids[i] = -1;
        }
    }

    sids_found = -1;

    log_message(LOG_DEFAULT, "ISA HardSID: closed");

    return 0;
}

int hs_isa_available(void)
{
    return sids_found;
}

/* ---------------------------------------------------------------------*/

void hs_isa_state_read(int chipno, struct sid_hs_snapshot_state_s *sid_state)
{
    sid_state->hsid_main_clk = 0;
    sid_state->hsid_alarm_clk = 0;
    sid_state->lastaccess_clk = 0;
    sid_state->lastaccess_ms = 0;
    sid_state->lastaccess_chipno = 0;
    sid_state->chipused = 0;
    sid_state->device_map[0] = 0;
    sid_state->device_map[1] = 0;
    sid_state->device_map[2] = 0;
    sid_state->device_map[3] = 0;
}

void hs_isa_state_write(int chipno, struct sid_hs_snapshot_state_s *sid_state)
{
}
#endif
#endif

