/** \file   autostart.
 * \brief   Automatic image loading and starting.
 *
 * \author  Teemu Rantanen <tvr@cs.hut.fi>
 * \author  Ettore Perazzoli <ettore@comm2000.it>
 * \author  Andre Fachat <a.fachat@physik.tu-chemnitz.de>
 * \author  Andreas Boose <viceteam@t-online.de>
 * \author  Thomas Bretz <tbretz@ph.tum.de>
 */

/*
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

/* #define DEBUG_AUTOSTART */

#include "vice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "archdep.h"
#include "autostart.h"
#include "autostart-prg.h"
#include "attach.h"
#include "cartridge.h"
#include "charset.h"
#include "cmdline.h"
#include "datasette.h"
#include "diskimage.h"
#include "drive.h"
#include "fileio.h"
#include "fsdevice.h"
#include "imagecontents.h"
#include "tapecontents.h"
#include "diskcontents.h"
#include "initcmdline.h"
#include "interrupt.h"
#include "ioutil.h"
#include "kbdbuf.h"
#include "lib.h"
#include "log.h"
#include "machine-bus.h"
#include "machine.h"
#include "maincpu.h"
#include "mem.h"
#include "monitor.h"
#include "network.h"
#include "resources.h"
#include "snapshot.h"
#include "tape.h"
#include "types.h"
#include "uiapi.h"
#include "util.h"
#include "vdrive.h"
#include "vdrive-bam.h"
#include "vice-event.h"

#ifdef DEBUG_AUTOSTART
#define DBG(_x_)        log_debug _x_
#else
#define DBG(_x_)
#endif

static void autostart_done(void);
static void autostart_finish(void);

/* Kernal addresses.  Set by `autostart_init()'.  */

static uint16_t blnsw;           /* Cursor Blink enable: 0 = Flash Cursor */
static int pnt;                 /* Pointer: Current Screen Line Address */
static int pntr;                /* Cursor Column on Current Line */
static int lnmx;                /* Physical Screen Line Length */

/* Current state of the autostart routine.  */
static enum {
    AUTOSTART_NONE,
    AUTOSTART_ERROR,
    AUTOSTART_HASTAPE,
    AUTOSTART_PRESSPLAYONTAPE,
    AUTOSTART_LOADINGTAPE,
    AUTOSTART_HASDISK,
    AUTOSTART_LOADINGDISK,
    AUTOSTART_HASSNAPSHOT,
    AUTOSTART_WAITLOADREADY,
    AUTOSTART_WAITLOADING,
    AUTOSTART_WAITSEARCHINGFOR,
    AUTOSTART_INJECT,
    AUTOSTART_DONE
} autostartmode = AUTOSTART_NONE;

#define AUTOSTART_WAIT_BLINK   0
#define AUTOSTART_NOWAIT_BLINK 1

/* Log descriptor.  */
log_t autostart_log = LOG_ERR;

/* Flag: was true drive emulation turned on when we started booting the disk image?  */
static int orig_drive_true_emulation_state = -1;

/* Flag: were device traps turned on when we started booting the disk image?  */
static int orig_device_traps_state = -1;

/* Flag: warp mode state before booting */
static int orig_warp_mode = -1;

/* PETSCII name of the program to load. NULL if default */
static char *autostart_program_name = NULL;

/* Minimum number of cycles before we feed BASIC with commands.  */
static CLOCK min_cycles;
static CLOCK autostart_initial_delay_cycles;

/* Flag: Do we want to switch true drive emulation on/off during autostart?
 * Normally, this is the same as handle_drive_true_emulation_by_machine;
 * however, the user can override this decision by specifying
 * -autostart-no-true-drive-emulation
 */
static int handle_drive_true_emulation_overridden;

/* Flag: Does the machine want us to switch true drive emulation on/off during autostart? */
static int handle_drive_true_emulation_by_machine;

/* Flag: autostart is initialized.  */
static int autostart_enabled = 0;

/* Flag: Autostart the file or just load it?  */
static unsigned int autostart_run_mode;

/* Flag: maincpu_clk isn't resetted yet */
static int autostart_wait_for_reset;

/* Flag: load stage after LOADING enters ROM area */
static int entered_rom = 0;

/* Flag: trap monitor after done */
static int trigger_monitor = 0;

int autostart_ignore_reset = 0; /* FIXME: only used by datasette.c, does it really have to be global? */

/* flag for special case handling of C128 80 columns mode */
static int c128_column4080_key;

/* ------------------------------------------------------------------------- */

int autostart_basic_load = 0;

static int AutostartRunWithColon = 0;

static int AutostartHandleTrueDriveEmulation = 0;

static int AutostartWarp = 0;

static int AutostartDelay = 0;
static int AutostartDelayRandom = 0;

static int AutostartPrgMode = AUTOSTART_PRG_MODE_VFS;

static char *AutostartPrgDiskImage = NULL;

static const char * const AutostartRunCommandsAvailable[] = {
    "RUN\r", "RUN:\r"
};

static const char * AutostartRunCommand = NULL;

static void set_handle_true_drive_emulation_state(void)
{
    handle_drive_true_emulation_overridden =
        AutostartHandleTrueDriveEmulation ?
        handle_drive_true_emulation_by_machine : 0;
}

/*! \internal \brief set if autostart should use LOAD ... ,1 */
static int set_autostart_basic_load(int val, void *param)
{
    autostart_basic_load = val ? 1 : 0;

    return 0;
}

/*! \internal \brief set if autostart should execute with a colon or not

 \param val
   if 0, the "RUN" command at the end of autostart is executed without
   a colon; else, it will be executed with a colon.

 \param param
   unused

 \return
   0 on success. else -1.
*/
static int set_autostart_run_with_colon(int val, void *param)
{
    AutostartRunWithColon = val ? 1 : 0;

    AutostartRunCommand = AutostartRunCommandsAvailable[AutostartRunWithColon];

    return 0;
}

/*! \internal \brief set if autostart should handle TDE or not

 \param val
   if 0, autostart does not handle TDE even if the machine says it can
   handle it.

 \param param
   unused

 \return
   0 on success. else -1.
*/
static int set_autostart_handle_tde(int val, void *param)
{
    AutostartHandleTrueDriveEmulation = val ? 1 : 0;

    set_handle_true_drive_emulation_state();

    return 0;
}

/*! \internal \brief set if autostart should enable warp mode */
static int set_autostart_warp(int val, void *param)
{
    AutostartWarp = val ? 1 : 0;

    return 0;
}

