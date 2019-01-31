/*
 * uiapi.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __USE_INLINE__

#include <proto/intuition.h>
#include <proto/exec.h>

#define VICE_UIAPI

#include "private.h"
#include "ui.h"
#include "cmdline.h"
#include "lib.h"
#include "loadlibs.h"
#include "machine.h"
#include "resources.h"
#include "types.h"
#include "ui.h"
#include "uilib.h"
#include "util.h"
#include "fullscreenarch.h"
#include "videoarch.h"
#include "mousedrv.h"
#include "statusbar.h"
#include "intl.h"
#include "translate.h"

typedef enum {
    UI_JAM_RESET, UI_JAM_HARD_RESET, UI_JAM_MONITOR, UI_JAM_NONE
} ui_jam_action_t;

enum {
    REQ_JAM_NONE,
    REQ_JAM_RESET,
    REQ_JAM_HARD_RESET,
    REQ_JAM_MONITOR
};

extern video_canvas_t *canvaslist;

/* UI-related resources.  */

ui_resources_t ui_resources;

static int set_fullscreen_enabled(int val, void *param)
{
    ui_resources.fullscreenenabled = val ? 1 : 0;

    video_arch_fullscreen_toggle();

    return 0;
}

#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
static int set_videooverlay_enabled(int value, void *param)
{
    int val = value ? 1 : 0;

    if (!xvideo_lib_loaded && val) {
        return -1;
    }

    ui_resources.videooverlayenabled = val;

    /* reuse the fullscreen toggle */
    video_arch_fullscreen_toggle();

    return 0;
}
#endif

static int set_statusbar_enabled(int val, void *param)
{
    ui_resources.statusbarenabled = val ? 1 : 0;

    video_arch_fullscreen_toggle();

    return 0;
}

static int set_save_resources_on_exit(int val, void *param)
{
    ui_resources.save_resources_on_exit = val ? 1 : 0;

    return 0;
}

static int set_confirm_on_exit(int val, void *param)
{
    ui_resources.confirm_on_exit = val ? 1 : 0;

    return 0;
}

static int set_initial_dir(const char *name, void *param)
{
    int index = (int)param;

    if (ui_resources.initialdir[index] != NULL && name != NULL) {
        if (strcmp(name, ui_resources.initialdir[index]) == 0) {
            return 0;
        }
    }
    util_string_set(&ui_resources.initialdir[index], name ? name : "");
    return 0;
}

static const resource_string_t resources_string[] = {
    { "InitialDefaultDir", "", RES_EVENT_NO, NULL,
      &ui_resources.initialdir[0], set_initial_dir, (void *)0 },
    RESOURCE_STRING_LIST_END
};

static const resource_string_t init_resources_string[] = {
    { "InitialDiskDir", "", RES_EVENT_NO, NULL,
      &ui_resources.initialdir[2], set_initial_dir, (void *)2 },
    { "InitialAutostartDir", "", RES_EVENT_NO, NULL,
      &ui_resources.initialdir[3], set_initial_dir, (void *)3 },
    { "InitialSnapshotDir", "", RES_EVENT_NO, NULL,
      &ui_resources.initialdir[5], set_initial_dir, (void *)5 },
    RESOURCE_STRING_LIST_END
};

static const resource_string_t init_tape_resources_string[] = {
    { "InitialTapeDir", "", RES_EVENT_NO, NULL,
      &ui_resources.initialdir[1], set_initial_dir, (void *)1 },
    RESOURCE_STRING_LIST_END
};

static const resource_string_t init_cart_resources_string[] = {
    { "InitialCartDir", "", RES_EVENT_NO, NULL,
      &ui_resources.initialdir[4], set_initial_dir, (void *)4 },
    RESOURCE_STRING_LIST_END
};

static const resource_int_t resources_int[] = {
    { "FullscreenEnabled", 0, RES_EVENT_NO, NULL,
      &ui_resources.fullscreenenabled, set_fullscreen_enabled, NULL },
    { "StatusBarEnabled", 1, RES_EVENT_NO, NULL,
      &ui_resources.statusbarenabled, set_statusbar_enabled, NULL },
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
    { "VideoOverlayEnabled", 0, RES_EVENT_NO, NULL,
      &ui_resources.videooverlayenabled, set_videooverlay_enabled, NULL },
#endif
    { "SaveResourcesOnExit", 0, RES_EVENT_NO, NULL,
      &ui_resources.save_resources_on_exit, set_save_resources_on_exit, NULL },
    { "ConfirmOnExit", 1, RES_EVENT_NO, NULL,
      &ui_resources.confirm_on_exit, set_confirm_on_exit, NULL },
    RESOURCE_INT_LIST_END
};

