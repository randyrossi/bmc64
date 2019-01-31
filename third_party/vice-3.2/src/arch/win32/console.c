/*
 * console.c - Console access interface.
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "console.h"
#include "fullscrn.h"
#include "lib.h"
#include "res.h"
#include "system.h"
#include "ui.h"
#include "uiapi.h"
#include "uimon.h"
#include "uimonmsg.h"
#include "winlong.h"
#include "winmain.h"
#include "archdep.h"


/*
 MAX_WIDTH is not allowed to be bigger than (MIN_XSIZE * MIN_YSIZE) !
*/

#define MAX_WIDTH   256 /* max. char count for an input line  */
#define MAX_HISTORY 20  /* maximum history entrys per console */

#define MIN_XSIZE 40
#define MIN_YSIZE 20

/*
 When resizing the window, the new window buffer is not reallocated for every
 one increment, but always in size of RESIZE_INCREMENT_...
 So, we don't need to reallocate very frequently and so we don't convert the 
 memory into little parts which cannot be rejoined.
*/
#define RESIZE_INCREMENT_X 40
#define RESIZE_INCREMENT_Y 20

/*
 The following structure contains the file-private informations of a console window
*/
typedef struct console_private_s {
    struct console_private_s *pcpNext; /* pointer to the next console_private_t structure */
    console_t *pConsole;
    TCHAR *pchWindowBuffer;            /* a buffer to the window contents */
    char *pchName;                     /*  the name of the window; this is used to recognize the window when re-opening */
    char *pchOnClose;                  /* the string to be returned when the window is closed */
    char *history[MAX_HISTORY];        /* ring buffer of pointers to the history entries */
    unsigned nHistory;                 /* index to next entry to be written in history buffer */
    unsigned nCurrentSelectHistory;    /* index relative to nHistory to the entry of the history which will be shown next */

    /* the position of the cursor in the window */
    unsigned xPos;
    unsigned yPos;

    unsigned xPosInputLineStart;
    unsigned yPosInputLineStart;

    unsigned yPosInputLineStartValid;

    /* count for how often we scrolled */
    unsigned scroll_up;

    unsigned xMax;
    unsigned yMax;

    /* the dimensions of one character in the window */
    unsigned xCharDimension;
    unsigned yCharDimension;

    HWND hwndConsole;   /* the HWND of the window */
    HWND hwndPreviousActive;
    HWND hwndParent;
    HDC  hdc;           /* a DC for writing inside the window */

    BOOLEAN bIsMdiChild;
    HWND hwndMdiClient;

    int xWindow;       /* the position of the window for re-opening */
    int yWindow;       /* the position of the window for re-opening */
    BOOLEAN bInputReady;

    BOOLEAN bCursorBlinkOn;
    int CursorSuspended;

    char achInputBuffer[MAX_WIDTH+1];
    unsigned cntInputBuffer;
    unsigned posInputBuffer;
    BOOLEAN bInsertMode;

    BOOLEAN bMarkMode;
    BOOLEAN bMarkModeBlock; /* if true, mark a rectangular block; else, mark as in an editor */
    BOOLEAN bIsMarked;
    int xMarkOrigin;
    int yMarkOrigin;
    unsigned xMarkStart;
    unsigned yMarkStart;
    unsigned xMarkEnd;
    unsigned yMarkEnd;

    FILE *fileOutput;    /* file handle for outputting */

    BOOLEAN bBreak;

} console_private_t;

typedef enum cursorstate_e { 
    CS_INIT,
    CS_SUSPEND,
    CS_RESUME,
    CS_RETRIGGER
} cursorstate;

static void cursor(console_private_t *pcp, cursorstate cs);

static void FileOpen(console_private_t *pcp)
{
    pcp->fileOutput = ui_console_save_dialog(pcp->hwndConsole);
}

static void FileClose(console_private_t *pcp)
{
    fclose(pcp->fileOutput);
    pcp->fileOutput = NULL;
}

static void FileOut(console_private_t *pcp, const char *const pstr)
{
    if (pcp->fileOutput) {
        fprintf(pcp->fileOutput, "%s", pstr);
    }
}

/*
 The only non-dynamic variable: Pointer to the first console_private_t
 structure.
*/
static console_private_t *first_window;

static void add_to_history(console_private_t *pcp, const char *entry)
{
    if (entry[0]) {
        /* delete old history entry */
        lib_free(pcp->history[pcp->nHistory]);
        pcp->history[pcp->nHistory] = lib_stralloc(entry);

        pcp->nHistory = (pcp->nHistory + 1) % MAX_HISTORY;
    }
}

static const char *get_history_entry(console_private_t *pcp)
{
    return pcp->history[(pcp->nHistory - pcp->nCurrentSelectHistory + MAX_HISTORY) % MAX_HISTORY];
    /*
    remark: "+ MAX_HISTORY" to avoid the following portability problem:
    what means -3 % 7? A compiler might output -3 or 4 at its own choice,
    which are both mathematically correct!
    */
}

// calculate a TCHAR pointer into the pchWindowBuffer
#define CALC_POS(_pcp, _xxx, _yyy) ((_pcp)->xMax * (_yyy) + (_xxx))

// calculate the size of a void pointer block in the pchWindowBuffer
#define CALC_SIZE(_pcp, _xxx, _yyy) (((_pcp)->xMax * (_yyy) + (_xxx)) * sizeof(TCHAR))