/*! \internal \brief set initial autostart delay. 0 means default. */
static int set_autostart_delay(int val, void *param)
{
    if ((val < 0) || (val > 1000)) {
        val = 0;
    }
    AutostartDelay = val;
    return 0;
}

/*! \internal \brief set initial autostart random delay. 0 means off, 1 means on. */
static int set_autostart_delayrandom(int val, void *param)
{
    AutostartDelayRandom = val ? 1 : 0;
    return 0;
}

/*! \internal \brief set autostart prg mode */
static int set_autostart_prg_mode(int val, void *param)
{
    if ((val < 0) || (val > AUTOSTART_PRG_MODE_LAST)) {
        val = AUTOSTART_PRG_MODE_DEFAULT;
    }
    AutostartPrgMode = val;

    return 0;
}

/*! \internal \brief set disk image name of autostart prg mode */

static int set_autostart_prg_disk_image(const char *val, void *param)
{
    if (util_string_set(&AutostartPrgDiskImage, val)) {
        return 0;
    }

    return 0;
}

/*! \brief string resources used by autostart */
static resource_string_t resources_string[] = {
    { "AutostartPrgDiskImage", NULL, RES_EVENT_NO, NULL,
      &AutostartPrgDiskImage, set_autostart_prg_disk_image, NULL },
    RESOURCE_STRING_LIST_END
};

/*! \brief integer resources used by autostart */
static const resource_int_t resources_int[] = {
    { "AutostartBasicLoad", 0, RES_EVENT_NO, (resource_value_t)0,
      &autostart_basic_load, set_autostart_basic_load, NULL },
    { "AutostartRunWithColon", 0, RES_EVENT_NO, (resource_value_t)0,
      &AutostartRunWithColon, set_autostart_run_with_colon, NULL },
    { "AutostartHandleTrueDriveEmulation", 0, RES_EVENT_NO, (resource_value_t)0,
      &AutostartHandleTrueDriveEmulation, set_autostart_handle_tde, NULL },
    { "AutostartWarp", 1, RES_EVENT_NO, (resource_value_t)0,
      &AutostartWarp, set_autostart_warp, NULL },
    { "AutostartPrgMode", AUTOSTART_PRG_MODE_DEFAULT, RES_EVENT_NO, (resource_value_t)0,
      &AutostartPrgMode, set_autostart_prg_mode, NULL },
    { "AutostartDelay", 0, RES_EVENT_NO, (resource_value_t)0,
      &AutostartDelay, set_autostart_delay, NULL },
    { "AutostartDelayRandom", 1, RES_EVENT_NO, (resource_value_t)0,
      &AutostartDelayRandom, set_autostart_delayrandom, NULL },
    RESOURCE_INT_LIST_END
};

/*! \brief initialize the resources
 \return
   0 on success, else -1.

 \remark
   Registers the integer resources
*/
int autostart_resources_init(void)
{
    resources_string[0].factory_value = archdep_default_autostart_disk_image_file_name();

    if (resources_register_string(resources_string) < 0) {
        return -1;
    }

    return resources_register_int(resources_int);
}

void autostart_resources_shutdown(void)
{
    lib_free(AutostartPrgDiskImage);
    lib_free(resources_string[0].factory_value);
}

/* ------------------------------------------------------------------------- */

static const cmdline_option_t cmdline_options[] =
{
    { "-basicload", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "AutostartBasicLoad", (resource_value_t)1,
      NULL, "On autostart, load to BASIC start (without ',1')" },
    { "+basicload", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "AutostartBasicLoad", (resource_value_t)0,
      NULL, "On autostart, load with ',1'" },
    { "-autostartwithcolon", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "AutostartRunWithColon", (resource_value_t)1,
      NULL, "On autostart, use the 'RUN' command with a colon, i.e., 'RUN:'" },
    { "+autostartwithcolon", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "AutostartRunWithColon", (resource_value_t)0,
      NULL, "On autostart, do not use the 'RUN' command with a colon; i.e., 'RUN'" },
    { "-autostart-handle-tde", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "AutostartHandleTrueDriveEmulation", (resource_value_t)1,
      NULL, "Handle True Drive Emulation on autostart" },
    { "+autostart-handle-tde", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "AutostartHandleTrueDriveEmulation", (resource_value_t)0,
      NULL, "Do not handle True Drive Emulation on autostart" },
    { "-autostart-warp", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "AutostartWarp", (resource_value_t)1,
      NULL, "Enable warp mode during autostart" },
    { "+autostart-warp", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "AutostartWarp", (resource_value_t)0,
      NULL, "Disable warp mode during autostart" },
    { "-autostartprgmode", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "AutostartPrgMode", NULL,
      "<Mode>", "Set autostart mode for PRG files (0: VirtualFS, 1: Inject, 2: Disk image)" },
    { "-autostartprgdiskimage", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "AutostartPrgDiskImage", NULL,
      "<Name>", "Set disk image for autostart of PRG files" },
    { "-autostart-delay", SET_RESOURCE, CMDLINE_ATTRIB_NEED_ARGS,
      NULL, NULL, "AutostartDelay", NULL,
      "<frames>", "Set initial autostart delay (0: use default)" },
    { "-autostart-delay-random", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "AutostartDelayRandom", (resource_value_t)1,
      NULL, "Enable random initial autostart delay." },
    { "+autostart-delay-random", SET_RESOURCE, CMDLINE_ATTRIB_NONE,
      NULL, NULL, "AutostartDelayRandom", (resource_value_t)0,
      NULL, "Disable random initial autostart delay." },
    CMDLINE_LIST_END
};

/*! \brief initialize the command-line options

 \return
   0 on success, else -1.

 \remark
   Registers the command-line options
*/
int autostart_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

/* Deallocate program name if we have one */
static void deallocate_program_name(void)
{
    lib_free(autostart_program_name);
    autostart_program_name = NULL;
}

static enum { YES, NO, NOT_YET } check(const char *s, unsigned int blink_mode)
{
    int screen_addr, line_length, cursor_column, addr, i;

    screen_addr = (int)(mem_read((uint16_t)(pnt)) | (mem_read((uint16_t)(pnt + 1)) << 8));
    cursor_column = (int)mem_read((uint16_t)(pntr));

    line_length = (int)(lnmx < 0 ? -lnmx : mem_read((uint16_t)(lnmx)) + 1);

    DBG(("check(%s) pnt:%04x pntr:%04x addr:%04x column:%d, linelen:%d blnsw:%04x(%d)",
         s, pnt, pntr, screen_addr, cursor_column, line_length, blnsw, mem_read(blnsw)));

    if (!kbdbuf_is_empty()) {
        return NOT_YET;
    }

