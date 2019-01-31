/*
 * uimon.c - Monitor access interface.
 *
 * Written by
 *  Spiro Trikaliotis <Spiro.Trikaliotis@gmx.de>
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

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <windows.h>
#include <commctrl.h>

#include "archdep.h"
#include "console.h"
#include "debug.h"
#include "fullscrn.h"
#include "lib.h"
#include "mon_register.h"
#include "mon_ui.h"
#include "mon_util.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uiapi.h"
#include "uimon.h"
#include "uimonmsg.h"
#include "winlong.h"
#include "winmain.h"

// #define DEBUG_UIMON

/* Debugging stuff.  */
#ifdef DEBUG_UIMON

#include "log.h"

static void uimon_debug(const char *format, ...)
{
    char *buffer;
    va_list args;

    va_start(args, format);
    buffer = lib_mvsprintf(format, args);
    va_end(args);
    OutputDebugString(buffer);
    log_message(LOG_DEFAULT,buffer);
    printf(buffer);
    lib_free(buffer);
}
#define UIM_DEBUG(x) uimon_debug x
#else
#define UIM_DEBUG(x)
#endif

#define UIMON_EXPERIMENTAL 1

static console_t *console_log_local = NULL;

#ifdef UIMON_EXPERIMENTAL

   #define OPEN_DISASSEMBLY_AS_POPUP 1
   #define OPEN_MEMORY_AS_POPUP 1
   #define OPEN_REGISTRY_AS_POPUP 1

static HWND hwndConsole = NULL;
static HWND hwndMdiClient = NULL;
static HWND hwndToolbar = NULL;
static HWND hwndMonitor = NULL;

typedef struct window_data_s window_data_t;

typedef LRESULT CALLBACK internal_window_procedure_t(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, window_data_t * window_data);

static internal_window_procedure_t reg_window_proc;
static internal_window_procedure_t dis_window_proc;
static internal_window_procedure_t mem_window_proc;

typedef enum window_type_e {
    WT_END = 0,
    WT_CONSOLE,
    WT_DISASSEMBLY,
    WT_REGISTER,
    WT_MEMORY
} window_type_t;

typedef struct window_data_extra_s {
    window_type_t window_type;
    int as_popup;
    MEMSPACE memspace;
} window_data_extra_t;

struct window_data_s {
    WNDPROC default_window_procedure;
    internal_window_procedure_t * window_procedure;
    window_data_extra_t * extra;
    void * private_data;
    /* DO NOT ADD ANYTHING HERE! It is already very large. Use window_data_extra_t instead! */
};

static window_data_t *window_data_create = NULL;

typedef struct reg_private_s {
    int charwidth;
    int charheight;
    unsigned int *LastShownRegs;
    unsigned int RegCount;
    MEMSPACE memspace;
} reg_private_t;

typedef struct dis_private_s
{
    int charwidth;
    int charheight;
    mon_disassembly_private_t mdp;
} dis_private_t;

typedef struct mem_private_s {
    int charwidth;
    int charheight;
    mon_memory_private_t mmp; // this *must* be the first entry!
} mem_private_t;

static unsigned int new_format_with_extra_data;

typedef struct uimon_client_windows_s {
    HWND hwnd;
    struct uimon_client_windows_s *next;
} uimon_client_windows_t;

uimon_client_windows_t *first_client_window = NULL;

static char *pchCommandLine   = NULL;

#define CONTENTS_CLASS MONITOR_CLASS TEXT(":Contents")

static HWND hwndParent = NULL;
static HWND hwndActive = NULL;

static void update_shown(void);

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
static inline void *uimon_icep(void **dest, void *xchg, void *compare)
{
    void *ret;
#ifdef __x86_64__
    __asm__ __volatile__( "lock; cmpxchgq %2,(%1)"
                          : "=a" (ret) : "r" (dest), "r" (xchg), "0" (compare) : "memory" );
#else
    __asm__ __volatile__( "lock; cmpxchgl %2,(%1)"
                          : "=a" (ret) : "r" (dest), "r" (xchg), "0" (compare) : "memory" );
#endif
    return ret;
}

#elif defined(_MSC_VER) && defined(__i386__)
__declspec(naked) void *uimon_icep(void **dest, void *xchg, void *compare)
{
    __asm mov eax, 12[esp];
    __asm mov ecx, 8[esp];
    __asm mov edx, 4[esp];
    __asm lock cmpxchg [edx], ecx;
    __asm ret;
}
#else
#define uimon_icep(x, y, z) InterlockedCompareExchangePointer(x, y, z)
#endif

#if defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__))
static inline void *uimon_iep(void **dest, void *val)
{
    void *ret;
#ifdef __x86_64__
    __asm__ __volatile__( "lock; xchgq %0,(%1)"
                          : "=r" (ret) :"r" (dest), "0" (val) : "memory" );
#else
    __asm__ __volatile__( "lock; xchgl %0,(%1)"
                          : "=r" (ret) : "r" (dest), "0" (val) : "memory" );
#endif
    return ret;
}
#elif defined(_MSC_VER) && defined(__i386__)
__declspec(naked) void *uimon_iep(void **dest, void *val)
{
    __asm mov eax, 8[esp];
    __asm mov edx, 4[esp];
    __asm lock xchg [edx], eax;
    __asm ret;
}
#else
#define uimon_iep(x, y) InterlockedExchangePointer(x, y)
#endif

static void add_client_window( HWND hwnd )
{
    uimon_client_windows_t *new_client = lib_malloc( sizeof * new_client );

    new_client->hwnd = hwnd;
    new_client->next = first_client_window;

    first_client_window = new_client;
}

static void delete_client_window( HWND hwnd )
{
    uimon_client_windows_t *pold = NULL;
    uimon_client_windows_t *p;

    for (p = first_client_window; p; pold = p, p = p->next) {
        if (p->hwnd == hwnd) {
            if (pold) {
                pold->next = p->next;
            } else {
                first_client_window = p->next;
            }

            lib_free(p);
            break;
        }
    }
}

static monitor_interface_t **monitor_interface;
static int count_monitor_interface;

#endif // #ifdef UIMON_EXPERIMENTAL

static console_t console_log_for_mon = { -50, -50, -50 };

void uimon_set_interface(monitor_interface_t **monitor_interface_init, int count)
{
#ifdef UIMON_EXPERIMENTAL
    monitor_interface = monitor_interface_init;
    count_monitor_interface = count;
#endif // #ifdef UIMON_EXPERIMENTAL
}

#ifdef UIMON_EXPERIMENTAL

#define WM_OWNCOMMAND (WM_USER+0x100)
#define WM_CHANGECOMPUTERDRIVE (WM_OWNCOMMAND + 1)
#define WM_GETWINDOWTYPE       (WM_OWNCOMMAND + 2)
#define WM_UPDATEVAL           (WM_OWNCOMMAND + 3)
#define WM_UPDATE              (WM_OWNCOMMAND + 4)
#define WM_SAVE_PARAMETERS     (WM_OWNCOMMAND + 5)
#define WM_RESTORE_PARAMETERS  (WM_OWNCOMMAND + 6)

/*
 The following definitions (RT_TOOLBAR, CToolBarData) are from the MFC sources
*/
#define RT_TOOLBAR MAKEINTRESOURCE(2410)

typedef struct CToolBarData {
    WORD wVersion;
    WORD wWidth;
    WORD wHeight;
    WORD wItemCount;
    WORD aItems[1];
} CToolBarData;

static HWND CreateAToolbar(HWND hwnd)
{
    HRSRC hRes = NULL;
    HGLOBAL hGlobal = NULL;
    HWND hToolbar = NULL;
    CToolBarData* pData = NULL;
    int i;
    int j;

    TBBUTTON *ptbb = NULL;

    do {
        hRes = FindResource( winmain_instance, MAKEINTRESOURCE(IDR_MONTOOLBAR), RT_TOOLBAR );

        if (hRes == NULL) {
            break;
        }

        hGlobal = LoadResource( winmain_instance, hRes );
        if (hGlobal == NULL) {
            break;
        }

        pData = LockResource(hGlobal);
        if (pData == NULL) {
            break;
        }

        if (pData->wVersion != 1) {
            break;
        }

        ptbb = lib_malloc(pData->wItemCount * sizeof * ptbb);
        if (!ptbb) {
            break;
        }

        for (i = j = 0; i < pData->wItemCount; i++) {
            if (pData->aItems[i]) {
                ptbb[i].iBitmap = j++;
                ptbb[i].fsStyle = TBSTYLE_BUTTON;
            } else {
                ptbb[i].iBitmap = 5;
                ptbb[i].fsStyle = TBSTYLE_SEP;
            }

            ptbb[i].idCommand = pData->aItems[i];

            ptbb[i].fsState = TBSTATE_ENABLED;
            ptbb[i].dwData  = 0;
            ptbb[i].iString = j;
        }

        hToolbar = CreateToolbarEx(hwnd,
                                   WS_CHILD,           // WORD ws,
                                   3,                  // UINT wID,
                                   j,                  // int nBitmaps,
                                   winmain_instance,   // HINSTANCE hBMInst,
                                   IDR_MONTOOLBAR,     // UINT wBMID,
                                   ptbb,               // LPCTBBUTTON lpButtons,
                                   pData->wItemCount,  // int iNumButtons,
                                   pData->wWidth,      // int dxButton,
                                   pData->wHeight,     // int dyButton,
                                   pData->wWidth,      // int dxBitmap,
                                   pData->wHeight,     // int dyBitmap,
                                   sizeof(*ptbb)       // UINT uStructSize
                                   );

        if (hToolbar) {
            ShowWindow(hToolbar,SW_SHOW);
        }

    } while (0);

    if (hGlobal != NULL) {
        FreeResource(hGlobal);
    }

    lib_free(ptbb);

    return hToolbar;
}

static HWND iOpenGeneric(HWND hwnd, DWORD dwStyleMDI, DWORD dwStylePopup, int x, int y, int dx, int dy, int as_popup,
                         internal_window_procedure_t *window_procedure, TCHAR *WindowName, window_type_t window_type, size_t private_data_size)
{
    HWND hwndNew;
    window_data_t * window_data = lib_malloc(sizeof * window_data);

    if (!window_data) {
        return 0;
    }

    window_data->extra = lib_malloc(sizeof * window_data->extra);
    window_data->private_data = lib_malloc(private_data_size);
    window_data->window_procedure = window_procedure;
    window_data->extra->as_popup = as_popup;
    window_data->extra->window_type = window_type;

    /* wait until we are the only window to be opened */
    while (uimon_icep((void **)&window_data_create, window_data, NULL)) {
        ui_dispatch_next_event();
    }

    if (as_popup) {
        window_data->default_window_procedure = DefWindowProc;
        hwndNew = CreateWindowEx(WS_EX_TOOLWINDOW, CONTENTS_CLASS, WindowName, dwStylePopup, x, y, dx, dy, hwndMonitor, NULL, winmain_instance, NULL);
    } else {
        MDICREATESTRUCT mcs = { 0 };

        if (x == CW_USEDEFAULT) {
            x = 0;
        }

        if (y == CW_USEDEFAULT) {
            y = 0;
        }

        mcs.szTitle = WindowName;
        mcs.szClass = CONTENTS_CLASS;
        mcs.style = dwStyleMDI;
        mcs.hOwner = winmain_instance;
        mcs.x = x;
        mcs.y = y;
        mcs.cx = dx;
        mcs.cy = dy;
        mcs.lParam = (LPARAM)window_data;

        window_data->default_window_procedure = DefMDIChildProc;

        hwndNew = (HWND)SendMessage(hwndMdiClient, WM_MDICREATE, 0, (LPARAM) &mcs);
    }

    add_client_window(hwndNew);

    if (as_popup) {
        ShowWindow(hwndNew, SW_SHOW);
    }

    return hwndNew;
}

