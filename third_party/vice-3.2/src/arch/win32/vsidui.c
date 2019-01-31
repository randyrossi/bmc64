/*
 * vsidui.c - Implementation of the VSID UI.
 *
 * Written by
 *  Dag Lem <resid@nimrod.no>
 *
 * Win32 implementation
 *  Emiliano 'iAN CooG' Peruch <iancoog@email.it>
 *  Adapted to common API by Andreas Matthies
 *  Menu addition by Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * based on c64ui.c written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andre Fachat <fachat@physik.tu-chemnitz.de>
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
#include <string.h>
#include <tchar.h>
#include <windows.h>

#include "intl.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "monitor.h"
#include "psid.h"
#include "res.h"
#include "resources.h"
#include "system.h"
#include "translate.h"
#include "ui.h"
#include "uiapi.h"
#include "uihelp.h"
#include "uilib.h"
#include "uisid.h"
#include "uisound.h"
#include "uispeed.h"
#include "vsync.h"
#include "winmain.h"

#include "vsidui.h"


#define countof(array) (sizeof(array) / sizeof((array)[0]))

enum {
    VSID_S_TITLE = 0,
    VSID_S_AUTHOR,
    VSID_S_RELEASED,
    VSID_S_BLANK_1,
    VSID_S_PLAYING,
    VSID_S_SYNC,
    VSID_S_MODEL,
    VSID_S_IRQ,
    VSID_S_BLANK_2,
    VSID_S_DRIVER,
    VSID_S_IMAGE,
    VSID_S_INIT,
    VSID_S_PLAY,
    VSID_S_BLANK_3,
    VSID_S_TIMER,
    VSID_S_LASTLINE
};

static const int c64_sid_baseaddress[] = { 0xd4, 0xd5, 0xd6, 0xd7, 0xde, 0xdf, -1 };


TCHAR st_AppName[] = TEXT("VSID - The VICE SID player");
char vsidstrings[VSID_S_LASTLINE + 1][80] = { { 0 } };

static int current_song;
static int songs;
static int default_song;

MSG msg ;
HWND hwnd;
WNDCLASS wndclass;

static HMENU menu;

static LRESULT CALLBACK window_proc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam);

/*****************************************************************************/
static void vsid_disp(int txout_x, int txout_y, const char *str1, const char* str2)
{
    HDC hDC;
    RECT r;
    SIZE size;
    char tmp[100];
    TCHAR st_tmp[100];
    int st_len;

    if (NULL != hwnd) {
        hDC = GetDC(hwnd);

        if (NULL != str2) {
            SelectObject(hDC, GetStockObject (SYSTEM_FIXED_FONT));
            GetTextExtentPoint32(hDC, TEXT(" "), 1, &size);
            sprintf(tmp, str1, str2);
            st_len = system_mbstowcs(st_tmp, tmp, 100);
            SetBkColor(hDC, GetSysColor(COLOR_BTNFACE));
            TextOut(hDC, 8 + (txout_x * size.cx), 8 + (txout_y * (size.cy + 3)), st_tmp, st_len);
        } else {
            GetClientRect(hwnd, &r);
            FillRect(hDC, &r, GetSysColorBrush(COLOR_BTNFACE));
        }
        ReleaseDC(hwnd, hDC);
    }
}

/*****************************************************************************/

static generic_trans_table_t generic_trans_table[] = {
    { IDM_SOUND_VOLUME_0,    TEXT("0%") },
    { IDM_SOUND_VOLUME_5,    TEXT("5%") },
    { IDM_SOUND_VOLUME_10,   TEXT("10%") },
    { IDM_SOUND_VOLUME_25,   TEXT("25%") },
    { IDM_SOUND_VOLUME_50,   TEXT("50%") },
    { IDM_SOUND_VOLUME_75,   TEXT("75%") },
    { IDM_SOUND_VOLUME_100,  TEXT("100%") },
    { IDM_SYNC_FACTOR_PAL,   TEXT("&PAL") },
    { IDM_SYNC_FACTOR_NTSC,  TEXT("&NTSC") },
    { IDM_MAXIMUM_SPEED_200, TEXT("200%") },
    { IDM_MAXIMUM_SPEED_100, TEXT("100%") },
    { IDM_MAXIMUM_SPEED_50,  TEXT("50%") },
    { IDM_MAXIMUM_SPEED_20,  TEXT("20%") },
    { IDM_MAXIMUM_SPEED_10,  TEXT("10%") },
    { 0, NULL }
};

