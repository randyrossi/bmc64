/*
 * ui.c
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

#include <string.h>

#include "private.h"
#include "charset.h"
#include "clipboard.h"
#include "datasette.h"
#include "tape.h"
#include "statusbar.h"
#include "kbd.h"
#include "kbdbuf.h"
#include "keyboard.h"
#include "joy.h"
#include "maincpu.h"
#include "interrupt.h"
#include "vice-event.h"
#include "monitor.h"
#include "info.h"
#include "uilib.h"
#include "lib.h"
#include "cmdline.h"
#include "uiattach.h"
#include "uires.h"
#include "log.h"
#include "pointer.h"
#include "fullscreenarch.h"
#include "fliplist.h"
#include "intl.h"
#include "translate.h"
#include "screen-shot.h"
#include "ui.h"
#include "util.h"
#include "machine.h"
#include "vicefeatures.h"

#include "network.h"
#include "vsyncapi.h"

#include "mui/filereq.h"
#include "mui/mui.h"
#include "mui/uiautostart.h"
#include "mui/uidrivesound.h"
#include "mui/uifliplist.h"
#include "mui/uijamaction.h"

#ifdef AMIGA_OS4
#include "mui/uijoystick.h"
#else
#include "mui/uijoystickll.h"
#endif

#include "mui/uinetwork.h"
#include "mui/uiram.h"
#include "mui/uiscreenshot.h"
#include "mui/uisnapshot.h"
#include "mui/uisound.h"

#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
#include <proto/cybergraphics.h>
#endif

static int do_quit_vice = 0;

/* vsid event handling */
void (*ui_event_handling)(void) = NULL;

struct MenuItem *step_item(struct MenuItem *first_item, int idm)
{
    struct MenuItem *cur_item, *sub_item;

    if (first_item == NULL) {
        return NULL;
    }

    /* Step thru the items */
    for (cur_item = first_item; cur_item != NULL; cur_item = cur_item->NextItem) {
        if (GTMENUITEM_USERDATA(cur_item) == (APTR)idm) {
            return cur_item;
        }
        sub_item = step_item(cur_item->SubItem, idm);
        if (sub_item != NULL) {
            return sub_item;
        }
    }

    return NULL;
}

struct MenuItem *step_menu(struct Menu *first_menu, int idm)
{
    struct Menu *cur_menu;
    struct MenuItem *item;

    /* Step thru the menu structure */
    for (cur_menu = first_menu; cur_menu != NULL; cur_menu = cur_menu->NextMenu) {
        item = step_item(cur_menu->FirstItem, idm);
        if (item != NULL) {
            return item;
        }
    }

    return NULL;
}

void toggle_menu_item(struct Menu *menu, int idm, int checked)
{
    struct MenuItem *item;

    item = step_menu(menu, idm);
    if (item != NULL) {
        if (checked) {
            item->Flags |= CHECKED;
        } else {
            item->Flags &= ~CHECKED;
        }
    }
}

static const ui_menu_toggle_t toggle_list[] = {
    { "Sound", IDM_TOGGLE_SOUND },
    { "DriveTrueEmulation", IDM_TOGGLE_DRIVE_TRUE_EMULATION },
    { "AutostartHandleTrueDriveEmulation", IDM_TOGGLE_AUTOSTART_HANDLE_TDE },
    { "WarpMode", IDM_TOGGLE_WARP_MODE },
    { "VirtualDevices", IDM_TOGGLE_VIRTUAL_DEVICES },
    { "SaveResourcesOnExit", IDM_TOGGLE_SAVE_SETTINGS_ON_EXIT },
    { "ConfirmOnExit", IDM_TOGGLE_CONFIRM_ON_EXIT },
    { "FullScreenEnabled", IDM_TOGGLE_FULLSCREEN },
    { "StatusbarEnabled", IDM_TOGGLE_STATUSBAR },
    { "KeySetEnable", IDM_JOYKEYS_TOGGLE },
    { "JoyOpposite", IDM_OPPOSITE_JOY_DIR },
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
    { "VideoOverlayEnabled", IDM_TOGGLE_OVERLAY },
#endif
    { NULL, 0 }
};