static HWND iOpenDisassembly(HWND hwnd, DWORD dwStyle, int x, int y, int dx, int dy)
{
    return iOpenGeneric(hwnd, dwStyle, WS_OVERLAPPEDWINDOW | WS_VSCROLL | dwStyle, x, y, dx, dy, OPEN_DISASSEMBLY_AS_POPUP, dis_window_proc, TEXT("Disassembly"), WT_DISASSEMBLY, sizeof(dis_private_t));
}

static HWND OpenDisassembly(HWND hwnd)
{
    // @SRT: TODO: Adjust parameter!
    return iOpenDisassembly(hwnd, 0, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300);
}

static HWND iOpenMemory(HWND hwnd, DWORD dwStyle, int x, int y, int dx, int dy)
{
    return iOpenGeneric(hwnd, dwStyle, WS_OVERLAPPEDWINDOW | WS_VSCROLL|dwStyle, x, y, dx, dy, OPEN_MEMORY_AS_POPUP, mem_window_proc, TEXT("Memory"), WT_MEMORY, sizeof(mem_private_t));
}

static HWND OpenMemory(HWND hwnd)
{
    // @SRT: TODO: Adjust parameter!
    return iOpenMemory( hwnd, 0, CW_USEDEFAULT, CW_USEDEFAULT, 300, 300 );
}

static void DestroyMdiWindow(HWND hwndMdiClient, HWND hwndChild)
{
    SendMessage(hwndMdiClient,WM_MDIDESTROY,(WPARAM)hwndChild,0);
}

static HWND iOpenConsole(HWND hwnd, BOOLEAN bOpen, DWORD dwStyle, int x, int y, int dx, int dy)
{
    console_log_for_mon.console_cannot_output = 0;

    if (bOpen) {
        console_log_local = uimon_console_open_mdi("Monitor", &hwndConsole, &hwndParent, &hwndMdiClient, dwStyle, x, y, dx, dy);
    } else {
        DestroyMdiWindow(hwndMdiClient, hwndConsole);

        hwndConsole = NULL;
    }
    return hwndConsole;
}

static HWND OpenConsole(HWND hwnd, BOOLEAN bOpen)
{
    // @SRT: TODO: Adjust parameter!
    return iOpenConsole(hwnd, bOpen, WS_MAXIMIZE, 0, 0, 0, 0);
}

static HWND iOpenRegistry(HWND hwnd, DWORD dwStyle, int x, int y, int dx, int dy)
{
    return iOpenGeneric(hwnd, dwStyle, WS_CAPTION | WS_POPUPWINDOW | dwStyle, x, y, dx, dy, OPEN_REGISTRY_AS_POPUP, reg_window_proc, TEXT("Register"), WT_REGISTER, sizeof(reg_private_t));
}

static HWND OpenRegistry(HWND hwnd)
{
    // @SRT: TODO: Adjust parameter!
    return iOpenRegistry(hwnd, 0, 30, 100, 100, 100);
}

/**********************************************************************************
***********************************************************************************
***********************************************************************************
***********************************************************************************
**********************************************************************************/

static void writeString3(char **where, BYTE **what, size_t* s)
{
    size_t size = (*s > 3) ? 3 : *s;

    long what1 = (size > 0) ? (*what)[0] : 0;
    long what2 = (size > 1) ? (*what)[1] : 0;
    long what3 = (size > 2) ? (*what)[2] : 0;

    unsigned long value;
    BYTE *b = (BYTE*)&value;

    *what += size;
    *s -= size;

    value = (what1 & 0xC0) << 18;
    value |= (what2 & 0xC0) << 20;
    value |= (what3 & 0xC0) << 22;

    value |= (what1 & 0x3F) << 0;
    value |= (what2 & 0x3F) << 8;
    value |= (what3 & 0x3F) << 16;

    *(*where)++ = 0x3F + *b++;
    *(*where)++ = 0x3F + *b++;
    *(*where)++ = 0x3F + *b++;
    *(*where)++ = 0x3F + *b++;
}

static char *encode(BYTE *content, size_t size)
{
    char *buffer;
    unsigned long prefix = (unsigned long)size;
    int i;
    BYTE *pb;
    size_t n;

    // calculate integrity check
    char *p = (char *)content;
    BYTE xor = (BYTE)((size & 0xFF) ^ (size >> 8));

    if (!content || size == 0) {
       return NULL;
    }

    for (i = (int)size; i; i--) {
        xor ^= *p++;
    }

    prefix |= (((long)xor) << 16); // put integrity check in prefix

    // make string
    buffer = lib_malloc(((size + 2) / 3) * 4 + 5);
    p = buffer;
    pb = (BYTE*)&prefix;
    n = 3;
    writeString3(&p, &pb, &n);

    while (size) {
        writeString3(&p, &content, &size);
    }

    *p = 0;

    return buffer;
}

static BOOLEAN getString3(BYTE **where, const char **what, size_t* s, BYTE* xor)
{
    BOOLEAN ok = TRUE;

    size_t size = (*s > 3) ? 3 : *s;

    unsigned long value;
    BYTE *b = (BYTE*)&value;

    BYTE what1;
    BYTE what2;
    BYTE what3;

    /*
     * What the hell is this supposed to do? Originally these lines looked like:
     *
     * if ((**what < 0x3F) || (**what > 0x7E)) ok = FALSE; *b++ = *(*what)++ - 0x3F;
     *
     * So, either misleadingly indented, or missing braces. And why four times
     * the same statement?
     *
     * -- Compyx
     */
    if ((**what < 0x3F) || (**what > 0x7E)) {
        ok = FALSE;
    }
    *b++ = *(*what)++ - 0x3F;
    if ((**what < 0x3F) || (**what > 0x7E)) {
        ok = FALSE;
    }
    *b++ = *(*what)++ - 0x3F;
    if ((**what < 0x3F) || (**what > 0x7E)) {
        ok = FALSE;
    }
    *b++ = *(*what)++ - 0x3F;
    if ((**what < 0x3F) || (**what > 0x7E)) {
        ok = FALSE;
    }
    *b++ = *(*what)++ - 0x3F;

    // intentionally convert from unsigned long to BYTE
    what1 = (BYTE)(((value >> 0) & 0x3F) | ((value >> 18) & 0xC0));
    what2 = (BYTE)(((value >> 8) & 0x3F) | ((value >> 20) & 0xC0));
    what3 = (BYTE)(((value >> 16) & 0x3F) | ((value >> 22) & 0xC0));

    *s -= size;

    if (size > 0) {
        *xor ^= what1;
        *(*where)++ = what1;
    }

    if (size > 1) {
        *xor ^= what2;
        *(*where)++ = what2;
    }

    if (size > 2) {
        *xor ^= what3;
        *(*where)++ = what3;
    }

    return ok;
}

static BYTE *decode(const char *content, size_t* plen)
{
    BOOLEAN ok = FALSE;
    size_t size;
    BYTE *buffer = NULL;

    if (content) {
        long prefix;

        BYTE xor = 0;
        {
            BYTE *pBYTE = (BYTE*)&prefix;
            size_t n = 3;
            ok = getString3(&pBYTE, &content, &n, &xor);
        }

        if (ok) {
            BYTE *p;
            p = buffer = lib_malloc((strlen(content) * 3) / 4); // @SRT
            xor = (BYTE)(prefix >> 16); // extract checksum from prefix
            *plen = size = (size_t)(prefix & 0xFFFF);  // extract size from prefix

            xor ^= (size & 0xFF) ^ (size >> 8); // include size in checksum

            while (size && ok) {
                ok = getString3(&p, &content, &size, &xor);
            }

            if (xor) {
                ok = FALSE;
            }
        }
    }

    if (!ok) {
        *plen = 0;
    }

    return ok ? buffer : NULL;
}

/**********************************************************************************
***********************************************************************************
***********************************************************************************
***********************************************************************************
**********************************************************************************/

struct WindowDimensions {
    BYTE *pMonitorDimensionsBuffer;
    BYTE *pMonitorDimensions;
    size_t MonitorLen;

    WINDOWPLACEMENT wpPlacement;

    BYTE *extra;
};
typedef struct WindowDimensions WindowDimensions;
typedef WindowDimensions *PWindowDimensions;

static BYTE GetByte(BYTE **p, size_t * len)
{
    --(*len);
    return *(*p)++;
}

static BOOLEAN GetPlacement(BYTE **p, size_t * len, WINDOWPLACEMENT *pwp)
{
    UINT i;

    PBYTE pNext = (PBYTE) pwp;
    for (i = sizeof(WINDOWPLACEMENT); (i > 0) && (*len > 0); i--) {
        *pNext++ = GetByte(p, len);
    }

    return (i == 0) ? FALSE : TRUE;
}

static BYTE *GetExtraData(BYTE **p, size_t * len)
{
    BYTE * extra = NULL;
    size_t extra_length = 0;
    size_t i;

    do {
        if (*len == 0) {
            break;
        }

        extra_length = GetByte(p, len);

        if (extra_length > *len) {
            break;
        }

        extra = lib_malloc(extra_length + 1);

        extra[0] = extra_length; /* FIXME: this is useless */

        for (i = 0; i < extra_length; i++) {
            extra[i] = GetByte(p, len);
        }
    } while (0);

    return extra;
}

static BYTE **WriteByte(BYTE **p, unsigned int a)
{
    *(*p)++ = a;
    return p;
}

static BYTE **WritePlacement(BYTE **p, WINDOWPLACEMENT *pwp)
{
    UINT i;
    PBYTE pNext = (PBYTE) pwp;
    for (i = 0; i <pwp->length; i++) {
        WriteByte(p, *pNext++);
    }

    return p;
}

static void WriteExtraData(BYTE ** p, BYTE * buffer, BYTE len)
{
    size_t i;
    WriteByte(p, len);

    for (i = 0; i < len; i++) {
        WriteByte(p, buffer[i]);
    }
}

static window_type_t GetNextMonitorDimensions(PWindowDimensions pwd)
{
    window_type_t ret;

    if (pwd->MonitorLen == 0) {
        ret = WT_END;
    } else {
        ret = GetByte(&(pwd->pMonitorDimensions), &pwd->MonitorLen);

        if (pwd->MonitorLen < (int)sizeof(WINDOWPLACEMENT)) {
            ret = WT_END;

            UIM_DEBUG(("UIMON.C: pwd->MonitorLen has size %u, should have size of at least %u.", pwd->MonitorLen, sizeof(WINDOWPLACEMENT)));
        } else {
            GetPlacement(&(pwd->pMonitorDimensions), &pwd->MonitorLen, &(pwd->wpPlacement));

            assert(pwd->extra == NULL);

            if (ret != WT_CONSOLE && new_format_with_extra_data) {
                pwd->extra = GetExtraData(&(pwd->pMonitorDimensions), &pwd->MonitorLen);
            }
        }
    }
    return ret;
}

static void SetNextMonitorDimensions(HWND hwnd, window_type_t wt, BYTE **p)
{
    WINDOWPLACEMENT wpPlacement;

    wpPlacement.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(hwnd, &wpPlacement);

    WriteByte(p, (BYTE)wt);

    WritePlacement(p, &wpPlacement);
}

