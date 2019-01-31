/*
 * intl.c - Localization routines for Amiga.
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

#define __USE_INLINE__

#include <proto/locale.h>

#include "archdep.h"
#include "cmdline.h"
#include "intl.h"
#include "lib.h"
#include "resources.h"
#include "translate.h"
#include "translate_languages.h"
#include "util.h"
#include "ui.h"

#define countof(array) (sizeof(array) / sizeof((array)[0]))

typedef struct amiga_iso_s {
    char *amiga_locale_language;
    char *iso_language_code;
} amiga_iso_t;


/* this table holds only the currently present translation languages */
static amiga_iso_t amiga_to_iso[] = {
    { "english.language", "en" },
    { "german.language", "de" },
    { "deutsch.language", "de" },
    { "danish.language", "da" },
    { "dansk.language", "da" },
    { "spanish.language", "es" },
    { "french.language", "fr" },
    { "français.language", "fr" },
    { "hungarian.language", "hu" },
    { "italian.language", "it" },
    { "italiano.language", "it" },
    { "korean.language", "ko" },
    { "dutch.language", "nl" },
    { "nederlands.language", "nl" },
    { "polish.language", "pl" },
    { "polski.language", "pl" },
    { "russian.language", "ru" },
    { "swedish.language", "sv" },
    { "svenska.language", "sv" },
    { "turkish.language", "tr" },
    { NULL, NULL }
};

/* --------------------------------------------------------------------- */

typedef struct intl_translate_s {
    int resource_id;
    char *text;
} intl_translate_t;

#include "intl_text.c"

#include "intl_table.h"

/* --------------------------------------------------------------------- */

static char *intl_text_table[countof(intl_translate_text_table)][countof(language_table)];

void intl_convert_mui_table(int text_id[], char *text[])
{
    int i;

    for (i = 0; text_id[i] > 0; i++) {
        text[i] = intl_translate_text(text_id[i]);
    }
    if (text_id[i] == 0) {
        text[i] = NULL;
    }
}

/* codepage conversion is not yet present */

char *intl_convert_cp(char *text, int cp)
{
    int len;
    char *buffer;

    if (text == NULL) {
        return NULL;
    }

    len = strlen(text);

    if (len == 0) {
        return NULL;
    }

    buffer = lib_stralloc(text);

    return buffer;
}

static char *intl_get_string_by_id(int id)
{
    unsigned int k;

    for (k = 0; k < countof(intl_string_table); k++) {
        if (intl_string_table[k].resource_id==id) {
            return intl_string_table[k].text;
        }
    }
    return NULL;
}

char *intl_translate_text(int en_resource)
{
    unsigned int i;

    for (i = 0; i < countof(intl_translate_text_table); i++) {
        if (intl_translate_text_table[i][0] == en_resource) {
            if (intl_translate_text_table[i][current_language_index] != 0 &&
                intl_text_table[i][current_language_index] != NULL &&
                strlen(intl_text_table[i][current_language_index]) != 0) {
                return intl_text_table[i][current_language_index];
            } else {
                return intl_text_table[i][0];
            }
        }
    }
    return "";
}

/* pre-translated main window caption text so the emulation won't
   slow down because of all the translation calls */

char *intl_speed_at_text;

/* --------------------------------------------------------------------- */

void intl_init(void)
{
    unsigned int i,j;

    for (i = 0; i < countof(language_table); i++) {
        for (j = 0; j < countof(intl_translate_text_table); j++) {
            if (intl_translate_text_table[j][i] == 0) {
                intl_text_table[j][i] = NULL;
            } else {
                intl_text_table[j][i] = (char *)intl_get_string_by_id(intl_translate_text_table[j][i]);
            }
        }
    }
}

int intl_translate_res(int resource)
{
    return resource;
}

void intl_shutdown(void)
{
}

/* --------------------------------------------------------------------- */


static void intl_update_pre_translated_text(void)
{
    intl_speed_at_text = intl_translate_text(IDS_S_AT_D_SPEED);
}

char *intl_arch_language_init(void)
{
    int i;
    struct Locale *amigalang;

    amigalang = OpenLocale(NULL);
    CloseLocale(amigalang);
    for (i = 0; amiga_to_iso[i].iso_language_code != NULL; i++) {
        if (!strcasecmp(amiga_to_iso[i].amiga_locale_language, amigalang->loc_LanguageName)) {
            return amiga_to_iso[i].iso_language_code;
        }
    }
    return "en";
}

void intl_update_ui(void)
{
    intl_update_pre_translated_text();
    ui_update_menus();
}
