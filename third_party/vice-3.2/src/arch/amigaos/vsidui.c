/*
 * vsidui.c - Implementation of the VSID-specific part of the UI.
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

#define __USE_INLINE__

#ifdef AMIGA_OS4
#define ASL_PRE_V38_NAMES
#endif

#include <stdio.h>
#include <exec/types.h>
#include <intuition/intuition.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/intuition.h>
#include <proto/dos.h>

#include "private.h"
#include "info.h"
#include "intl.h"
#include "log.h"
#include "pointer.h"
#include "psid.h"
#include "sid.h"
#include "sound.h"
#include "translate.h"
#include "ui.h"
#include "util.h"
#include "vsidui.h"

#include "mui/mui.h"

#define UI_VSID
#define UI_MENU_NAME vsid_ui_translation_menu
#define UI_TRANSLATED_MENU_NAME vsid_ui_menu

#include "vsiduires.h"

static struct Window *vsid_window = NULL;
static struct Menu *vsid_menu = NULL;
static struct Process *self;
static struct Window *orig_windowptr;
static char fname[1024] = "";

static int current_song;
static int songs;
static int default_song;

static int do_quit_vice = 0;

static struct NewMenu *machine_specific_menu = vsid_ui_menu;
static APTR VisualInfo;

const struct TextAttr vsid_font_attr = {
#ifdef AMIGA_MORPHOS
    "XHelvetica.font", 11, FS_NORMAL, 0
#else
    "Helvetica.font", 11, FS_NORMAL, 0
#endif
};

extern void toggle_menu_item(struct Menu *menu, int idm, int checked);

enum {
    VSID_S_TITLE = 0,
    VSID_S_AUTHOR,
    VSID_S_RELEASED,
    VSID_S_SYNC,
    VSID_S_MODEL,
    VSID_S_IRQ,
    VSID_S_PLAYING,
    VSID_S_TIMER,
    VSID_S_LASTLINE
};

static char vsidstrings[VSID_S_LASTLINE + 1][80] = { { 0 } };

static struct IntuiText vsid_text[VSID_S_LASTLINE + 1];

static const ui_menu_toggle_t toggle_list[] = {
    { "Sound", IDM_TOGGLE_SOUND },
    { "WarpMode", IDM_TOGGLE_WARP_MODE },
    { "SaveResourcesOnExit", IDM_TOGGLE_SAVE_SETTINGS_ON_EXIT },
    { "ConfirmOnExit", IDM_TOGGLE_CONFIRM_ON_EXIT },
    { "PSIDKeepEnv", IDM_PSID_OVERRIDE },
    { "SidFilters", IDM_SID_FILTERS },
    { NULL, 0 }
};

static ui_res_possible_values_t SpeedValues[] = {
    { 0, IDM_MAXIMUM_SPEED_NO_LIMIT },
    { 10, IDM_MAXIMUM_SPEED_10 },
    { 20, IDM_MAXIMUM_SPEED_20 },
    { 50, IDM_MAXIMUM_SPEED_50 },
    { 100, IDM_MAXIMUM_SPEED_100 },
    { 200, IDM_MAXIMUM_SPEED_200 },
    { -1, 0 }
};

static const ui_res_value_list_t value_list[] = {
    { "Speed", SpeedValues, IDM_MAXIMUM_SPEED_CUSTOM },
    { NULL, NULL, 0 }
};

static char *vsid_sound_formats[] = {
    "iff",
    "aiff",
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

static int vsid_sample_rates[] = {
    8000,
    11025,
    22050,
    44100
};

static int vsid_buffer_sizes[] = {
    100,
    150,
    200,
    250,
    300,
    350
};

static int vsid_fragment_sizes[] = {
    SOUND_FRAGMENT_SMALL,
    SOUND_FRAGMENT_MEDIUM,
    SOUND_FRAGMENT_LARGE
};

static int vsid_speed_adjustments[] = {
    SOUND_ADJUST_FLEXIBLE,
    SOUND_ADJUST_ADJUSTING,
    SOUND_ADJUST_EXACT
};

static int vsid_volumes[] = {
    100,
    50,
    25,
    10,
    5,
    0
};

static int vsid_output_modes[] = {
    SOUND_OUTPUT_SYSTEM,
    SOUND_OUTPUT_MONO,
    SOUND_OUTPUT_STEREO
};

static int vsid_sid_engines[] = {
    SID_ENGINE_FASTSID
#ifdef HAVE_RESID
   ,SID_ENGINE_RESID
#endif
#ifdef HAVE_CATWEASELMKIII
   ,SID_ENGINE_CATWEASELMKIII
#endif
#ifdef HAVE_HARDSID
   ,SID_ENGINE_HARDSID
#endif
};

static int vsid_fastsid_models[] = {
    SID_MODEL_6581,
    SID_MODEL_8580
};

#ifdef HAVE_RESID
static int vsid_resid_models[] = {
    SID_MODEL_6581,
    SID_MODEL_8580,
    SID_MODEL_8580D
};
#endif

static int vsid_requester(char *title, char *msg, char *buttons, int defval)
{
    struct EasyStruct *uiRequester = NULL;
    int retval;

    uiRequester = lib_AllocMem(sizeof(struct EasyStruct), MEMF_ANY);
    if (uiRequester) {
        uiRequester->es_StructSize = sizeof(struct EasyStruct);
        uiRequester->es_Flags = 0;
        uiRequester->es_Title = title;
        uiRequester->es_TextFormat = msg;
        uiRequester->es_GadgetFormat = buttons;

        retval = EasyRequest(vsid_window, uiRequester, NULL, NULL);
    } else {
        fprintf(stderr,"%s : %s\n",title, msg);
        return defval;
    }
    lib_FreeMem(uiRequester, sizeof(struct EasyStruct));
    return retval;
}

static void vsid_clear_window(void)
{
    ULONG vsid_pen;

    vsid_pen = GetAPen(vsid_window->RPort);
    SetAPen(vsid_window->RPort, GetBPen(vsid_window->RPort));
    RectFill(vsid_window->RPort, 25, 30, 350, 150);
    SetAPen(vsid_window->RPort, vsid_pen);
}

static void vsid_update_text(void)
{
    int i;

    vsid_clear_window();
    for (i = 0; i < VSID_S_LASTLINE; i++) {
        PrintIText(vsid_window->RPort, &vsid_text[i], 0, 0);
    }
}

static char *VSID_BrowseFile(char *select_text, char *pattern)
{
    struct FileRequester *request;

    request = (struct FileRequester *)AllocAslRequestTags(ASL_FileRequest, ASL_Hail, select_text, (struct TagItem *)TAG_DONE);

    if (AslRequestTags(request, ASLFR_Window, vsid_window,
                                ASLFR_InitialDrawer, "PROGDIR:",
                                ASLFR_InitialPattern, pattern,
                                ASLFR_PositiveText, select_text,
                                (struct TagItem *)TAG_DONE)) {
        fname[0] = 0;
        strcat(fname,request->rf_Dir);
        if (fname[strlen(fname) - 1] != (UBYTE)58) {
            strcat(fname, "/");
        }
        strcat(fname, request->rf_File);
        if (fname[strlen(fname) - 1] != (UBYTE)58 && fname[strlen(fname) - 1] != '/') {
            if (request) {
                FreeAslRequest(request);
            }
            return fname;
        }
    }
    if (request) {
        FreeAslRequest(request);
    }

    return NULL;
}

static int vsid_menu_update(void)
{
    int i, j;
    int value;
    int result;

    for (i = 0; toggle_list[i].name != NULL; i++) {
        value = 0;
        resources_get_value(toggle_list[i].name, (void *)&value);
        toggle_menu_item(vsid_menu, toggle_list[i].idm, value);
    }

    for (i = 0; value_list[i].name != NULL; i++) {
        value = -1;
        result = resources_get_value(value_list[i].name, (void *)&value);
        if (result == 0) {
            unsigned int checked = 0;

            toggle_menu_item(vsid_menu, value_list[i].default_idm, 0);
            for (j = 0; value_list[i].vals[j].idm != 0; j++) {
                if (value == value_list[i].vals[j].value && !checked) {
                    toggle_menu_item(vsid_menu, value_list[i].vals[j].idm, 1);
                    checked = 1;
                } else {
                    toggle_menu_item(vsid_menu, value_list[i].vals[j].idm, 0);
                }
            }
            if (checked == 0 && value_list[i].default_idm > 0) {
                toggle_menu_item(vsid_menu, value_list[i].default_idm, 1);
            }
        }
    }

    ResetMenuStrip(vsid_window, vsid_menu);

    return 0;
}

static int vsid_menu_create(void)
{
    struct Screen* pubscreen = NULL;
    int i, j;

    pubscreen = LockPubScreen(NULL);

    if (pubscreen == NULL) {
        return -1;
    }

    if (!(VisualInfo = GetVisualInfo(pubscreen, TAG_DONE))) {
        return -1;
    }

    for (i = 0, j = 0; vsid_ui_translation_menu[i].nm_Type != NM_END; i++) {
        vsid_ui_menu[j].nm_Type = vsid_ui_translation_menu[i].nm_Type;
        vsid_ui_menu[j].nm_CommKey = vsid_ui_translation_menu[i].nm_CommKey;
        vsid_ui_menu[j].nm_Flags = vsid_ui_translation_menu[i].nm_Flags;
        vsid_ui_menu[j].nm_MutualExclude = vsid_ui_translation_menu[i].nm_MutualExclude;
        vsid_ui_menu[j].nm_UserData = vsid_ui_translation_menu[i].nm_UserData;
        switch (vsid_ui_translation_menu[i].nm_Label) {
            case 0:
                vsid_ui_menu[j++].nm_Label = (STRPTR)NM_BARLABEL;
                break;
            default:
                vsid_ui_menu[j++].nm_Label = translate_text(vsid_ui_translation_menu[i].nm_Label);
                break;
        }
    }
    vsid_ui_menu[i].nm_Type = NM_END;
    vsid_ui_menu[i].nm_CommKey = NULL;
    vsid_ui_menu[i].nm_Flags = 0;
    vsid_ui_menu[i].nm_MutualExclude = 0L;
    vsid_ui_menu[i].nm_UserData = NULL;
    vsid_ui_menu[i].nm_Label = NULL;

    if (!(vsid_menu = CreateMenus(vsid_ui_menu, GTMN_FrontPen, 0L, TAG_DONE))) {
        return -1;
    }

    LayoutMenus(vsid_menu, VisualInfo, GTMN_NewLookMenus, TRUE, TAG_DONE);

    SetMenuStrip(vsid_window, vsid_menu);

    vsid_menu_update();

    UnlockPubScreen(NULL, pubscreen);

    return 0;
}

static void vsid_menu_destroy(void)
{
    if (vsid_menu) {
        ClearMenuStrip(vsid_window);
        FreeMenus(vsid_menu);
        vsid_menu = NULL;
    }

    if (VisualInfo) {
        FreeVisualInfo(VisualInfo);
        VisualInfo = NULL;
    }
}

static void vsid_menu_rebuild(void)
{
    vsid_menu_destroy();
    vsid_menu_create();
}

static int vsid_menu_handle(int idm)
{
    char *fname = NULL;
    char *ext = NULL;
    char *tmp = NULL;
    char *curlang;
    int i;

    switch (idm) {
        case IDM_NEXT_TUNE:
            if (current_song < songs) {
                current_song++;
                psid_ui_set_tune(current_song, NULL);
                vsid_ui_display_tune_nr(current_song);
                vsid_ui_set_default_tune(default_song);
                vsid_ui_display_nr_of_tunes(songs);
            }
            break;
        case IDM_PREVIOUS_TUNE:
            if (current_song > 1) {
                current_song--;
                psid_ui_set_tune(current_song, NULL);
                vsid_ui_display_tune_nr(current_song);
                vsid_ui_set_default_tune(default_song);
                vsid_ui_display_nr_of_tunes(songs);
            }
            break;
        case IDM_LOAD_PSID_FILE:
            fname = VSID_BrowseFile(translate_text(IDS_PSID_SELECT), "#?");
            if (fname != NULL) {
                if (machine_autodetect_psid(fname) >= 0) {
                    psid_init_driver();
                    machine_play_psid(0);
                    for (i = 0; i < VSID_S_LASTLINE; i++) {
                        *vsidstrings[i] = 0;
                    }
                    machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
                    songs = psid_tunes(&default_song);
                    current_song = default_song;
                    psid_ui_set_tune(current_song, NULL);
                    vsid_ui_display_tune_nr(current_song);
                    vsid_ui_set_default_tune(default_song);
                    vsid_ui_display_nr_of_tunes(songs);
                }
            }
            break;
        case IDM_RESET_HARD:
            machine_trigger_reset(MACHINE_RESET_MODE_HARD);
            break;
        case IDM_RESET_SOFT:
            machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
            break;
        case IDM_EXIT:
            do_quit_vice = 1;
            break;
        case IDM_SETTINGS_SAVE_FILE:
            fname = VSID_BrowseFile(translate_text(IDS_CONFIG_FILENAME_SELECT), "#?");
            if (fname != NULL) {
                if (resources_save(fname) < 0) {
                    ui_error(translate_text(IDMES_CANNOT_SAVE_SETTINGS));
                } else {
                    ui_message(translate_text(IDMES_SETTINGS_SAVED_SUCCESS));
                }
            }
            break;
        case IDM_SETTINGS_LOAD_FILE:
            fname = VSID_BrowseFile(translate_text(IDS_CONFIG_FILENAME_SELECT), "#?");
            if (fname != NULL) {
                if (resources_load(fname) < 0) {
                    ui_error(translate_text(IDMES_CANNOT_LOAD_SETTINGS));
                } else {
                    ui_message(translate_text(IDMES_SETTINGS_LOAD_SUCCESS));
                }
            }
            break;
        case IDM_SETTINGS_SAVE:
            if (resources_save(NULL) < 0) {
                ui_error(translate_text(IDMES_CANNOT_SAVE_SETTINGS));
            } else {
                ui_message(translate_text(IDMES_SETTINGS_SAVED_SUCCESS));
            }
            break;
        case IDM_SETTINGS_LOAD:
            if (resources_load(NULL) < 0) {
                ui_error(translate_text(IDMES_CANNOT_LOAD_SETTINGS));
            } else {
                ui_message(translate_text(IDMES_SETTINGS_LOAD_SUCCESS));
            }
            break;
        case IDM_SETTINGS_DEFAULT:
            resources_set_defaults();
            ui_message(translate_text(IDMES_DFLT_SETTINGS_RESTORED));
            break;
        case IDM_SAMPLE_RATE:
            i = vsid_requester(translate_text(IDS_SAMPLE_RATE), translate_text(IDS_SAMPLE_RATE), "11025 Hz | 22050 Hz | 44100 Hz | 8000 Hz", 0);
            resources_set_int("SoundSampleRate", vsid_sample_rates[i]);
            break;
        case IDM_BUFFER_SIZE:
            i = vsid_requester(translate_text(IDS_BUFFER_SIZE), translate_text(IDS_BUFFER_SIZE), "150 msec | 200 msec | 250 msec | 300 msec | 350 msec | 100 msec", 0);
            resources_set_int("SoundBufferSize", vsid_buffer_sizes[i]);
            break;
        case IDM_FRAGMENT_SIZE:
            fname = util_concat(translate_text(IDS_MEDIUM), " | ", translate_text(IDS_LARGE), " | ", translate_text(IDS_SMALL), NULL);
            i = vsid_requester(translate_text(IDS_FRAGMENT_SIZE), translate_text(IDS_FRAGMENT_SIZE), fname, 0);
            resources_set_int("SoundFragmentSize", vsid_fragment_sizes[i]);
            lib_free(fname);
            break;
        case IDM_SPEED_ADJUSTMENT:
            fname = util_concat(translate_text(IDS_ADJUSTING), " | ", translate_text(IDS_EXACT), " | ", translate_text(IDS_FLEXIBLE), NULL);
            i = vsid_requester(translate_text(IDS_SPEED_ADJUSTMENT), translate_text(IDS_SPEED_ADJUSTMENT), fname, 0);
            resources_set_int("SoundSpeedAdjustment", vsid_speed_adjustments[i]);
            lib_free(fname);
            break;
        case IDM_VOLUME:
            i = vsid_requester(translate_text(IDS_VOLUME), translate_text(IDS_VOLUME), "50% | 25% | 10% | 5% | 0% | 100%", 0);
            resources_set_int("SoundVolume", vsid_volumes[i]);
            break;
        case IDM_SOUND_OUTPUT_MODE:
            fname = util_concat(translate_text(IDS_MONO), " | ", translate_text(IDS_STEREO), " | ", translate_text(IDS_SYSTEM), NULL);
            i = vsid_requester(translate_text(IDS_SOUND_OUTPUT_MODE), translate_text(IDS_SOUND_OUTPUT_MODE), fname, 0);
            resources_set_int("SoundOutput", vsid_output_modes[i]);
            lib_free(fname);
            break;
        case IDM_SID_ENGINE_MODEL:
            fname = util_concat(
#ifdef HAVE_RESID
                                "ReSID | ",
#endif
#ifdef HAVE_CATWEASELMKIII
                                "Catweasel MK3 | ",
#endif
#ifdef HAVE_HARDSID
                                "HardSID | ",
#endif
                                "Fast SID", NULL);
            i = vsid_requester(translate_text(IDS_SID_ENGINE), translate_text(IDS_SID_ENGINE), fname, 0);
            resources_set_int("SidEngine", vsid_sid_engines[i]);
            lib_free(fname);
            switch (vsid_sid_engines[i]) {
                case SID_ENGINE_FASTSID:
                    i = vsid_requester(translate_text(IDS_SID_MODEL), translate_text(IDS_SID_MODEL), "8580 | 6581", 0);
                    resources_set_int("SidModel", vsid_fastsid_models[i]);
                    break;
#ifdef HAVE_RESID
                case SID_ENGINE_RESID:
                    i = vsid_requester(translate_text(IDS_SID_MODEL), translate_text(IDS_SID_MODEL), "8580 | 8580D | 6581", 0);
                    resources_set_int("SidModel", vsid_resid_models[i]);
                    break;
#endif
            }
            break;
        case IDM_AMOUNT_OF_EXTRA_SIDS:
            i = vsid_requester(translate_text(IDS_AMOUNT_OF_EXTRA_SIDS), translate_text(IDS_AMOUNT_OF_EXTRA_SIDS), "1 | 2 | 0", 0);
            resources_get_int("SidStereo", i);
            break;
#ifdef HAVE_RESID
        case IDM_SAMPLE_METHOD:
            fname = util_concat(translate_text(IDS_INTERPOLATING), " | ", translate_text(IDS_RESAMPLING), " | ", translate_text(IDS_FAST_RESAMPLING), " | ", translate_text(IDS_FAST), NULL);
            i = vsid_requester(translate_text(IDS_SAMPLE_METHOD), translate_text(IDS_SAMPLE_METHOD), fname, 0);
            resources_set_int("SidResidSampling", i);
            lib_free(fname);
            break;
#endif
        case IDM_SOUND_RECORD_START:
            ext = lib_stralloc("AIFF | VOC | WAV");
#ifdef USE_LAMEMP3
            tmp = util_concat(ext, " | MP3", NULL);
            lib_free(ext);
            ext = tmp;
#endif
#ifdef USE_FLAC
            tmp = util_concat(ext, " | FLAC", NULL);
            lib_free(ext);
            ext = tmp;
#endif
#ifdef USE_VORBIS
            tmp = util_concat(ext, " | OGG", NULL);
            lib_free(ext);
            ext = tmp;
#endif
            tmp = util_concat(ext, " | IFF", NULL);
            lib_free(ext);
            ext = tmp;

            i = vsid_requester(translate_text(IDS_SOUND_RECORD_FORMAT), translate_text(IDS_SOUND_RECORD_FORMAT), ext, 0);
            resources_set_string("SoundRecordDeviceName", "");
            resources_set_string("SoundRecordDeviceName", vsid_sound_formats[i]);
            break;
        case IDM_SOUND_RECORD_STOP:
            resources_set_string("SoundRecordDeviceName", "");
            break;
        case IDM_LANGUAGE_ENGLISH:
            resources_get_value("Language", (void *)&curlang);
            if (strcasecmp(curlang, "en")) {
                resources_set_value("Language", (resource_value_t *)"en");
                vsid_menu_rebuild();
            }
            break;
        case IDM_LANGUAGE_DANISH:
            resources_get_value("Language", (void *)&curlang);
            if (strcasecmp(curlang, "da")) {
                resources_set_value("Language", (resource_value_t *)"da");
                vsid_menu_rebuild();
            }
            break;
        case IDM_LANGUAGE_GERMAN:
            resources_get_value("Language", (void *)&curlang);
            if (strcasecmp(curlang, "de")) {
                resources_set_value("Language", (resource_value_t *)"de");
                vsid_menu_rebuild();
            }
            break;
        case IDM_LANGUAGE_SPANISH:
            resources_get_value("Language", (void *)&curlang);
            if (strcasecmp(curlang, "es")) {
                resources_set_value("Language", (resource_value_t *)"es");
                vsid_menu_rebuild();
            }
            break;
        case IDM_LANGUAGE_FRENCH:
            resources_get_value("Language", (void *)&curlang);
            if (strcasecmp(curlang, "fr")) {
                resources_set_value("Language", (resource_value_t *)"fr");
                vsid_menu_rebuild();
            }
            break;
        case IDM_LANGUAGE_ITALIAN:
            resources_get_value("Language", (void *)&curlang);
            if (strcasecmp(curlang, "it")) {
                resources_set_value("Language", (resource_value_t *)"it");
                vsid_menu_rebuild();
            }
            break;
        case IDM_LANGUAGE_KOREAN:
            resources_get_value("Language", (void *)&curlang);
            if (strcasecmp(curlang, "ko")) {
                resources_set_value("Language", (resource_value_t *)"ko");
                vsid_menu_rebuild();
            }
            break;
        case IDM_LANGUAGE_DUTCH:
            resources_get_value("Language", (void *)&curlang);
            if (strcasecmp(curlang, "nl")) {
                resources_set_value("Language", (resource_value_t *)"nl");
                vsid_menu_rebuild();
            }
            break;
        case IDM_LANGUAGE_POLISH:
            resources_get_value("Language", (void *)&curlang);
            if (strcasecmp(curlang, "pl")) {
                resources_set_value("Language", (resource_value_t *)"pl");
                vsid_menu_rebuild();
            }
            break;
        case IDM_LANGUAGE_HUNGARIAN:
            resources_get_value("Language", (void *)&curlang);
            if (strcasecmp(curlang, "hu")) {
                resources_set_value("Language", (resource_value_t *)"hu");
                vsid_menu_rebuild();
            }
            break;
        case IDM_LANGUAGE_RUSSIAN:
            resources_get_value("Language", (void *)&curlang);
            if (strcasecmp(curlang, "ru")) {
                resources_set_value("Language", (resource_value_t *)"ru");
                vsid_menu_rebuild();
            }
            break;
        case IDM_LANGUAGE_SWEDISH:
            resources_get_value("Language", (void *)&curlang);
            if (strcasecmp(curlang, "sv")) {
                resources_set_value("Language", (resource_value_t *)"sv");
                vsid_menu_rebuild();
            }
            break;
        case IDM_LANGUAGE_TURKISH:
            resources_get_value("Language", (void *)&curlang);
            if (strcasecmp(curlang, "tr")) {
                resources_set_value("Language", (resource_value_t *)"tr");
                vsid_menu_rebuild();
            }
            break;
        default:
            {
                int i, j, command_found = 0;

                for (i = 0; toggle_list[i].name != NULL && !command_found; i++) {
                    if (toggle_list[i].idm == idm) {
                        resources_toggle(toggle_list[i].name, NULL);
                        command_found = 1;
                    }
                }

                for (i = 0; value_list[i].name != NULL && !command_found; i++) {
                    for (j = 0; value_list[i].vals[j].idm != 0 && !command_found; j++) {
                        if (value_list[i].vals[j].idm == idm) {
                            resources_set_value(value_list[i].name, (resource_value_t) value_list[i].vals[j].value);
                            command_found = 1;
                        }
                    }
                }
            }
            break;
    }

    return 0;
}

static void vsid_event_handling(void)
{
    int done = 1;
    unsigned long imCode, imClass;
    struct IntuiMessage *imsg;

    /* Check for IDCMP messages */
    while ((imsg = (struct IntuiMessage *)GetMsg(vsid_window->UserPort))) {
        imClass = imsg->Class;
        imCode = imsg->Code;

        switch (imClass) {
            case IDCMP_MENUPICK:
                pointer_to_default();
                while (imCode != MENUNULL) {
                    struct MenuItem *n = ItemAddress(vsid_menu, imCode);
                    vsid_menu_handle((int)GTMENUITEM_USERDATA(n));
                    imCode = n->NextSelect;
                }
                vsid_menu_update();
                done = 1;
                break;
            default:
                break;
        }

        ReplyMsg((struct Message *)imsg);

        switch (imClass) {
            case IDCMP_CLOSEWINDOW:
                do_quit_vice = 1;
                break;
            default:
                break;
        }
    }

    if (do_quit_vice) {
        exit(0); // I think it's safe to quit here
    }
}

