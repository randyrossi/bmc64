/*
 * netplaycontrolwindowcontroller.h - netplay control
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

@interface NetplayControlWindowController : VICEResourceWindowController
{
    IBOutlet NSButton *connectToButton;
    IBOutlet NSButton *startServerButton;
    IBOutlet NSButton *disconnectButton;

    IBOutlet NSTextField *serverNameTextField;
    IBOutlet NSTextField *serverPortTextField;

    IBOutlet NSButton *control00;
    IBOutlet NSButton *control01;
    IBOutlet NSButton *control02;
    IBOutlet NSButton *control03;
    IBOutlet NSButton *control04;

    IBOutlet NSButton *control05;
    IBOutlet NSButton *control06;
    IBOutlet NSButton *control07;
    IBOutlet NSButton *control08;
    IBOutlet NSButton *control09;

    IBOutlet NSTextField *infoTextField;

    NSButton *control[10];
    int flags[10];
}

-(void)updateResources:(NSNotification *)notification;

-(IBAction)connectTo:(id)sender;
-(IBAction)startServerButton:(id)sender;
-(IBAction)disconnect:(id)sender;

-(IBAction)setServerName:(id)sender;
-(IBAction)setServerPort:(id)sender;
-(IBAction)changedControl:(id)sender;

@end