static void mark_window_i(console_private_t *pcp, BOOLEAN bMark)
{
    static BOOLEAN bIsMarked = FALSE;
    static int xMinOld, yMinOld, xMaxOld, yMaxOld;

    int xMin, yMin, xMax, yMax;
    RECT rect;

    if (bMark) {
        assert(pcp->yMarkStart <= pcp->yMarkEnd);
        assert((pcp->yMarkStart < pcp->yMarkEnd) || (pcp->xMarkStart <= pcp->xMarkEnd));

        xMinOld = xMin = pcp->xMarkStart;
        yMinOld = yMin = pcp->yMarkStart;
        xMaxOld = xMax = pcp->xMarkEnd;
        yMaxOld = yMax = pcp->yMarkEnd;

        bIsMarked = TRUE;
    } else {
        if (!bIsMarked) {
            return;
        }

        xMin = xMinOld;
        yMin = yMinOld;
        xMax = xMaxOld;
        yMax = yMaxOld;
        bIsMarked = FALSE;
    }

    cursor(pcp, CS_SUSPEND);

    if (pcp->bMarkModeBlock) {
        rect.left = xMin * pcp->xCharDimension;
        rect.top = yMin * pcp->yCharDimension;
        rect.right = xMax * pcp->xCharDimension;
        rect.bottom = yMax * pcp->yCharDimension;

        InvertRect(pcp->hdc, &rect);
    } else {
        if (yMin + 1 == yMax) {
            /* we have just one line */
            rect.left = xMin * pcp->xCharDimension;
            rect.top = yMin * pcp->yCharDimension;
            rect.right = xMax * pcp->xCharDimension;
            rect.bottom = yMax * pcp->yCharDimension;
        
            InvertRect(pcp->hdc, &rect);
        } else {
            /* handle first line */
            rect.left = xMin * pcp->xCharDimension;
            rect.top = yMin * pcp->yCharDimension;
            rect.right = pcp->xMax * pcp->xCharDimension;
            rect.bottom = (yMin + 1) * pcp->yCharDimension;

            InvertRect(pcp->hdc, &rect);

            /* handle intermediate lines */
            rect.left = 0;
            rect.top = (yMin + 1) * pcp->yCharDimension;
            rect.right = pcp->xMax * pcp->xCharDimension;
            rect.bottom = (yMax - 1) * pcp->yCharDimension;

            InvertRect(pcp->hdc, &rect);

            /* handle last line */
            rect.left = 0;
            rect.top = (yMax - 1) * pcp->yCharDimension;
            rect.right = xMax * pcp->xCharDimension;
            rect.bottom = yMax * pcp->yCharDimension;

            InvertRect(pcp->hdc, &rect);
        }
    }

    cursor(pcp, CS_RESUME);
}

static void unmark_window(console_private_t *pcp)
{
    mark_window_i(pcp, FALSE);
}

static void mark_window(console_private_t *pcp)
{
    unmark_window(pcp);

    if (pcp->bIsMarked) {
        mark_window_i(pcp, TRUE);
    }
}

static void redraw_window(console_private_t *pcp, LPPAINTSTRUCT pps)
{
    unsigned row;

    unsigned yMin = 0;
    unsigned yMax = pcp->pConsole->console_yres;

    unsigned xMin = 0;
    unsigned xMax = pcp->pConsole->console_xres;

    cursor(pcp, CS_SUSPEND);

    unmark_window(pcp);

#if 0
    if (pps)
    {
        /* we have an update region, so update only necessary parts */
        xMin = pps->rcPaint.left / pcp->xCharDimension;
        yMin = pps->rcPaint.top / pcp->yCharDimension;

        /*
         the "+ ..." force a rounding up.
        */
        xMax = (pps->rcPaint.right + pcp->xCharDimension-1) / pcp->xCharDimension;
        yMax = (pps->rcPaint.bottom + pcp->yCharDimension-1) / pcp->yCharDimension;
    }
#endif 

    for (row = yMin; row < yMax; row++) {
        /* draw a single line */
        TextOut(pcp->hdc, xMin * pcp->xCharDimension, row * pcp->yCharDimension, &(pcp->pchWindowBuffer[CALC_POS(pcp, xMin, row)]), xMax);
    }

    mark_window(pcp);

    cursor(pcp, CS_RESUME);
}

static void clear_buffer(TCHAR *pos, unsigned count)
{
    while (count != 0)
    {
        *pos++ = TEXT(' ');
        count--;
    }
}

static void move_upwards(console_private_t *pcp)
{
    if (pcp->yPos > 0) {
        --pcp->yPos;
    }
}

static void scroll_up(console_private_t *pcp)
{
    cursor(pcp, CS_SUSPEND);

    /* mark that we scrolled up by one line */
    ++pcp->scroll_up;

    /* move all lines one line up */
    memmove(pcp->pchWindowBuffer, &pcp->pchWindowBuffer[pcp->xMax], CALC_SIZE(pcp, 0, pcp->yMax - 1));

    /* clear the last line */
    clear_buffer(&pcp->pchWindowBuffer[CALC_POS(pcp, 0, pcp->yMax - 1)], pcp->xMax);

    move_upwards(pcp);

    /* force repainting of the whole window */
#if 0
    /*
     @SRT this variant takes less processor time because
     hopefully, the window needs not to be updated with
     every single scroll.
    */
    InvalidateRect(pcp->hwndConsole, NULL, FALSE );
#else
    /*
     @SRT this variant looks more realistic since every
     single scroll can be seen by the user
    */
    redraw_window(pcp, NULL);
#endif

    /* Adjust the line where the input begins */
    pcp->yPosInputLineStart--;

    cursor(pcp, CS_RESUME);
}

static void move_downwards(console_private_t *pcp)
{
    if (++pcp->yPos >= pcp->pConsole->console_yres - 1) {
        /* we must scroll the window */
        scroll_up(pcp);
    }
}

static void move_backwards(console_private_t *pcp)
{
    if (pcp->xPos > 0) {
        --pcp->xPos;
    } else {
        pcp->xPos = pcp->pConsole->console_xres - 1;
        move_upwards(pcp);
    }
}

static void move_forwards(console_private_t *pcp)
{
    if (++pcp->xPos >= pcp->pConsole->console_xres) {
        pcp->xPos = 0;
        move_downwards(pcp);
    }
}

/*
 calculate the dimensions of one character 
 and set console_private_t structure accordingly
*/
static void get_char_dimensions(console_private_t *pcp)
{
    SIZE size;

    GetTextExtentPoint32(pcp->hdc, TEXT(" "), 1, &size);

    pcp->xCharDimension  = size.cx;
    pcp->yCharDimension = size.cy; 
}

static void size_window(console_private_t *pcp)
{ 
    RECT rect;

    cursor(pcp, CS_SUSPEND);

    GetClientRect(pcp->hwndConsole, &rect);

    ClientToScreen(pcp->hwndConsole, (LPPOINT)&rect);
    ClientToScreen(pcp->hwndConsole, ((LPPOINT)&rect) + 1);

    if (pcp->bIsMdiChild) {
        ScreenToClient(pcp->hwndMdiClient, (LPPOINT)&rect);
        ScreenToClient(pcp->hwndMdiClient, ((LPPOINT)&rect) + 1);
    }

    rect.right = rect.left + pcp->pConsole->console_xres * pcp->xCharDimension;
    rect.bottom = rect.top + pcp->pConsole->console_yres * pcp->yCharDimension;

    AdjustWindowRect(&rect, (DWORD)GetWindowLongPtr(pcp->hwndConsole, GWL_STYLE), FALSE);

    MoveWindow(pcp->hwndConsole, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, TRUE);

    pcp->xWindow = rect.left;
    pcp->yWindow = rect.top;

    cursor(pcp, CS_RESUME);
}