static const ui_res_possible_values_t RefreshRateValues[] = {
    { 0, IDM_REFRESH_RATE_AUTO },
    { 1, IDM_REFRESH_RATE_1 },
    { 2, IDM_REFRESH_RATE_2 },
    { 3, IDM_REFRESH_RATE_3 },
    { 4, IDM_REFRESH_RATE_4 },
    { 5, IDM_REFRESH_RATE_5 },
    { 6, IDM_REFRESH_RATE_6 },
    { 7, IDM_REFRESH_RATE_7 },
    { 8, IDM_REFRESH_RATE_8 },
    { 9, IDM_REFRESH_RATE_9 },
    { 10, IDM_REFRESH_RATE_10 },
    { -1, 0 }
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

static ui_res_possible_values_t RecordingOptions[] = {
    { EVENT_START_MODE_FILE_SAVE, IDM_EVENT_START_MODE_SAVE },
    { EVENT_START_MODE_FILE_LOAD, IDM_EVENT_START_MODE_LOAD },
    { EVENT_START_MODE_RESET, IDM_EVENT_START_MODE_RESET },
    { EVENT_START_MODE_PLAYBACK, IDM_EVENT_START_MODE_PLAYBACK },
    { -1, 0 }
};

static const ui_res_possible_values_t SyncFactor[] = {
    { MACHINE_SYNC_PAL, IDM_SYNC_FACTOR_PAL },
    { MACHINE_SYNC_NTSC, IDM_SYNC_FACTOR_NTSC },
    { MACHINE_SYNC_NTSCOLD, IDM_SYNC_FACTOR_NTSCOLD },
    { -1, 0 }
};

static const ui_res_value_list_t value_list[] = {
    { "RefreshRate", RefreshRateValues, 0 },
    { "Speed", SpeedValues, IDM_MAXIMUM_SPEED_CUSTOM },
    { "MachineVideoStandard", SyncFactor, 0 },
    { "EventStartMode", RecordingOptions, 0 },
    { NULL, NULL, 0 }
};

static struct TranslateNewMenu *machine_specific_translation_menu = NULL;
static struct NewMenu *machine_specific_menu = NULL;
static ui_machine_specific_t machine_specific_function = NULL;
static const ui_menu_toggle_t *machine_specific_toggles = NULL;
static const ui_res_value_list_t *machine_specific_values = NULL;

void ui_register_menu_translation_layout(struct TranslateNewMenu *menu)
{
    machine_specific_translation_menu = menu;
}

void ui_register_menu_layout(struct NewMenu *menu)
{
    machine_specific_menu = menu;
}

void ui_register_machine_specific(ui_machine_specific_t func)
{
    machine_specific_function = func;
}

void ui_register_menu_toggles(const ui_menu_toggle_t *toggles)
{
    machine_specific_toggles = toggles;
}

void ui_register_res_values(const ui_res_value_list_t *valuelist)
{
    machine_specific_values = valuelist;
}

#define ID_FTXT MAKE_ID('F','T','X','T')
#define ID_CHRS MAKE_ID('C','H','R','S')

struct Library *IFFParseBase;

static void ui_copy_clipboard(void)
{
    struct IFFHandle *iff = NULL;
    long unitnumber = 0;
    long error;
    int textlen;
    char *text = NULL;

    do {
        text = clipboard_read_screen_output("\n");

        if (text == NULL) {
            break;
        }

        if (!(IFFParseBase = OpenLibrary("iffparse.library", 0L))) {
            break;
        }

        if (!(iff = AllocIFF())) {
            break;
        }

        if (!(iff->iff_Stream = (ULONG)OpenClipboard(unitnumber))) {
            break;
        }

        InitIFFasClip(iff);

        if ((error = OpenIFF(iff, IFFF_WRITE))) {
            break;
        }

        if (!(error = PushChunk(iff, ID_FTXT, ID_FORM, IFFSIZE_UNKNOWN))) {
            if (!(error = PushChunk(iff, 0, ID_CHRS, IFFSIZE_UNKNOWN))) {
                textlen = strlen(text);
                WriteChunkBytes(iff, text, textlen);
            }
        }

    } while (0);

    if (iff) {
        CloseIFF(iff);

        if (iff->iff_Stream) {
            CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);
        }
        FreeIFF(iff);
    }

    if (IFFParseBase) {
        CloseLibrary(IFFParseBase);
    }

    lib_free(text);
}