static ui_menu_translation_table_t vsidui_menu_translation_table[] = {
    { IDM_EXIT, IDS_MI_EXIT },
    { IDM_ABOUT, IDS_MI_ABOUT },
    { IDM_HELP, IDS_MP_HELP },
    { IDM_PAUSE, IDS_MI_PAUSE },
    { IDM_RESET_HARD, IDS_MI_RESET_HARD },
    { IDM_RESET_SOFT, IDS_MI_RESET_SOFT },
    { IDM_TOGGLE_SOUND, IDS_MI_TOGGLE_SOUND },
    { IDM_SOUND_RECORD_START, IDS_MI_SOUND_RECORD_START },
    { IDM_SOUND_RECORD_STOP, IDS_MI_SOUND_RECORD_STOP },
    { IDM_TOGGLE_ALWAYSONTOP, IDS_MI_TOGGLE_ALWAYSONTOP },
    { IDM_TOGGLE_CPU_AFFINITY, IDS_MI_TOGGLE_CPU_AFFINITY },
    { IDM_SOUND_SETTINGS, IDS_MI_SOUND_SETTINGS },
    { IDM_SID_SETTINGS, IDS_MI_SID_SETTINGS },
    { IDM_SETTINGS_SAVE_FILE, IDS_MI_SETTINGS_SAVE_FILE },
    { IDM_SETTINGS_LOAD_FILE, IDS_MI_SETTINGS_LOAD_FILE },
    { IDM_SETTINGS_SAVE, IDS_MI_SETTINGS_SAVE },
    { IDM_SETTINGS_LOAD, IDS_MI_SETTINGS_LOAD },
    { IDM_SETTINGS_DEFAULT, IDS_MI_SETTINGS_DEFAULT },
    { IDM_TOGGLE_SAVE_SETTINGS_ON_EXIT, IDS_MI_SAVE_SETTINGS_ON_EXIT },
    { IDM_TOGGLE_CONFIRM_ON_EXIT, IDS_MI_CONFIRM_ON_EXIT },
    { IDM_LANG_EN, IDS_MI_LANG_EN },
    { IDM_LANG_DA, IDS_MI_LANG_DA },
    { IDM_LANG_DE, IDS_MI_LANG_DE },
    { IDM_LANG_ES, IDS_MI_LANG_ES },
    { IDM_LANG_FR, IDS_MI_LANG_FR },
    { IDM_LANG_HU, IDS_MI_LANG_HU },
    { IDM_LANG_IT, IDS_MI_LANG_IT },
    { IDM_LANG_KO, IDS_MI_LANG_KO },
    { IDM_LANG_NL, IDS_MI_LANG_NL },
    { IDM_LANG_PL, IDS_MI_LANG_PL },
    { IDM_LANG_RU, IDS_MI_LANG_RU },
    { IDM_LANG_SV, IDS_MI_LANG_SV },
    { IDM_LANG_TR, IDS_MI_LANG_TR },
    { IDM_CMDLINE, IDS_MI_CMDLINE },
    { IDM_KEYS, IDS_MI_KEYS },
    { IDM_FEATURES, IDS_MI_FEATURES },
    { IDM_CONTRIBUTORS, IDS_MI_CONTRIBUTORS },
    { IDM_LICENSE, IDS_MI_LICENSE },
    { IDM_WARRANTY, IDS_MI_WARRANTY },
    { IDM_LOAD_PSID_FILE, IDS_MI_LOAD_PSID_FILE },
    { IDM_SELECT_TUNE, IDS_MI_SELECT_TUNE },
    { IDM_NEXT_TUNE, IDS_MI_NEXT_TUNE },
    { IDM_PREVIOUS_TUNE, IDS_MI_PREVIOUS_TUNE },
    { IDM_PSID_OVERRIDE, IDS_MI_PSID_OVERRIDE },
    { IDM_MONITOR, IDS_MI_MONITOR },
    { IDM_MAXIMUM_SPEED_NO_LIMIT, IDS_MI_MAXIMUM_SPEED_NO_LIMIT },
    { IDM_MAXIMUM_SPEED_CUSTOM, IDS_MI_MAXIMUM_SPEED_CUSTOM },
    { IDM_TOGGLE_WARP_MODE, IDS_MI_TOGGLE_WARP_MODE },
    { 0, 0 }
};

static ui_popup_translation_table_t vsidui_popup_translation_table[] = {
    { 1, IDS_MP_FILE, NULL },
    { 2, IDS_MP_SOUND_RECORDING, NULL },
    { 2, IDS_MP_RESET, NULL },
    { 1, IDS_MP_TUNES, NULL },
    { 1, IDS_MP_SETTINGS, NULL },
    { 2, IDS_MP_MAXIMUM_SPEED, NULL },
    { 2, IDS_MP_SOUND_VOLUME, NULL },
    { 2, IDS_MP_VIDEO_STANDARD, NULL },
    { 1, IDS_MP_LANGUAGE, NULL },
    { 1, IDS_MP_HELP, NULL },
    { 0, 0, NULL }
};