/*
 allocate memory for the window buffer 
 and initializie console_private_t structure
*/
static console_private_t *allocate_window_memory(console_private_t *pcp)
{
    if (!pcp->pchWindowBuffer) {
        unsigned n;

        n = pcp->xMax * pcp->yMax;

        /* allocate buffer for window contents */
        pcp->pchWindowBuffer = lib_malloc(sizeof(TCHAR) * n);

        /* clear the buffer with spaces */
        clear_buffer(pcp->pchWindowBuffer, n);
    }

    return pcp;
}

static console_private_t *reallocate_window_memory(console_private_t* pcp, unsigned xDim, unsigned yDim)
{
    unsigned xOldDim = pcp->xMax;
    unsigned yOldDim = pcp->yMax;
    TCHAR *pOldBuffer = pcp->pchWindowBuffer;

    unsigned y;

    /* get new memory buffer */
    pcp->pchWindowBuffer = NULL;
    pcp->xMax = max(xDim, pcp->xMax + RESIZE_INCREMENT_X);
    pcp->yMax = max(yDim, pcp->yMax + RESIZE_INCREMENT_Y);

    allocate_window_memory(pcp);

    /* now, copy the contents of the old buffer into the new one */
    for (y = 0; y < yOldDim; y++) {
        memmove(&pcp->pchWindowBuffer[CALC_POS(pcp, 0, y)], &pOldBuffer[xOldDim * y], sizeof(TCHAR) * xOldDim);
    }

    /* we're done, release the old buffer */
    lib_free(pOldBuffer);

    return pcp;
}

static void free_window_memory(console_private_t *pcp)
{
    lib_free(pcp->pchWindowBuffer);
    pcp->pchWindowBuffer = NULL;

    if (pcp->fileOutput) {
        FileClose(pcp);
    }

    lib_free(pcp->pchName);
    lib_free(pcp->pchOnClose);
    lib_free(pcp->pConsole);

    if (pcp->history[0]) {
        int i;

        for (i = 0; i < MAX_HISTORY; i++) {
            if (pcp->history[i] == NULL) {
                break;
            }

            lib_free(pcp->history[i]);
        }
    }

    lib_free(pcp);
}

static void draw_current_character(console_private_t *pcp)
{
    TextOut(pcp->hdc, pcp->xPos * pcp->xCharDimension, pcp->yPos * pcp->yCharDimension, &pcp->pchWindowBuffer[CALC_POS(pcp, pcp->xPos, pcp->yPos)], 1);
}

static BOOLEAN is_cursor_in_marked_area( console_private_t *pcp )
{
    if (pcp->bIsMarked) {
        if (pcp->bMarkModeBlock) {
            return ((pcp->xPos >= pcp->xMarkStart) && (pcp->xPos <= pcp->xMarkEnd) && (pcp->yPos >= pcp->yMarkStart) && (pcp->yPos <= pcp->yMarkEnd)) ? TRUE : FALSE;
        } else {
            // line before the mark?
            if (pcp->yPos < pcp->yMarkStart) {
                return FALSE;
            }

            // line after the mark?
            if (pcp->yPos > pcp->yMarkEnd) {
                return FALSE;
            }

            // same line as start of mark, but before x position?
            if ((pcp->yPos == pcp->yMarkStart) && (pcp->xPos < pcp->xMarkStart)) {
                return FALSE;
            }

            // same line as end of mark, but after x position?
            if ((pcp->yPos == pcp->yMarkEnd) && (pcp->xPos > pcp->xMarkEnd)) {
                return FALSE;
            }

            // if all fails, the cursor is within the mark
            return TRUE;
        }
    }

    return FALSE;
}

static void restore_current_character( console_private_t *pcp )
{
    /* remark: in general, we don't need to bother 
       synchronizing with the blink timer, because the 
       timer cannot effectively expire unless the next
       message is dispatched due to the cooperative
       multitasking in Windows.
    */

    if (pcp->bCursorBlinkOn) {
        pcp->bCursorBlinkOn = FALSE;
        draw_current_character(pcp);

        if (is_cursor_in_marked_area(pcp)) {
            RECT rect;

            rect.left = pcp->xPos * pcp->xCharDimension;
            rect.right = rect.left + pcp->xCharDimension;
            rect.top = pcp->yPos * pcp->yCharDimension;
            rect.bottom = rect.top  + pcp->yCharDimension;

            InvertRect(pcp->hdc, &rect);
        }
    }
}

static void paint_cursor(console_private_t *pcp)
{
    int yFirstCursorLine = pcp->yPos * pcp->yCharDimension + pcp->yCharDimension;
    int yLastCursorLine = pcp->yPos * pcp->yCharDimension + pcp->yCharDimension;
    int xLeft =  pcp->xPos * pcp->xCharDimension;
    int xRight = (pcp->xPos + 1) * pcp->xCharDimension - 1;
    int rop2Old;
    POINT point;

    pcp->bCursorBlinkOn = TRUE;

    yFirstCursorLine -= pcp->yCharDimension / (pcp->bInsertMode ? 2 : 4);

    /*
     paint the cursor
    */

    rop2Old = SetROP2(pcp->hdc, R2_NOT);

    while (yFirstCursorLine < yLastCursorLine) {
        MoveToEx(pcp->hdc, xLeft, yFirstCursorLine, &point);
        LineTo(pcp->hdc, xRight, yFirstCursorLine++);
    }

    SetROP2(pcp->hdc, rop2Old);
}

static void console_out_character(console_private_t *pcp, const unsigned char ch)
{
    cursor(pcp, CS_SUSPEND);

    if (!pcp->bMarkMode) {
        if (pcp->bIsMarked) {
            unmark_window(pcp);
            pcp->bIsMarked = FALSE;
        }
    }

    if (ch >= 32) {
        pcp->pchWindowBuffer[CALC_POS(pcp, pcp->xPos, pcp->yPos)] = ch;
        draw_current_character(pcp);
        move_forwards(pcp);
    } else {
        /* do we have a backspace? */
        if (ch == 8) {
            move_backwards(pcp);
            pcp->pchWindowBuffer[CALC_POS(pcp, pcp->xPos, pcp->yPos)] = ' ';
            draw_current_character(pcp);
        }

        /* do we have a return? */
        if ( (ch == 13) || (ch == '\n')) {
            pcp->xPos = 0;
            move_downwards(pcp);
        }
    }

    cursor(pcp, CS_RESUME);
}