static void OpenFromWindowDimensions(HWND hwnd, PWindowDimensions wd)
{
    window_type_t wt;
    HWND hwndOpened = NULL;

    while ((wt = GetNextMonitorDimensions(wd)) != WT_END) {
        switch (wt) {
            case WT_CONSOLE:
                hwndOpened = iOpenConsole(hwnd, TRUE, 0, 0, 0, 0, 0);
                break;
            case WT_MEMORY:
                hwndOpened = OpenMemory(hwnd);
                break;
            case WT_DISASSEMBLY:
                hwndOpened = OpenDisassembly(hwnd);
                break;
            case WT_REGISTER:
                hwndOpened = OpenRegistry(hwnd);
                break;
            case WT_END:
                /* this cannot occur, but since gcc complains if not specified... */
                hwndOpened = NULL;
        };

        SetWindowPlacement(hwndOpened, &(wd->wpPlacement));

        if (wd->extra != NULL) {
            SendMessage(hwndOpened, WM_RESTORE_PARAMETERS, (WPARAM)wd->extra, 0);
            lib_free(wd->extra);
            wd->extra = NULL;
        }
    }

    lib_free(wd->pMonitorDimensionsBuffer);
    lib_free(wd);
}

static PWindowDimensions LoadMonitorDimensions(HWND hwnd)
{
    PWindowDimensions ret = NULL;

    const char *dimensions;
    BYTE *buffer;
    size_t len;
    BOOLEAN bError = TRUE;

    resources_get_string("MonitorDimensions", &dimensions);
    buffer = decode(dimensions, &len);

    do {
        BYTE *p = buffer;

        if (len == 0 ) {
            break;
        }

        if (len < 8) {
            break;
        }

        if (*p == 0x0) {
            new_format_with_extra_data = 1;
            ++p;
        } else {
            new_format_with_extra_data = 0;
        }

        ret = lib_malloc(sizeof(*ret));
        bError = GetPlacement((BYTE **)(&p), &len, &(ret->wpPlacement));

        if (bError) {
            lib_free(ret);
            ret = NULL;
            break;
        }

        SetWindowPlacement(hwnd, &(ret->wpPlacement));

        ret->extra = NULL;
        if (new_format_with_extra_data) {
            /*
             * read some additional data that might be available.
             * not used now, but might be in the future.
             */
            char *extra_data = (char *)GetExtraData(&p, &len);
            lib_free(extra_data);
        }

        ret->pMonitorDimensionsBuffer = buffer;
        ret->pMonitorDimensions = p;
        ret->MonitorLen = len;

    } while (0);

    if (bError) {
        lib_free(buffer);
    }

    return ret;
}

static VOID iWindowStore(HWND hwnd, BYTE **p)
{
    LONG WindowType = WT_CONSOLE;

    SendMessage(hwnd, WM_GETWINDOWTYPE, 0, (LPARAM) &WindowType);

    SetNextMonitorDimensions(hwnd, WindowType, p);

    if (WindowType != WT_CONSOLE) {
        /* allow every Window to add its own local parameters */
        SendMessage(hwnd, WM_SAVE_PARAMETERS, (WPARAM) p, 0);
    }
}

static int  nHwndStack = 0;
static HWND hwndStack[256]; // @SRT

static VOID WindowStore(BYTE **p)
{
    int HwndStackIterator;

    /* store the dimensions of every Window */
    for (HwndStackIterator = nHwndStack - 1; HwndStackIterator >= 0; --HwndStackIterator) {
        iWindowStore(hwndStack[HwndStackIterator], p);
    }

    nHwndStack = 0;
}

static BOOL CALLBACK WindowStoreProc(HWND hwnd, LPARAM lParam)
{
    hwndStack[nHwndStack++] = hwnd;
    return TRUE;
}

static void StoreMonitorDimensions(HWND hwnd)
{
    char *dimensions;
    static BYTE buffer[1024]; // @SRT
    BYTE *p = buffer;
    uimon_client_windows_t *clients;

    WINDOWPLACEMENT wpPlacement;
    wpPlacement.length = sizeof(WINDOWPLACEMENT);

    GetWindowPlacement(hwnd, &wpPlacement);

    /* set marker: this is the new format, with extra data for the Windows! */
    WriteByte(&p, 0);

    WritePlacement(&p, &wpPlacement);

    WriteExtraData(&p, NULL, 0);

    if (hwndConsole) {
        WindowStoreProc(hwndConsole, 0);
    }
    for (clients = first_client_window; clients; clients = clients->next) {
        WindowStoreProc( clients->hwnd, 0);
    }

    WindowStore(&p);

    dimensions = encode(buffer, p - buffer);
    resources_set_string("MonitorDimensions", dimensions);
    lib_free(dimensions);
}

static void EnableCommands( HMENU hmnu, HWND hwndToolbar )
{
#define ENABLE(xID, xENABLE)                                         \
    EnableMenuItem(hmnu, (xID), (xENABLE) ? MF_ENABLED : MF_GRAYED); \
    SendMessage(hwndToolbar, TB_ENABLEBUTTON, (xID), MAKELONG((xENABLE), 0))

#define CHECK(xID, xON)                                            \
    CheckMenuItem(hmnu, (xID), (xON) ? MF_CHECKED : MF_UNCHECKED); \
    SendMessage(hwndToolbar, TB_CHECKBUTTON, (xID), MAKELONG((xON), 0))

    ENABLE(IDM_MON_OPEN, 0);
    ENABLE(IDM_MON_SAVE, 0);
    ENABLE(IDM_MON_PRINT, 0);
#if 0
    ENABLE(IDM_MON_STOP_DEBUG, 0);
#endif
    ENABLE(IDM_MON_STOP_EXEC, 0);
    ENABLE(IDM_MON_CURRENT, 0);
#if 0
    ENABLE(IDM_MON_STEP_INTO, 1);
    ENABLE(IDM_MON_STEP_OVER, 1);
    ENABLE(IDM_MON_SKIP_RETURN, 1);
#endif
    ENABLE(IDM_MON_GOTO_CURSOR, 0);
    ENABLE(IDM_MON_EVAL, 0);
    ENABLE(IDM_MON_WND_EVAL, 0);
#if 0
    CHECK(IDM_MON_WND_REG, hwndReg ? TRUE : FALSE);
#endif
    ENABLE(IDM_MON_WND_MEM, 1);
#if 0
    CHECK(IDM_MON_WND_DIS, hwndDis ? TRUE : FALSE);
#endif
    CHECK(IDM_MON_WND_CONSOLE, hwndConsole ? TRUE : FALSE);
    ENABLE(IDM_MON_HELP, 0);
#if 0
    ENABLE(IDM_MON_CASCADE, 1);
    ENABLE(IDM_MON_TILE_HORIZ, 1);
    ENABLE(IDM_MON_TILE_VERT, 1);
    ENABLE(IDM_MON_ARRANGE_ICONS, 1);
#endif
}

static WORD SwitchOffUnavailableDrives(WORD ulMask)
{
    int drive_type;
    int drive_true_emulation;

    resources_get_int("DriveTrueEmulation", &drive_true_emulation);

    if (!drive_true_emulation) {
        ulMask &= ~(MDDPC_SET_DRIVE8|MDDPC_SET_DRIVE9|MDDPC_SET_DRIVE10|MDDPC_SET_DRIVE11);
    } else {
        resources_get_int("Drive8Type", &drive_type);
        if (drive_type == 0) {
            ulMask &= ~ MDDPC_SET_DRIVE8;
        }
        resources_get_int("Drive9Type", &drive_type);
        if (drive_type == 0) {
            ulMask &= ~ MDDPC_SET_DRIVE9;
        }
        resources_get_int("Drive10Type", &drive_type);
        if (drive_type == 0) {
            ulMask &= ~ MDDPC_SET_DRIVE10;
        }
        resources_get_int("Drive11Type", &drive_type);
        if (drive_type == 0) {
            ulMask &= ~ MDDPC_SET_DRIVE11;
        }
    }

    return ulMask;
}

static void SetMemspace(HWND hwnd, window_data_t *window_data, MEMSPACE memspace)
{
    BOOL bComputer = FALSE;
    BOOL bDrive8 = FALSE;
    BOOL bDrive9 = FALSE;
    BOOL bDrive10 = FALSE;
    BOOL bDrive11 = FALSE;
    HMENU hmnu = GetMenu(hwnd);
    int drive_true_emulation;
    WORD ulMask = MDDPC_SET_DRIVE8 | MDDPC_SET_DRIVE9 | MDDPC_SET_DRIVE10 | MDDPC_SET_DRIVE11;

    TCHAR *pText = TEXT("");

    window_data->extra->memspace = memspace;

    switch (memspace) {
        case e_comp_space:
            bComputer = TRUE;
            pText = TEXT("Computer");
            break;
        case e_disk8_space:
            bDrive8 = TRUE;
            pText = TEXT("Drive 8");
            break;
        case e_disk9_space:
            bDrive9 = TRUE;
            pText = TEXT("Drive 9");
            break;
        case e_disk10_space:
            bDrive10 = TRUE;
            pText = TEXT("Drive 10");
            break;
        case e_disk11_space:
            bDrive11 = TRUE;
            pText = TEXT("Drive 11");
            break;
        /* 
            these two cases should not occur; 
            they're just there to avoid the warning 
        */
        case e_default_space: /* FALL THROUGH */
        case e_invalid_space:
            break; 
    }

    resources_get_int("DriveTrueEmulation", &drive_true_emulation);

    ulMask = drive_true_emulation ? SwitchOffUnavailableDrives(ulMask) : 0;

    ENABLE(IDM_MON_COMPUTER, 1);
    ENABLE(IDM_MON_DRIVE8, (ulMask & MDDPC_SET_DRIVE8) ? 1 : 0);
    ENABLE(IDM_MON_DRIVE9, (ulMask & MDDPC_SET_DRIVE9) ? 1 : 0);
    ENABLE(IDM_MON_DRIVE10, (ulMask & MDDPC_SET_DRIVE10) ? 1 : 0);
    ENABLE(IDM_MON_DRIVE11, (ulMask & MDDPC_SET_DRIVE11) ? 1 : 0);
    CHECK(IDM_MON_COMPUTER, bComputer);
    CHECK(IDM_MON_DRIVE8, drive_true_emulation ? bDrive8 : FALSE);
    CHECK(IDM_MON_DRIVE9, drive_true_emulation ? bDrive9 : FALSE);
    CHECK(IDM_MON_DRIVE10, drive_true_emulation ? bDrive10 : FALSE);
    CHECK(IDM_MON_DRIVE11, drive_true_emulation ? bDrive11 : FALSE);

    if (drive_true_emulation) {
        TCHAR pOldText[256];
        int  n = GetWindowText(hwnd, pOldText, 256);

        if (n != 0) {
            TCHAR *pWrite = _tcschr(pOldText, TEXT(':'));
            if (pWrite == NULL) {
                pWrite = _tcschr(pOldText, 0);
                *pWrite = TEXT(':');
            }
        
            _tcscpy(++pWrite, pText);
            SetWindowText(hwnd, pOldText);
        }
    }
}

static void ClearMemspace(HWND hwnd)
{
    HMENU hmnu = GetMenu(hwnd);

    ENABLE(IDM_MON_COMPUTER, 0);
    ENABLE(IDM_MON_DRIVE8, 0);
    ENABLE(IDM_MON_DRIVE9, 0);
    ENABLE(IDM_MON_DRIVE10, 0);
    ENABLE(IDM_MON_DRIVE11, 0);
    CHECK(IDM_MON_COMPUTER, FALSE);
    CHECK(IDM_MON_DRIVE8, FALSE);
    CHECK(IDM_MON_DRIVE9, FALSE);
    CHECK(IDM_MON_DRIVE10, FALSE);
    CHECK(IDM_MON_DRIVE11, FALSE);
}

