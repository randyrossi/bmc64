/*
 * ui.cc - BeOS user interface.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *  Marcus Sutton <loggedoubt@gmail.com>
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

#include <Alert.h>
#include <Application.h>
#include <Clipboard.h>
#include <FilePanel.h>
#include <Path.h>
#include <ScrollView.h>
#include <TextView.h>
#include <View.h>
#include <Window.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/utsname.h>

#include "statusbar.h"
#include "ui_file.h"
#include "vicewindow.h"

extern "C" {
#include "attach.h"
#include "autostart.h"
#include "autostart-prg.h"
#include "archdep.h"
#include "charset.h"
#include "cmdline.h"
#include "clipboard.h"
#include "constants.h"
#include "datasette.h"
#include "drive.h"
#include "fliplist.h"
#include "gfxoutput.h"
#include "imagecontents.h"
#include "info.h"
#include "interrupt.h"
#include "kbd.h"
#include "kbdbuf.h"
#include "keyboard.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "main.h"
#include "maincpu.h"
#include "mem.h"
#include "monitor.h"
#include "mos6510.h"
#include "mouse.h"
#include "network.h"
#include "printer.h"
#include "resources.h"
#include "sampler.h"
#include "sound.h"
#include "tape.h"
#include "translate.h"
#include "types.h"
#include "ui.h"
#include "ui_autostart.h"
#include "ui_datasette.h"
#include "ui_device.h"
#include "ui_keymap.h"
#include "ui_netplay.h"
#include "ui_ram.h"
#include "ui_sound.h"
#include "userport_joystick.h"
#include "util.h"
#include "version.h"
#include "vice-event.h"
#include "viceapp.h"
#include "vicefeatures.h"
#include "videoarch.h"
#include "vsync.h"
#include "vsyncapi.h"

#ifdef USE_SVN_REVISION
#include "svnversion.h"
#endif
}

/* sometimes we may need pointers to the ViceWindows */
#define MAX_WINDOWS 2
ViceWindow *windowlist[MAX_WINDOWS];
int window_count = 0;

/* This check is needed for haiku, since it always returns 1 on
   SupportsWindowMode() */
int CheckForHaiku(void)
{
    struct utsname name;

    uname(&name);
    if (!strncasecmp(name.sysname, "Haiku", 5)) {
        return -1;
    }
    return 0;
}

/* List of resources that can be switched on and off from the menus.  */
ui_menu_toggle  toggle_list[] = {
    { "Sound", MENU_TOGGLE_SOUND },
    { "SaveResourcesOnExit", MENU_TOGGLE_SAVE_SETTINGS_ON_EXIT },
    { "ConfirmOnExit", MENU_TOGGLE_CONFIRM_ON_EXIT },
    { NULL, 0 }
};

ui_menu_toggle  non_vsid_toggles[] = {
    { "DriveTrueEmulation", MENU_TOGGLE_DRIVE_TRUE_EMULATION },
    { "DriveSoundEmulation", MENU_TOGGLE_DRIVE_SOUND_EMULATION },
    { "DirectWindow", MENU_TOGGLE_DIRECTWINDOW },
    { "WarpMode", MENU_TOGGLE_WARP_MODE },
    { "VirtualDevices", MENU_TOGGLE_VIRTUAL_DEVICES },
    { "IECDevice4", MENU_PRINTER_4_IEC },
    { "AutostartWarp", MENU_AUTOSTART_WARP },
    { "AutostartRunWithColon", MENU_USE_COLON_WITH_RUN },
    { "AutostartBasicLoad", MENU_LOAD_TO_BASIC_START },
    { "AutostartDelayRandom", MENU_AUTOSTART_DELAY_RANDOM },
    { "AutostartHandleTrueDriveEmulation", MENU_TOGGLE_HANDLE_TDE_AUTOSTART },
    { "UserportJoy", MENU_TOGGLE_USERPORT_JOY },
    { "JoyOpposite", MENU_ALLOW_OPPOSITE_JOY },
    { "TapeSenseDongle", MENU_TOGGLE_TAPEPORT_TAPE_SENSE_DONGLE },
    { "DTLBasicDongle", MENU_TOGGLE_TAPEPORT_DTL_BASIC_DONGLE },
    { "CPClockF83", MENU_TOGGLE_TAPEPORT_CP_CLOCK_F83 },
    { "CPClockF83Save", MENU_TOGGLE_TAPEPORT_CP_CLOCK_F83_SAVE },
    { "TapeLog", MENU_TOGGLE_TAPEPORT_TAPELOG },
    { "BBRTCSave", MENU_TOGGLE_BBRTC_DATA_SAVE },
    { NULL, 0 }
};

/*  List of resources which can have multiple mutual exclusive menu entries. */
ui_res_possible_values RefreshRateValues[] = {
    { 0, MENU_REFRESH_RATE_AUTO },
    { 1, MENU_REFRESH_RATE_1 },
    { 2, MENU_REFRESH_RATE_2 },
    { 3, MENU_REFRESH_RATE_3 },
    { 4, MENU_REFRESH_RATE_4 },
    { 5, MENU_REFRESH_RATE_5 },
    { 6, MENU_REFRESH_RATE_6 },
    { 7, MENU_REFRESH_RATE_7 },
    { 8, MENU_REFRESH_RATE_8 },
    { 9, MENU_REFRESH_RATE_9 },
    { 10, MENU_REFRESH_RATE_10 },
    { -1, 0 }
};

ui_res_possible_values SpeedValues[] = {
    { 0, MENU_MAXIMUM_SPEED_NO_LIMIT },
    { 10, MENU_MAXIMUM_SPEED_10 },
    { 20, MENU_MAXIMUM_SPEED_20 },
    { 50, MENU_MAXIMUM_SPEED_50 },
    { 100, MENU_MAXIMUM_SPEED_100 },
    { 200, MENU_MAXIMUM_SPEED_200 },
    { -1, 0 }
};

ui_res_possible_values RecordingOptions[] = {
    { EVENT_START_MODE_FILE_SAVE, MENU_EVENT_START_MODE_SAVE },
    { EVENT_START_MODE_FILE_LOAD, MENU_EVENT_START_MODE_LOAD },
    { EVENT_START_MODE_RESET, MENU_EVENT_START_MODE_RESET },
    { EVENT_START_MODE_PLAYBACK, MENU_EVENT_START_MODE_PLAYBACK },
    { -1, 0 }
};

ui_res_possible_values SyncFactor[] = {
    { MACHINE_SYNC_PAL, MENU_SYNC_FACTOR_PAL },
    { MACHINE_SYNC_NTSC, MENU_SYNC_FACTOR_NTSC },
    { MACHINE_SYNC_NTSCOLD, MENU_SYNC_FACTOR_NTSCOLD },
    { MACHINE_SYNC_PALN, MENU_SYNC_FACTOR_PALN },
    { -1, 0 }
};

ui_res_possible_values UserportJoyType[] = {
    { USERPORT_JOYSTICK_CGA, MENU_USERPORT_JOY_CGA },
    { USERPORT_JOYSTICK_PET, MENU_USERPORT_JOY_PET },
    { USERPORT_JOYSTICK_HUMMER, MENU_USERPORT_JOY_HUMMER },
    { USERPORT_JOYSTICK_OEM, MENU_USERPORT_JOY_OEM },
    { USERPORT_JOYSTICK_HIT, MENU_USERPORT_JOY_HIT },
    { USERPORT_JOYSTICK_KINGSOFT, MENU_USERPORT_JOY_KINGSOFT },
    { USERPORT_JOYSTICK_STARBYTE, MENU_USERPORT_JOY_STARBYTE },
    { -1, 0 }
};