static void ui_paste_clipboard_text(void)
{
    struct IFFHandle *iff = NULL;
    struct ContextNode *cn;
    long unitnumber = 0;
    long error;
    int textlen = 0;
    char *text_in_petscii = NULL;
    
    do {
        if (!(IFFParseBase = OpenLibrary("iffparse.library", 0L))) {
            break;
        }

        if (!(iff = AllocIFF())) {
            break;
        }

        if (!(iff->iff_Stream = (ULONG)OpenClipboard(unitnumber))) {
            break;
        }

        InitIFFasClip (iff);

        if ((error = OpenIFF(iff, IFFF_READ))) {
            break;
        }

        if ((error = StopChunk(iff, ID_FTXT, ID_CHRS))) {
            break;
        }

        while (1)
        {
            error = ParseIFF(iff, IFFPARSE_SCAN);
            if (error == IFFERR_EOC) {
                continue;
            } else {
                if (error) {
                    break;
                }
            }

            cn = CurrentChunk(iff);

            if ((cn) && (cn->cn_Type == ID_FTXT) && (cn->cn_ID == ID_CHRS)) {
                textlen = cn->cn_Size;
                text_in_petscii = lib_malloc(textlen + 1);
                error = ReadChunkBytes(iff, text_in_petscii, textlen);
                break;
            }
        }

        if (text_in_petscii == NULL) {
            break;
        }

        text_in_petscii[textlen] = 0;

        charset_petconvstring((uint8_t *)text_in_petscii, 0);

        kbdbuf_feed(text_in_petscii);

    } while (0);

    if (iff) {
        CloseIFF(iff);

        if (iff->iff_Stream) {
            CloseClipboard((struct ClipboardHandle *)iff->iff_Stream);
        }

        FreeIFF(iff);
    }

    if (IFFParseBase) {
        CloseLibrary(IFFParseBase);
    }

    lib_free(text_in_petscii);
}

int ui_menu_create(video_canvas_t *canvas)
{
    struct Screen* pubscreen = NULL;
    int i, j;

    if (machine_specific_menu == NULL) {
        return -1;
    }

    pubscreen = LockPubScreen(NULL);

    if (pubscreen == NULL) {
        return -1;
    }

    if (!(canvas->os->VisualInfo = GetVisualInfo(pubscreen, TAG_DONE))) {
        return -1;
    }

    for (i = 0, j = 0; machine_specific_translation_menu[i].nm_Type != NM_END; i++) {
        if (machine_class == VICE_MACHINE_C64SC) {
            switch (machine_specific_translation_menu[i].nm_Label) {
                /* disable video standard menu for x64sc, and skip item seperator */
                case IDMS_OLD_NTSC_M:
                    i++;
                /* disable video standard menu for x64sc */
                case IDMS_NTSC_M:
                case IDMS_PAL_G:
                case IDMS_VIDEO_STANDARD:
                    continue;
            }
        } else {
            switch (machine_specific_translation_menu[i].nm_Label) {
                /* enable c64 model settings menu item for x64sc only */
                case IDMS_C64_MODEL_SETTINGS:
                case IDMS_C64_PAL:
                case IDMS_C64C_PAL:
                case IDMS_C64_OLD_PAL:
                case IDMS_C64_NTSC:
                case IDMS_C64C_NTSC:
                case IDMS_C64_OLD_NTSC:
                case IDMS_DREAN:
                case IDMS_C64SX_PAL:
                case IDMS_C64SX_NTSC:
                case IDMS_C64_JAP:
                case IDMS_C64_GS:
                case IDMS_PET64_PAL:
                case IDMS_PET64_NTSC:
                case IDMS_ULTIMAX:
                case IDMS_CUSTOM_C64_MODEL:
                    continue;
            }
        }
        machine_specific_menu[j].nm_Type = machine_specific_translation_menu[i].nm_Type;
        machine_specific_menu[j].nm_CommKey = machine_specific_translation_menu[i].nm_CommKey;
        machine_specific_menu[j].nm_Flags = machine_specific_translation_menu[i].nm_Flags;
        machine_specific_menu[j].nm_MutualExclude = machine_specific_translation_menu[i].nm_MutualExclude;
        machine_specific_menu[j].nm_UserData = machine_specific_translation_menu[i].nm_UserData;

        if (machine_specific_translation_menu[i].nm_Label == 0) {
            machine_specific_menu[j++].nm_Label = (STRPTR)NM_BARLABEL;
        } else {
            machine_specific_menu[j++].nm_Label = translate_text(machine_specific_translation_menu[i].nm_Label);
        }
    }
    machine_specific_menu[i].nm_Type = NM_END;
    machine_specific_menu[i].nm_CommKey = NULL;
    machine_specific_menu[i].nm_Flags = 0;
    machine_specific_menu[i].nm_MutualExclude = 0L;
    machine_specific_menu[i].nm_UserData = NULL;
    machine_specific_menu[i].nm_Label = NULL;

    if (!(canvas->os->menu = CreateMenus(machine_specific_menu, GTMN_FrontPen, 0L, TAG_DONE))) {
        return -1;
    }

    LayoutMenus(canvas->os->menu, canvas->os->VisualInfo, GTMN_NewLookMenus, TRUE, TAG_DONE);

    SetMenuStrip(canvas->os->window, canvas->os->menu);

    ui_menu_update(canvas);

    UnlockPubScreen(NULL, pubscreen);

    return 0;
}