int vsid_ui_init(void)
{
    int i;

    vsid_window = (struct Window *)OpenWindowTags(NULL,
                                                  WA_Left, 20,
                                                  WA_Top, 20,
                                                  WA_Width, 400,
                                                  WA_Height, 200,
                                                  WA_Title, (ULONG)"VSID: The VICE SID player",
                                                  WA_DepthGadget, TRUE,
                                                  WA_CloseGadget, TRUE,
                                                  WA_SizeGadget, TRUE,
                                                  WA_DragBar, TRUE,
                                                  WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_MENUPICK | IDCMP_MENUVERIFY,
                                                  TAG_END);
    if (vsid_window == NULL) {
        return -1;
    }

    ui_event_handling = vsid_event_handling;

    if (vsid_menu_create() != 0) {
        return -1;
    }

    for (i = 0; i < VSID_S_LASTLINE; i++) {
        vsidstrings[i][0] = 0;
        vsid_text[i].LeftEdge = 10;
        vsid_text[i].TopEdge = 25 + i * 10;
        vsid_text[i].IText = vsidstrings[i];
        vsid_text[i].ITextFont = &vsid_font_attr;
        vsid_text[i].DrawMode = JAM1;
        vsid_text[i].FrontPen = 1;
        vsid_text[i].NextText = NULL;
    }

    vsid_update_text();

    self = (APTR)FindTask(NULL);
    orig_windowptr = self->pr_WindowPtr;
    self->pr_WindowPtr = vsid_window;

    return mui_init();
}