ui_res_possible_values AutostartPrgMode[] = {
    { AUTOSTART_PRG_MODE_VFS, MENU_AUTOSTART_PRG_VIRTUAL_FS },
    { AUTOSTART_PRG_MODE_INJECT, MENU_AUTOSTART_PRG_INJECT },
    { AUTOSTART_PRG_MODE_DISK, MENU_AUTOSTART_PRG_DISK_IMAGE },
    { -1, 0 }
};

ui_res_possible_values Printer4Emulation[] = {
    { PRINTER_DEVICE_NONE, MENU_PRINTER_4_EMULATION_NONE },
    { PRINTER_DEVICE_FS, MENU_PRINTER_4_EMULATION_FS },
    { -1, 0 }
};

ui_res_possible_values SamplerEmulation[] = {
    { SAMPLER_DEVICE_FILE, MENU_SAMPLER_DEVICE_MEDIA_FILE },
    { SAMPLER_DEVICE_PORTAUDIO, MENU_SAMPLER_DEVICE_PORTAUDIO },
    { -1, 0 }
};

ui_res_possible_values SamplerGain[] = {
    { 10, MENU_SAMPLER_GAIN_10 },
    { 25, MENU_SAMPLER_GAIN_25 },
    { 50, MENU_SAMPLER_GAIN_50 },
    { 75, MENU_SAMPLER_GAIN_75 },
    { 100, MENU_SAMPLER_GAIN_100 },
    { 110, MENU_SAMPLER_GAIN_110 },
    { 125, MENU_SAMPLER_GAIN_125 },
    { 150, MENU_SAMPLER_GAIN_150 },
    { 175, MENU_SAMPLER_GAIN_175 },
    { 200, MENU_SAMPLER_GAIN_200 },
    { -1, 0 }
};

ui_res_possible_strings Printer4Driver[] = {
    { "ascii", MENU_PRINTER_4_DRIVER_ASCII },
    { "mps803", MENU_PRINTER_4_DRIVER_MPS803 },
    { "nl10", MENU_PRINTER_4_DRIVER_NL10 },
    { "raw", MENU_PRINTER_4_DRIVER_RAW },
    { NULL, 0 }
};

ui_res_possible_strings Printer4OutputType[] = {
    { "text", MENU_PRINTER_4_TYPE_TEXT },
    { "graphics", MENU_PRINTER_4_TYPE_GFX },
    { NULL, 0 }
};

ui_res_possible_values Printer4OutputDevice[] = {
    { 0, MENU_PRINTER_4_DEV_1 },
    { 1, MENU_PRINTER_4_DEV_2 },
    { 2, MENU_PRINTER_4_DEV_3 },
    { -1, 0 }
};

ui_res_possible_values CpuJamActions[] = {
    { MACHINE_JAM_ACTION_DIALOG, MENU_JAM_ACTION_ASK },
    { MACHINE_JAM_ACTION_CONTINUE, MENU_JAM_ACTION_CONTINUE },
    { MACHINE_JAM_ACTION_MONITOR, MENU_JAM_ACTION_START_MONITOR },
    { MACHINE_JAM_ACTION_RESET, MENU_JAM_ACTION_RESET },
    { MACHINE_JAM_ACTION_HARD_RESET, MENU_JAM_ACTION_HARD_RESET },
    { MACHINE_JAM_ACTION_QUIT, MENU_JAM_ACTION_QUIT_EMULATOR },
    { -1, 0 }
};

ui_res_possible_values TapeLogDestinations[] = {
    { 0, MENU_TAPEPORT_TAPELOG_DEFAULT_LOGFILE },
    { 1, MENU_TAPEPORT_TAPELOG_USER_LOGFILE },
    { -1, 0 }
};

ui_res_possible_values DoodleOversize[] = {
    { NATIVE_SS_OVERSIZE_SCALE, MENU_SCREENSHOT_DOODLE_OVERSIZE_SCALE },
    { NATIVE_SS_OVERSIZE_CROP_LEFT_TOP, MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_LEFT_TOP },
    { NATIVE_SS_OVERSIZE_CROP_CENTER_TOP, MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_MIDDLE_TOP },
    { NATIVE_SS_OVERSIZE_CROP_RIGHT_TOP, MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_RIGHT_TOP },
    { NATIVE_SS_OVERSIZE_CROP_LEFT_CENTER, MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_LEFT_CENTER },
    { NATIVE_SS_OVERSIZE_CROP_CENTER, MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_MIDDLE_CENTER },
    { NATIVE_SS_OVERSIZE_CROP_RIGHT_CENTER, MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_RIGHT_CENTER },
    { NATIVE_SS_OVERSIZE_CROP_LEFT_BOTTOM, MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_LEFT_BOTTOM },
    { NATIVE_SS_OVERSIZE_CROP_CENTER_BOTTOM, MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_MIDDLE_BOTTOM },
    { NATIVE_SS_OVERSIZE_CROP_RIGHT_BOTTOM, MENU_SCREENSHOT_DOODLE_OVERSIZE_CROP_RIGHT_BOTTOM },
    { -1, 0 }
};

ui_res_possible_values DoodleUndersize[] = {
    { NATIVE_SS_UNDERSIZE_SCALE, MENU_SCREENSHOT_DOODLE_UNDERSIZE_SCALE },
    { NATIVE_SS_UNDERSIZE_BORDERIZE, MENU_SCREENSHOT_DOODLE_UNDERSIZE_BORDERIZE },
    { -1, 0 }
};

ui_res_possible_values KoalaOversize[] = {
    { NATIVE_SS_OVERSIZE_SCALE, MENU_SCREENSHOT_KOALA_OVERSIZE_SCALE },
    { NATIVE_SS_OVERSIZE_CROP_LEFT_TOP, MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_LEFT_TOP },
    { NATIVE_SS_OVERSIZE_CROP_CENTER_TOP, MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_MIDDLE_TOP },
    { NATIVE_SS_OVERSIZE_CROP_RIGHT_TOP, MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_RIGHT_TOP },
    { NATIVE_SS_OVERSIZE_CROP_LEFT_CENTER, MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_LEFT_CENTER },
    { NATIVE_SS_OVERSIZE_CROP_CENTER, MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_MIDDLE_CENTER },
    { NATIVE_SS_OVERSIZE_CROP_RIGHT_CENTER, MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_RIGHT_CENTER },
    { NATIVE_SS_OVERSIZE_CROP_LEFT_BOTTOM, MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_LEFT_BOTTOM },
    { NATIVE_SS_OVERSIZE_CROP_CENTER_BOTTOM, MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_MIDDLE_BOTTOM },
    { NATIVE_SS_OVERSIZE_CROP_RIGHT_BOTTOM, MENU_SCREENSHOT_KOALA_OVERSIZE_CROP_RIGHT_BOTTOM },
    { -1, 0 }
};

ui_res_possible_values KoalaUndersize[] = {
    { NATIVE_SS_UNDERSIZE_SCALE, MENU_SCREENSHOT_KOALA_UNDERSIZE_SCALE },
    { NATIVE_SS_UNDERSIZE_BORDERIZE, MENU_SCREENSHOT_KOALA_UNDERSIZE_BORDERIZE },
    { -1, 0 }
};

ui_res_value_list value_list[] = {
    { "RefreshRate", RefreshRateValues },
    { "Speed", SpeedValues },
    { "MachineVideoStandard", SyncFactor },
    { "JAMAction", CpuJamActions },
    { NULL, NULL }
};

