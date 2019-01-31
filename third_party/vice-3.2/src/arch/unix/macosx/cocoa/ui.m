/*
 * ui.m - MacVICE UI callbacks
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *  Michael Klein <michael.klein@puffin.lb.shuttle.de>
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

#include "types.h"
#include "uiapi.h"
#include "archdep.h"

#import "vicemachine.h"
#import "viceapplication.h"

#include "interrupt.h"

// ----- VICEMachineNotifier -----

int ui_init_finish(void)
{
    return 0;
}

int ui_init_finalize(void)
{
    // tell all that the machine init is now done
    [[theVICEMachine machineNotifier] postMachineInitDoneNotification];

    // report an resouce update to the UI
    [[theVICEMachine machineNotifier] postChangedResourcesNotification];
    
    return 0;
}

int ui_init(int *argc, char **argv)
{
    return 0;
}

void ui_shutdown(void)
{
}

// ----- pause stuff -----
static void pause_trap(uint16_t addr, void *data)
{
    vsync_suspend_speed_eval();
    while ([theVICEMachine isPaused]) {
        ui_dispatch_next_event();
    }
}

int ui_emulation_is_paused(void)
{
    return [theVICEMachine isPaused];
}

int ui_pause_emulation(int flag)
{
    if (flag && !ui_emulation_is_paused()) {
        [theVICEMachine setPaused:1];
        interrupt_maincpu_trigger_trap(pause_trap, 0);
    } else {
        [theVICEMachine setPaused:0];
    }
}

// ----- Drive Status Display -----

void ui_enable_drive_status(ui_drive_enable_t enable, int * color)
{
    int enableStatus = (int)enable;
    [[theVICEMachine machineNotifier] setEnableDriveStatus:(int)enableStatus color:color];
    [[theVICEMachine machineNotifier] postEnableDriveStatusNotification];
}

void ui_display_drive_track(unsigned int drive_number, unsigned int drive_base,
                            unsigned int half_track_number)
{
    [[theVICEMachine machineNotifier] postDisplayDriveTrackNotification:drive_number
                                               driveBase:drive_base
                                               halfTrack:half_track_number];
}

void ui_display_drive_led(int drive_number, unsigned int pwm1, unsigned int pwm2)
{
    [[theVICEMachine machineNotifier] postDisplayDriveLedNotification:drive_number 
                                                  pwm1:pwm1
                                                  pwm2:pwm2];
}

void ui_display_drive_current_image(unsigned int drive_number,
                                    const char *image)
{
    NSString *string = [NSString stringWithCString:image encoding:NSUTF8StringEncoding];
    [[theVICEMachine machineNotifier] postDisplayDriveImageNotification:drive_number
                                                   image:string];
}

// ----- Tape Status & Control -----

void ui_set_tape_status(int enable)
{
    [[theVICEMachine machineNotifier] postEnableTapeStatusNotification:enable];
}

void ui_display_tape_current_image(const char *image)
{
    NSString *string = [NSString stringWithCString:image encoding:NSUTF8StringEncoding];
    [[theVICEMachine machineNotifier] postDisplayTapeImageNotification:string];
}

void ui_display_tape_control_status(int control)
{
    [[theVICEMachine machineNotifier] postDisplayTapeControlStatusNotification:control];
}    

void ui_display_tape_motor_status(int motor)
{
    [[theVICEMachine machineNotifier] postDisplayTapeMotorStatusNotification:motor];
}

void ui_display_tape_counter(int counter)
{
    [[theVICEMachine machineNotifier] postDisplayTapeCounterNotification:counter];
}

// ---------- Status Messages ----------

void ui_display_speed(float speed, float frame_rate, int warp_enabled)
{
    [[theVICEMachine machineNotifier] postDisplaySpeedNotification:speed
                                          frameRate:frame_rate
                                        warpEnabled:(BOOL)warp_enabled];
}

// Event Recording/Playback

void ui_display_recording(int recording_status)
{
    [[theVICEMachine machineNotifier] postDisplayRecordingNotification:(BOOL)recording_status];
}

void ui_display_playback(int playback_status, char *version)
{
    NSString *string = [NSString stringWithCString:version ? version : ""  encoding:NSUTF8StringEncoding];
    [[theVICEMachine machineNotifier] postDisplayPlaybackNotification:(BOOL)playback_status
                                               version:string];
}

void ui_display_event_time(unsigned int current, unsigned int total)
{
    [[theVICEMachine machineNotifier] postDisplayEventTimeNotification:current totalTime:total];
}

void ui_display_statustext(const char *text, int fade_out)
{
    // TODO
    NSLog(@"Status: %s %d",text,fade_out);
}

void ui_display_joyport(uint8_t *joyport)
{
    [[theVICEMachine machineNotifier] postDisplayJoystickNotification:(int)joyport[1]
                                        secondJoystick:(int)joyport[2]];
}

void ui_display_volume(int vol)
{
    [[theVICEMachine machineNotifier] postVolumeChangeNotification:vol];
}

void ui_update_menus()
{
    // post a resource update
    [[theVICEMachine machineNotifier] postChangedResourcesNotification];
}

// ----- Alerts & Dialogs -----

void ui_error(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    NSString *string = [[NSString alloc] 
        initWithFormat:[NSString stringWithCString:format encoding:NSUTF8StringEncoding]
             arguments:args];
    va_end(args);

    [[theVICEMachine app] runErrorMessage:string];
    [string release];
}

void ui_message(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    NSString *string = [[NSString alloc] 
        initWithFormat:[NSString stringWithCString:format encoding:NSUTF8StringEncoding]
             arguments:args];
    va_end(args);

    [[theVICEMachine app] runWarningMessage:string];
    [string release];
}

ui_jam_action_t ui_jam_dialog(const char *format,...)
{
    va_list args;
    va_start(args, format);
    NSString * string = [[NSString alloc] 
        initWithFormat:[NSString stringWithCString:format encoding:NSUTF8StringEncoding]
             arguments:args];
    va_end(args);

    int result = [[theVICEMachine app] runCPUJamDialog:string];
    [string release];
    return result;
}

int ui_extend_image_dialog(void)
{
    BOOL yes = [[theVICEMachine app] runExtendImageDialog];
    return yes ? 1:0;
}

char* ui_get_file(const char *format,...)
{
    va_list args;
    va_start(args, format);
    NSString * string = [[NSString alloc] 
        initWithFormat:[NSString stringWithCString:format encoding:NSUTF8StringEncoding]
             arguments:args];
    va_end(args);

    NSString * path = [[theVICEMachine app] getOpenFileName:string types:nil];
    if (path != nil) {
        return strdup([path cStringUsingEncoding:NSUTF8StringEncoding]);
    } else {
        return NULL;
    }
}