static void console_out_printables_only(console_private_t *pcp, char *buffer, unsigned int length)
{
    cursor(pcp, CS_SUSPEND);

    if (!pcp->bMarkMode && pcp->bIsMarked) {
        unmark_window(pcp);
        pcp->bIsMarked = FALSE;
    }

    /* output line by line, until everything is output */

    while (length > 0) {
        unsigned int partlength = min(pcp->pConsole->console_xres - pcp->xPos, length);

#ifdef WIN32_UNICODE_SUPPORT
        mbstowcs(&pcp->pchWindowBuffer[CALC_POS(pcp, pcp->xPos, pcp->yPos)], buffer, partlength);
#else
        memcpy(&pcp->pchWindowBuffer[CALC_POS(pcp, pcp->xPos, pcp->yPos)], buffer, partlength);
#endif

        /* draw the current line */
        TextOut(pcp->hdc, pcp->xPos * pcp->xCharDimension, pcp->yPos * pcp->yCharDimension, &(pcp->pchWindowBuffer[CALC_POS(pcp, pcp->xPos, pcp->yPos)]), partlength);

        /* advance the buffer by the output part */
        buffer += partlength;
        length -= partlength;

        /* advance the current screen position */
        pcp->xPos += partlength;

        /* handle wrap-around, if necessary */
        if (pcp->xPos == pcp->pConsole->console_xres) {
            pcp->xPos = 0;
            move_downwards(pcp);
        }
    }

    cursor(pcp, CS_RESUME);
}

static void advance_pos(console_private_t *pcp, unsigned int count)
{
    int xPos = pcp->xPos + count;
    int yAdd = xPos / pcp->pConsole->console_xres;

    pcp->xPos = xPos % pcp->pConsole->console_xres;
    pcp->yPos += yAdd;
}

static void draw_current_input( console_private_t *pcp )
{
    int xPos = pcp->xPos;
    int yPos = pcp->yPos;

    if (!pcp->yPosInputLineStartValid) {
        pcp->xPosInputLineStart = pcp->xPos;
        pcp->yPosInputLineStart = pcp->yPos;
        pcp->cntInputBuffer = 0;
    }

    /* go to where the input line begins */
    pcp->xPos = pcp->xPosInputLineStart;
    pcp->yPos = pcp->yPosInputLineStart;

    /* set zero at end of input buffer */
    pcp->achInputBuffer[pcp->cntInputBuffer] = 0;

    /* initialize to find out if the output was scrolled up */
    pcp->scroll_up = 0;

    cursor(pcp, CS_SUSPEND);

    console_out(pcp->pConsole, "%s", pcp->achInputBuffer);

    /* output a blank to delete a possibly character after the
       input (needed for outputting after backspace or delete
    */
    console_out_character(pcp, ' ');

    /* restore cursor position */
    pcp->xPos = xPos;
    pcp->yPos = yPos - pcp->scroll_up;

    pcp->yPosInputLineStart -= pcp->scroll_up;

    cursor(pcp, CS_RESUME);
}

static void cursor(console_private_t *pcp, cursorstate cs)
{
    switch(cs) {
        case CS_INIT:
            pcp->bCursorBlinkOn = FALSE;
            pcp->CursorSuspended = 1;
            /* FALL THROUGH */
        case CS_RETRIGGER:
            SetTimer(pcp->hwndConsole, 1, 500, NULL);
            break;
        case CS_RESUME:
            pcp->bCursorBlinkOn = FALSE;
            if (--pcp->CursorSuspended == 0) {
                cursor(pcp, CS_RETRIGGER);
            }
            break;
        case CS_SUSPEND:
            if (pcp->CursorSuspended++ == 0) {
                KillTimer(pcp->hwndConsole, 1);
                restore_current_character(pcp);
            }
            break;
    }
}

int console_out(console_t *log, const char *format, ...)
{
    console_private_t *pcp = log->private;

    va_list ap;

    char *buffer;
    char *pBuffer;

    pcp->bBreak = FALSE;

    va_start(ap, format);
    buffer = lib_mvsprintf(format, ap);
    va_end(ap);
    pBuffer = buffer;

    FileOut(pcp, pBuffer);

    /* restore character under cursor */
    cursor(pcp, CS_SUSPEND);

    /* perform an optimization:
     * as long as no special char is used, output line by line.
     * only special chars (ascii < 32) are handled individually.
     */
    while (*pBuffer) {
        char *p = pBuffer;

        /* advance p to the first non-printable char (< 32) */
        while (*p >= 32) {
            p++;
        }

        /* output the printable parts */
        if (p - pBuffer > 0) {
            console_out_printables_only(pcp, pBuffer, (int)(p - pBuffer));
        }

        /* now, process the exta char(s), if any */
        while (*p && (*p < 32)) {
            console_out_character(pcp, *p++);
        }

        pBuffer = p;
    }

    cursor(pcp, CS_RESUME);

    lib_free(buffer);

    return pcp->bBreak ? -1 : 0;
}

char *console_in(console_t *log, const char *prompt)
{
    console_private_t *pcp = log->private ;

    char *p;

    console_out(log, "%s", prompt);

    pcp->xPosInputLineStart = pcp->xPos;
    pcp->yPosInputLineStart = pcp->yPos;
    pcp->yPosInputLineStartValid = 1;

    pcp->posInputBuffer = pcp->cntInputBuffer = 0;
    pcp->bInsertMode = TRUE;
    pcp->bInputReady = FALSE;

    pcp->nCurrentSelectHistory = 0;

    /* allow the cursor to start */
    cursor(pcp, CS_RESUME);

    do {
        ui_dispatch_next_event();
    } while (!pcp->bInputReady);

    /* stop the cursor */
    cursor(pcp, CS_SUSPEND);

    p = lib_stralloc(pcp->achInputBuffer);

    /* Remove trailing newlines.  */
    {
        int len;

        for (len = (int)strlen(p); len > 0 && (p[len - 1] == '\r' || p[len - 1] == '\n'); len--) {
            p[len - 1] = '\0';
        }
    }

    /* output input as output into file */
    FileOut(pcp, p);
    FileOut(pcp, "\n");

    add_to_history(pcp, p);

    return p;
}

static void replace_current_input(console_private_t *pcp, const char *p)
{
    unsigned nOldBufferLength = pcp->cntInputBuffer;

    /* restore character under cursor */
    cursor(pcp, CS_SUSPEND);

    strcpy( pcp->achInputBuffer, p );
    pcp->cntInputBuffer = (unsigned int)strlen(pcp->achInputBuffer);

    draw_current_input(pcp);

    if (!pcp->yPosInputLineStartValid) {
        pcp->xPosInputLineStart = pcp->xPos;
        pcp->yPosInputLineStart = pcp->yPos;
    }

    pcp->xPos = pcp->xPosInputLineStart;
    pcp->yPos = pcp->yPosInputLineStart;
    advance_pos(pcp, pcp->cntInputBuffer);

    pcp->posInputBuffer = pcp->cntInputBuffer;

    /* test: is the old line longer than the new one? */
    if (pcp->cntInputBuffer < nOldBufferLength) {
        /* yes, delete old lines with blanks */
        int xPos = pcp->xPos;
        int yPos = pcp->yPos;

        nOldBufferLength -= pcp->cntInputBuffer;

        while (nOldBufferLength-- > 0) {
            console_out_character(pcp, ' ');
        }

        pcp->xPos = xPos;
        pcp->yPos = yPos;
    }

    cursor(pcp, CS_RESUME);
}