static void uimon_after_set_command(void)
{
    if (hwndConsole) {
        SendMessage(hwndConsole, WM_CONSOLE_INSERTLINE, 0, 0);
    }
}

#define SET_COMMAND(_cmd) mon_set_command(console_log_local, _cmd, uimon_after_set_command)

static void ResizeMdiClient(HWND hwnd)
{
    RECT rect;

    if (hwndMdiClient) {
        WORD wHeightToolbar = 0;

        if (hwndToolbar) {
            GetWindowRect(hwndToolbar, &rect);
            wHeightToolbar = (WORD)(rect.bottom - rect.top);
        }

        GetClientRect(hwnd, &rect);

        MoveWindow(hwndMdiClient, rect.left, rect.top + wHeightToolbar, rect.right - rect.left, rect.bottom - rect.top - wHeightToolbar, TRUE);
    }
}

#define MAX(_x, _y) ((_x) > (_y) ? (_x) : (_y))

static BOOL CALLBACK WindowMinsizeProc(HWND hwnd, LPARAM lParam)
{
    LPPOINT minDimen = (LPPOINT)lParam;
    MINMAXINFO mmi;

    memset(&mmi, 0, sizeof(mmi));

    if (SendMessage(hwnd, WM_GETMINMAXINFO, 0, (LPARAM)&mmi) == 0) {
        minDimen->x = MAX(minDimen->x, mmi.ptMinTrackSize.x);
        minDimen->y = MAX(minDimen->y, mmi.ptMinTrackSize.y);
    }
    return TRUE;
}

static void MinsizeMdiClient(HWND hwnd, POINT *pMin)
{
    memset(pMin, 0, sizeof(*pMin));

    if (hwndMdiClient) {
        RECT rectWindow;
        RECT rectClient;

        memset(pMin, 0, sizeof(*pMin));

        /* get the minimum size of all the MDI windows */
        EnumChildWindows(hwndMdiClient, (WNDENUMPROC)WindowMinsizeProc, (LPARAM)pMin);

        /* now, add our surrounding box to this */
        GetWindowRect(hwnd, &rectWindow);
        GetClientRect(hwnd, &rectClient);

        pMin->x += (rectWindow.right - rectWindow.left) - (rectClient.right - rectClient.left);
        pMin->y += (rectWindow.bottom - rectWindow.top) - (rectClient.bottom - rectClient.top);
    }
}

static void OnConsoleResize(void)
{
    if (console_log_local) {
        memcpy(&console_log_for_mon, console_log_local, sizeof(struct console_s));
    }
}

static void OnCommand(HWND hwnd, WORD wNotifyCode, WORD wID, HWND hwndCtrl)
{
    switch (wID) {
        case IDM_MON_CASCADE:
            SendMessage(hwndMdiClient, WM_MDICASCADE, 0, 0);
            break;
        case IDM_MON_TILE_HORIZ:
            SendMessage(hwndMdiClient, WM_MDITILE, MDITILE_HORIZONTAL, 0);
            break;
        case IDM_MON_TILE_VERT:
            SendMessage(hwndMdiClient, WM_MDITILE, MDITILE_VERTICAL, 0);
            break;
        case IDM_MON_ARRANGE_ICONS:
            SendMessage(hwndMdiClient, WM_MDIICONARRANGE, 0, 0);
            break;
        case IDM_EXIT:
            /* FALL THROUGH */
        case IDM_MON_STOP_DEBUG:
            SET_COMMAND("x");
            break;
        case IDM_MON_STEP_INTO:
            SET_COMMAND("z");
            break;
        case IDM_MON_STEP_OVER:
            SET_COMMAND("n");
            break;
        case IDM_MON_SKIP_RETURN:
            SET_COMMAND("ret");
            break;
        case IDM_MON_WND_MEM:
            OpenMemory(hwnd);
            break;
        case IDM_MON_WND_DIS:
            OpenDisassembly(hwnd);
            break;
        case IDM_MON_WND_REG:
            OpenRegistry(hwnd);
            break;
        case IDM_MON_WND_CONSOLE:
            OpenConsole(hwnd,(BOOLEAN)(hwndConsole ? FALSE : TRUE));
            EnableCommands(GetMenu(hwnd), hwndToolbar);
            break;
        case IDM_MON_COMPUTER: /* FALL THROUGH */
        case IDM_MON_DRIVE8: /* FALL THROUGH */
        case IDM_MON_DRIVE9: /* FALL THROUGH */
        case IDM_MON_DRIVE10: /* FALL THROUGH */
        case IDM_MON_DRIVE11:
            if (hwndActive) {
                SendMessage(hwndActive, WM_CHANGECOMPUTERDRIVE, wID, 0);
            }
            break;
    }
}

/* window procedure */
static LRESULT CALLBACK mon_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
        case WM_CLOSE:
            SET_COMMAND("x");
            /*
            return 0 so we don't use DefFrameProc(), so we're sure 
            we do exactly the same as we would do if "x" was
            entered from the keyboard.
            */
            return 0;
        case WM_DESTROY:
            return DefFrameProc(hwnd, hwndMdiClient, msg, wParam, lParam);
        case WM_CONSOLE_RESIZED:
            OnConsoleResize();
            return 0;
        case WM_CONSOLE_ACTIVATED:
            hwndActive = NULL;
            ClearMemspace(hwnd);
            return 0;
        case WM_GETMINMAXINFO:
            {
                LPMINMAXINFO lpmmi; 
                POINT minSize;

                DefWindowProc(hwnd, msg, wParam, lParam);

                /* adjust: minimum size */
                MinsizeMdiClient(hwnd, &minSize);

                lpmmi = (LPMINMAXINFO) lParam; // address of structure 

                lpmmi->ptMinTrackSize.x = lpmmi->ptMinTrackSize.x + minSize.x;
                lpmmi->ptMinTrackSize.y = lpmmi->ptMinTrackSize.y + minSize.y;
            }
            return 0;
        case WM_CONSOLE_CLOSED:
            console_log_local = NULL;
            hwndConsole = NULL;
            EnableCommands(GetMenu(hwnd), hwndToolbar);
            return 0;
        case WM_SIZE:
            {
                if (wParam != SIZE_MINIMIZED) {
                    // Tell the toolbar to resize itself to fill the top of the window.
                    if (hwndToolbar) {
                        SendMessage(hwndToolbar, TB_AUTOSIZE, 0L, 0L);
                    }
                    ResizeMdiClient(hwnd);
                }
            }
            /* do not call default - it will reposition the MDICLIENT */
            return 0;
        case WM_CREATE:
            hwndToolbar = CreateAToolbar(hwnd);

            EnableCommands(GetMenu(hwnd),hwndToolbar);
            ui_translate_monitor_menu(GetMenu(hwnd));
            {
                CLIENTCREATESTRUCT ccs;
                ccs.hWindowMenu  = GetSubMenu(GetMenu(hwnd), 3);
                ccs.idFirstChild = IDM_WNDCHILD;
                hwndMdiClient = CreateWindow(TEXT("MdiClient"), NULL, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VSCROLL | WS_HSCROLL,
                                             0, 0, 0, 0, hwnd, (HMENU)0xCAC, winmain_instance, (LPSTR)&ccs);

                ResizeMdiClient(hwnd);
                ShowWindow( hwndMdiClient, SW_SHOW );
            }

            ClearMemspace(hwnd);
            break;
        case WM_COMMAND:
            OnCommand(hwnd, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
            break;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;

                BeginPaint(hwnd, &ps);
                EndPaint(hwnd, &ps);

                return 0;
            }
    }

    return DefFrameProc(hwnd, hwndMdiClient, msg, wParam, lParam);
}

static void ActivateChild(BOOL bActivated, HWND hwndOwn, window_data_t *window_data)
{
    if (bActivated) {
        // we are activated
        hwndActive = hwndOwn;
        SetMemspace(hwndOwn, window_data, window_data->extra->memspace);
    } else {
        // we are deactivated
        hwndActive = NULL;
        ClearMemspace(hwndOwn);
    }
}

static void update_last_shown_regs(reg_private_t *prp)
{
    mon_reg_list_t *pMonRegs = mon_register_list_get(prp->memspace);
    mon_reg_list_t *p;
    unsigned int cnt;

    if (prp->LastShownRegs != NULL) {
        for (p = pMonRegs, cnt = 0; p->name != NULL; p++, cnt++ ) {
            if (cnt < prp->RegCount) {
                prp->LastShownRegs[cnt] = p->val;
            }
        }
    }

    lib_free(pMonRegs);
}

static BOOLEAN output_register(HDC hdc, reg_private_t *prp, RECT *clientrect)
{
    mon_reg_list_t *pMonRegs = mon_register_list_get(prp->memspace);
    mon_reg_list_t *p;

    BOOLEAN changed_dimensions = FALSE;
    int x = 0;
    unsigned int cnt;

    if (prp->LastShownRegs == NULL) {
        // initialize the values which have been last shown
        for (p = pMonRegs, cnt = 0; p->name != NULL; p++, cnt++) {
        }

        prp->RegCount = cnt;
        prp->LastShownRegs = lib_malloc(sizeof(*prp->LastShownRegs) * cnt);

        // ensure that ALL registers appear changed this time!
        for (p = pMonRegs, cnt = 0; p->name != NULL; p++) {
            prp->LastShownRegs[cnt++] = ~(p->val);
        }
    }

    // clear client area of window
    {
        HGDIOBJ hg = SelectObject(hdc, GetStockObject(NULL_PEN));
        Rectangle(hdc, clientrect->left, clientrect->top, clientrect->right, clientrect->bottom);
        SelectObject(hdc, hg);
    }

    for (p = pMonRegs, cnt = 0; p->name != NULL; p++, cnt++)
    {
        TCHAR st_buf[12];
        int namelen = (int)strlen(p->name);
        int center = 0;
        int vallen;

        int changedbits = 0;

        if (cnt < prp->RegCount) {
            changedbits = prp->LastShownRegs[cnt] ^ p->val;
        }

        if (p->flags & MON_REGISTER_IS_FLAGS) {
            unsigned int i;
            unsigned int val = p->val << (16 - p->size);
            unsigned int changed_i = changedbits << (16 - p->size);

            for (i = 0;i < p->size; i++) {
                TCHAR pw = val & 0x8000 ? TEXT('1') : TEXT('0');
                BOOL changed = changed_i & 0x8000 ? TRUE : FALSE;

                val <<= 1;
                changed_i <<= 1;

                // output value of register
                SetTextColor(hdc, RGB(changed ? 0xFF : 0, 0, 0));
                TextOut(hdc, (x + i) * prp->charwidth, prp->charheight, &pw, 1);
            }

            vallen = p->size;
        } else {
            switch (p->size) {
                case 8:
                   vallen = 2;
                    _stprintf(st_buf, TEXT("%02X "), p->val);
                    break;
                case 16:
                    vallen = 4;
                    _stprintf(st_buf, TEXT("%04X "), p->val );
                    break;
                default:
                    vallen = namelen;
                    st_buf[0] = 0;
                    break;
            }

            // output value of register
            SetTextColor(hdc, RGB(changedbits ? 0xFF : 0, 0, 0));
            TextOut(hdc, x * prp->charwidth, prp->charheight, st_buf, vallen);
        }

        center = (vallen - namelen) / 2;

        // output name of register
        SetTextColor(hdc, RGB(0, 0, 0));
        system_mbstowcs(st_buf, p->name, 12);
        TextOut(hdc, (x + center) * prp->charwidth, 0, st_buf, namelen);

        x += vallen + 1;
    }

    --x;
    if (x * prp->charwidth != clientrect->right) {
        clientrect->right = x * prp->charwidth;
        changed_dimensions = TRUE;
    }

    if (2 * prp->charheight != clientrect->bottom) {
        clientrect->bottom = 2 * prp->charheight;
        changed_dimensions = TRUE;
    }

    if (changed_dimensions) {
        lib_free( prp->LastShownRegs );
        prp->LastShownRegs = NULL;
        prp->RegCount = 0;

        /* we will be redrawn in the not so far future! */
    }

    lib_free(pMonRegs);

    return changed_dimensions;
}

