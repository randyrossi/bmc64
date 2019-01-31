/*
 * uidrivepetcbm2.c
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
#ifdef AMIGA_M68K
#define _INLINE_MUIMASTER_H
#endif
#include "mui.h"

#include "drive.h"
#include "uidrivepetcbm2.h"
#include "intl.h"
#include "translate.h"

static int drive_number_strings_translate[] = {
    IDMS_DRIVE_8,
    IDMS_DRIVE_9,
    IDMS_DRIVE_10,
    IDMS_DRIVE_11,
    0
};

static char *drive_number_strings[countof(drive_number_strings_translate)];

static int drive_type_strings_translate[] = {
    IDS_NONE,
    -1
};

static char *drive_type_strings_8[] = {
    NULL,		/* "None" placeholder */
    "2031",
    "2040",
    "3040",
    "4040",
    "1001",
    "8050",
    "8250",
    NULL
};

static const int drive_type_values_8[] = {
    DRIVE_TYPE_NONE,
    DRIVE_TYPE_2031,
    DRIVE_TYPE_2040,
    DRIVE_TYPE_3040,
    DRIVE_TYPE_4040,
    DRIVE_TYPE_1001,
    DRIVE_TYPE_8050,
    DRIVE_TYPE_8250,
    -1
};

static char *drive_type_strings_9[] = {
    NULL,		/* "None" placeholder */
    "2031",
    "1001",
    NULL
};

static const int drive_type_values_9[] = {
    DRIVE_TYPE_NONE,
    DRIVE_TYPE_2031,
    DRIVE_TYPE_1001,
    -1
};

static char *drive_type_strings_10[] = {
    NULL,		/* "None" placeholder */
    "2031",
    "1001",
    NULL
};

static const int drive_type_values_10[] = {
    DRIVE_TYPE_NONE,
    DRIVE_TYPE_2031,
    DRIVE_TYPE_1001,
    -1
};

static char *drive_type_strings_11[] = {
    NULL,		/* "None" placeholder */
    "2031",
    "1001",
    NULL
};

static const int drive_type_values_11[] = {
    DRIVE_TYPE_NONE,
    DRIVE_TYPE_2031,
    DRIVE_TYPE_1001,
    -1
};

static int drive_extend_strings_translate[] = {
    IDS_NEVER_EXTEND,
    IDS_ASK_ON_EXTEND,
    IDS_EXTEND_ON_ACCESS,
    0
};

static char *drive_extend_strings[countof(drive_extend_strings_translate)];

static const int drive_extend_values[] = {
    DRIVE_EXTEND_NEVER,
    DRIVE_EXTEND_ASK,
    DRIVE_EXTEND_ACCESS,
    -1
};

static int ui_rpm_range[] = {
    25000,
    35000
};

static int ui_wobble_range[] = {
    0,
    1000
};

#define DECL(device)                                                                                                       \
    { NULL, MUI_TYPE_RADIO,   "Drive" #device "Type", drive_type_strings_ ## device, drive_type_values_ ## device, NULL }, \
    { NULL, MUI_TYPE_RADIO,   "Drive" #device "ExtendImagePolicy", drive_extend_strings, drive_extend_values, NULL },      \
    { NULL, MUI_TYPE_INTEGER, "Drive" #device "RPM", NULL, ui_rpm_range, NULL },                                           \
    { NULL, MUI_TYPE_INTEGER, "Drive" #device "Wobble", NULL, ui_wobble_range, NULL },

#define DECL_NUM (2)

static ui_to_from_t ui_to_from[] = { DECL(8) DECL(9) DECL(10) DECL(11) UI_END };

static APTR build_gui(void)
{
    static char **drive_type_strings[4] = {
        drive_type_strings_8, drive_type_strings_9, drive_type_strings_10, drive_type_strings_11
    };

    APTR window, page;
    ui_to_from_t *data;
    int num;

    window = RegisterObject,
               MUIA_Register_Titles, drive_number_strings,
             End;

    for (num = 0; num < 4; num++) {
        data = &ui_to_from[DECL_NUM * num];
        page = GroupObject,
                 MUIA_Group_Horiz, TRUE,
                 Child, data[0].object = RadioObject,
                   MUIA_Frame, MUIV_Frame_Group,
                   MUIA_FrameTitle, translate_text(IDS_DRIVE_TYPE),
                   MUIA_Radio_Entries, drive_type_strings[num],
                 End,
                 Child, data[1].object = RadioObject,
                   MUIA_Frame, MUIV_Frame_Group,
                   MUIA_FrameTitle, translate_text(IDS_40_TRACK_HANDLING),
                   MUIA_Radio_Entries, drive_extend_strings,
                 End,
                 Child, ui_to_from[2].object = StringObject,
                   MUIA_Frame, MUIV_Frame_String,
                   MUIA_FrameTitle, translate_text(IDS_DRIVE_RPM),
                   MUIA_String_Accept, "0123456789",
                   MUIA_String_MaxLen, 5+1,
                 End,
                 Child, ui_to_from[3].object = StringObject,
                   MUIA_Frame, MUIV_Frame_String,
                   MUIA_FrameTitle, translate_text(IDS_DRIVE_WOBBLE),
                   MUIA_String_Accept, "0123456789",
                   MUIA_String_MaxLen, 5+1,
                 End,
               End;

        DoMethod(window, OM_ADDMEMBER, page);
    }

    return window;
}

void uidrivepetcbm2_settings_dialog(void)
{
    intl_convert_mui_table(drive_number_strings_translate, drive_number_strings);
    intl_convert_mui_table(drive_type_strings_translate, drive_type_strings_8);
    intl_convert_mui_table(drive_type_strings_translate, drive_type_strings_9);
    intl_convert_mui_table(drive_type_strings_translate, drive_type_strings_10);
    intl_convert_mui_table(drive_type_strings_translate, drive_type_strings_11);
    intl_convert_mui_table(drive_extend_strings_translate, drive_extend_strings);
    mui_show_dialog(build_gui(), translate_text(IDS_DRIVE_SETTINGS), ui_to_from);
}