void vsid_ui_close(void)
{
    vsid_menu_destroy();

    if (vsid_window) {
        CloseWindow(vsid_window);
    }
}

void vsid_ui_display_name(const char *name)
{
    sprintf(vsidstrings[VSID_S_TITLE], "Title: %s", name);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_TITLE]);
    vsid_update_text();
}

void vsid_ui_display_author(const char *author)
{
    sprintf(vsidstrings[VSID_S_AUTHOR], "Author: %s", author);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_AUTHOR]);
    vsid_update_text();
}

void vsid_ui_display_copyright(const char *copyright)
{
    sprintf(vsidstrings[VSID_S_RELEASED], "Released: %s", copyright);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_RELEASED]);
    vsid_update_text();
}

void vsid_ui_display_sync(int sync)
{
    sprintf(vsidstrings[VSID_S_SYNC], "Using %s sync", sync == MACHINE_SYNC_PAL ? "PAL" : "NTSC");
    log_message(LOG_DEFAULT, "%s",vsidstrings[VSID_S_SYNC]);
    vsid_update_text();
}

void vsid_ui_display_sid_model(int model)
{
    sprintf(vsidstrings[VSID_S_MODEL], "Using %s emulation", model == 0 ? "MOS6581" : "MOS8580");
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_MODEL]);
    vsid_update_text();
}