ui_res_value_list non_vsid_values[] = {
    { "EventStartMode", RecordingOptions },
    { "UserportJoyType", UserportJoyType },
    { "AutostartPrgMode", AutostartPrgMode },
    { "Printer4", Printer4Emulation },
    { "Printer4TextDevice", Printer4OutputDevice },
    { "TapeLogDestination", TapeLogDestinations },
    { "SamplerDevice", SamplerEmulation },
    { "SamplerGain", SamplerGain },
    { "DoodleOversizeHandling", DoodleOversize },
    { "DoodleUndersizeHandling", DoodleUndersize },
    { "KoalaOversizeHandling", KoalaOversize },
    { "KoalaUndersizeHandling", KoalaUndersize },
    { NULL, NULL }
};

ui_res_string_list non_vsid_strings[] = {
    { "Printer4Driver", Printer4Driver },
    { "Printer4Output", Printer4OutputType },
    { NULL, NULL }
};

ui_menu_toggle *machine_specific_toggles = NULL;
ui_res_value_list *machine_specific_values = NULL;

void ui_register_menu_toggles(ui_menu_toggle *toggles)
{
    machine_specific_toggles = toggles;
}

void ui_register_res_values(ui_res_value_list *valuelist)
{
    machine_specific_values = valuelist;
}

ui_machine_specific_t ui_machine_specific = NULL;

void ui_register_machine_specific(ui_machine_specific_t func)
{
    ui_machine_specific = func;
}

/* ------------------------------------------------------------------------ */
/* UI-related resources.  */

static int joystickdisplay;
static int save_resources_on_exit;
static int confirm_on_exit;

static int set_joystickdisplay(int val, void *param)
{
    joystickdisplay = val ? 1 : 0;

    ui_enable_joyport();

    return 0;
}

static int set_save_resources_on_exit(int val, void *param)
{
    save_resources_on_exit = val ? 1 : 0;

    return 0;
}

static int set_confirm_on_exit(int val, void *param)
{
    confirm_on_exit = val ? 1 : 0;

    return 0;
}

static const resource_int_t resources_int[] = {
    { "JoystickDisplay", 0, RES_EVENT_NO, NULL,
      &joystickdisplay, set_joystickdisplay, NULL },
    { "SaveResourcesOnExit", 0, RES_EVENT_NO, NULL,
      &save_resources_on_exit, set_save_resources_on_exit, NULL },
    { "ConfirmOnExit", 1, RES_EVENT_NO, NULL,
      &confirm_on_exit, set_confirm_on_exit, NULL },
    RESOURCE_INT_LIST_END
};

int ui_resources_init(void)
{
    return resources_register_int(resources_int);
}

void ui_resources_shutdown(void)
{
}

/* ------------------------------------------------------------------------ */