/* List of resources that can be switched on and off from the menus.  */
static const ui_menu_toggle_t toggle_list[] = {
    { "Sound", IDM_TOGGLE_SOUND },
    { "WarpMode", IDM_TOGGLE_WARP_MODE },
    { "SaveResourcesOnExit", IDM_TOGGLE_SAVE_SETTINGS_ON_EXIT },
    { "ConfirmOnExit", IDM_TOGGLE_CONFIRM_ON_EXIT },
    { "AlwaysOnTop", IDM_TOGGLE_ALWAYSONTOP },
    { "PSIDKeepEnv", IDM_PSID_OVERRIDE },
    { NULL, 0 }
};

static const ui_res_possible_values_t SyncFactor[] = {
    { MACHINE_SYNC_PAL, IDM_SYNC_FACTOR_PAL },
    { MACHINE_SYNC_NTSC, IDM_SYNC_FACTOR_NTSC },
    { MACHINE_SYNC_NTSCOLD, IDM_SYNC_FACTOR_NTSCOLD },
    { MACHINE_SYNC_PALN, IDM_SYNC_FACTOR_PALN },
    { -1, 0 }
};

static const ui_res_possible_values_t SoundVolume[] = {
    { 0, IDM_SOUND_VOLUME_0 },
    { 5, IDM_SOUND_VOLUME_5 },
    { 10, IDM_SOUND_VOLUME_10 },
    { 25, IDM_SOUND_VOLUME_25 },
    { 50, IDM_SOUND_VOLUME_50 },
    { 75, IDM_SOUND_VOLUME_75 },
    { 100, IDM_SOUND_VOLUME_100 },
    { -1, 0 }
};

static const ui_res_possible_values_t SpeedValues[] = {
    { 200, IDM_MAXIMUM_SPEED_200 },
    { 100, IDM_MAXIMUM_SPEED_100 },
    { 50, IDM_MAXIMUM_SPEED_50, },
    { 20, IDM_MAXIMUM_SPEED_20 },
    { 10, IDM_MAXIMUM_SPEED_10 },
    { 0, IDM_MAXIMUM_SPEED_NO_LIMIT },
    { -1, 0 }
};

#ifdef DEBUG
static const ui_res_possible_values_t TraceMode[] = {
    { DEBUG_NORMAL, IDM_DEBUG_MODE_NORMAL },
    { DEBUG_SMALL, IDM_DEBUG_MODE_SMALL },
    { DEBUG_HISTORY, IDM_DEBUG_MODE_HISTORY },
    { DEBUG_AUTOPLAY, IDM_DEBUG_MODE_AUTOPLAY },
    { -1, 0 }
};
#endif

static const ui_res_value_list_t value_list[] = {
    { "MachineVideoStandard", SyncFactor, 0 },
    { "SoundVolume", SoundVolume, IDM_SOUND_VOLUME_CUSTOM },
#ifdef DEBUG
    { "TraceMode", TraceMode, 0},
#endif
    { "Speed", SpeedValues, IDM_MAXIMUM_SPEED_CUSTOM },
    { NULL, NULL, 0 }
};

static const struct {
    char *lang_code;
    UINT item_id;
} ui_lang_menu_entries [] = {
    { "en", IDM_LANG_EN },
    { "da", IDM_LANG_DA },
    { "de", IDM_LANG_DE },
    { "es", IDM_LANG_ES },
    { "fr", IDM_LANG_FR },
    { "hu", IDM_LANG_HU },
    { "it", IDM_LANG_IT },
    { "ko", IDM_LANG_KO },
    { "nl", IDM_LANG_NL },
    { "pl", IDM_LANG_PL },
    { "ru", IDM_LANG_RU },
    { "sv", IDM_LANG_SV },
    { "tr", IDM_LANG_TR },
    { NULL, 0}
};

static void ui_translate_menu_items(HMENU menu, ui_menu_translation_table_t *trans_table)
{
    int i;

    if (trans_table == NULL) {
        return;
    }

    for (i = 0; trans_table[i].idm != 0; i++) {
        uilib_localize_menu_item(menu, trans_table[i].idm, trans_table[i].ids);
    }

    for (i = 0; generic_trans_table[i].idm != 0; i++) {
        uilib_set_menu_item_text(menu, generic_trans_table[i].idm, MF_BYCOMMAND, generic_trans_table[i].idm, generic_trans_table[i].text);
    }
}

