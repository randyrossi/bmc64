/*
 * intl.c - Localization routines for Win32.
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

#include "vice.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <windows.h>

#include "archdep.h"
#include "cmdline.h"
#include "intl.h"
#include "lib.h"
#include "machine.h"
#include "res.h"
#include "resources.h"
#include "stringid.h"
#include "sysfile.h"
#include "translate.h"
#include "translate_languages.h"
#include "util.h"
#include "ui.h"
#include "winmain.h"

#define countof(array) (sizeof(array) / sizeof((array)[0]))

typedef struct windows_iso_s {
    int windows_code;
    char *iso_language_code;
    int code_page;
    int hotkey_cap;
} windows_iso_t;

/* this table holds only the currently present translation
   languages, to add a new translation look at
   http://www.unicode.org/unicode/onlinedat/languages.html
   for the corresponding windows and iso codes */

static windows_iso_t windows_to_iso[] = {
    { LANG_ENGLISH, "en", 28591, 1 },
    { LANG_DANISH, "da", 28591, 1 },
    { LANG_GERMAN, "de", 28591, 1 },
    { LANG_SPANISH, "es", 28591, 1 },
    { LANG_FRENCH, "fr", 28591, 1 },
    { LANG_HUNGARIAN, "hu", 28592, 1 },
    { LANG_ITALIAN, "it", 28591, 1 },
    { LANG_KOREAN, "it", 949, 0 },
    { LANG_DUTCH, "nl", 28591, 1 },
    { LANG_POLISH, "pl", 28592, 1 },
    { LANG_RUSSIAN, "ru", 28595, 1 },
    { LANG_SWEDISH, "sv", 28591, 1 },
    { LANG_TURKISH, "tr", 28599, 1 },
    { 0, NULL, 0, 0 }
};

static int intl_table[][countof(language_table)] = {

/* res.rc */
/* en */ { IDR_MENUMONITOR,
/* da */   IDR_MENUMONITOR_DA,    /* fuzzy */
/* de */   IDR_MENUMONITOR_DE,
/* es */   IDR_MENUMONITOR_ES,    /* fuzzy */
/* fr */   IDR_MENUMONITOR_FR,
/* hu */   IDR_MENUMONITOR_HU,
/* it */   IDR_MENUMONITOR_IT,
/* ko */   IDR_MENUMONITOR_KO,    /* fuzzy */
/* nl */   IDR_MENUMONITOR_NL,
/* pl */   IDR_MENUMONITOR_PL,    /* fuzzy */
/* ru */   IDR_MENUMONITOR_RU,    /* fuzzy */
/* sv */   IDR_MENUMONITOR_SV,
/* tr */   IDR_MENUMONITOR_TR },  /* fuzzy */

};

/* --------------------------------------------------------------------- */

/* codepage 28591 (ISO 8859-1) to current codepage conversion tables */

static char cp28591[256];

static const WCHAR wcp28591[256] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
    0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
    0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
    0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
    0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x00d0, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
    0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
    0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x00f0, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
    0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x00ff
};


/* codepage 28592 (ISO 8859-2) to current codepage conversion tables */

static char cp28592[256];

static const WCHAR wcp28592[256] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
    0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
    0x00a0, 0x0104, 0x02d8, 0x0141, 0x00a4, 0x013d, 0x015a, 0x00a7,
    0x00a8, 0x0160, 0x015e, 0x0164, 0x0179, 0x00ad, 0x017d, 0x017b,
    0x00b0, 0x0105, 0x02db, 0x0142, 0x00b4, 0x013e, 0x015b, 0x02c7,
    0x00b8, 0x0161, 0x015f, 0x0165, 0x017a, 0x02dd, 0x017e, 0x017c,
    0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7,
    0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e,
    0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7,
    0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df,
    0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7,
    0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f,
    0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7,
    0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9
};


/* codepage 28595 (ISO 8859-5) to current codepage conversion tables */

static char cp28595[256];

