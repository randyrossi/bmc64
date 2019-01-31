/*
 * iecdrivesettingswindowcontroller.h - IECDriveSettings dialog controller
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


#import <Cocoa/Cocoa.h>
#import "viceresourcewindowcontroller.h"

@class VICEAppController;

@interface IECDriveSettingsWindowController : VICEResourceWindowController
{
    IBOutlet NSSegmentedControl *driveChooser;

    IBOutlet NSButton *enableIECDrive;

    IBOutlet NSTextField *imagePath;
    IBOutlet NSButton *attachReadOnly;
    IBOutlet NSButton *attachImage;
    IBOutlet NSButton *autostartImage;

    IBOutlet NSTextField *dirPath;
    IBOutlet NSButton *readP00Files;
    IBOutlet NSButton *writeP00Files;
    IBOutlet NSButton *hideNonP00Files;
    IBOutlet NSButton *mountDir;

    IBOutlet NSButton *useOpenCBMDevice;
    IBOutlet NSButton *useRawFloppy;

    IBOutlet NSTextField *status;

    int driveOffset;
    int driveCount;
}

-(void)updateResources:(NSNotification *)notification;
-(void)updateDiskImageName:(NSNotification *)notification;
-(void)updateStatus;

-(IBAction)toggleDrive:(id)sender;

-(IBAction)changeUseIECDrive:(id)sender;

-(IBAction)changeAttachReadOnly:(id)sender;
-(IBAction)attachImage:(id)sender;
-(IBAction)autostartImage:(id)sender;

-(IBAction)changeReadP00Files:(id)sender;
-(IBAction)changeWriteP00Files:(id)sender;
-(IBAction)changeHideNonP00Files:(id)sender;
-(IBAction)mountDir:(id)sender;

-(IBAction)activateOpenCMBDevice:(id)sender;
-(IBAction)activateRawFloppyDevice:(id)sender;

@end