static void ui_translate_menu_popups(HMENU menu, ui_popup_translation_table_t *trans_table)
{
    int pos1 = -1;
    int pos2 = -1;
    int pos3 = -1;

    HMENU menu1 = NULL;
    HMENU menu2 = NULL;
    HMENU menu3 = NULL;

    int i = 0;

    if (trans_table == NULL) {
        return;
    }

    while (trans_table[i].level != 0) {
        switch (trans_table[i].level) {
            case 1:
                menu1 = NULL;
                while (menu1 == NULL) {
                    pos1++;
                    menu1 = GetSubMenu(menu, pos1);
                }
                if (trans_table[i].ids != 0) {
                    uilib_localize_menu_popup(menu, pos1, menu1, trans_table[i].ids);
                }
                pos2 = -1;
                pos3 = -1;
                break;
            case 2:
                menu2 = NULL;
                while (menu2 == NULL) {
                    pos2++;
                    menu2 = GetSubMenu(menu1, pos2);
                }
                uilib_localize_menu_popup(menu1, pos2, menu2, trans_table[i].ids);
                pos3 = -1;
                break;
            case 3:
                menu3 = NULL;
                while (menu3 == NULL) {
                    pos3++;
                    menu3 = GetSubMenu(menu2, pos3);
                }
                uilib_localize_menu_popup(menu2, pos3, menu3, trans_table[i].ids);
                break;
        }
        i++;
    }
}

/*****************************************************************************/
static char *shortcut_keys_txt =
    "The following shortcut keys are available:\n\n"
    "0\t- Restart the current tune\n"
    "<SPACE>\t- Pause/unpause the tune\n"
    "W\t- Fast forward the tune (while holding W)\n"
    "+\t- Sound volume up\n"
    "-\t- Sound volume down\n"
    "<LEFT>\t- Next tune\n"
    "<DOWN>\t- Next tune\n"
    "<RIGHT>\t- Previous tune\n"
    "<UP>\t- Previous tune\n";

/*****************************************************************************/
static void update_menus(HWND hwnd)
{
    unsigned int i, j;
    int value;
    int result;
    const char *lang;
    HMENU menu = GetMenu(hwnd);

    for (i = 0; toggle_list[i].name != NULL; i++) {
        resources_get_int(toggle_list[i].name, &value);
        CheckMenuItem(menu, toggle_list[i].item_id, value ? MF_CHECKED : MF_UNCHECKED);
    }

    for (i = 0; value_list[i].name != NULL; i++) {
        result = resources_get_int(value_list[i].name, &value);
        if (result == 0) {
            unsigned int checked = 0;

            CheckMenuItem(menu, value_list[i].default_item_id, MF_UNCHECKED);
            for (j = 0; value_list[i].vals[j].item_id != 0; j++) {
                if (value == value_list[i].vals[j].value && !checked) {
                    CheckMenuItem(menu, value_list[i].vals[j].item_id, MF_CHECKED);
                    checked = 1;
                } else {
                    CheckMenuItem(menu, value_list[i].vals[j].item_id, MF_UNCHECKED);
                }
            }
            if (checked == 0 && value_list[i].default_item_id > 0) {
                CheckMenuItem(menu, value_list[i].default_item_id, MF_CHECKED);
            }
        }
    }
    CheckMenuItem(menu, IDM_PAUSE, ui_emulation_is_paused() ? MF_CHECKED : MF_UNCHECKED);

    resources_get_string("Language", &lang);
    for (i = 0; (ui_lang_menu_entries[i].lang_code != NULL) && (i < countof(ui_lang_menu_entries)); i++) {
        if (strcmp(lang, ui_lang_menu_entries[i].lang_code) == 0) {
            CheckMenuItem(menu, ui_lang_menu_entries[i].item_id, MF_CHECKED);
        } else {
            CheckMenuItem(menu, ui_lang_menu_entries[i].item_id, MF_UNCHECKED);
        }
    }
}

static void ui_set_language(unsigned int lang_id)
{
    unsigned int i;

    for (i = 0; (ui_lang_menu_entries[i].lang_code != NULL) && (i < countof(ui_lang_menu_entries)); i++) {
        if (ui_lang_menu_entries[i].item_id == lang_id) {
            resources_set_string("Language", ui_lang_menu_entries[i].lang_code);
            break;
        }
    }
}

static void vsid_ui_translate(void)
{
    menu = LoadMenu(winmain_instance, MAKEINTRESOURCE(IDR_MENUVSID));
    ui_translate_menu_items(menu, vsidui_menu_translation_table);
    ui_translate_menu_popups(menu, vsidui_popup_translation_table);
    SetMenu(hwnd, menu);
}