static const WCHAR wcp28595[256] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
    0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
    0x00a0, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407,
    0x0408, 0x0409, 0x040a, 0x040b, 0x040c, 0x00ad, 0x040e, 0x040f,
    0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
    0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
    0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
    0x0428, 0x0429, 0x042a, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f,
    0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
    0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f,
    0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
    0x0448, 0x0449, 0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f,
    0x2116, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457,
    0x0458, 0x0459, 0x045a, 0x045b, 0x045c, 0x00a7, 0x045e, 0x045f
};


/* codepage 28599 (ISO 8859-9) to current codepage conversion tables */

static char cp28599[256];

static const WCHAR wcp28599[256] = {
    0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006, 0x0007,
    0x0008, 0x0009, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x000f,
    0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016, 0x0017,
    0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e, 0x001f,
    0x0020, 0x0021, 0x0022, 0x0023, 0x0024, 0x0025, 0x0026, 0x0027,
    0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d, 0x002e, 0x002f,
    0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0037,
    0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d, 0x003e, 0x003f,
    0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047,
    0x0048, 0x0049, 0x004a, 0x004b, 0x004c, 0x004d, 0x004e, 0x004f,
    0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057,
    0x0058, 0x0059, 0x005a, 0x005b, 0x005c, 0x005d, 0x005e, 0x005f,
    0x0060, 0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067,
    0x0068, 0x0069, 0x006a, 0x006b, 0x006c, 0x006d, 0x006e, 0x006f,
    0x0070, 0x0071, 0x0072, 0x0073, 0x0074, 0x0075, 0x0076, 0x0077,
    0x0078, 0x0079, 0x007a, 0x007b, 0x007c, 0x007d, 0x007e, 0x007f,
    0x0080, 0x0081, 0x0082, 0x0083, 0x0084, 0x0085, 0x0086, 0x0087,
    0x0088, 0x0089, 0x008a, 0x008b, 0x008c, 0x008d, 0x008e, 0x008f,
    0x0090, 0x0091, 0x0092, 0x0093, 0x0094, 0x0095, 0x0096, 0x0097,
    0x0098, 0x0099, 0x009a, 0x009b, 0x009c, 0x009d, 0x009e, 0x009f,
    0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
    0x00a8, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
    0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
    0x00b8, 0x00b9, 0x00ba, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00bf,
    0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
    0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
    0x011e, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
    0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x0130, 0x015e, 0x00df,
    0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
    0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
    0x011f, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
    0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x0131, 0x015f, 0x00ff
};

char *intl_convert_cp(char *text, int cp)
{
    int len,i;
    char *cp_table;
    char *buffer;

    if (text == NULL) {
        return NULL;
    }
    len = (int)strlen(text);
    if (len == 0) {
        return NULL;
    }
    switch (cp) {
        case 28591:
            cp_table = cp28591;
            break;
        case 28592:
            cp_table = cp28592;
            break;
        case 28595:
            cp_table = cp28595;
            break;
        case 28599:
            cp_table = cp28599;
            break;
        default:
            cp_table = cp28591;
    }
    buffer = lib_stralloc(text);
    for (i = 0; i < len; i++) {
        buffer[i] = cp_table[(unsigned char)text[i]];
    }
    return buffer;
}

/* --------------------------------------------------------------------- */

static int hotkeys_loaded = 0;

typedef struct windows_hotkey_s {
    char hotkey;
    int occurence;
} windows_hotkey_t;

static windows_hotkey_t windows_hotkeys[LAST_IDS + 16][countof(language_table)];

static char line_buffer[512];

#define FOUND_UNKNOWN_LINE  0
#define FOUND_EMPTY_LINE    1
#define FOUND_COMMENT_LINE  2
#define FOUND_IDS_LINE      3

static int gethotkeyline(FILE *file)
{
    int c = 0;
    int counter = 0;

    while (c != '\n' && !feof(file) && counter < 511) {
        c = fgetc(file);
        if (c != 0xd) {
            line_buffer[counter++] = (char)c;
        }
    }
    line_buffer[counter] = 0;

    if (line_buffer[0] == 0) {
        return FOUND_EMPTY_LINE;
    }

    if (line_buffer[0] == '#') {
        return FOUND_COMMENT_LINE;
    }

    if (line_buffer[0] == 'I' && line_buffer[1] == 'D' && line_buffer[2] == 'S' && line_buffer[3] == '_') {
        return FOUND_IDS_LINE;
    }

    return FOUND_UNKNOWN_LINE;
}

