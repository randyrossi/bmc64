/*
 * sidsettingswindowcontroller.h - SIDSettings dialog controller
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
#import "sid/sid.h"

@interface SIDSettingsWindowController : VICEResourceWindowController
{
    IBOutlet NSPopUpButton *engineModelButton;
    IBOutlet NSButton      *stereoSidEnableButton;
    IBOutlet NSPopUpButton *stereoSidAddressButton;
    IBOutlet NSButton      *filterEnableButton;
    
    IBOutlet NSPopUpButton *residSampleMethodButton;
    IBOutlet NSSlider      *residPassbandSlider;
    IBOutlet NSTextField   *residPassbandText;
    
    BOOL hasStereo;
    BOOL hasFilters;
    const int *stereoAddrs;
    sid_engine_model_t **engine_model_list;
}

-(void)updateResources:(NSNotification *)notification;

-(IBAction)popupEngineModel:(id)sender;
-(IBAction)toggleStereoSid:(id)sender;
-(IBAction)popupStereoSidAddress:(id)sender;
-(IBAction)toggleSidFilter:(id)sender;

-(IBAction)popupResidSampleMethod:(id)sender;
-(IBAction)slideResidPasspand:(id)sender;
-(IBAction)enterResidPassband:(id)sender;

@end