    if (blink_mode == AUTOSTART_WAIT_BLINK && cursor_column != 0) {
        return NOT_YET;
    }

    if (blink_mode == AUTOSTART_WAIT_BLINK && blnsw != 0 && mem_read(blnsw) != 0) {
        return NOT_YET;
    }

    if (blink_mode == AUTOSTART_WAIT_BLINK) {
        addr = screen_addr - line_length;
    } else {
        addr = screen_addr;
    }

    for (i = 0; s[i] != '\0'; i++) {
        if (mem_read((uint16_t)(addr + i)) != s[i] % 64) {
            if (mem_read((uint16_t)(addr + i)) != (uint8_t)32) {
                return NO;
            }
            return NOT_YET;
        }
    }
    return YES;
}

static void set_true_drive_emulation_mode(int on, int unit)
{
    log_message(autostart_log, "Turning TDE %s for unit %d.", on ? "on" : "off", unit);
    resources_set_int_sprintf("Drive%iTrueEmulation", on, unit);
    ui_update_menus();
}

static int get_true_drive_emulation_state(int unit)
{
    int value;

    if (resources_get_int_sprintf("Drive%iTrueEmulation", &value, unit) < 0) {
        return 0;
    }

    return value;
}

static void set_warp_mode(int on)
{
    resources_set_int("WarpMode", on);
    ui_update_menus();
}

static int get_warp_mode(void)
{
    int value;

    if (resources_get_int("WarpMode", &value) < 0) {
        return 0;
    }

    return value;
}

static int get_device_traps_state(void)
{
    int value;

    if (resources_get_int("VirtualDevices", &value) < 0) {
        return 0;
    }

    return value;
}

static void set_device_traps_state(int on)
{
    resources_set_int("VirtualDevices", on);
    ui_update_menus();
}

static void enable_warp_if_requested(void)
{
    /* enable warp mode? */
    if (AutostartWarp) {
        orig_warp_mode = get_warp_mode();
        if (!orig_warp_mode) {
            log_message(autostart_log, "Turning Warp mode on");
            set_warp_mode(1);
        }
    }
}

static void disable_warp_if_was_requested(void)
{
    /* disable warp mode */
    if (AutostartWarp) {
        if (!orig_warp_mode) {
            log_message(autostart_log, "Turning Warp mode off");
            set_warp_mode(0);
        }
    }
}

static void check_rom_area(void)
{
    /* enter ROM ? */
    if (!entered_rom) {
        if (reg_pc >= 0xe000) {
            log_message(autostart_log, "Entered ROM at $%04x", reg_pc);
            entered_rom = 1;
        }
    } else {
        /* special case for auto-starters: ROM left. We also consider
         * BASIC area to be ROM, because it's responsible for writing "READY."
         */
        if (machine_addr_in_ram(reg_pc)) {
            log_message(autostart_log, "Left ROM for $%04x", reg_pc);
            disable_warp_if_was_requested();
            autostart_done();
        }
    }
}

/* ------------------------------------------------------------------------- */

/* remember the state of some settings before we do autostart:
 * tde enabled/disabled, device traps enabled/disabled, warpmode enabled/disabled
 * 
 * this should get called once, at the beginning of the autostart
 */
static void init_drive_emulation_state(int unit)
{
    DBG(("init_drive_emulation_state tde:%d traps:%d warp:%d",
        get_true_drive_emulation_state(unit), get_device_traps_state(), get_warp_mode()
    ));
    
    if (orig_drive_true_emulation_state == -1) {
        orig_drive_true_emulation_state = get_true_drive_emulation_state(unit);
    }
    if (orig_device_traps_state == -1) {
        orig_device_traps_state = get_device_traps_state();
    }
    if (orig_warp_mode == -1) {
        orig_warp_mode = get_warp_mode();
    }
}

/* restore the state of all settings whe changed during autostart.
 * 
 * this should get called on any "exit" of the autostart, error etc
 */
static void restore_drive_emulation_state(int unit)
{
    DBG(("restore_drive_emulation_state"));
    if (orig_device_traps_state != -1) {
        /* set device traps to original state */
        if (get_device_traps_state() != orig_device_traps_state) {
            log_message(autostart_log, "Turning virtual device traps %s.",
                        orig_device_traps_state ? "on" : "off");
            set_device_traps_state(orig_device_traps_state);
        }
    }
    if (orig_drive_true_emulation_state != -1) {
        /* set TDE to original state */
        if (get_true_drive_emulation_state(unit) != orig_drive_true_emulation_state) {
            log_message(autostart_log, "Turning TDE %s.",
                        orig_drive_true_emulation_state ? "on" : "off");
            set_true_drive_emulation_mode(orig_drive_true_emulation_state, unit);
        }
    }
    if (orig_warp_mode != -1) {
        /* set warp to original state */
        if (get_warp_mode() != orig_warp_mode) {
            log_message(autostart_log, "Turning Warp mode %s.",
                        orig_warp_mode ? "on" : "off");
            set_warp_mode(orig_warp_mode);
        }
    }
    
    /* make sure we refresh these next time we do autostart via gui */
    orig_drive_true_emulation_state = - 1;
    orig_device_traps_state = - 1;
    orig_warp_mode = -1;

    
    DBG(("restore_drive_emulation_state tde:%d traps:%d warp:%d",
        get_true_drive_emulation_state(unit), get_device_traps_state(), get_warp_mode()
    ));
}

/* ------------------------------------------------------------------------- */

static void load_snapshot_trap(uint16_t unused_addr, void *unused_data)
{
    if (autostart_program_name
        && machine_read_snapshot((char *)autostart_program_name, 0) < 0) {
        snapshot_display_error();
    }

    ui_update_menus();
}

/* ------------------------------------------------------------------------- */

/* Reset autostart.  */
void autostart_reinit(CLOCK min_cycles_, int handle_tde,
                      int blnsw_, int pnt_, int pntr_, int lnmx_)
{
    blnsw = (uint16_t)(blnsw_);
    pnt = pnt_;
    pntr = pntr_;
    lnmx = lnmx_;

    min_cycles = min_cycles_;

    handle_drive_true_emulation_by_machine = handle_tde;

    set_handle_true_drive_emulation_state();

    if (min_cycles_) {
        autostart_enabled = 1;
    } else {
        autostart_enabled = 0;
    }
}

