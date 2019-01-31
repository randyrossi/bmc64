/*
 * drivesettingswindowcontroller.m - DriveSettings dialog controller
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

#import "iecdrivesettingswindowcontroller.h"
#import "viceapplication.h"
#import "vicenotifications.h"
#import "viceappcontroller.h"

#include "drive.h"
#include "attach.h"

@implementation IECDriveSettingsWindowController

-(id)init
{
    self = [super initWithWindowNibName:@"IECDriveSettings"];

    // listen to resource changes
    [self registerForResourceUpdate:@selector(updateResources:)];

    // we need to listen to disk image changes, too
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(updateDiskImageName:)
                                                 name:VICEDisplayDriveImageNotification
                                               object:nil];

    // TODO: set emulator dependent
    driveOffset = 8;
    driveCount = 4;

    return self;
}

-(void)windowDidLoad
{
    // setup drive chooser
    [driveChooser setSegmentCount:driveCount];
    int i;
    for (i = 0; i < driveCount; i++) {
        NSString *driveName = [NSString stringWithFormat:@"Drive %d", i + driveOffset];
        [driveChooser setLabel:driveName forSegment:i];
    }
    [driveChooser setSelectedSegment:0];

    // setup dialog from resources
    [self updateResources:nil];

    [super windowDidLoad];
}

-(void)updateResources:(NSNotification *)notification
{
    int curDriveId = [driveChooser selectedSegment];
    int curDrive = curDriveId + driveOffset;

    // toggle IECDevice checkmark and selection
    int isIecDrive = [self getIntResource:@"IECDevice%d" withNumber:curDrive];
    [enableIECDrive setState:isIecDrive];
    int fileSystemDevice = [self getIntResource:@"FileSystemDevice%d" withNumber:curDrive];

    // set disk image name
    NSString *diskName = [[VICEApplication theMachineController] getDiskName:curDrive];
    [imagePath setStringValue:(diskName != nil) ? diskName : @""];
    BOOL isImagePath = isIecDrive && (fileSystemDevice == ATTACH_DEVICE_FS) && (diskName != nil);
    [imagePath setEnabled:isImagePath];

    // attach image read only
    int doAttachReadOnly = [self getIntResource:@"AttachDevice%dReadOnly" withNumber:curDrive];
    [attachReadOnly setState:doAttachReadOnly];
    [attachReadOnly setEnabled:isImagePath];

    // image buttons
    [attachImage setEnabled:isIecDrive];
    [autostartImage setEnabled:isIecDrive && (curDriveId == 0)];

    // dir path
    NSString *dir = [self getStringResource:@"FSDevice%dDir" withNumber:curDrive];
    [dirPath setStringValue:(dir != nil) ? dir : @""];
    BOOL isDirPath = isIecDrive && (fileSystemDevice == ATTACH_DEVICE_FS) && (diskName == nil);
    [dirPath setEnabled:isDirPath];

    // dir buttons
    [mountDir setEnabled:isIecDrive];

    // set state of dir options
    [readP00Files setEnabled:isDirPath];
    [writeP00Files setEnabled:isDirPath];
    [hideNonP00Files setEnabled:isDirPath];
    int doReadP00Files = [self getIntResource:@"FSDevice%dConvertP00" withNumber:curDrive];
    int doWriteP00Files = [self getIntResource:@"FSDevice%dSaveP00" withNumber:curDrive];
    int doHideNonP00Files = [self getIntResource:@"FSDevice%dHideCBMFiles" withNumber:curDrive];
    [readP00Files setState:doReadP00Files];
    [writeP00Files setState:doWriteP00Files];
    [hideNonP00Files setState:doHideNonP00Files];

    // other buttons
    [useOpenCBMDevice setEnabled:isIecDrive];
    [useRawFloppy setEnabled:isIecDrive];

    // status
    [self updateStatus];
}

-(void)updateStatus
{
    const char *names[5] = {
        "None",
        "File System",
        "Real Drive",
        "Raw Drive",
        "Disk Image"
    };

    int curDriveId = [driveChooser selectedSegment];
    int curDrive = curDriveId + driveOffset;

    int isIecDrive = [self getIntResource:@"IECDevice%d" withNumber:curDrive];
    int state = 0;
    if (isIecDrive) {
        state = [self getIntResource:@"FileSystemDevice%d" withNumber:curDrive];
        NSString *diskName = [[VICEApplication theMachineController] getDiskName:curDrive];
        if (state == 1 && (diskName != nil)) {
            state = 4;
        }
    }
    [status setStringValue:[NSString stringWithFormat:@"Type: %s", names[state]]];    
}

-(void)updateDiskImageName:(NSNotification *)notification
{
    NSDictionary * dict = [notification userInfo];
    int drive = [[dict objectForKey:@"drive"] intValue];
    int curDrive = [driveChooser selectedSegment];
    
    if (drive == curDrive) {
        NSString *image = [dict objectForKey:@"image"];
        [image retain];
        [imagePath setStringValue:image];
    }
}

// ----- Actions -----

-(IBAction)toggleDrive:(id)sender
{
    [self updateResources:nil];
}

-(IBAction)changeUseIECDrive:(id)sender
{
    int curDrive = [driveChooser selectedSegment] + driveOffset;
    int on = [sender state];
    [self setIntResource:@"IECDevice%d"
              withNumber:curDrive
                 toValue:on];

    // drive emu settings depend on this so notify
    [self triggerResourceUpdate:self];
}

-(IBAction)changeAttachReadOnly:(id)sender
{
    int curDrive = [driveChooser selectedSegment] + driveOffset;
    int on = [sender state];
    [self setIntResource:@"AttachDevice%dReadOnly"
              withNumber:curDrive
                 toValue:on];
}

-(IBAction)attachImage:(id)sender
{
    int curDrive = [driveChooser selectedSegment] + driveOffset;
    [self setIntResource:@"FileSystemDevice%d"
              withNumber:curDrive
                 toValue:ATTACH_DEVICE_FS];
    
    [[VICEApplication theAppController] attachDiskImageForUnit:curDrive];
    [self updateResources:nil];
}

-(IBAction)autostartImage:(id)sender
{
    int curDrive = [driveChooser selectedSegment] + driveOffset;
    [self setIntResource:@"FileSystemDevice%d"
              withNumber:curDrive
                 toValue:ATTACH_DEVICE_FS];

    [[VICEApplication theAppController] smartAttachImage:self];
    [self updateResources:nil];
}

-(IBAction)changeReadP00Files:(id)sender
{
    int curDrive = [driveChooser selectedSegment] + driveOffset;
    int on = [sender state];
    [self setIntResource:@"FSDevice%dConvertP00"
              withNumber:curDrive
                 toValue:on];
}

-(IBAction)changeWriteP00Files:(id)sender
{
    int curDrive = [driveChooser selectedSegment] + driveOffset;
    int on = [sender state];
    [self setIntResource:@"FSDevice%dSaveP00"
              withNumber:curDrive
                 toValue:on];
}

-(IBAction)changeHideNonP00Files:(id)sender
{
    int curDrive = [driveChooser selectedSegment] + driveOffset;
    int on = [sender state];
    [self setIntResource:@"FSDevice%dHideCBMFiles"
              withNumber:curDrive
                 toValue:on];
}

-(IBAction)mountDir:(id)sender
{
    int curDrive = [driveChooser selectedSegment] + driveOffset;

    // get directory
    NSString *dir = [[[VICEApplication theAppController] getFilePanel] pickDirectoryWithTitle:@"Pick Directory to Mount"];
    if (dir == nil) {
        return;
    }
    [self setStringResource:@"FSDevice%dDir" withNumber:curDrive toValue:dir];
    
    // remove mounted image otherwise dir wont work
    NSString *diskName = [[VICEApplication theMachineController] getDiskName:curDrive];
    if (diskName!=nil) {
        [[VICEApplication theMachineController] detachDiskImage:curDrive];
    }
    
    [self setIntResource:@"FileSystemDevice%d"
              withNumber:curDrive
                 toValue:ATTACH_DEVICE_FS];
    [self updateResources:nil];
}

-(IBAction)activateOpenCMBDevice:(id)sender
{
    int curDrive = [driveChooser selectedSegment] + driveOffset;
    [self setIntResource:@"FileSystemDevice%d"
              withNumber:curDrive
                 toValue:ATTACH_DEVICE_REAL];
    [self updateResources:nil];
}

-(IBAction)activateRawFloppyDevice:(id)sender
{
    int curDrive = [driveChooser selectedSegment] + driveOffset;
    [self setIntResource:@"FileSystemDevice%d"
              withNumber:curDrive
                 toValue:ATTACH_DEVICE_RAW];
    [self updateResources:nil];
}

@end