/*! \brief Determine cursor position at the time when the message was generated.
 
 This function is a replacement for the asynchronous WIN32 API GetCursorPos().
 GetCursorPos() determines the current cursor position. In contrast, this
 function here determines the cursor position at the time the last message was
 removed from the message queue with a call to GetMessage(). This way, we can
 ensure that the position has not changed since the current message was generated.

 \param point
    pointer to a POINT structure that will contain the cursor position

 \remark
    Calling this function from anywhere but the Window function (or the message
    loop itself) results in unpredicted behaviour.

 \remark
    Cf. http://blogs.msdn.com/oldnewthing/archive/2009/06/18/9771135.aspx 
    for a motivation of this function.
*/
static VOID GetCursorPosAtMessageTime(LPPOINT point)
{
#ifdef WATCOM_COMPILE
    GetCursorPos(point);
#else
    DWORD pos = GetMessagePos();

    POINTSTOPOINT(*point, pos);
#endif
}

typedef int ExecuteGenericPopup_callback_t(HMENU hPopupMenu, WORD ulDefault, WORD ulMask, int * nMenuCount, void * pContext);

/* make shotcuts for defining menu entries */
#define IMAKE_ENTRY(_FLAG_, _ID_, _TEXT_, _ENABLE_)                   \
    mii.fState = _ENABLE_ | ((ulDefault & _FLAG_) ? MFS_DEFAULT : 0); \
    mii.wID = _ID_;                                                   \
    mii.dwTypeData = _TEXT_;                                          \
    mii.cch = _tcslen(mii.dwTypeData);                                \
    InsertMenuItem( hPopupMenu, nMenuCount++, 1, &mii );

#define MAKE_ENTRY(_ID_, _TEXT_) \
    IMAKE_ENTRY(0, _ID_, _TEXT_, MFS_ENABLED)

#define MAKE_COND_ENTRY(_FLAG_, _ID_, _TEXT_)           \
    if (ulMask & _FLAG_) {                              \
        if (ulDefault & _FLAG_) {                       \
            uDefaultCommand = _ID_;                     \
        }                                               \
        IMAKE_ENTRY(_FLAG_, _ID_, _TEXT_, MFS_ENABLED) \
    }

#define MAKE_ENDISABLE_ENTRY(_FLAGS_, _ID_, _TEXT_) \
    IMAKE_ENTRY(0, _ID_, _TEXT_, ((ulMask & _FLAGS_) ? MFS_ENABLED : MFS_DISABLED))

#define MAKE_SEPARATOR()                               \
    mii.fType = MFT_SEPARATOR;                         \
    mii.fState = MFS_ENABLED;                          \
    mii.wID = 0;                                       \
    InsertMenuItem(hPopupMenu, nMenuCount++, 1, &mii); \
    mii.fType = MFT_STRING;

static int ExecuteGenericPopup(HWND hwnd, LPARAM lParam, BOOL bExecuteDefault, ExecuteGenericPopup_callback_t * callback, void * context)
{
    WORD ulDefault = 0;
    WORD ulMask = 0xffff;

    /* switch off drives that are not currently available */
    ulMask = SwitchOffUnavailableDrives(ulMask);

    /* now, create the appropriate pop up menu */
    {
        MENUITEMINFO mii;
        HMENU hPopupMenu;
        POINT curpos;
        int nMenuCount = 0;
        UINT uDefaultCommand = 0;

        hPopupMenu = CreatePopupMenu();

        /* global initializations */
        mii.cbSize = sizeof(mii);
        mii.fMask = MIIM_STATE | MIIM_ID | MIIM_TYPE;
        mii.fType = MFT_STRING;

        if (callback) {
            int nNewMenuCount = 0;
            int NewDefaultCommand;

            NewDefaultCommand = callback(hPopupMenu, ulDefault, ulMask, &nNewMenuCount, context);
            if (NewDefaultCommand >= 0) {
                uDefaultCommand = NewDefaultCommand;
            }
            nMenuCount += nNewMenuCount;
        }

        if (ulMask & (MDDPC_SET_COMPUTER | MDDPC_SET_DRIVE8 | MDDPC_SET_DRIVE9 | MDDPC_SET_DRIVE10 | MDDPC_SET_DRIVE11)) {
            MAKE_ENDISABLE_ENTRY(MDDPC_SET_COMPUTER, IDM_MON_COMPUTER, TEXT("&Computer"));
            MAKE_ENDISABLE_ENTRY(MDDPC_SET_DRIVE8, IDM_MON_DRIVE8, TEXT("Drive &8"));
            MAKE_ENDISABLE_ENTRY(MDDPC_SET_DRIVE9, IDM_MON_DRIVE9, TEXT("Drive &9"));
            MAKE_ENDISABLE_ENTRY(MDDPC_SET_DRIVE10, IDM_MON_DRIVE10, TEXT("Drive 1&0"));
            MAKE_ENDISABLE_ENTRY(MDDPC_SET_DRIVE11, IDM_MON_DRIVE11, TEXT("Drive 1&1"));
        }

        GetCursorPosAtMessageTime(&curpos);

        if (bExecuteDefault) {
            SendMessage(hwnd, WM_COMMAND, uDefaultCommand, 0);
        } else {
            TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, curpos.x, curpos.y, 0, hwnd, 0);
        }
        DestroyMenu(hPopupMenu);
    }

    return 0;
}

static int ExecuteRegistryPopup(HWND hwnd, reg_private_t *prp, LPARAM lParam, BOOL bExecuteDefault)
{
    return ExecuteGenericPopup(hwnd, lParam, bExecuteDefault, NULL, NULL);
}

/* window procedure */
static LRESULT CALLBACK reg_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, window_data_t * window_data)
{
    reg_private_t * prp = window_data->private_data;

    switch (msg) {
        case WM_UPDATEVAL:
            update_last_shown_regs(prp);
            return 0;
        case WM_LBUTTONDOWN:
            return ExecuteRegistryPopup(hwnd, prp, lParam, TRUE);
        case WM_RBUTTONDOWN:
            return ExecuteRegistryPopup(hwnd, prp, lParam, FALSE);
        case WM_NCCREATE:
            prp->LastShownRegs = NULL;
            prp->RegCount = 0;
            break;
        case WM_CREATE:
            {
                HDC hdc = GetDC(hwnd);
                SIZE size;

                SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));

                // get height and width of a character
                GetTextExtentPoint32(hdc, TEXT(" "), 1, &size);
                prp->charwidth  = size.cx;
                prp->charheight = size.cy;

                prp->memspace = e_comp_space;

                break;
            }
        case WM_SAVE_PARAMETERS:
            {
                BYTE **p = (BYTE **)wParam;

                BYTE buffer[1];
                buffer[0] = prp->memspace;
                WriteExtraData(p, buffer, sizeof(buffer));
            }
            return 0;
        case WM_RESTORE_PARAMETERS:
            if (wParam) {
                BYTE *p = (BYTE *)wParam;
                if (*p > 0) {
                    prp->memspace = p[0];
                }
                SetMemspace(hwnd, window_data, prp->memspace);
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        case WM_CHANGECOMPUTERDRIVE:
            /* FALL THROUGH */
        case WM_COMMAND:
            {
                switch (LOWORD(wParam)) {
                    case IDM_MON_COMPUTER:
                        prp->memspace = e_comp_space;
                        break;
                    case IDM_MON_DRIVE8:
                        prp->memspace = e_disk8_space;
                        break;
                    case IDM_MON_DRIVE9:
                        prp->memspace = e_disk9_space;
                        break;
                    case IDM_MON_DRIVE10:
                        prp->memspace = e_disk10_space;
                        break;
                    case IDM_MON_DRIVE11:
                        prp->memspace = e_disk11_space;
                        break;
                }
                SetMemspace(hwnd, window_data, prp->memspace);
                InvalidateRect(hwnd,NULL,FALSE);
            }
            break;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc;
                RECT rect;
                BOOLEAN changed_dimension;

                GetClientRect(hwnd, &rect);

                hdc = BeginPaint(hwnd,&ps);
                changed_dimension = output_register(hdc, prp, &rect);
                EndPaint(hwnd, &ps);

                if (changed_dimension) {
                    // resize the window

                    ClientToScreen(hwnd, (LPPOINT)&rect);
                    ClientToScreen(hwnd, ((LPPOINT)&rect) + 1);

                    if (!window_data->extra->as_popup) {
                        ScreenToClient(hwndMdiClient, (LPPOINT)&rect);
                        ScreenToClient(hwndMdiClient, ((LPPOINT)&rect) + 1);
                    }

                    AdjustWindowRectEx(&rect,
                                       (DWORD)GetWindowLongPtr(hwnd, GWL_STYLE), FALSE,
                                       (DWORD)GetWindowLongPtr(hwnd, GWL_EXSTYLE));

                    MoveWindow(hwnd, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);
                }
                return 0;
            }
    }

    return window_data->default_window_procedure(hwnd, msg, wParam, lParam);
}

struct ExecuteDisassemblyPopup_callback_context_s {
    WORD xPos;
    WORD yPos;
    dis_private_t * pdp;
};

static ExecuteGenericPopup_callback_t ExecuteDisassemblyPopup_callback;

static int ExecuteDisassemblyPopup_callback(HMENU hPopupMenu, WORD ulDefault, WORD ulMask, int *nNewMenuCount, void * pContext)
{
    int nMenuCount = 0;
    int uDefaultCommand = -1;

    struct ExecuteDisassemblyPopup_callback_context_s * context = pContext;

    MENUITEMINFO mii;

    assert(context != NULL);

    /* determine which commands should be visible, and which one is the default */
    mon_disassembly_determine_popup_commands( &context->pdp->mdp, context->xPos, context->yPos, &ulMask, &ulDefault );

    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE | MIIM_ID | MIIM_TYPE;
    mii.fType = MFT_STRING;

    MAKE_ENTRY(IDM_MON_GOTO_PC, TEXT("go &to PC"));
    MAKE_ENTRY(IDM_MON_GOTO_ADDRESS, TEXT("&go to address"));

    MAKE_SEPARATOR();

    MAKE_ENTRY(IDM_MON_SET_NEXT_INSTRUCTION, TEXT("set &next instruction"));

    MAKE_SEPARATOR();

    MAKE_COND_ENTRY(MDDPC_SET_BREAKPOINT, IDM_MON_SET_BP, TEXT("&set breakpoint"));
    MAKE_COND_ENTRY(MDDPC_UNSET_BREAKPOINT, IDM_MON_UNSET_BP, TEXT("&unset breakpoint"));
    MAKE_COND_ENTRY(MDDPC_ENABLE_BREAKPOINT, IDM_MON_ENABLE_BP, TEXT("&enable breakpoint"));
    MAKE_COND_ENTRY(MDDPC_DISABLE_BREAKPOINT, IDM_MON_DISABLE_BP, TEXT("&disable breakpoint"));

    *nNewMenuCount = nMenuCount;

    return uDefaultCommand;
}