static void remember_current_input_and_delete(console_private_t *pcp, char **pOldInput)
{
    /* finish the current input buffer with a zero */
    pcp->achInputBuffer[pcp->cntInputBuffer] = 0;

    /* remember the current input buffer in another variable */
    *pOldInput = lib_stralloc(pcp->achInputBuffer);

    /* delete the current input line */
    replace_current_input(pcp, "");
}

static void external_resize_window( console_private_t *pcp, int nWidth, int nHeight )
{
    /*
     the "+ ..." force a rounding up.
    */
    unsigned xDim = (nWidth + pcp->xCharDimension - 1) / pcp->xCharDimension;
    unsigned yDim = (nHeight + pcp->yCharDimension - 1) / pcp->yCharDimension;

    HWND hwndFrame;

    char *currentInput;

    cursor(pcp, CS_SUSPEND);

    /* First of all, remember the current input line */
    remember_current_input_and_delete(pcp, &currentInput);

    /* 
    test if window is bigger than ever before; if so,
    get new memory for new window buffer
    */
    if ((xDim > pcp->xMax) || (yDim > pcp->yMax)) {
        reallocate_window_memory(pcp, xDim, yDim);
    }

    pcp->pConsole->console_xres = xDim;
    pcp->pConsole->console_yres = yDim;

    /* make sure the cursor is inside the visible area */
    while (pcp->yPos >= yDim - 1) {
        scroll_up(pcp);
    }

    hwndFrame = (HWND)GetWindowLongPtr((HWND)GetWindowLongPtr(pcp->hwndConsole, GWLP_HWNDPARENT), GWLP_HWNDPARENT);
    SendMessage(hwndFrame, WM_CONSOLE_RESIZED, 0, 0);

    /* now, restore the current input */
    replace_current_input(pcp, currentInput);

    lib_free(currentInput);

    cursor(pcp, CS_RESUME);
}

static void WriteInClipboard(HWND hwnd, const char *buffer)
{
    HGLOBAL hgm = GlobalAlloc(GMEM_MOVEABLE, strlen(buffer) + 1);
    LPVOID lp  = GlobalLock(hgm);

    lstrcpy(lp, buffer);

    OpenClipboard(hwnd);
    EmptyClipboard();
    SetClipboardData(CF_TEXT, hgm);
    CloseClipboard();
}

static void MarkModeInClipboard(console_private_t *pcp)
{
    if (pcp->bIsMarked) {
        /*
            +2 because: CR/LF is added per line
        1st +1 because: we need a trailing zero
        2nd +1 because: we add a zero at the *beginning* (so that the removal
                        of the trailing spaces will not go before the buffer.
        */
        char *buffer = lib_malloc((pcp->xMax + 2) * pcp->yMax + 1 + 1);
        char *p = buffer + 1;

        unsigned xMin, yMin, xMax, yMax, row;

        buffer[0] = 0; // make sure that removing trailing spaces will not go beyond this

        assert(pcp->yMarkStart <= pcp->yMarkEnd);
        assert((pcp->yMarkStart < pcp->yMarkEnd) || (pcp->xMarkStart <= pcp->xMarkEnd));

        xMin = pcp->xMarkStart;
        yMin = pcp->yMarkStart;
        xMax = pcp->xMarkEnd;
        yMax = pcp->yMarkEnd;

        if (pcp->bMarkModeBlock) {
            for (row = yMin; row < yMax; row++) {
                assert(xMax >= xMin);
                memcpy(p, &pcp->pchWindowBuffer[CALC_POS(pcp, xMin, row)], xMax - xMin);
                p += xMax - xMin;

                /* delete trailing spaces */
                assert(p > buffer);
                while (p[-1] == ' ') {
                    --p;
                    assert(p > buffer);
                }

                *p++ = 13;
                *p++ = 10;
            }
        } else {
            if (yMin + 1 == yMax) {
                /* we have just one line */
                assert(xMax >= xMin);
                memcpy(p, &pcp->pchWindowBuffer[CALC_POS(pcp, xMin, yMin)], xMax - xMin);
                p += xMax - xMin;

                /* delete trailing spaces */
                assert(p > buffer);
                while (p[-1] == ' ') {
                    --p;
                    assert(p > buffer);
                }
            } else {
                /* handle first line */
                assert(pcp->xMax >= xMin);
                memcpy(p, &pcp->pchWindowBuffer[CALC_POS(pcp, xMin, yMin)], pcp->xMax-xMin);
                p += pcp->xMax - xMin;

                /* delete trailing spaces */
                assert(p > buffer);
                while (p[-1] == ' ') {
                    --p;
                    assert(p > buffer);
                }

                *p++ = 13;
                *p++ = 10;

                /* handle intermediate lines */
                for (row = yMin + 1; row < yMax - 1; row++) {
                    memcpy(p, &pcp->pchWindowBuffer[CALC_POS(pcp, 0, row)], pcp->xMax);
                    p += pcp->xMax;

                    /* delete trailing spaces */
                    assert(p > buffer);
                    while (p[-1] == ' ') {
                        --p;
                        assert(p > buffer);
                    }

                    *p++ = 13;
                    *p++ = 10;
                }

                /* handle last line */
                memcpy(p, &pcp->pchWindowBuffer[CALC_POS(pcp, 0, yMax - 1)], xMax);
                p += xMax;

                /* delete trailing spaces */
                assert(p > buffer);
                while (p[-1] == ' ') {
                    --p;
                    assert(p > buffer);
                }
            }
        }

        assert(p > buffer);
        assert(p < &buffer[(pcp->xMax + 2) * pcp->yMax + 1 + 1]);
        *p = 0;

        // +1 because we have written a leading zero at buffer[0]
        WriteInClipboard(pcp->hwndConsole, buffer + 1);
        lib_free(buffer);
    }
}