/* Initialize autostart.  */
int autostart_init(CLOCK min_cycles_, int handle_drive_true_emulation,
                   int blnsw_, int pnt_, int pntr_, int lnmx_)
{
    autostart_prg_init();

    autostart_reinit(min_cycles_, handle_drive_true_emulation, blnsw_, pnt_, pntr_, lnmx_);

    if (autostart_log == LOG_ERR) {
        autostart_log = log_open("AUTOSTART");
        if (autostart_log == LOG_ERR) {
            return -1;
        }
    }

    return 0;
}

void autostart_disable(void)
{
    if (!autostart_enabled) {
        return;
    }

    DBG(("autostart_disable (ERROR)"));

    autostartmode = AUTOSTART_ERROR;
    trigger_monitor = 0;
    deallocate_program_name();
    log_error(autostart_log, "Turned off.");
}

/* Control if the monitor will be triggered after an autostart */
void autostart_trigger_monitor(int enable)
{
    trigger_monitor = enable;
}

/* this is called after successful loading */
static void autostart_finish(void)
{
    DBG(("autostart_finish"));

    if (autostart_run_mode == AUTOSTART_MODE_RUN) {
        log_message(autostart_log, "Starting program.");
        if ((machine_class == VICE_MACHINE_C128) && (c128_column4080_key == 0)) {
            kbdbuf_feed("GRAPHIC5:");
        }
        /* log_message(autostart_log, "Run command is: '%s' (%s)", AutostartRunCommand, AutostartDelayRandom ? "delayed" : "no delay"); */
        if (AutostartDelayRandom) {
            kbdbuf_feed_runcmd(AutostartRunCommand);
        } else {
            kbdbuf_feed(AutostartRunCommand);
        }
    } else {
        log_message(autostart_log, "Program loaded.");
        if ((machine_class == VICE_MACHINE_C128) && (c128_column4080_key == 0)) {
            kbdbuf_feed("GRAPHIC5\x0d");
        }
    }
    /* printf("autostart_finish cmdline_get_autostart_mode(): %d\n", cmdline_get_autostart_mode()); */
    /* inject string given to -keybuf option on commandline into keyboard buffer */
    if (cmdline_get_autostart_mode() != AUTOSTART_MODE_NONE) {
        kbdbuf_feed_cmdline();
    }
}

/* This is called if all steps of an autostart operation were passed successfully */
static void autostart_done(void)
{
    DBG(("autostart_done"));

    restore_drive_emulation_state(8);

    autostartmode = AUTOSTART_DONE;

    if (machine_class == VICE_MACHINE_C128) {
        /* restore original state of key */
        resources_set_int("C128ColumnKey", c128_column4080_key);
    }

    /* Enter monitor after done */
    if (trigger_monitor) {
        trigger_monitor = 0;
        monitor_startup_trap();
        log_message(autostart_log, "Done. Returning to Monitor.");
    } else {
        log_message(autostart_log, "Done.");
    }
}

/* ------------------------------------------------------------------------- */

/* This function is called by the `serialreceivebyte()' trap as soon as EOF
   is reached.  */
static void disk_eof_callback(void)
{
    DBG(("disk_eof_callback"));

    if (handle_drive_true_emulation_overridden) {
        uint8_t id[2], *buffer = NULL;
        unsigned int track, sector;
        /* FIXME: shouldnt this loop over all drives? */
        /* FIXME: what exactly is this stuff supposed to do? */        
        if (orig_drive_true_emulation_state) {
            log_message(autostart_log, "Turning true drive emulation on.");
            if (vdrive_bam_get_disk_id(8, 0, id) == 0) { // VDRIVE_EFFORT
                vdrive_get_last_read(&track, &sector, &buffer);
            }
        }
        /*set_true_drive_emulation_mode(orig_drive_true_emulation_state, 8);*/
        if (orig_drive_true_emulation_state) {
            if (buffer) {
                log_message(autostart_log, "Restoring true drive state of drive 8.");
                drive_set_disk_memory(id, track, sector, drive_context[0]);
                drive_set_last_read(track, sector, buffer, drive_context[0]);
            } else {
                log_message(autostart_log, "No Disk Image in drive 8.");
            }
        }
    }

    if (autostartmode != AUTOSTART_NONE) {
        autostart_finish();
    }

    autostart_done();

    machine_bus_eof_callback_set(NULL);

    disable_warp_if_was_requested();
}

#if 0
/* This function is called by the `serialattention()' trap before
   returning.  */
static void disk_attention_callback(void)
{
    machine_bus_attention_callback_set(NULL);

    /* Next step is waiting for end of loading, to turn true drive emulation
       on.  */
    machine_bus_eof_callback_set(disk_eof_callback);
}
#endif

/* ------------------------------------------------------------------------- */

static void advance_hastape(void)
{
    char *tmp;
    DBG(("advance_hastape"));

    switch (check("READY.", AUTOSTART_WAIT_BLINK)) {
        case YES:
            log_message(autostart_log, "Loading file.");
            if (autostart_program_name) {
                tmp = util_concat("LOAD\"", autostart_program_name, "\":\r", NULL);
                kbdbuf_feed(tmp);
                lib_free(tmp);
            } else {
                kbdbuf_feed("LOAD:\r");
            }
            autostartmode = AUTOSTART_PRESSPLAYONTAPE;
            entered_rom = 0;
            deallocate_program_name();
            break;
        case NO:
            disable_warp_if_was_requested();
            autostart_disable();
            break;
        case NOT_YET:
            break;
    }
}

static void advance_pressplayontape(void)
{
    switch (check("PRESS PLAY ON TAPE", AUTOSTART_NOWAIT_BLINK)) {
        case YES:
            autostartmode = AUTOSTART_LOADINGTAPE;
            datasette_control(DATASETTE_CONTROL_START);
            break;
        case NO:
            disable_warp_if_was_requested();
            autostart_disable();
            break;
        case NOT_YET:
            break;
    }
}

static void advance_loadingtape(void)
{
    switch (check("READY.", AUTOSTART_WAIT_BLINK)) {
        case YES:
            disable_warp_if_was_requested();
            autostart_finish();
            autostart_done();
            break;
        case NO:
            disable_warp_if_was_requested();
            autostart_disable();
            break;
        case NOT_YET:
            /* leave autostart and disable warp if ROM area was left */
            check_rom_area();
            break;
    }
}