static int ExecuteDisassemblyPopup(HWND hwnd, dis_private_t *pdp, LPARAM lParam, BOOL bExecuteDefault)
{
    struct ExecuteDisassemblyPopup_callback_context_s context;

    context.xPos = LOWORD(lParam) / pdp->charwidth;
    context.yPos = HIWORD(lParam) / pdp->charheight;
        context.pdp = pdp;

    return ExecuteGenericPopup(hwnd, lParam, bExecuteDefault, ExecuteDisassemblyPopup_callback, &context);
}

static ExecuteGenericPopup_callback_t ExecuteMemoryPopup_callback;

static int ExecuteMemoryPopup_callback(HMENU hPopupMenu, WORD ulDefault, WORD ulMask, int *nNewMenuCount, void * pContext)
{
    int nMenuCount = 0;
    int uDefaultCommand = -1;

    MENUITEMINFO mii;
    mii.cbSize = sizeof(mii);
    mii.fMask = MIIM_STATE | MIIM_ID | MIIM_TYPE;
    mii.fType = MFT_STRING;

    MAKE_ENTRY(IDM_MON_GOTO_PC, TEXT("go &to PC"));
    MAKE_ENTRY(IDM_MON_GOTO_ADDRESS, TEXT("&go to address"));

    MAKE_SEPARATOR();

    *nNewMenuCount = nMenuCount;

    return uDefaultCommand;
}

static int ExecuteMemPopup(HWND hwnd, mem_private_t *pdp, LPARAM lParam, BOOL bExecuteDefault)
{
    return ExecuteGenericPopup(hwnd, lParam, bExecuteDefault, ExecuteMemoryPopup_callback, NULL);
}

typedef enum LINETYPE_S { 
    LT_NORMAL,     
    LT_EXECUTE, 
    LT_EXECUTE_BREAKPOINT,
    LT_EXECUTE_BREAKPOINT_INACTIVE,
    LT_BREAKPOINT, 
    LT_BREAKPOINT_INACTIVE,
    LT_LAST
} LINETYPE; 

static const COLORREF crTextLineType[LT_LAST] = {
    RGB( 0x00, 0x00, 0x00 ), // LT_NORMAL
    RGB( 0xFF, 0xFF, 0xFF ), // LT_EXECUTE
    RGB( 0xFF, 0xFF, 0xFF ), // LT_EXECUTE_BREAKPOINT
    RGB( 0xFF, 0xFF, 0xFF ), // LT_EXECUTE_BREAKPOINT_INACTIVE
    RGB( 0x00, 0x00, 0x00 ), // LT_BREAKPOINT
    RGB( 0x00, 0x00, 0x00 )  // LT_BREAKPOINT_INACTIVE
};

static const COLORREF crBackLineType[LT_LAST] = {
    RGB( 0xFF, 0xFF, 0xFF ), // LT_NORMAL
    RGB( 0x00, 0x00, 0xFF ), // LT_EXECUTE
    RGB( 0x00, 0x80, 0x80 ), // LT_EXECUTE_BREAKPOINT
    RGB( 0x00, 0x00, 0xFF ), // LT_EXECUTE_BREAKPOINT_INACTIVE
    RGB( 0xFF, 0x00, 0x00 ), // LT_BREAKPOINT
    RGB( 0xFF, 0xFF, 0x00 )  // LT_BREAKPOINT_INACTIVE
};

static BOOL CALLBACK navigate_window_proc(HWND hwnd, UINT msg, WPARAM wParam,LPARAM lParam, LRESULT *lResult,
                                          window_data_t *window_data, mon_navigate_private_t *mnp)
{
    switch (msg)
    {
        case WM_CHANGECOMPUTERDRIVE:
            switch (wParam) {
                case IDM_MON_COMPUTER:
                    mon_navigate_set_memspace(mnp, e_comp_space);
                    break;
                case IDM_MON_DRIVE8:
                    mon_navigate_set_memspace(mnp, e_disk8_space);
                    break;
                case IDM_MON_DRIVE9:
                    mon_navigate_set_memspace(mnp, e_disk9_space);
                    break;
                case IDM_MON_DRIVE10:
                    mon_navigate_set_memspace(mnp, e_disk10_space);
                    break;
                case IDM_MON_DRIVE11:
                    mon_navigate_set_memspace(mnp, e_disk11_space);
                    break;
            }
            SetMemspace(hwnd, window_data, mon_navigate_get_memspace(mnp));
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_VSCROLL:
            {
                SCROLLINFO ScrollInfo;
                BOOLEAN changed = FALSE;

                ScrollInfo.cbSize = sizeof(ScrollInfo);
                ScrollInfo.fMask = SIF_POS|SIF_TRACKPOS;
                GetScrollInfo(hwnd, SB_VERT, &ScrollInfo);

                ScrollInfo.fMask = SIF_POS;

                switch (LOWORD(wParam)) {
                    case SB_THUMBPOSITION:
                        *lResult = window_data->default_window_procedure(hwnd, msg, wParam, lParam);
                        return TRUE;
                    case SB_THUMBTRACK:
                        ScrollInfo.nPos = mon_navigate_scroll_to(mnp, (WORD)ScrollInfo.nTrackPos);
                        changed = TRUE;
                        break;
                    case SB_LINEUP:
                        ScrollInfo.nPos = mon_navigate_scroll(mnp, MON_SCROLL_UP);
                        changed = TRUE;
                        break;
                    case SB_PAGEUP:
                        ScrollInfo.nPos = mon_navigate_scroll(mnp, MON_SCROLL_PAGE_UP);
                        changed = TRUE;
                        break;
                    case SB_LINEDOWN:
                        ScrollInfo.nPos = mon_navigate_scroll(mnp, MON_SCROLL_DOWN);
                        changed = TRUE;
                        break;
                    case SB_PAGEDOWN:
                        ScrollInfo.nPos = mon_navigate_scroll(mnp, MON_SCROLL_PAGE_DOWN);
                        changed = TRUE;
                        break;
                }

                if (changed) {
                    SetScrollInfo(hwnd, SB_VERT, &ScrollInfo, TRUE);
                    InvalidateRect(hwnd, NULL, FALSE);
                    UpdateWindow(hwnd);
                }
            }
            break;
        case WM_KEYDOWN:
            switch ((int)wParam) { /* nVirtKey */
                case VK_UP:
                    SendMessage(hwnd, WM_VSCROLL, SB_LINEUP, 0);
                    *lResult = 0;
                    return TRUE;
                case VK_DOWN:
                    SendMessage(hwnd, WM_VSCROLL, SB_LINEDOWN, 0);
                    *lResult = 0;
                    return TRUE;
                case VK_PRIOR:
                    SendMessage(hwnd, WM_VSCROLL, SB_PAGEUP, 0);
                    *lResult = 0;
                    return TRUE;
                case VK_NEXT:
                    SendMessage(hwnd, WM_VSCROLL, SB_PAGEDOWN, 0);
                    *lResult = 0;
                    return TRUE;
            }
            break;
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_MON_GOTO_PC:
                    mon_navigate_goto_pc(mnp);
                    break;
                case IDM_MON_GOTO_ADDRESS:
                    {
                        char *result;
#if 0
                        // @@@@@SRT not yet implemented
                        // result = uimon_inputaddress( "Please enter the address you want to go to:" );
#else
                        // for testing purposes 
                        result = lib_stralloc("a474");
#endif
                        if (result) {
                            mon_navigate_goto_string(mnp, result);
                            lib_free(result);
                        }
                    }
                    break;
                case IDM_MON_COMPUTER:
                case IDM_MON_DRIVE8:
                case IDM_MON_DRIVE9:
                case IDM_MON_DRIVE10:
                case IDM_MON_DRIVE11:
                    SendMessage(hwnd, WM_CHANGECOMPUTERDRIVE, LOWORD(wParam), 0);
                    mon_navigate_goto_pc(mnp);
                    break;
            }

            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
            break;
    }

    return FALSE;
}

#define ST_BUF_LEN 128

