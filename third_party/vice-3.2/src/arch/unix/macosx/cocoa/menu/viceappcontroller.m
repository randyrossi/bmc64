/*
 * viceappcontroller.m - VICEAppController - base app controller class
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

#include "info.h"
#include "sid.h"

#import "viceappcontroller.h"
#import "viceapplication.h"
#import "vicenotifications.h"
#import "vicemachinecontroller.h"

#import "drivesettingswindowcontroller.h"
#import "iecdrivesettingswindowcontroller.h"
#import "printersettingswindowcontroller.h"
#import "keyboardsettingswindowcontroller.h"
#import "joysticksettingswindowcontroller.h"
#import "sidsettingswindowcontroller.h"
#import "soundsettingswindowcontroller.h"
#import "videosettingswindowcontroller.h"

#import "viceinformationwindowcontroller.h"
#import "resourceeditorcontroller.h"
#import "recordhistorywindowcontroller.h"
#import "recordmediawindowcontroller.h"
#import "netplaycontrolwindowcontroller.h"

@implementation VICEAppController

-(void)awakeFromNib
{
    // register resource updates
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(updateResources:)
                                                 name:VICEChangedResourcesNotification
                                               object:nil];
}

-(void)dealloc
{
    [driveSettingsController release];
    [iecDriveSettingsController release];
    [printerSettingsController release];
    [keyboardSettingsController release];
    [joystickSettingsController release];
    [sidSettingsController release];
    [soundSettingsController release];
    [video0SettingsController release];
    [video1SettingsController release];
    
    [infoController release];
    [resourceEditorController release];
    [recordHistoryController release];
    [recordMediaController release];
    [netplayController release];
    
    [filePanel release];
    
    [super dealloc];
}

// ----- Machine -----

- (IBAction)togglePause:(id)sender
{
    BOOL isPaused = [[VICEApplication theMachine] togglePause];
    [sender setState:isPaused?NSOnState:NSOffState]; 

    // post notification about pause change
    [[NSNotificationCenter defaultCenter] postNotificationName:VICETogglePauseNotification
                                                        object:[NSNumber numberWithBool:isPaused]
                                                      userInfo:nil];
}

- (IBAction)resetMachine:(id)sender
{
    [[VICEApplication theMachineController] resetMachine:[sender tag]];
}

- (IBAction)resetDrive:(id)sender
{
    [[VICEApplication theMachineController] resetDrive:[sender tag]];
}

- (IBAction)activateMonitor:(id)sender
{
    [[VICEApplication theMachineController] activateMonitor];
}

// ----- Disk Image -----

- (IBAction)smartAttachImage:(id)sender
{
    NSString *path = [[self getFilePanel] pickOpenFileWithPreviewAndType:@"CBMImage" allowRun:YES];
    if (path!=nil) {
        int progNum = [filePanel selectedProgNum];
        BOOL doRun  = [filePanel selectedAutostart];
        if (![[VICEApplication theMachineController] 
                smartAttachImage:path 
                     withProgNum:progNum
                          andRun:doRun])
            [VICEApplication runErrorMessage:@"Error attaching image!"];
    }
}

- (IBAction)attachDiskImage:(id)sender
{
    int unit = [sender tag];
    [self attachDiskImageForUnit:unit];
}

- (void)attachDiskImageForUnit:(int)unit
{    
    NSString *path = [[self getFilePanel] pickOpenFileWithPreviewAndType:@"DiskImage" allowRun:NO];
    if (path!=nil) {
        if (![[VICEApplication theMachineController] attachDiskImage:unit 
                                                               path:path]) {
            [VICEApplication runErrorMessage:@"Error attaching image!"];
        }
    }
}

- (IBAction)createAndAttachDiskImage:(id)sender
{
    NSArray *extensions = [NSArray arrayWithObjects:
        @"d64", @"d67", @"d71", @"d80", @"d81", @"d82", @"g64", @"p64", @"x64", @"d1m", @"d2m", @"d4m", nil];
    NSArray *labels = [NSArray arrayWithObjects:
        @"D64", @"D67", @"D71", @"D80", @"D81", @"D82", @"G64", @"P64", @"X64", @"D1m", @"D2m", @"D4m", nil];

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    NSSavePanel * panel = [NSSavePanel savePanel];
    NSView * accessories = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 310, 79)];
    [accessories autorelease];

    NSTextField * name_label = [[NSTextField alloc] initWithFrame:NSMakeRect(4, 46, 80, 17)];
    [name_label autorelease];
    [name_label setAlignment:NSRightTextAlignment];
    [name_label setEditable:NO];
    [name_label setBordered:NO];
    [name_label setDrawsBackground:NO];
    [name_label setStringValue:@"Name:"];

    NSTextField * name_entry = [[NSTextField alloc] initWithFrame:NSMakeRect(89, 44, 180, 22)];
    [name_entry autorelease];

    NSTextField * type_label = [[NSTextField alloc] initWithFrame:NSMakeRect(4, 18, 80, 17)];
    [type_label autorelease];
    [type_label setAlignment:NSRightTextAlignment];
    [type_label setEditable:NO];
    [type_label setBordered:NO];
    [type_label setDrawsBackground:NO];
    [type_label setStringValue:@"Type:"];

    NSPopUpButton * type_button = [[NSPopUpButton alloc] initWithFrame:NSMakeRect(86, 12, 98, 26)];
    [type_button addItemsWithTitles:labels];
    [type_button autorelease];

    [accessories addSubview:name_label];
    [accessories addSubview:name_entry];
    [accessories addSubview:type_button];
    [accessories addSubview:type_label];

    [panel setAccessoryView:accessories];
    [panel setTitle:@"Create Disk Image"];
    [panel setPrompt:@"Create"];

    if ([panel runModal] == NSFileHandlingPanelOKButton) {
        int type = [type_button indexOfSelectedItem];
        NSString * path = [[[panel URL] path] stringByAppendingPathExtension:[extensions objectAtIndex:type]];

        if (![[VICEApplication theMachineController] 
                    createDiskImage:type
                               path:path
                               name:[name_entry stringValue]]) {
            [VICEApplication runErrorMessage:@"Error creating image!"];
        } else {
            int unit = [sender tag];
            if (![[VICEApplication theMachineController] attachDiskImage:unit 
                                                                   path:path]) {
                [VICEApplication runErrorMessage:@"Error attaching image!"];
            }
        }
    }
    [pool release];
}

- (IBAction)detachDiskImage:(id)sender
{
    int unit = [sender tag];
    [[VICEApplication theMachineController] detachDiskImage:unit];
}

// ----- Fliplist -----

- (IBAction)fliplistAddCurrentImage:(id)sender
{
    int unit = [sender tag];
    [[VICEApplication theMachineController] addCurrentToFliplist:unit];
}

- (IBAction)fliplistRemoveCurrentImage:(id)sender
{
    int unit = [sender tag];
    [[VICEApplication theMachineController] removeFromFliplist:unit path:nil];
}

- (IBAction)fliplistAttachNextImage:(id)sender
{
    int unit = [sender tag];
    [[VICEApplication theMachineController] attachNextInFliplist:unit direction:TRUE];
}

- (IBAction)fliplistAttachPrevImage:(id)sender
{
    int unit = [sender tag];
    [[VICEApplication theMachineController] attachNextInFliplist:unit direction:TRUE];
}

- (IBAction)fliplistLoad:(id)sender
{
    int unit = [sender tag];
    NSString *path = [[self getFilePanel] pickOpenFileWithType:@"FlipList"];
    if (path!=nil) {
        if (![[VICEApplication theMachineController] loadFliplist:unit path:path autoAttach:TRUE])
            [VICEApplication runErrorMessage:@"Error loading fliplist!"];
    }
}

- (IBAction)fliplistSave:(id)sender
{
    int unit = [sender tag];
    NSString *path = [[self getFilePanel] pickSaveFileWithType:@"FlipList"];
    if (path!=nil) {
        if (![[VICEApplication theMachineController] saveFliplist:unit path:path])
            [VICEApplication runErrorMessage:@"Error saving fliplist!"];
    }
}

// ----- Tape Image -----

- (IBAction)attachTapeImage:(id)sender
{
    NSString *path = [[self getFilePanel] pickOpenFileWithPreviewAndType:@"TapeImage" allowRun:NO];
    if (path!=nil) {
        [[VICEApplication theMachineController] attachTapeImage:path];
    }
}

- (IBAction)detachTapeImage:(id)sender
{
    [[VICEApplication theMachineController] detachTapeImage];
}

// ----- Snapshot -----

- (IBAction)loadSnapshot:(id)sender
{
    NSString *path = [[self getFilePanel] pickOpenFileWithType:@"Snapshot"];
    if (path!=nil) {
        [[VICEApplication theMachineController] loadSnapshot:path];
    }
}

- (IBAction)saveSnapshot:(id)sender
{
    NSArray *types = [NSArray arrayWithObject:@"vsf"];

    NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
    NSSavePanel * panel = [NSSavePanel savePanel];
    NSView * accessories = [[NSView alloc] initWithFrame:NSMakeRect(0, 0, 125, 62)];
    [accessories autorelease];

    NSButton * saveRomsCheck = [[NSButton alloc] initWithFrame:NSMakeRect(16, 32, 91, 18)];
    [saveRomsCheck autorelease];
    [saveRomsCheck setButtonType:NSSwitchButton];
    [saveRomsCheck setTitle:@"Save ROMS"];

    NSButton * saveDisksCheck = [[NSButton alloc] initWithFrame:NSMakeRect(16, 12, 91, 18)];
    [saveDisksCheck autorelease];
    [saveDisksCheck setButtonType:NSSwitchButton];
    [saveDisksCheck setTitle:@"Save Disks"];

    [accessories addSubview:saveRomsCheck];
    [accessories addSubview:saveDisksCheck];

    [panel setAccessoryView:accessories];
    [panel setTitle:@"Save Snapshot"];
    [panel setAllowedFileTypes:types];

    if ([panel runModal] == NSFileHandlingPanelOKButton) {
        BOOL saveRoms  = ([saveRomsCheck state] == NSOnState);
        BOOL saveDisks = ([saveDisksCheck state] == NSOnState);
        NSString * path = [[panel URL] path];
        if (path!=nil) {
            [[VICEApplication theMachineController] saveSnapshot:path withROMS:saveRoms andDisks:saveDisks];
        }
    }
    [pool release];
}

- (IBAction)loadQuickSnapshot:(id)sender
{
    [[VICEApplication theMachineController] loadQuickSnapshot];
}

- (IBAction)saveQuickSnapshot:(id)sender
{
    [[VICEApplication theMachineController] saveQuickSnapshot];
}

- (IBAction)showRecordHistory:(id)sender
{
    if (!recordHistoryController) {
        recordHistoryController = [[RecordHistoryWindowController alloc] init];
    }
    [recordHistoryController showWindow:self];
}

- (IBAction)showRecordMedia:(id)sender
{
    if (!recordMediaController) {
        recordMediaController = [[RecordMediaWindowController alloc] init];
    }
    [recordMediaController showWindow:self];
}

- (IBAction)showNetplay:(id)sender
{
    if (!netplayController) {
        netplayController = [[NetplayControlWindowController alloc] init];
    }
    [netplayController showWindow:self];
}

// ----- Options -----

- (IBAction)pickRefreshRate:(id)sender
{
    int rate = [sender tag];
    [self setIntResource:@"RefreshRate" toValue:rate];
    [self updateOptionsResources];
}

- (IBAction)pickMaximumSpeed:(id)sender
{
    int speed = [sender tag];
    [self setIntResource:@"Speed" toValue:speed];
    [self updateOptionsResources];
}

- (IBAction)pickCustomMaximumSpeed:(id)sender
{
    //TODO
    [self setIntResource:@"Speed" toValue:42];
    [self updateOptionsResources];
}

- (IBAction)toggleWarpMode:(id)sender
{
    [self setIntResource:@"WarpMode" toValue:![sender state]];
    [self updateOptionsResources];
}

- (IBAction)toggleMachineVideoStandard:(id)sender
{
    [self setIntResource:@"MachineVideoStandard" toValue:[sender tag]];
    [self updateOptionsResources];
}

- (IBAction)toggleSoundPlayback:(id)sender
{
    [self setIntResource:@"Sound" toValue:![sender state]];
    [self updateOptionsResources];
}

- (IBAction)toggleTrueDriveEmulation:(id)sender
{
    [self setIntResource:@"DriveTrueEmulation" toValue:![sender state]];
    [self updateOptionsResources];
    [driveSettingsController updateResources:nil];
}

- (IBAction)toggleVirtualDevices:(id)sender
{
    [self setIntResource:@"VirtualDevices" toValue:![sender state]];
    [self updateOptionsResources];
}

- (IBAction)swapJoysticks:(id)sender
{
    int joyDev1 = [self getIntResource:@"JoyDevice1"];
    int joyDev2 = [self getIntResource:@"JoyDevice2"];
    [self setIntResource:@"JoyDevice1" toValue:joyDev2];
    [self setIntResource:@"JoyDevice2" toValue:joyDev1];
    [joystickSettingsController updateResources:nil];
}

- (IBAction)toggleKeySetEnable:(id)sender
{
    [self setIntResource:@"KeySetEnable" toValue:![sender state]];
    [self updateOptionsResources];
}

// Mouse

- (IBAction)toggleMouseEmulation:(id)sender
{
    [self setIntResource:@"Mouse" toValue:![sender state]];
    [self updateOptionsResources];
}

- (IBAction)toggleLightPenEmulation:(id)sender
{
    int state = ![sender state];
    [self setIntResource:@"Lightpen" toValue:state];
    [self updateOptionsResources];
    
    // directly post a notification here as lightpen driver has no callback
    NSDictionary * dict =
        [NSDictionary dictionaryWithObject:[NSNumber numberWithInt:state]
                                    forKey: @"lightpen"];
    [[NSNotificationCenter defaultCenter] postNotificationName:VICEToggleLightpenNotification
                                                        object:self
                                                      userInfo:dict];
}

// Video Rendering

- (IBAction)toggleVideo0VideoCache:(id)sender
{
    NSString *res = [NSString stringWithFormat:@"%@VideoCache", [video0Menu title]];
    [self setIntResource:res toValue:![sender state]];
    [self updateVideo0Resources];
}

- (IBAction)toggleVideo0DoubleSize:(id)sender
{
    NSString *res = [NSString stringWithFormat:@"%@DoubleSize", [video0Menu title]];
    [self setIntResource:res toValue:![sender state]];
    [self updateVideo0Resources];
}

- (IBAction)toggleVideo0DoubleScan:(id)sender
{
    NSString *res = [NSString stringWithFormat:@"%@DoubleScan", [video0Menu title]];
    [self setIntResource:res toValue:![sender state]];
    [self updateVideo0Resources];
}

- (IBAction)setVideo0Filter:(id)sender
{
    int type = [sender tag];
    NSString *res = [NSString stringWithFormat:@"%@Filter", [video0Menu title]];
    [self setIntResource:res toValue:type];
    [self updateVideo0Resources];
}

- (IBAction)toggleVideo1VideoCache:(id)sender
{
    NSString *res = [NSString stringWithFormat:@"%@VideoCache", [video1Menu title]];
    [self setIntResource:res toValue:![sender state]];
    [self updateVideo1Resources];
}

- (IBAction)toggleVideo1DoubleSize:(id)sender
{
    NSString *res = [NSString stringWithFormat:@"%@DoubleSize", [video1Menu title]];
    [self setIntResource:res toValue:![sender state]];
    [self updateVideo1Resources];
}

- (IBAction)toggleVideo1DoubleScan:(id)sender
{
    NSString *res = [NSString stringWithFormat:@"%@DoubleScan", [video1Menu title]];
    [self setIntResource:res toValue:![sender state]];
    [self updateVideo1Resources];
}

- (IBAction)setVideo1Filter:(id)sender
{
    int type = [sender tag];
    NSString *res = [NSString stringWithFormat:@"%@Filter", [video1Menu title]];
    [self setIntResource:res toValue:type];
    [self updateVideo1Resources];
}

// ----- Settings -----

- (IBAction)showDriveSettings:(id)sender
{
    if (!driveSettingsController) {
        driveSettingsController = [[DriveSettingsWindowController alloc] init];
    }
    [driveSettingsController showWindow:self];
}

- (IBAction)showIECDriveSettings:(id)sender
{
    if (!iecDriveSettingsController) {
        iecDriveSettingsController = [[IECDriveSettingsWindowController alloc] init];
    }
    [iecDriveSettingsController showWindow:self];
}

- (IBAction)showPrinterSettings:(id)sender
{
    if (!printerSettingsController) {
        printerSettingsController = [[PrinterSettingsWindowController alloc] init];
    }
    [printerSettingsController showWindow:self];
}

- (IBAction)showKeyboardSettings:(id)sender
{
    if (!keyboardSettingsController) {
        keyboardSettingsController = [[KeyboardSettingsWindowController alloc] init];
    }
    [keyboardSettingsController showWindow:self];
}

- (IBAction)showJoystickSettings:(id)sender
{
#ifdef HAS_JOYSTICK
    if (!joystickSettingsController) {
        joystickSettingsController = [[JoystickSettingsWindowController alloc] init];
    }
    [joystickSettingsController showWindow:self];
#endif
}

- (IBAction)showSIDSettings:(id)sender
{
    if (!sidSettingsController) {
        sidSettingsController = [[SIDSettingsWindowController alloc] init];
    }
    [sidSettingsController showWindow:self];
}

- (IBAction)showSoundSettings:(id)sender
{
    if (!soundSettingsController) {
        soundSettingsController = [[SoundSettingsWindowController alloc] init];
    }
    [soundSettingsController showWindow:self];
}

- (IBAction)showVideo0Settings:(id)sender
{
    if (!video0SettingsController) {
        video0SettingsController = [[VideoSettingsWindowController alloc] initWithChipName:[video0Menu title]];
    }
    [video0SettingsController showWindow:self];
}

- (IBAction)showVideo1Settings:(id)sender
{
    if (!video1SettingsController) {
        video1SettingsController = [[VideoSettingsWindowController alloc] initWithChipName:[video1Menu title]];
    }
    [video1SettingsController showWindow:self];
}

// ----- Resources -----

- (IBAction)showResourceEditor:(id)sender
{
    if (!resourceEditorController) {
        resourceEditorController = [[ResourceEditorController alloc] init];
    }
    [resourceEditorController showWindow:self];
}

- (IBAction)saveResources:(id)sender
{
    if (![[VICEApplication theMachineController] saveResources:nil]) {
        [VICEApplication runErrorMessage:@"Error saving Resources!"];
    }
}

- (IBAction)loadResources:(id)sender
{
    if (![[VICEApplication theMachineController] loadResources:nil]) {
        [VICEApplication runErrorMessage:@"Error loading Resources!"];
    };
}

- (IBAction)resetResources:(id)sender
{
    [[VICEApplication theMachineController] resetResources];    
}

- (IBAction)toggleSaveResourcesOnExit:(id)sender
{
    int toggle = ![sender state];
    [self setIntResource:@"SaveResourcesOnExit" toValue:toggle];
    [self updateSettingsResources];
}

- (IBAction)toggleConfirmOnExit:(id)sender
{
    int toggle = ![sender state];
    [self setIntResource:@"ConfirmOnExit" toValue:toggle];
    [self updateSettingsResources];    
}

// ----- Info -----

- (IBAction)showInformation:(id)sender
{
    static const char *tag2text[] = {
        info_license_text, info_warranty_text, info_contrib_text
    };
    const char *info = tag2text[[sender tag]];

    if (!infoController) {
        infoController = [[VICEInformationWindowController alloc] init];
    }
    [infoController showWindow:self];
    [infoController setInformationText:[NSString stringWithCString:info encoding:NSISOLatin1StringEncoding]];
}

// ----- Resource Updates -----

- (void)updateResources:(id)sender
{
    [self updateMachineResources];
    [self updateOptionsResources];
    [self updateSettingsResources];
    if(video0Menu != nil) {
        [self updateVideo0Resources];
    }
    if(video1Menu != nil) {
        [self updateVideo1Resources];
    }
}

- (void)updateMachineResources
{
}

- (BOOL)updateSubMenuCheckState:(NSMenu *)menu withTag:(int)tagValue
{
    int numItems = [menu numberOfItems];
    int i;
    BOOL foundTag = NO;
    for (i=0;i<numItems;i++) {
        NSMenuItem *item = [menu itemAtIndex:i];
        BOOL check = ([item tag] == tagValue);
        [item setState:check ? NSOnState : NSOffState];
        if (check)
            foundTag = YES;
    }
    return foundTag;
}


- (void)updateOptionsResources
{   
    // RefreshRate
    [self updateSubMenuCheckState:refreshRateMenu 
                          withTag:[self getIntResource:@"RefreshRate"]];
    
    // Speed
    BOOL foundSpeed = [self updateSubMenuCheckState:maximumSpeedMenu
                                withTag:[self getIntResource:@"Speed"]];
    [[maximumSpeedMenu itemAtIndex:7] setState:foundSpeed ? NSOffState : NSOnState];
    
    // WarpMode
    [warpModeMenuItem setState:[self getIntResource:@"WarpMode"]];
    
    // Machine Video Standard
    if(machineVideoStandardMenu != nil) {
        [self updateSubMenuCheckState:machineVideoStandardMenu 
            withTag:[self getIntResource:@"MachineVideoStandard"]];
    }
    
    // Sound
    [soundPlaybackMenuItem setState:[self getIntResource:@"Sound"]];
    // TrueDriveEmulation
    [trueDriveEmulationMenuItem setState:[self getIntResource:@"DriveTrueEmulation"]];
    // VirtaulDevices
    [virtualDevicesMenuItem setState:[self getIntResource:@"VirtualDevices"]];
    // KeySetEnable
    [keySetEnableMenuItem setState:[self getIntResource:@"KeySetEnable"]];

    // Mouse
    if(mouseEmulationMenuItem != nil) {
        [mouseEmulationMenuItem setState:[self getIntResource:@"Mouse"]];
    }
    // Lightpen
    if(lightPenEmulationMenuItem != nil) {
        [lightPenEmulationMenuItem setState:[self getIntResource:@"LightPen"]];
    }
}

- (void)updateVideo0Resources
{
    NSString *name = [video0Menu title];
    
    NSString *res1 = [NSString stringWithFormat:@"%@VideoCache", name];    
    [video0VideoCacheMenuItem setState:[self getIntResource:res1]];

    NSString *res2 = [NSString stringWithFormat:@"%@DoubleSize", name];    
    [video0DoubleSizeMenuItem setState:[self getIntResource:res2]];
    
    NSString *res3 = [NSString stringWithFormat:@"%@DoubleScan", name];        
    [video0DoubleScanMenuItem setState:[self getIntResource:res3]];
    
    NSString *res4 = [NSString stringWithFormat:@"%@Filter", name];    
    int type = [self getIntResource:res4];
    [video0Type0MenuItem setState:(type == 0)];
    [video0Type1MenuItem setState:(type == 1)];
    [video0Type2MenuItem setState:(type == 2)];
}

- (void)updateVideo1Resources
{
    NSString *name = [video1Menu title];
    
    NSString *res1 = [NSString stringWithFormat:@"%@VideoCache", name];    
    [video1VideoCacheMenuItem setState:[self getIntResource:res1]];

    NSString *res2 = [NSString stringWithFormat:@"%@DoubleSize", name];    
    [video1DoubleSizeMenuItem setState:[self getIntResource:res2]];
    
    NSString *res3 = [NSString stringWithFormat:@"%@DoubleScan", name];        
    [video1DoubleScanMenuItem setState:[self getIntResource:res3]];
    
    NSString *res4 = [NSString stringWithFormat:@"%@Filter", name];    
    int type = [self getIntResource:res4];
    [video1Type0MenuItem setState:(type == 0)];
    [video1Type1MenuItem setState:(type == 1)];
    [video1Type2MenuItem setState:(type == 2)];    
}

- (void)updateSettingsResources
{
    // SaveResourcesOnExit
    int saveResourcesOnExit = [self getIntResource:@"SaveResourcesOnExit"];
    [saveResourcesOnExitMenuItem setState:saveResourcesOnExit];
    
    // ConfirmOnExit
    int confirmOnExit = [self getIntResource:@"ConfirmOnExit"];
    [confirmOnExitMenuItem setState:confirmOnExit];
}

// ----- Tools -----

- (VICEFilePanel *)getFilePanel
{
    if(filePanel == nil) {
        filePanel = [[VICEFilePanel alloc] init];
    }
    return filePanel;
}

- (BOOL)setIntResource:(NSString *)name toValue:(int)value
{
    [[VICEApplication theMachineController] 
        setIntResource:name value:[NSNumber numberWithInt:value]];
    return TRUE;
}

- (int)getIntResource:(NSString *)name
{
    NSNumber *number = [[VICEApplication theMachineController]
        getIntResource:name];
    if (number==nil)
        return 0;
    return [number intValue];
}

@end
