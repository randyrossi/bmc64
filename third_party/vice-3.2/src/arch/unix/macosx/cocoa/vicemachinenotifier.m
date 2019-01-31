/*
 * vicemachinenotifier.m - VICEMachineNotifier - post machine notifications
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

#import "vicemachinenotifier.h"
#import "vicenotifications.h"
#import "vicemachine.h"
#import "viceapplication.h"

@implementation VICEMachineNotifier

-(id)init
{
    driveEnableMask = 0;
    driveEnableColorMask = 0;
    mouseEnabled = NO;
    lightpenEnabled = NO;
    return self;
}

// ----- Machine State -----

-(void)postMachineInitDoneNotification
{
    NSDictionary *dict = [NSDictionary dictionary];
    
    [self postNotificationName:VICEMachineInitDoneNotification
                      userInfo:dict];
}

// ----- Drive Notification -----

-(void)setEnableDriveStatus:(int)enable color:(int *)color
{
    driveEnableMask = enable;
    driveEnableColorMask = 0;
    // build color mask
    int i;
    for (i=0;i<DRIVE_NUM;i++) {
        if (color[i])
            driveEnableColorMask |= 1<<i;
    }
}

-(void)postEnableDriveStatusNotification
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithInt:driveEnableMask], @"enabled_drives",
            [NSNumber numberWithInt:driveEnableColorMask], @"drive_led_color",
            nil, nil];

    [self postNotificationName:VICEEnableDriveStatusNotification
                      userInfo:dict];
}

-(void)postDisplayDriveTrackNotification:(unsigned int)drive 
                               driveBase:(unsigned int)driveBase
                               halfTrack:(unsigned int)halfTrack
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObjectsAndKeys:
                [NSNumber numberWithInt:drive], @"drive",
                [NSNumber numberWithInt:driveBase], @"drive_base",
                [NSNumber numberWithFloat:(halfTrack)/2], @"track",
                nil, nil];

    [self postNotificationName:VICEDisplayDriveTrackNotification
                      userInfo:dict];
}

-(void)postDisplayDriveLedNotification:(unsigned int)drive
                                  pwm1:(unsigned int)pwm1
                                  pwm2:(unsigned int)pwm2
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithInt:drive], @"drive",
            [NSNumber numberWithBool:pwm1], @"active",
            nil, nil];
    
    [self postNotificationName:VICEDisplayDriveLedNotification
                      userInfo:dict];
}

-(void)postDisplayDriveImageNotification:(unsigned int)drive
                                   image:(NSString *)image
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithInt:drive], @"drive",
            image, @"image",
            nil, nil];

    [self postNotificationName:VICEDisplayDriveImageNotification
                      userInfo:dict];
}

// ----- Tape Notification -----

-(void)postEnableTapeStatusNotification:(int)enable
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:enable]
                                    forKey:@"enable"];

    [self postNotificationName:VICEEnableTapeStatusNotification
                      userInfo:dict];    
}

-(void)postDisplayTapeImageNotification:(NSString *)image
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObject:image
                                    forKey:@"image"];

    [self postNotificationName:VICEDisplayTapeImageNotification
                      userInfo:dict];
}

-(void)postDisplayTapeControlStatusNotification:(int)control
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:control]
                                    forKey:@"status"];

    [self postNotificationName:VICEDisplayTapeControlStatusNotification
                      userInfo:dict];
}

-(void)postDisplayTapeMotorStatusNotification:(int)motor
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:motor]
                                    forKey:@"status"];

    [self postNotificationName:VICEDisplayTapeMotorStatusNotification
                      userInfo:dict];
}

-(void)postDisplayTapeCounterNotification:(int)counter
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:counter]
                                    forKey: @"counter"];

    [self postNotificationName:VICEDisplayTapeCounterNotification
                      userInfo:dict];
}

-(void)postDisplayJoystickNotification:(int)joy1 secondJoystick:(int)joy2
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithInt:joy1], @"joy1",
            [NSNumber numberWithInt:joy2], @"joy2",
            nil, nil];
    
    [self postNotificationName:VICEDisplayJoystickNotification
                      userInfo:dict];
}

// ----- Status -----

-(void)postDisplaySpeedNotification:(float)speed
                          frameRate:(float)frameRate
                        warpEnabled:(BOOL)warpEnabled
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObjectsAndKeys:
                         [NSNumber numberWithFloat:speed], @"speed",
                         [NSNumber numberWithFloat:frameRate], @"frame_rate",
                         [NSNumber numberWithBool:warpEnabled], @"warp_enabled",
                         nil, nil];
    
    [self postNotificationName:VICEDisplaySpeedNotification
                      userInfo:dict];
}

-(void)postDisplayRecordingNotification:(BOOL)on
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:on]
                                    forKey: @"recording"];

    [self postNotificationName:VICEDisplayRecordingNotification
                      userInfo:dict];
}

-(void)postDisplayPlaybackNotification:(BOOL)on version:(NSString *)version
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObjectsAndKeys:
                         [NSNumber numberWithBool:on], @"playback",
                         version, @"version",
                         nil, nil];

    [self postNotificationName:VICEDisplayPlaybackNotification
                      userInfo:dict];
}

-(void)postDisplayEventTimeNotification:(int)time totalTime:(int)totalTime
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObjectsAndKeys:
                         [NSNumber numberWithInt:time], @"time",
                         [NSNumber numberWithInt:totalTime], @"totalTime",
                         nil, nil];

    [self postNotificationName:VICEDisplayEventTimeNotification
                      userInfo:dict];
}

// ----- Mouse & Lightpen -----

- (void)postToggleMouseNotification:(BOOL)on
{
    mouseEnabled = on;
    NSDictionary * dict =
        [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:on]
                                    forKey: @"mouse"];

    [self postNotificationName:VICEToggleMouseNotification
                      userInfo:dict];
}

- (void)postToggleLightpenNotification:(BOOL)on
{
    lightpenEnabled = on;
    NSDictionary * dict =
        [NSDictionary dictionaryWithObject:[NSNumber numberWithBool:on]
                                    forKey: @"lightpen"];

    [self postNotificationName:VICEToggleLightpenNotification
                      userInfo:dict];
}

// ----- Sound -----

- (void)postVolumeChangeNotification:(int)volume
{
    NSDictionary * dict =
        [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:volume]
                                    forKey: @"volume"];

    [self postNotificationName:VICEChangeVolumeNotification
                      userInfo:dict];
}

// ----- Resources -----

- (void)postChangedResourcesNotification
{
    NSDictionary *dict = [NSDictionary dictionary];
    
    [self postNotificationName:VICEChangedResourcesNotification
                      userInfo:dict];
}

// post a remote notification
- (void)postNotificationName:(NSString *)notificationName 
                    userInfo:(NSDictionary *)userInfo
{
    NSArray *array = [NSArray arrayWithObjects:notificationName, userInfo, nil];
#if 1
    // use non-blocking selector call in main thread
    [[VICEApplication sharedApplication] 
            performSelectorOnMainThread:@selector(postRemoteNotification:) 
                             withObject:array
                          waitUntilDone:NO];
#else
    // use proxy call to main thread - blocks if main thread
    [[theVICEMachine app] postRemoteNotification:array];
#endif
}


// ----- Common Tools -----

// re-post all notifications required for a new window
-(void)notifyNewWindow
{
    // repost drive enable state
    [self postEnableDriveStatusNotification];
    // repost mouse enable state
    [self postToggleMouseNotification:mouseEnabled];
    // repost lightpen enable state
    [self postToggleLightpenNotification:lightpenEnabled];
}

@end
