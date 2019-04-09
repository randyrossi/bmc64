/*
 * vsidui.c - Implementation of the VSID UI.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Emiliano 'iAN CooG' Peruch <iancoog@email.it>
 *  Dag Lem <resid@nimrod.no>
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
#include <stdlib.h>

#include "debug.h"
#include "c64mem.h"
#include "lib.h"
#include "log.h"
#include "machine.h"
#include "menu_common.h"
#include "menu_debug.h"
#include "menu_help.h"
#include "menu_jam.h"
#include "menu_monitor.h"
#include "menu_reset.h"
#include "menu_settings.h"
#include "menu_sid.h"
#include "menu_sound.h"
#include "menu_speed.h"
#include "psid.h"
#include "ui.h"
#include "uifonts.h"
#include "uifilereq.h"
#include "uimenu.h"
#include "videoarch.h"
#include "vsidui.h"
#include "vsidui_sdl.h"

/* ---------------------------------------------------------------------*/
/* static variables / functions */

static int sdl_vsid_tunes = 0;
static int sdl_vsid_current_tune = 0;
static int sdl_vsid_default_tune = 0;

enum {
    VSID_CS_TITLE = 0,
    VSID_S_TITLE,
    VSID_CS_AUTHOR,
    VSID_S_AUTHOR,
    VSID_CS_RELEASED,
    VSID_S_RELEASED,
    VSID_S_SYNC,
    VSID_S_MODEL,
    VSID_S_IRQ,
    VSID_S_PLAYING,
    VSID_S_TUNES,
    VSID_S_DEFAULT,
    VSID_S_TIMER,
    VSID_S_INFO_DRIVER,
    VSID_S_INFO_IMAGE,
    VSID_S_INFO_INIT_PLAY,
    VSID_S_NUM
};

static char vsidstrings[VSID_S_NUM][41] = {{0}};

/* ---------------------------------------------------------------------*/
/* menu */

static UI_MENU_CALLBACK(load_psid_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose PSID file", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            if (machine_autodetect_psid(name) < 0) {
                ui_error("Could not load PSID file");
            }
            lib_free(name);
            psid_init_driver();
            machine_play_psid(0);
            machine_trigger_reset(MACHINE_RESET_MODE_SOFT);
            return sdl_menu_text_exit_ui;
        }
    }
    return NULL;
}

#define SDLUI_VSID_CMD_MASK     0x8000
#define SDLUI_VSID_CMD_NEXT     0x8001
#define SDLUI_VSID_CMD_PREV     0x8002
#define SDLUI_VSID_CMD_DEFAULT  0x8003

static UI_MENU_CALLBACK(vsidui_tune_callback)
{
    int command_or_tune = vice_ptr_to_int(param);

    if (activated) {
        int tune = sdl_vsid_current_tune;

        if (command_or_tune == SDLUI_VSID_CMD_NEXT) {
            ++tune;
        } else if (command_or_tune == SDLUI_VSID_CMD_PREV) {
            --tune;
        } else if (command_or_tune == SDLUI_VSID_CMD_DEFAULT) {
            tune = sdl_vsid_default_tune;
        } else {
            tune = command_or_tune;
        }

        if ((tune < 1) || (tune > sdl_vsid_tunes)) {
            return NULL;
        }

        if (tune != sdl_vsid_current_tune) {
            sdl_vsid_current_tune = tune;
            sdl_ui_menu_radio_helper(1, (ui_callback_data_t)int_to_void_ptr(tune), "PSIDTune");
        }
    } else {
        if (command_or_tune == sdl_vsid_current_tune) {
            return sdl_menu_text_tick;
        } else if (command_or_tune > sdl_vsid_tunes && !(command_or_tune & SDLUI_VSID_CMD_MASK)) {
            return MENU_NOT_AVAILABLE_STRING;
        }
    }
    return NULL;
}

/* This menu is static so hotkeys can be assigned.
   Only 23 tunes are listed, which is hopefully enough for most cases. */