static BOOLEAN MarkModeStart(console_private_t *pcp, int fwKeys, short xPos, short yPos, BOOLEAN bMarkModeBlock)
{
    if (pcp->bMarkMode) {
        return FALSE;
    }

    unmark_window(pcp);
    pcp->bMarkModeBlock = bMarkModeBlock;

    pcp->bIsMarked = pcp->bMarkMode = TRUE;

    pcp->xMarkOrigin = pcp->xMarkStart = xPos / pcp->xCharDimension;
    pcp->xMarkEnd = pcp->xMarkStart + 1;

    pcp->yMarkOrigin = pcp->yMarkStart = yPos / pcp->yCharDimension;
    pcp->yMarkEnd = pcp->yMarkStart + 1;

    SetCapture(pcp->hwndConsole);

    mark_window(pcp);

    return TRUE;
}

static BOOLEAN MarkModeMove(console_private_t *pcp, int fwKeys, short xPos1, short yPos1)
{
    RECT rect;

    int xPos = xPos1;
    int yPos = yPos1;

    if (!pcp->bMarkMode) {
        return FALSE;
    }

    /* since we capture the mouse, make sure the values are within the visible area */
    GetClientRect(pcp->hwndConsole, &rect);
    if (xPos < rect.left) {
        xPos = rect.left;
    }
    if (yPos < rect.top) {
        yPos = rect.top;
    }
    if (xPos > rect.right) {
        xPos = rect.right;
    }
    if (yPos > rect.bottom) {
        yPos = rect.bottom;
    }

    /* convert window coordinates to text coordinates */
    xPos /= pcp->xCharDimension;
    yPos /= pcp->yCharDimension;

    if (pcp->bMarkModeBlock) {
        pcp->xMarkStart = min(xPos, pcp->xMarkOrigin);
        pcp->xMarkEnd = max(xPos, pcp->xMarkOrigin) + 1;
        pcp->yMarkStart = min(yPos, pcp->yMarkOrigin);
        pcp->yMarkEnd = max(yPos, pcp->yMarkOrigin) + 1;
    } else {
        if (yPos > pcp->yMarkOrigin) {
            pcp->xMarkStart = pcp->xMarkOrigin;
            pcp->xMarkEnd = xPos + 1;
            pcp->yMarkStart = pcp->yMarkOrigin;
            pcp->yMarkEnd = yPos + 1;
        } else if (yPos == pcp->yMarkOrigin) {
            pcp->xMarkStart = min(xPos, pcp->xMarkOrigin);
            pcp->xMarkEnd = max(xPos, pcp->xMarkOrigin) + 1;
            pcp->yMarkStart = yPos;
            pcp->yMarkEnd = yPos + 1;
        } else {
            pcp->xMarkStart = xPos;
            pcp->xMarkEnd = pcp->xMarkOrigin + 1;
            pcp->yMarkStart = yPos;
            pcp->yMarkEnd = pcp->yMarkOrigin + 1;
        }
    }

    assert(pcp->yMarkStart <= pcp->yMarkEnd);
    assert((pcp->yMarkStart < pcp->yMarkEnd) || (pcp->xMarkStart <= pcp->xMarkEnd));

    mark_window(pcp);

    return TRUE;
}

static BOOLEAN MarkModeEnd(console_private_t *pcp, int fwKeys, short xPos, short yPos)
{
    if (!pcp->bMarkMode) {
        return FALSE;
    }

    /* update the marking */
    MarkModeMove(pcp, fwKeys, xPos, yPos);

    pcp->bMarkMode = FALSE;

    ReleaseCapture();

    return TRUE;
}

static BOOLEAN  bIsMdiChild = FALSE;