static void advance_hasdisk(void)
{
    char *tmp, *temp_name;
    int traps;

    DBG(("advance_hasdisk"));

    switch (check("READY.", AUTOSTART_WAIT_BLINK)) {
        case YES:

            /* autostart_program_name may be petscii or ascii at this point,
               ANDing the charcodes with 0x7f here is a cheap way to prevent
               illegal characters in the printed message */
            if (autostart_program_name) {
                temp_name = tmp = lib_stralloc(autostart_program_name);
                while (*tmp) {
                    *tmp++ &= 0x7f;
                }
                log_message(autostart_log, "Loading program '%s'", temp_name);
                lib_free(temp_name);
            } else {
                log_message(autostart_log, "Loading program '*'");
            }

            DBG(("advance_hasdisk traps:%d tde:%d\n", 
                 get_device_traps_state(), get_true_drive_emulation_state(8)));

            tmp = lib_msprintf("LOAD\"%s\",8%s:\r",
                               autostart_program_name ?
                               autostart_program_name : "*",
                               autostart_basic_load ? "" : ",1");
            DBG(("advance_hasdisk LOAD\"%s\",8%s:\r",
                               autostart_program_name ?
                               autostart_program_name : "*",
                               autostart_basic_load ? "" : ",1"));
            kbdbuf_feed(tmp);
            lib_free(tmp);

            /* FIXME: before, the code selected one of three different code
               pathes, depending on the state of traps and warpmode.
               it seems a bit strange that this is needed, and also that simple
               doing the same will work better too.
               i am leaving the following here for experimentation while
               completely debugging the autostart madness */
            
            /* switch to next state ("searching...") */
#if 1
            autostartmode = AUTOSTART_WAITSEARCHINGFOR;
#endif
#if 0
            /* be most compatible if warp is disabled */
            autostart_finish();
            autostart_done(); /* -> AUTOSTART_DONE */
#endif  
#if 0
            autostartmode = AUTOSTART_LOADINGDISK;
            machine_bus_attention_callback_set(disk_attention_callback);
#endif

#if 0
            /* this is what the code did before the rework. but why? */            
            if (!traps) {
                if (AutostartWarp) {
                    autostartmode = AUTOSTART_WAITSEARCHINGFOR;
                } else {
                    /* be most compatible if warp is disabled */
                    autostart_finish();
                    autostart_done();
                }
            } else {
                autostartmode = AUTOSTART_LOADINGDISK;
                machine_bus_attention_callback_set(disk_attention_callback);
            }
#endif
            deallocate_program_name();
            break;
        case NO:
            orig_drive_true_emulation_state = get_true_drive_emulation_state(8);
            orig_device_traps_state = get_device_traps_state();
            disable_warp_if_was_requested();
            autostart_disable();
            break;
        case NOT_YET:
            /* leave autostart and disable warp if ROM area was left */
            check_rom_area();
            break;
    }
}

static void advance_hassnapshot(void)
{
    switch (check("READY.", AUTOSTART_WAIT_BLINK)) {
        case YES:
            autostart_done();
            log_message(autostart_log, "Restoring snapshot.");
            interrupt_maincpu_trigger_trap(load_snapshot_trap, 0);
            break;
        case NO:
            autostart_disable();
            break;
        case NOT_YET:
            break;
    }
}

/* ----- stages for tde disk loading with warp --------------------------- */

static void advance_waitsearchingfor(void)
{
    DBG(("advance_waitsearchingfor"));  

    switch (check("SEARCHING FOR", AUTOSTART_NOWAIT_BLINK)) {
        case YES:
            log_message(autostart_log, "Searching for ...");
            autostartmode = AUTOSTART_WAITLOADING;
            break;
        case NO:
            /* check if we are already in the next line showing LOADING ? */
            /* if (check("LOADING", AUTOSTART_NOWAIT_BLINK) == YES) { */
            /* FIXME: checking only for the beginning of the line will catch
                      some more problem cases */
            if (check("LO", AUTOSTART_NOWAIT_BLINK) == YES) {
                log_message(autostart_log, "Searching for ... missed, got LOADING");
                /* proceed as if mode was AUTOSTART_WAITLOADING */
                entered_rom = 0;
                autostartmode = AUTOSTART_WAITLOADREADY;
                break;
            }        
            log_message(autostart_log, "NO Searching for ...");
            disable_warp_if_was_requested();
            autostart_disable();
            break;
        case NOT_YET:
            /* leave autostart and disable warp if ROM area was left */
            check_rom_area();
            break;
    }
}

static void advance_waitloading(void)
{
    DBG(("advance_waitloading"));

    switch (check("LOADING", AUTOSTART_NOWAIT_BLINK)) {
        case YES:
            log_message(autostart_log, "Loading");
            entered_rom = 0;
            autostartmode = AUTOSTART_WAITLOADREADY;
            break;
        case NO:
            /* still showing SEARCHING FOR ? */
            if (check("SEARCHING FOR", AUTOSTART_NOWAIT_BLINK) == YES) {
                return;
            }
            /* no something else is shown -> error! */
            log_message(autostart_log, "NO Loading");
            disable_warp_if_was_requested();
            autostart_disable();
            break;
        case NOT_YET:
            /* leave autostart and disable warp if ROM area was left */
            check_rom_area();
            break;
    }
}

static void advance_waitloadready(void)
{
    DBG(("advance_waitloadready"));

    switch (check("READY.", AUTOSTART_WAIT_BLINK)) {
        case YES:
            log_message(autostart_log, "Ready");
            disable_warp_if_was_requested();
            autostart_finish();
            autostart_done();
            break;
        case NO:
            log_message(autostart_log, "NO Ready");
            disable_warp_if_was_requested();
            autostart_disable();
            break;
        case NOT_YET:
            /* leave autostart and disable warp if ROM area was left */
            check_rom_area();
            break;
    }
}

/* After a reset a PRG file has to be injected into RAM */
static void advance_inject(void)
{
    if (autostart_prg_perform_injection(autostart_log) < 0) {
        disable_warp_if_was_requested();
        autostart_disable();
    } else {
        /* wait for ready cursor and type RUN */
        autostartmode = AUTOSTART_WAITLOADREADY;
    }
}

/* Execute the actions for the current `autostartmode', advancing to the next
   mode if necessary.  */