/* UI-related command-line options.  */
static const cmdline_option_t cmdline_options[] = {
    { "-joydisplay", SET_RESOURCE, 0,
      NULL, NULL, "JoystickDisplay", (resource_value_t) 1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Enable joystick display" },
    { "+joydisplay", SET_RESOURCE, 0,
      NULL, NULL, "JoystickDisplay", (resource_value_t) 0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Disable joystick display" },
    { "-saveres", SET_RESOURCE, 0,
      NULL, NULL, "SaveResourcesOnExit", (resource_value_t) 1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Save resources on exit" },
    { "+saveres", SET_RESOURCE, 0,
      NULL, NULL, "SaveResourcesOnExit", (resource_value_t) 0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Do not save resources on exit" },
    { "-confirmexit", SET_RESOURCE, 0,
      NULL, NULL, "ConfirmOnExit", (resource_value_t) 1,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Confirm exiting the emulator" },
    { "+confirmexit", SET_RESOURCE, 0,
      NULL, NULL, "ConfirmOnExit", (resource_value_t) 0,
      USE_PARAM_STRING, USE_DESCRIPTION_STRING,
      IDCLS_UNUSED, IDCLS_UNUSED,
      NULL, "Do not confirm exiting the emulator" },
    CMDLINE_LIST_END
};

int ui_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}


/* Exit.  */
void ui_exit(void)
{
}

static void ui_exit_early(void)
{
    /* Disable SIGINT.  This is done to prevent the user from keeping C-c
       pressed and thus breaking the cleanup process, which might be
       dangerous.  */
    signal(SIGINT, SIG_IGN);
    log_message(LOG_DEFAULT, "\nExiting...");
    machine_shutdown();
    putchar ('\n');
}

/* ------------------------------------------------------------------------ */

typedef struct {
    char name[256];
    int valid;
} snapfiles;

static snapfiles files[10];
static int lastindex;
static int snapcounter;

static void save_quicksnapshot_trap(uint16_t unused_addr, void 
*unused_data)
{
    int i, j;
    char *fullname;
    char *fullname2;

    if (lastindex == -1) {
        lastindex = 0;
        strcpy(files[lastindex].name, "quicksnap0.vsf");
    } else {
        if (lastindex == 9) {
            if (snapcounter == 10) {
                fullname = util_concat(archdep_boot_path(), "/", machine_name, "/", files[0].name, NULL);
                remove(fullname);
                free(fullname);
                for (i = 1; i < 10; i++) {
                    fullname = util_concat(archdep_boot_path(), "/", machine_name, "/", files[i].name, NULL);
                    fullname2 = util_concat(archdep_boot_path(), "/", machine_name, "/", files[i - 1].name, NULL);
                    rename(fullname, fullname2);
                    free(fullname);
                    free(fullname2);
                }
            } else {
                for (i = 0; i < 10; i++) {
                    if (files[i].valid == 0) {
                        break;
                    }
                }
                for (j = i + 1; j < 10; j++) {
                    if (files[j].valid) {
                        strcpy(files[i].name,files[j].name);
                        files[i].name[strlen(files[i].name) - 5] = '0' + i;
                        fullname = util_concat(archdep_boot_path(), "/", machine_name, "/", files[j].name, NULL);
                        fullname2 = util_concat(archdep_boot_path(), "/", machine_name, "/", files[i].name, NULL);
                        rename(fullname, fullname2);
                        free(fullname);
                        free(fullname2);
                        i++;
                    }
                }
                strcpy(files[i].name, files[0].name);
                files[i].name[strlen(files[i].name) - 5] = '0' + i;
                lastindex = i;
            }
        } else {
            strcpy(files[lastindex + 1].name, files[lastindex].name);
            lastindex++;
            files[lastindex].name[strlen(files[lastindex].name) - 5] = '0' + lastindex;
        }
    }

    fullname = util_concat(archdep_boot_path(), "/", machine_name, "/", files[lastindex].name, NULL);
    if (machine_write_snapshot(fullname, 0, 0, 0) < 0) {
        snapshot_display_error();
    }
    free(fullname);
}

static void load_quicksnapshot_trap(uint16_t unused_addr, void *unused_data)
{
    char *fullname;

    fullname = util_concat(archdep_boot_path(), "/", machine_name, "/", files[lastindex].name, NULL);
    if (machine_read_snapshot(fullname, 0) < 0) {
        snapshot_display_error();
    }
    free(fullname);
}

static void scan_files(void)
{
    int i;
    char *filename;
    char *fullname;
    BEntry entry;
        
    snapcounter = 0;
    lastindex = -1;
    for (i = 0; i < 10; i++) {
        files[i].valid = 0;
          filename = lib_stralloc("quicksnap?.vsf");
        filename[strlen(filename) - 5] = '0' + i;
        fullname = util_concat(archdep_boot_path(), "/", machine_name, "/", filename, NULL);
        entry.SetTo(fullname);
        if (entry.Exists()) {
              strcpy(files[i].name, filename);
            files[i].valid = 1;
            if (i > lastindex) {
                lastindex = i;
            }
            snapcounter++;
        }
        free(filename);
        free(fullname);
    }
}


/* ------------------------------------------------------------------------- */
static int is_paused = 0;

/* Toggle displaying of paused state.  */
void ui_display_paused(int flag)
{
    /* use ui_display_speed() and warp flag to encode pause mode */ 
    if (flag) {
        ui_display_speed(0, 0, -1);
    }   else {
        ui_display_speed(0, 0, -2);
    }
}

static void pause_trap(uint16_t addr, void *data)
{
    ui_display_paused(1);
    vsync_suspend_speed_eval();
    while (is_paused) {
        snooze(1000);
        ui_dispatch_events();
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

static void ui_copy_clipboard(void)
{
    BMessage *clippy = (BMessage *)NULL;
    char *text = NULL;

    text = clipboard_read_screen_output("\n");

    if (text != NULL) {
        if (be_clipboard->Lock()) {
            be_clipboard->Clear();
            if (clippy = be_clipboard->Data()) {
                clippy->AddData("text/plain", B_MIME_TYPE, text, strlen(text));
                be_clipboard->Commit();
            }
            be_clipboard->Unlock();
        }
        lib_free(text);
    }
}
static void ui_handle_paste_text(BMessage *msg)
{
    const char *text;
    char *text_in_petscii = NULL;
    ssize_t textlen = 0;

    if ((msg->FindData("text/plain", B_MIME_TYPE, (const void **)&text, &textlen)) != B_OK) {
        // ui_error("No text pasted ?!");
        return;
    }

    if (textlen != 0) {
        text_in_petscii = (char *)lib_malloc(textlen + 1);
        memcpy(text_in_petscii, text, textlen);
        text_in_petscii[textlen] = 0;
        charset_petconvstring((unsigned char *)text_in_petscii, 0);
        kbdbuf_feed(text_in_petscii);
        lib_free(text_in_petscii);
    }
} 

static void ui_paste_clipboard_text(void)
{
    BMessage *clippy = (BMessage *)NULL;

    if (be_clipboard->Lock()) {
        if (clippy = be_clipboard->Data()) {
            ui_handle_paste_text(clippy);
        }
        be_clipboard->Unlock();
    }
} 

static void ui_handle_dropped_file(BMessage *msg)
{
    entry_ref ref;
    status_t err;
    BPath *path;
    // int32 buttons;

    if ((err = msg->FindRef("refs", 0, &ref)) != B_OK) {
        // ui_error("No File selected ?!");
        return;
    }
    // err = msg->FindInt32("buttons", &buttons);
    path = new BPath(&ref);

    if (autostart_autodetect(path->Path(), NULL, 0, AUTOSTART_MODE_RUN) < 0) {
        ui_error("Cannot autostart specified file.");
    }
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
    str = (char *)lib_malloc(len);
    lstr = str;
    list = vice_get_feature_list();
    while (list->symbol) {
        sprintf(lstr, "%4s\t%s (%s)\n", list->isdefined ? "yes " : "no  ", list->descr, list->symbol);
        lstr += strlen(lstr);
        ++list;
    }
    return str;
}

/* here the stuff for queueing and dispatching ui commands */
/*---------------------------------------------------------*/
#define MAX_MESSAGE_QUEUE_SIZE  256
static BMessage message_queue[MAX_MESSAGE_QUEUE_SIZE];
static int num_queued_messages;

void ui_add_event(void *msg)
{
    if (num_queued_messages < MAX_MESSAGE_QUEUE_SIZE) {
        message_queue[num_queued_messages++] = *(BMessage*)msg;
    }
}

int ui_handle_toggle_items(ui_menu_toggle *toggles, int msg_item_id)
{
    int m;
    
    for (m = 0; toggles[m].name != NULL; m++) {
        if (toggles[m].item_id == msg_item_id) {
            resources_toggle(toggles[m].name, NULL);
            ui_update_menus();
            return 1;
        }
    }

    return 0;
}

int ui_handle_value_items(ui_res_value_list *values, int msg_item_id)
{
    int m;
    
    for (m = 0; values[m].name != NULL; m++) {
        ui_res_possible_values *vl = values[m].vals;

        for (; vl->item_id > 0; vl++) {
            if (vl->item_id == msg_item_id) {
                resources_set_int(values[m].name, vl->value);
                ui_update_menus();
                return 1;
            }
        }
    }

    return 0;
}

int ui_handle_string_items(ui_res_string_list *string_list, int msg_item_id)
{
    int m;
    
    for (m = 0; string_list[m].name != NULL; m++) {
        ui_res_possible_strings *vl = string_list[m].strings;

        for (; vl->item_id > 0; vl++) {
            if (vl->item_id == msg_item_id) {
                resources_set_string(string_list[m].name, vl->string);
                ui_update_menus();
                return 1;
            }
        }
    }

    return 0;
}

/* This is called when turning datasette on/off from the menu. */
static void ui_draw_tape_status();

void ui_dispatch_events(void)
{
    int i;
    int attachdrive;
    int key;

    for (i = 0; i < num_queued_messages; i++) {

        /* Handle machine specific commands first. */
        if (ui_machine_specific) {
            ui_machine_specific(&message_queue[i], windowlist[0]);
        }

        switch (message_queue[i].what) {
            case B_KEY_DOWN:
            case B_UNMAPPED_KEY_DOWN:
                message_queue[i].FindInt32("key", (int32*)&key);
                if (machine_class != VICE_MACHINE_VSID) {
                    kbd_handle_keydown(key);
                }
                break;
            case B_KEY_UP:
            case B_UNMAPPED_KEY_UP:
                message_queue[i].FindInt32("key", (int32*)&key);
                if (machine_class != VICE_MACHINE_VSID) {
                    kbd_handle_keyup(key);
                }
                break;
            case B_SAVE_REQUESTED:
            case B_REFS_RECEIVED:
                /* the file- or save-panel was closed */
                /* now we can use the selected file */
                ui_select_file_action(&message_queue[i]);
                break;
            case B_SIMPLE_DATA:
                /* handle a file being dropped on the */
                /* window from tracker */
                ui_handle_dropped_file(&message_queue[i]);
                break;
            case B_MIME_DATA:
                /* handle text being dropped on the window */
                ui_handle_paste_text(&message_queue[i]);
                break;
           case MENU_EXIT_REQUESTED:
                {
                    int32 button = 0;
                    BAlert *alert;

                    if (confirm_on_exit) {
                        alert = new BAlert("Quit BeVICE", "Do you really want to exit BeVICE??", "Yes", "No", NULL, B_WIDTH_AS_USUAL, B_INFO_ALERT);
                        vsync_suspend_speed_eval();
                        button = alert->Go();
                    }
                    if (button == 0) {
                        if (save_resources_on_exit) {
                            if (resources_save(NULL) < 0) {
                                ui_error("Cannot save settings.");
                            }
                        }

                        /* send message to quit the application */
                        /* and exit the emulation thread        */
                        ui_exit_early();
                        BMessenger messenger(APP_SIGNATURE);
                        BMessage message(WINDOW_CLOSED);
                        messenger.SendMessage(&message, be_app);
                        exit_thread(0);
                    }
                    break;
                }
            case MENU_RESET_SOFT:
                machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
                break;
            case MENU_RESET_HARD:
                machine_trigger_reset(MACHINE_RESET_MODE_HARD);
                break;
            case MENU_AUTOSTART_DELAY:
                ui_autostart();
                break;
            case MENU_AUTOSTART_PRG_DISK_IMAGE_SELECT:
                ui_select_file(B_OPEN_PANEL, AUTOSTART_DISK_IMAGE_FILE, (void*)0);
                break;
            case MENU_AUTOSTART:
                ui_select_file(B_OPEN_PANEL, AUTOSTART_FILE, (void*)0);
                break;
            case MENU_ATTACH_DISK8:
                attachdrive = 8;
                ui_select_file(B_OPEN_PANEL, DISK_FILE, (void*)&attachdrive);
                break;
            case MENU_ATTACH_DISK9:
                attachdrive = 9;
                ui_select_file(B_OPEN_PANEL, DISK_FILE, (void*)&attachdrive);
                break;
            case MENU_ATTACH_DISK10:
                attachdrive = 10;
                ui_select_file(B_OPEN_PANEL, DISK_FILE, (void*)&attachdrive);
                break;
            case MENU_ATTACH_DISK11:
                attachdrive = 11;
                ui_select_file(B_OPEN_PANEL, DISK_FILE, (void*)&attachdrive);
                break;
            case MENU_DETACH_DISK8:
                file_system_detach_disk(8);
                break;
            case MENU_DETACH_DISK9:
                file_system_detach_disk(9);
                break;
            case MENU_DETACH_DISK10:
                file_system_detach_disk(10);
                break;
            case MENU_DETACH_DISK11:
                file_system_detach_disk(11);
                break;
            case MENU_ATTACH_TAPE:
                attachdrive = 1;
                ui_select_file(B_OPEN_PANEL, TAPE_FILE, (void*)1);
                break;
            case MENU_DETACH_TAPE:
                tape_image_detach(1);
                break;
            case MENU_FLIP_ADD:
                fliplist_add_image(8);
                break;
            case MENU_FLIP_REMOVE:
                fliplist_remove(8, NULL);
                break;
            case MENU_FLIP_NEXT:
                fliplist_attach_head(8, 1);
                break;
            case MENU_FLIP_PREVIOUS:
                fliplist_attach_head(8, 0);
                break;
            case MENU_DATASETTE_START:
                datasette_control(DATASETTE_CONTROL_START);
                break;
            case MENU_DATASETTE_STOP:
                datasette_control(DATASETTE_CONTROL_STOP);
                break;
            case MENU_DATASETTE_FORWARD:
                datasette_control(DATASETTE_CONTROL_FORWARD);
                break;
            case MENU_DATASETTE_REWIND:
                datasette_control(DATASETTE_CONTROL_REWIND);
                break;
            case MENU_DATASETTE_RECORD:
                datasette_control(DATASETTE_CONTROL_RECORD);
                break;
            case MENU_DATASETTE_RESET:
                datasette_control(DATASETTE_CONTROL_RESET);
                break;
            case MENU_DATASETTE_COUNTER:
                datasette_control(DATASETTE_CONTROL_RESET_COUNTER);
                break;
            case MENU_TOGGLE_TAPEPORT_DATASETTE:
                resources_toggle("Datasette", NULL);
                ui_update_menus();
                ui_draw_tape_status();
                break;
            case MENU_SNAPSHOT_LOAD:
                ui_select_file(B_OPEN_PANEL, SNAPSHOTLOAD_FILE, (void*)0);
                break;
            case MENU_SNAPSHOT_SAVE:
                ui_select_file(B_SAVE_PANEL, SNAPSHOTSAVE_FILE, (void*)0);
                break;
            case MENU_LOADQUICK:
                scan_files();
                if (snapcounter > 0) {
                    interrupt_maincpu_trigger_trap(load_quicksnapshot_trap, (void *)0);
                }
                break;
            case MENU_SAVEQUICK:
                scan_files();
                interrupt_maincpu_trigger_trap(save_quicksnapshot_trap, (void *)0);
                break;
            case MENU_NETPLAY_SERVER:
                network_start_server();
                break;
            case MENU_NETPLAY_CLIENT:
                network_connect_client();
                break;
            case MENU_NETPLAY_DISCONNECT:
                network_disconnect();
                break;
            case MENU_NETPLAY_SETTINGS:
                ui_netplay();
                break;
            case MENU_EVENT_TOGGLE_RECORD:
                if (event_record_active()) {
                    event_record_stop();
                } else {
                    event_record_start();
                }
                break;
            case MENU_EVENT_TOGGLE_PLAYBACK:
                if (event_playback_active()) {
                    event_playback_stop();
                } else {
                    event_playback_start();
                }
                break;
            case MENU_EVENT_SETMILESTONE:
                event_record_set_milestone();
                break;
            case MENU_EVENT_RESETMILESTONE:
                event_record_reset_milestone();
                break;
            case MENU_EVENT_SNAPSHOT_START:
                ui_select_file(B_SAVE_PANEL, SNAPSHOT_HISTORY_START, NULL);
                break;
            case MENU_EVENT_SNAPSHOT_END:
                ui_select_file(B_SAVE_PANEL, SNAPSHOT_HISTORY_END, NULL);
                break;
            case MENU_MONITOR:
                monitor_startup_trap();
                break;
            case MENU_PAUSE:
                ui_pause_emulation(!ui_emulation_is_paused());
                break;
            case MENU_SINGLE_FRAME_ADVANCE:
                if (ui_emulation_is_paused()) {
                    vsyncarch_advance_frame();
                }
                break;
            case MENU_COPY:
                ui_copy_clipboard();
                break;
            case MENU_PASTE:
                ui_paste_clipboard_text();
                break;
            case MENU_SOUND_RECORD_AIFF:
                resources_set_string("SoundRecordDeviceName", "");
                ui_select_file(B_SAVE_PANEL, AIFF_FILE, (void*)0);
                break;
            case MENU_SOUND_RECORD_IFF:
                resources_set_string("SoundRecordDeviceName", "");
                ui_select_file(B_SAVE_PANEL, IFF_FILE, (void*)0);
                break;
#ifdef USE_LAMEMP3
            case MENU_SOUND_RECORD_MP3:
                resources_set_string("SoundRecordDeviceName", "");
                ui_select_file(B_SAVE_PANEL, MP3_FILE, (void*)0);
                break;
#endif
#ifdef USE_FLAC
            case MENU_SOUND_RECORD_FLAC:
                resources_set_string("SoundRecordDeviceName", "");
                ui_select_file(B_SAVE_PANEL, FLAC_FILE, (void*)0);
                break;
#endif
#ifdef USE_VORBIS
            case MENU_SOUND_RECORD_VORBIS:
                resources_set_string("SoundRecordDeviceName", "");
                ui_select_file(B_SAVE_PANEL, VORBIS_FILE, (void*)0);
                break;
#endif
            case MENU_SOUND_RECORD_VOC:
                resources_set_string("SoundRecordDeviceName", "");
                ui_select_file(B_SAVE_PANEL, VOC_FILE, (void*)0);
                break;
            case MENU_SOUND_RECORD_WAV:
                resources_set_string("SoundRecordDeviceName", "");
                ui_select_file(B_SAVE_PANEL, WAV_FILE, (void*)0);
                break;
            case MENU_SOUND_RECORD_STOP:
                resources_set_string("SoundRecordDeviceName", "");
                ui_display_statustext("Media Recording Stopped...", 1);
                break;
            case MENU_DEVICE_SETTINGS:
                ui_device();
                break;
            case MENU_DATASETTE_SETTINGS:
                ui_datasette();
                break;
            case MENU_SOUND_SETTINGS:
                ui_sound();
                break;
            case MENU_RAM_SETTINGS:
                ui_ram();
                break;
            case MENU_SETTINGS_LOAD:
                if (resources_load(NULL) < 0) {
                    ui_error("Cannot load settings.");
                }
                ui_update_menus();
                break;
            case MENU_SETTINGS_SAVE:
                if (resources_save(NULL) < 0) {
                    ui_error("Cannot save settings.");
                }
                fliplist_save_list((unsigned int)-1, archdep_default_fliplist_file_name());
                break;
            case MENU_SETTINGS_DEFAULT:
                resources_set_defaults();
                ui_update_menus();
                break;
            case MENU_PRINTER_4_SEND_FF:
                printer_formfeed(0);
                break;
            case MENU_OUTPUT_DEVICE_1:
                ui_select_file(B_SAVE_PANEL, PRINTER_OUTPUT_FILE1, (void*)0);
                break;
            case MENU_OUTPUT_DEVICE_2:
                ui_select_file(B_SAVE_PANEL, PRINTER_OUTPUT_FILE2, (void*)0);
                break;
            case MENU_OUTPUT_DEVICE_3:
                ui_select_file(B_SAVE_PANEL, PRINTER_OUTPUT_FILE3, (void*)0);
                break;
            case MENU_ABOUT:
                char *abouttext;
                char *tmp;

                tmp = util_concat("BeVICE Version ", VERSION,
#ifdef USE_SVN_REVISION
                                  " rev" VICE_SVN_REV_STRING,
#endif
                                  "\n\n",
                                  NULL);
                for (i = 0; core_team[i].name; i++) {
                    abouttext = util_concat(tmp, "\xC2\xA9 ", core_team[i].years, " ", core_team[i].name, "\n", NULL);
                    lib_free(tmp);
                    tmp = abouttext;
                }

                abouttext = util_concat(tmp,
                                        "\nOfficial VICE homepage:\n",
                                        "http://vice-emu.sourceforge.net/",
                                        NULL);
                lib_free(tmp);
                ui_message(abouttext);
                lib_free(abouttext);
                break;
            case MENU_CONTRIBUTORS:
                ui_show_text("Contributors", "Who made what?", info_contrib_text);
                break;
            case MENU_LICENSE:
                ui_show_text("License", "VICE license (GNU General Public License)", info_license_text);
                break;
            case MENU_WARRANTY:
                ui_show_text("No warranty!", "VICE is distributed WITHOUT ANY WARRANTY!", info_warranty_text);
                break;
            case MENU_CMDLINE:
                {
                    char *options;

                    options = cmdline_options_string();
                    ui_show_text("Command line options", "Which command line options are available?", options);
                    free(options);
                    break;
                }
            case MENU_COMPILE_TIME_FEATURES:
                {
                    char *features;

                    features = get_compiletime_features();
                    ui_show_text("Compile time features", "Which compile time features are available?", features);
                    free(features);
                    break;
                }
            case MESSAGE_ATTACH_READONLY:
                {
                    int res_val;

                    message_queue[i].FindInt32("device",(int32*)&attachdrive);
                    resources_get_int_sprintf("AttachDevice%dReadonly", &res_val, attachdrive);
                    resources_set_int_sprintf("AttachDevice%dReadonly", !res_val, attachdrive);
                    break;
                }
            case MENU_SCREENSHOT_BMP_SCREEN0:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_BMP_FILE_SCREEN0, (void*)0);
                break;
            case MENU_SCREENSHOT_BMP_SCREEN1:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_BMP_FILE_SCREEN1, (void*)0);
                break;
            case MENU_SCREENSHOT_DOODLE_SCREEN0:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_DOODLE_FILE_SCREEN0, (void*)0);
                break;
            case MENU_SCREENSHOT_DOODLE_SCREEN1:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_DOODLE_FILE_SCREEN1, (void*)0);
                break;
            case MENU_SCREENSHOT_DOODLE_COMPRESSED_SCREEN0:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_DOODLE_COMPRESSED_FILE_SCREEN0, (void*)0);
                break;
            case MENU_SCREENSHOT_DOODLE_COMPRESSED_SCREEN1:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_DOODLE_COMPRESSED_FILE_SCREEN1, (void*)0);
                break;
#ifdef HAVE_GIF
            case MENU_SCREENSHOT_GIF_SCREEN0:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_GIF_FILE_SCREEN0, (void*)0);
                break;
            case MENU_SCREENSHOT_GIF_SCREEN1:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_GIF_FILE_SCREEN1, (void*)0);
                break;
#endif
            case MENU_SCREENSHOT_GODOT_SCREEN0:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_GODOT_FILE_SCREEN0, (void*)0);
                break;
            case MENU_SCREENSHOT_GODOT_SCREEN1:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_GODOT_FILE_SCREEN1, (void*)0);
                break;
            case MENU_SCREENSHOT_IFF_SCREEN0:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_IFF_FILE_SCREEN0, (void*)0);
                break;
            case MENU_SCREENSHOT_IFF_SCREEN1:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_IFF_FILE_SCREEN1, (void*)0);
                break;
