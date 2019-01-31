/*
 * printersettingswindowcontroller.m - PrinterSettings dialog controller
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

#include "vice.h"
#include "machine.h"

#import "printersettingswindowcontroller.h"
#import "viceapplication.h"
#import "vicenotifications.h"

#include "printer.h"

static const char *tag[3] = { "Userport", "4", "5" };

@implementation PrinterSettingsWindowController

-(id)init
{
    self = [super initWithWindowNibName:@"PrinterSettings"];
    [self registerForResourceUpdate:@selector(updateResources:)];
    return self;
}

-(void)windowDidLoad
{
    // determine machine setup
    hasIECPrinter = YES;
    hasUserportPrinter = YES;
    switch(machine_class) {
        case VICE_MACHINE_PLUS4:
            hasUserportPrinter = NO;
            break;
        case VICE_MACHINE_PET:
        case VICE_MACHINE_CBM5x0:
        case VICE_MACHINE_CBM6x0:
            hasIECPrinter = NO;
            break;
    }
    
    // setup printer selector
    if(!hasUserportPrinter) {
        [printerChooser setEnabled:NO forSegment:0];
        [printerChooser setSelectedSegment:1];
    }
    
    if(!hasIECPrinter) {
        [printerChooser setEnabled:NO forSegment:1];
        [printerChooser setEnabled:NO forSegment:2];
        [printerChooser setSelectedSegment:0];
    }
    
    [self updateResources:nil];
    [super windowDidLoad];
}

-(void)updateResources:(NSNotification *)notification
{
    int printerId = [printerChooser selectedSegment];

    // IEC Device
    [useIECDevice setHidden:(printerId == 0)];
    if (printerId>0) {
        int doUse = [self getIntResource:@"IECDevice%d" withNumber:printerId+3];
        [useIECDevice setState:doUse];
    }

    // printer emulation
    int emul = [self getIntResource:[NSString stringWithFormat:@"Printer%s", tag[printerId]]];
    [printerEmulation selectItemAtIndex:emul];

    // send form feed button
    [sendFormFeed setEnabled:(emul > 0)];

    // file output options
    BOOL hasFileOutput = (emul == PRINTER_DEVICE_FS);
    [driver setEnabled:hasFileOutput];
    [output setEnabled:hasFileOutput];
    [textDevice setEnabled:hasFileOutput];

    NSString *driverStr = [self getStringResource:
        [NSString stringWithFormat:@"Printer%sDriver", tag[printerId]]];
    NSString *outputStr = [self getStringResource:
        [NSString stringWithFormat:@"Printer%sOutput", tag[printerId]]];
    int textDeviceVal = [self getIntResource:
        [NSString stringWithFormat:@"Printer%sTextDevice", tag[printerId]]];
    [driver selectItemWithTitle:[driverStr uppercaseString]];
    [output selectItemWithTitle:[outputStr capitalizedString]];
    [textDevice selectItemAtIndex:textDeviceVal];

    // printer text devices
    NSString *textDevice1 = [self getStringResource:@"PrinterTextDevice1"];
    [printerTextDevice1 setStringValue:textDevice1];
    NSString *textDevice2 = [self getStringResource:@"PrinterTextDevice2"];
    [printerTextDevice2 setStringValue:textDevice2];
    NSString *textDevice3 = [self getStringResource:@"PrinterTextDevice3"];
    [printerTextDevice3 setStringValue:textDevice3];
    
    [printerTextDevice1 setEnabled:hasFileOutput];
    [printerTextDevice2 setEnabled:hasFileOutput];
    [printerTextDevice3 setEnabled:hasFileOutput];
}

// ----- Actions -----

-(IBAction)togglePrinter:(id)sender
{
    [self updateResources:nil];
}

-(IBAction)changedIECDevice:(id)sender
{
    int curPrinter = [printerChooser selectedSegment] + 3;
    int on = [sender state];
    [self setIntResource:@"IECDevice%d"
              withNumber:curPrinter
                 toValue:on];

    [self updateResources:nil];
}

-(IBAction)changedPrinterEmulation:(id)sender
{
    int curPrinter = [printerChooser selectedSegment];
    int emu = [sender indexOfSelectedItem];
    [self setIntResource:[NSString stringWithFormat:@"Printer%s",tag[curPrinter]]
                 toValue:emu];

    [self updateResources:nil];
}

-(IBAction)doSendFormFeed:(id)sender
{
    int curPrinter = [printerChooser selectedSegment];
    static const int map[3] = { 2, 0, 1 };

    [[VICEApplication theMachineController] printerFormFeed:map[curPrinter]];
}

-(IBAction)changedDriver:(id)sender
{
    int curPrinter = [printerChooser selectedSegment];
    NSMenuItem *cell = [sender selectedItem];
    [self setStringResource:[NSString stringWithFormat:@"Printer%sDriver", tag[curPrinter]]
                    toValue:[[cell title] lowercaseString]];
}

-(IBAction)changedOutput:(id)sender
{
    int curPrinter = [printerChooser selectedSegment];
    NSMenuItem *cell = [sender selectedItem];
    [self setStringResource:[NSString stringWithFormat:@"Printer%sOutput", tag[curPrinter]]
                    toValue:[[cell title] lowercaseString]];
}

-(IBAction)changedTextDevice:(id)sender
{
    int curPrinter = [printerChooser selectedSegment];
    int emu = [sender indexOfSelectedItem];
    [self setIntResource:[NSString stringWithFormat:@"Printer%sTextDevice", tag[curPrinter]]
                 toValue:emu];
}

-(IBAction)changedPrinterTextDevice:(id)sender
{
    int id = [sender tag];
    NSString *value = [sender stringValue];
    [self setStringResource:@"PrinterTextDevice%d" withNumber:id toValue:value];
}

@end