int vsid_ui_init(void)
{
    wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wndclass.lpfnWndProc = window_proc;
    wndclass.cbClsExtra = 0 ;
    wndclass.cbWndExtra = 0 ;
    wndclass.hInstance = winmain_instance ;
    wndclass.hIcon = LoadIcon(winmain_instance, MAKEINTRESOURCE(IDI_ICON1));
    wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndclass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
    wndclass.lpszMenuName = MAKEINTRESOURCE(IDR_MENUVSID);
    wndclass.lpszClassName = st_AppName;

    RegisterClass(&wndclass);
    if (!hwnd) {   /* do not recreate on drag&drop */
        hwnd = CreateWindow(st_AppName, st_AppName, WS_SYSMENU, 0, 0, 380, 340, NULL, NULL, winmain_instance, NULL);
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        window_handles[0] = hwnd;
        number_of_windows++;
    } else {
        SetForegroundWindow(hwnd);
    }

    vsid_ui_translate();

    ShowWindow(hwnd, SW_SHOW);
    DragAcceptFiles(hwnd, TRUE);
    return 0;
}

void vsid_ui_display_name(const char *name)
{
    sprintf(vsidstrings[VSID_S_TITLE],  "Title:    %s", name);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_TITLE]);
}

void vsid_ui_display_author(const char *author)
{
    sprintf(vsidstrings[VSID_S_AUTHOR], "Author:   %s", author);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_AUTHOR]);
}

void vsid_ui_display_copyright(const char *copyright)
{
    sprintf(vsidstrings[VSID_S_RELEASED], "Released: %s", copyright);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_RELEASED]);
}

void vsid_ui_display_sync(int sync)
{
    sprintf(vsidstrings[VSID_S_SYNC], "Using %s sync ", sync == MACHINE_SYNC_PAL ? "PAL" : "NTSC");
    log_message(LOG_DEFAULT, "%s",vsidstrings[VSID_S_SYNC]);
}

void vsid_ui_display_sid_model(int model)
{
    sprintf(vsidstrings[VSID_S_MODEL], "Model: %s", model == 0 ? "MOS6581" : "MOS8580");

    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_MODEL]);
}

void vsid_ui_display_tune_nr(int nr)
{
    sprintf(vsidstrings[VSID_S_PLAYING], "Tune: %2d /  0  (Default: 00)", nr);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_PLAYING]);
}

void vsid_ui_set_default_tune(int nr)
{
    char dummy[4];
    sprintf(dummy,"%2d", nr);

    log_message(LOG_DEFAULT, "Default Tune: %i", nr);
    vsidstrings[VSID_S_PLAYING][25] = dummy[0];
    vsidstrings[VSID_S_PLAYING][26] = dummy[1];
}

void vsid_ui_display_nr_of_tunes(int count)
{
    char dummy[4];
    sprintf(dummy,"%2d", count);

    log_message(LOG_DEFAULT, "Number of Tunes: %i", count);
    vsidstrings[VSID_S_PLAYING][12] = dummy[0];
    vsidstrings[VSID_S_PLAYING][13] = dummy[1];
}

void vsid_ui_display_time(unsigned int sec)
{
    char dummy[] = "Time: %02d:%02d:%02d";
    unsigned int h,m,s;

    s = sec;
    h = sec / 3600;
    sec = sec - (h * 3600);
    m = sec / 60;
    sec = sec - (m * 60);
    sprintf(vsidstrings[VSID_S_TIMER], dummy, h, m, sec);
    vsid_disp(0, VSID_S_TIMER, "%s", vsidstrings[VSID_S_TIMER]);
    if (((h + m) == 0) && (s < 2)) {
        InvalidateRect(hwnd, NULL, FALSE);
    }
    UpdateWindow(hwnd);
}

void vsid_ui_display_irqtype(const char *irq)
{
    sprintf(vsidstrings[VSID_S_IRQ], "IRQ: %s", irq);
}

void vsid_ui_setdrv(char* driver_info_text)
{
    unsigned long val1, val2;
    char *s;

    /* Driver info: Driver=$xxxx, Image=$xxxx-$xxxx, Init=$xxxx, Play=$xxxx */
    s = driver_info_text;
    while ((*s != 0) && (*s != '$')) { s++; } s++; /* forward behind next $ */
    val1 = strtoul(s, NULL, 16);
    sprintf(vsidstrings[VSID_S_DRIVER], "Driver: $%04lx", val1);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_DRIVER]);
    val1 = strtoul(s + 13, NULL, 16);
    val2 = strtoul(s + 19, NULL, 16);
    sprintf(vsidstrings[VSID_S_IMAGE], "Image:  $%04lx-$%04lx", val1, val2);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_IMAGE]);
    val1 = strtoul(s + 31, NULL, 16);
    sprintf(vsidstrings[VSID_S_INIT], "Init:   $%04lx", val1);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_INIT]);
    val1 = strtoul(s + 43, NULL, 16);
    sprintf(vsidstrings[VSID_S_PLAY], "Play:   $%04lx", val1);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_PLAY]);
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