void ui_display_paused(int paused)
{
    video_canvas_t *canvas;
    for (canvas = canvaslist; canvas; canvas = canvas->next) {
        struct Window *window = canvas->os->window;

        sprintf(canvas->os->window_title, "%s%s", canvas->os->window_name, (paused) ? " (paused)" : "");
        SetWindowTitles(window, canvas->os->window_title, (void *)-1);
    }
}

static int is_paused = 0;

static void pause_trap(uint16_t addr, void *data)
{
    ui_display_paused(1);
    vsync_suspend_speed_eval();
    while (is_paused) {
        timer_usleep(vice_timer, 1000000 / 100);
        ui_event_handle();
    }
}

void ui_pause_emulation(int flag)
{
    if (network_connected()) {
        return;
    }

    if (flag && !is_paused) {
        is_paused = 1;
        interrupt_maincpu_trigger_trap(pause_trap, 0);
    } else {
        ui_display_paused(0);
        is_paused = 0;
    }
}

int ui_emulation_is_paused(void)
{
    return is_paused;
}

int ui_menu_update(video_canvas_t *canvas)
{
    int i, j;
    int value;
    int result;

    for (i = 0; toggle_list[i].name != NULL; i++) {
        value = 0;
        resources_get_value(toggle_list[i].name, (void *)&value);
        toggle_menu_item(canvas->os->menu, toggle_list[i].idm, value);
    }

    if (machine_specific_toggles) {
        for (i = 0; machine_specific_toggles[i].name != NULL; i++) {
            value = 0;
            resources_get_value(machine_specific_toggles[i].name, (void *)&value);
            toggle_menu_item(canvas->os->menu, machine_specific_toggles[i].idm, value);
        }
    }

    for (i = 0; value_list[i].name != NULL; i++) {
        value = -1;
        result = resources_get_value(value_list[i].name, (void *)&value);
        if (result == 0) {
            unsigned int checked = 0;

            toggle_menu_item(canvas->os->menu, value_list[i].default_idm, 0);
            for (j = 0; value_list[i].vals[j].idm != 0; j++) {
                if (value == value_list[i].vals[j].value && !checked) {
                    toggle_menu_item(canvas->os->menu, value_list[i].vals[j].idm, 1);
                    checked = 1;
                } else {
                    toggle_menu_item(canvas->os->menu, value_list[i].vals[j].idm, 0);
                }
            }
            if (checked == 0 && value_list[i].default_idm > 0) {
                toggle_menu_item(canvas->os->menu, value_list[i].default_idm, 1);
            }
        }
    }

    if (machine_specific_values) {
        for (i = 0; machine_specific_values[i].name != NULL; i++) {
            value = -1;
            result = resources_get_value(machine_specific_values[i].name,
                                         (void *)&value);
            if (result == 0) {
                for (j = 0; machine_specific_values[i].vals[j].idm != 0; j++) {
                    if (value == machine_specific_values[i].vals[j].value) {
                        toggle_menu_item(canvas->os->menu, value_list[i].vals[j].idm, 1);
                    } else {
                        toggle_menu_item(canvas->os->menu, value_list[i].vals[j].idm, 0);
                    }
                }
            }
        }
    }

    toggle_menu_item(canvas->os->menu, IDM_PAUSE, ui_emulation_is_paused());

    ResetMenuStrip(canvas->os->window, canvas->os->menu);

    return 0;
}

static char *get_compiletime_features(void)
{
    feature_list_t *list;
    char *str, *lstr;
    unsigned int len = 0;

    list = vice_get_feature_list();
    while (list->symbol) {
        len += strlen(list->descr) + strlen(list->symbol) + (15);
        ++list;
    }
    str = lib_malloc(len);
    lstr = str;
    list = vice_get_feature_list();
    while (list->symbol) {
        sprintf(lstr, "%4s\t%s (%s)\n", list->isdefined ? "yes " : "no  ", list->descr, list->symbol);
        lstr += strlen(lstr);
        ++list;
    }
    return str;
}