/* window procedure */
static LRESULT CALLBACK dis_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, window_data_t *window_data)
{
    LRESULT lResult;
    dis_private_t *pdp = window_data->private_data;

    if (navigate_window_proc(hwnd, msg, wParam, lParam, &lResult, window_data, &pdp->mdp.navigate)) {
        return lResult;
    }

    switch (msg) {
        case WM_SAVE_PARAMETERS:
            {
                BYTE **p = (BYTE **)wParam;
                BYTE buffer[1];

                buffer[0] = mon_navigate_get_memspace(&pdp->mdp.navigate);

                WriteExtraData(p, buffer, sizeof buffer);
            }
            return 0;
        case WM_RESTORE_PARAMETERS:
            if (wParam) {
                BYTE *p = (BYTE *)wParam;
                if (*p > 0) {
                    mon_navigate_set_memspace(&pdp->mdp.navigate, p[0]);
                }
                SetMemspace(hwnd, window_data, mon_navigate_get_memspace(&pdp->mdp.navigate));
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        case WM_CREATE:
            {
                HDC hdc = GetDC(hwnd);
                SIZE size;

                SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));

                // get height and width of a character
                GetTextExtentPoint32(hdc, TEXT(" "), 1, &size);
                pdp->charwidth = size.cx;
                pdp->charheight = size.cy;

                {
                    SCROLLINFO ScrollInfo;
                    ScrollInfo.cbSize = sizeof(ScrollInfo);
                    ScrollInfo.fMask  = SIF_RANGE;

                    ScrollInfo.nMin = 0;
                    ScrollInfo.nMax = 0x10000;

                    SetScrollInfo(hwnd, SB_VERT, &ScrollInfo, FALSE);
                }

                // initialize some window parameter
                mon_disassembly_init(&pdp->mdp);

                {
                    SCROLLINFO ScrollInfo;

                    ScrollInfo.cbSize = sizeof(ScrollInfo);
                    ScrollInfo.fMask = SIF_POS;
                    GetScrollInfo(hwnd, SB_VERT, &ScrollInfo);

                    ScrollInfo.nPos = mon_navigate_scroll(&pdp->mdp.navigate, MON_SCROLL_NOTHING);

                    SetScrollInfo(hwnd, SB_VERT, &ScrollInfo, TRUE);
                    InvalidateRect(hwnd, NULL, FALSE);
                    UpdateWindow(hwnd);
                }
                break;
            }
        case WM_UPDATE:
            mon_disassembly_update(&pdp->mdp);
            return 0;
        case WM_LBUTTONDOWN:
            return ExecuteDisassemblyPopup(hwnd, pdp, lParam, TRUE);
        case WM_RBUTTONDOWN:
            return ExecuteDisassemblyPopup(hwnd, pdp, lParam, FALSE);
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_MON_SET_NEXT_INSTRUCTION:
                    mon_disassembly_set_next_instruction(&pdp->mdp);
                    uimon_notify_change();
                    break;
                case IDM_MON_SET_BP:
                    mon_disassembly_set_breakpoint(&pdp->mdp);
                    break;
                case IDM_MON_UNSET_BP:
                    mon_disassembly_unset_breakpoint(&pdp->mdp);
                    break;
                case IDM_MON_ENABLE_BP:
                    mon_disassembly_enable_breakpoint(&pdp->mdp);
                    break;
                case IDM_MON_DISABLE_BP:
                    mon_disassembly_disable_breakpoint(&pdp->mdp);
                    break;
            }
            InvalidateRect(hwnd, NULL, FALSE);
            UpdateWindow(hwnd);
            break;
        case WM_PAINT:
            {
                mon_disassembly_t *md_contents = NULL;
                PAINTSTRUCT ps;
                HDC hdc;
                RECT rect;
                int nHeightToPrint;
                COLORREF crOldTextColor;
                COLORREF crOldBkColor;
                HPEN hpenOld;
                HBRUSH hbrushOld;

                int i;

                HBRUSH hbrushBack[LT_LAST];
                HPEN hpenBack[LT_LAST];

                GetClientRect(hwnd, &rect);
                nHeightToPrint = (rect.bottom - rect.top) / pdp->charheight + 1;

                hdc = BeginPaint(hwnd, &ps);

                for (i = 0; i < LT_LAST; i++) {
                    hbrushBack[i] = CreateSolidBrush(crBackLineType[i]);
                    hpenBack[i] = CreatePen(PS_SOLID, 1, crBackLineType[i]);
                }

                crOldTextColor = SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF) );
                crOldBkColor = SetBkColor(hdc, RGB(0, 0, 0));
                hpenOld = SelectObject(hdc, GetStockObject(BLACK_PEN));
                hbrushOld = SelectObject(hdc, GetStockObject(BLACK_BRUSH));

                md_contents = mon_disassembly_get_lines(&pdp->mdp, nHeightToPrint, nHeightToPrint - 1);

                if (md_contents) {
                    for (i = 0; i < nHeightToPrint; i++) {
                        TCHAR st_buf[ST_BUF_LEN];
                        mon_disassembly_t *next = md_contents->next;

                        LINETYPE lt;

                        COLORREF crText;
                        COLORREF crBack;

                        if (md_contents->flags.active_line) {
                            if (md_contents->flags.is_breakpoint) {
                                if (md_contents->flags.breakpoint_active) {
                                    lt = LT_EXECUTE_BREAKPOINT;
                                } else {
                                    lt = LT_EXECUTE_BREAKPOINT_INACTIVE;
                                }
                            } else {
                                lt = LT_EXECUTE;
                            }
                        } else {
                            if (md_contents->flags.is_breakpoint) {
                                if (md_contents->flags.breakpoint_active) {
                                    lt = LT_BREAKPOINT;
                                } else {
                                    lt = LT_BREAKPOINT_INACTIVE;
                                }
                            } else {
                                lt = LT_NORMAL;
                            }
                        }

                        crText = crTextLineType[lt];
                        crBack = crBackLineType[lt];

                        SetTextColor(hdc, crText);
                        SetBkColor(hdc, crBack);

                        system_mbstowcs(st_buf, md_contents->content, ST_BUF_LEN);
                        TextOut(hdc, 0, i * pdp->charheight, st_buf, md_contents->length);

                        /* make sure we clear all that is right from the text */
                        SelectObject(hdc, hbrushBack[lt]);
                        SelectObject(hdc, hpenBack[lt]);
                        Rectangle(hdc, md_contents->length * pdp->charwidth, i * pdp->charheight, rect.right + 1, (i + 1) * pdp->charheight);

                        lib_free(md_contents->content);
                        lib_free(md_contents);
                        md_contents = next;
                    }
                }
                /* restore old settings */
                SelectObject(hdc, hpenOld);
                SelectObject(hdc, hbrushOld);
                SetTextColor(hdc, crOldTextColor);
                SetBkColor(hdc, crOldBkColor);

                for (i = 0; i < LT_LAST; i++) {
                    DeleteObject(hbrushBack[i]);
                    DeleteObject(hpenBack[i]);
                }

                EndPaint(hwnd,&ps);
            }
    }

    return window_data->default_window_procedure(hwnd, msg, wParam, lParam);
}

/* window procedure */
static LRESULT CALLBACK mem_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam, window_data_t *window_data)
{
    LRESULT lResult;
    mem_private_t *pmp = window_data->private_data;

    if (navigate_window_proc(hwnd, msg, wParam, lParam, &lResult, window_data, &pmp->mmp.navigate)) {
        return lResult;
    }

    switch (msg) {
        case WM_SAVE_PARAMETERS:
            {
                BYTE **p = (BYTE **)wParam;

                BYTE buffer[5];
                WORD startaddress = mon_navigate_get_startaddress(&pmp->mmp.navigate);

                buffer[0] = mon_navigate_get_memspace(&pmp->mmp.navigate);
                buffer[1] = startaddress & 0xFFu;
                buffer[2] = (startaddress >> 8) & 0xFFu;
                buffer[3] = 0;
                buffer[4] = 0;

                WriteExtraData(p, buffer, sizeof buffer);
            }
            return 0;
        case WM_RESTORE_PARAMETERS:
            if (wParam) {
                BYTE *p = (BYTE *) wParam;

                if (*p > 0) {
                    WORD startaddress = (p[4] << 24) | (p[3] << 16) | (p[2] << 8) | p[1];

                    mon_navigate_set_memspace(&pmp->mmp.navigate, p[0]);
                    mon_navigate_set_startaddress(&pmp->mmp.navigate, startaddress);
                }
                SetMemspace(hwnd, window_data, mon_navigate_get_memspace(&pmp->mmp.navigate));
                InvalidateRect(hwnd, NULL, FALSE);
            }
            return 0;
        case WM_CREATE:
            {
                HDC hdc = GetDC(hwnd);
                SIZE size;

                SelectObject(hdc, GetStockObject(ANSI_FIXED_FONT));

                mon_memory_init(&pmp->mmp);

                // get height and width of a character
                GetTextExtentPoint32(hdc, TEXT(" "), 1, &size);
                pmp->charwidth = size.cx;
                pmp->charheight = size.cy;

                {
                    SCROLLINFO ScrollInfo;
                    ScrollInfo.cbSize = sizeof(ScrollInfo);
                    ScrollInfo.fMask = SIF_RANGE;

                    ScrollInfo.nMin = 0;
                    ScrollInfo.nMax = 0x10000;

                    SetScrollInfo(hwnd, SB_VERT, &ScrollInfo, FALSE);
                }
                break;
            }
        case WM_LBUTTONDOWN:
            return ExecuteMemPopup(hwnd, pmp, lParam, TRUE);
        case WM_RBUTTONDOWN:
            return ExecuteMemPopup(hwnd, pmp, lParam, FALSE);
        case WM_PAINT:
            {
#if 0
                TCHAR st_buf[ST_BUF_LEN];
                mon_memory_t * contents = NULL;
                PAINTSTRUCT ps;
                HDC hdc;
                RECT rect;
                int nHeightToPrint;
                int i;

                hdc = BeginPaint(hwnd, &ps);

                GetClientRect(hwnd, &rect);
                nHeightToPrint = (rect.bottom - rect.top) / pmp->charheight + 1;

                contents = mon_memory_get_lines(&pmp->mmp, nHeightToPrint, nHeightToPrint - 1);

                if (contents) {
                    for (i = 0; i < nHeightToPrint; i++) {
                        mon_memory_t *next = contents->next;

                        LINETYPE lt;

                        COLORREF crText;
                        COLORREF crBack;

                        if (contents->flags.active_line) {
                            if (contents->flags.is_breakpoint) {
                                if (contents->flags.breakpoint_active) {
                                    lt = LT_EXECUTE_BREAKPOINT;
                                } else {
                                    lt = LT_EXECUTE_BREAKPOINT_INACTIVE;
                                }
                            } else {
                                lt = LT_EXECUTE;
                            }
                        } else {
                            if (contents->flags.is_breakpoint) {
                                if (contents->flags.breakpoint_active) {
                                    lt = LT_BREAKPOINT;
                                } else {
                                    lt = LT_BREAKPOINT_INACTIVE;
                                }
                            } else {
                                lt = LT_NORMAL;
                            }
                        }

                        crText = crTextLineType[lt];
                        crBack = crBackLineType[lt];

                        SetTextColor(hdc, crText);
                        SetBkColor(hdc, crBack);

                        system_mbstowcs(st_buf, contents->content, ST_BUF_LEN);
                        TextOut(hdc, 0, i * pmp->charheight, st_buf, contents->length);

                        /* make sure we clear all that is right from the text */
                        Rectangle(hdc, contents->length * pmp->charwidth, i * pmp->charheight, rect.right + 1, (i + 1) * pmp->charheight);

                        lib_free(contents->content);
                        lib_free(contents);
                        contents = next;
                    }
                }
                EndPaint(hwnd, &ps);
#else
                TCHAR st_buf[ST_BUF_LEN];
                mon_disassembly_t *md_contents = NULL;
                PAINTSTRUCT ps;
                HDC hdc;
                RECT rect;
                int nHeightToPrint;
                COLORREF crOldTextColor;
                COLORREF crOldBkColor;
                HPEN hpenOld;
                HBRUSH hbrushOld;

                int i;

                typedef enum LINETYPE_S { 
                    LT_NORMAL,     
                    LT_EXECUTE, 
                    LT_EXECUTE_BREAKPOINT,
                    LT_EXECUTE_BREAKPOINT_INACTIVE,
                    LT_BREAKPOINT, 
                    LT_BREAKPOINT_INACTIVE,
                    LT_LAST
                } LINETYPE;

                const COLORREF crTextLineType[LT_LAST] = {
                    RGB( 0x00, 0x00, 0x00 ), // LT_NORMAL
                    RGB( 0xFF, 0xFF, 0xFF ), // LT_EXECUTE
                    RGB( 0xFF, 0xFF, 0xFF ), // LT_EXECUTE_BREAKPOINT
                    RGB( 0xFF, 0xFF, 0xFF ), // LT_EXECUTE_BREAKPOINT_INACTIVE
                    RGB( 0x00, 0x00, 0x00 ), // LT_BREAKPOINT
                    RGB( 0x00, 0x00, 0x00 )  // LT_BREAKPOINT_INACTIVE
                };

                const COLORREF crBackLineType[LT_LAST] = {
                    RGB( 0xFF, 0xFF, 0xFF ), // LT_NORMAL
                    RGB( 0x00, 0x00, 0xFF ), // LT_EXECUTE
                    RGB( 0x00, 0x80, 0x80 ), // LT_EXECUTE_BREAKPOINT
                    RGB( 0x00, 0x00, 0xFF ), // LT_EXECUTE_BREAKPOINT_INACTIVE
                    RGB( 0xFF, 0x00, 0x00 ), // LT_BREAKPOINT
                    RGB( 0xFF, 0xFF, 0x00 )  // LT_BREAKPOINT_INACTIVE
                };

                HBRUSH hbrushBack[LT_LAST];
                HPEN hpenBack[LT_LAST];

                GetClientRect(hwnd, &rect);
                nHeightToPrint = (rect.bottom - rect.top) / pmp->charheight + 1;

                hdc = BeginPaint(hwnd, &ps);

                for (i = 0; i < LT_LAST; i++) {
                    hbrushBack[i] = CreateSolidBrush(crBackLineType[i]);
                    hpenBack[i] = CreatePen(PS_SOLID, 1, crBackLineType[i]);
                }

                crOldTextColor = SetTextColor(hdc, RGB(0xFF, 0xFF, 0xFF));
                crOldBkColor = SetBkColor(hdc, RGB(0, 0, 0));
                hpenOld = SelectObject(hdc, GetStockObject(BLACK_PEN));
                hbrushOld = SelectObject(hdc, GetStockObject(BLACK_BRUSH));

                md_contents = mon_dump_get_lines(&pmp->mmp, nHeightToPrint, nHeightToPrint - 1);

                for (i = 0; i < nHeightToPrint; i++) {
                    mon_disassembly_t *next = md_contents->next;

                    LINETYPE lt;

                    COLORREF crText;
                    COLORREF crBack;

                    if (md_contents->flags.active_line) {
                        if (md_contents->flags.is_breakpoint) {
                            if (md_contents->flags.breakpoint_active) {
                                lt = LT_EXECUTE_BREAKPOINT;
                            } else {
                                lt = LT_EXECUTE_BREAKPOINT_INACTIVE;
                            }
                        } else {
                            lt = LT_EXECUTE;
                        }
                    } else {
                        if (md_contents->flags.is_breakpoint) {
                            if (md_contents->flags.breakpoint_active) {
                                lt = LT_BREAKPOINT;
                            } else {
                                lt = LT_BREAKPOINT_INACTIVE;
                            }
                        } else {
                            lt = LT_NORMAL;
                        }
                    }

                    crText = crTextLineType[lt];
                    crBack = crBackLineType[lt];

                    SetTextColor(hdc, crText);
                    SetBkColor(hdc, crBack);

                    system_mbstowcs(st_buf, md_contents->content, ST_BUF_LEN);
                    TextOut(hdc, 0, i * pmp->charheight, st_buf, md_contents->length);

                    /* make sure we clear all that is right from the text */
                    SelectObject(hdc, hbrushBack[lt]);
                    SelectObject(hdc, hpenBack[lt]);
                    Rectangle(hdc, md_contents->length * pmp->charwidth, i * pmp->charheight, rect.right + 1, (i + 1) * pmp->charheight);

                    lib_free(md_contents->content);
                    lib_free(md_contents);
                    md_contents = next;
                }

                /* restore old settings */
                SelectObject(hdc, hpenOld);
                SelectObject(hdc, hbrushOld);
                SetTextColor(hdc, crOldTextColor);
                SetBkColor(hdc, crOldBkColor);

                for (i = 0; i < LT_LAST; i++) {
                    DeleteObject(hbrushBack[i]);
                    DeleteObject(hpenBack[i]);
                }

                EndPaint(hwnd, &ps);

#endif
            }
    }

    return window_data->default_window_procedure(hwnd, msg, wParam, lParam);
}