#ifdef HAVE_JPEG
            case MENU_SCREENSHOT_JPEG_SCREEN0:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_JPEG_FILE_SCREEN0, (void*)0);
                break;
            case MENU_SCREENSHOT_JPEG_SCREEN1:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_JPEG_FILE_SCREEN1, (void*)0);
                break;
#endif
            case MENU_SCREENSHOT_KOALA_SCREEN0:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_KOALA_FILE_SCREEN0, (void*)0);
                break;
            case MENU_SCREENSHOT_KOALA_SCREEN1:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_KOALA_FILE_SCREEN1, (void*)0);
                break;
            case MENU_SCREENSHOT_KOALA_COMPRESSED_SCREEN0:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_KOALA_COMPRESSED_FILE_SCREEN0, (void*)0);
                break;
            case MENU_SCREENSHOT_KOALA_COMPRESSED_SCREEN1:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_KOALA_COMPRESSED_FILE_SCREEN1, (void*)0);
                break;
            case MENU_SCREENSHOT_PCX_SCREEN0:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_PCX_FILE_SCREEN0, (void*)0);
                break;
            case MENU_SCREENSHOT_PCX_SCREEN1:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_PCX_FILE_SCREEN1, (void*)0);
                break;
#ifdef HAVE_PNG
            case MENU_SCREENSHOT_PNG_SCREEN0:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_PNG_FILE_SCREEN0, (void*)0);
                break;
            case MENU_SCREENSHOT_PNG_SCREEN1:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_PNG_FILE_SCREEN1, (void*)0);
                break;