int ui_menu_handle(video_canvas_t *canvas, int idm)
{
    char *fname = NULL;
    char *curlang;

    if (machine_specific_function != NULL) {
        machine_specific_function(canvas, idm);
    }

    switch (idm) {
#ifdef AMIGA_AROS
        case IDM_ATTACH_8:
            uiattach_aros(canvas, 8);
            break;
        case IDM_ATTACH_9:
            uiattach_aros(canvas, 9);
            break;
        case IDM_ATTACH_10:
            uiattach_aros(canvas, 10);
            break;
        case IDM_ATTACH_11:
            uiattach_aros(canvas, 11);
            break;
        case IDM_DETACH_8:
            file_system_detach_disk(8);
            break;
        case IDM_DETACH_9:
            file_system_detach_disk(9);
            break;
        case IDM_DETACH_10:
            file_system_detach_disk(10);
            break;
        case IDM_DETACH_11:
            file_system_detach_disk(11);
            break;
        case IDM_DETACH_ALL:
            file_system_detach_disk(8);
            file_system_detach_disk(9);
            file_system_detach_disk(10);
            file_system_detach_disk(11);
            break;
        case IDM_ATTACH_TAPE:
            uiattach_aros(canvas, 1);
            break;
        case IDM_DETACH_TAPE:
            tape_image_detach(1);
            break;
#else
        case IDM_ATTACH_8:
        case IDM_ATTACH_9:
        case IDM_ATTACH_10:
        case IDM_ATTACH_11:
        case IDM_DETACH_8:
        case IDM_DETACH_9:
        case IDM_DETACH_10:
        case IDM_DETACH_11:
        case IDM_DETACH_ALL:
        case IDM_ATTACH_TAPE:
        case IDM_DETACH_TAPE:
        case IDM_AUTOSTART:
            uiattach_command(canvas, idm);
            break;
#endif
        case IDM_JAM_ACTION:
            ui_jamaction_settings_dialog();
            break;
        case IDM_DRIVE_SOUND:
            ui_drivesound_settings_dialog();
            break;
        case IDM_RESET_HARD:
            machine_trigger_reset(MACHINE_RESET_MODE_HARD);
            break;
        case IDM_RESET_SOFT:
            machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
            break;
        case IDM_RESET_DRIVE8:
            drive_cpu_trigger_reset(0);
            break;
        case IDM_RESET_DRIVE9:
            drive_cpu_trigger_reset(1);
            break;
        case IDM_RESET_DRIVE10:
            drive_cpu_trigger_reset(2);
            break;
        case IDM_RESET_DRIVE11:
            drive_cpu_trigger_reset(3);
            break;
        case IDM_COPY:
            ui_copy_clipboard();
            break;
        case IDM_PASTE:
            ui_paste_clipboard_text();
            break;
        case IDM_SWAP_JOYSTICK:
            ui_joystick_swap_joystick();
            break;
        case IDM_SWAP_USERPORT_JOYSTICK:
            ui_joystick_swap_extra_joystick();
            break;
        case IDM_PAUSE:
            ui_pause_emulation(!ui_emulation_is_paused());
            break;
        case IDM_SINGLE_FRAME_ADVANCE:
            if (ui_emulation_is_paused()) {
                vsyncarch_advance_frame();
            }
            break;
        case IDM_EXIT:
            do_quit_vice = 1;
            break;
        case IDM_FLIP_ADD:
            fliplist_add_image(8);
            break;
        case IDM_FLIP_REMOVE:
            fliplist_remove(8, NULL);
            break;
        case IDM_FLIP_NEXT:
            fliplist_attach_head(8, 1);
            break;
        case IDM_FLIP_PREVIOUS:
            fliplist_attach_head(8, 0);
            break;
        case IDM_FLIP_LOAD:
            uifliplist_load_dialog(canvas);
            break;
        case IDM_FLIP_SAVE:
            uifliplist_save_dialog(canvas);
            break;
        case IDM_SNAPSHOT_LOAD:
            uisnapshot_load_dialog(canvas);
            break;
        case IDM_SNAPSHOT_SAVE:
            uisnapshot_save_dialog(canvas);
            break;
        case IDM_DATASETTE_CONTROL_STOP:
            datasette_control(DATASETTE_CONTROL_STOP);
            break;
        case IDM_DATASETTE_CONTROL_START:
            datasette_control(DATASETTE_CONTROL_START);
            break;
        case IDM_DATASETTE_CONTROL_FORWARD:
            datasette_control(DATASETTE_CONTROL_FORWARD);
            break;
        case IDM_DATASETTE_CONTROL_REWIND:
            datasette_control(DATASETTE_CONTROL_REWIND);
            break;
        case IDM_DATASETTE_CONTROL_RECORD:
            datasette_control(DATASETTE_CONTROL_RECORD);
            break;
        case IDM_DATASETTE_CONTROL_RESET:
            datasette_control(DATASETTE_CONTROL_RESET);
            break;
        case IDM_DATASETTE_RESET_COUNTER:
            datasette_control(DATASETTE_CONTROL_RESET_COUNTER);
            break;
        case IDM_SETTINGS_SAVE_FILE:
            fname = BrowseFile(translate_text(IDS_CONFIG_FILENAME_SELECT), "#?", canvas);
            if (fname != NULL) {
                if (resources_save(fname) < 0) {
                    ui_error(translate_text(IDMES_CANNOT_SAVE_SETTINGS));
                } else {
                    ui_message(translate_text(IDMES_SETTINGS_SAVED_SUCCESS));
                }
            }
            break;
        case IDM_SETTINGS_LOAD_FILE:
            fname = BrowseFile(translate_text(IDS_CONFIG_FILENAME_SELECT), "#?", canvas);
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
        case IDM_MONITOR:
            if (!ui_emulation_is_paused()) {
                monitor_startup_trap();
            }
            break;
        case IDM_ABOUT:
            ui_about();
            break;
        case IDM_CONTRIBUTORS:
            ui_show_text(translate_text(IDMES_VICE_CONTRIBUTORS), translate_text(IDMES_WHO_MADE_WHAT), info_contrib_text);
            break;
        case IDM_LICENSE:
            ui_show_text(translate_text(IDMS_LICENSE), "VICE license (GNU General Public License)", info_license_text);
            break;
        case IDM_WARRANTY:
            ui_show_text(translate_text(IDMS_NO_WARRANTY), translate_text(IDMES_VICE_DIST_NO_WARRANTY), info_warranty_text);
            break;
        case IDM_CMDLINE:
            {
                char *options;

                options = cmdline_options_string();
                ui_show_text(translate_text(IDMS_COMMAND_LINE_OPTIONS), translate_text(IDMES_WHICH_COMMANDS_AVAILABLE), options);
                lib_free(options);
            }
            break;
        case IDM_FEATURES:
            {
                char *features = NULL;

                features = get_compiletime_features();
                ui_show_text(translate_text(IDMS_COMPILE_FEATURES), translate_text(IDMES_WHICH_COMPILE_FEATURES_AVAILABLE), features);
                lib_free(features);
            }
            break;
        case IDM_EVENT_DIRECTORY:
            fname = uilib_select_file(translate_text(IDS_SELECT_START_SNAPSHOT),
                                      UILIB_FILTER_ALL | UILIB_FILTER_SNAPSHOT,
                                      UILIB_SELECTOR_TYPE_FILE_SAVE,
                                      UILIB_SELECTOR_STYLE_EVENT_START);
            lib_free(fname);
            fname = uilib_select_file(translate_text(IDS_SELECT_END_SNAPSHOT),
                                      UILIB_FILTER_ALL | UILIB_FILTER_SNAPSHOT,
                                      UILIB_SELECTOR_TYPE_FILE_SAVE,
                                      UILIB_SELECTOR_STYLE_EVENT_END);
            lib_free(fname);
            break;
        case IDM_EVENT_TOGGLE_RECORD:
            {
                int recording_new = (event_record_active() ? 0 : 1);

                if (recording_new) {
                    event_record_start();
                } else {
                    event_record_stop();
                }
            }
            break;
        case IDM_EVENT_TOGGLE_PLAYBACK:
            {
                int playback_new = (event_playback_active() ? 0 : 1);

                if (playback_new) {
                    event_playback_start();
                } else {
                    event_playback_stop();
                }
            }
            break;
        case IDM_EVENT_SETMILESTONE:
            event_record_set_milestone();
            break;
        case IDM_EVENT_RESETMILESTONE:
            event_record_reset_milestone();
            break;
        case IDM_MEDIAFILE:
            ui_screenshot_dialog(canvas);
            break;
        case IDM_NATIVE_SCREENSHOT_SETTINGS:
            ui_screenshot_settings_dialog();
            break;
        case IDM_RAM_SETTINGS:
            ui_ram_settings_dialog();
            break;
        case IDM_SOUND_SETTINGS:
            ui_sound_settings_dialog();
            break;
#ifdef HAVE_NETWORK
        case IDM_NETWORK_SETTINGS:
            ui_network_dialog();
            break;
#endif
        case IDM_SOUND_RECORD_START:
            ui_sound_record_settings_dialog(canvas);
            break;
        case IDM_AUTOSTART_SETTINGS:
            ui_autostart_settings_dialog(canvas);
            break;
        case IDM_SOUND_RECORD_STOP:
            resources_set_string("SoundRecordDeviceName", "");
            ui_display_statustext(translate_text(IDS_SOUND_RECORDING_STOPPED), 1);
            break;
        case IDM_LANGUAGE_ENGLISH:
            resources_set_value("Language", (resource_value_t *)"en");
            break;
        case IDM_LANGUAGE_DANISH:
            resources_set_value("Language", (resource_value_t *)"da");
            break;
        case IDM_LANGUAGE_GERMAN:
            resources_set_value("Language", (resource_value_t *)"de");
            break;
        case IDM_LANGUAGE_SPANISH:
            resources_set_value("Language", (resource_value_t *)"es");
            break;
        case IDM_LANGUAGE_FRENCH:
            resources_set_value("Language", (resource_value_t *)"fr");
            break;
        case IDM_LANGUAGE_ITALIAN:
            resources_set_value("Language", (resource_value_t *)"it");
            break;
        case IDM_LANGUAGE_KOREAN:
            resources_set_value("Language", (resource_value_t *)"ko");
            break;
        case IDM_LANGUAGE_DUTCH:
            resources_set_value("Language", (resource_value_t *)"nl");
            break;
        case IDM_LANGUAGE_POLISH:
            resources_set_value("Language", (resource_value_t *)"pl");
            break;
        case IDM_LANGUAGE_HUNGARIAN:
            resources_set_value("Language", (resource_value_t *)"hu");
            break;
        case IDM_LANGUAGE_RUSSIAN:
            resources_set_value("Language", (resource_value_t *)"ru");
            break;
        case IDM_LANGUAGE_SWEDISH:
            resources_set_value("Language", (resource_value_t *)"sv");
            break;
        case IDM_LANGUAGE_TURKISH:
            resources_set_value("Language", (resource_value_t *)"tr");
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

                if (machine_specific_toggles) {
                    for (i = 0; machine_specific_toggles[i].name != NULL && !command_found; i++) {
                        if (machine_specific_toggles[i].idm == idm) {
                            resources_toggle(machine_specific_toggles[i].name, NULL);
                            command_found = 1;
                        }
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

                if (machine_specific_values) {
                    for (i = 0; machine_specific_values[i].name != NULL && !command_found; i++) {
                        for (j = 0; machine_specific_values[i].vals[j].idm != 0 && !command_found; j++) {
                            if (machine_specific_values[i].vals[j].idm == idm) {
                                resources_set_value(machine_specific_values[i].name, (resource_value_t)machine_specific_values[i].vals[j].value);
                                command_found = 1;
                            }
                        }
                    }
                }
            }
            break;
    }

    return 0;
}

void ui_event_handle(void)
{
    int done = 1;

    if (ui_event_handling) {
        return ui_event_handling();
    }

    do {
        video_canvas_t *canvas;
        for (canvas = canvaslist; canvas; canvas = canvas->next) {
            unsigned long imCode, imClass;
            struct Window *window;
            struct IntuiMessage *imsg;
            int mousex, mousey;
            char *curlang;
            char oldlang[4];

            window = canvas->os->window;

            /* FIXME: bad hack */
            if (canvas->os->menu == NULL) {
                if (ui_menu_create(canvas) == -1) {
                    exit(-1);
                }
            }

            /* Check for IDCMP messages */
            while ((imsg = (struct IntuiMessage *)GetMsg(canvas->os->window->UserPort))) {
                imClass = imsg->Class;
                imCode = imsg->Code;
                mousex = imsg->MouseX;
                mousey = imsg->MouseY;

                switch (imClass) {
                    case IDCMP_MENUPICK:
                        pointer_to_default();
                        resources_get_value("Language", (void *)&curlang);
                        strncpy(oldlang, curlang, sizeof(oldlang));
                        while (imCode != MENUNULL) {
                            struct MenuItem *n = ItemAddress(canvas->os->menu, imCode);

                            ui_menu_handle(canvas, (int)GTMENUITEM_USERDATA(n));
                            imCode = n->NextSelect;
                        }
                        resources_get_value("Language", (void *)&curlang);
                        if (strcasecmp(curlang, oldlang)) {
                            ui_menu_destroy(canvas);
                            if (ui_menu_create(canvas) == -1) {
                                exit(-1);
                            }
                        }
                        ui_menu_update(canvas);
                        done = 1;
                        break;
                    case IDCMP_SIZEVERIFY:
                        canvas->waiting_for_resize = 1;
                        break;
                    default:
                        break;
                }

                ReplyMsg((struct Message *)imsg);

                switch (imClass) {
                    case IDCMP_CLOSEWINDOW:
                        do_quit_vice = 1;
                        break;
                    case IDCMP_RAWKEY:
                        if (!ui_emulation_is_paused()) {
                            if (imCode & IECODE_UP_PREFIX) {
                                /* key is released */
                                imCode &= 0x7f;
                                if (!joystick_handle_key(imCode, 0)) {
                                    keyboard_key_released((signed long)imCode);
                                }
                            } else {
                                /* key is pressed */
                                imCode &= 0x7f;
                                if (!joystick_handle_key(imCode, 1)) {
                                    keyboard_key_pressed((signed long)imCode);
                                }
                            }
                        }
                        break;
                    case IDCMP_CHANGEWINDOW:
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
                        if (canvas->os->vlayer_handle) {
                            canvas->waiting_for_resize = 0;
                            if ((LONG)canvas->os->vlayer_colorkey != -1) {
                                FillPixelArray(window->RPort, window->BorderLeft, window->BorderTop,
                                               window->Width - window->BorderLeft- window->BorderRight,
                                               window->Height - window->BorderTop - window->BorderBottom, /* - statusheight,*/
                                               canvas->os->vlayer_colorkey);
                            }
                            statusbar_refresh(REFRESH_ALL);
                            break;
                        }
#endif
                        if (canvas->waiting_for_resize) {
                            struct Window *window = canvas->os->window;

                            canvas->waiting_for_resize = 0;
                            BltBitMapRastPort(canvas->os->window_bitmap, 0, 0, window->RPort, window->BorderLeft, window->BorderTop, canvas->width, canvas->height, 0xc0);
                            statusbar_refresh(REFRESH_ALL);
                        }
                        break;
                    case IDCMP_MENUVERIFY:
                        pointer_show();
                        switch (imCode) {
                            case MENUHOT:
                                /* Menu action can be cancelled by msg->Code = MENUCANCEL; */
                                break;
                            default:
                                break;
                        }
                        vsync_suspend_speed_eval();
                        done = 0;
                        break;
                    default:
                        break;
                }
            }
        }
    } while (!done);

    video_arch_fullscreen_update();

    if (do_quit_vice) {
        int confirm_on_exit, save_on_exit;

        resources_get_value("ConfirmOnExit", &confirm_on_exit);
        resources_get_value("SaveResourcesOnExit", &save_on_exit);

        if (confirm_on_exit) {
            do_quit_vice = ui_requester(translate_text(IDS_VICE_QUESTION), translate_text(IDS_REALLY_EXIT), translate_text(IDS_YES_NO), 1);
        }

        if (do_quit_vice) {
            if (save_on_exit) {
                if (resources_save(NULL) < 0) {
                    ui_error(translate_text(IDMES_CANNOT_SAVE_SETTINGS));
                } else {
                    ui_message(translate_text(IDMES_SETTINGS_SAVED_SUCCESS));
                }
            }

            exit(0); // I think it's safe to quit here
        }
    }
}

void ui_dispatch_next_event(void)
{
}

void ui_dispatch_events(void)
{
    ui_event_handle();
}

int ui_menu_destroy(video_canvas_t *canvas)
{
    if (canvas->os->menu) {
      ClearMenuStrip(canvas->os->window);
      FreeMenus(canvas->os->menu);
      canvas->os->menu = NULL;
    }

    if (canvas->os->VisualInfo) {
      FreeVisualInfo(canvas->os->VisualInfo);
      canvas->os->VisualInfo = NULL;
    }

    return 0;
}
