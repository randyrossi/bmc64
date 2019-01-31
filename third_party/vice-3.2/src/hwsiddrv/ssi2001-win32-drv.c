/*
 * ssi2001-win32-drv.c - SSI2001 (ISA SID card) support for WIN32.
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

 - Windows 95C (Direct ISA I/O, inpout32.dll is incompatible with windows 95)
 - Windows 98SE (winio.dll ISA I/O)
 - Windows 98SE (inpout32.dll ISA I/O)
 - Windows 98SE (Direct ISA I/O)
 - Windows ME (winio.dll ISA I/O)
 - Windows ME (inpout32.dll ISA I/O)
 - Windows ME (Direct ISA I/O)
 - Windows NT 3.51 (inpout32.dll ISA I/O)
 - Windows NT 4.0 (winio32.dll ISA I/O)
 - Windows NT 4.0 (inpout32.dll ISA I/O)
 - Windows 2000 (winio32.dll ISA I/O)
 - Windows 2000 (inpout32.dll ISA I/O)
 - Windows XP (winio32.dll ISA I/O)
 - Windows XP (inpout32.dll ISA I/O)
 - Windows 2003 Server (winio32.dll ISA I/O)
 - Windows 2003 Server (inpout32.dll ISA I/O)
 */

#include "vice.h"

#ifdef WIN32_COMPILE

#ifdef HAVE_SSI2001
#include <windows.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>

#include "log.h"
#include "types.h"
#include "wininpoutp.h"

#include "ssi2001.h"

#define SSI2008_BASE 0x280

static int ssi2001_use_lib = 0;
static int ssi2001_use_inpout_dll = 0;
static int ssi2001_use_winio_dll = 0;

#define MAXSID 1

static int sids_found = -1;

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
static void ssi2001_outb(unsigned int addrint, short value)
{
    WORD addr = (WORD)addrint;

    /* make sure the above conversion did not loose any details */
    assert(addr == addrint);

    if (ssi2001_use_lib) {
        if (ssi2001_use_winio_dll) {
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

static BYTE ssi2001_inb(unsigned int addrint)
{
    WORD addr = (WORD)addrint;
    DWORD tmp;
    BYTE retval = 0;

    /* make sure the above conversion did not loose any details */
    assert(addr == addrint);

    if (ssi2001_use_lib) {
        if (ssi2001_use_winio_dll) {
            winio_inp32fp(addr, &tmp, 1);
            retval = (BYTE)tmp;
        } else {
            retval = (BYTE)inpout_inp32fp(addr);
        }
    } else {
#ifdef  _M_IX86
#ifdef WATCOM_COMPILE
        return inp(addr);
#else
        return _inp(addr);
#endif
#endif
    }
    return retval;
}

int ssi2001_drv_read(uint16_t addr, int chipno)
{
    if (chipno < MAXSID && addr < 0x20) {
        return ssi2001_inb(SSI2008_BASE + (addr & 0x1f));
    }
    return 0;
}

void ssi2001_drv_store(uint16_t addr, uint8_t outval, int chipno)
{
    if (chipno < MAXSID && addr < 0x20) {
        ssi2001_outb(SSI2008_BASE + (addr & 0x1f), outval);
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

static int detect_sid(void)
{
    int i;

    for (i = 0x18; i >= 0; --i) {
        ssi2001_drv_store((WORD)i, 0, 0);
    }

    ssi2001_drv_store(0x12, 0xff, 0);

    for (i = 0; i < 100; ++i) {
        if (ssi2001_drv_read(0x1b, 0)) {
            return 0;
        }
    }

    ssi2001_drv_store(0x0e, 0xff, 0);
    ssi2001_drv_store(0x0f, 0xff, 0);
    ssi2001_drv_store(0x12, 0x20, 0);

    for (i = 0; i < 100; ++i) {
        if (ssi2001_drv_read(0x1b, 0)) {
            return 1;
        }
    }
    return 0;
}

static void close_device(void)
{
    if (ssi2001_use_lib) {
        if (ssi2001_use_winio_dll) {
            shutdown32fp();
        }
        FreeLibrary(hLib);
        hLib = NULL;
        ssi2001_use_winio_dll = 0;
        ssi2001_use_inpout_dll = 0;
    }
}

int ssi2001_drv_open(void)
{
    char *libname = NULL;

    if (!sids_found) {
        return -1;
    }

    if (sids_found > 0) {
        return 0;
    }

    sids_found = 0;

    log_message(LOG_DEFAULT, "Detecting ISA SSI2001 boards.");

#ifdef WINIOOLDNAME
    if (hLib == NULL) {
        libname = WINIOOLDNAME;
        hLib = LoadLibrary(libname);
        ssi2001_use_inpout_dll = 0;
        ssi2001_use_winio_dll = 1;
    }
#endif

    if (hLib == NULL) {
        libname = WINIODLLNAME;
        hLib = LoadLibrary(libname);
        ssi2001_use_inpout_dll = 0;
        ssi2001_use_winio_dll = 1;
    }

    if (hLib == NULL) {
        libname = INPOUTDLLNAME;
        hLib = LoadLibrary(libname);
        ssi2001_use_inpout_dll = 1;
        ssi2001_use_winio_dll = 0;
    }

    ssi2001_use_lib = 0;

    if (hLib != NULL) {
        log_message(LOG_DEFAULT, "Opened %s.", libname);

        if (ssi2001_use_inpout_dll) {
            inpout_inp32fp = (inpout_inpfuncPtr)GetProcAddress(hLib, "Inp32");
            if (inpout_inp32fp != NULL) {
                inpout_oup32fp = (inpout_oupfuncPtr)GetProcAddress(hLib, "Out32");
                if (inpout_oup32fp != NULL) {
                    log_message(LOG_DEFAULT, "Using %s for ISA I/O access.", libname);
                    ssi2001_use_lib = 1;
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
                                ssi2001_use_lib = 1;
                            } else {
                                log_message(LOG_DEFAULT, "Cannot init %s.", libname);
                            }
                        }
                    }
                }
            }
        }
        if (!ssi2001_use_lib) {
            log_message(LOG_DEFAULT, "Cannot get I/O functions in %s, using direct I/O access.", libname);
        }
    } else {
        log_message(LOG_DEFAULT, "Cannot open %s, trying direct ISA I/O access.", libname);
    }

    if (!(GetVersion() & 0x80000000) && ssi2001_use_lib == 0) {
        log_message(LOG_DEFAULT, "Cannot use direct I/O access on Windows NT/2000/Server/XP/Vista/7/8/10.");
        return -1;
    }

    if (detect_sid()) {
        sids_found = 1;
        log_message(LOG_DEFAULT, "ISA SSI2001 SID: opened.");
        return 0;
    }

    log_message(LOG_DEFAULT, "No ISA SSI2001 found.");

    close_device();

    return -1;
}

int ssi2001_drv_close(void)
{
    close_device();

    sids_found = -1;

    log_message(LOG_DEFAULT, "ISA SSI2001 SID: closed.");

    return 0;
}

int ssi2001_drv_available(void)
{
    return sids_found;
}
#endif
#endif