static void intl_set_hotkey(void)
{
    int i = 0;
    int ids = 0, langid = -1;
    int occurence = 1;
    int found_string = 0;
    char *langpos = NULL;
    char *hotkeypos = NULL;
    char *occpos = NULL;

    /* get to the first 'space' */
    while (!isspace(line_buffer[i]) && line_buffer[i] != 0) {
        i++;
    }

    /* check if end-of-line */
    if (line_buffer[i] == 0) {
        return;
    }

    line_buffer[i++] = 0;

    /* skip spaces */
    while (isspace(line_buffer[i])) {
        i++;
    }

    /* check if end-of-line */
    if (line_buffer[i] == 0) {
        return;
    }

    langpos = line_buffer + i;

    /* get the the first 'space' */
    while (!isspace(line_buffer[i]) && line_buffer[i] != 0) {
        i++;
    }

    /* check if end-of-line */
    if (line_buffer[i] == 0) {
        return;
    }

    line_buffer[i++] = 0;

    /* skip spaces */
    while (isspace((unsigned char)line_buffer[i])) {
        i++;
    }

    /* check if end-of-line */
    if (line_buffer[i] == 0) {
        return;
    }

    hotkeypos = line_buffer + i;

    /* get the the first 'space' */
    while (!isspace((unsigned char)line_buffer[i]) && line_buffer[i] != 0) {
        i++;
    }

    /* get optional occurence number */
    if (line_buffer[i] != 0) {

        line_buffer[i++] = 0;

        /* skip spaces */
        while (isspace(line_buffer[i])) {
            i++;
        }
        if (line_buffer[i] != 0) {
            occpos = line_buffer + i;
        }
    }

    /* find the ids belonging to the IDS_* string */
    for (i = 0; idslist[i].str != NULL && found_string == 0; i++) {
        if (!strcmp(idslist[i].str, line_buffer)) {
            found_string = 1;
            ids = idslist[i].cmd;
        }
    }

    /* check if ids not found */
    if (found_string == 0) {
        return;
    }

    /* find the language number belonging to the language identifier string */
    if (!strcmp(langpos, "**")) {
        found_string = 1;
        langid = -1;
    } else {
        found_string = 0;
        for (i = 0; i < countof(language_table) && found_string == 0; i++) {
            if (!strcmp(language_table[i], langpos)) {
                langid = i;
                found_string = 1;
            }
        }
    }

    /* check if language found */
    if (found_string == 0) {
        return;
    }

    /* check if the hotkey is only 1 character */
    if (strlen(hotkeypos) != 1) {
        return;
    }

    /* check for the optional occurence number */
    if (occpos != NULL) {
        occurence = atoi(occpos);
    }

    /* put entry into the hotkey table */
    if (langid == -1) {
        for (i = 0; i < countof(language_table); i++) {
            if (windows_to_iso[i].hotkey_cap) {
                windows_hotkeys[ids][i].hotkey = hotkeypos[0];
                windows_hotkeys[ids][i].occurence = occurence;
            }
        }
    } else {
        if (windows_to_iso[langid].hotkey_cap) {
            windows_hotkeys[ids][langid].hotkey = hotkeypos[0];
            windows_hotkeys[ids][langid].occurence = occurence;
        }
    }
}

static int intl_load_hotkey_table(void)
{
    int i, j, found;
    FILE *fhotkeys;
    char *name;

    /* init table */
    for (i = 0; i < LAST_IDS; i++) {
        for (j = 0; j < countof(language_table); j++) {
            windows_hotkeys[i][j].hotkey = 0;
            windows_hotkeys[i][j].occurence = 0;
        }
    }

    name = util_concat(archdep_boot_path(), "\\", machine_name, "/win_hotkeys.vhk", NULL);
    fhotkeys = fopen(name, MODE_READ_TEXT);
    lib_free(name);
    if (fhotkeys == NULL) {
        return 0;
    }

    while (!feof(fhotkeys)) {
        found = gethotkeyline(fhotkeys);
        if (found == FOUND_IDS_LINE) {
            intl_set_hotkey();
        }
    }
    fclose(fhotkeys);
    return 1;
}

