/*
 * uisound.c
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

#include "sound.h"
#include "uisound.h"
#include "intl.h"
#include "translate.h"

static video_canvas_t *sound_canvas;

static char *ui_sound_freq[] = {
    "8000 Hz",
    "11025 Hz",
    "22050 Hz",
    "44100 Hz",
    NULL
};

static const int ui_sound_freq_values[] = {
    8000,
    11025,
    22050,
    44100,
    -1
};

static char *ui_sound_buffer[] = {
    "100 msec",
    "150 msec",
    "200 msec",
    "250 msec",
    "300 msec",
    "350 msec",
    NULL
};

static const int ui_sound_buffer_values[] = {
    100,
    150,
    200,
    250,
    300,
    350,
    -1
};

static int ui_sound_adjusting_translate[] = {
    IDS_FLEXIBLE,
    IDS_ADJUSTING,
    IDS_EXACT,
    0
};

static char *ui_sound_adjusting[countof(ui_sound_adjusting_translate)];

static const int ui_sound_adjusting_values[] = {
    SOUND_ADJUST_FLEXIBLE,
    SOUND_ADJUST_ADJUSTING,
    SOUND_ADJUST_EXACT,
    -1
};

static int ui_sound_fragment_size_translate[] = {
    IDS_VERY_SMALL,
    IDS_SMALL,
    IDS_MEDIUM,
    IDS_LARGE,
    IDS_VERY_LARGE,
    0
};

static char *ui_sound_fragment_size[countof(ui_sound_fragment_size_translate)];

static const int ui_sound_fragment_size_values[] = {
    SOUND_FRAGMENT_VERY_SMALL,
    SOUND_FRAGMENT_SMALL,
    SOUND_FRAGMENT_MEDIUM,
    SOUND_FRAGMENT_LARGE,
    SOUND_FRAGMENT_VERY_LARGE,
    -1
};

static int ui_sound_output_mode_translate[] = {
    IDS_SYSTEM,
    IDS_MONO,
    IDS_STEREO,
    0
};

static char *ui_sound_output_mode[countof(ui_sound_output_mode_translate)];

static const int ui_sound_output_mode_values[] = {
    SOUND_OUTPUT_SYSTEM,
    SOUND_OUTPUT_MONO,
    SOUND_OUTPUT_STEREO,
    -1
};

static char *ui_sound_formats[] = {
    "aiff",
    "iff",
    "voc",
    "wav",
#ifdef USE_LAMEMP3
    "mp3",
#endif
#ifdef USE_FLAC
    "flac",
#endif
#ifdef USE_VORBIS
    "ogg",
#endif
    NULL
};

static const int ui_volume_range[] = {
    0,
    100
};

static ui_to_from_t ui_to_from[] = {
    { NULL, MUI_TYPE_CYCLE, "SoundSampleRate", ui_sound_freq, ui_sound_freq_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SoundBufferSize", ui_sound_buffer, ui_sound_buffer_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SoundFragmentSize", ui_sound_fragment_size, ui_sound_fragment_size_values, NULL },
    { NULL, MUI_TYPE_CYCLE, "SoundSpeedAdjustment", ui_sound_adjusting, ui_sound_adjusting_values, NULL },
    { NULL, MUI_TYPE_INTEGER, "SoundVolume", NULL, ui_volume_range, NULL },
    { NULL, MUI_TYPE_CYCLE, "SoundOutput", ui_sound_output_mode, ui_sound_output_mode_values, NULL },
    UI_END /* mandatory */
};

static ui_to_from_t ui_to_from_record[] = {
    { NULL, MUI_TYPE_FILENAME, "SoundRecordDeviceArg", NULL, NULL, NULL },
    UI_END /* mandatory */
};

static ULONG Browse_Record_File(struct Hook *hook, Object *obj, APTR arg)
{
    char *fname = NULL;

    fname = BrowseFile(translate_text(IDS_SELECT_RECORD_FILE), "#?", sound_canvas);

    if (fname != NULL) {
        set(ui_to_from_record[0].object, MUIA_String_Contents, fname);
    }

    return 0;
}

static APTR build_gui(void)
{
    return GroupObject,
             CYCLE(ui_to_from[0].object, translate_text(IDS_SAMPLE_RATE), ui_sound_freq)
             CYCLE(ui_to_from[1].object, translate_text(IDS_BUFFER_SIZE), ui_sound_buffer)
             CYCLE(ui_to_from[2].object, translate_text(IDS_FRAGMENT_SIZE), ui_sound_fragment_size)
             CYCLE(ui_to_from[3].object, translate_text(IDS_SPEED_ADJUSTMENT), ui_sound_adjusting)
             NSTRING(ui_to_from[4].object, translate_text(IDS_VOLUME), "0123456789", 5+1)
             CYCLE(ui_to_from[5].object, translate_text(IDS_SOUND_OUTPUT_MODE), ui_sound_output_mode)
           End;
}

static APTR format;

static APTR build_gui_record(void)
{
    APTR app, ui, ok, browse_button, cancel;

#ifdef AMIGA_MORPHOS
    static const struct Hook BrowseRecordHook = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)Browse_Record_File, NULL };
#else
    static const struct Hook BrowseRecordHook = { { NULL, NULL }, (VOID *)Browse_Record_File, NULL, NULL };
#endif

    app = mui_get_app();

    ui = GroupObject,
           CYCLE(format, translate_text(IDS_SOUND_RECORD_FORMAT), ui_sound_formats)
           FILENAME(ui_to_from_record[0].object, translate_text(IDS_SOUND_RECORD_FILE), browse_button)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        DoMethod(browse_button, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_CallHook, &BrowseRecordHook);
    }

    return ui;
}

void ui_sound_settings_dialog(void)
{
    intl_convert_mui_table(ui_sound_adjusting_translate, ui_sound_adjusting);
    intl_convert_mui_table(ui_sound_fragment_size_translate, ui_sound_fragment_size);
    intl_convert_mui_table(ui_sound_output_mode_translate, ui_sound_output_mode);
    mui_show_dialog(build_gui(), translate_text(IDS_SOUND_SETTINGS), ui_to_from);
}

void ui_sound_record_settings_dialog(video_canvas_t *canvas)
{
    APTR window;
    int val;

    sound_canvas = canvas;

    window = mui_make_simple_window(build_gui_record(), translate_text(IDS_SOUND_RECORD_SETTINGS));

    if (window != NULL) {
        mui_add_window(window);
        ui_get_to(ui_to_from_record);
        set(format, MUIA_Cycle_Active, 1);
        set(window, MUIA_Window_Open, TRUE);
        if (mui_run() == BTN_OK) {
            ui_get_from(ui_to_from_record);
            get(format, MUIA_Cycle_Active, (APTR)&val);
            resources_set_string("SoundRecordDeviceName", "");
            resources_set_string("SoundRecordDeviceName", ui_sound_formats[val]);
            ui_display_statustext(translate_text(IDS_SOUND_RECORDING_STARTED), 1);
        }
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}