static LRESULT CALLBACK generic_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    window_data_t window_data;

    window_data.window_procedure = (void *)GetWindowLongPtr(hwnd, offsetof(window_data_t, window_procedure));

    if (window_data.window_procedure == NULL) {
        window_data_t * new_window_data = uimon_iep((void **)&window_data_create, NULL);

        assert(new_window_data != NULL);

        SetWindowLongPtr(hwnd, offsetof(window_data_t, window_procedure), (LONG_PTR)new_window_data->window_procedure);
        SetWindowLongPtr(hwnd, offsetof(window_data_t, default_window_procedure), (LONG_PTR)new_window_data->default_window_procedure);
        SetWindowLongPtr(hwnd, offsetof(window_data_t, private_data), (LONG_PTR)new_window_data->private_data);
        SetWindowLongPtr(hwnd, offsetof(window_data_t, extra), (LONG_PTR)new_window_data->extra);

        window_data.window_procedure = new_window_data->window_procedure;

        new_window_data->extra->memspace = e_comp_space;

        lib_free(new_window_data);
    }

    window_data.default_window_procedure = (void *)GetWindowLongPtr(hwnd, offsetof(window_data_t, default_window_procedure));
    window_data.private_data = (void *)GetWindowLongPtr(hwnd, offsetof(window_data_t, private_data));
    window_data.extra = (void *)GetWindowLongPtr(hwnd, offsetof(window_data_t, extra));

    switch (msg) {
        LONG * lp;

        case WM_NCDESTROY:
            delete_client_window(hwnd);

            if (window_data.private_data) {
                // clear the private_data info 
                SetWindowLongPtr(hwnd, offsetof(window_data_t, private_data), 0);

                // clear the extra info
                SetWindowLongPtr(hwnd, offsetof(window_data_t, extra), 0);

                lib_free(window_data.extra);
                lib_free(window_data.private_data);
            }

            break;
        case WM_GETWINDOWTYPE:
            lp = (PLONG)lParam;
            *lp = window_data.extra->window_type;
            return 0;
        case WM_ACTIVATE:
            if (window_data.extra->as_popup) {
                if (LOWORD(wParam) != WA_INACTIVE) {
                    ActivateChild(TRUE, hwnd, &window_data);
                }
            }
            break;
        case WM_MDIACTIVATE:
            if (!window_data.extra->as_popup) {
                ActivateChild(((HWND)wParam == hwnd) ? FALSE : TRUE, hwnd, &window_data);
            }
            break;
    }

    return window_data.window_procedure(hwnd, msg, wParam, lParam, &window_data);
}

static void uimon_init(void)
{
    static BOOLEAN bFirstTime = TRUE;

    if (bFirstTime) {
        WNDCLASSEX wc;

        bFirstTime = FALSE;

        /* Register window class for the monitor window */
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_CLASSDC;
        wc.lpfnWndProc = mon_window_proc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = winmain_instance;
        wc.hIcon = LoadIcon(winmain_instance, MAKEINTRESOURCE(IDI_ICON1));
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0xc0, 0xc0, 0xc0));
        wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENUMONITOR);
        wc.lpszClassName = MONITOR_CLASS;
        wc.hIconSm = NULL;

        RegisterClassEx(&wc);

        /* Register window class for the disassembler window */
        wc.lpfnWndProc = generic_window_proc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = sizeof(window_data_t);
        wc.hIcon = NULL;
        wc.hCursor = NULL;
        wc.hbrBackground = CreateSolidBrush(RGB(0xFF, 0xFF, 0xFF));
        wc.lpszMenuName = 0;
        wc.lpszClassName = CONTENTS_CLASS;
        wc.hIconSm = NULL;

        RegisterClassEx(&wc);
    }
}

static BOOL CALLBACK WindowUpdateProc(HWND hwnd, LPARAM lParam)
{
    SendMessage(hwnd, WM_UPDATE, 0, 0);
    InvalidateRect(hwnd, NULL, FALSE);
    UpdateWindow(hwnd);
    return TRUE;
}
 
static void UpdateAll(void)
{
    uimon_client_windows_t *p;

    if (hwndMdiClient) {
        EnumChildWindows(hwndMdiClient,(WNDENUMPROC)WindowUpdateProc,(LPARAM)NULL);
    }

    for (p = first_client_window; p; p = p->next) {
        WindowUpdateProc(p->hwnd, 0);
    }

    InvalidateRect(hwndMonitor, NULL, FALSE);
    UpdateWindow(hwndMonitor);
}

static BOOL CALLBACK WindowUpdateShown(HWND hwnd, LPARAM lParam)
{
    SendMessage(hwnd, WM_UPDATEVAL, 0, 0);
    return TRUE;
}
 
static void update_shown(void)
{
    uimon_client_windows_t *p;

    if (hwndMdiClient) {
        EnumChildWindows(hwndMdiClient, (WNDENUMPROC)WindowUpdateShown, (LPARAM)NULL);
    }

    for (p = first_client_window; p; p = p->next) {
        WindowUpdateShown(p->hwnd, 0);
    }
}

#endif // #ifdef UIMON_EXPERIMENTAL

void uimon_notify_change()
{
#ifdef UIMON_EXPERIMENTAL
    UpdateAll();
#endif // #ifdef UIMON_EXPERIMENTAL
}

void uimon_window_close(void)
{
    console_log_for_mon.console_cannot_output = 1;

#ifdef UIMON_EXPERIMENTAL

    update_shown();

    StoreMonitorDimensions(hwndMonitor);
    DestroyWindow(hwndMonitor);
    hwndMonitor = hwndMdiClient = NULL;

    ResumeFullscreenMode(hwndParent);

#else // #ifdef UIMON_EXPERIMENTAL
    console_close(console_log_local);

#endif // #ifdef UIMON_EXPERIMENTAL

    console_log_local = NULL;
}

console_t *uimon_window_open(void)
{
#ifdef UIMON_EXPERIMENTAL

    WindowDimensions *wd;
    int x = CW_USEDEFAULT;
    int y = CW_USEDEFAULT;
    int dx = 472;
    int dy = (dx * 3) / 4;

    hwndParent = GetActiveWindow();

    SuspendFullscreenMode(hwndParent);

    uimon_init();

    hwndMonitor = CreateWindow(MONITOR_CLASS,
                               TEXT("VICE monitor"),
                               WS_OVERLAPPED | WS_CLIPCHILDREN | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX,
                               x, y,
                               dx, dy,
                               NULL, // @SRT GetActiveWindow(),
                               NULL,
                               winmain_instance,
                               NULL);

    wd = LoadMonitorDimensions(hwndMonitor);

    if (!wd) {
        OpenConsole(hwndMonitor, TRUE);
    } else {
        OpenFromWindowDimensions(hwndMonitor, wd);
    }

    if (console_log_local) {
        memcpy(&console_log_for_mon, console_log_local, sizeof(struct console_s));
    } else {
        /*
         @SRT: Temporary work-around...
        */
        console_log_for_mon.console_xres = 80;
        console_log_for_mon.console_yres = 5;
        console_log_for_mon.console_can_stay_open = 1;
    }

    EnableCommands(GetMenu(hwndMonitor), hwndToolbar);

    SetActiveWindow(hwndMonitor);

    ShowWindow(hwndMonitor, SW_SHOW);

    return &console_log_for_mon;

#else // #ifdef UIMON_EXPERIMENTAL

    console_log_local = console_open("Monitor");
    return console_log_local;

#endif // #ifdef UIMON_EXPERIMENTAL
}

void uimon_window_suspend(void)
{
    console_log_for_mon.console_cannot_output = 1;

#ifdef UIMON_EXPERIMENTAL

    update_shown();
    
#else // #ifdef UIMON_EXPERIMENTAL

    uimon_window_close();

#endif // #ifdef UIMON_EXPERIMENTAL
}

console_t *uimon_window_resume(void)
{
    console_log_for_mon.console_cannot_output = 0;

#ifdef UIMON_EXPERIMENTAL

    pchCommandLine = NULL;

    return &console_log_for_mon;

#else // #ifdef UIMON_EXPERIMENTAL

    return uimon_window_open();

#endif // #ifdef UIMON_EXPERIMENTAL
}

int uimon_out(const char *buffer)
{
    int   rc = 0;

    if (console_log_local) {
        rc = console_out(console_log_local, "%s", buffer);
    }
    return rc;
}

char *uimon_get_in(char **ppchCommandLine, const char *prompt) {
#ifdef UIMON_EXPERIMENTAL
    char *p = NULL;

    if (console_log_local) {
        /* we have a console, so try to input data from there... */
        p = console_in(console_log_local, prompt);
    } else {
        /* we don't have a console, make sure we can do something useful
           by dispatching the events
        */
        while (!*ppchCommandLine && !console_log_local) {
            ui_dispatch_next_event();
        }
    }
    return p;
#else // #ifdef UIMON_EXPERIMENTAL
    return console_in(console_log_local, prompt);
#endif // #ifdef UIMON_EXPERIMENTAL
}
