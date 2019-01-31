/*
 * joysticksettingswindowcontroller.h - JoystickSettings dialog controller
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
#import "keypressview.h"

#define JOY_INTERNAL
#include "config.h"
#include "joy.h"

enum {
    KEYSET_NORTH = 0,
    KEYSET_SOUTH = 1,
    KEYSET_WEST = 2,
    KEYSET_EAST = 3,
    KEYSET_NORTHWEST = 4,
    KEYSET_SOUTHWEST = 5,
    KEYSET_NORTHEAST = 6,
    KEYSET_SOUTHEAST = 7,
    KEYSET_FIRE = 8,
    KEYSET_SIZE = 9
} JoystickKeyset;

@interface JoystickSettingsWindowController : VICEResourceWindowController
{
    IBOutlet NSPopUpButton *joystick1Mode;
    IBOutlet NSPopUpButton *joystick2Mode;
    IBOutlet NSPopUpButton *joystickExtra1Mode;
    IBOutlet NSPopUpButton *joystickExtra2Mode;

    // Keyset Definition
    IBOutlet NSPopUpButton *keySetSelect;
    IBOutlet KeyPressView *keyPressView;
    IBOutlet NSButton *keySetNorth;
    IBOutlet NSButton *keySetSouth;
    IBOutlet NSButton *keySetWest;
    IBOutlet NSButton *keySetEast;
    IBOutlet NSButton *keySetNorthWest;
    IBOutlet NSButton *keySetNorthEast;
    IBOutlet NSButton *keySetSouthWest;
    IBOutlet NSButton *keySetSouthEast;
    IBOutlet NSButton *keySetFire;

    // HID Device
    IBOutlet NSPopUpButton *hidDeviceSelect;
    IBOutlet NSPopUpButton *hidName;
    IBOutlet NSButton      *hitRefresh;

    IBOutlet NSPopUpButton *hidXAxis;
    IBOutlet NSButton      *hidXDetect;
    IBOutlet NSTextField   *hidXThreshold;
    IBOutlet NSButton      *hidXLogical;
    IBOutlet NSTextField   *hidXMin;
    IBOutlet NSTextField   *hidXMax;

    IBOutlet NSPopUpButton *hidYAxis;
    IBOutlet NSButton      *hidYDetect;
    IBOutlet NSTextField   *hidYThreshold;
    IBOutlet NSButton      *hidYLogical;
    IBOutlet NSTextField   *hidYMin;
    IBOutlet NSTextField   *hidYMax;

    IBOutlet NSButton      *hidNorth;
    IBOutlet NSButton      *hidSouth;
    IBOutlet NSButton      *hidWest;
    IBOutlet NSButton      *hidEast;
    IBOutlet NSButton      *hidFire;
    IBOutlet NSButton      *hidAltFire;

    IBOutlet NSButton      *hidAFA;
    IBOutlet NSButton      *hidAFB;
    IBOutlet NSTextField   *hidAFAPress;
    IBOutlet NSTextField   *hidAFARelease;
    IBOutlet NSTextField   *hidAFBPress;
    IBOutlet NSTextField   *hidAFBRelease;
    
    IBOutlet NSPopUpButton *hidHat;
    IBOutlet NSButton      *hidHatDetect;

    // Keep an array of buttons
    NSButton *keyButtons[KEYSET_SIZE];
    NSButton *hidButtons[HID_NUM_BUTTONS];
    NSButton *hidAutoButtons[HID_NUM_AUTO_BUTTONS];
    NSTextField *hidAutoPress[HID_NUM_AUTO_BUTTONS];
    NSTextField *hidAutoRelease[HID_NUM_AUTO_BUTTONS];
}

-(void)updateResources:(NSNotification *)notification;

-(IBAction)changeJoystick1Mode:(id)sender;
-(IBAction)changeJoystick2Mode:(id)sender;
-(IBAction)changeJoystickExtra1Mode:(id)sender;
-(IBAction)changeJoystickExtra2Mode:(id)sender;

// Keyset Actions
// for all buttons:
-(IBAction)toggleKeyset:(id)sender;
-(IBAction)defineKeysetButton:(id)sender;
-(IBAction)clearKeyset:(id)sender;

// HID Actions
-(IBAction)toggleHidDevice:(id)sender;
-(IBAction)pickHidName:(id)sender;
-(IBAction)refreshHidList:(id)sender;

-(IBAction)pickXAxis:(id)sender;
-(IBAction)detectXAxis:(id)sender;
-(IBAction)setXThreshold:(id)sender;
-(IBAction)setXMin:(id)sender;
-(IBAction)setXMax:(id)sender;
-(IBAction)toggleXLogical:(id)sender;

-(IBAction)pickYAxis:(id)sender;
-(IBAction)detectYAxis:(id)sender;
-(IBAction)setYThreshold:(id)sender;
-(IBAction)setYMin:(id)sender;
-(IBAction)setYMax:(id)sender;
-(IBAction)toggleYLogical:(id)sender;

-(IBAction)defineHidButton:(id)sender;

-(IBAction)pickHat:(id)sender;
-(IBAction)detectHat:(id)sender;
-(IBAction)defineAFButton:(id)sender;
-(IBAction)setAFParam:(id)sender;

@end