void autostart_advance(void)
{
    if (!autostart_enabled) {
        return;
    }

    if (maincpu_clk < autostart_initial_delay_cycles) {
        autostart_wait_for_reset = 0;
        return;
    }

    if (autostart_wait_for_reset) {
        return;
    }

    switch (autostartmode) {
        case AUTOSTART_HASTAPE: /* wait for "READY.", to AUTOSTART_PRESSPLAYONTAPE */
            advance_hastape();
            break;
        case AUTOSTART_PRESSPLAYONTAPE: /* wait for "PRESS PLAY ON TAPE", to AUTOSTART_LOADINGTAPE */
            advance_pressplayontape();
            break;
        case AUTOSTART_LOADINGTAPE: /* wait for "READY." */
            advance_loadingtape();
            break;
        case AUTOSTART_HASDISK: /* wait for "READY.", to  AUTOSTART_WAITSEARCHINGFOR or AUTOSTART_LOADINGDISK */
            advance_hasdisk();
            break;
        case AUTOSTART_HASSNAPSHOT: /* wait for "READY." */
            advance_hassnapshot();
            break;
        case AUTOSTART_WAITLOADREADY: /* wait for "READY." */
            advance_waitloadready();
            break;
        case AUTOSTART_WAITLOADING: /* wait for "LOADING", to AUTOSTART_WAITLOADREADY */
            advance_waitloading();
            break;
        case AUTOSTART_WAITSEARCHINGFOR:  /* wait for "SEARCHING FOR", to AUTOSTART_WAITLOADING */
            advance_waitsearchingfor();
            break;
        case AUTOSTART_INJECT: /* to AUTOSTART_WAITLOADREADY */
            advance_inject();
            break;
        case AUTOSTART_ERROR:
            log_message(autostart_log, "Error");
            restore_drive_emulation_state(8);
            autostartmode = AUTOSTART_DONE;
            break;

        default:
            return;
    }
}

/* Clean memory and reboot for autostart.  */
static void reboot_for_autostart(const char *program_name, unsigned int mode,
                                 unsigned int runmode)
{
    int rnd;
    char *temp_name = NULL, *temp;

    if (!autostart_enabled) {
        return;
    }

    DBG(("reboot_for_autostart %s mode: %u runmode: %u", program_name, mode, runmode));

    /* program_name may be petscii or ascii at this point, ANDing the charcodes
       with 0x7f here is a cheap way to prevent illegal characters in the
       printed message */
    if (program_name) {
        temp_name = temp = lib_stralloc(program_name);
        while (*temp) {
            *temp++ &= 0x7f;
        }
    }
    log_message(autostart_log, "Resetting the machine to autostart '%s'",
                program_name ? temp_name : "*");
    if (program_name) {
        lib_free(temp_name);
    }

    /* on x128 autostart will only work in 40 columns mode (and can not be fixed
       easily for VDC mode). We work around that by switching to 40 columns and
       back if needed */
    if (machine_class == VICE_MACHINE_C128) {
        resources_get_int("C128ColumnKey", &c128_column4080_key);
        resources_set_int("C128ColumnKey", 1);
    }

    mem_powerup();

    autostart_ignore_reset = 1;
    deallocate_program_name();
    if (program_name && program_name[0]) {
        autostart_program_name = lib_stralloc(program_name);
    }

    autostartmode = mode;
    autostart_run_mode = runmode;
    autostart_wait_for_reset = 1;

    autostart_initial_delay_cycles = min_cycles;
    resources_get_int("AutostartDelayRandom", &rnd);
    if (rnd) {
        /* additional random delay of up to 10 frames */
        autostart_initial_delay_cycles += lib_unsigned_rand(1, machine_get_cycles_per_frame() * 10);
    }
    DBG(("reboot_for_autostart - autostart_initial_delay_cycles: %u", autostart_initial_delay_cycles));

    machine_trigger_reset(MACHINE_RESET_MODE_HARD);

    /* enable warp before reset */
    if (mode != AUTOSTART_HASSNAPSHOT) {
        enable_warp_if_requested();
    }
}

/* ------------------------------------------------------------------------- */

/* Autostart snapshot file `file_name'.  */
int autostart_snapshot(const char *file_name, const char *program_name)
{
    uint8_t vmajor, vminor;
    snapshot_t *snap;

    if (network_connected() || event_record_active() || event_playback_active()
        || file_name == NULL || !autostart_enabled) {
        return -1;
    }

    deallocate_program_name();  /* not needed at all */

    if (!(snap = snapshot_open(file_name, &vmajor, &vminor, machine_get_name()))) {
        autostartmode = AUTOSTART_ERROR;
        return -1;
    }

    log_message(autostart_log, "Loading snapshot file `%s'.", file_name);
    snapshot_close(snap);

    /*autostart_program_name = lib_stralloc(file_name);
    interrupt_maincpu_trigger_trap(load_snapshot_trap, 0);*/
    /* use for snapshot */
    reboot_for_autostart(file_name, AUTOSTART_HASSNAPSHOT, AUTOSTART_MODE_RUN);

    return 0;
}

/* Autostart tape image `file_name'.  */
int autostart_tape(const char *file_name, const char *program_name,
                   unsigned int program_number, unsigned int runmode)
{
    uint8_t do_seek = 1;

    if (network_connected() || event_record_active() || event_playback_active()
        || !file_name || !autostart_enabled) {
        return -1;
    }

    /* make sure to init TDE and traps status before each autostart */
    init_drive_emulation_state(8);
    
    datasette_control(DATASETTE_CONTROL_STOP);

    if (!(tape_image_attach(1, file_name) < 0)) {
        log_message(autostart_log,
                    "Attached file `%s' as a tape image.", file_name);
        if (!tape_tap_attached()) {
            if (program_number == 0 || program_number == 1) {
                do_seek = 0;
            }
            program_number -= 1;
        }
        if (do_seek) {
            if (program_number > 0) {
                /* program numbers in tape_seek_to_file() start at 0 */
                tape_seek_to_file(tape_image_dev1, program_number - 1);
            } else {
                tape_seek_start(tape_image_dev1);
            }
        }
        if (!tape_tap_attached()) {
            resources_set_int("VirtualDevices", 1); /* Kludge: for t64 images we need devtraps ON */
        }
        reboot_for_autostart(program_name, AUTOSTART_HASTAPE, runmode);

        return 0;
    }

    DBG(("autostart_tape (ERROR)"));
    autostartmode = AUTOSTART_ERROR;
    deallocate_program_name();

    return -1;
}

/* Cope with 0xa0 padded file names.  */
static void autostart_disk_cook_name(char **name)
{
    unsigned int pos;

    pos = 0;

    while ((*name)[pos] != '\0') {
        if (((unsigned char)((*name)[pos])) == 0xa0) {
            char *ptr;

            ptr = lib_malloc(pos + 1);
            memcpy(ptr, *name, pos);
            ptr[pos] = '\0';
            lib_free(*name);
            *name = ptr;
            break;
        }
        pos++;
    }
}