/* FIXME: still generate this menu at runtime, there ARE .sid files with 256 tunes! */
static const ui_menu_entry_t vsid_tune_menu[] = {
    { "Tune 1",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)1 },
    { "Tune 2",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)2 },
    { "Tune 3",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)3 },
    { "Tune 4",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)4 },
    { "Tune 5",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)5 },
    { "Tune 6",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)6 },
    { "Tune 7",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)7 },
    { "Tune 8",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)8 },
    { "Tune 9",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)9 },
    { "Tune 10",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)10 },
    { "Tune 11",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)11 },
    { "Tune 12",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)12 },
    { "Tune 13",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)13 },
    { "Tune 14",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)14 },
    { "Tune 15",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)15 },
    { "Tune 16",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)16 },
    { "Tune 17",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)17 },
    { "Tune 18",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)18 },
    { "Tune 19",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)19 },
    { "Tune 20",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)20 },
    { "Tune 21",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)21 },
    { "Tune 22",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)22 },
    { "Tune 23",
      MENU_ENTRY_OTHER_TOGGLE,
      vsidui_tune_callback,
      (ui_callback_data_t)23 },
    SDL_MENU_LIST_END
};

UI_MENU_DEFINE_TOGGLE(PSIDKeepEnv)

static const ui_menu_entry_t vsid_main_menu[] = {
    { "Load PSID file",
      MENU_ENTRY_DIALOG,
      load_psid_callback,
      NULL },
    { "Select tune",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)vsid_tune_menu },
    { "Next tune",
      MENU_ENTRY_OTHER,
      vsidui_tune_callback,
      (ui_callback_data_t)SDLUI_VSID_CMD_NEXT },
    { "Previous tune",
      MENU_ENTRY_OTHER,
      vsidui_tune_callback,
      (ui_callback_data_t)SDLUI_VSID_CMD_PREV },
    { "Default tune",
      MENU_ENTRY_OTHER,
      vsidui_tune_callback,
      (ui_callback_data_t)SDLUI_VSID_CMD_DEFAULT },
    { "Override PSID settings",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_PSIDKeepEnv_callback,
      NULL },
    { "SID settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_c64_menu },
    { "Sound settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sound_output_menu },
    { "Reset",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)reset_menu },
    { "Action on CPU JAM",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)jam_menu },
    { "Speed settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)speed_menu_vsid },
    { "Pause",
      MENU_ENTRY_OTHER_TOGGLE,
      pause_callback,
      NULL },
    { "Monitor",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)monitor_menu },
#ifdef DEBUG
    { "Debug",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)debug_menu },
#endif
    { "Help",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)help_menu },
    { "Settings management",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)settings_manager_menu_vsid },
    { "Quit emulator",
      MENU_ENTRY_OTHER,
      quit_callback,
      NULL },
    SDL_MENU_LIST_END
};


/* ---------------------------------------------------------------------*/
/* vsidui_sdl.h draw func */

static void draw_func(void)
{
    int i, n;

    for (n = i = 0; i < (int)VSID_S_NUM; ++i, ++n) {
        sdl_ui_print(vsidstrings[i], 0, n);
        if ((i == 5) || (i == 8) || (i == 11) || (i == 12)) {
            ++n;
        }
    }
}

/* ---------------------------------------------------------------------*/
/* vsidui.h */
static void vsid_set_menu_params(int index, menu_draw_t *menu_draw)
{
    /* VICII */
    menu_draw->max_text_x = 40;
    menu_draw->color_front = menu_draw->color_default_front = 1;
    menu_draw->color_back = menu_draw->color_default_back = 0;
    menu_draw->color_cursor_back = 6;
    menu_draw->color_cursor_revers = 0;
    menu_draw->color_active_green = 13;
    menu_draw->color_inactive_red = 2;
    menu_draw->color_active_grey = 15;
    menu_draw->color_inactive_grey = 11;

    sdl_ui_set_menu_params = NULL;
}

/** \brief  Pre-initialize the UI before the canvas window gets created
 *
 * \return  0 on success, -1 on failure
 */
int vsid_ui_init_early(void)
{
    return 0;
}

/** \brief  Initialize the UI
 *
 * \return  0 on success, -1 on failure
 */