#endif
            case MENU_SCREENSHOT_PPM_SCREEN0:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_PPM_FILE_SCREEN0, (void*)0);
                break;
            case MENU_SCREENSHOT_PPM_SCREEN1:
                ui_select_file(B_SAVE_PANEL, SCREENSHOT_PPM_FILE_SCREEN1, (void*)0);
                break;
            case MENU_TAPEPORT_TAPLOG_FILENAME:
                ui_select_file(B_SAVE_PANEL, TAPELOG_FILE, (void*)0);
                break;
            case MENU_SAMPLER_FILENAME:
                ui_select_file(B_SAVE_PANEL, SAMPLER_MEDIA_FILE, (void*)0);
                break;
            case MENU_KEYMAP_SETTINGS:
                ui_keymap();
                break;

            case MESSAGE_SET_RESOURCE:
                {
                    const char *res_name;
                    int32 res_val;
                    const char *res_val_str;

                    if (message_queue[i].FindString("resname", &res_name) == B_OK) {
                        if (message_queue[i].FindInt32("resval", &res_val) == B_OK) {
                            resources_set_int(res_name, res_val);
                        }
                        if (message_queue[i].FindString("resvalstr", &res_val_str) == B_OK) {
                            resources_set_string(res_name, res_val_str);
                        }
                    }
                    break;
                }
            default:
                if (message_queue[i].what >= 'M000' && message_queue[i].what <= 'M999') {

                    /* Handle the TOGGLE-Menuitems */
                    if (ui_handle_toggle_items(toggle_list, message_queue[i].what)) {
                        break;
                    }
                    if (machine_class != VICE_MACHINE_VSID) {
                        if (ui_handle_toggle_items(non_vsid_toggles, message_queue[i].what)) {
                            break;
                        }
                    }
                    if (machine_specific_toggles) {
                        if (ui_handle_toggle_items(machine_specific_toggles, message_queue[i].what)) {
                            break;
                        }
                    }

                    /* Handle VALUE-Menuitems */
                    if (ui_handle_value_items(value_list, message_queue[i].what)) {
                        break;
                    }
                    if (machine_class != VICE_MACHINE_VSID) {
                        if (ui_handle_value_items(non_vsid_values, message_queue[i].what)) {
                            break;
                        }
                    }
                    if (machine_specific_values) {
                        if (ui_handle_value_items(machine_specific_values, message_queue[i].what)) {
                            break;
                        }
                    }

                    /* Handle STRING-Menuitems */
                    if (machine_class != VICE_MACHINE_VSID) {
                        if (ui_handle_string_items(non_vsid_strings, message_queue[i].what)) {
                            break;
                        }
                    }
                }
                break;
        }
    }
    num_queued_messages = 0;
}