static int quitting = 0;

void vsid_ui_close(void)
{
    int quit = 1;
    int confirm_on_exit, save_on_exit;

    resources_get_int("ConfirmOnExit", &confirm_on_exit);
    resources_get_int("SaveResourcesOnExit", &save_on_exit);

    if (!quitting && confirm_on_exit) {
        if (MessageBox(hwnd, intl_translate_tcs(IDS_REALLY_EXIT), TEXT("VICE"), MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2 | MB_TASKMODAL) == IDYES) {
            quit = 1;
        } else {
            quit = 0;
        }
    }

    if (!quitting && quit) {
        if (save_on_exit) {
            if (resources_save(NULL) < 0) {
                ui_error(translate_text(IDS_CANNOT_SAVE_SETTINGS));
            }
        }
        DestroyWindow(hwnd);
        hwnd = NULL;
        quitting = 1;
    }
}

/*****************************************************************************/

static uilib_localize_dialog_param vsid_dialog_trans[] = {
    { IDC_VSID_TUNE_LABEL, IDS_TUNE_NR, 0 },
    { IDOK, IDS_OK, 0 },
    { IDCANCEL, IDS_CANCEL, 0 },
    { 0, 0, 0 }
};

static uilib_dialog_group vsid_left_group[] = {
    { IDC_VSID_TUNE_LABEL, 0 },
    { 0, 0 }
};

static uilib_dialog_group vsid_right_group[] = {
    { IDC_VSID_TUNE, 0 },
    { 0, 0 }
};

static int move_buttons_group[] = {
    IDOK,
    IDCANCEL,
    0
};

static void init_vsid_dialog(HWND hwnd)
{
    HWND temp_hwnd;
    int xpos;
    int i;
    TCHAR st[10];
    RECT rect;

    /* translate all dialog items */
    uilib_localize_dialog(hwnd, vsid_dialog_trans);

    /* adjust the size of the elements in the left group */
    uilib_adjust_group_width(hwnd, vsid_left_group);

    /* get the max x of the left group */
    uilib_get_group_max_x(hwnd, vsid_left_group, &xpos);

    /* move the right group to the correct position */
    uilib_move_group(hwnd, vsid_right_group, xpos + 10);

    /* get the max x of the right group */
    uilib_get_group_max_x(hwnd, vsid_right_group, &xpos);

    /* set the width of the dialog to 'surround' all the elements */
    GetWindowRect(hwnd, &rect);
    MoveWindow(hwnd, rect.left, rect.top, xpos + 20, rect.bottom - rect.top, TRUE);

    /* recenter the buttons in the newly resized dialog window */
    uilib_center_buttons(hwnd, move_buttons_group, 0);

    temp_hwnd = GetDlgItem(hwnd, IDC_VSID_TUNE);
    for (i = 0; i < songs; i++) {
        _itot(i + 1, st, 10);
        SendMessage(temp_hwnd, CB_ADDSTRING, 0, (LPARAM)st);
    }
    SendMessage(temp_hwnd, CB_SETCURSEL, (WPARAM)current_song - 1, 0);
}

static void end_vsid_dialog(HWND hwnd)
{
    int temp = (int)SendMessage(GetDlgItem(hwnd, IDC_VSID_TUNE), CB_GETCURSEL, 0, 0) + 1;

    if (temp != current_song) {
        current_song = temp;
        psid_ui_set_tune(current_song, NULL);
        vsid_ui_display_tune_nr(current_song);
        vsid_ui_set_default_tune(default_song);
        vsid_ui_display_nr_of_tunes(songs);
    }
}

static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    int command;

    switch (msg) {
        case WM_COMMAND:
            command = LOWORD(wparam);
            switch (command) {
                case IDOK:
                    end_vsid_dialog(hwnd);
                case IDCANCEL:
                    EndDialog(hwnd, 0);
                    return TRUE;
            }
            return FALSE;
        case WM_CLOSE:
            EndDialog(hwnd, 0);
            return TRUE;
        case WM_INITDIALOG:
            init_vsid_dialog(hwnd);
            return TRUE;
    }
    return FALSE;
}

static void ui_select_vsid_tune(HWND hwnd)
{
    DialogBox(winmain_instance, (LPCTSTR)(UINT_PTR)IDD_VSID_TUNE_DIALOG, hwnd, dialog_proc);
}

/*****************************************************************************/

