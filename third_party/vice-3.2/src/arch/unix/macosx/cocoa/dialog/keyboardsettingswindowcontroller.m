/*
 * keyboardsettingswindowcontroller.m - KeyboardSettings dialog controller
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

#import "keyboardsettingswindowcontroller.h"
#import "viceapplication.h"
#import "vicenotifications.h"
#import "viceappcontroller.h"

#include "machine.h"

@implementation KeyboardSettingsWindowController

-(id)init
{
    self = [super initWithWindowNibName:@"KeyboardSettings"];
    
    hasDE = (machine_class == VICE_MACHINE_C64) ||
            (machine_class == VICE_MACHINE_C64DTV);
    
    [self registerForResourceUpdate:@selector(updateResources:)];
        
    return self;
}

-(void)windowDidLoad
{
    [symDEFile setEnabled:hasDE];
    [symDEPickButton setEnabled:hasDE];
    if(!hasDE) {
        [keymapSelector removeRow:2];
    }
    
    [self updateResources:nil];
    [super windowDidLoad];
}

-(void)updateResources:(NSNotification *)notification
{
    // selector
    int index = [self getIntResource:@"KeymapIndex"];
    [keymapSelector selectCellAtRow:index column:0];

    // files
    NSString *symUSFileStr = [self getStringResource:@"KeymapSymFile"];
    NSString *posFileStr = [self getStringResource:@"KeymapPosFile"];
    [symUSFile setStringValue:symUSFileStr];
    [posFile setStringValue:posFileStr];

    if(hasDE) {
        NSString *symDEFileStr = [self getStringResource:@"KeymapSymDeFile"];
        [symDEFile setStringValue:symDEFileStr];
    }
}

// ----- Actions -----

NSString *tags[3] = { @"KeymapSymFile", @"KeymapSymDeFile", @"KeymapPosFile" };

-(IBAction)selectKeymap:(id)sender
{
    int index = [[sender selectedCell] tag];
    [self setIntResource:@"KeymapIndex" toValue:index];
}

-(IBAction)changedKeymapFile:(id)sender
{
    int index = [sender tag];
    [self setStringResource:tags[index] toValue:[sender stringValue]];
}

-(IBAction)dumpKeymap:(id)sender
{
    VICEAppController *appCtrl = [VICEApplication theAppController];
    NSString *path = [[appCtrl getFilePanel] pickSaveFileWithType:@"KeyMap"];
    if (path != nil) {
        BOOL ok = [[VICEApplication theMachineController] dumpKeymap:path];
        if (!ok) {
            [VICEApplication runErrorMessage:@"Error dumping Keymap!"];
        }
    }
}

-(IBAction)pickKeymap:(id)sender
{
    VICEAppController *appCtrl = [VICEApplication theAppController];
    NSString *path = [[appCtrl getFilePanel] pickOpenFileWithType:@"KeyMap"];
    if (path != nil) {
        int index = [sender tag];
        [self setStringResource:tags[index] toValue:path];
        [self updateResources:nil];
    }
}

@end