void ui_dispatch_next_event(void)
{
}

/* -----------------------------------------------------------*/
/* Initialize the UI before setting all the resource values.  */

int ui_init(int *argc, char **argv)
{
    num_queued_messages = 0;
    return 0;
}

void ui_shutdown(void)
{
}

/* Initialize the UI after setting all the resource values.  */
int ui_init_finish(void)
{
    atexit(ui_exit);
    return 0;
}

int ui_init_finalize(void)
{
    return 0;
}

/* ------------------------------------------------------------------------- */

/* this displays text in a seperate window with scroller */
class TextWindow : public BWindow {
    public:
        TextWindow(const char *, const char*, const char*);
        ~TextWindow();
    private:
        BTextView *textview;
        BScrollView *scrollview;
};

TextWindow::TextWindow(
    const char *caption,
    const char *header,
    const char *text) : BWindow(BRect(0,0,500,300), caption, B_DOCUMENT_WINDOW, B_NOT_ZOOMABLE|B_NOT_RESIZABLE) {

    textview = new BTextView(BRect(0, 0, 500 - B_V_SCROLL_BAR_WIDTH, 300), "VICE textview", BRect(10, 10, 490 - B_V_SCROLL_BAR_WIDTH, 300), B_FOLLOW_NONE, B_WILL_DRAW);
    textview->MakeEditable(false);
    textview->MakeSelectable(false);
    textview->SetViewColor(230, 240, 230, 0);

    scrollview = new BScrollView("vice scroller", textview, B_FOLLOW_NONE, 0, false, true);
    textview->SetText(text);
    textview->Insert("\n\n");
    textview->Insert(0, header, strlen(header));
    AddChild(scrollview);
    MoveTo(50, 50);
    Show();
}

TextWindow::~TextWindow() {
    RemoveChild(scrollview);
    delete textview;
    delete scrollview;
}

void ui_show_text(const char *caption, const char *header, const char *text)
{
    new TextWindow(caption, header, text);
}

/* Report an error to the user (`printf()' style).  */
void ui_error(const char *format, ...)
{
    char tmp[1024];
    va_list args;

    va_start(args, format);
    vsprintf(tmp, format, args);
    va_end(args);

    ui_error_string(tmp);
}