/* window procedure */
static LRESULT CALLBACK console_window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    console_private_t *pcp = (console_private_t*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (pcp) {
        bIsMdiChild = pcp->bIsMdiChild;
    }

    switch (msg) {
        case WM_GETMINMAXINFO:
            if (bIsMdiChild) {
                DefMDIChildProc(hwnd, msg, wParam, lParam );
            } else {
                DefWindowProc(hwnd, msg, wParam, lParam);
            }

            /* adjust: minimum size */
            if (pcp) {
                LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam; // address of structure

                lpmmi->ptMinTrackSize.x += MIN_XSIZE * pcp->xCharDimension;
                lpmmi->ptMinTrackSize.y += MIN_YSIZE * pcp->yCharDimension;
            }
            return 0;
        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED) {
                if (pcp) {
                    external_resize_window(pcp, LOWORD(lParam), HIWORD(lParam));
                }
            }
            break;
        case WM_CLOSE:
            /* if the window is closed, i.e. by pressing the close
               button, we simulate the typing of a specific line
            */
            /* inform parent that window is closed */
            if (bIsMdiChild) {
                HWND hwndFrame = (HWND)GetWindowLongPtr((HWND)GetWindowLongPtr(hwnd, GWLP_HWNDPARENT), GWLP_HWNDPARENT);
                SendMessage(hwndFrame, WM_CONSOLE_CLOSED, 0, 0);
                pcp->bInputReady = TRUE;
                replace_current_input(pcp, "");
            } else {
                if (pcp->achInputBuffer) {
                    pcp->bInputReady = TRUE;
                    replace_current_input(pcp, pcp->pchOnClose);
                    console_out_character(pcp, '\n');
                }
            }
            break;
        case WM_TIMER:
            if (wParam == 1) {
                if (pcp->CursorSuspended == 0) {
                    if (pcp->bCursorBlinkOn) {
                        /* restore previous character */
                        restore_current_character(pcp);
                    } else {
                        /* paint cursor */
                        paint_cursor(pcp);
                    }
                }

                cursor(pcp, CS_RETRIGGER);
                return 0;
            }
            break;
        case WM_KEYDOWN:
            {
                int nVirtKey = (int)wParam;

                switch (nVirtKey) {
                    case VK_UP:
                        if (pcp->nCurrentSelectHistory < MAX_HISTORY) {
                            const char *p;
                            ++pcp->nCurrentSelectHistory;

                            p = get_history_entry(pcp);

                            if (p) {
                                replace_current_input(pcp, p);
                            } else {
                                /* undo the increment above */
                                /*
                                 remark: get_history_entry() above depends on the
                                 increment! 
                                */
                                --pcp->nCurrentSelectHistory;
                            }
                        }
                        return 0;
                    case VK_DOWN:
                        if (pcp->nCurrentSelectHistory > 1) {
                            --pcp->nCurrentSelectHistory;
                            replace_current_input(pcp, get_history_entry(pcp));
                        } else {
                            pcp->nCurrentSelectHistory = 0;
                            replace_current_input(pcp, "");
                        }
                        return 0;
                    case VK_LEFT:
                        /* restore character under cursor */
                        cursor(pcp, CS_SUSPEND);

                        if (pcp->posInputBuffer > 0) {
                            --pcp->posInputBuffer;
                            move_backwards(pcp);
                        }
                        cursor(pcp, CS_RESUME);
                        return 0;
                    case VK_RIGHT:
                        /* restore character under cursor */
                        cursor(pcp, CS_SUSPEND);

                        if (pcp->posInputBuffer < pcp->cntInputBuffer) {
                            ++pcp->posInputBuffer;
                            move_forwards(pcp);
                        }
                        cursor(pcp, CS_RESUME);
                        return 0;
                    case VK_HOME:
                        /* restore character under cursor */
                        cursor(pcp, CS_SUSPEND);

                        pcp->xPos = pcp->xPosInputLineStart;
                        pcp->yPos = pcp->yPosInputLineStart;

                        pcp->posInputBuffer = 0;

                        cursor(pcp, CS_RESUME);
                        return 0;
                    case VK_END:
                        /* restore character under cursor */
                        cursor(pcp, CS_SUSPEND);

                        pcp->xPos = pcp->xPosInputLineStart;
                        pcp->yPos = pcp->yPosInputLineStart;

                        advance_pos(pcp, pcp->cntInputBuffer);
                        pcp->posInputBuffer = pcp->cntInputBuffer;

                        cursor(pcp, CS_RESUME);
                        return 0;
                    case VK_INSERT:
                        /* repaint the cursor: */
                        cursor(pcp, CS_SUSPEND);
                        pcp->bInsertMode = pcp->bInsertMode ? FALSE : TRUE;
                        cursor(pcp, CS_RESUME);
                        return 0;
                    case VK_DELETE:
                        cursor(pcp, CS_SUSPEND);
                        /* check not to clear more characters than there were */
                        if (pcp->posInputBuffer < pcp->cntInputBuffer) {
                            /* only process del if we're not at the end of the buffer */
                            --pcp->cntInputBuffer;
                            memmove(&pcp->achInputBuffer[pcp->posInputBuffer], &pcp->achInputBuffer[pcp->posInputBuffer + 1], pcp->cntInputBuffer - pcp->posInputBuffer);
                        }

                        draw_current_input(pcp);

                        cursor(pcp, CS_RESUME);
                        return 0;
                }
            }
            break;
        case WM_CONSOLE_INSERTLINE:
            pcp->bInputReady = TRUE;
            break;
        case WM_CHAR:
            {
                /* a key is pressed, process it! */
                char chCharCode = (char)wParam;

                /* restore character under cursor */
                cursor(pcp, CS_SUSPEND);

                if (chCharCode >= 32) {
                    /* it's a printable character, process it */

                    if (pcp->bInsertMode) {
                        /* insert mode */

                        /* only insert if there's room in the buffer */
                        if (pcp->cntInputBuffer < MAX_WIDTH) {
                            ++pcp->cntInputBuffer;

                            memmove(&pcp->achInputBuffer[pcp->posInputBuffer + 1], &pcp->achInputBuffer[pcp->posInputBuffer], pcp->cntInputBuffer - pcp->posInputBuffer);

                            draw_current_input(pcp);

                            pcp->achInputBuffer[pcp->posInputBuffer++] = chCharCode;

                            /* output the character */
                            console_out_character(pcp, chCharCode);
                        }
                    } else {
                        /* overwrite mode */

                        /* processing only if the buffer is not full! */
                        if (pcp->cntInputBuffer < MAX_WIDTH) {
                            pcp->achInputBuffer[pcp->posInputBuffer++] = chCharCode;

                            /* output the character */
                            console_out_character(pcp, chCharCode);

                            /* if we're at the end of the buffer, it's a kind of insert mode */
                            if (pcp->cntInputBuffer < pcp->posInputBuffer) {
                                ++pcp->cntInputBuffer;
                            }
                        }
                    }

                    cursor(pcp, CS_RESUME);
                    return 0;
                }


                switch (chCharCode) {
                    case 8:
                        /* it's a backspace, process it if possible */

                        /* check not to clear more characters than there were */
                        if (pcp->posInputBuffer > 0) {
                            /* move the characters forward */

                            if (pcp->posInputBuffer < pcp->cntInputBuffer) {
                                memmove(&pcp->achInputBuffer[pcp->posInputBuffer - 1], &pcp->achInputBuffer[pcp->posInputBuffer], pcp->cntInputBuffer - pcp->posInputBuffer);

                                --pcp->cntInputBuffer;
                                draw_current_input(pcp);

                                move_backwards(pcp);
                            } else {
                                /* only last character deleted, use faster method */
                                console_out_character(pcp, chCharCode);
                                --pcp->cntInputBuffer;
                            }

                            --pcp->posInputBuffer;
                        }

                        cursor(pcp, CS_RESUME);
                        return 0;
                    case 13:
                        /* it's a CR, so the input is ready */
                        pcp->achInputBuffer[pcp->cntInputBuffer] = 0;
                        pcp->bInputReady = TRUE;

                        console_out_character(pcp, chCharCode);

                        pcp->yPosInputLineStartValid = 0;

                        cursor(pcp, CS_RESUME);
                        return 0;
                    case 12: /* 12 is ASCII for CTRL+L */
                        if (pcp->fileOutput) {
                            FileClose(pcp);
                        } else {
                            FileOpen(pcp);
                        }
                        break;
                    case 3: /* 3 is ASCII for CTRL+C */
                        /* it's a CTRL+C: Copy to clipboard */
                        MarkModeInClipboard(pcp); 

                        cursor(pcp, CS_RESUME);
                        return 0;
                    case 27: /* 27 is an ESCape */
                        pcp->bBreak = TRUE;

                        cursor(pcp, CS_RESUME);
                        return 0;
                }

                /* any other key will be ignored */
            }
            cursor(pcp, CS_RESUME);
            break;
        case WM_MDIACTIVATE:
            if (((HWND)lParam == hwnd) && !IsIconic(hwnd)) {
                SetFocus(hwnd);
            }
            break;
        case WM_LBUTTONDOWN:
            /* the user wants to mark a region */
            if (MarkModeStart(pcp, (int)wParam, LOWORD(lParam), HIWORD(lParam), FALSE)) {
                return 0;
            }
            break;
        case WM_RBUTTONDOWN:
            /* the user wants to mark a region */
            if (MarkModeStart(pcp, (int)wParam, LOWORD(lParam), HIWORD(lParam), TRUE)) {
                return 0;
            }
            break;
        case WM_LBUTTONUP:
            /* FALL THROUGH */
        case WM_RBUTTONUP:
            /* the user wants to mark a region */
            if (MarkModeEnd(pcp, (int)wParam, LOWORD(lParam), HIWORD(lParam))) {
                return 0;
            }
            break;
        case WM_MOUSEMOVE:
            /* the user wants to mark a region */
            if (MarkModeMove(pcp, (int)wParam, LOWORD(lParam), HIWORD(lParam))) {
                return 0;
            }
            break;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;

                BeginPaint(hwnd, &ps);

                redraw_window(pcp, &ps);

                EndPaint(hwnd, &ps);

                return 0;
            }
    }

    if (bIsMdiChild) {
        return DefMDIChildProc(hwnd, msg, wParam, lParam);
    } else {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

static console_private_t *find_console_entry(const char *id)
{
    console_private_t *pcp = first_window;

    while (pcp) {
        if (strcmp(pcp->pchName, id) == 0) {
            break;
        }

        pcp = pcp->pcpNext;
    }

    if (!pcp) {
        console_t *pConsole;
        
        pConsole = lib_malloc(sizeof(console_t));
        pcp = lib_malloc(sizeof(console_private_t));

        /* clear the whole structures */
        memset(pConsole, 0, sizeof(console_t));
        memset(pcp, 0, sizeof(console_private_t));

        /* link the structures to each other */
        pcp->pConsole = pConsole;
        pConsole->private = pcp;

        /* copy the console name into the structure */
        pcp->pchName = lib_stralloc(id);

        /* set the input to be returned when window is closed */
        /* @SRT TODO: this should be set by a function! */
        pcp->pchOnClose = lib_stralloc("x");

        /* do first inits */
        pcp->xMax = pcp->pConsole->console_xres = 80;
        pcp->yMax = pcp->pConsole->console_yres = 25;

        pcp->xWindow = CW_USEDEFAULT;
        pcp->yWindow = CW_USEDEFAULT;

        pcp->fileOutput = NULL;

        /* now, link the console_private_t structure into the list */
        pcp->pcpNext = first_window;
        first_window = pcp;
    }

    return pcp;
}

static console_t *console_open_internal(const char *id, HWND hwndParent, HWND hwndMdiClient, DWORD dwStyle, int x, int y, int dx, int dy)
{
    TCHAR st_id[12];
    console_private_t *pcp;

    system_mbstowcs(st_id, id, 12);

    pcp = find_console_entry(id);

    allocate_window_memory(pcp);

    pcp->hwndParent = hwndParent; 
    bIsMdiChild = pcp->bIsMdiChild = hwndMdiClient ? TRUE : FALSE;
    pcp->hwndMdiClient = hwndMdiClient;

    pcp->pchOnClose = NULL;

    pcp->bIsMarked = pcp->bMarkMode = FALSE;

    cursor(pcp, CS_INIT);

    if (pcp->bIsMdiChild) {
        pcp->hwndConsole = CreateMDIWindow(CONSOLE_CLASS,
                                           st_id,
                                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX | dwStyle,
                                           x, // pcp->xWindow,
                                           y, // pcp->yWindow,
                                           dx, // CW_USEDEFAULT,
                                           dy, // 0,
                                           hwndMdiClient,
                                           winmain_instance,
                                           0);

        /* no previous active window */
        pcp->hwndPreviousActive = NULL;
    } else {
        SuspendFullscreenMode(pcp->hwndParent);

        pcp->hwndConsole = CreateWindow(CONSOLE_CLASS,
                                        st_id,
                                        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX,
                                        pcp->xWindow,
                                        pcp->yWindow,
                                        1,
                                        1,
                                        NULL, // @@@@@@ pcp->hwndParent,
                                        NULL,
                                        winmain_instance,
                                        NULL);

        /* get the previous active window, and set myself active */
        pcp->hwndPreviousActive = SetActiveWindow(pcp->hwndConsole);
    }

    /* get a DC and select proper font */
    pcp->hdc = GetDC(pcp->hwndConsole);
    SelectObject(pcp->hdc, GetStockObject(ANSI_FIXED_FONT));

    /* set colors for output */
    SetTextColor(pcp->hdc, GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor(pcp->hdc, GetSysColor(COLOR_WINDOW));

    /* store pointer to structure with window */
    SetWindowLongPtr(pcp->hwndConsole, GWLP_USERDATA, (UINT_PTR)pcp);

    /* get the dimensions of one char */
    get_char_dimensions(pcp);

    /* set the window to the correct size */
    size_window(pcp);

    /* now show the window */
    ShowWindow(pcp->hwndConsole, SW_SHOW);

    pcp->pConsole->console_can_stay_open = 0;

    return pcp->pConsole;
}

console_t *console_open(const char *id)
{
    return console_open_internal(id, GetActiveWindow(), NULL, 0, 0, 0, 0, 0);
}

console_t *uimon_console_open_mdi(const char *id, void *hw, void *hwndParent, void *hwMdiClient, uint32_t dwStyle, int x, int y, int dx, int dy)
{
    console_t *console_log;

    console_log = console_open_internal(id, *(HWND*)hwndParent, *(HWND*)hwMdiClient, dwStyle, x, y, dx, dy);

    if (hw) {
        *(HWND *)hw = console_log->private->hwndConsole;
    }

    return console_log;
}

int console_close(console_t *log)
{
    console_private_t *pcp = log->private;

    ReleaseDC(pcp->hwndConsole, pcp->hdc);

    DestroyWindow(pcp->hwndConsole);

    pcp->hwndConsole = NULL;

    /* set the previous active window as new active one */
    if (pcp->hwndPreviousActive) {
        SetActiveWindow(pcp->hwndPreviousActive);
    }

    if (!pcp->bIsMdiChild) {
        ResumeFullscreenMode(pcp->hwndParent);
    }

    return 0;
}

int console_init( void )
{
    WNDCLASSEX wc;

    /* mark: we don't have any console_private_t yet */
    first_window = NULL;

    /* Register 2nd window class for the monitor window */
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = console_window_proc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = winmain_instance;
    wc.hIcon = LoadIcon(winmain_instance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0) + 1);
    wc.lpszMenuName = 0; /* @SRT: no menu yet MAKEINTRESOURCE(menu); */
    wc.lpszClassName = CONSOLE_CLASS;
    wc.hIconSm = NULL;

    RegisterClassEx(&wc);

    return 0;
}

int console_close_all( void )
{
    console_private_t *pcp;

    /* step through the windows, close all and free the used memory locations */
    pcp = first_window;

    while (pcp) {
        console_private_t *pcpNext = pcp->pcpNext;

        console_close(pcp->pConsole);
        free_window_memory(pcp);

        pcp = pcpNext;
    }

    first_window = NULL;

    UnregisterClass(CONSOLE_CLASS, winmain_instance);

    return 0;
}