static char *intl_add_hotkey(char *text, int ids, int lang)
{
    char *ret = NULL;
    int i = 0;
    int j = 0;
    int occ = 0;

    if (windows_hotkeys[ids][lang].hotkey == 0 || text == NULL || text[0] == 0) {
        return text;
    } else {
        ret = lib_malloc(strlen(text) + 2);
        while (text[i] != 0) {
            if (text[i] == windows_hotkeys[ids][lang].hotkey) {
                occ++;
                if (occ == windows_hotkeys[ids][lang].occurence) {
                    ret[j++] = '&';
                }
            }
            ret[j++] = text[i++];
        }
        ret[j] = 0;
        lib_free(text);

        return ret;
    }
}

/* --------------------------------------------------------------------- */

#ifdef WIN32_UNICODE_SUPPORT
static WCHAR *intl_wcs_table[LAST_IDS + 16][countof(language_table)];
static BYTE wcs_cache[(((LAST_IDS / 16) + 1) / 8) + 1];
#endif

static char *intl_text_table[LAST_IDS + 16][countof(language_table)];
static BYTE text_cache[(((LAST_IDS / 16) + 1) / 8) + 1];

/* It also prepares any codepage conversion tables. */

static void intl_text_init(void)
{
#ifdef WIN32_UNICODE_SUPPORT
    ZeroMemory(intl_wcs_table, sizeof(intl_wcs_table));
    ZeroMemory(wcs_cache, sizeof(wcs_cache));
#endif

    ZeroMemory(intl_text_table, sizeof(intl_text_table));
    ZeroMemory(text_cache, sizeof(text_cache));

    /* prepare the codepage 28591 (ISO 8859-1) to current codepage conversion */
    WideCharToMultiByte(CP_ACP, 0, wcp28591, 256, cp28591, 256, NULL, NULL);

    /* prepare the codepage 28592 (ISO 8859-2) to current codepage conversion */
    WideCharToMultiByte(CP_ACP, 0, wcp28592, 256, cp28592, 256, NULL, NULL);

    /* prepare the codepage 28595 (ISO 8859-5) to current codepage conversion */
    WideCharToMultiByte(CP_ACP, 0, wcp28595, 256, cp28595, 256, NULL, NULL);

    /* prepare the codepage 28599 (ISO 8859-9) to current codepage conversion */
    WideCharToMultiByte(CP_ACP, 0, wcp28599, 256, cp28599, 256, NULL, NULL);

    /* load the hotkey table */
    hotkeys_loaded = intl_load_hotkey_table();
}

static void intl_text_free(void)
{
    unsigned int i, j;

    for (i = 0; i < countof(language_table); i++) {
#ifdef WIN32_UNICODE_SUPPORT
        for (j = 0; j < countof(intl_wcs_table); j++) {
            lib_free(intl_wcs_table[j][i]);
        }
#endif
        for (j = 0; j < countof(intl_text_table); j++) {
            lib_free(intl_text_table[j][i]);
        }
    }
}

#ifdef WIN32_UNICODE_SUPPORT
WCHAR *intl_translate_wcs(int en_resource)
{
    WCHAR *p, *text;
    unsigned int i;
    int j, k;
    HRSRC hRes;
    HGLOBAL hGlob;
    int length;

    if (!(wcs_cache[en_resource >> 7] & (1 << ((en_resource >> 4) & 7)))) {
        j = (en_resource >> 4) + 1;
        for (i = 0; i < countof(language_table); i++) {
            hRes = FindResourceEx(NULL, RT_STRING, MAKEINTRESOURCE(j), (WORD)MAKELANGID(windows_to_iso[i].windows_code, SUBLANG_NEUTRAL));
            if (hRes) {
                hGlob = LoadResource(NULL, hRes);
                p = LockResource(hGlob);
                for (k = 0; k < 16; k++) {
                    length = *p++;
                    text = lib_malloc((length + 1) * sizeof(WCHAR));
                    ZeroMemory(text, (length + 1) * sizeof(WCHAR));
                    memcpy(text, p, length * sizeof(WCHAR));
                    p = p + length;
                    intl_wcs_table[((j - 1) << 4) + k][i] = text;
/*                  if (hotkeys_loaded) {
                        intl_text_table[((j - 1) << 4) + k][i] = intl_add_hotkey(intl_text_table[((j - 1) << 4) + k][i], ((j - 1) << 4) + k, i);
                    } */
                }
                FreeResource(hGlob);
            } else {
                for (k = 0; k < 16; k++) {
                    intl_wcs_table[((j - 1) << 4) + k][i] = NULL;
                }
            }
        }
        wcs_cache[en_resource >> 7] |= (1 << ((en_resource >> 4) & 7));
    }
    text = intl_wcs_table[en_resource][current_language_index];
    if (text == NULL) {
        text = intl_wcs_table[en_resource][0];
    }
    return text;
}
#endif