static void load_psid_file(HWND window, char *name)
{
    int i;

    if (machine_autodetect_psid(name) >= 0) {
        vsid_disp(0, 0,  NULL, NULL);
        psid_init_driver();
        vsid_ui_init();
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
        InvalidateRect(window, NULL, TRUE);
    }
}

static void handle_default_command(WPARAM wparam, LPARAM lparam, HWND hwnd)
{
    int i, j, command_found = 0;

    for (i = 0; toggle_list[i].name != NULL && !command_found; i++) {
        if (toggle_list[i].item_id == wparam) {
            resources_toggle(toggle_list[i].name, NULL);
            command_found = 1;
        }
    }

    for (i = 0; value_list[i].name != NULL && !command_found; i++) {
        for (j = 0; value_list[i].vals[j].item_id != 0 && !command_found; j++) {
            if (value_list[i].vals[j].item_id == wparam) {
                resources_set_int(value_list[i].name, value_list[i].vals[j].value);
                command_found = 1;
            }
        }
    }
}

static void handle_wm_command(WPARAM wparam, LPARAM lparam, HWND hwnd)
{
    char *name = NULL;

    switch (wparam) {
        case IDM_LOAD_PSID_FILE:
            name = uilib_select_file(hwnd, intl_translate_tcs(IDS_PSID_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_LOAD, UILIB_SELECTOR_STYLE_DEFAULT);
            if (name != NULL) {
                load_psid_file(hwnd, name);
                lib_free(name);
            }
            break;
        case IDM_SELECT_TUNE:
            ui_select_vsid_tune(hwnd);
            break;
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
        case IDM_PAUSE:
            ui_pause_emulation(!ui_emulation_is_paused());
            break;
        case IDM_RESET_HARD:
            machine_trigger_reset(MACHINE_RESET_MODE_HARD);
            break;
        case IDM_RESET_SOFT:
            machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
            break;
        case IDM_MONITOR:
            monitor_startup_trap();
            break;
        case IDM_MAXIMUM_SPEED_CUSTOM:
            ui_speed_settings_dialog(hwnd);
            break;
        case IDM_EXIT:
            PostMessage(hwnd, WM_CLOSE, wparam, lparam);
            break;
        case IDM_SOUND_RECORD_START:
            ui_sound_record_settings_dialog(hwnd);
            break;
        case IDM_SOUND_RECORD_STOP:
            resources_set_string("SoundRecordDeviceName", "");
            break;
        case IDM_SOUND_SETTINGS:
            ui_sound_settings_dialog(hwnd);
            break;
        case IDM_SID_SETTINGS:
            ui_sid_settings_dialog(hwnd, c64_sid_baseaddress);
            break;
        case IDM_SETTINGS_SAVE_FILE:
            if ((name = uilib_select_file(hwnd, intl_translate_tcs(IDS_SAVE_CONFIG_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_SAVE, UILIB_SELECTOR_STYLE_DEFAULT)) != NULL) {
                if (resources_save(name) < 0) {
                    ui_error(translate_text(IDS_CANNOT_SAVE_SETTINGS));
                } else {
                    ui_message(translate_text(IDS_SETTINGS_SAVED_SUCCESS));
                }
                lib_free(name);
            }
            break;
        case IDM_SETTINGS_LOAD_FILE:
            if ((name = uilib_select_file(hwnd, intl_translate_tcs(IDS_LOAD_CONFIG_FILE), UILIB_FILTER_ALL, UILIB_SELECTOR_TYPE_FILE_LOAD, UILIB_SELECTOR_STYLE_DEFAULT)) != NULL) {
                if (resources_load(name) < 0) {
                    ui_error(translate_text(IDS_CANNOT_LOAD_SETTINGS));
                } else {
                    ui_message(translate_text(IDS_SETTINGS_LOADED_SUCCESS));
                }
                lib_free(name);
            }
            break;
        case IDM_SETTINGS_SAVE:
            if (resources_save(NULL) < 0) {
                ui_error(translate_text(IDS_CANNOT_SAVE_SETTINGS));
            } else {
                ui_message(translate_text(IDS_SETTINGS_SAVED_SUCCESS));
            }
            break;
        case IDM_SETTINGS_LOAD:
            if (resources_load(NULL) < 0) {
                ui_error(translate_text(IDS_CANNOT_LOAD_SETTINGS));
            } else {
                ui_message(translate_text(IDS_SETTINGS_LOADED_SUCCESS));
            }
            break;
        case IDM_SETTINGS_DEFAULT:
            resources_set_defaults();
            ui_message(translate_text(IDS_DEFAULT_SETTINGS_RESTORED));
            break;
        case IDM_LANG_EN:
        case IDM_LANG_DA:
        case IDM_LANG_DE:
        case IDM_LANG_ES:
        case IDM_LANG_FR:
        case IDM_LANG_HU:
        case IDM_LANG_IT:
        case IDM_LANG_KO:
        case IDM_LANG_NL:
        case IDM_LANG_PL:
        case IDM_LANG_RU:
        case IDM_LANG_SV:
        case IDM_LANG_TR:
            ui_set_language((unsigned int)wparam);
            vsid_ui_translate();
            break;
        case IDM_ABOUT:
        case IDM_HELP:
        case IDM_CONTRIBUTORS:
        case IDM_LICENSE:
        case IDM_WARRANTY:
        case IDM_CMDLINE:
        case IDM_FEATURES:
            uihelp_dialog(hwnd, wparam);
            break;
        case IDM_KEYS:
            ui_show_text(hwnd, IDS_VSID_KEYS, intl_translate_tcs(IDS_VSID_KEYS_AVAILABLE), shortcut_keys_txt);
            break;
        default:
            handle_default_command(wparam, lparam, hwnd);
    }
}

static void handle_wm_dropfiles(HWND window, HDROP hDrop)
{
    TCHAR st_name[MAX_PATH];
    char *name;

    DragQueryFile(hDrop, 0, st_name, MAX_PATH);
    name = system_wcstombs_alloc(st_name);
    load_psid_file(window, name);
    system_wcstombs_free(name);
    DragFinish(hDrop);
}

/* Window procedure.  All messages are handled here.  */
static LRESULT CALLBACK window_proc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
    PAINTSTRUCT ps;
    int i;
    int vol;

    switch (msg) {
        case WM_CREATE:
            songs = psid_tunes(&default_song);
            current_song = default_song;
#if 0
            if (songs == 0) {
                log_message(LOG_DEFAULT, "Vsid: no file specified, quitting");
                return -1;
            }
#endif
            DragAcceptFiles(window, TRUE);
            return 0;
        case WM_KEYDOWN:
            switch(wparam) {
                case '0':
                    psid_ui_set_tune(current_song, NULL);
                    break;
                case ' ':
                    ui_pause_emulation(!ui_emulation_is_paused());
                    break;
                case 'W':
                    resources_set_int("WarpMode", 1);
                    break;
                case VK_ADD:
                    resources_get_int("SoundVolume", &vol);
                    ++vol;
                    if (vol > 100) {
                        vol = 100;
                    }
                    resources_set_int("SoundVolume", vol);
                    break;
                case VK_SUBTRACT:
                    resources_get_int("SoundVolume", &vol);
                    --vol;
                    if (vol < 0) {
                        vol = 0;
                    }
                    resources_set_int("SoundVolume", vol);
                    break;
                case VK_LEFT:
                case VK_DOWN:
                    if (current_song > 1) {
                        current_song--;
                        psid_ui_set_tune(current_song, NULL);
                        vsid_ui_display_tune_nr(current_song);
                        vsid_ui_set_default_tune(default_song);
                        vsid_ui_display_nr_of_tunes(songs);
                        InvalidateRect(window, NULL, 0);
                    }
                    break;
                case VK_RIGHT:
                case VK_UP:
                    if (current_song < songs) {
                        current_song++;
                        psid_ui_set_tune(current_song, NULL);
                        vsid_ui_display_tune_nr(current_song);
                        vsid_ui_set_default_tune(default_song);
                        vsid_ui_display_nr_of_tunes(songs);
                        InvalidateRect(window, NULL, 0);
                    }
                    break;
            }
            return 0;
        case WM_KEYUP:
             switch(wparam) {
                case 'W':
                    resources_set_int("WarpMode", 0);
                    break;
             }
            return 0;
        case WM_SIZE:
            return 0;
        case WM_COMMAND:
            vsync_suspend_speed_eval();
            handle_wm_command(wparam, lparam, window);
            return 0;
        case WM_ENTERMENULOOP:
            vsync_suspend_speed_eval();
            update_menus(window);
            break;
        case WM_MOVE:
            break;
        case WM_CLOSE:
            vsync_suspend_speed_eval();
            vsid_ui_close();
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_DROPFILES:
            handle_wm_dropfiles(window, (HDROP)wparam);
            return 0;
        case WM_PAINT:
            {
                BeginPaint(window, &ps);
                if (*vsidstrings[VSID_S_TIMER]) {    /* start only when timer string has been filled */
                    for (i = 0; i < VSID_S_LASTLINE; i++) {
                        vsid_disp(0, i, "%s", vsidstrings[i]);
                    }
                }

                EndPaint(window, &ps);
                return 0;
            }
    }

    return DefWindowProc(window, msg, wparam, lparam);
}
