/*
 * printersettingswindowcontroller.h - PrinterSettings dialog controller
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

@interface PrinterSettingsWindowController : VICEResourceWindowController
{
    IBOutlet NSSegmentedControl *printerChooser;

    IBOutlet NSButton *useIECDevice;
    IBOutlet NSPopUpButton *printerEmulation;
    IBOutlet NSButton *sendFormFeed;

    IBOutlet NSPopUpButton *driver;
    IBOutlet NSPopUpButton *output;
    IBOutlet NSPopUpButton *textDevice;

    IBOutlet NSTextField *printerTextDevice1;
    IBOutlet NSTextField *printerTextDevice2;
    IBOutlet NSTextField *printerTextDevice3;
    
    BOOL hasUserportPrinter;
    BOOL hasIECPrinter;
}

-(void)updateResources:(NSNotification *)notification;

-(IBAction)togglePrinter:(id)sender;

-(IBAction)changedIECDevice:(id)sender;
-(IBAction)changedPrinterEmulation:(id)sender;
-(IBAction)doSendFormFeed:(id)sender;
-(IBAction)changedDriver:(id)sender;
-(IBAction)changedOutput:(id)sender;
-(IBAction)changedTextDevice:(id)sender;

-(IBAction)changedPrinterTextDevice:(id)sender;

@end