static void setup_for_disk(int unit)
{
    if (handle_drive_true_emulation_overridden) {
        /* disable TDE if device traps are enabled, 
           enable TDE if device traps are disabled */
        if (orig_device_traps_state) {
            if (orig_drive_true_emulation_state) {
                log_message(autostart_log, "Turning true drive emulation off.");
                set_true_drive_emulation_mode(0, unit);
            }
        } else {
            if (!orig_drive_true_emulation_state) {
                log_message(autostart_log, "Turning true drive emulation on.");
                set_true_drive_emulation_mode(1, unit);
            }
            if (!get_true_drive_emulation_state(unit)) {
                log_message(LOG_ERR, "True drive emulation is not enabled, Turning virtual device traps on.");
                set_device_traps_state(1);
                if (!get_device_traps_state()) {
                    log_message(LOG_ERR, "Virtual device traps are not enabled.");
                }
            }
        }
    } else {
        /* disable traps when TDE is enabled,
           enable traps when TDE is disabled. */
        if (orig_drive_true_emulation_state) {
            if (orig_device_traps_state) {
                log_message(autostart_log, "Turning  virtual device traps off.");
                set_device_traps_state(0);
            }
        } else {
            if (!orig_device_traps_state) {
                log_message(autostart_log, "Turning  virtual device traps on.");
                set_device_traps_state(1);
            }
            if (!get_device_traps_state()) {
                log_message(LOG_ERR, "Virtual device traps are not enabled.");
            }
        }
    }
    DBG(("setup for disk: TDE: %s  Traps: %s handle TDE: %s", 
        get_true_drive_emulation_state(unit) ? "on" : "off", 
        get_device_traps_state() ? "on" : "off",
        handle_drive_true_emulation_overridden ? "yes" : "no"
        ));
}

/* Autostart disk image `file_name'.  */
int autostart_disk(const char *file_name, const char *program_name,
                   unsigned int program_number, unsigned int runmode)
{
    char *name = NULL;

    DBG(("autostart_disk"));

    if (network_connected() || event_record_active() || event_playback_active()
        || !file_name || !autostart_enabled) {
        return -1;
    }

    /* make sure to init TDE and traps status before each autostart */
    init_drive_emulation_state(8);

    /* Get program name first to avoid more than one file handle open on
       image.  */
    if (!program_name && program_number > 0) {
        image_contents_t *contents = diskcontents_filesystem_read(file_name);
        if (contents) {
            name = image_contents_filename_by_number(contents, program_number);
            image_contents_destroy(contents);
        }
    } else {
        name = lib_stralloc(program_name ? program_name : "*");
    }

    if (name) {
        autostart_disk_cook_name(&name);
        if (!(file_system_attach_disk(8, file_name) < 0)) {

#if 0
            vdrive_t *vdrive;
            struct disk_image_s *diskimg;
#endif

            log_message(autostart_log,
                        "Attached file `%s' as a disk image.", file_name);

            /*
             * Simple attempt at implementing setting the current drive type
             * based on the image type as per feature request #319.
             *
             * I'm leaving this commented out since I don't think I'll be able
             * properly implement this AND properly test it before the code
             * freeze on 2018-12-3 (I have a lot of other stuff to do). I will
             * pick this up again after 3.3 has been released.
             *
             * --compyx
             */
#if 0
            /* shitty code, we really need to extend the drive API to
             * get at these sorts for things without breaking into core code
             */
            vdrive = file_system_get_vdrive(8);
            if (vdrive == NULL) {
                log_error(LOG_ERR, "Failed to get vdrive reference for unit 8.");
            } else {
                diskimg = vdrive->image;
                if (diskimg == NULL) {
                    log_error(LOG_ERR, "Failed to get disk image for unit 8.");
                } else {
                    printf("\nGOT IMAGE TYPE %u.\n\n", diskimg->type);
                    if (resources_set_int("Drive8Type", diskimg->type) < 0) {
                        log_error(LOG_ERR, "Failed to set drive type.");
                    }
                    drive_cpu_trigger_reset(0);
                }
            }
#endif
            setup_for_disk(8);
            reboot_for_autostart(name, AUTOSTART_HASDISK, runmode);
            lib_free(name);

            return 0;
        }
    }

    DBG(("autostart_disk: ERROR"));
    autostartmode = AUTOSTART_ERROR;
    deallocate_program_name();
    lib_free(name);

    return -1;
}

static void setup_for_prg(int mode, int unit)
{
    switch (mode) {
        case AUTOSTART_PRG_MODE_VFS:
            if (handle_drive_true_emulation_overridden) {
                if (orig_drive_true_emulation_state) {
                    log_message(autostart_log, "Turning true drive emulation off.");
                    set_true_drive_emulation_mode(0, unit);
                }
            }
            if (get_true_drive_emulation_state(unit)) {
                log_message(LOG_ERR, "True drive emulation is still enabled.");
            }
            if (!orig_device_traps_state) {
                log_message(autostart_log, "Turning virtual device traps on.");
                set_device_traps_state(1);
            }
            if (!get_device_traps_state()) {
                log_message(LOG_ERR, "Virtual device traps are not enabled.");
            }
            
            /* resources_set_int("VirtualDevices", 1); */
            resources_set_int("FSDevice8ConvertP00", 1); /* FIXME: not preserved */
            file_system_detach_disk(8);
            resources_set_int("FileSystemDevice8", ATTACH_DEVICE_FS); /* FIXME: not preserved */
            break;
        case AUTOSTART_PRG_MODE_INJECT:
            break;
        case AUTOSTART_PRG_MODE_DISK:
            setup_for_disk(8);
            break;
    }

    DBG(("setup for prg: TDE: %s  Traps: %s handle TDE: %s", 
        get_true_drive_emulation_state(unit) ? "on" : "off", 
        get_device_traps_state() ? "on" : "off",
        handle_drive_true_emulation_overridden ? "yes" : "no"
        ));
}

/* Autostart PRG file `file_name'.  The PRG file can either be a raw CBM file
   or a P00 file */