int vsid_ui_init(void)
{
    unsigned int width;
    unsigned int height;

    /* set function pointers to handle drag-n-drop of SID files */
    sdl_vsid_set_init_func(psid_init_driver);
    sdl_vsid_set_play_func(machine_play_psid);

    sdl_ui_set_menu_params = vsid_set_menu_params;
    uikeyboard_menu_create();
    uisid_menu_create();

    sdl_ui_set_main_menu(vsid_main_menu);
    sdl_ui_vicii_font_init();

    sdl_vsid_draw_init(draw_func);
    sdl_vsid_activate();

    sprintf(vsidstrings[VSID_CS_TITLE], "Title:");
    sprintf(vsidstrings[VSID_CS_AUTHOR], "Author:");
    sprintf(vsidstrings[VSID_CS_RELEASED], "Released:");

    sdl_ui_init_draw_params();

    width = sdl_active_canvas->draw_buffer->draw_buffer_width;
    height = sdl_active_canvas->draw_buffer->draw_buffer_height;
    sdl_active_canvas->draw_buffer_vsid = lib_calloc(1, sizeof(draw_buffer_t));
    sdl_active_canvas->draw_buffer_vsid->draw_buffer = lib_malloc(width * height);

    draw_buffer_vsid = sdl_active_canvas->draw_buffer_vsid->draw_buffer;

    memset(sdl_active_canvas->draw_buffer_vsid->draw_buffer, 0, width * height);

    return 0;
}

void vsid_ui_display_name(const char *name)
{
    strncpy(vsidstrings[VSID_S_TITLE], name, 40);
    log_message(LOG_DEFAULT, "Title: %s", vsidstrings[VSID_S_TITLE]);
}

void vsid_ui_display_author(const char *author)
{
    strncpy(vsidstrings[VSID_S_AUTHOR], author, 40);
    log_message(LOG_DEFAULT, "Author: %s", vsidstrings[VSID_S_AUTHOR]);
}

void vsid_ui_display_copyright(const char *copyright)
{
    strncpy(vsidstrings[VSID_S_RELEASED], copyright, 40);
    log_message(LOG_DEFAULT, "Released: %s", vsidstrings[VSID_S_RELEASED]);
}

void vsid_ui_display_sync(int sync)
{
    sprintf(vsidstrings[VSID_S_SYNC], "Using %s sync", sync == MACHINE_SYNC_PAL ? "PAL" : "NTSC");
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_SYNC]);
}

void vsid_ui_display_sid_model(int model)
{
    sprintf(vsidstrings[VSID_S_MODEL], "Using %s emulation", model == 0 ? "MOS6581" : "MOS8580");
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_MODEL]);
}

void vsid_ui_set_default_tune(int nr)
{
    sprintf(vsidstrings[VSID_S_DEFAULT], "Default tune: %d", nr);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_DEFAULT]);
    sdl_vsid_default_tune = nr;
}

void vsid_ui_display_tune_nr(int nr)
{
    sprintf(vsidstrings[VSID_S_PLAYING], "Playing tune: %-3d", nr);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_PLAYING]);
    sdl_vsid_current_tune = nr;

    if (sdl_vsid_state & SDL_VSID_ACTIVE) {
        sdl_vsid_state |= SDL_VSID_REPAINT;
    }
}

void vsid_ui_display_nr_of_tunes(int count)
{
    sprintf(vsidstrings[VSID_S_TUNES], "Number of tunes: %d", count);
    log_message(LOG_DEFAULT, "%s", vsidstrings[VSID_S_TUNES]);
    sdl_vsid_tunes = count;
}

void vsid_ui_display_time(unsigned int sec)
{
    unsigned int h, m;

    h = sec / 3600;
    sec = sec - (h * 3600);
    m = sec / 60;
    sec = sec - (m * 60);
    h = h % 100;
    sprintf(vsidstrings[VSID_S_TIMER], "%02d:%02d:%02d", h, m, sec);

    if (sdl_vsid_state & SDL_VSID_ACTIVE) {
        sdl_vsid_state |= SDL_VSID_REPAINT;
    }
}

void vsid_ui_display_irqtype(const char *irq)
{
    sprintf(vsidstrings[VSID_S_IRQ], "Using %s interrupt", irq);
}

void vsid_ui_setdrv(char* driver_info_text)
{
    /* FIXME magic values */
    strncpy(vsidstrings[VSID_S_INFO_DRIVER], &(driver_info_text[0]), 12);
    strncpy(vsidstrings[VSID_S_INFO_IMAGE], &(driver_info_text[14]), 17);
    strncpy(vsidstrings[VSID_S_INFO_INIT_PLAY], &(driver_info_text[33]), 40);
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


void vsid_ui_close(void)
{
    uikeyboard_menu_shutdown();
    uisid_menu_shutdown();
    sdl_ui_vicii_font_shutdown();
}
