/*
 * drivesettingswindowcontroller.h - DriveSettings dialog controller
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
#import "viceresourcewindowcontroller.h"

@interface DriveSettingsWindowController : VICEResourceWindowController
{
    IBOutlet NSSegmentedControl *driveChooser;

    IBOutlet NSBox *idleBox;
    IBOutlet NSBox *expansionBox;

    IBOutlet NSMatrix *driveType;
    IBOutlet NSMatrix *trackHandling;
    IBOutlet NSMatrix *idleMethod;
    IBOutlet NSMatrix *parallelCable;
    IBOutlet NSButton *driveExpansion_2000;
    IBOutlet NSButton *driveExpansion_4000;
    IBOutlet NSButton *driveExpansion_6000;
    IBOutlet NSButton *driveExpansion_8000;
    IBOutlet NSButton *driveExpansion_A000;
    IBOutlet NSTextField *RPM;
    IBOutlet NSTextField *wobble;

    int driveTypeMap[DRIVE_TYPE_NUM];
    int numDriveTypes;
    int driveOffset;
    int driveCount;

    BOOL hasIEC;
    BOOL hasParallel;
    BOOL hasExpansion;
    BOOL hasIdle;
}

-(void)updateResources:(NSNotification *)notification;

-(IBAction)toggleDrive:(id)sender;
-(IBAction)changedDriveType:(id)sender;
-(IBAction)changedTrackHandling:(id)sender;
-(IBAction)changedDriveExpansion2000:(id)sender;
-(IBAction)changedDriveExpansion4000:(id)sender;
-(IBAction)changedDriveExpansion6000:(id)sender;
-(IBAction)changedDriveExpansion8000:(id)sender;
-(IBAction)changedDriveExpansionA000:(id)sender;
-(IBAction)changedIdleMethod:(id)sender;
-(IBAction)toggledParallelCable:(id)sender;
-(IBAction)changedRPM:(NSTextField*)sender;
-(IBAction)changedWobble:(NSTextField*)sender;

-(int)mapToDriveType:(int)driveId;
-(int)mapFromDriveType:(int)driveType;

@end
