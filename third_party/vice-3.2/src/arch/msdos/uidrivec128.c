/*
 * uidrivec128.c
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

#include "drive.h"
#include "lib.h"
#include "resources.h"
#include "tui.h"
#include "tuimenu.h"
#include "uidrive.h"


TUI_MENU_DEFINE_RADIO(Drive8ExtendImagePolicy)
TUI_MENU_DEFINE_RADIO(Drive9ExtendImagePolicy)
TUI_MENU_DEFINE_RADIO(Drive10ExtendImagePolicy)
TUI_MENU_DEFINE_RADIO(Drive11ExtendImagePolicy)

static TUI_MENU_CALLBACK(drive_extend_image_policy_submenu_callback)
{
    int unit = (int)param;
    char *rname;
    int v;

    rname = lib_msprintf("Drive%dExtendImagePolicy", unit);
    resources_get_int(rname, &v);
    lib_free(rname);

    switch (v) {
        case DRIVE_EXTEND_NEVER:
            return "Never extend";
        case DRIVE_EXTEND_ASK:
            return "Ask on extend";
        case DRIVE_EXTEND_ACCESS:
            return "Extend on access";
        default:
            return "Unknown";
    }
}

#define DEFINE_DRIVE_EXTEND_IMAGE_POLICY_SUBMENU(num)                           \
static tui_menu_item_def_t drive##num##_extend_image_policy_submenu[] = {       \
    { "_Never extend",                                                          \
      "Never create more than 35 tracks",                                       \
      radio_Drive##num##ExtendImagePolicy_callback,                             \
      (void *)DRIVE_EXTEND_NEVER, 0,                                            \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                         \
    { "_Ask on extend",                                                         \
      "Ask the user before creating extra tracks",                              \
      radio_Drive##num##ExtendImagePolicy_callback,                             \
      (void *)DRIVE_EXTEND_ASK, 0,                                              \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                         \
    { "_Extend on access",                                                      \
      "Automagically extend the disk image if extra (>35) tracks are accessed", \
      radio_Drive##num##ExtendImagePolicy_callback,                             \
      (void *)DRIVE_EXTEND_ACCESS, 0,                                           \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                         \
    TUI_MENU_ITEM_DEF_LIST_END                                                  \
};

DEFINE_DRIVE_EXTEND_IMAGE_POLICY_SUBMENU(8)
DEFINE_DRIVE_EXTEND_IMAGE_POLICY_SUBMENU(9)
DEFINE_DRIVE_EXTEND_IMAGE_POLICY_SUBMENU(10)
DEFINE_DRIVE_EXTEND_IMAGE_POLICY_SUBMENU(11)

TUI_MENU_DEFINE_TOGGLE(DriveTrueEmulation)

TUI_MENU_DEFINE_RADIO(Drive8Type)
TUI_MENU_DEFINE_RADIO(Drive9Type)
TUI_MENU_DEFINE_RADIO(Drive10Type)
TUI_MENU_DEFINE_RADIO(Drive11Type)

static TUI_MENU_CALLBACK(drive_type_submenu_callback)
{
    int value;

    switch ((int)param) {
        default:
        case 8:
            resources_get_int("Drive8Type", &value);
            break;
        case 9:
            resources_get_int("Drive9Type", &value);
            break;
        case 10:
            resources_get_int("Drive10Type", &value);
            break;
        case 11:
            resources_get_int("Drive11Type", &value);
            break;
    }

    switch (value) {
        case 0:
            return "None";
        case DRIVE_TYPE_1540:
            return "1540, 5\"1/4 SS";
        case DRIVE_TYPE_1541:
            return "1541, 5\"1/4 SS";
        case DRIVE_TYPE_1541II:
            return "1541-II, 5\"1/4 SS";
        case DRIVE_TYPE_1571:
            return "1571, 5\"1/4 DS";
        case DRIVE_TYPE_1571CR:
            return "1571cr, 5\"1/4 DS";
        case DRIVE_TYPE_1581:
            return "1581, 3\"1/2 SD DS";
        case DRIVE_TYPE_2000:
            return "2000, 3\"1/2 HD DS";
        case DRIVE_TYPE_4000:
            return "4000, 3\"1/2 ED DS";
        case DRIVE_TYPE_2031:
            return "2031, 5\"1/4 SS, IEEE488";
        case DRIVE_TYPE_2040:
            return "2040, 5\"1/4 SD, IEEE488";
        case DRIVE_TYPE_3040:
            return "3040, 5\"1/4 SD, IEEE488";
        case DRIVE_TYPE_4040:
            return "4040, 5\"1/4 SD, IEEE488";
        case DRIVE_TYPE_1001:
            return "1001, 5\"1/4 DS, IEEE488";
        case DRIVE_TYPE_8050:
            return "8050, 5\"1/4 SD, IEEE488";
        case DRIVE_TYPE_8250:
            return "8250, 5\"1/4 DD, IEEE488";
        default:
            return "(Unknown)";
    }
}

#define DEFINE_DRIVE_MODEL_SUBMENU(num)                                                 \
static tui_menu_item_def_t drive##num##_type_submenu[] = {                              \
    { "_None",                                                                          \
      "Disable hardware-level emulation of drive #" #num,                               \
      radio_Drive##num##Type_callback, (void *)0, 0,                                    \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "1540, 5\"1/4 SS",                                                                \
      "Emulate a 1540 5\"1/4 single-sided disk drive as unit #" #num,                   \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_1540, 0,                      \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "_1541, 5\"1/4 SS",                                                               \
      "Emulate a 1541 5\"1/4 single-sided disk drive as unit #" #num,                   \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_1541, 0,                      \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "1541-_II, 5\"1/4 SS",                                                            \
      "Emulate a 1541-II 5\"1/4 single-sided disk drive as unit #" #num,                \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_1541II, 0,                    \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "15_71, 5\"1/4 DS",                                                               \
      "Emulate a 1571 5\"1/4 double-sided disk drive as unit #" #num,                   \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_1571, 0,                      \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "1571_cr, 5\"1/4 DS",                                                             \
      "Emulate a 1571cr 5\"1/4 double-sided disk drive as unit #" #num,                 \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_1571CR, 0,                    \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "15_81, 3\"1/2 DD DS",                                                            \
      "Emulate a 1581 3\"1/2 double density, double-sided disk drive as unit #" #num,   \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_1581, 0,                      \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "2000, 3\"1/2 HD DS",                                                             \
      "Emulate a 2000 3\"1/2 high density, double-sided disk drive as unit #" #num,     \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_2000, 0,                      \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "4000, 3\"1/2 ED DS",                                                             \
      "Emulate a 4000 3\"1/2 extended density, double-sided disk drive as unit #" #num, \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_4000, 0,                      \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "_2031, 5\"1/4 SS IEEE488",                                                       \
      "Emulate a 2031 5\"1/4 single-sided IEEE disk drive as unit #" #num,              \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_2031, 0,                      \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "_2040, 5\"1/4 SD IEEE488",                                                       \
      "Emulate a 2040 5\"1/4 SD IEEE disk drive as unit #" #num,                        \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_2040, 0,                      \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "_3040, 5\"1/4 SD IEEE488",                                                       \
      "Emulate a 3040 5\"1/4 SD IEEE disk drive as unit #" #num,                        \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_3040, 0,                      \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "_4040, 5\"1/4 SD IEEE488",                                                       \
      "Emulate a 4040 5\"1/4 SD IEEE disk drive as unit #" #num,                        \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_4040, 0,                      \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "1_001, 5\"1/4 DS IEEE488",                                                       \
      "Emulate a 1001 5\"1/4 DS IEEE disk drive as unit #" #num,                        \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_1001, 0,                      \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "8050, 5\"1/4 _SD IEEE488",                                                       \
      "Emulate a 8050 5\"1/4 SD IEEE disk drive as unit #" #num,                        \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_8050, 0,                      \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    { "8520, 5\"1/4 _DD IEEE488",                                                       \
      "Emulate a 8250 5\"1/4 DD IEEE disk drive as unit #" #num,                        \
      radio_Drive##num##Type_callback, (void *)DRIVE_TYPE_8250, 0,                      \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                                 \
    TUI_MENU_ITEM_DEF_LIST_END                                                          \
};

DEFINE_DRIVE_MODEL_SUBMENU(8)
DEFINE_DRIVE_MODEL_SUBMENU(9)
DEFINE_DRIVE_MODEL_SUBMENU(10)
DEFINE_DRIVE_MODEL_SUBMENU(11)

TUI_MENU_DEFINE_RADIO(Drive8IdleMethod)
TUI_MENU_DEFINE_RADIO(Drive9IdleMethod)
TUI_MENU_DEFINE_RADIO(Drive10IdleMethod)
TUI_MENU_DEFINE_RADIO(Drive11IdleMethod)

static TUI_MENU_CALLBACK(drive_idle_method_submenu_callback)
{
    int value;

    switch ((int)param) {
        default:
        case 8:
            resources_get_int("Drive8IdleMethod", &value);
            break;
        case 9:
            resources_get_int("Drive9IdleMethod", &value);
            break;
        case 10:
            resources_get_int("Drive10IdleMethod", &value);
            break;
        case 11:
            resources_get_int("Drive11IdleMethod", &value);
            break;
    }

    switch (value) {
        case DRIVE_IDLE_NO_IDLE:
            return "None";
        case DRIVE_IDLE_TRAP_IDLE:
            return "Trap idle";
        case DRIVE_IDLE_SKIP_CYCLES:
            return "Skip cycles";
        default:
            return "(Unknown)";
    }
}

#define DEFINE_DRIVE_IDLE_METHOD_SUBMENU(num)                           \
static tui_menu_item_def_t drive##num##_idle_method_submenu[] = {       \
    { "_None",                                                          \
      "Always run the drive CPU as on the real thing",                  \
      radio_Drive##num##IdleMethod_callback,                            \
      (void *)DRIVE_IDLE_NO_IDLE, 0,                                    \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                 \
    { "_Trap Idle",                                                     \
      "Stop running the drive CPU when entering the idle DOS loop",     \
      radio_Drive##num##IdleMethod_callback,                            \
      (void *)DRIVE_IDLE_TRAP_IDLE, 0,                                  \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                 \
    { "_Skip Cycles",                                                   \
      "Skip drive CPU cycles when the IEC bus is not used for a while", \
      radio_Drive##num##IdleMethod_callback,                            \
      (void *)DRIVE_IDLE_SKIP_CYCLES, 0,                                \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                                 \
    TUI_MENU_ITEM_DEF_LIST_END                                          \
};

DEFINE_DRIVE_IDLE_METHOD_SUBMENU(8)
DEFINE_DRIVE_IDLE_METHOD_SUBMENU(9)
DEFINE_DRIVE_IDLE_METHOD_SUBMENU(10)
DEFINE_DRIVE_IDLE_METHOD_SUBMENU(11)

TUI_MENU_DEFINE_RADIO(Drive8ParallelCable)
TUI_MENU_DEFINE_RADIO(Drive9ParallelCable)
TUI_MENU_DEFINE_RADIO(Drive10ParallelCable)
TUI_MENU_DEFINE_RADIO(Drive11ParallelCable)

static TUI_MENU_CALLBACK(drive_parallel_cable_submenu_callback)
{
    int value;

    switch ((int)param) {
        default:
        case 8:
            resources_get_int("Drive8ParallelCable", &value);
            break;
        case 9:
            resources_get_int("Drive9ParallelCable", &value);
            break;
        case 10:
            resources_get_int("Drive10ParallelCable", &value);
            break;
        case 11:
            resources_get_int("Drive11ParallelCable", &value);
            break;
    }

    switch (value) {
        case DRIVE_PC_NONE:
            return "None";
        case DRIVE_PC_STANDARD:
            return "Standard";
        case DRIVE_PC_DD3:
            return "Dolphin Dos 3";
        case DRIVE_PC_FORMEL64:
            return "Formel64";
        default:
            return "(Unknown)";
    }
}

#define DEFINE_DRIVE_PARALLEL_CABLE_SUBMENU(num)                     \
static tui_menu_item_def_t drive##num##_parallel_cable_submenu[] = { \
    { "_None",                                                       \
      "No parallel cable",                                           \
      radio_Drive##num##ParallelCable_callback,                      \
      (void *)DRIVE_PC_NONE, 0,                                      \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                              \
    { "_Standard",                                                   \
      "Standard parallel cable",                                     \
      radio_Drive##num##ParallelCable_callback,                      \
      (void *)DRIVE_PC_STANDARD, 0,                                  \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                              \
    { "_Dolphin DOS 3",                                              \
      "Standard parallel cable",                                     \
      radio_Drive##num##ParallelCable_callback,                      \
      (void *)DRIVE_PC_DD3, 0,                                       \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                              \
    { "_Formel64",                                                   \
      "Standard parallel cable",                                     \
      radio_Drive##num##ParallelCable_callback,                      \
      (void *)DRIVE_PC_FORMEL64, 0,                                  \
      TUI_MENU_BEH_CLOSE, NULL, NULL },                              \
    TUI_MENU_ITEM_DEF_LIST_END                                       \
};

DEFINE_DRIVE_PARALLEL_CABLE_SUBMENU(8)
DEFINE_DRIVE_PARALLEL_CABLE_SUBMENU(9)
DEFINE_DRIVE_PARALLEL_CABLE_SUBMENU(10)
DEFINE_DRIVE_PARALLEL_CABLE_SUBMENU(11)

#define DEFINE_DRIVE_EXPANSIONS_SUBMENU(num)                         \
TUI_MENU_DEFINE_TOGGLE(Drive##num##RAM2000)                          \
TUI_MENU_DEFINE_TOGGLE(Drive##num##RAM4000)                          \
TUI_MENU_DEFINE_TOGGLE(Drive##num##RAM6000)                          \
TUI_MENU_DEFINE_TOGGLE(Drive##num##RAM8000)                          \
TUI_MENU_DEFINE_TOGGLE(Drive##num##RAMA000)                          \
TUI_MENU_DEFINE_TOGGLE(Drive##num##ProfDOS)                          \
TUI_MENU_DEFINE_TOGGLE(Drive##num##SuperCard)                        \
TUI_MENU_DEFINE_TOGGLE(Drive##num##RTCSave)                          \
                                                                     \
static tui_menu_item_def_t ui_drive_expanions_##num##_menu_def[] = { \
    { "RAM at $2000-$3FFF", "RAM at $2000-$3FFF",                    \
      toggle_Drive##num##RAM2000_callback, NULL, 3,                  \
      TUI_MENU_BEH_CONTINUE, NULL, NULL },                           \
    { "RAM at $4000-$5FFF", "RAM at $4000-$5FFF",                    \
      toggle_Drive##num##RAM4000_callback, NULL, 3,                  \
      TUI_MENU_BEH_CONTINUE, NULL, NULL },                           \
    { "RAM at $6000-$7FFF", "RAM at $6000-$7FFF",                    \
      toggle_Drive##num##RAM6000_callback, NULL, 3,                  \
      TUI_MENU_BEH_CONTINUE, NULL, NULL },                           \
    { "RAM at $8000-$9FFF", "RAM at $8000-$9FFF",                    \
      toggle_Drive##num##RAM8000_callback, NULL, 3,                  \
      TUI_MENU_BEH_CONTINUE, NULL, NULL },                           \
    { "RAM at $A000-$BFFF", "RAM at $A000-$BFFF",                    \
      toggle_Drive##num##RAMA000_callback, NULL, 3,                  \
      TUI_MENU_BEH_CONTINUE, NULL, NULL },                           \
    { "Professional DOS", "Professional DOS",                        \
      toggle_Drive##num##ProfDOS_callback, NULL, 3,                  \
      TUI_MENU_BEH_CONTINUE, NULL, NULL },                           \
    { "SuperCard+", "SuperCard+",                                    \
      toggle_Drive##num##SuperCard_callback, NULL, 3,                \
      TUI_MENU_BEH_CONTINUE, NULL, NULL },                           \
    { "Save FD2000/4000 RTC data when changed",                      \
      "Save FD2000/4000 RTC data when changed",                      \
      toggle_Drive##num##RTCSave_callback, NULL, 3,                  \
      TUI_MENU_BEH_CONTINUE, NULL, NULL },                           \
    TUI_MENU_ITEM_DEF_LIST_END                                       \
};

DEFINE_DRIVE_EXPANSIONS_SUBMENU(8)
DEFINE_DRIVE_EXPANSIONS_SUBMENU(9)
DEFINE_DRIVE_EXPANSIONS_SUBMENU(10)
DEFINE_DRIVE_EXPANSIONS_SUBMENU(11)

static TUI_MENU_CALLBACK(ui_set_rpm_callback)
{
    int num = (int)param;

    if (been_activated) {
        int current_rpm, value;
        char buf[10];

        resources_get_int_sprintf("Drive%iRPM", &current_rpm, num);
        sprintf(buf, "%d", current_rpm);

        if (tui_input_string("RPM", "Enter the RPM (multiplied by 1000):", buf, 10) == 0) {
            value = atoi(buf);
            resources_set_int_sprintf("Drive%iRPM", value, num);
        } else {
            return NULL;
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(ui_set_wobble_callback)
{
    int num = (int)param;

    if (been_activated) {
        int current_wobble, value;
        char buf[10];

        resources_get_int_sprintf("Drive%iWobble", &current_wobble, num);
        sprintf(buf, "%d", current_wobble);

        if (tui_input_string("Wobble", "Enter the wobble:", buf, 10) == 0) {
            value = atoi(buf);
            resources_set_int_sprintf("Drive%iWobble", value, num);
        } else {
            return NULL;
        }
    }
    return NULL;
}

tui_menu_item_def_t drivec128_drive8_settings_submenu[] = {
    { "Drive #_8 model:",
      "Specify model for drive #8",
      drive_type_submenu_callback, (void *)8, 26,
      TUI_MENU_BEH_CONTINUE, drive8_type_submenu,
      "Drive 8 model" },
    { "Drive #8 idle method:",
      "Specify idle method for drive #8",
      drive_idle_method_submenu_callback, (void *)8, 12,
      TUI_MENU_BEH_CONTINUE, drive8_idle_method_submenu,
      "Drive 8 idle method" },
    { "Drive #8 Parallel Cable:",
      "Specify parallel cable for drive #8",
      drive_parallel_cable_submenu_callback, (void *)8, 12,
      TUI_MENU_BEH_CONTINUE, drive8_parallel_cable_submenu,
      "Drive 8 parallel cable" },
    { "Drive #8 40-Track Image Support:",
      "Settings for dealing with 40-track disk images in drive #8",
      drive_extend_image_policy_submenu_callback, (void *)8, 16,
      TUI_MENU_BEH_CONTINUE, drive8_extend_image_policy_submenu, "" },
    { "Drive #8 Expansions...",
      "Drive #8 Expansions", 
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_drive_expanions_8_menu_def, NULL },
    { "Drive #8 RPM",
      "Set the RPM for drive #8",
      ui_set_rpm_callback, (void *)8, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Drive #8 wobble",
      "Set the wobble for drive #8",
      ui_set_wobble_callback, (void *)8, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

tui_menu_item_def_t drivec128_drive9_settings_submenu[] = {
    { "Drive #_9 model:",
      "Specify model for drive #9",
      drive_type_submenu_callback, (void *)9, 26,
      TUI_MENU_BEH_CONTINUE, drive9_type_submenu,
      "Drive 9 model" },
    { "Drive #9 idle method:",
      "Specify idle method for drive #9",
      drive_idle_method_submenu_callback, (void *)9, 12,
      TUI_MENU_BEH_CONTINUE, drive9_idle_method_submenu,
      "Drive 9 idle method" },
    { "Drive #9 Parallel Cable:",
      "Specify parallel cable for drive #9",
      drive_parallel_cable_submenu_callback, (void *)9, 12,
      TUI_MENU_BEH_CONTINUE, drive9_parallel_cable_submenu,
      "Drive 9 parallel cable" },
    { "Drive #9 40-Track Image Support:",
      "Settings for dealing with 40-track disk images in drive #9",
      drive_extend_image_policy_submenu_callback, (void *)9, 16,
      TUI_MENU_BEH_CONTINUE, drive9_extend_image_policy_submenu, "" },
    { "Drive #9 Expansions...",
      "Drive #9 Expansions", 
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_drive_expanions_9_menu_def, NULL },
    { "Drive #9 RPM",
      "Set the RPM for drive #9",
      ui_set_rpm_callback, (void *)9, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Drive #9 wobble",
      "Set the wobble for drive #9",
      ui_set_wobble_callback, (void *)9, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

tui_menu_item_def_t drivec128_drive10_settings_submenu[] = {
    { "Drive #1_0 model:",
      "Specify model for drive #10",
      drive_type_submenu_callback, (void *)10, 26,
      TUI_MENU_BEH_CONTINUE, drive10_type_submenu,
      "Drive 10 model" },
    { "Drive #10 idle method:",
      "Specify idle method for drive #10",
      drive_idle_method_submenu_callback, (void *)10, 12,
      TUI_MENU_BEH_CONTINUE, drive10_idle_method_submenu,
      "Drive 10 idle method" },
    { "Drive #10 Parallel Cable:",
      "Specify parallel cable for drive #10",
      drive_parallel_cable_submenu_callback, (void *)10, 12,
      TUI_MENU_BEH_CONTINUE, drive10_parallel_cable_submenu,
      "Drive 10 parallel cable" },
    { "Drive #10 40-Track Image Support:",
      "Settings for dealing with 40-track disk images in drive #10",
      drive_extend_image_policy_submenu_callback, (void *)10, 16,
      TUI_MENU_BEH_CONTINUE, drive10_extend_image_policy_submenu, "" },
    { "Drive #10 Expansions...",
      "Drive #10 Expansions", 
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_drive_expanions_10_menu_def, NULL },
    { "Drive #10 RPM",
      "Set the RPM for drive #10",
      ui_set_rpm_callback, (void *)10, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Drive #10 wobble",
      "Set the wobble for drive #10",
      ui_set_wobble_callback, (void *)10, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

tui_menu_item_def_t drivec128_drive11_settings_submenu[] = {
    { "Drive #_11 model:",
      "Specify model for drive #11",
      drive_type_submenu_callback, (void *)11, 26,
      TUI_MENU_BEH_CONTINUE, drive11_type_submenu,
      "Drive 11 model" },
    { "Drive #11 idle method:",
      "Specify idle method for drive #11",
      drive_idle_method_submenu_callback, (void *)11, 12,
      TUI_MENU_BEH_CONTINUE, drive11_idle_method_submenu,
      "Drive 11 idle method" },
    { "Drive #11 Parallel Cable:",
      "Specify parallel cable for drive #11",
      drive_parallel_cable_submenu_callback, (void *)11, 12,
      TUI_MENU_BEH_CONTINUE, drive11_parallel_cable_submenu,
      "Drive 11 parallel cable" },
    { "Drive #11 40-Track Image Support:",
      "Settings for dealing with 40-track disk images in drive #11",
      drive_extend_image_policy_submenu_callback, (void *)11, 16,
      TUI_MENU_BEH_CONTINUE, drive11_extend_image_policy_submenu, "" },
    { "Drive #11 Expansions...",
      "Drive #11 Expansions", 
      NULL, NULL, 0,
      TUI_MENU_BEH_CONTINUE, ui_drive_expanions_11_menu_def, NULL },
    { "Drive #11 RPM",
      "Set the RPM for drive #11",
      ui_set_rpm_callback, (void *)11, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Drive #11 wobble",
      "Set the wobble for drive #11",
      ui_set_wobble_callback, (void *)11, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    TUI_MENU_ITEM_DEF_LIST_END
};

tui_menu_item_def_t drivec128_settings_submenu[] = {
    { "True Drive _Emulation:",
      "Enable hardware-level floppy drive emulation",
      toggle_DriveTrueEmulation_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "--" },
    { "Drive #_8 settings:",
      "Drive #8 settings",
      NULL, NULL, 26,
      TUI_MENU_BEH_CONTINUE, drivec128_drive8_settings_submenu,
      "Drive 8 settings" },
    { "Drive #_9 settings:",
      "Drive #9 settings",
      NULL, NULL, 26,
      TUI_MENU_BEH_CONTINUE, drivec128_drive9_settings_submenu,
      "Drive 9 settings" },
    { "Drive #1_0 settings:",
      "Drive #10 settings",
      NULL, NULL, 26,
      TUI_MENU_BEH_CONTINUE, drivec128_drive10_settings_submenu,
      "Drive 10 settings" },
    { "Drive #_11 settings:",
      "Drive #11 settings",
      NULL, NULL, 26,
      TUI_MENU_BEH_CONTINUE, drivec128_drive11_settings_submenu,
      "Drive 11 settings" },
    TUI_MENU_ITEM_DEF_LIST_END
};
