/*
 * vicemachinenotifier.h - VICEMachineNotifer - post machine notifications
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
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

#include "drive.h"

//some Cocoa headers give compile errors if DEBUG is defined without a value
#ifdef DEBUG
#undef DEBUG
#define DEBUG 1
#endif
#import <Cocoa/Cocoa.h>

/* The VICEMachineNotifier posts notifications of the Machine Thread */

@interface VICEMachineNotifier : NSObject
{
    int driveEnableMask;
    int driveEnableColorMask;
    BOOL mouseEnabled;
    BOOL lightpenEnabled;
}

-(id)init;

// ----- Machine State -----
-(void)postMachineInitDoneNotification;

// ----- Drive Display -----
// set enabled drive status
-(void)setEnableDriveStatus:(int)enable color:(int *)color;
// post notification
-(void)postEnableDriveStatusNotification;

// display drive track
-(void)postDisplayDriveTrackNotification:(unsigned int)drive 
                               driveBase:(unsigned int)driveBase
                               halfTrack:(unsigned int)halfTrack;
// display drive led
-(void)postDisplayDriveLedNotification:(unsigned int)drive
                                  pwm1:(unsigned int)pwm1
                                  pwm2:(unsigned int)pwm2;

// display image name
-(void)postDisplayDriveImageNotification:(unsigned int)drive
                                   image:(NSString *)image;

// ----- Tape Display -----
// display tape status
-(void)postEnableTapeStatusNotification:(int)enable;

// display image name
-(void)postDisplayTapeImageNotification:(NSString *)image;

// display control status
-(void)postDisplayTapeControlStatusNotification:(int)control;

// display motor status
-(void)postDisplayTapeMotorStatusNotification:(int)motor;

// display counter
-(void)postDisplayTapeCounterNotification:(int)counter;

// ----- Status Message -----
// display emulator speed
-(void)postDisplaySpeedNotification:(float)speed
                          frameRate:(float)frameRate
                        warpEnabled:(BOOL)warpEnabled;
                        
// display recording notification
-(void)postDisplayRecordingNotification:(BOOL)on;

// display playback notification
-(void)postDisplayPlaybackNotification:(BOOL)on version:(NSString *)version;

// display event time notification
-(void)postDisplayEventTimeNotification:(int)time totalTime:(int)totalTime;

// display joystick
-(void)postDisplayJoystickNotification:(int)joy1 secondJoystick:(int)joy2;

// post a remote changed resources notification
- (void)postChangedResourcesNotification;

// post a toggle mouse notification
- (void)postToggleMouseNotification:(BOOL)on;

// post a toggle lightpen notification
- (void)postToggleLightpenNotification:(BOOL)on;

// post a volume change
- (void)postVolumeChangeNotification:(int)volume;

// post a remote notification
- (void)postNotificationName:(NSString *)notificationName 
                    userInfo:(NSDictionary *)userInfo;

// prepare new window and re-post necessary notifications
-(void)notifyNewWindow;

@end