int autostart_prg(const char *file_name, unsigned int runmode)
{
    fileio_info_t *finfo;
    int result;
    const char *boot_file_name;
    int mode;

    DBG(("autostart_prg"));

    if (network_connected() || event_record_active() || event_playback_active()) {
        return -1;
    }

    /* open prg file */
    finfo = fileio_open(file_name, NULL, FILEIO_FORMAT_RAW | FILEIO_FORMAT_P00,
                        FILEIO_COMMAND_READ | FILEIO_COMMAND_FSNAME,
                        FILEIO_TYPE_PRG);

    /* can't open file */
    if (finfo == NULL) {
        log_error(autostart_log, "Cannot open `%s'.", file_name);
        return -1;
    }

    /* make sure to init TDE and traps status before each autostart */
    init_drive_emulation_state(8);

    setup_for_prg(AutostartPrgMode, 8);

    /* determine how to load file */
    switch (AutostartPrgMode) {
        case AUTOSTART_PRG_MODE_VFS:
            log_message(autostart_log, "Loading PRG file `%s' with virtual FS on unit #8.", file_name);
            result = autostart_prg_with_virtual_fs(file_name, finfo, autostart_log);
            mode = AUTOSTART_HASDISK;
            boot_file_name = (const char *)finfo->name;
            break;
        case AUTOSTART_PRG_MODE_INJECT:
            log_message(autostart_log, "Loading PRG file `%s' with direct RAM injection.", file_name);
            result = autostart_prg_with_ram_injection(file_name, finfo, autostart_log);
            mode = AUTOSTART_INJECT;
            boot_file_name = NULL;
            break;
        case AUTOSTART_PRG_MODE_DISK:
            {
            char *savedir;
            log_message(autostart_log, "Loading PRG file `%s' with autostart disk image.", file_name);
            /* create the directory where the image should be written first */
            util_fname_split(AutostartPrgDiskImage, &savedir, NULL);
            ioutil_mkdir(savedir, IOUTIL_MKDIR_RWXU);
            lib_free(savedir);
            result = autostart_prg_with_disk_image(file_name, finfo, autostart_log, AutostartPrgDiskImage);
            mode = AUTOSTART_HASDISK;
            boot_file_name = "*";
            }
            break;
        default:
            log_error(autostart_log, "Invalid PRG autostart mode: %d", AutostartPrgMode);
            result = -1;
            break;
    }

    /* Now either proceed with disk image booting or prg injection after reset */
    if (result >= 0) {
        ui_update_menus();
        reboot_for_autostart(boot_file_name, mode, runmode);
    }

    /* close prg file */
    fileio_close(finfo);

    return result;
}

/* ------------------------------------------------------------------------- */

int autostart_autodetect_opt_prgname(const char *file_prog_name,
                                     unsigned int alt_prg_number,
                                     unsigned int runmode)
{
    char *tmp;
    int result;

    /* Check for image:prg -format.  */
    tmp = strrchr(file_prog_name, ':');
    if (tmp) {
        char *autostart_prg_name;
        char *autostart_file;

        autostart_file = lib_stralloc(file_prog_name);
        autostart_prg_name = strrchr(autostart_file, ':');
        *autostart_prg_name++ = '\0';
        /* Does the image exist?  */
        if (util_file_exists(autostart_file)) {
            char *name;

            charset_petconvstring((uint8_t *)autostart_prg_name, 0);
            name = charset_replace_hexcodes(autostart_prg_name);
            result = autostart_autodetect(autostart_file, name, 0, runmode);
            lib_free(name);
        } else {
            result = autostart_autodetect(file_prog_name, NULL, alt_prg_number, runmode);
        }
        lib_free(autostart_file);
    } else {
        result = autostart_autodetect(file_prog_name, NULL, alt_prg_number, runmode);
    }
    return result;
}

/* Autostart `file_name', trying to auto-detect its type.  */
int autostart_autodetect(const char *file_name, const char *program_name,
                         unsigned int program_number, unsigned int runmode)
{
    if (network_connected() || event_record_active() || event_playback_active()
        || file_name == NULL) {
        return -1;
    }

    if (!autostart_enabled) {
        log_error(autostart_log,
                  "Autostart is not available on this setup.");
        return -1;
    }

    /* make sure to init TDE and traps status before each autostart */
    init_drive_emulation_state(8);

    log_message(autostart_log, "Autodetecting image type of `%s'.", file_name);

    if (autostart_disk(file_name, program_name, program_number, runmode) == 0) {
        log_message(autostart_log, "`%s' recognized as disk image.", file_name);
        return 0;
    }

    if (machine_class != VICE_MACHINE_C64DTV && machine_class != VICE_MACHINE_SCPU64) {
        if (autostart_tape(file_name, program_name, program_number, runmode) == 0) {
            log_message(autostart_log, "`%s' recognized as tape image.", file_name);
            return 0;
        }
    }

    if (autostart_snapshot(file_name, program_name) == 0) {
        log_message(autostart_log, "`%s' recognized as snapshot image.",
                    file_name);
        return 0;
    }

    if ((machine_class == VICE_MACHINE_C64) || (machine_class == VICE_MACHINE_C64SC) ||
       (machine_class == VICE_MACHINE_SCPU64) ||(machine_class == VICE_MACHINE_C128)) {
        if (cartridge_attach_image(CARTRIDGE_CRT, file_name) == 0) {
            log_message(autostart_log, "`%s' recognized as cartridge image.",
                        file_name);
            return 0;
        }
    }

    if (autostart_prg(file_name, runmode) == 0) {
        log_message(autostart_log, "`%s' recognized as program/p00 file.",
                    file_name);
        return 0;
    }

    log_error(autostart_log, "`%s' is not a valid file.", file_name);
    return -1;
}

/* Autostart the image attached to device `num'.  */
int autostart_device(int num)
{
    if (network_connected() || event_playback_active() || event_record_active()
        || !autostart_enabled) {
        return -1;
    }
    
    /* make sure to init TDE and traps status before each autostart */
    init_drive_emulation_state(8);

    switch (num) {
        case 8:
            reboot_for_autostart(NULL, AUTOSTART_HASDISK, AUTOSTART_MODE_RUN);
            return 0;
        case 1:
            reboot_for_autostart(NULL, AUTOSTART_HASTAPE, AUTOSTART_MODE_RUN);
            return 0;
    }
    return -1;
}

int autostart_in_progress(void)
{
    return ((autostartmode != AUTOSTART_NONE) && (autostartmode != AUTOSTART_DONE));
}

/* Disable autostart on reset.  */
void autostart_reset(void)
{
    int oldmode;

    if (!autostart_enabled) {
        return;
    }

    if (!autostart_ignore_reset
        && autostartmode != AUTOSTART_NONE
        && autostartmode != AUTOSTART_ERROR) {
        oldmode = autostartmode;
        autostartmode = AUTOSTART_NONE;
        if (oldmode != AUTOSTART_DONE) {
            disk_eof_callback();
        }
        autostartmode = AUTOSTART_NONE;
        trigger_monitor = 0;
        deallocate_program_name();
        log_message(autostart_log, "Turned off.");
    }
    autostart_ignore_reset = 0;
}

void autostart_shutdown(void)
{
    deallocate_program_name();

    autostart_prg_shutdown();
}

#ifdef ANDROID_COMPILE
void loader_set_warpmode(int on)
{
    set_warp_mode(on);
}
#endif