int ui_resources_init(void)
{
    translate_resources_init();

    if (resources_register_string(resources_string) < 0) {
        return -1;
    }

    if (machine_class != VICE_MACHINE_VSID) {
        if (resources_register_string(init_resources_string) < 0) {
            return -1;
        }
        if (machine_class != VICE_MACHINE_SCPU64 && machine_class != VICE_MACHINE_C64DTV) {
            if (resources_register_string(init_tape_resources_string) < 0) {
                return -1;
            }
        }
        if (machine_class != VICE_MACHINE_C64DTV) {
            if (resources_register_string(init_cart_resources_string) < 0) {
                return -1;
            }
        }
    }

    return resources_register_int(resources_int);
}

void ui_resources_shutdown(void)
{
    int i;

    translate_resources_shutdown();

    for (i = 0; i < UILIB_SELECTOR_STYLES_NUM; i++) {
        lib_free(ui_resources.initialdir[i]);
    }
}

/* ------------------------------------------------------------------------ */

/* UI-related command-line options.  */

static const cmdline_option_t cmdline_options[] = {
    { "-initialdiskdir", SET_RESOURCE, 1,
      NULL, NULL, "InitialDiskDir", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NAME, IDS_SPECIFY_INITIAL_DISK_DIR,
      NULL, NULL },
    { "-initialautostartdir", SET_RESOURCE, 1,
      NULL, NULL, "InitialAutostartDir", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NAME, IDS_SPECIFY_INITIAL_AUTOSTART_DIR,
      NULL, NULL },
    { "-initialsnapshotdir", SET_RESOURCE, 1,
      NULL, NULL, "InitialSnapshotDir", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NAME, IDS_SPECIFY_INITIAL_SNAPSHOT_DIR,
      NULL, NULL },
    CMDLINE_LIST_END
};

static const cmdline_option_t tape_cmdline_options[] = {
    { "-initialtapedir", SET_RESOURCE, 1,
      NULL, NULL, "InitialTapeDir", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NAME, IDS_SPECIFY_INITIAL_TAPE_DIR,
      NULL, NULL },
    CMDLINE_LIST_END
};

static const cmdline_option_t cart_cmdline_options[] = {
    { "-initialcartdir", SET_RESOURCE, 1,
      NULL, NULL, "InitialCartDir", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NAME, IDS_SPECIFY_INITIAL_CART_DIR,
      NULL, NULL },
    CMDLINE_LIST_END
};

static const cmdline_option_t common_cmdline_options[] = {
    { "-saveres", SET_RESOURCE, 0,
      NULL, NULL, "SaveResourcesOnExit", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_SAVE_SETTINGS_ON_EXIT,
      NULL, NULL },
    { "+saveres", SET_RESOURCE, 0,
      NULL, NULL, "SaveResourcesOnExit", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_NEVER_SAVE_SETTINGS_EXIT,
      NULL, NULL },
    { "-confirmexit", SET_RESOURCE, 0,
      NULL, NULL, "ConfirmOnExit", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_CONFIRM_QUITING_VICE,
      NULL, NULL },
    { "+confirmexit", SET_RESOURCE, 0,
      NULL, NULL, "ConfirmOnExit", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_NEVER_CONFIRM_QUITING_VICE,
      NULL, NULL },
    { "-initialdefaultdir", SET_RESOURCE, 1,
      NULL, NULL, "InitialDefaultDir", NULL,
      USE_PARAM_ID, USE_DESCRIPTION_ID,
      IDCLS_P_NAME, IDS_SPECIFY_INITIAL_DEFAULT_DIR,
      NULL, NULL },
    { "-fullscreen", SET_RESOURCE, 0,
      NULL, NULL, "FullscreenEnabled", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_ENABLE_FULLSCREEN,
      NULL, NULL },
    { "+fullscreen", SET_RESOURCE, 0,
      NULL, NULL, "FullscreenEnabled", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_DISABLE_FULLSCREEN,
      NULL, NULL },
    { "-statusbar", SET_RESOURCE, 0,
      NULL, NULL, "StatusBarEnabled", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_ENABLE_STATUSBAR,
      NULL, NULL },
    { "+statusbar", SET_RESOURCE, 0,
      NULL, NULL, "StatusBarEnabled", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_DISABLE_STATUSBAR,
      NULL, NULL },
#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
    { "-videooverlay", SET_RESOURCE, 0,
      NULL, NULL, "VideoOverlayEnabled", (resource_value_t)1,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_ENABLE_VIDEOOVERLAY,
      NULL, NULL },
    { "+videooverlay", SET_RESOURCE, 0,
      NULL, NULL, "VideoOverlayEnabled", (resource_value_t)0,
      USE_PARAM_STRING, USE_DESCRIPTION_ID,
      IDCLS_UNUSED, IDS_DISABLE_VIDEOOVERLAY,
      NULL, NULL },
#endif
    CMDLINE_LIST_END
};

