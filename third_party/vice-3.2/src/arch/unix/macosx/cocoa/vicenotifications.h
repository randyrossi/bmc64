/*
 * vicenotifications.h - MacVICE notifications
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

// Machine Init Done
#define VICEMachineInitDoneNotification           @"VICEMachineInitDoneNotification"

// Display in VICE Window
#define VICEDisplayRecordingNotification          @"VICEDisplayRecordingNotification"
#define VICEDisplayPlaybackNotification           @"VICEDisplayPlaybackNotification"
#define VICEDisplayEventTimeNotification          @"VICEDisplayEventTimeNotification"
#define VICEDisplaySpeedNotification              @"VICEDisplaySpeedNotification"
#define VICEDisplayJoystickNotification           @"VICEDisplayJoystickNotification"

// Drive Status Display
#define VICEEnableDriveStatusNotification         @"VICEEnableDriveStatusNotification"
#define VICEDisplayDriveLedNotification           @"VICEDisplayDriveLedNotification"
#define VICEDisplayDriveImageNotification         @"VICEDisplayDriveImageNotification"
#define VICEDisplayDriveTrackNotification         @"VICEDisplayDriveTrackNotification"

// Tape Status Display
#define VICEEnableTapeStatusNotification          @"VICEEnableTapeStatusNotification"
#define VICEDisplayTapeImageNotification          @"VICEDisplayTapeImageNotification"
#define VICEDisplayTapeControlStatusNotification  @"VICEDisplayTapeControlStatusNotification"
#define VICEDisplayTapeMotorStatusNotification    @"VICEDisplayTapeMotorStatusNotification"
#define VICEDisplayTapeCounterNotification        @"VICEDisplayTapeCounterNotification"

// Pause State
#define VICETogglePauseNotification               @"VICETogglePauseNotification"

// Mouse & Lightpen
#define VICEToggleMouseNotification               @"VICEToggleMouseNotification"
#define VICEToggleLightpenNotification            @"VICEToggleLightpenNotification"

// Resource Updates
#define VICEChangedResourcesNotification          @"VICEResourcesChanged"

// Volume Change
#define VICEChangeVolumeNotification              @"VICEChangeVolumeNotification"

// Monitor notification from Application
#define VICEMonitorStateNotification              @"VICEMonitorStateNotification"
#define VICEMonitorStateOff             0
#define VICEMonitorStateOn              1
#define VICEMonitorStateSuspend         2
#define VICEMonitorStateResume          3
#define VICEMonitorUpdateNotification             @"VICEMonitorUpdateNotification"