/* Report an error to the user (one string).  */
void ui_error_string(const char *text)
{
    BAlert *messagebox;

    messagebox = new BAlert("error", text, "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_STOP_ALERT);
    vsync_suspend_speed_eval();
    messagebox->Go();
}

/* Report a message to the user (`printf()' style).  */
void ui_message(const char *format,...)
{
    BAlert *messagebox;
    char tmp[1024];
    va_list args;

    va_start(args, format);
    vsprintf(tmp, format, args);
    va_end(args);
    messagebox = new BAlert("info", tmp, "OK", NULL, NULL, B_WIDTH_AS_USUAL, B_INFO_ALERT);
    vsync_suspend_speed_eval();
    messagebox->Go();
}

/* Handle the "CPU JAM" case.  */
ui_jam_action_t ui_jam_dialog(const char *format,...)
{
    ui_error("Jam %s - Resetting the machine...", format);
    return UI_JAM_HARD_RESET;
}

/* Handle the "Do you want to extend the disk image to 40-track format"?
   dialog.  */
int ui_extend_image_dialog(void)
{
    int ret;
    BAlert *mb;
        
    mb = new BAlert("VICE question", "Do you want to extend the image?", "Yes", "No", NULL, B_WIDTH_AS_USUAL, B_IDEA_ALERT);
    ret = mb->Go();

    return ret == 0;
}

void ui_update_menus(void)
{
    int i;

    for (i = 0; i < window_count; i++) {
        windowlist[i]->Update_Menu_Toggles(toggle_list);
        windowlist[i]->Update_Menu_Toggles(machine_specific_toggles);
        windowlist[i]->Update_Menu_Value_Lists(value_list);
        windowlist[i]->Update_Menu_Value_Lists(machine_specific_values);
        if (machine_class != VICE_MACHINE_VSID) {
            windowlist[i]->Update_Menu_Toggles(non_vsid_toggles);
            windowlist[i]->Update_Menu_Value_Lists(non_vsid_values);
            windowlist[i]->Update_Menu_String_Lists(non_vsid_strings);
        }
    }
}

static int statustext_display_time = 0;

void ui_display_statustext(const char *text, int fade_out)
{
    int i;

    for (i = 0; i < window_count; i++) {
        if (windowlist[i]->Lock()) {
            if (windowlist[i]->statusbar) {
                windowlist[i]->statusbar->DisplayMessage(text);
            }
            windowlist[i]->Unlock();
        }
    }
    if (fade_out > 0) {
        statustext_display_time = 5;
    } else {
        statustext_display_time = 0;
    }
}

/* ------------------------------------------------------------------------- */
/* Dispay the current emulation speed.  */
void ui_display_speed(float percent, float framerate, int warp_flag)
{
    int i;

    for (i = 0; i < window_count; i++) {
        if (windowlist[i]->Lock()) {
            if (windowlist[i]->statusbar) {
                windowlist[i]->statusbar->DisplaySpeed(percent, framerate, warp_flag);
            }
            windowlist[i]->Unlock();
        }
    }

    if (statustext_display_time > 0) {
        statustext_display_time--;
        if (statustext_display_time == 0) {
            ui_display_statustext("", 0);
        }
    }
}

static ui_drive_enable_t ui_drive_enabled;
static int ui_status_led[DRIVE_NUM];
static double ui_status_track[DRIVE_NUM];
static int *ui_drive_active_led;
static char *ui_drive_image_name[DRIVE_NUM];
static char *ui_tape_image_name;


static void ui_display_drive_status(int drive_num)
{
    int i;
    int drive_num_active;
    int drive_led_index;

    for (i = 0; i < window_count; i++) {
        while (!windowlist[i]->Lock());
        if (windowlist[i]->statusbar) {
            int this_drive = 1 << drive_num;

            if (ui_drive_enabled & this_drive) {
                drive_num_active = drive_num;
            } else {
                drive_num_active = -1 - drive_num; /* this codes erasing the statusbar */
            }
            if (!ui_status_led[drive_num]) {
                drive_led_index = -1;
            } else {
                drive_led_index = ui_drive_active_led[drive_num];
            }
            windowlist[i]->statusbar->DisplayDriveStatus(drive_num_active, drive_led_index, ui_status_track[drive_num]);
        }
        windowlist[i]->Unlock();
    }
}

void ui_enable_drive_status(ui_drive_enable_t enable, int *drive_led_color)
{
    ui_drive_enabled = enable;
    ui_drive_active_led = drive_led_color;      
    ui_display_drive_status(0);
    ui_display_drive_status(1);
    ui_display_drive_status(2);
    ui_display_drive_status(3);
}

/* Toggle displaying of the drive track.  */
/* drive_base is either 8 or 0 depending on unit or drive display.
   Dual drives display drive 0: and 1: instead of unit 8: and 9: */
void ui_display_drive_track(unsigned int drive_number, unsigned int drive_base, unsigned int half_track_number)
{
    double track_number = (double)half_track_number / 2.0;

    ui_status_track[drive_number] = track_number;
    ui_display_drive_status(drive_number);
}

/* Toggle displaying of the drive LED.  */
void ui_display_drive_led(int drivenum, unsigned int led_pwm1, unsigned int led_pwm2)
{
    int status = 0;

    if (led_pwm1 > 100) {
        status |= 1;
    }

    if (led_pwm2 > 100) {
        status |= 2;
    }

    ui_status_led[drivenum] = status;
    ui_display_drive_status(drivenum);
}

static void ui_display_image(int drivenum)
{
    int i;
    char *image;
    
    if (drivenum < 0) {
        image = ui_tape_image_name;
    } else {
        image = ui_drive_image_name[drivenum];
    }
    for (i = 0; i < window_count; i++) {
        while (!windowlist[i]->Lock());
        if (windowlist[i]->statusbar) {
            windowlist[i]->statusbar->DisplayImage(drivenum, image);
        }
        windowlist[i]->Unlock();
    }
}

/* display current image */
void ui_display_drive_current_image(unsigned int drivenum, const char *image)
{
    char *directory_name;
        
    if (drivenum >= 4) {
        return;
    }
                
    if (ui_drive_image_name[drivenum]) {
        free(ui_drive_image_name[drivenum]);
    }

    util_fname_split(image, &directory_name, &ui_drive_image_name[drivenum]);
    free(directory_name);
    ui_display_image(drivenum);
}

void ui_display_tape_current_image(const char *image)
{
    char *directory_name;

    if (ui_tape_image_name) {
        free(ui_tape_image_name);
    }

    util_fname_split(image, &directory_name, &ui_tape_image_name);
    free(directory_name);
    ui_display_image(-1);
}

static int ui_tape_enabled = 0;
static int ui_tape_counter = -1;
static int ui_tape_motor = -1;
static int ui_tape_control = -1;

static void ui_draw_tape_status()
{
    int enabled = 0;
    int i;

    if (machine_class != VICE_MACHINE_VSID && machine_class != VICE_MACHINE_C64DTV && machine_class != VICE_MACHINE_SCPU64) {
        resources_get_int("Datasette", &enabled);
    }

    enabled |= ui_tape_enabled;

    for (i = 0; i < window_count; i++) {
        while (!windowlist[i]->Lock());
        if (windowlist[i]->statusbar) {
            windowlist[i]->statusbar->DisplayTapeStatus(enabled, ui_tape_counter, ui_tape_motor, ui_tape_control);
        }
        windowlist[i]->Unlock();
    }
}

/* tape-status on*/
void ui_set_tape_status(int tape_status)
{
    if (ui_tape_enabled != tape_status) {
        ui_tape_enabled = tape_status;
        ui_draw_tape_status();
    }
}

void ui_display_tape_motor_status(int motor)
{  
    if (ui_tape_motor != motor) {
        ui_tape_motor = motor;
        ui_draw_tape_status();
    }
}

void ui_display_tape_control_status(int control)
{
    if (ui_tape_control != control) {
        ui_tape_control = control;
        ui_draw_tape_status();
    }
}

void ui_display_tape_counter(int counter)
{
    if (ui_tape_counter != counter) {
        ui_tape_counter = counter;
        ui_draw_tape_status();
    }
}

void ui_display_recording(int recording_status)
{
    if (recording_status) {
        ui_display_statustext("Recoring history...", 1);
    } else {
        ui_display_statustext("", 0);
    }
}

void ui_display_playback(int playback_status, char *version)
{
    if (playback_status) {
        ui_display_statustext("History playback...", 1);
    } else {
        ui_display_statustext("", 0);
    }
}

void ui_display_event_time(unsigned int current, unsigned int total)
{
    char text[256];

    if (statustext_display_time > 0) {
        return;
    }

    if (total == 0) {
        sprintf(text, "Recording %02d:%02d", current / 60, current % 60);
    } else {
        sprintf(text, "Playback %02d:%02d (%02d:%02d)", current / 60, current % 60, total / 60, total % 60);
    }
    ui_display_statustext(text, 0);
}

static uint8_t ui_joystick_status[3] = { 255, 255, 255 };

static void ui_display_joyport(int port_num)
{
    int i;
        
    if (!joystickdisplay) {
        return;
    }

    for (i = 0; i < window_count; i++) {
        while (!windowlist[i]->Lock());
        if (windowlist[i]->statusbar) {
            windowlist[i]->statusbar->DisplayJoyport(port_num, ui_joystick_status[port_num]);
        }
        windowlist[i]->Unlock();
    }
}
                
void ui_enable_joyport(void)
{
    int i;

    for (i = 0; i < window_count; i++) {
        while (!windowlist[i]->Lock());
        if (windowlist[i]->statusbar) {
            windowlist[i]->statusbar->EnableJoyport(joystickdisplay);
            windowlist[i]->Unlock();
        }
    }
    ui_joystick_status[1]=ui_joystick_status[2]=0;
    ui_display_joyport(1);
    ui_display_joyport(2);
}


void ui_display_joyport(uint8_t *joyport)
{
    int i;

    for (i = 1; i <= 2; i++) {
        if (ui_joystick_status[i] != joyport[i]) {
            ui_joystick_status[i] = joyport[i];
            ui_display_joyport(i);
        }
    }
}

void ui_statusbar_update()
{
    int i;

    for (i = 0; i < 4; i++) {
        ui_display_drive_status(i);
        ui_display_image(i);
    }
    ui_draw_tape_status();
    ui_display_image(-1);
    ui_enable_joyport();
}

int ui_set_window_mode(int use_direct_window)
{
    int i;
    ViceWindow *w;
    struct video_canvas_s *c;

    for (i = 0; i < window_count; i++) {
        w = windowlist[i];
        if (w != NULL) {
            w->Lock();

            /* only use DirectWindow if Window Mode is supported */
            if (!w->SupportsWindowMode() || CheckForHaiku()) {
                use_direct_window = 0;
            }

            w->use_direct_window = use_direct_window;

            if (use_direct_window) {
                w->view->SetViewColor(B_TRANSPARENT_32_BIT);
            } else {
                w->view->SetViewColor(255, 255, 255);
            }

            w->Unlock();

            c = (struct video_canvas_s*) (w->canvas);
            if (c != NULL) {
                video_canvas_refresh_all(c);
            }
        }
    }
    return use_direct_window;
}

void ui_display_volume(int vol)
{
}

char* ui_get_file(const char *format,...)
{
    return NULL;
}