char *intl_translate_text(int en_resource)
{
    char *text;
    unsigned int i;
    int j, k;
    char temp_buffer[4098 * sizeof(TCHAR)];
    HRSRC hRes;
    HGLOBAL hGlob;
    WORD *p;
    int length;

    if (!(text_cache[en_resource >> 7] & (1 << ((en_resource >> 4) & 7)))) {
        j = (en_resource >> 4) + 1;
        for (i = 0; i < countof(language_table); i++) {
            hRes = FindResourceEx(NULL, RT_STRING, MAKEINTRESOURCE(j), (WORD)MAKELANGID(windows_to_iso[i].windows_code, SUBLANG_NEUTRAL));
            if (hRes) {
                hGlob = LoadResource(NULL, hRes);
                p = LockResource(hGlob);
                for (k = 0; k < 16; k++) {
                    ZeroMemory(temp_buffer, sizeof(temp_buffer));
                    length = *p++;
                    WideCharToMultiByte(CP_ACP, 0, p, length, temp_buffer, 4096, NULL, NULL);
                    p = p + length;
                    intl_text_table[((j - 1) << 4) + k][i] = lib_stralloc(temp_buffer);
                    if (hotkeys_loaded) {
                        intl_text_table[((j - 1) << 4) + k][i] = intl_add_hotkey(intl_text_table[((j - 1) << 4) + k][i], ((j - 1) << 4) + k, i);
                    }
                }
                FreeResource(hGlob);
            } else {
                for (k = 0; k < 16; k++) {
                    intl_text_table[((j - 1) << 4) + k][i] = NULL;
                }
            }
        }
        text_cache[en_resource >> 7] |= (1 << ((en_resource >> 4) & 7));
    }
    text = intl_text_table[en_resource][current_language_index];
    if (text == NULL) {
        text = intl_text_table[en_resource][0];
    }
    return text;
}

/* pre-translated main window caption text so the emulation won't
   slow down because of all the translation calls */

TCHAR *intl_speed_at_text;

/* --------------------------------------------------------------------- */

void intl_init(void)
{
    intl_text_init();
}

int intl_translate_res(int en_resource)
{
    unsigned int i;

    if (!strcmp(current_language,"en")) {
        return en_resource;
    }

    for (i = 0; i < countof(intl_table); i++) {
        if (intl_table[i][0] == en_resource) {
            return intl_table[i][current_language_index];
        }
    }
    return en_resource;
}

void intl_shutdown(void)
{
    intl_text_free();
}

/* --------------------------------------------------------------------- */

static void intl_update_pre_translated_text(void)
{
    intl_speed_at_text = intl_translate_tcs(IDS_S_AT_D_SPEED);
}

char *intl_arch_language_init(void)
{
/* unused current language recognition code */
#if 0
    int i;
    WORD winlang;

    winlang = GetUserDefaultLangID() & 0x3ff;
    for (i = 0; windows_to_iso[i].iso_language_code != NULL; i++) {
        if (windows_to_iso[i].windows_code == winlang) {
            return windows_to_iso[i].iso_language_code;
        }
    }
#endif
    return "en";
}

void intl_update_ui(void)
{
    intl_update_pre_translated_text();
    ui_update_menu();
}