int ui_cmdline_options_init(void)
{
    translate_cmdline_options_init();

    if (machine_class != VICE_MACHINE_VSID) {
        if (cmdline_register_options(cmdline_options) < 0) {
            return -1;
        }
        if (machine_class != VICE_MACHINE_SCPU64 && machine_class != VICE_MACHINE_C64DTV) {
            if (cmdline_register_options(tape_cmdline_options) < 0) {
                return -1;
            }
        }
        if (machine_class != VICE_MACHINE_C64DTV) {
            if (cmdline_register_options(cart_cmdline_options) < 0) {
                return -1;
            }
        }
    }

    return cmdline_register_options(common_cmdline_options);
}

int ui_init(int *argc, char **argv)
{
    return 0;
}

int ui_init_finish(void)
{
    return 0;
}

int ui_init_finalize(void)
{
    return 0;
}

void ui_shutdown(void)
{
    /* if mousedrv.c inputhandler is active, remove it */
    rem_inputhandler();
}

int ui_requester(char *title, char *msg, char *buttons, int defval)
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

        retval = EasyRequest(canvaslist->os->window, uiRequester, NULL, NULL);
    } else {
        fprintf(stderr,"%s : %s\n",title, msg);
        return defval;
    }
    lib_FreeMem(uiRequester, sizeof(struct EasyStruct));
    return retval;
}

/* Print a message.  */
void ui_message(const char *format,...)
{
    va_list ap;
    char *tmp;

    va_start(ap, format);
    tmp = lib_mvsprintf(format,ap);
    va_end(ap);

    ui_requester(translate_text(IDMES_VICE_MESSAGE), tmp, translate_text(IDMES_OK), 0);

    lib_free(tmp);
}

/* Print an error message.  */
void ui_error(const char *format,...)
{
    va_list ap;
    char *tmp;

    va_start(ap, format);
    tmp = lib_mvsprintf(format,ap);
    va_end(ap);

    ui_requester(translate_text(IDMES_VICE_ERROR), tmp, translate_text(IDMES_OK), 0);

    lib_free(tmp);
}

/* Show a CPU JAM dialog.  */
ui_jam_action_t ui_jam_dialog(const char *format, ...)
{
    va_list ap;
    char *tmp;
    int action;
    ui_jam_action_t jamaction = UI_JAM_RESET;

    va_start(ap, format);
    tmp = lib_mvsprintf(format,ap);
    va_end(ap);

    action = ui_requester("VICE CPU Jam", tmp, "RESET|HARD RESET|MONITOR|CONTINUE", REQ_JAM_RESET);

    switch(action) {
        case REQ_JAM_RESET:
            jamaction = UI_JAM_RESET;
            break;
        case REQ_JAM_HARD_RESET:
            jamaction = UI_JAM_HARD_RESET;
            break;
        case REQ_JAM_MONITOR:
            jamaction = UI_JAM_MONITOR;
            break;
        case REQ_JAM_NONE:
            jamaction = UI_JAM_NONE;
            break;
    }
    return jamaction;
}

/* Update all menu entries.  */
void ui_update_menus(void)
{
}

/* Recording UI */
void ui_display_playback(int playback_status, char *version)
{
}

void ui_display_recording(int recording_status)
{
}

void ui_display_event_time(unsigned int current, unsigned int total)
{
}

/* joystick UI */
void ui_display_joyport(uint8_t *joyport)
{
}

void ui_display_statustext(const char *text, int fade_out)
{
    statusbar_set_statustext(text, fade_out);
}

void ui_display_volume(int vol)
{
}

char* ui_get_file(const char *format,...)
{
    return NULL;
}