void vsid_ui_display_tune_nr(int nr)
{
    sprintf(vsidstrings[VSID_S_PLAYING], "Playing Tune: %2d /  0  -  Default Tune: 00", nr);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_PLAYING]);
    vsid_update_text();
}

void vsid_ui_display_nr_of_tunes(int count)
{
    char dummy[4];
    sprintf(dummy,"%2d", count);

    log_message(LOG_DEFAULT, "Number of Tunes: %i", count);
    vsidstrings[VSID_S_PLAYING][19] = dummy[0];
    vsidstrings[VSID_S_PLAYING][20] = dummy[1];
    vsid_update_text();
}

void vsid_ui_set_default_tune(int nr)
{
    char dummy[4];
    sprintf(dummy,"%2d", nr);

    log_message(LOG_DEFAULT, "Default Tune: %i", nr);
    vsidstrings[VSID_S_PLAYING][40] = dummy[0];
    vsidstrings[VSID_S_PLAYING][41] = dummy[1];
    vsid_update_text();
}

void vsid_ui_display_time(unsigned int sec)
{
}

void vsid_ui_display_irqtype(const char *irq)
{
    sprintf(vsidstrings[VSID_S_IRQ], "Using %s interrupt", irq);
    log_message(LOG_DEFAULT, "Using %s interrupt", irq);
    vsid_update_text();
}

void vsid_ui_setdrv(char* driver_info_text)
{
}


/** \brief  Set driver address
 *
 * \param[in]   addr    driver address
 */
void vsid_ui_set_driver_addr(uint16_t addr)
{
}


/** \brief  Set load address
 *
 * \param[in]   addr    load address
 */
void vsid_ui_set_load_addr(uint16_t addr)
{
}


/** \brief  Set init routine address
 *
 * \param[in]   addr    init routine address
 */
void vsid_ui_set_init_addr(uint16_t addr)
{
}


/** \brief  Set play routine address
 *
 * \param[in]   addr    play routine address
 */
void vsid_ui_set_play_addr(uint16_t addr)
{
}


/** \brief  Set size of SID on actual machine
 *
 * \param[in]   size    size of SID
 */
void vsid_ui_set_data_size(uint16_t size)
{
}

